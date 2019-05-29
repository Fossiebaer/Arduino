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

StaticJsonBuffer<200> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();
int numConn = 0;

bool readingSensor = false;

void sendUpdate()
{
    char tmp[100];
    root.printTo(tmp);
    _server->sendPublicHttpUpdate(tmp);
    free(tmp);
}

void readAmbientValues()
{
    readingSensor = true;
    char tmp[10];
    sprintf(tmp, "%.0f", sensor.readTemperature());
    root["temp"] = tmp;
    sprintf(tmp, "%.0f", (sensor.readPressure() / 100.0F));
    root["press"] = tmp;
    sprintf(tmp, "%.0f", sensor.readHumidity());
    root["hum"] = tmp;
    if(numConn > 0)
    {
        sendUpdate();
    }
    free(tmp);
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
             free(tmp);
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

	unsigned status;
    	
	status = sensor.begin();
	if (!status) 
	{
		Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
		while (1);
	}

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
