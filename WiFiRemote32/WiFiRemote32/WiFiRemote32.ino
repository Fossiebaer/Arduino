/*
 Name:		WiFiRemote32.ino
 Created:	28.02.2018 18:06:12
 Author:	olli
*/
#include <dummy.h>
#include <EspHomeBase.h>
// the setup function runs once when you press reset or power the board

EspHomeBase *_server;
char tempBuf[TEMP_SIZE];
#define BUTTON_PIN_BITMASK 0x200000000 // 2^33 in hex, means gpio 33
bool sent;

void AckCallback(byte* payload, unsigned int length) {
	esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
	Serial.println("Going to sleep...");
	esp_deep_sleep_start();
}

void sendSignal() {
	_server->sendMqttMessage(MQTT_SET, "ON", "1");
}

void setup() {
	Serial.begin(115200);
	Serial.println("");
	sent = false;
	_server = EspHomeBase::getInstance();
	//_server->changeMode(MODE_MQTT);
	while (!EspHomeBase::ready) {
		sleep(5);
	}
	Serial.println("Register MQTT callback...");
	_server->registerMqttCallback("ON", MQTT_STATE, AckCallback);
	
	
}

// the loop function runs over and over again until power down or reset
void loop() {
	_server->process();
	if (!sent) {
		Serial.println("Sending Signal...");
		sendSignal();
		sent = true;
	}
}
