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
	if (EEPROM.read(0) == 1) {
		devMode = MODE_UNSPEC;
		int m = EEPROM.read(2);
		if (m <= 3) {
			devMode = static_cast<DeviceMode>(m);
		}
	}
	else {
		devMode = MODE_CONFIG;
	}
	if (devMode == MODE_UNSPEC) {
		devMode = MODE_CONFIG;
		uint8_t m = (int)devMode & 0x0F;
		EEPROM.write(2, m);
	}
	if (devMode == MODE_CONFIG) {
		
	}

}

EspHomeBase::~EspHomeBase()
{
}
