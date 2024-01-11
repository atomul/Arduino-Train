#include "Tunnel.h"

#include <Arduino.h>

#include "Utilities.h"

Tunnel::Tunnel(const TunnelSettings& tunnelSettings)
	: m_proximitySensorEntrance(NULL)
	, m_proximitySensorExit(NULL)
	, m_shouldMonitorTunnelEntrances(false)
	, m_tunnelObserver(NULL)
	, m_trainTunnelState(TRAIN_TUNNEL_STATE::TRAIN_OUTSIDE_TUNNEL)
{
	m_tunnelSettings = tunnelSettings;

	m_proximitySensorEntrance = new ProximitySensor(tunnelSettings.pin_proximitySensor_entrance, tunnelSettings.proximitySensorThresholdValue);
	m_proximitySensorEntrance->RegisterProximitySensorObserver(this);

	m_proximitySensorExit = new ProximitySensor(tunnelSettings.pin_proximitySensor_exit, tunnelSettings.proximitySensorThresholdValue);
	m_proximitySensorExit->RegisterProximitySensorObserver(this);

	m_photoresistorSensorEntrance = new PhotoresistorSensor(tunnelSettings.photoresistorSensorSettings_entrance);
	m_photoresistorSensorExit = new PhotoresistorSensor(tunnelSettings.photoresistorSensorSettings_exit);

	m_photoresistorSensorEntrance->RegisterPhotoresistorSensorObserver(this);
	m_photoresistorSensorExit->RegisterPhotoresistorSensorObserver(this);

	pinMode(tunnelSettings.pin_mosfet_gate_lights, OUTPUT);
}

bool Tunnel::TunnelSettings::AreSettingsValid()
{
	return true;
}

void Tunnel::Update()
{
	if (m_shouldMonitorTunnelEntrances)
	{
		switch (m_tunnelSettings.trainDetectionMode)
		{
			case TRAIN_DETECTION_MODE::PROXIMITY_SENSORS:
			{
				m_proximitySensorEntrance->Update();
				m_proximitySensorExit->Update();
				break;
			}
			case TRAIN_DETECTION_MODE::PHOTORESISTOR_SENSORS:
			{
				m_photoresistorSensorEntrance->Update();
				m_photoresistorSensorExit->Update();
				break;
			}
		}
	}
}

void Tunnel::ToggleSettings_TrainDetectionMode()
{
	if (m_tunnelSettings.trainDetectionMode == TRAIN_DETECTION_MODE::PROXIMITY_SENSORS)
	{
		ChangeSettings_TrainDetectionMode(TRAIN_DETECTION_MODE::PHOTORESISTOR_SENSORS);
	}
	else if (m_tunnelSettings.trainDetectionMode == TRAIN_DETECTION_MODE::PHOTORESISTOR_SENSORS)
	{
		ChangeSettings_TrainDetectionMode(TRAIN_DETECTION_MODE::PROXIMITY_SENSORS);
	}
}

void Tunnel::ChangeSettings_TrainDetectionMode(TRAIN_DETECTION_MODE trainDetectionMode)
{
	if (m_tunnelSettings.trainDetectionMode != trainDetectionMode)
	{
		m_tunnelSettings.trainDetectionMode = trainDetectionMode;
		if (m_tunnelSettings.trainDetectionMode == TRAIN_DETECTION_MODE::PROXIMITY_SENSORS)
		{
			Serial.println("Train detection mode changed to PROXIMITY sensors");
		}
		else if (m_tunnelSettings.trainDetectionMode == TRAIN_DETECTION_MODE::PHOTORESISTOR_SENSORS)
		{
			Serial.println("Train detection mode changed to PHOTORESISTOR sensors");
		}

		ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_OUTSIDE_TUNNEL);
	}
	else
	{
		Serial.println("Train detection mode NOT changed. Already in this mode.");
	}
}

void Tunnel::ChangeSettings_TunnelLightMode(TUNNEL_LIGHT_MODE lightMode)
{
	m_tunnelSettings.lightMode = lightMode;
}

void Tunnel::ChangeSettings_PhotoresistorEntrance_LightDifferenceThreshold(unsigned short int lightDifferenceThreshold)
{
	m_photoresistorSensorEntrance->ChangeSettings_LightDifferenceThreshold(lightDifferenceThreshold);
}

void Tunnel::ChangeSettings_PhotoresistorExit_LightDifferenceThreshold(unsigned short int lightDifferenceThreshold)
{
	m_photoresistorSensorExit->ChangeSettings_LightDifferenceThreshold(lightDifferenceThreshold);
}

