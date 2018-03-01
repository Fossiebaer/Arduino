/*
 Name:		WiFiRemote32.ino
 Created:	28.02.2018 18:06:12
 Author:	olli
*/
#include <EspHomeBase.h>
// the setup function runs once when you press reset or power the board
ConfigServer *s = nullptr;

void setup() {
	s = ConfigServer::getInstance();
}

// the loop function runs over and over again until power down or reset
void loop() {
  
}
