// 
// Class ConfigServer
// Starts and maintains a webserver, hosting primarily a configuration website.
// Though, via the not found handler, other files can be served. 
// 
#include <DNSServer.h>
#include "ConfigServer.h"

ConfigServer *ConfigServer::_instanz = 0;
ESP8266WebServer *ConfigServer::server = 0;
char pageBuf[MAX_PAGE_SIZE];
char temp[800];
replaceEntry ConfigServer::replaceList[10];
int ConfigServer::replaceCount = 0;
DNSServer dnsServer;

ConfigServer::ConfigServer()
{
	static ESP8266WebServer temp(80);
	server = &temp;
}

ConfigServer::~ConfigServer()
{
}

void ConfigServer::start()
{
	if (server != 0) {
		server->on("/", HTTP_GET, handleConfig);
		server->on("/set", HTTP_POST, handleSet);
		server->onNotFound(handleFile);
		ESP.wdtFeed();
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
		server->handleClient();
		dnsServer.processNextRequest();
		ESP.wdtFeed();
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

void ConfigServer::handleConfig()
{
	if (fileExists("/config.html")) {
		int sz = readFile("/config.html", tempBuf, TEMP_SIZE);
		if (sz > 0) {
			int rep = ConfigServer::getInstance()->replacer(pageBuf, tempBuf);
			if (rep > 0) {
				server->send(200, "text/html", pageBuf);
			}
			else {
				server->send(500, "text/html", "File parsing failed!");
			}
		}
		else {
			server->send(500, "text/html", "File reading failed!");
		}
	}
	else {
		server->send(404, "text/html", "File not found!");
	}
}

void ConfigServer::handleSet()
{
	int num = ConfigServer::server->args();
	for (int i = 0; i < num; i++) {
		if (strcmp(ConfigServer::server->argName(i).c_str(), "submit") != 0) {
			setConfigParam(ConfigServer::server->argName(i).c_str(), ConfigServer::server->arg(i).c_str());
		}
	}
	setConfigParam("set", "1");
	ConfigServer::server->send(200, "text/html", "Configuration saved successfully.");
}

void ConfigServer::handleFile()
{
	if (fileExists(ConfigServer::server->uri().c_str())) {
		if (fileSize(ConfigServer::server->uri().c_str()) <= MAX_PAGE_SIZE) {
			int sz = readFile(ConfigServer::server->uri().c_str(), pageBuf, MAX_PAGE_SIZE);
			if (sz > -1) {
				server->send(200, "text/html", pageBuf);
			}
			else {
				server->send(500, "text/html", "File could not be opened");
			}
		}
		else {
			/// TODO: Handle larger files in chunks, meanwhile return internal server error
			server->send(500, "text/html", "File is too large to be served!");
		}
	}
	else {
		server->send(404, "text/html", "File not found!");
	}
}

int ConfigServer::replacer(char * buf, char * src)
{
	strcpy(buf, src);
	for (int i = 0; i < replaceCount; i++) {
		char *placeholder = strstr(buf, ConfigServer::replaceList[i].needle);
		int nLen = strlen(ConfigServer::replaceList[i].needle);
		ESP.wdtFeed();
		if (placeholder) {
			int index = (int)(placeholder - buf);
			int add = ConfigServer::replaceList[i].handler(temp);
			strcpy(src, buf + index + nLen);
			buf[index] = '\0';
			strcat(buf, temp);
			ESP.wdtFeed();
			strcat(buf, src);
			ESP.wdtFeed();
		}
	}
	return strlen(buf);
}
