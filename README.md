# EspHomeBase
Library for SmartHome Devices based on espressifs ESP8266 and ESP32. The communication to a SmartHome server is established 
via the MQTT-protocol. Alternatively, devices with a web-interface ca be implemented.

The goal of this library is to provide the necessary basic functionality for creating smart home sensors and actors. 
To reach this goal, the library uses some libraries, created by other authors and ties them up to build a basic framework.

# Prerequisites
EspHomeBase relies on the following libraries:
 - PubSubClient
 - ArduinoJson
 - ESPAsyncWebServer
The latter requires some libraries itself:
 - ESPAsyncTcp for ESP8266 or
 - AsyncTcp for ESP32

# Features
Json-file based configuration:
the configuration for devices based on this library is stored in a Json-file located on SPIFFS filesystem on the controller. You can either upload a preconfigured file to the device or configure the device via configuration webpage.

Web based configuration:
if the device has an empty configuration file or connection the the configured wireless LAN fails too often, the library starts an wireless access point, a webserver and delivers the configuration webpage. The webpage is user-defined. So it can have all required parameters.


 
# Usage and Documentation
The library contains some examples for demonstrating the usage of the library. Detailed information about usage as well as the documentation can be found in the wiki.
