/*
 Name:		MQTTSensor.ino
 Created:	26.05.2019 09:55:55
 Author:	Oliver Lenert

 This example demonstrates a simple switch for smarthome application via MQTT broker

*/
#include <EspHomeBase.h>

EspHomeBase *_server;
char tempBuf[TEMP_SIZE];
#define BUTTON_PIN_BITMASK 0x200000000 // 2^33 in hex, means gpio 33, ESP32 only
bool sent;

void AckCallback(byte* payload, unsigned int length) {
#if defined(ARDUINO_ARCH_ESP32)
	esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
	esp_deep_sleep_start();
#elif defined(ARDUINO_ARCH_ESP8266)
	ESP.deepSleep(0);
#endif
}

void sendSignal() {
	_server->sendMqttMessage(MQTT_SET, "ON", "1");
}

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);
	Serial.println("");
	sent = false;
	_server = EspHomeBase::getInstance();
	while (!EspHomeBase::ready) {
		delay(5);
	}
	_server->changeMode(MODE_MQTT);
	_server->registerMqttCallback("ON", MQTT_STATE, AckCallback);
}

// the loop function runs over and over again until power down or reset
void loop() {
	_server->process();
	if (!sent) {
		sendSignal();
		sent = true;
	}
}
