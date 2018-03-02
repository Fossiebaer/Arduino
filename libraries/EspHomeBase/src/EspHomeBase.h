/*
 Name:		EspHomeBase.h
 Created:	27.02.2018 17:24:11
 Author:	olli
 Editor:	http://www.visualmicro.com
*/

#ifndef _EspHomeBase_h
#define _EspHomeBase_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include "FileAccess\FileAccess.h"
#include "ConfigServer\ConfigServer.h"
#include <PubSubClient.h>

enum DeviceMode {
	MODE_HTTP=0,
	MODE_MQTT=1,
	MODE_CONFIG=2,
	MODE_UNSPEC=3
};

enum MessageType {
	CMD_SET,
	STATE_GET,
	STATE_SET
};

typedef void (*MqttCallback)(char *topic, byte* payload, unsigned int length);
typedef void(*HttpCallback)(AsyncWebServerRequest *req);

class EspHomeBase {
public:
	EspHomeBase *getInstance();
	void setMode(DeviceMode mode);
	void registerMqttCallback(const char *channel, MqttCallback cb);
	void registerHttpCallback(const char *url, WebRequestMethod method, HttpCallback cb);
	void sendMqttMessage(MessageType cmd, const char *channel, const char *val);  
private:
	EspHomeBase();
	~EspHomeBase();
	DeviceMode mode;
	static EspHomeBase *_instance;
	static ConfigServer *_server;
	static DNSServer *_dnsServer;
	static WiFiClient *_netclient;
	static PubSubClient *_mqttclient;
	DeviceMode devMode;
};
#endif

