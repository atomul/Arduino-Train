//#include "src/Train.h"
//#include "src/Logger.h"

#include "src/SystemController.h"
#include "src/macro-logger/MacroLogger.h"

//Train* k_train;

SystemController g_systemController;

// the setup function runs once when you press reset or power the board
void setup() {

	//Logger::instance().begin(Serial, 250, 64, Logger::DropOldest);

	Serial.begin(9600);
	Logger::set_level(Logger::Level::TRACE);
	

	////////////////////////////////////////////////////////////////////////////////

	//delay(2000);

	//Logger::instance().log(F("--------------------"));
	//Logger::instance().log(F("Starting program..."));
	//Logger::instance().log(F("--------------------"));

	///*
	//Serial.println(F(""));
	//Serial.println(F(""));
	//Serial.println(F("--------------------"));
	//Serial.println(F("Starting program."));
	//Serial.println(F("--------------------"));	
	//Serial.flush();
	////*/

	////k_tank = new Tank();
	//k_train = new Train();
	////Serial.flush();

	////Serial.println(F("new Train done."));
	//Logger::instance().log(F("new Train done."));

	Serial.println(F(""));

	LOG_INFO("--------------------");
	LOG_INFO("Starting program....");
	LOG_INFO("--------------------");
	LOG_INFO("");

	g_systemController.Setup();

	LOG_INFO("--------------------");
	LOG_INFO("Started program.");
	LOG_INFO("--------------------");
	LOG_INFO("");
}

// the loop function runs over and over again forever
void loop() {
	//Logger::instance().update();

	//k_train->Update();

	g_systemController.Update();
}