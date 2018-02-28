/*
 Name:		WiFiRemote.ino
 Created:	27.02.2018 17:27:12
 Author:	olli
*/
#include <EspHomeBase.h>
ConfigServer *s = nullptr;
// the setup function runs once when you press reset or power the board

void setup() {
	s = ConfigServer::getInstance();
}

// the loop function runs over and over again until power down or reset
void loop() {
  
}
