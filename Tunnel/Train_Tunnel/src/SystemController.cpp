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

	m_numberOfTrackSections = config::track_sections::MAX_TRACK_SECTIONS;

	/////////////////////////
	//// TRACK_SECTION_1 ////
	/////////////////////////

	hBridgeChannelSettings.enable = config::track_sections::PIN_TRACK_SECTION_1_H_BRIDGE_ENABLE;
	hBridgeChannelSettings.input1 = config::track_sections::PIN_TRACK_SECTION_1_H_BRIDGE_INPUT_1;
	hBridgeChannelSettings.input2 = config::track_sections::PIN_TRACK_SECTION_1_H_BRIDGE_INPUT_2;
	
	trackSectionSettings.sectionId = 0;
	trackSectionSettings.trackSectionRole = TrackSectionRole::Normal;
	trackSectionSettings.hBridgeChannelSettings = hBridgeChannelSettings;
	trackSectionSettings.numberOfCheckpoints = config::track_sections::NUMBER_OF_CHECKPOINTS_TRACK_SECTION_1;
	trackSectionSettings.pinProximitySensorEntrance = config::track_sections::PIN_TRACK_SECTION_1_PROXIMITY_SENSOR_ENTRANCE;
	trackSectionSettings.pinProximitySensorExit = config::track_sections::PIN_TRACK_SECTION_1_PROXIMITY_SENSOR_EXIT;

	for (int i = 0; i < trackSectionSettings.numberOfCheckpoints; i++)
	{
		trackSectionSettings.pinsCheckpoints[i] = config::track_sections::PINS_TRACK_SECTION_1_PROXIMITY_SENSOR_CHECKPOINTS[i];
	}	
	
	m_trackSections[0].SetListener(&SystemController::TrackSectionEventThunk, this);
	//trackSectionSettings.DebugPrint();
	delay(200);
	m_trackSections[0].Setup(trackSectionSettings);

	///////////////////////////////////////////////////////////////////////////
	return;
	///////////////////////////////////////////////////////////////////////////
	// 
	/////////////////////////
	//// TRACK_SECTION_2 ////
	/////////////////////////

	trackSectionSettings.sectionId = 1;
	trackSectionSettings.trackSectionRole = TrackSectionRole::Overlap;
	trackSectionSettings.numberOfCheckpoints = config::track_sections::NUMBER_OF_CHECKPOINTS_TRACK_SECTION_2;
	trackSectionSettings.pinProximitySensorEntrance = config::track_sections::PIN_TRACK_SECTION_2_PROXIMITY_SENSOR_ENTRANCE;
	trackSectionSettings.pinProximitySensorExit = config::track_sections::PIN_TRACK_SECTION_2_PROXIMITY_SENSOR_EXIT;;

	trackSectionSettings.numberOfCheckpoints = config::track_sections::NUMBER_OF_CHECKPOINTS_TRACK_SECTION_2;
	for (int i = 0; i < trackSectionSettings.numberOfCheckpoints; i++)
	{
		trackSectionSettings.pinsCheckpoints[i] = config::track_sections::PINS_TRACK_SECTION_2_PROXIMITY_SENSOR_CHECKPOINTS[i];
	}
	m_trackSections[1].SetListener(&SystemController::TrackSectionEventThunk, this);
	m_trackSections[1].Setup(trackSectionSettings);

	/////////////////////////
	//// TRACK_SECTION_3 ////
	/////////////////////////

	trackSectionSettings.sectionId = 2;
	trackSectionSettings.trackSectionRole = TrackSectionRole::Normal;
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

TrackSection& SystemController::GetNextTrackSection(uint8_t currentId)
{
	uint8_t nextIndex = (currentId + 1) % m_numberOfTrackSections ;
	return m_trackSections[nextIndex];
}

////////////////////////////////////////////////////////////////////////////////


//uint8_t CalcNextSpeedTargetPercentage(uint8_t numberOfCheckpoints, uint8_t reachedCheckpoint /*0..N*/, uint8_t startPct = 100)
//{
//	// reachedMarkerIndex: 0 = B/start, 1 = cp1, 2 = cp2, ... N = cpN
//	uint8_t denom = (uint8_t)(numberOfCheckpoints + 1);
//	uint8_t i = (uint8_t)(reachedCheckpoint + 1); // segmentIndex 1..N+1
//
//	// target = startPct * (denom - i) / denom
//	uint16_t num = (uint16_t)startPct * (uint16_t)(denom - i);
//	return (uint8_t)(num / denom); // floor
//}

////////////////////////////////////////////////////////////////////////////////

