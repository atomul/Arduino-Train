#include "SystemController.h"

#include <Arduino.h>

#include "_Config.h"

#include "macro-logger/MacroLogger.h"

//Speed = map(sensorVal, 0, 1023, 0, 255);
// map from one range to another.

// H-Bridge / L293D with octocoupler
// https://forum.arduino.cc/t/l293d-control-via-isp847-opto-isolator-problem/71953
// https://www.youtube.com/watch?v=KXGSGzxefZc&ab_channel=element14presents
// https://www.youtube.com/watch?v=Vm0k1yyVurQ&ab_channel=LewisLoflin

SystemController::SystemController()
{
	

}

SystemController::~SystemController()
{

}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void SystemController::Setup()
{
	SetupControlCenter();
	SetupTunnel();
	SetupTrackSections();

	pinMode(config::track_sections::PIN_RELAY_1X, INPUT);
	pinMode(config::track_sections::PIN_RELAY_X2, INPUT);
}

////////////////////////////////////////////////////////////////////////////////

void SystemController::SetupControlCenter()
{
	m_controlCenter.Setup();
	m_controlCenter.SetListener(&SystemController::ControlCenterEventThunk, this);
}

void SystemController::SetupTunnel()
{
	Tunnel::TunnelSettings tunnelSettings;

	tunnelSettings.trainDetectionMode = Tunnel::TRAIN_DETECTION_MODE::PROXIMITY_SENSORS;
	tunnelSettings.lightMode = Tunnel::TUNNEL_LIGHT_MODE::AUTOMATIC_TRAIN_DETECTION;

	tunnelSettings.pin_mosfet_gate_lights = config::tunnel::PIN_LIGHTS_MOSFET;

	tunnelSettings.pin_proximitySensor_entrance = config::tunnel::PIN_PROXIMITY_SENSOR_ENTRANCE;
	tunnelSettings.pin_proximitySensor_exit = config::tunnel::PIN_PROXIMITY_SENSOR_EXIT;
	tunnelSettings.proximitySensorThresholdValue = config::tunnel::PROXIMITY_SENSOR_THRESHOLD_VALUE;

	m_tunnel.SetSettings(tunnelSettings);
	m_tunnel.SetListener(&SystemController::TunnelEventThunk, this);
	
	m_tunnel.StartStopMonitoringTunnelEntrances(true);

	m_tunnel.Setup();
}

void SystemController::SetupTrackSections()
{
	HBridge_L293D::HBridge_L293D_Channel_Settings hBridgeChannelSettings;
	TrackSectionSettings trackSectionSettings;
	uint8_t pinsCheckpoints[config::track_sections::MAX_TRACK_SECTIONS_CHECKPOINTS]{};

	/////////////////////////
	//// TRACK_SECTION_1 ////
	/////////////////////////

	hBridgeChannelSettings.enable = config::track_sections::PIN_TRACK_SECTION_1_H_BRIDGE_ENABLE;
	hBridgeChannelSettings.input1 = config::track_sections::PIN_TRACK_SECTION_1_H_BRIDGE_INPUT_1;
	hBridgeChannelSettings.input2 = config::track_sections::PIN_TRACK_SECTION_1_H_BRIDGE_INPUT_2;
	
	trackSectionSettings.hBridgeChannelSettings = hBridgeChannelSettings;
	trackSectionSettings.numberOfCheckpoints = config::track_sections::NUMBER_OF_CHECKPOINTS_TRACK_SECTION_1;
	trackSectionSettings.pinProximitySensorEntrance = config::track_sections::PIN_TRACK_SECTION_1_PROXIMITY_SENSOR_ENTRANCE;
	trackSectionSettings.pinProximitySensorExit = config::track_sections::PIN_TRACK_SECTION_1_PROXIMITY_SENSOR_EXIT;
	
	for (int i = 0; i < trackSectionSettings.numberOfCheckpoints; i++)
	{
		trackSectionSettings.pinsCheckpoints[i] = config::track_sections::PINS_TRACK_SECTION_1_PROXIMITY_SENSOR_CHECKPOINTS[i];
	}	
	trackSectionSettings.trackSectionRole = TrackSectionRole::Normal;
	
	m_trackSections[0].SetListener(&SystemController::TrackSectionEventThunk, this);
	LOG_WARNING("trackSectionSettings.numberOfCheckpoints: %d", trackSectionSettings.numberOfCheckpoints);
	trackSectionSettings.DebugPrint();
	delay(200);
	m_trackSections[0].Setup(trackSectionSettings);

	///////////////////////////////////////////////////////////////////////////
	return;
	///////////////////////////////////////////////////////////////////////////
	// 
	/////////////////////////
	//// TRACK_SECTION_2 ////
	/////////////////////////

	trackSectionSettings.numberOfCheckpoints = config::track_sections::NUMBER_OF_CHECKPOINTS_TRACK_SECTION_2;
	trackSectionSettings.pinProximitySensorEntrance = config::track_sections::PIN_TRACK_SECTION_2_PROXIMITY_SENSOR_ENTRANCE;
	trackSectionSettings.pinProximitySensorExit = config::track_sections::PIN_TRACK_SECTION_2_PROXIMITY_SENSOR_EXIT;;
	trackSectionSettings.trackSectionRole = TrackSectionRole::Overlap;

	trackSectionSettings.numberOfCheckpoints = config::track_sections::NUMBER_OF_CHECKPOINTS_TRACK_SECTION_2;
	for (int i = 0; i < trackSectionSettings.numberOfCheckpoints; i++)
	{
		trackSectionSettings.pinsCheckpoints[i] = config::track_sections::PINS_TRACK_SECTION_2_PROXIMITY_SENSOR_CHECKPOINTS[i];
	}
	m_trackSections[1].SetListener(&SystemController::TrackSectionEventThunk, this);
	m_trackSections[1].Setup(trackSectionSettings);

	/////////////////////////
	//// TRACK_SECTION_2 ////
	/////////////////////////

	for (uint8_t i = 0; i < config::track_sections::MAX_TRACK_SECTIONS_CHECKPOINTS; ++i) {
		pinsCheckpoints[i] = INVALID_PIN;
	}

	hBridgeChannelSettings.enable = config::track_sections::PIN_TRACK_SECTION_3_H_BRIDGE_ENABLE;
	hBridgeChannelSettings.input1 = config::track_sections::PIN_TRACK_SECTION_3_H_BRIDGE_INPUT_1;
	hBridgeChannelSettings.input2 = config::track_sections::PIN_TRACK_SECTION_3_H_BRIDGE_INPUT_2;

	trackSectionSettings.hBridgeChannelSettings = hBridgeChannelSettings;
	trackSectionSettings.numberOfCheckpoints = 1;
	trackSectionSettings.pinProximitySensorEntrance = config::track_sections::PIN_TRACK_SECTION_3_PROXIMITY_SENSOR_ENTRANCE;
	trackSectionSettings.pinProximitySensorExit = config::track_sections::PIN_TRACK_SECTION_3_PROXIMITY_SENSOR_EXIT;
	
	trackSectionSettings.numberOfCheckpoints = config::track_sections::NUMBER_OF_CHECKPOINTS_TRACK_SECTION_3;
	for (int i = 0; i < trackSectionSettings.numberOfCheckpoints; i++)
	{
		trackSectionSettings.pinsCheckpoints[i] = config::track_sections::PINS_TRACK_SECTION_3_PROXIMITY_SENSOR_CHECKPOINTS[i];
	}

	trackSectionSettings.trackSectionRole = TrackSectionRole::Normal;

	m_trackSections[2].SetListener(&SystemController::TrackSectionEventThunk, this);
	m_trackSections[2].Setup(trackSectionSettings);
}

