#include "Train.h"

#include <Arduino.h>
#include <Servo	>
#include <stdint.h>

Train::Train()
{
	SetupControlCenter();
	SetupTunnel();

	lastLightDifferenceThresholdPrinted = 0;
}

void Train::SetupControlCenter()
{
	m_controlCenter = new ControlCenter();
	m_controlCenter->RegisterObserver(this);
}

void Train::SetupTunnel()
{
	Tunnel::TunnelSettings tunnelSettings;

	tunnelSettings.trainDetectionMode = Tunnel::TRAIN_DETECTION_MODE::PROXIMITY_SENSORS;
	tunnelSettings.lightMode = Tunnel::TUNNEL_LIGHT_MODE::AUTOMATIC_TRAIN_DETECTION;
	
	tunnelSettings.pin_mosfet_gate_lights = 9;
	
	tunnelSettings.pin_proximitySensor_entrance = 3;
	tunnelSettings.pin_proximitySensor_exit = 4;
	tunnelSettings.proximitySensorThresholdValue = 300;

	PhotoresistorSensor::PhotoresistorSensorSettings photoresistorSettings;
	photoresistorSettings.pin = A0;
	photoresistorSettings.eventMode = PhotoresistorSensor::PHOTORESISTOR_EVENT_MODE::AND_MODE;
	photoresistorSettings.lightDifferenceThreshold = 300;
	photoresistorSettings.readTimeInterval = 100;
	tunnelSettings.photoresistorSensorSettings_entrance = photoresistorSettings;
	
	photoresistorSettings.pin = A1;
	tunnelSettings.photoresistorSensorSettings_exit = photoresistorSettings;

	m_tunnel = new Tunnel(tunnelSettings);
	m_tunnel->RegisterTunnelObserver(this);
	m_tunnel->StartStopMonitoringTunnelEntrances(false);
}

void Train::Update()
{
	m_tunnel->Update();
	m_controlCenter->Update();
}

void Train::OnSwitchLightsOn()
{
	m_tunnel->TurnOnLights();
}

void Train::OnSwitchLightsOff()
{
	m_tunnel->TurnOffLights();
}

void Train::OnSwitchTunnelDetectionOn()
{
	m_tunnel->StartStopMonitoringTunnelEntrances(true);
}

void Train::OnSwitchTunnelDetectionOff()
{
	m_tunnel->StartStopMonitoringTunnelEntrances(false);
}

void Train::OnLightsOverrideChanged(bool areLightsOverridden)
{
	if (areLightsOverridden)
	{
		m_tunnel->ChangeSettings_TunnelLightMode(Tunnel::TUNNEL_LIGHT_MODE::MANUAL);
	}
	else
	{
		m_tunnel->ChangeSettings_TunnelLightMode(Tunnel::TUNNEL_LIGHT_MODE::AUTOMATIC_TRAIN_DETECTION);
	}
}

void Train::OnLightSensitivityChanged(unsigned short int lightDifferenceThreshold)
{
	unsigned short int difference = 0;
	if (lastLightDifferenceThresholdPrinted > lightDifferenceThreshold) difference = lastLightDifferenceThresholdPrinted - lightDifferenceThreshold;
	else difference = lightDifferenceThreshold - lastLightDifferenceThresholdPrinted;
	if (difference >= 50)
	{
		String str = "Tunnel settings - light sensitivity:\"" + String(lightDifferenceThreshold) + String("\"");
		Serial.println(str);
		lastLightDifferenceThresholdPrinted = lightDifferenceThreshold;
	}

	m_tunnel->ChangeSettings_PhotoresistorEntrance_LightDifferenceThreshold(lightDifferenceThreshold);
	m_tunnel->ChangeSettings_PhotoresistorExit_LightDifferenceThreshold(lightDifferenceThreshold);
}

void Train::OnChangeTrainDetectionMode()
{
	m_tunnel->ToggleSettings_TrainDetectionMode();
}

void Train::OnTunnelEvent(const Tunnel::TunnelEventInfo& eventInfo)
{
	// do nothing
}

Train::~Train()
{


}