/*

endPct = 0   -> hard stop before/at boundary
endPct > 0   -> approach / creep into next section
N = 0        -> handle separately (no intermediate braking markers)

===============================================================
N = 0 checkpoints (only start -> end)
---------------------------------------------------------------
Event                 reachedCheckpoint   end=0   end=10   end=25
---------------------------------------------------------------
Enter section (B)     0                   0       10       25


===============================================================
N = 1 checkpoint
---------------------------------------------------------------
Event                 reachedCheckpoint   end=0   end=10   end=25
---------------------------------------------------------------
Enter section (B)     0                   50      55       62
Reach cp1             1                   0       10       25


===============================================================
N = 2 checkpoints
---------------------------------------------------------------
Event                 reachedCheckpoint   end=0   end=10   end=25
---------------------------------------------------------------
Enter section (B)     0                   66      70       75
Reach cp1             1                   33      40       50
Reach cp2             2                   0       10       25


===============================================================
N = 3 checkpoints
---------------------------------------------------------------
Event                 reachedCheckpoint   end=0   end=10   end=25
---------------------------------------------------------------
Enter section (B)     0                   75      77       81
Reach cp1             1                   50      55       62
Reach cp2             2                   25      32       43
Reach cp3             3                   0       10       25

===============================================================
N = 3 checkpoints, approachSpeedPct = 50
---------------------------------------------------------------
Event                 reachedCheckpoint   end=0   end=10   end=25
---------------------------------------------------------------
Enter section (B)     0                   37      40       43
Reach cp1             1                   25      30       37
Reach cp2             2                   12      20       31
Reach cp3             3                   0       10       25

*/

uint8_t CalcNextSpeedTargetPercentage(
	uint8_t numberOfCheckpoints,
	uint8_t reachedCheckpoint /*0..N*/,
	uint8_t lastCheckpointTargetSpeed = 0, // <-- maybe last checkpoint we don't want to stop, but creep (0 for full stop, 25 for approach). E.g. AXB, B is occupied, X not, I allow enter X, so I shouldn't stop at last checkpoint of A, but slow down and continue into X
	uint8_t trackSectionEntrySpeedPct = 100) // speed at which train is entering (maybe train is not at 100% speed when entering a section and starting computation so we don't want to ramp up then slow down again)
{
	uint8_t denom = (uint8_t)(numberOfCheckpoints + 1);
	uint8_t i = (uint8_t)(reachedCheckpoint + 1); // 1..N+1

	// clamp
	if (i > denom) i = denom;

	// frac = (denom - i) / denom in integer math
	uint16_t span = (uint16_t)(trackSectionEntrySpeedPct - lastCheckpointTargetSpeed);
	uint16_t num = span * (uint16_t)(denom - i);

	return (uint8_t)(lastCheckpointTargetSpeed + (num / denom)); // floor
}

////////////////////////////////////////////////////////////////////////////////