void Tunnel::ChangeTunnelStateState(TRAIN_TUNNEL_STATE tunnelState)
{
	m_trainTunnelState = tunnelState;
	Serial.print("Tunnel state changed: \"");

	switch (tunnelState)
	{
	case TRAIN_TUNNEL_STATE::TRAIN_OUTSIDE_TUNNEL:
	{
		Serial.print("TRAIN_OUTSIDE_TUNNEL");
		break;
	}
	case TRAIN_TUNNEL_STATE::TRAIN_APROACHING_TUNNEL_ENTRANCE:
	{
		Serial.print("TRAIN_APROACHING_TUNNEL_ENTRANCE");
		
		if (m_tunnelSettings.lightMode == TUNNEL_LIGHT_MODE::AUTOMATIC_TRAIN_DETECTION)
		{
			TurnOnLights();
		}

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
		if (m_tunnelSettings.lightMode == TUNNEL_LIGHT_MODE::AUTOMATIC_TRAIN_DETECTION)
		{
			TurnOffLights();
		}
		break;
	}
	}

	TunnelEventInfo eventInfo(TUNNEL_EVENT_TYPE::TRAIN_TUNNEL_STATE_EVENT);
	eventInfo.state = m_trainTunnelState;
	SendEvent(eventInfo);

	Serial.print("\"");
	Serial.println("");
}

void Tunnel::OnProximityChanged(bool hasObstacle, uint8_t pin)
{
	if (m_tunnelSettings.trainDetectionMode != TRAIN_DETECTION_MODE::PROXIMITY_SENSORS)
	{
		return;
	}

	switch (m_trainTunnelState)
	{
		case TRAIN_TUNNEL_STATE::TRAIN_OUTSIDE_TUNNEL:
		{
			if (pin == m_proximitySensorEntrance->GetPin())
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
			if (m_tunnelSettings.lightMode == TUNNEL_LIGHT_MODE::AUTOMATIC_TRAIN_DETECTION)
			{
				TurnOnLights();
			}
			ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_IN_TUNNEL);
			break;
		}
		case TRAIN_TUNNEL_STATE::TRAIN_IN_TUNNEL:
		{
			if (pin == m_proximitySensorExit->GetPin())
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
			if (pin == m_proximitySensorExit->GetPin())
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
			ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_OUTSIDE_TUNNEL);

			break;
		}
	}
}

void Tunnel::OnLuminosityChanged(const PhotoresistorSensor::IPhotoresistorSensorObserver::PhotoresistorSensorEventInfo& eventInfo)
{
	if (m_tunnelSettings.trainDetectionMode != TRAIN_DETECTION_MODE::PHOTORESISTOR_SENSORS)
	{
		return;
	}

	switch (m_trainTunnelState)
	{
		case TRAIN_TUNNEL_STATE::TRAIN_OUTSIDE_TUNNEL:
		{
			if (eventInfo.pin == m_proximitySensorEntrance->GetPin())
			{
				if (eventInfo.lightChangeType == PhotoresistorSensor::PHOTORESISTOR_LIGHT_CHANGE::DARKENED)
				{
					ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_APROACHING_TUNNEL_ENTRANCE);
				}
			}

			break;
		}
		case TRAIN_TUNNEL_STATE::TRAIN_APROACHING_TUNNEL_ENTRANCE:
		{
			ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_IN_TUNNEL);
			break;
		}
		case TRAIN_TUNNEL_STATE::TRAIN_IN_TUNNEL:
		{
			if (eventInfo.pin == m_proximitySensorExit->GetPin())
			{
				if (eventInfo.lightChangeType == PhotoresistorSensor::PHOTORESISTOR_LIGHT_CHANGE::DARKENED)
				{
					ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_EXITING_TUNNEL);
				}
			}

			break;
		}
		case TRAIN_TUNNEL_STATE::TRAIN_EXITING_TUNNEL:
		{
			if (eventInfo.pin == m_proximitySensorExit->GetPin())
			{
				if (eventInfo.lightChangeType == PhotoresistorSensor::PHOTORESISTOR_LIGHT_CHANGE::BRIGHTENED)
				{
					ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_EXITED_TUNNEL);
				}
			}

			break;
		}
		case TRAIN_TUNNEL_STATE::TRAIN_EXITED_TUNNEL:
		{
			ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_OUTSIDE_TUNNEL);

			break;
		}
	}
}

void Tunnel::TurnOnLights()
{
	digitalWrite(m_tunnelSettings.pin_mosfet_gate_lights, HIGH);
	Serial.println("Lights ON\"");
}

void Tunnel::TurnOffLights()
{
	digitalWrite(m_tunnelSettings.pin_mosfet_gate_lights, LOW);
	Serial.println("Lights OFF\"");
}

void Tunnel::RegisterTunnelObserver(ITunnelObserver* tunnelObserver)
{
	m_tunnelObserver = tunnelObserver;
}


void Tunnel::SendEvent(const TunnelEventInfo& eventInfo)
{
	if (m_tunnelObserver)
	{
		m_tunnelObserver->OnTunnelEvent(eventInfo);
	}
}
