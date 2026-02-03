//#include "src/Train.h"
//#include "src/Logger.h"

#include "src/SystemController.h"
#include "src/macro-logger/MacroLogger.h"

#include "src/HBridge.h"

//Train* k_train;

SystemController g_systemController;
HBridgeMotorController hbridge;

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

	constexpr uint8_t PIN_PWS = 11;
	constexpr uint8_t PIN_DIRECTION_1= A1;
	constexpr uint8_t PIN_DIRECTION_2 = A2;

	Serial.println(F(""));

	LOG_INFO("--------------------");
	LOG_INFO("Starting program....");
	LOG_INFO("--------------------");
	LOG_INFO("");

	//g_systemController.Setup();

	HBridge_L293D::HBridge_L293D_Channel_Settings settings;
	settings.enable = PIN_PWS;
	settings.input1 = PIN_DIRECTION_1;
	settings.input2 = PIN_DIRECTION_2;

	hbridge.AssignHBridgeChannel(settings);
	hbridge.SetDirection(HBridgeMotorController::MOTOR_DIRECTION_BACKWARD);
	hbridge.Enable();
	
	hbridge.SetSpeed(255);
	hbridge.SetChangeSpeedAutomatically(true);
	hbridge.SetTargetSpeedPercentage(0, 10000);

	LOG_INFO("--------------------");
	LOG_INFO("Started program.");
	LOG_INFO("--------------------");
	LOG_INFO("");
}

// the loop function runs over and over again forever
void loop() {

	//g_systemController.Update();

	hbridge.Update();

	return;

	digitalWrite(11, HIGH);
	digitalWrite(12, HIGH);

	delay(1000);

	digitalWrite(11, LOW);
	digitalWrite(12, LOW);

	delay(1000);
}