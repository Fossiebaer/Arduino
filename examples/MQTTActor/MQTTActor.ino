/*
Name:		MQTTActor.ino
Created:	12.03.2018 17:04:12
Author:	Oliver Lenert

Demonstrates the usage of EspHomeBase library and NeoPixelBus_by_Makuna
for building an SmartHomeActor based on an ESP WiFi microcontroller

This example is for connecting to a smarthome server like openHAB or FHEM via
a MQTT broker

*/
#include <NeoPixelBus.h>
#include <NeoPixelBrightnessBus.h>
#include <NeoPixelAnimator.h>
#include <EspHomeBase.h>
#include <regex.h>

// For checking if message payload is numerical
regex_t numericRegex;

EspHomeBase *_server; // Instance of HomeBase  
char tempBuf[TEMP_SIZE]; // Buffer for Filecontents, TEMP_SIZE is defined in FileAccess.h
const uint16_t numPixel = 115;
const uint8_t pinPixel = 5; // Pin for LED-Data. Not relevant for ESP8266

const float bfactor = 0.5f; // Factor for limiting Brightness

boolean colorChange = false; // Status of color change
boolean lastChange = false;  // Save last toggle bit 

#define COLOR_SATURATION 128 // Set to 50%, because HsbColor does seem to work like HslColor, which would be white at 50% luminance

// Instantiate LED driver
NeoPixelBus<NeoGrbFeature, NeoEsp8266Dma800KbpsMethod> strip(numPixel, pinPixel);
// You could try this instead, if above doesn't work
//NeoPixelBus<NeoGrbFeature, NeoEsp8266Dma400KbpsMethod> strip(numPixel, pinPixel);
NeoPixelAnimator animations(numPixel, NEO_CENTISECONDS); // Animator for LED Strip


// Stores the colorvalues and curve type for animations
struct MyAnimationState {
	RgbColor StartingColor;
	RgbColor EndingColor;
	AnimEaseFunction Easeing;
};

// Struct for storing LED State
struct MyLightState {
	float hue = 0.0f;
	float sat = 0.0f;
	float lum = 0.0f;
};

MyAnimationState animationState[numPixel]; // Objekt for storing pixel values
MyLightState lightingState; // Single LED state object

// Updates the leds on every animation step,
// is called once per pixel
void AnimUpdate(const AnimationParam &param) {
	float progress = param.progress;

	RgbColor updatedColor = RgbColor::LinearBlend(
		animationState[param.index].StartingColor,
		animationState[param.index].EndingColor,
		progress);
	strip.SetPixelColor(param.index, updatedColor);
}

// Rotates the LEDs values 
void RotationUpdate(const AnimationParam &param) {
	if (param.state == AnimationState_Completed) {
		animations.RestartAnimation(param.index);

		strip.RotateLeft(1);
	}
}

// Replaces the placeholders in the website with the current color values
int colorReplacer(char *buf, const char *needle) {
	if (strcmp(needle, "%color%") == 0) {
		sprintf(buf, "#%02x%02x%02x", animationState[0].StartingColor.R, animationState[0].StartingColor.G, animationState[0].StartingColor.B);
		Serial.print("colorReplacer: ");
		Serial.println(buf);
	}
	else if (strcmp(needle, "%color2%") == 0) {
		sprintf(buf, "#%02x%02x%02x", animationState[0].EndingColor.R, animationState[0].EndingColor.G, animationState[0].EndingColor.B);
		Serial.print("color2Replacer: ");
		Serial.println(buf);
	}
}

// Selects random color and sets the LED values
void PickRandom()
{
	HsbColor newColor = HsbColor(random(360) / 360.0f, 1.0f, lightingState.lum);
	for (int pixel = 0; pixel < numPixel; pixel++) {
		animationState[pixel].StartingColor = strip.GetPixelColor(pixel); // Starting color = current color
		animationState[pixel].EndingColor = newColor; // Ending color = random color
		animations.StartAnimation(pixel, 500, AnimUpdate); 
	}
}

// Send a state message per MQTT
// Message contains Hue,Saturation,Brightness
void sendState() {
	char msgBuf[12];
	sprintf(msgBuf, "%d,%d,%d", (int)(lightingState.hue * 360), (int)(lightingState.sat * 100), (int)((lightingState.lum / bfactor) * 100));
	_server->sendMqttMessage(MQTT_STATE, "color", msgBuf);
}

//MQTT-Callback functions
void handleOnOff(byte* payload, unsigned int len) {
	HsbColor newCol;
	if (payload[0] == '0') { // Switch off
		newCol = HsbColor(0.0, 0.0, 0.0);
	}
	else if (payload[0] == '1') { // Switch on

		if (lightingState.lum < 0.2) { 
			//newCol = HsbColor(0.07f, 0.2f, 0.5f);
			lightingState.hue = 0.7f;
			lightingState.lum = 0.5f;
			lightingState.sat = 0.2f;
		}
		newCol = HsbColor(lightingState.hue, lightingState.sat, lightingState.lum);
	}
	else { // Unknown command value
		return;
	}
	/*
	*  Set Pixels colors
	*/
	for (int i = 0; i < numPixel; i++) {
		strip.SetPixelColor(i, newCol);
	}
	strip.Show();
	delay(1);
}