void SystemController::OnTrackSectionEvent(const TrackSectionEventInfo& info)
{
	switch (info.eventType)
	{
		case TrackSectionEventType::ALERT:
		{
			LOG_TRACE("OnTrackSectionEvent - Alert received: %s", TrackSectionAlertTypeToStr(info.alertType));
			break;
		}
		case TrackSectionEventType::OTHER:
		{
			LOG_TRACE("OnTrackSectionEvent - Train fully inside");
			break;
		}
		case TrackSectionEventType::CHANGE_STATE:
		{
			TrackSection& trackSection = m_trackSections[info.sectionId];
			bool isOverlapSection = trackSection.IsOverlapSection();

			// for now I'm presuming all sections are sequential
			TrackSection& nextOverlap = isOverlapSection ? GetNextTrackSection(trackSection.GetSectionId() + 2) : GetNextTrackSection(trackSection.GetSectionId() + 1);
			TrackSection& nextTrainSection = isOverlapSection ? GetNextTrackSection(trackSection.GetSectionId() + 1) : GetNextTrackSection(trackSection.GetSectionId() + 2);
			
			// if tracks aren't sequential, previous may be whoever reserved
			TrackSection& previousTrackSection = isOverlapSection ? GetNextTrackSection(trackSection.GetSectionId() - 1) : GetNextTrackSection(trackSection.GetSectionId() - 2);

			uint8_t approachSpeed = 0;

			switch (info.state)
			{
			case TrackSectionState::TRAIN_OUTSIDE_SECTION:
			{
				break;
			}
			case TrackSectionState::TRAIN_ENTERING_SECTION:
			case TrackSectionState::TRAIN_REACHED_CHECKPOINT:
			{
				bool needToStop = false;
				uint8_t lastCheckpointTargetSpeed = 0;
				uint8_t trackSectionEntrySpeedPct = 100; // maybe "trackSection.GetCurrentSpeedPercentage()" instead of default of 100. Speed at which train is entering (maybe train is not at 100% speed when entering a section and starting computation so we don't want to ramp up then slow down again)
				uint8_t maxSpeed = 100;

				if (trackSection.IsOverlapSection() == false)
				{
					if ((nextOverlap.IsReserved() && nextOverlap.IsReservedByTrackSectionId(trackSection.GetSectionId()) == false)
						|| nextOverlap.IsOccupied())
					{	
						lastCheckpointTargetSpeed = 0;
						/*
						if (info.state == TrackSectionState::TRAIN_ENTERING_SECTION)
						{
							if (trackSection.GetNumberOfCheckpoints() == 0)
							{
								needToStop = true;
							}
						}
						// train is not fully inside, can't stop now. NEED to treat "train fully inside" event probably.
						//*/
					}
					else
					{
						if (nextOverlap.IsReservedByTrackSectionId(trackSection.GetSectionId()) == false)
						{
							nextOverlap.Reserve(trackSection.GetSectionId());
						}

						if (nextTrainSection.IsReserved() || nextTrainSection.IsOccupied())
						{
							// just slow down to 25, don't stop
							lastCheckpointTargetSpeed = 25;
						}
						else
						{
							// don't reserve here. Will do it in overlap section
							// ramp up to 100
							lastCheckpointTargetSpeed = maxSpeed;

						}

						if (info.state == TrackSectionState::TRAIN_ENTERING_SECTION)
						{
							if (trackSection.GetNumberOfCheckpoints() == 0)
							{
								nextOverlap.SetOverlapSectionOwner(TrackSectionOverlapOwner::A);
							}

						}
						else if (info.state == TrackSectionState::TRAIN_REACHED_CHECKPOINT)
						{
							if (trackSection.DidReachLastSection())
							{
								// no need to match anything here as Overlap will be connected to this and has no power
								nextOverlap.SetOverlapSectionOwner(TrackSectionOverlapOwner::A);
							}
						}
					}
				}
				else // Is OVERLAP section
				{
					if ((nextTrainSection.IsReserved() && nextTrainSection.IsReservedByTrackSectionId(trackSection.GetSectionId()) == false)
						|| nextTrainSection.IsOccupied()
						)
					{
						lastCheckpointTargetSpeed = 0;

						// here train needs to stop as next section is occupied
						if (info.state == TrackSectionState::TRAIN_REACHED_CHECKPOINT)
						{
							needToStop = true;
						}
					}
					else
					{
						if (nextTrainSection.IsReservedByTrackSectionId(trackSection.GetSectionId()) == false)
						{
							nextTrainSection.Reserve(trackSection.GetSectionId());
						}

						lastCheckpointTargetSpeed = maxSpeed;

						if (info.state == TrackSectionState::TRAIN_REACHED_CHECKPOINT)
						{
							nextTrainSection.MatchSpeed(previousTrackSection);
							//previousTrackSection.Coast();
							nextOverlap.SetOverlapSectionOwner(TrackSectionOverlapOwner::B);
						}
					}
				}

				if (needToStop)
				{
					trackSection.StopWithCoast();
				}
				else
				{
					// if I'm overlap I can't set speed percentage. There is no HBridge assigned. set on previous?
					if (trackSection.IsOverlapSection() == false)
					{
						trackSection.SetTargetSpeedPercentage(
							CalcNextSpeedTargetPercentage(trackSection.GetNumberOfCheckpoints(), trackSection.GetCheckpointReached(), lastCheckpointTargetSpeed, trackSectionEntrySpeedPct),
							1000
						);
					}
					else
					{
						previousTrackSection.SetTargetSpeedPercentage(
							CalcNextSpeedTargetPercentage(trackSection.GetNumberOfCheckpoints(), trackSection.GetCheckpointReached(), lastCheckpointTargetSpeed, trackSectionEntrySpeedPct),
							1000
						);
					}
				}
			}
			case TrackSectionState::TRAIN_EXITING_SECTION:
			{
				break;
			}
			case TrackSectionState::TRAIN_FULLY_EXITED_SECTION:
			{
				if (trackSection.IsOverlapSection() == false)
				{
				}
				else
				{
					trackSection.ReleaseReservation();
				}
				// clear reservation, clear occupied (they may already be cleared in the state change of the TrackSection
				break;
			}
			}
			break;
		}
		default:
		{
			break;
		}
	}
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

