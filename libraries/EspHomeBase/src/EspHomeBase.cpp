/*
 Name:		EspHomeBase.cpp
 Created:	27.02.2018 17:24:11
 Author:	olli
 Editor:	http://www.visualmicro.com
*/

#include <EEPROM.h>
#include "EspHomeBase.h"

EspHomeBase *EspHomeBase::_instance = 0;
ConfigServer *EspHomeBase::_server = 0;
DNSServer *EspHomeBase::_dnsServer = 0;
WiFiClient *EspHomeBase::_netclient;
PubSubClient *EspHomeBase::_mqttclient = 0;
MqttCBEntry EspHomeBase::callbacks[5];
int EspHomeBase::cbCount = 0;
bool EspHomeBase::ready = false;
char locBuf[500];
bool subcribed;

int replaceFunc(char *buf, const char*needle);
int getAPs(char *buf, const char *needle);

#if defined(ARDUINO_ARCH_ESP32)
void WiFiEvent(WiFiEvent_t event);
#elif defined(ARDUINO_ARCH_ESP8266)
WiFiEventHandler WifiConnectHandler;
WiFiEventHandler WifiDisconnectHandler;
void onWifiConnect(const WiFiEventStationModeGotIP &evt);
void onWifiDisconnect(const WiFiEventStationModeDisconnected &evt);
#endif

EspHomeBase * EspHomeBase::getInstance()
{
	if (_instance == 0) {
		_instance = new EspHomeBase();
	}
	return _instance;
}

void EspHomeBase::changeMode(DeviceMode mode)
{
	uint8_t v = EEPROM.read(2);
	DeviceMode m = static_cast<DeviceMode>(v);
	if (mode != m) {
		devMode = mode;
		EEPROM.write(3, (uint8_t)(mode & 0x0F));
		EEPROM.commit();
		ESP.restart();
	}
}

void EspHomeBase::registerMqttCallback(const char * channel, MessageType type, MqttCallback cb)
{
	if (devMode == MODE_MQTT && ready && cbCount < 5) {
		for (int i = 0; i < cbCount; i++) {
			if (strcmp(channel, callbacks[i].channel) == 0) {
				return;
			}
		}
		MqttCBEntry entry;
		strcpy(entry.channel, channel);
		entry.cb = cb;
		entry.type = type;
		callbacks[cbCount] = entry;
		cbCount ++;
	}
}

void EspHomeBase::registerHttpCallback(const char * url, WebRequestMethod method, HttpCallback cb)
{
	_server->addUrlHandler(url, method, cb);
}

bool EspHomeBase::sendMqttMessage(MessageType cmd, const char * channel, const char * val)
{
	if (_mqttclient != 0 && _mqttclient->connected()) {
		const char *tpc = getConfigParam("mqtt_top_topic");
		const char *dtp = getConfigParam("mqtt_dev_topic");
		strcpy(tempBuf, tpc);
		if (tempBuf[strlen(tempBuf) - 1] != '/' && dtp[0] != '/') {
			strcat(tempBuf, "/");
		}
		strcat(tempBuf, dtp);
		if (tempBuf[strlen(tempBuf) - 1] != '/') {
			strcat(tempBuf, "/");
		}
		switch (cmd) {
		case MQTT_GET: 
			strcat(tempBuf, "GET/");
			break;
		case MQTT_SET:
			strcat(tempBuf, "SET/");
			break;
		case MQTT_STATE:
			strcat(tempBuf, "STATUS/");
			break;
		}
		strcat(tempBuf, channel);
		Serial.println(tempBuf);
		return _mqttclient->publish(tempBuf, val);
	}
	return false;
}

void EspHomeBase::process()
{
	if (devMode == MODE_CONFIG) {
		_dnsServer->processNextRequest();
	}
	else if (devMode == MODE_MQTT) {
		if (!subcribed) {
			
			subcribed = true;
		}
		_mqttclient->loop();
	}
}