////////////////////////////////////////////////////////////////////////////////

void SystemController::OnControlCenterEvent(ControlCenterEvent eventType, const ControlCenterEventInfo& controlCenterEventInfo)
{
	switch (eventType)
	{
	case ControlCenterEvent::TunnelTrainDetectionChanged:
	{
		LOG_TRACE("TunnelTrainDetectionChanged");

		static bool shouldTurnOn = false;
		if (shouldTurnOn)
		{
			digitalWrite(11, HIGH);
		}
		else
		{
			digitalWrite(11, LOW);
		}
		shouldTurnOn = !shouldTurnOn;
		break;
	}
	case ControlCenterEvent::TunnelLightDetectionChanged:
	{
		LOG_TRACE("TunnelLightDetectionChanged");

		static bool shouldTurnOn = false;
		if (shouldTurnOn)
		{
			digitalWrite(10, HIGH);
		}
		else
		{
			digitalWrite(10, LOW);
		}
		shouldTurnOn = !shouldTurnOn;
		
		break;
	}
	case ControlCenterEvent::TestButtonClick:
	{
		LOG_TRACE("TestButtonClick");
		static bool areLightsOn = false;
		if (areLightsOn)
		{
			m_tunnel.TurnOffLights();
		}
		else
		{
			m_tunnel.TurnOnLights();
		}
		areLightsOn = !areLightsOn;
			
		break;
	}
	default:
	{

	}
	}
}

void SystemController::ControlCenterEventThunk(void* context, ControlCenterEvent eventType,  const ControlCenterEventInfo& controlCenterEventInfo)
{
	static_cast<SystemController*>(context)->OnControlCenterEvent(eventType, controlCenterEventInfo);
}

void SystemController::OnTunnelEvent(const Tunnel::TunnelEventInfo& eventInfo)
{
	// Check if train is entering / exiting and turn on / off lights
	//LOG_TRACE("OnTunnelEvent %d, %d", (int)eventInfo.eventType, (int)eventInfo.state);
}

void SystemController::TunnelEventThunk(void* context, const Tunnel::TunnelEventInfo& eventInfo)
{
	static_cast<SystemController*>(context)->OnTunnelEvent(eventInfo);
}

////////////////////////////////////////////////////////////////////////////////

void SystemController::OnTrackSectionEvent(const TrackSectionEventInfo& info)
{

}

void SystemController::TrackSectionEventThunk(void* context, const TrackSectionEventInfo& info)
{
	static_cast<SystemController*>(context)->OnTrackSectionEvent(info);
}

////////////////////////////////////////////////////////////////////////////////

void SystemController::Update()
{
	m_controlCenter.Update();
	m_tunnel.Update();

	//for (uint8_t index = 0; index < config::track_sections::NUMBER_OF_TRACK_SECTIONS; index++)
	for (uint8_t index = 0; index < 1; index++)
	{
		m_trackSections[index].Update();
	}
}