// MQTT callback function for switching color changer on and off
void handleStartStopAnimation(byte *payload, unsigned int len) {
	boolean chgStatus = false;
	if (len > 0 && payload[0] == 0x31) { // If payload == "1"
		chgStatus = true;
	}
	if (lastChange != chgStatus) {  // State changed
		lastChange = chgStatus;
		if (colorChange) { // was on, switch off
			colorChange = false;
			// Set to last ligthing values
			HsbColor newCol = HsbColor(lightingState.hue, lightingState.sat, lightingState.lum);
			for (int i = 0; i < numPixel; i++) {
				strip.SetPixelColor(i, newCol);
			}
			strip.Show();
		}
		else { // was off, switch on
			PickRandom();
			colorChange = true;
		}
	}
}

// MQTT callback function for setting brightness change
void handleBrightness(byte* payload, unsigned int len) {
	// copy payload to buffer
	int j = 0;
	for (int i = 0; i < len; i++) {
		tempBuf[i] = (char)payload[i];
		j++;
	}
	tempBuf[j] = '\0';
	// check if payload is numerical
	int res = regexec(&numericRegex, tempBuf, 0, NULL, 0);
	if (!res) { // is numerical
		// calculate new brightness value
		float val = strtof(tempBuf, NULL);
		float brightness = val / 100.0f;
		brightness = brightness * bfactor;
		// set new brightness value
		lightingState.lum = brightness;
		HsbColor newCol = HsbColor(lightingState.hue, lightingState.sat, brightness);
		for (int i = 0; i < numPixel; i++) {
			strip.SetPixelColor(i, newCol);
		}
		strip.Show();
		sendState();
	}
}

// MQTT callback function for setting new color value
void handleColor(byte* payload, unsigned int len) {
	// copy payload to buffer
	int j = 0;
	for (int i = 0; i < len; i++) {
		tempBuf[i] = (char)payload[i];
		j++;
	}
	tempBuf[j] = '\0';
	// split buffer into single values
	char *ptr;
	int i = 0;
	ptr = strtok(tempBuf, ",");
	Serial.println(ptr);
	// convert values for hue, saturation and brightness
	float hue = 0.0;
	float sat = 0.0;
	float lum = 0.0;
	while (ptr != NULL && i < 3) {
		float val = strtof(ptr, NULL);
		if (i == 0) {
			val = val / 360.0f;
			hue = val;
		}
		else if (i == 1) {
			val = val / 100.0f;
			sat = val;
		}
		else if (i = 2) {
			val = val / 100.0f;
			val = val * bfactor;
			//lum = val;
			lum = lightingState.lum;
		}

		i++;
		ptr = strtok(NULL, ",");
	}
	// set new color values
	lightingState.hue = hue;
	lightingState.sat = sat;
	lightingState.lum = lum;
	HsbColor col = HsbColor(hue, sat, lum);
	for (int i = 0; i < numPixel; i++) {
		strip.SetPixelColor(i, col);
	}
	delay(1);
	strip.Show();
	sendState();
}

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);
	Serial.println("");
	strip.Begin();
	HsbColor startcol = HsbColor(0.067f, 1.0f, 0.2f);
	strip.SetPixelColor(0, startcol);
	strip.Show();
	delay(1);
	// get instance of EspHomeBase
	_server = EspHomeBase::getInstance();
	// wait till EspHomeBase is ready and set up
	while (!EspHomeBase::ready) {
		delay(5);
	}
	// set connection mode of EspHomeBase.
	_server->changeMode(MODE_MQTT);

	_server->registerMqttCallback("onoff", MQTT_SET, handleOnOff);
	_server->registerMqttCallback("brightness", MQTT_SET, handleBrightness);
	_server->registerMqttCallback("color", MQTT_SET, handleColor);
	_server->registerMqttCallback("changer", MQTT_SET, handleStartStopAnimation);

	HsbColor col = HsbColor(0.07f, 0.0f, 0.5f);
	for (int i = 0; i < numPixel; i++) {
		strip.SetPixelColor(i, col);
	}
	strip.Show();
	Serial.println("");
}

// the loop function runs over and over again until power down or reset
void loop() {
	if (colorChange) {
		if (animations.IsAnimating())
		{
			//Serial.println("loop animating");
			// the normal loop just needs these two to run the active animations
			animations.UpdateAnimations();
			strip.Show();
		}
		else
		{
			// no animations runnning, start some 
			//
			PickRandom(); // 0.0 = black, 0.25 is normal, 0.5 is bright
		}
	}
	_server->process();
	delay(2);
}