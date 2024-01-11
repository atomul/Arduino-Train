#include "Tank.h"

#include <Arduino.h>

const uint8_t k_pin_mosfet_gate = 9;

const uint8_t k_pin_photoresistor_entrance = A0;
const uint8_t k_pin_photoresistor_exit = A1;

const uint8_t k_pin_proximitySensor_entrance = 3;
const uint8_t k_pin_proximitySensor_exit = 4;

#define LOGIC_1 0
#define LOGIC_2 1

Tank::Tank()
{
	m_isEnabled = false;
	m_threshholdValue = 200;
	m_areLightsOverridden = false;
	m_startReadTime = millis();
	m_startPrintTime = millis();
	m_train_detection_mode = TRAIN_DETECTION_MODE::TRAIN_DETECTION_MODE_PHOTORESISTORS;

	const unsigned int proximitySensorThresholdValue = 300;

	ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_OUTSIDE_TUNNEL);
	m_lastEntranceValue = analogRead(k_pin_photoresistor_entrance);
	m_lastExitValue = analogRead(k_pin_photoresistor_exit);

	m_proximitySensoEntrance = new ProximitySensor(k_pin_proximitySensor_entrance, proximitySensorThresholdValue);
	m_proximitySensoEntrance->RegisterProximitySensorObserver(this);

	m_proximitySensoExit = new ProximitySensor(k_pin_proximitySensor_exit, proximitySensorThresholdValue);
	m_proximitySensoExit->RegisterProximitySensorObserver(this);

	SetupRemote();

	pinMode(k_pin_mosfet_gate, OUTPUT);
}

Tank::~Tank()
{
}

void Tank::SetupRemote()
{
	m_controlCenter = new ControlCenter();
	m_controlCenter->RegisterObserver(this);
}

void Tank::ChangeTunnelStateState(TRAIN_TUNNEL_STATE tunnel_state)
{
	m_train_tunnel_state = tunnel_state;
	Serial.print("Tunnel state changed: \"");
	
	switch (tunnel_state)
	{
		case TRAIN_TUNNEL_STATE::TRAIN_OUTSIDE_TUNNEL:
		{
			Serial.print("TRAIN_OUTSIDE_TUNNEL");
			break;
		}
		case TRAIN_TUNNEL_STATE::TRAIN_APROACHING_TUNNEL_ENTRANCE:
		{
			Serial.print("TRAIN_APROACHING_TUNNEL_ENTRANCE");
			break;
		}
		case TRAIN_TUNNEL_STATE::TRAIN_IN_TUNNEL:
		{
			Serial.print("TRAIN_IN_TUNNEL");
			break;
		}
		case TRAIN_TUNNEL_STATE::TRAIN_EXITING_TUNNEL:
		{
			Serial.print("TRAIN_EXITING_TUNNEL");
			break;
		}
		case TRAIN_TUNNEL_STATE::TRAIN_EXITED_TUNNEL:
		{
			Serial.print("TRAIN_EXITED_TUNNEL");
			break;
		}
	}
	Serial.print("\"");
	Serial.println("");
}

