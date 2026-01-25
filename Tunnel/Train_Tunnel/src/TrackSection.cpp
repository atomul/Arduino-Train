#include "TrackSection.h"

#include "macro-logger/MacroLogger.h"

TrackSection::TrackSection()
	: m_isOccupied(false)
	, m_isReserved(false)
	, m_isTrainFullyInside(false)
	, m_state(TrackSectionState::UNKNOWN)
	, m_checkpointReached()
	, m_numberOfCheckpoints(0)
	, m_savedTime(false)
	, m_alert(TrackSectionAlertType::NONE)
	, m_reservedByTrackSectionId(INVALID_ID)
{
	
}

TrackSection::~TrackSection()
{
}

void TrackSection::SetTargetSpeedPercentage(uint8_t speedPercentage, unsigned long timeIntervalMiliseconds)
{
	m_hBridgeController.SetTargetSpeedPercentage(speedPercentage, timeIntervalMiliseconds);
}

uint8_t TrackSection::GetCurrentSpeedPercentage()
{
	return m_hBridgeController.GetCurrentSpeedPercentage();
}

void TrackSection::Setup(const TrackSectionSettings& settings)
{
	//settings.DebugPrint();
	m_settings = settings;
	m_sectionId = settings.sectionId;

	if (m_settings.trackSectionRole == TrackSectionRole::Normal)
	{
		m_hBridgeController.AssignHBridgeChannel(m_settings.hBridgeChannelSettings);
		m_hBridgeController.SetTargetSpeedPercentage(0, 0);
	}

	ProximitySensorSettings proximitySensorSettings;
	proximitySensorSettings.threshold = config::track_sections::PROXIMITY_SENSOR_THRESHOLD_VALUE;
	
	// ToDo remove when actual sensor. This is for button simulating IR sensor
	proximitySensorSettings.objectInProximityValue = !OBJECT_IN_PROXIMITY_VALUE;

	proximitySensorSettings.pin = m_settings.pinProximitySensorEntrance;
	m_proximitySensorEntrance.SetSettings(proximitySensorSettings);
	m_proximitySensorEntrance.SetListener(&TrackSection::ProximitySensorEventThunk, this);
	m_proximitySensorEntrance.Setup();

	proximitySensorSettings.pin = m_settings.pinProximitySensorExit;
	m_proximitySensorExit.SetSettings(proximitySensorSettings);
	m_proximitySensorExit.SetListener(&TrackSection::ProximitySensorEventThunk, this);
	m_proximitySensorExit.Setup();

	if (m_proximitySensorEntrance.IsObjectDetected() || m_proximitySensorExit.IsObjectDetected())
	{
		m_isOccupied = true;
	}

	uint8_t numberOfValidCheckpoints = 0;

	for (uint8_t index = 0; index < m_settings.numberOfCheckpoints; index++)
	{
		//LOG_ERROR("%d, %d", index, m_settings.numberOfCheckpoints);
		if (m_settings.pinsCheckpoints[index] != INVALID_PIN)
		{
			m_proximitySensorsIds[index] = index;

			proximitySensorSettings.pin = m_settings.pinsCheckpoints[index];
			proximitySensorSettings.customInfo = &m_proximitySensorsIds[index];
			//LOG_ERROR("CUSTOM_INFO %d", (*(uint8_t *)proximitySensorSettings.customInfo));
			
			// ToDo remove when actual sensor. This is for button simulating IR sensor
			proximitySensorSettings.objectInProximityValue = !OBJECT_IN_PROXIMITY_VALUE;

			//LOG_ERROR("%d", proximitySensorSettings.pin);
			//delay(200);

			m_proximitySensorsCheckpoints[index].SetSettings(proximitySensorSettings);
			m_proximitySensorsCheckpoints[index].SetListener(&TrackSection::ProximitySensorEventThunk, this);
			m_proximitySensorsCheckpoints[index].Setup();

			if (m_proximitySensorsCheckpoints[index].IsObjectDetected())
			{
				m_isOccupied = true;
			}

			numberOfValidCheckpoints++;
		}
	}
	m_settings.numberOfCheckpoints = numberOfValidCheckpoints;
	m_numberOfCheckpoints = numberOfValidCheckpoints;
}

