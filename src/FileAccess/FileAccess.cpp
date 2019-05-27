// 
// 
// 

#include "FileAccess.h"

//char tempBuf[1200]; // Instantiated in ino file

bool ready = false;

bool fileExists(const char * name)
{
	if (!ready) {
		fsReady();
	}
	if (ready) {
		return SPIFFS.exists(name);
	}
	return false;
}

int fileSize(const char * name)
{
	if (fileExists(name)) {
		File f = SPIFFS.open(name, "r");
		int result = f.size();
		f.close();
		return result;
	}
	return 0;
}

bool fsReady(void)
{
	if (!ready) {
		ready = SPIFFS.begin();
	}
	return ready;
}

int readFile(const char * name, char * buf, int max)
{
	if (fileExists(name)) {
		File f = SPIFFS.open(name, "r");
		if (!f) {
			Serial.println("Open file failed!");
			f.close();
			return -1;
		}
		int charsRead = f.read((uint8_t *)buf, max);
		f.close();
		if (charsRead <= 0) return -1;
		return charsRead;
	}
	return -1;
}

int readFileChunked(AsyncWebServerRequest *req)
{
	if (fileExists(req->url().c_str())) {
		AsyncWebServerResponse *response = req->beginResponse(SPIFFS, req->url().c_str());
		req->send(response);
		return 1;
	}
	return -1;
}

int writeFile(const char * name, const char * buf, int len)
{
	File f = SPIFFS.open(name, "w");
	if (!f) return -1;
	int written = f.write((uint8_t *)buf, len);
	f.close();
	return written;
}

const char * getConfigParam(const char * key)
{
	if (fileExists("/config.json")) {
		File cfg = SPIFFS.open("/config.json", "r");
		if (!cfg) {
			cfg.close();
			return nullptr;
		}
		int sz = cfg.size();
		cfg.read((uint8_t *)tempBuf, sz);
		cfg.close();
		StaticJsonBuffer<200> jB;
		JsonObject &obj = jB.parseObject(tempBuf);
		if (!obj.success()) {
			return nullptr;
		}
		return obj[key];
	}
	return nullptr;
}

bool setConfigParam(const char * key, const char * value)
{
	StaticJsonBuffer<200> jB;
	if (fileExists("/config.json")) {
		File in = SPIFFS.open("/config.json", "r");
		int sz = in.size();
		in.read((uint8_t *)tempBuf, sz);
		in.close();
		JsonObject &obj = jB.parseObject(tempBuf);
		obj[key] = value;
		File out = SPIFFS.open("/config.json", "w");
		obj.printTo(out);
		out.close();
		return true;
	}
	else {
		JsonObject &obj = jB.createObject();
		obj[key] = value;
		File out = SPIFFS.open("/config.json", "w");
		obj.printTo(out);
		out.close();
		return true;
	}
	return false;
}