void Tank::Update()
{
	m_controlCenter->Update();
	m_proximitySensoEntrance->Update();
	m_proximitySensoExit->Update();

	int currentTime = millis();

	int entranceValue = analogRead(k_pin_photoresistor_entrance);
	int exitValue = analogRead(k_pin_photoresistor_exit);

	 /*
	 Serial.print("Last entrance value: \"");
	 Serial.print(m_lastEntranceValue);
	 Serial.print("\"");

	 Serial.print("Entrance value: \"");
	 Serial.print(entranceValue);
	 Serial.print("\"");

	 Serial.print("\"");

	 Serial.print("Last exit value: \"");
	 Serial.print(exitValue);
	 Serial.print("\"");	 
	 
	 Serial.print("Exit value: \"");
	 Serial.print(exitValue);
	 Serial.print("\"");
	 Serial.println("");

	 Serial.print("\"");
	 //*/

	 if (m_lastEntranceValue == -1)
	 {
		 m_lastEntranceValue = entranceValue;
	 }

	 if (m_lastExitValue == -1)
	 {
		 m_lastExitValue = exitValue;
	 }
	 
	 if (m_train_detection_mode == TRAIN_DETECTION_MODE::TRAIN_DETECTION_MODE_PHOTORESISTORS)
	 {
		 //if (entranceValue < entranceValue)
		 switch (m_train_tunnel_state)
		 {
			 case TRAIN_TUNNEL_STATE::TRAIN_OUTSIDE_TUNNEL:
			 {
				 // if outside check if train passes over (sensor doesn't detect light)
				 /*
				 Serial.print("Last entrance value: \"");
				 Serial.print(m_lastEntranceValue);
				 Serial.print("\"");

				 Serial.print("Threshold value: \"");
				 Serial.print(k_threshholdValue);
				 Serial.print("\"");

				 Serial.println("");
				 //*/
	#if LOGIC_1
				 if (m_lastEntranceValue < m_threshholdValue)
				 {
					 /*
					 Serial.print("Last entrance value: \"");
					 Serial.print(m_lastEntranceValue);
					 Serial.print("\"");

					 Serial.print("Entrance value: \"");
					 Serial.print(entranceValue);
					 Serial.print("\"");

					 Serial.println("");
					 //*/
					 if (entranceValue >= m_threshholdValue)
					 {
						 ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_APROACHING_TUNNEL_ENTRANCE);
					 }
				 }
	#elif LOGIC_2

				 if (entranceValue > m_lastEntranceValue)
				 {
					 if ((entranceValue - m_lastEntranceValue) >= m_threshholdValue)
					 {
						 ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_APROACHING_TUNNEL_ENTRANCE);
						 //m_lastEntranceValue = entranceValue;
						 m_lastExitValue = exitValue;
					 }
				 }

				 // update values from time to time to keep up with current light conditions
				 if (m_train_tunnel_state == TRAIN_TUNNEL_STATE::TRAIN_OUTSIDE_TUNNEL)
				 {
					 if (currentTime - m_startReadTime >= 60000)
					 {
						 m_startReadTime = currentTime;
						 m_lastEntranceValue = entranceValue;
						 m_lastExitValue = exitValue;
					 }
				 }
	#endif

				 break;
			 }
			 case TRAIN_TUNNEL_STATE::TRAIN_APROACHING_TUNNEL_ENTRANCE:
			 {
				 TryToggleLights(LIGHTS_MODE::LIGHTS_ON);
				 ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_IN_TUNNEL);
				 break;
			 }
			 case TRAIN_TUNNEL_STATE::TRAIN_IN_TUNNEL:
			 {
	#if LOGIC_1
				 //if (m_lastExitValue < k_threshholdValue)
				 {
					 if (exitValue > m_threshholdValue)
					 {
						 ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_EXITING_TUNNEL);
					 }
				 }
	#elif LOGIC_2
				 if (exitValue > m_lastExitValue)
				 {
					 if ((exitValue - m_lastExitValue) >= m_threshholdValue)
					 {
						 ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_EXITING_TUNNEL);
						 m_lastExitValue = exitValue + (0.1 * exitValue);
					 }
				 }
	#endif
				 break;
			 }
			 case TRAIN_TUNNEL_STATE::TRAIN_EXITING_TUNNEL:
			 {
	#if LOGIC_1
				 //if (m_lastExitValue >= k_threshholdValue)
				 {
					 if (exitValue < m_threshholdValue)
					 {
						 ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_EXITED_TUNNEL);
					 }
				 }
	#elif LOGIC_2
				 if (exitValue < m_lastExitValue)
				 {
					 if ((m_lastExitValue - exitValue) >= m_threshholdValue)
					 {
						 ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_EXITED_TUNNEL);
					 }
				 }
	#endif
				 break;
			 }
			 case TRAIN_TUNNEL_STATE::TRAIN_EXITED_TUNNEL:
			 {
				 TryToggleLights(LIGHTS_MODE::LIGHTS_OFF);
				 ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_OUTSIDE_TUNNEL);

				 m_lastEntranceValue = entranceValue;
				 m_lastExitValue = exitValue;

				 break;
			 }
		 }
	 }
	 
	 else if (m_train_detection_mode == TRAIN_DETECTION_MODE::TRAIN_DETECTION_MODE_PROXIMITY_SENSORS)
	 {
		 switch (m_train_tunnel_state)
		 {
			 case TRAIN_TUNNEL_STATE::TRAIN_OUTSIDE_TUNNEL:
			 {
				 break;
			 }
			 case TRAIN_TUNNEL_STATE::TRAIN_APROACHING_TUNNEL_ENTRANCE:
			 {
				 TryToggleLights(LIGHTS_MODE::LIGHTS_ON);
				 ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_IN_TUNNEL);
				 break;
			 }
			 case TRAIN_TUNNEL_STATE::TRAIN_IN_TUNNEL:
			 {
				 break;
			 }
			 case TRAIN_TUNNEL_STATE::TRAIN_EXITING_TUNNEL:
			 {
				 break;
			 }
			 case TRAIN_TUNNEL_STATE::TRAIN_EXITED_TUNNEL:
			 {
				 TryToggleLights(LIGHTS_MODE::LIGHTS_OFF);
				 ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_OUTSIDE_TUNNEL);

				 break;
			 }
		 }
	 }

	 if (currentTime - m_startPrintTime >= 60000)
	{
		m_startPrintTime = currentTime;
		/*
		Serial.print("Last entrance value: \"");
		Serial.print(m_lastEntranceValue);
		Serial.print("\"");

		Serial.print("   ");

		Serial.print("Entrance value: \"");
		Serial.print(entranceValue);
		Serial.print("\"");

		Serial.print("   ");

		Serial.print("Last exit value: \"");
		Serial.print(exitValue);
		Serial.print("\"");

		Serial.print("   ");

		Serial.print("Exit value: \"");
		Serial.print(exitValue);
		Serial.print("\"");

		Serial.print("   ");

		Serial.print("Threshold value: \"");
		Serial.print(m_threshholdValue);
		Serial.print("\"");

		Serial.print("   ");

		Serial.print("ent - last ent: \"");
		Serial.print(entranceValue > m_lastEntranceValue);
		Serial.print("\"");

		Serial.print("   ");

		Serial.print("exit - last exit: \"");
		Serial.print(exitValue - m_lastExitValue);
		Serial.print("\"");

		Serial.print("   ");

		Serial.print("last exit - exit: \"");
		Serial.print(m_lastExitValue - exitValue);
		Serial.print("\"");

		Serial.println("");
		//*/
	}

}

