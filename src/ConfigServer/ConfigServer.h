// ConfigServer.h

#ifndef _CONFIGSERVER_h
#define _CONFIGSERVER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include <AsyncEventSource.h>
#if defined(ARDUINO_ARCH_ESP8266)
//#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <ESP8266WiFiType.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFiUdp.h>
#include <WiFiType.h>
#include <WiFiSTA.h>
#include <WiFiServer.h>
#include <WiFiScan.h>
#include <WiFiMulti.h>
#include <WiFiGeneric.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <WiFi.h>
#include <ETH.h>
#include <AsyncTCP.h>
#endif
#include <WebResponseImpl.h>
#include <WebHandlerImpl.h>
#include <WebAuthentication.h>
#include <StringArray.h>
#include <SPIFFSEditor.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>

#include <DNSServer.h>
#include <AsyncJson.h>

#include "FileAccess\FileAccess.h"
//#include "EspHomeBase.h"

#define MAX_REPLACERS 10

typedef int (*replaceHandler)(char *buf, const char *needle);
typedef struct replaceEntry {
	char needle[20] = "";
	replaceHandler handler = nullptr;
};

class ConfigServer {
public:
	~ConfigServer();
	void startConfig();
	void startWeb();
	void addUrlHandler(const char *url, WebRequestMethod method, ArRequestHandlerFunction f);
	static ConfigServer *getInstance();
	void handleClient();
	int replacer(char * buf, char * src);
	//static ESP8266WebServer *server;
	static AsyncWebServer *server;
	static AsyncWebSocket *socket;
	bool addReplaceHandler(const char needle[], replaceHandler handler);
private:
	static replaceEntry replaceList[];
	static int replaceCount;
	ConfigServer();
	static void handleConfig(AsyncWebServerRequest *req);
	static void handleSet(AsyncWebServerRequest *req);
	static void handleFile(AsyncWebServerRequest *req);
	static ConfigServer *_instanz;
	void feedTheDog();
};

#define MAX_PAGE_SIZE 4000
extern char pageBuf[];

#endif

