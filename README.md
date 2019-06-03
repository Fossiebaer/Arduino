# EspHomeBase
Library for SmartHome Devices based on espressifs ESP8266 and ESP32. The communication to a SmartHome server is established 
via the MQTT-protocol. Alternatively, devices with a web-interface ca be implemented.

The goal of this library is to provide the necessary basic functionality for creating smart home sensors and actors. 
To reach this goal, the library uses some libraries, created by other authors and ties them up to build a basic framework.

# Prerequisites
EspHomeBase relies on the following libraries:
 - PubSubClient
 - ESPAsyncWebServer
The latter requires some libraries itself:
 - ArduinoJson (version 5, ESPAsyncWebServer in the current version is not compatible with ArduinoJson version 6)
 - ESPAsyncTcp for ESP8266 or
 - AsyncTcp for ESP32
 
# Usage an Documentation
The library contains some examples for demonstrating the usage of the library. Detailed information as well as the documentation 
can be found in the wiki.
