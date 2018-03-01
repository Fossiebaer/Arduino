// FileAccess.h

#ifndef _FILEACCESS_h
#define _FILEACCESS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include <FS.h>
#if defined(ARDUINO_ARCH_ESP32)
#include <SPIFFS.h>
#include <vfs_api.h>
#include <FSImpl.h>
#endif
#include <ArduinoJson.h>
#define TEMP_SIZE 2000
extern char tempBuf[];
bool fileExists(const char *name);
int fileSize(const char *name);
bool fsReady(void);
int readFile(const char *name, char *buf, int max);
int writeFile(const char *name, const char*buf, int len);
const char *getConfigParam(const char *key);
bool setConfigParam(const char *key, const char *value);
#endif

