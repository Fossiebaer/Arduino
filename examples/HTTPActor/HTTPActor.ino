/*
Name:		HTTPActor.ino
Created:	12.03.2018 17:04:12
Author:	Oliver Lenert

Demonstrates the usage of EspHomeBase library and NeoPixelBus_by_Makuna
for building an SmartHomeActor based on an ESP WiFi microcontroller

This example starts a webserver and presents a website for controlling the connected LEDs

*/
#include <NeoPixelBus.h>
#include <NeoPixelBrightnessBus.h>
#include <NeoPixelAnimator.h>
#include <EspHomeBase.h>


EspHomeBase *_server;
char tempBuf[TEMP_SIZE];

const uint16_t numPixel = 80;

const uint8_t pinPixel = 5;

#define COLOR_SATURATION 128

bool test = false;

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(numPixel, pinPixel);
NeoPixelAnimator animations(numPixel, NEO_CENTISECONDS);

// Stores the colorvalues and curve type for animations
struct MyAnimationState {
	RgbColor StartingColor;
	RgbColor EndingColor;
	AnimEaseFunction Easeing;
};

MyAnimationState animationState[numPixel];

// Updates the leds on every animation step
void AnimUpdate(const AnimationParam &param) {
	float progress = animationState[param.index].Easeing(param.progress);

	RgbColor updatedColor = RgbColor::LinearBlend(
		animationState[param.index].StartingColor,
		animationState[param.index].EndingColor,
		progress);
	strip.SetPixelColor(param.index, updatedColor);
}

// Delivers the website for controlling the leds
void handleGet(AsyncWebServerRequest  *req) {
	readFile("/ws2812.html", tempBuf, TEMP_SIZE);
	_server->replace(pageBuf, tempBuf);
	AsyncWebServerResponse *resp = req->beginResponse(200, "text/html", pageBuf);

	req->send(resp);
}

// Handles the POST request from the control website
void handlePost(AsyncWebServerRequest *req) {
	// if not yet done, start the led strip
	if (!test) {
		test = true;
	}
	// get first color value from request
	const char *color = req->arg("color").c_str();
	// convert string to RGB values
	memcpy(tempBuf, color + 1, 2);
	tempBuf[2] = '\0';
	uint8_t r = strtol(tempBuf, NULL, 16);
	memcpy(tempBuf, color + 3, 2);
	tempBuf[2] = '\0';
	uint8_t g = strtol(tempBuf, NULL, 16);
	strncpy(tempBuf, color + 5, 3);
	uint8_t b = strtol(tempBuf, NULL, 16);
	RgbColor c(r, g, b);
	// get second color from request
	const char *color2 = req->arg("color2").c_str();
	// convert string to RGB values
	memcpy(tempBuf, color2 + 1, 2);
	tempBuf[2] = '\0';
	uint8_t r2 = strtol(tempBuf, NULL, 16);
	memcpy(tempBuf, color2 + 3, 2);
	tempBuf[2] = '\0';
	uint8_t g2 = strtol(tempBuf, NULL, 16);
	strncpy(tempBuf, color2 + 5, 3);
	uint8_t b2 = strtol(tempBuf, NULL, 16);
	RgbColor c2(r2, g2, b2);



	// Update all LEDs with new colors and start the animation
	for (int i = 0; i < numPixel; i++) {
		animationState[i].StartingColor = c;
		animationState[i].EndingColor = c2;
		animationState[i].Easeing = NeoEase::CubicInOut;
		animations.StartAnimation(i, 250, AnimUpdate);
	}
	strip.Show();
	// return control website to browser
	readFile("/ws2812.html", tempBuf, TEMP_SIZE);
	_server->replace(pageBuf, tempBuf);
	AsyncWebServerResponse *resp = req->beginResponse(200, "text/html", pageBuf);
	req->send(resp);
}

// Replaces the placeholders in the website with the current color values
int colorReplacer(char *buf, const char *needle) {
	if (strcmp(needle, "%color%") == 0) {
		sprintf(buf, "#%02x%02x%02x", animationState[0].StartingColor.R, animationState[0].StartingColor.G, animationState[0].StartingColor.B);
	}
	else if (strcmp(needle, "%color2%") == 0) {
		sprintf(buf, "#%02x%02x%02x", animationState[0].EndingColor.R, animationState[0].EndingColor.G, animationState[0].EndingColor.B);
	}
}

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);
	Serial.println("");
	strip.Begin();

	RgbColor c(0, 0, 0);
	for (int i = 0; i < numPixel; i++) {
		strip.SetPixelColor(i, c);
	}
	strip.Show();
	// get instance of EspHomeBase
	_server = EspHomeBase::getInstance();
	// wait till EspHomeBase is ready and set up
	while (!EspHomeBase::ready) {
		delay(5);
	}
	// set connection mode of EspHomeBase. In this case connected to WiFi Network, presenting website
	_server->changeMode(MODE_HTTP);

	// register the control website with the webserver
	_server->registerHttpCallback("/ws2812", HTTP_GET, handleGet);
	_server->registerHttpCallback("/ws2812", HTTP_POST, handlePost);
	// register the color replacer with the webserver
	_server->registerReplacerCallback("%color%", colorReplacer);
	_server->registerReplacerCallback("%color2%", colorReplacer);
}

// the loop function runs over and over again until power down or reset
void loop() {
	// if animation is started, continue with next step
	if (animations.IsAnimating()) {
		animations.UpdateAnimations();
		strip.Show();
	}
	// if animation has ended, swap color values and restart animation
	else if (test) {
		for (int i = 0; i < numPixel; i++) {
			RgbColor temp = animationState[i].StartingColor;
			animationState[i].StartingColor = animationState[i].EndingColor;
			animationState[i].EndingColor = temp;
			animations.StartAnimation(i, 250, AnimUpdate);
		}
	}
	delay(50);
}