bool TrackSection::IsOccupied_checkRawOccupancy()
{
	if (m_proximitySensorEntrance.IsObjectDetected() == true)
	{
		return true;
	}
	if (m_proximitySensorExit.IsObjectDetected() == true)
	{
		return true;
	}
	for (int i = 0; i < m_numberOfCheckpoints; i++)
	{
		if (m_proximitySensorsCheckpoints[i].IsObjectDetected() == true)
		{
			return true;
		}
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////

void TrackSection::Reserve(uint8_t trackSectionId)
{
	m_isReserved = true;
	m_reservedByTrackSectionId = trackSectionId;
}

void TrackSection::ReleaseReservation()
{
	m_isReserved = false;
	m_reservedByTrackSectionId = INVALID_ID;
}

void TrackSection::MatchSpeed(const TrackSection& section)
{
	m_hBridgeController.MatchSpeed(section.GetHBridge());
}

const HBridgeMotorController& TrackSection::GetHBridge()
{
	return m_hBridgeController;
}

bool TrackSection::IsReserved()
{
	return m_isReserved; 
}

int8_t TrackSection::GetReservedByTrackSectionId()
{
	return m_reservedByTrackSectionId;
}

bool TrackSection::IsReservedByTrackSectionId(uint8_t trackSectionId)
{
	if (m_isReserved && m_reservedByTrackSectionId == trackSectionId)
	{
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////

void TrackSection::SetListener(TrackSectionHandler handler, void* context)
{
	m_context = context;
	m_handler = handler;
}

void TrackSection::Update()
{
	if (m_state == TrackSectionState::UNKNOWN)
	{
		unsigned long currentTime = millis();
		if (m_savedTime == false)
		{
			m_lastTime = currentTime;
			m_savedTime = true;
		}
		else
		{
			if ((currentTime - m_lastTime >= 3000))
			{
				if (IsOccupied_checkRawOccupancy() == false)
				{
					ChangeState(TrackSectionState::TRAIN_OUTSIDE_SECTION);
				}
			}
		}
	}

	m_proximitySensorEntrance.Update();
	m_proximitySensorExit.Update();

	for (uint8_t index = 0; index < m_numberOfCheckpoints; index++)
	{
		m_proximitySensorsCheckpoints[index].Update();
	}
}

void TrackSection::SendEvent(const TrackSectionEventType& eventType)
{
	TrackSectionEventInfo info;
	info.sectionId = m_sectionId;
	info.state = m_state;
	info.checkpointId = m_checkpointReached;
	info.numberOfCheckpoints = m_numberOfCheckpoints;
	info.eventType = eventType;
	info.isTrainFullyInside = m_isTrainFullyInside;
	info.alertType = m_alert;

	SendEvent(info);
}

void TrackSection::SendAlert(const TrackSectionAlertType& alertType)
{
	LOG_TRACE("Sending alert: %s", TrackSectionAlertTypeToStr(alertType));

	SendEvent(TrackSectionEventType::ALERT);
	m_alert = TrackSectionAlertType::NONE;
}

void TrackSection::SendEvent(const TrackSectionEventInfo& info)
{
	if (m_handler)
	{
		m_handler(m_context, info);
	}
}

void TrackSection::ChangeState(TrackSectionState state)
{
	m_state = state;;
	//m_state = TrackSectionState::UNKNOWN;

	if (m_state == TrackSectionState::TRAIN_REACHED_CHECKPOINT)
	{
		LOG_INFO("TrackSection: Change state TRAIN_REACHED_CHECKPOINT, \"%d\"", (uint8_t)m_checkpointReached);
	}
	else
	{
		LOG_TRACE("TrackSectionState: %s", TrackSectionStateToStr(state));
	}
	switch (m_state)
	{
		case TrackSectionState::UNKNOWN:
		{
			// Initial or error state
			// Decide how to recover: reset sensors, force TRAIN_OUTSIDE_SECTION, etc.
			break;
		}

		case TrackSectionState::TRAIN_OUTSIDE_SECTION:
		{
			m_isOccupied = false;
			m_checkpointReached = 0;
			// No part of the train is inside this section
			// Wait for entrance sensor trigger
			// Possibly ensure section is powered / ready
			SendEvent(TrackSectionEventType::CHANGE_STATE);
			break;
		}

		case TrackSectionState::TRAIN_ENTERING_SECTION:
		{
			// Front of train has crossed the entrance sensor
			// Section should be powered and stable
			// You might start timing or counting blocks here
			m_isOccupied = true;
			m_isTrainFullyInside = false;
			SendEvent(TrackSectionEventType::CHANGE_STATE);
			break;
		}

		case TrackSectionState::TRAIN_REACHED_CHECKPOINT:
		{
			// One of the internal checkpoints was triggered
			// Could be used for speed adjustments, braking, logic decisions
			m_isOccupied = true;
			SendEvent(TrackSectionEventType::CHANGE_STATE);
			break;
		}

		case TrackSectionState::TRAIN_EXITING_SECTION:
		{
			// Front of train has crossed the exit sensor
			// Prepare to release section or transfer ownership
			m_isOccupied = true;
			m_isTrainFullyInside = false;
			SendEvent(TrackSectionEventType::CHANGE_STATE);
			break;
		}

		case TrackSectionState::TRAIN_FULLY_EXITED_SECTION:
		{
			// Train has completely left the section
			// Section can be powered down or assigned to another train
			
			m_isOccupied = false;
			m_isReserved = false;
			m_checkpointReached = 0;
			m_isTrainFullyInside = false;
			// ToDo: where should reserve logic be? I say in the controller

			SendEvent(TrackSectionEventType::CHANGE_STATE);
			ChangeState(TrackSectionState::TRAIN_OUTSIDE_SECTION);
			break;
		}

		default:
		{
			// Safety net — should never happen
			LOG_ERROR("TrackSectionState: invalid value (%d)", static_cast<uint8_t>(m_state));
			break;
		}
	}

}

void TrackSection::OnProximityEvent(const ProximitySensorInfo& info)
{
	//LOG_ERROR("OnProximityEvent %d, %d", info.inputPin, (int)info.isObjectDetected);
	switch (m_state)
	{
	case TrackSectionState::UNKNOWN:
	{
		if (info.isObjectDetected == true)
		{
			if (info.inputPin == m_proximitySensorEntrance.GetPin())
			{
				ChangeState(TrackSectionState::TRAIN_ENTERING_SECTION);
			}
			else if (info.inputPin == m_proximitySensorExit.GetPin())
			{
				ChangeState(TrackSectionState::TRAIN_EXITING_SECTION);
			}
			else // this is a checkpoint
			{
				m_checkpointReached = info.customInfo ? *(static_cast<uint8_t*>(info.customInfo)) : INVALID_PIN;

				ChangeState(TrackSectionState::TRAIN_REACHED_CHECKPOINT);
			}
		}
		else // (info.isObjectDetected == false)
		{
			if (info.inputPin == m_proximitySensorEntrance.GetPin())
			{
				if (m_proximitySensorExit.IsObjectDetected() == false)
				{
					OnTrainFullyInside();
				}
			}
			else if (info.inputPin == m_proximitySensorExit.GetPin())
			{
				ChangeState(TrackSectionState::TRAIN_FULLY_EXITED_SECTION);
			}
			else // this is a checkpoint
			{
			}
		}

		break;
	}
	case TrackSectionState::TRAIN_OUTSIDE_SECTION:
	{
		if (info.isObjectDetected == true)
		{
			if (info.inputPin == m_proximitySensorEntrance.GetPin())
			{
				ChangeState(TrackSectionState::TRAIN_ENTERING_SECTION);
			}
			else if (info.inputPin == m_proximitySensorExit.GetPin())
			{
				// how is this possible? bad sensors until exi?
				ChangeState(TrackSectionState::TRAIN_EXITING_SECTION);
			}
			else // this is a checkpoint
			{
				m_checkpointReached = info.customInfo ? *(static_cast<uint8_t*>(info.customInfo)) : INVALID_PIN;
				
				// how is this possible? bad enter sensor?
				ChangeState(TrackSectionState::TRAIN_REACHED_CHECKPOINT);
			}
		}
		else
		{
			// how is this possible?
			if (info.inputPin == m_proximitySensorEntrance.GetPin())
			{
				if (m_proximitySensorExit.IsObjectDetected() == false)
				{
					OnTrainFullyInside();
				}
			}
			else if (info.inputPin == m_proximitySensorExit.GetPin())
			{
				ChangeState(TrackSectionState::TRAIN_FULLY_EXITED_SECTION);
			}
			else // this is a checkpoint
			{
			}
		}
		break;
	}
	case TrackSectionState::TRAIN_ENTERING_SECTION:
	{
		if (info.isObjectDetected == true)
		{
			if (info.inputPin == m_proximitySensorExit.GetPin())
			{
				ChangeState(TrackSectionState::TRAIN_EXITING_SECTION);
			}
			else
			{
				m_checkpointReached = info.customInfo ? *(static_cast<uint8_t*>(info.customInfo)) : INVALID_PIN;
				ChangeState(TrackSectionState::TRAIN_REACHED_CHECKPOINT);
			}
		}
		else
		{
			if (info.inputPin == m_proximitySensorEntrance.GetPin())
			{
				if (m_proximitySensorExit.IsObjectDetected() == false)
				{
					OnTrainFullyInside();
				}
			}
		}
		break;
	}
	case TrackSectionState::TRAIN_REACHED_CHECKPOINT:
	{
		if (info.isObjectDetected == true)
		{
			if (info.inputPin == m_proximitySensorEntrance.GetPin())
			{
				// nonsense, unless another train entered here
			}
			else if (info.inputPin == m_proximitySensorExit.GetPin())
			{
				ChangeState(TrackSectionState::TRAIN_EXITING_SECTION);
			}
			else
			{
				uint8_t checkpoint = info.customInfo ? *(static_cast<uint8_t*>(info.customInfo)) : INVALID_PIN;
				if (checkpoint > m_checkpointReached) // fail safe
				{
					m_checkpointReached = checkpoint;
					ChangeState(TrackSectionState::TRAIN_REACHED_CHECKPOINT);
				}
			}
		}
		else
		{
			if (info.inputPin == m_proximitySensorEntrance.GetPin())
			{
				if (m_proximitySensorExit.IsObjectDetected() == false)
				{
					OnTrainFullyInside();
				}
			}
			else if (info.inputPin == m_proximitySensorExit.GetPin())
			{
				// should not happen as we are not in exiting;
				ChangeState(TrackSectionState::TRAIN_FULLY_EXITED_SECTION);
			}
		}
		break;
	}
	case TrackSectionState::TRAIN_EXITING_SECTION:
	{
		if (info.isObjectDetected == true)
		{
			SendAlert(TrackSectionAlertType::TRAIN_ENTERED_WHILE_SECTION_OCCUPIED);
		}
		else if (info.isObjectDetected == false)
		{
			if (info.inputPin == m_proximitySensorExit.GetPin())
			{
				ChangeState(TrackSectionState::TRAIN_FULLY_EXITED_SECTION);
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

void TrackSection::OnTrainFullyInside()
{
	LOG_TRACE("Train fully inside - send event");
	m_isOccupied = true;
	m_isTrainFullyInside = true;
	SendEvent(TrackSectionEventType::OTHER);
}

void TrackSection::StopWithCoast()
{
	SetTargetSpeedPercentage(0, 10);
}

void TrackSection::StopWithBreak()
{
	// change this to inverting polarity of motor so it stops hard
	SetTargetSpeedPercentage(0, 10);
}

void TrackSection::SetOverlapSectionOwner(TrackSectionOverlapOwner owner)
{
	if (m_settings.trackSectionRole != TrackSectionRole::Overlap)
	{
		return;
	}

	if (m_settings.pinPosOverlapOwner == INVALID_PIN || m_settings.pinNegOverlapOwner == INVALID_PIN)
	{
		return;
	}

	switch (owner)
	{
		case TrackSectionOverlapOwner::A:
		{
			digitalWrite(m_settings.pinPosOverlapOwner, HIGH);
			digitalWrite(m_settings.pinNegOverlapOwner, HIGH);
			break;
		}
		case TrackSectionOverlapOwner::B:
		{
			digitalWrite(m_settings.pinPosOverlapOwner, LOW);
			digitalWrite(m_settings.pinNegOverlapOwner, LOW);
			break;
		}
		default:
		{
			break;
		}

		delay(2);
	}
}

void TrackSection::ProximitySensorEventThunk(void* context, const ProximitySensorInfo& info)
{
	static_cast<TrackSection*>(context)->OnProximityEvent(info);
}

const char* TrackSectionStateToStr(TrackSectionState state)
{
	switch (state)
	{
		case TrackSectionState::UNKNOWN:					{ return "UNKNOWN"; break; }
		case TrackSectionState::TRAIN_OUTSIDE_SECTION:		{ return "TRAIN_OUTSIDE_SECTION"; break; }
		case TrackSectionState::TRAIN_ENTERING_SECTION:		{ return "TRAIN_ENTERING_SECTION"; break; }
		case TrackSectionState::TRAIN_REACHED_CHECKPOINT:	{ return "TRAIN_REACHED_CHECKPOINT"; break; }
		case TrackSectionState::TRAIN_EXITING_SECTION:		{ return "TRAIN_EXITING_SECTION"; break; }
		case TrackSectionState::TRAIN_FULLY_EXITED_SECTION:	{ return "TRAIN_FULLY_EXITED_SECTION"; break; }
		default: {
			return "INVALID"; break;
		}
	}

	return nullptr;
}

const char* TrackSectionAlertTypeToStr(TrackSectionAlertType state)
{
	switch (state)
	{
		case TrackSectionAlertType::NONE:									{ return "NONE"; }
		case TrackSectionAlertType::TRAIN_ENTERED_WHILE_SECTION_OCCUPIED:	{ return "TRAIN_ENTERED_WHILE_SECTION_OCCUPIED"; }
		default:
		{
			return "INVALID";
			break;
		}
	}

	return nullptr;
}
