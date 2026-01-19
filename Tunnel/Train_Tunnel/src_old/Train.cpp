#include "Train.h"

#include <Arduino.h>
//#include <Servo	>
#include <stdint.h>

//Speed = map(sensorVal, 0, 1023, 0, 255);
// map from one range to another.

// H-Bridge / L293D with octocoupler
// https://forum.arduino.cc/t/l293d-control-via-isp847-opto-isolator-problem/71953
// https://www.youtube.com/watch?v=KXGSGzxefZc&ab_channel=element14presents
// https://www.youtube.com/watch?v=Vm0k1yyVurQ&ab_channel=LewisLoflin

Train::Train()
{
	delay(1000);
	Serial.println(F("Train::Train start"));
	Serial.flush();

	SetupControlCenter();

	delay(1000);
	Serial.println(F("SetupControlCenter complete"));
	Serial.flush();

	SetupTunnel();

	Serial.println(F("SetupTunnel complete"));
	Serial.flush();

	SetupTrackSections();

	Serial.println(F("SetupTrackSections complete"));
	Serial.flush();

	lastLightDifferenceThresholdPrinted = 0;

	Serial.println(F("Train::Train complete"));
	Serial.flush();
}

void Train::SetupControlCenter()
{
	delay(1000);
	Serial.println(F("SetupControlCenter start"));
	Serial.flush();

	m_controlCenter = new ControlCenter();
	
	delay(1000);
	Serial.println(F("new ControlCenter complete"));
	Serial.flush();

	m_controlCenter->Setup();

	delay(1000);
	Serial.println(F("m_controlCenter->Setup complete"));
	Serial.flush();

	m_controlCenter->RegisterObserver(this);

	Serial.println(F("RegisterObserver Complete"));
	Serial.flush();
}

void Train::SetupTunnel()
{
	Tunnel::TunnelSettings tunnelSettings;

	tunnelSettings.trainDetectionMode = Tunnel::TRAIN_DETECTION_MODE::PROXIMITY_SENSORS;
	tunnelSettings.lightMode = Tunnel::TUNNEL_LIGHT_MODE::AUTOMATIC_TRAIN_DETECTION;
	
	tunnelSettings.pin_mosfet_gate_lights = 8;
	
	tunnelSettings.pin_proximitySensor_entrance = 3;
	tunnelSettings.pin_proximitySensor_exit = 4;
	tunnelSettings.proximitySensorThresholdValue = 300;

	/*
	PhotoresistorSensor::PhotoresistorSensorSettings photoresistorSettings;
	photoresistorSettings.pin = A0;
	photoresistorSettings.eventMode = PhotoresistorSensor::PHOTORESISTOR_EVENT_MODE::AND_MODE;
	photoresistorSettings.lightDifferenceThreshold = 300;
	photoresistorSettings.readTimeInterval = 100;
	tunnelSettings.photoresistorSensorSettings_entrance = photoresistorSettings;
	
	photoresistorSettings.pin = A1;
	tunnelSettings.photoresistorSensorSettings_exit = photoresistorSettings;
	//*/

	m_tunnel = new Tunnel(tunnelSettings);
	m_tunnel->RegisterTunnelObserver(this);
	m_tunnel->StartStopMonitoringTunnelEntrances(false);

	Serial.println(F("SetupTunnel Complete"));
}

void Train::SetupTrackSections()
{
	HBridge_L293D::HBridge_L293D_Channel_Settings channelSettings;
	channelSettings.enable = 9;
	channelSettings.input1 = 6;
	channelSettings.input2 = 7;

	m_trackSection1.AssignHBridgeChannel(channelSettings);
	m_trackSection1.SetTargetSpeedPercentage(0,0);

	Serial.println(F("SetupTrackSections Complete"));
}

unsigned long lastPrint = 0;


void Train::Update()
{
	/*
	m_tunnel->Update();
	m_controlCenter->Update();
	m_trackSection1.Update();
	//*/

	if (millis() - lastPrint >= 5000) {
		lastPrint = millis();
		Serial.println(F("Debug message"));
	}

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

/*
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
//*/

void Train::OnChangeTrainDetectionMode()
{
	m_tunnel->ToggleSettings_TrainDetectionMode();
}

void Train::OnTestButtonClick()
{
	static bool s_isTrackSection1Enabled = false;
	s_isTrackSection1Enabled = !s_isTrackSection1Enabled;

	static bool s_isTrackSectionDirectionForward = true;

	if (s_isTrackSection1Enabled)
	{
		Serial.println("OnTestButtonClick - enable");
		m_trackSection1.Enable();
		m_trackSection1.SetTargetSpeedPercentage(100, 7000);

		if (s_isTrackSectionDirectionForward)
		{
			m_trackSection1.SetDirection(HBridgeMotorController::MotorDirection::MOTOR_DIRECTION_FORWARD);
		}
		else
		{
			m_trackSection1.SetDirection(HBridgeMotorController::MotorDirection::MOTOR_DIRECTION_BACKWARD);
		}
		s_isTrackSectionDirectionForward = !s_isTrackSectionDirectionForward;
	}
	else
	{
		Serial.println("OnTestButtonClick - disable");
		m_trackSection1.SetTargetSpeedPercentage(0, 7000);
		//m_trackSection1.Disable();
	}
}

void Train::OnTunnelEvent(const Tunnel::TunnelEventInfo& eventInfo)
{
	// do nothing
}

Train::~Train()
{


}