void Tank::OnLightsOverrideChanged(bool areLightsOverridden)
{
	m_areLightsOverridden = areLightsOverridden;
	Serial.println((m_areLightsOverridden == true) ? "Lights manual override ON." : "Lights manual override OFF.");
}

void Tank::OnChangeTrainDetectionMode()
{
	if (m_train_detection_mode == TRAIN_DETECTION_MODE::TRAIN_DETECTION_MODE_PHOTORESISTORS)
	{
		m_train_detection_mode = TRAIN_DETECTION_MODE::TRAIN_DETECTION_MODE_PROXIMITY_SENSORS;
		Serial.println("Train detection mode changed to PROXIMITY sensors");
	}
	else if (m_train_detection_mode == TRAIN_DETECTION_MODE::TRAIN_DETECTION_MODE_PROXIMITY_SENSORS)
	{
		m_train_detection_mode = TRAIN_DETECTION_MODE::TRAIN_DETECTION_MODE_PHOTORESISTORS;
		Serial.println("Train detection mode changed to PHOTORESISTOR sensors");
	}		
}

void Tank::OnSwitchLightsOn()
{
	ToggleLights(LIGHTS_MODE::LIGHTS_ON);
}

void Tank::OnSwitchLightsOff()
{
	ToggleLights(LIGHTS_MODE::LIGHTS_OFF);
}

void Tank::TryToggleLights(LIGHTS_MODE mode)
{
	if (!m_areLightsOverridden)
	{
		ToggleLights(mode);
	}
}

void Tank::ToggleLights(LIGHTS_MODE mode)
{
	(mode == LIGHTS_MODE::LIGHTS_ON) ? digitalWrite(k_pin_mosfet_gate, HIGH) : digitalWrite(k_pin_mosfet_gate, LOW);
	Serial.print("Lights: \"");
	Serial.print((mode == LIGHTS_MODE::LIGHTS_ON) ? "ON" : "OFF");
	Serial.print("\"");
	Serial.println("");
}

void Tank::OnLightSensitivityChanged(unsigned short int lightDifferenceThreshold)
{
	m_threshholdValue = lightDifferenceThreshold;
	/*
	Serial.print("Threshold: \"");
	Serial.print(m_threshholdValue);
	Serial.print("\"");
	Serial.println("");
	//*/
}

//////////

void Tank::OnProximityChanged(bool hasObstacle, uint8_t pin)
{
	if (m_train_detection_mode != TRAIN_DETECTION_MODE::TRAIN_DETECTION_MODE_PROXIMITY_SENSORS)
	{
		return;
	}

	switch (m_train_tunnel_state)
	{
		case TRAIN_TUNNEL_STATE::TRAIN_OUTSIDE_TUNNEL:
		{
			if (pin == k_pin_proximitySensor_entrance)
			{
				if (hasObstacle)
				{
					ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_APROACHING_TUNNEL_ENTRANCE);
				}
			}
		
			break;
		}
		case TRAIN_TUNNEL_STATE::TRAIN_APROACHING_TUNNEL_ENTRANCE:
		{
			TryToggleLights(LIGHTS_MODE::LIGHTS_ON);
			ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_IN_TUNNEL);
			break;
		}
		case TRAIN_TUNNEL_STATE::TRAIN_IN_TUNNEL:
		{
			if (pin == k_pin_proximitySensor_exit)
			{
				if (hasObstacle)
				{
					ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_EXITING_TUNNEL);
				}
			}

			break;
		}
		case TRAIN_TUNNEL_STATE::TRAIN_EXITING_TUNNEL:
		{
			if (pin == k_pin_proximitySensor_exit)
			{
				if (!hasObstacle)
				{
					ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_EXITED_TUNNEL);
				}
			}

			break;
		}
		case TRAIN_TUNNEL_STATE::TRAIN_EXITED_TUNNEL:
		{
			TryToggleLights(LIGHTS_MODE::LIGHTS_OFF);
			ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_OUTSIDE_TUNNEL);

			break;
		}
	}
}