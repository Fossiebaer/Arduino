/*
 Name:		HTTPSensor.ino
 Created:	27.05.2019 07:19:41
 Author:	olli
*/

// the setup function runs once when you press reset or power the board
#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include <EspHomeBase.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>


EspHomeBase *_server;
char tempBuf[TEMP_SIZE]; // Buffer for Filecontents, TEMP_SIZE is defined in FileAccess.h
Adafruit_BME280 sensor;

// the variables for the ambient data
float temp, hum, press; 

StaticJsonBuffer<100> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();
int numConn = 0;

bool readingSensor = false;

void sendUpdate()
{
    char tmp[100];
    root.printTo(tmp);
    Serial.println(tmp);
    _server->sendPublicHttpUpdate(tmp);
}

void readAmbientValues()
{
    readingSensor = true;
    char tmp[10];
    char consoleMsg[40];
    Serial.println("Start reading...");
    //sprintf(tmp, "%.0f", sensor.readTemperature());
    //sprintf(consoleMsg, "Temperatur read: %s °C", tmp);
    //Serial.println(consoleMsg);
    root.set("temp", sensor.readTemperature());
    delay(1);
    //sprintf(tmp, "%.0f", (sensor.readPressure() / 100.0F));
    //sprintf(consoleMsg, "Pressure read: %s hPa", tmp);
    //Serial.println(consoleMsg);
    root.set("press", (sensor.readPressure() / 100.0f));
    //sprintf(tmp, "%.0f", sensor.readHumidity());
    //sprintf(consoleMsg, "Humidity read: %s \%rel", tmp);
    //Serial.println(consoleMsg);
    root.set("hum", sensor.readHumidity());
    delay(1);
    sendUpdate();
    readingSensor = false;
}

void wsEventHandler(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        if (!readingSensor)
        {
             char tmp[100];
             root.printTo(tmp);
             client->printf(tmp);
             numConn++;
             // free(tmp);
        }
    }
    else if (type == WS_EVT_DISCONNECT)
    {
         numConn--;
    }
}

void setup() 
{
    Serial.begin(115200);
    Serial.println("");
    // get instance of EspHomeBase
    _server = EspHomeBase::getInstance();
    // wait till EspHomeBase is ready and set up
    while (!EspHomeBase::ready) {
        delay(5);
    }
    // set connection mode of EspHomeBase.
    _server->changeMode(MODE_HTTP);

    _server->registerSocketEventHandler(&wsEventHandler);
    //Wire.begin(2, 1);
	unsigned status = false;
    while (!status)
    {
        delay(10);
        Serial.println("No sensor, rescan...");
	    status = sensor.begin();
    }
    Serial.println("Sensor found, going on...");
	char tmp[10];
	sprintf(tmp, "%.0f", sensor.readTemperature());
	root["temp"] = tmp;
	sprintf(tmp, "%.0f", (sensor.readPressure() / 100.0F));
	root["press"] = tmp;
	sprintf(tmp, "%.0f", sensor.readHumidity());
	root["hum"] = tmp;
}

// the loop function runs over and over again until power down or reset
void loop() 
{
	_server->process();
	readAmbientValues();
    delay(30000);
}