EspHomeBase::EspHomeBase()
{
	subcribed = false;
	EEPROM.begin(512);
	Serial.println("Startup...");
	int rsc = EEPROM.read(1);
	int test = EEPROM.read(0);
	rsc++;
	EEPROM.write(1, rsc);
	EEPROM.commit();
	if (rsc > 5){
		Serial.println("Connection failed too often. Starting Config...");
		test = 0;
		EEPROM.write(0, test);
		EEPROM.commit();
	}
	if (test == 1) {
		int m = EEPROM.read(2);
		const char *set = getConfigParam("set");
		if (m <= 3 && strcmp(set, "1") == 0 ) {
			Serial.println("Configuration present, starting normal mode...");
			devMode = static_cast<DeviceMode>(m);
		}
		else {
			devMode = MODE_UNSPEC;
		}
	}
	else {
		devMode = MODE_UNSPEC;
	}
	if (devMode == MODE_UNSPEC) {
		devMode = MODE_CONFIG;
		//uint8_t m = (int)devMode & 0x0F;
		EEPROM.write(0, 1);
		//EEPROM.write(2, m);
		EEPROM.commit();
	}
	if (devMode == MODE_CONFIG) {
		Serial.println("Starting config mode...");
		WiFi.mode(WIFI_AP_STA);
		WiFi.softAP("ESP_Config_AP", "passwd0815");
		EspHomeBase::_server = ConfigServer::getInstance();
		IPAddress myIP = WiFi.softAPIP();
		Serial.println(myIP);
		_dnsServer = new DNSServer();
		_dnsServer->start(53, "*", myIP);
		_server->addReplaceHandler("%ssid_box%", getAPs);
		_server->addReplaceHandler("%mqtt_server%", replaceFunc);
		_server->addReplaceHandler("%mqtt_user%", replaceFunc);
		_server->addReplaceHandler("%mqtt_top_topic%", replaceFunc);
		_server->addReplaceHandler("%mqtt_dev_topic%", replaceFunc);
		_server->startConfig();
		uint8_t m = EEPROM.read(2);
		devMode = static_cast<DeviceMode>(m);
		EEPROM.write(0, 1);
		//EEPROM.write(2, m);
		EEPROM.write(1, 0);
		EEPROM.commit();

	} else {
		WiFi.mode(WIFI_STA);
#if defined(ARDUINO_ARCH_ESP32)
		WiFi.onEvent(WiFiEvent);
#elif defined(ARDUINO_ARCH_ESP8266)
		WifiConnectHandler = WiFi.onStationModeGotIP(&onWifiConnect);
		WifiDisconnectHandler = WiFi.onStationModeDisconnected(&onWifiDisconnect);
#endif
		WiFi.begin(getConfigParam("ssid_box"), getConfigParam("passwd"));
		Serial.print("Connecting WiFi, reset ");
		Serial.printf("%d", 5 - EEPROM.read(1));
		Serial.println(" times to enter config mode.");
		while (WiFi.status() != WL_CONNECTED) {
			Serial.print(".");
#if defined(ARDUINO_ARCH_ESP32)
			sleep(1);
#elif defined(ARDUINO_ARCH_ESP8266)
			delay(1);
#endif
		}
		Serial.print("Connected. IP: ");
		Serial.println(WiFi.localIP());
		if (devMode == MODE_MQTT) {
			static WiFiClient locCl;
			//_mqttclient = new PubSubClient(getConfigParam("mqtt_server"), 1883, EspHomeBase::mqttCallback, locCl);
			_mqttclient = new PubSubClient(locCl);
			_mqttclient->setServer(getConfigParam("mqtt_server"), 1883);
			_mqttclient->setCallback(EspHomeBase::mqttCallback);
			//readFile("uuid", locBuf, 500);
			if (_mqttclient->connect("esp32_test", getConfigParam("mqtt_user"), getConfigParam("mqtt_pass"))) {
				while (!_mqttclient->connected()) {
					Serial.print(".");
#if defined(ARDUINO_ARCH_ESP32)
					sleep(1);
#elif defined(ARDUINO_ARCH_ESP8266)
					delay(1);
#endif
				}
				const char *tempStr = getConfigParam("mqtt_top_topic");
				if (tempStr[0] != '/') {
					strcpy(locBuf, "/");
					strcat(locBuf, tempStr);
				}
				else {
					strcpy(locBuf, tempStr);
				}
				const char *devStr = getConfigParam("mqtt_dev_topic");
				if (locBuf[strlen(locBuf) - 1] != '/' && devStr[0] != '/') {
					strcat(locBuf, "/");
				}
				else if (locBuf[strlen(locBuf) - 1] == '/' && devStr[0] == '/') {
					locBuf[strlen(locBuf) - 1] = '\0';
				}
				strcat(locBuf, devStr);
				if (locBuf[strlen(locBuf) - 1] != '/') {
					strcat(locBuf, "/#");
				}
				else {
					strcat(locBuf, "#");
				}
				_mqttclient->subscribe(locBuf);
			}
			else {
				Serial.println("MQTT_Connection failed!");
			}
		}
		if (devMode == MODE_HTTP) {

		}
	}
	ready = true;
	Serial.println("EspHomeBase ready!");
}

