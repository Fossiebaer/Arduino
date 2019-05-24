// 
// Class ConfigServer
// Starts and maintains a webserver, hosting primarily a configuration website.
// Though, via the not found handler, other files can be served. 
// 
#include <DNSServer.h>
#include "ConfigServer.h"

ConfigServer *ConfigServer::_instanz = 0;
AsyncWebServer *ConfigServer::server = 0;
char pageBuf[MAX_PAGE_SIZE];
replaceEntry ConfigServer::replaceList[10];
int ConfigServer::replaceCount = 0;
DNSServer dnsServer;
char repBuf[500];

ConfigServer::ConfigServer()
{
	static AsyncWebServer tempserver(80);
	server = &tempserver;
}

ConfigServer::~ConfigServer()
{
}

void ConfigServer::startConfig()
{
	if (server != 0) {
		server->on("/", HTTP_GET, handleConfig);
		server->on("/set", HTTP_POST, handleSet);
		server->onNotFound(handleFile);
		feedTheDog();
		server->begin();
		Serial.println("Configserver started...");
	}
	else {
		Serial.println("No Server Instance.");
	}
	IPAddress myIP = WiFi.softAPIP();
	dnsServer.start(53, "*", myIP);
}

void ConfigServer::startWeb()
{
	if (server != 0) {
		server->onNotFound(handleFile);
		server->begin();
	}
}

void ConfigServer::addUrlHandler(const char *url, WebRequestMethod method, ArRequestHandlerFunction f)
{
	if (server != 0) {
		server->on(url, method, f);
		//server->on(url, "text/html", );
	}
}

ConfigServer *ConfigServer::getInstance()
{
	if (_instanz == 0) {
		_instanz = new ConfigServer();
	}
	return _instanz;
}

void ConfigServer::handleClient()
{
	if(server != 0){
		//server->handleClient();
		dnsServer.processNextRequest();
		
		feedTheDog();
	}
}

bool ConfigServer::addReplaceHandler(const char needle[], replaceHandler handler)
{
	if (replaceCount < MAX_REPLACERS) {
		replaceEntry entry;
		strncpy(entry.needle, needle, 19);
		entry.handler = handler;
		replaceList[replaceCount] = entry;
		replaceCount++;
		return true;
	} 
	return false;
}

void ConfigServer::handleConfig(AsyncWebServerRequest *req)
{
	if (fileExists("/config.html")) {
		int sz = readFile("/config.html", tempBuf, TEMP_SIZE);
		Serial.println(tempBuf);
		if (sz > 0) {
			int rep = ConfigServer::getInstance()->replacer(pageBuf, tempBuf);
			if (rep > 0) {
				req->send(200, "text/html", pageBuf);
			}
			else {
				req->send(500, "text/html", "File parsing failed!");
			}
		}
		else {
			req->send(500, "text/html", "File reading failed!");
		}
	}
	else {
		req->send(404, "text/html", "File not found!");
	}
}

void ConfigServer::handleSet(AsyncWebServerRequest *req)
{
	int num = req->args();
	for (int i = 0; i < num; i++) {
		if (strcmp(req->argName(i).c_str(), "submit") != 0) {
			setConfigParam(req->argName(i).c_str(), req->arg(i).c_str());
		}
	}
	setConfigParam("set", "1");
	req->send(200, "text/html", "Configuration saved successfully.");
}

void ConfigServer::handleFile(AsyncWebServerRequest *req)
{
	Serial.print("Handle file ");
	Serial.println(req->url().c_str());
	if (fileExists(req->url().c_str())) {
		Serial.println("File found");
		if (fileSize(req->url().c_str()) <= MAX_PAGE_SIZE) {
			int sz = readFile(req->url().c_str(), pageBuf, MAX_PAGE_SIZE);
			Serial.println("File opened.");
			if (sz > -1) {
				req->send(200, "text/html", pageBuf);
			}
			else {
				req->send(500, "text/html", "File could not be opened");
			}
		}
		else {
			/// TODO: Handle larger files in chunks, meanwhile return internal server error
			req->send(500, "text/html", "File is too large to be served!");
		}
	}
	else {
		req->send(404, "text/html", "File not found!");
	}
}


int ConfigServer::replacer(char * buf, char * src)
{
	strcpy(buf, src);
	for (int i = 0; i < replaceCount; i++) {
		char *placeholder = strstr(buf, ConfigServer::replaceList[i].needle);
		int nLen = strlen(ConfigServer::replaceList[i].needle);
		feedTheDog();
		if (placeholder) {
			int index = (int)(placeholder - buf);
			int add = ConfigServer::replaceList[i].handler(repBuf, ConfigServer::replaceList[i].needle);
			Serial.println("Replacing complete, merging");
			strcpy(src, buf + index + nLen);
			buf[index] = '\0';
			if (add > 0) {
				strcat(buf, repBuf);
			}
			feedTheDog();
			strcat(buf, src);
			Serial.println("Merging complete");
			feedTheDog();
		}
	}
	return strlen(buf);
}

void ConfigServer::feedTheDog()
{
#if defined(ARDUINO_ARCH_ESP8266)
	ESP.wdtFeed();
#elif defined(ARDUINO_ARCH_ESP32)
	delay(1);
#endif
}
