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
WiFiClient *EspHomeBase::_netclient = 0;
PubSubClient *EspHomeBase::_mqttclient = 0;

int replaceFunc(char *buf, const char*needle);
int getAPs(char *buf, const char *needle);

EspHomeBase * EspHomeBase::getInstance()
{
	if (_instance == 0) {
		_instance = new EspHomeBase();
	}
	return _instance;
}

void EspHomeBase::setMode(DeviceMode mode)
{
	if (mode != devMode) {
		devMode = mode;
		EEPROM.write(3, (uint8_t)(mode & 0x0F));
		EEPROM.commit();
		ESP.restart();
	}
}

void EspHomeBase::registerMqttCallback(const char * channel, MqttCallback cb)
{
	_mqttclient->setCallback(cb);
}

void EspHomeBase::registerHttpCallback(const char * url, WebRequestMethod method, HttpCallback cb)
{
}

void EspHomeBase::sendMqttMessage(MessageType cmd, const char * channel, const char * val)
{
}

EspHomeBase::EspHomeBase()
{
	EEPROM.begin(512);
	int rsc = EEPROM.read(1);
	int test = EEPROM.read(0);
	rsc++;
	EEPROM.write(1, rsc);
	EEPROM.commit();
	if (rsc > 5){
		test = 0;
		EEPROM.write(0, test);
		EEPROM.commit();
	}
	if (test == 1) {
		int m = EEPROM.read(2);
		if (m <= 3) {
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
		uint8_t m = (int)devMode & 0x0F;
		EEPROM.write(0, 1);
		EEPROM.write(2, m);
		EEPROM.commit();
	}
	if (devMode == MODE_CONFIG) {
		WiFi.mode(WIFI_AP_STA);
		WiFi.softAP("ESP_Config_AP", "passwd");
		EspHomeBase::_server = ConfigServer::getInstance();
		IPAddress myIP = WiFi.softAPIP();
		_dnsServer = new DNSServer();
		_dnsServer->start(53, "*", myIP);
		_server->addReplaceHandler("%ssid_box%", getAPs);
		_server->addReplaceHandler("%mqtt_server%", replaceFunc);
		_server->addReplaceHandler("%mqtt_user%", replaceFunc);
		_server->addReplaceHandler("%mqtt_top_topic%", replaceFunc);
		_server->addReplaceHandler("%mqtt_dev_topic%", replaceFunc);
		_server->startConfig();
	}

}

EspHomeBase::~EspHomeBase()
{
}

int replaceFunc(char *buf, const char*needle) {
	int len = strlen(needle);
	len = len - 2;
	memcpy(tempBuf, needle + 1, len);
	tempBuf[len] = '\0';
	const char *val = getConfigParam(tempBuf);
	strcpy(buf, val);
	return strlen(val);
}

int getAPs(char *buf, const char *needle) {
	int n = WiFi.scanNetworks();
	if (n > 0) {
		for (int i = 0; i < n; i++) {
			strcat(buf, "<option>");
			strcat(buf, WiFi.SSID(i).c_str());
			strcat(buf, "</option>");
		}
	}
	return n;
}