EspHomeBase::~EspHomeBase()
{
	_server->~ConfigServer();
	_dnsServer = nullptr;
	_netclient->~WiFiClient();
	_mqttclient = nullptr;
	_instance = nullptr;
}

int replaceFunc(char *buf, const char*needle) {
	int len = strlen(needle);
	Serial.print("Replacing ");
	Serial.println(needle);
	len = len - 2;
	memcpy(locBuf, needle + 1, len);
	locBuf[len] = '\0';
	Serial.println(locBuf);
	const char *val = getConfigParam(locBuf);
	Serial.print("Wert: ");
	Serial.println(val);
	if (val != nullptr && val != NULL && strlen(val) > 0 && val[0] != '\0') {
		strcpy(buf, val);
		return strlen(val);
	}
	return 0;
}

int getAPs(char *buf, const char *needle) {
	int n = WiFi.scanNetworks();
	if (n > 0) {
		for (int i = 0; i < n; i++) {
			Serial.print("Added AP: ");
			Serial.println(WiFi.SSID(i).c_str());
			strcat(buf, "<option>");
			strcat(buf, WiFi.SSID(i).c_str());
			strcat(buf, "</option>");
		}
	}
	return n;
}

void EspHomeBase::mqttCallback(char * topic, byte * payload, unsigned int length)
{
	Serial.println("MQTT-Callback...");
	Serial.println(topic);
	for (int i = 0; i < cbCount; i++){
		switch (callbacks[i].type) {
		case MQTT_GET:
			if (strstr(topic, "/GET/") > 0) {
				if (strstr(topic, callbacks[i].channel) > 0) {
					callbacks[i].cb(payload, length);
					continue;
				}
			}
		case MQTT_SET:
			if (strstr(topic, "/SET/") > 0) {
				if (strstr(topic, callbacks[i].channel) > 0) {
					callbacks[i].cb(payload, length);
					continue;
				}
			}
		case MQTT_STATE:
			if (strstr(topic, "/STATE/") > 0) {
				if (strstr(topic, callbacks[i].channel) > 0) {
					callbacks[i].cb(payload, length);
					continue;
				}
			}
		}
	}
}

#if defined(ARDUINO_ARCH_ESP32)
void WiFiEvent(WiFiEvent_t event)
{
	Serial.printf("[WiFi-event] event: %d\n", event);
	switch (event) {
		case SYSTEM_EVENT_STA_GOT_IP:
		EEPROM.write(1, 0);
		EEPROM.commit();
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		uint8_t val = EEPROM.read(1);
		EEPROM.write(1, (val + 1));
		EEPROM.commit();
		break;
	}
}
#elif defined(ARDUINO_ARCH_ESP8266)
void onWifiConnect(const WiFiEventStationModeGotIP &evt) {
	EEPROM.write(1, 0);
	EEPROM.commit();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected &evt) {
	if (evt.reason == WIFI_DISCONNECT_REASON_AUTH_EXPIRE || evt.reason == WIFI_DISCONNECT_REASON_AUTH_FAIL || evt.reason == WIFI_DISCONNECT_REASON_NO_AP_FOUND) {
		uint8_t val = EEPROM.read(1);
		val++;
		EEPROM.write(1, val);
		EEPROM.commit();

		if (val >= 5) {
			ESP.restart();
		}
	}
}
#endif
