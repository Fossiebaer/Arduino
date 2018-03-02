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
char temp[1200];
replaceEntry ConfigServer::replaceList[10];
int ConfigServer::replaceCount = 0;
DNSServer dnsServer;

ConfigServer::ConfigServer()
{
	static AsyncWebServer server(80);
	//server = &temp;
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
	}
	else {
		Serial.println("No Server Instance.");
	}
	IPAddress myIP = WiFi.softAPIP();
	dnsServer.start(53, "*", myIP);
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
		strncpy(entry.needle, needle, 9);
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
	if (fileExists(req->url().c_str())) {
		if (fileSize(req->url().c_str()) <= MAX_PAGE_SIZE) {
			int sz = readFile(req->url().c_str(), pageBuf, MAX_PAGE_SIZE);
			if (sz > -1) {
				
				//server->send(200, "text/html", pageBuf);
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
			int add = ConfigServer::replaceList[i].handler(temp);
			strcpy(src, buf + index + nLen);
			buf[index] = '\0';
			strcat(buf, temp);
			feedTheDog();
			strcat(buf, src);
			feedTheDog();
		}
	}
	return strlen(buf);
}

void ConfigServer::feedTheDog()
{
#if defined(ARDUINO_ARCH_ESP8266)
	ESP.wdtFeed();
#endif
}
