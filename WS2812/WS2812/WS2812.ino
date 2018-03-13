/*
Name:		WS2812.ino
Created:	12.03.2018 17:04:12
Author:	olli
*/
#include <NeoPixelBus.h>
#include <NeoPixelBrightnessBus.h>
#include <NeoPixelAnimator.h>
#include <dummy.h>
#include <EspHomeBase.h>
// the setup function runs once when you press reset or power the board

EspHomeBase *_server;
char tempBuf[TEMP_SIZE];

const uint16_t numPixel = 3;

const uint8_t pinPixel = 5;

#define COLOR_SATURATION 128

bool test = false;

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(numPixel, pinPixel);
NeoPixelAnimator animations(numPixel, NEO_CENTISECONDS);

struct MyAnimationState {
	RgbColor StartingColor;
	RgbColor EndingColor;
	AnimEaseFunction Easeing;
};

MyAnimationState animationState[numPixel];

void AnimUpdate(const AnimationParam &param) {
	float progress = animationState[param.index].Easeing(param.progress);

	RgbColor updatedColor = RgbColor::LinearBlend(
		animationState[param.index].StartingColor,
		animationState[param.index].EndingColor,
		progress);
	strip.SetPixelColor(param.index, updatedColor);
}

void handleGet(AsyncWebServerRequest  *req) {
	readFile("/ws2812.html", tempBuf, TEMP_SIZE);
	_server->replace(pageBuf, tempBuf);
	AsyncWebServerResponse *resp = req->beginResponse(200, "text/html", pageBuf);

	req->send(resp);
}

void handlePost(AsyncWebServerRequest *req) {
	if (!test) {
		test = true;
		strip.Begin();
	}
	const char *color = req->arg("color").c_str();
	//strcpy(tempBuf, color + 1);
	memcpy(tempBuf, color + 1, 2);
	tempBuf[2] = '\0';
	uint8_t r = strtol(tempBuf, NULL, 16);
	memcpy(tempBuf, color + 3, 2);
	tempBuf[2] = '\0';
	uint8_t g = strtol(tempBuf, NULL, 16);
	strncpy(tempBuf, color + 5, 3);
	uint8_t b = strtol(tempBuf, NULL, 16);

	RgbColor c(r, g, b);
	const char *color2 = req->arg("color2").c_str();
	//strcpy(tempBuf, color + 1);
	memcpy(tempBuf, color2 + 1, 2);
	tempBuf[2] = '\0';
	uint8_t r2 = strtol(tempBuf, NULL, 16);
	memcpy(tempBuf, color2 + 3, 2);
	tempBuf[2] = '\0';
	uint8_t g2 = strtol(tempBuf, NULL, 16);
	strncpy(tempBuf, color2 + 5, 3);
	uint8_t b2 = strtol(tempBuf, NULL, 16);
	RgbColor c2(r2, g2, b2);

	for (int i = 0; i < numPixel; i++) {
		animationState[i].StartingColor = c;
		animationState[i].EndingColor = c2;
		animationState[i].Easeing = NeoEase::CubicInOut;
		//delay(1);
		animations.StartAnimation(i, 250, AnimUpdate);
	}
		
	strip.Show();
	readFile("/ws2812.html", tempBuf, TEMP_SIZE);
	_server->replace(pageBuf, tempBuf);
	AsyncWebServerResponse *resp = req->beginResponse(200, "text/html", pageBuf);
	req->send(resp);
	
}

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

void setup() {
	Serial.begin(115200);
	Serial.println("");
	
	_server = EspHomeBase::getInstance();
	while (!EspHomeBase::ready) {
		sleep(5);
	}
	_server->changeMode(MODE_AP_HTTP);
	_server->registerHttpCallback("/ws2812", HTTP_GET, handleGet);
	_server->registerHttpCallback("/ws2812", HTTP_POST, handlePost);
	_server->registerReplacerCallback("%color%", colorReplacer);
	_server->registerReplacerCallback("%color2%", colorReplacer);
	//strip.Begin();
}

// the loop function runs over and over again until power down or reset
void loop() {
	if (animations.IsAnimating()) {
		animations.UpdateAnimations();
		strip.Show();
	}
	else if(test){
		for (int i = 0; i < numPixel; i++) {
			RgbColor temp = animationState[i].StartingColor;
			animationState[i].StartingColor = animationState[i].EndingColor;
			animationState[i].EndingColor = temp;
			animations.StartAnimation(i, 250, AnimUpdate);
		}
	}
	delay(50);
}
