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
	MODE_AP_HTTP=2,
	MODE_CONFIG=3,
	MODE_UNSPEC=4
};

enum MessageType {
	MQTT_SET,
	MQTT_GET,
	MQTT_STATE
};

typedef void(*MqttCallback)(byte* payload, unsigned int length);
typedef void(*HttpCallback)(AsyncWebServerRequest *req);

typedef struct MqttCBEntry {
	char channel[16];
	MessageType type;
	MqttCallback cb;
};


class EspHomeBase {
public:
	static EspHomeBase *getInstance();
	void changeMode(DeviceMode mode);
	void registerMqttCallback(const char *channel, MessageType type, MqttCallback cb);
	void registerHttpCallback(const char *url, WebRequestMethod method, HttpCallback cb);
	void registerReplacerCallback(const char needle[], replaceHandler handler);
	void replace(char * buf, char * src);
	bool sendMqttMessage(MessageType cmd, const char *channel, const char *val);
	void process();
	static bool ready;
	DeviceMode devMode;
private:
	EspHomeBase();
	~EspHomeBase();
	static void mqttCallback(char* topic, byte* payload, unsigned int length);
	DeviceMode mode;
	static int cbCount;
	static MqttCBEntry callbacks[];
	static EspHomeBase *_instance;
	static ConfigServer *_server;
	static DNSServer *_dnsServer;
	static WiFiClient *_netclient;
	static PubSubClient *_mqttclient;
};
#endif

