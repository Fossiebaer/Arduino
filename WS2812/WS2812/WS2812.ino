/*
Name:		WS2812.ino
Created:	12.03.2018 17:04:12
Author:	olli
*/
#include <FastLED.h>
#include <dummy.h>
#include <EspHomeBase.h>
// the setup function runs once when you press reset or power the board

EspHomeBase *_server;
char tempBuf[TEMP_SIZE];

#define NUM_LEDS 1

#define DATA_PIN 5

CRGB leds[NUM_LEDS];

void handleGet(AsyncWebServerRequest  *req) {
	AsyncWebServerResponse *resp = req->beginResponse(SPIFFS, "/ws2812.html");

	req->send(resp);
}

void handlePost(AsyncWebServerRequest *req) {
	int num = req->args();
	for (int i = 0; i < num; i++) {
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

		for (int i = 0; i < NUM_LEDS; i++) {
			leds[i].r = r;
			leds[i].g = g;
			leds[i].b = b;
			delay(1);
		}
		FastLED.show();
		readFile("/ws2812.html", tempBuf, TEMP_SIZE);
		_server->replace(pageBuf, tempBuf);
		AsyncWebServerResponse *resp = req->beginResponse(200, "text/html", pageBuf);
		req->send(resp);
	}
}

int colorReplacer(char *buf, const char *needle) {
	if (strcmp(needle, "%color%") == 0) {
		sprintf(buf, "#%02x%02x%02x", leds[0].r, leds[0].g, leds[0].b);
		Serial.print("colorReplacer: ");
		Serial.println(buf);
	}
}

void setup() {
	Serial.begin(115200);
	Serial.println("");
	FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
	_server = EspHomeBase::getInstance();
	while (!EspHomeBase::ready) {
		sleep(5);
	}
	_server->changeMode(MODE_HTTP);
	_server->registerHttpCallback("/ws2812", HTTP_GET, handleGet);
	_server->registerHttpCallback("/ws2812", HTTP_POST, handlePost);
	_server->registerReplacerCallback("%color%", colorReplacer);

}

// the loop function runs over and over again until power down or reset
void loop() {
	delay(30);
}
