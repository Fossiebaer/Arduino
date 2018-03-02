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

EspHomeBase * EspHomeBase::getInstance()
{
	if (_instance == 0) {
		_instance = new EspHomeBase();
	}
	return _instance;
}

void EspHomeBase::setMode(DeviceMode mode)
{
	if (mode);
}

void EspHomeBase::registerMqttCallback(const char * channel, MqttCallback cb)
{
}

void EspHomeBase::registerHttpCallback(const char * url, const char * method, HttpCallback cb)
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

	}

}

EspHomeBase::~EspHomeBase()
{
}
