#include "TrackSection.h"

#include "macro-logger/MacroLogger.h"

TrackSection::TrackSection()
	: m_isOccupied(false)
	, m_isReserved(false)
	, m_isTrainFullyInside(false)
	, m_state(TrackSectionState::UNKNOWN)
	, m_checkpointReached()
	, m_numberOfCheckpoints(0)
{
	
}

TrackSection::~TrackSection()
{
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

	proximitySensorSettings.pin = m_settings.pinProximitySensorEntrance;
	m_proximitySensorEntrance.SetSettings(proximitySensorSettings);
	m_proximitySensorEntrance.SetListener(&TrackSection::ProximitySensorEventThunk, this);
	m_proximitySensorEntrance.Setup();

	proximitySensorSettings.pin = m_settings.pinProximitySensorExit;
	m_proximitySensorExit.SetSettings(proximitySensorSettings);
	m_proximitySensorExit.SetListener(&TrackSection::ProximitySensorEventThunk, this);
	m_proximitySensorExit.Setup();

	uint8_t numberOfValidCheckpoints = 0;

	LOG_ERROR("m_settings.numberOfCheckpoints %d", m_settings.numberOfCheckpoints);
	delay(200);
	for (uint8_t index = 0; index < m_settings.numberOfCheckpoints; index++)
	{
		//LOG_ERROR("%d, %d", index, m_settings.numberOfCheckpoints);
		if (m_settings.pinsCheckpoints[index] != INVALID_PIN)
		{
			m_proximitySensorsIds[index] = index;

			proximitySensorSettings.pin = m_settings.pinsCheckpoints[index];
			proximitySensorSettings.customInfo = &m_proximitySensorsIds[index];
			//LOG_ERROR("CUSTOM_INFO %d", (*(uint8_t *)proximitySensorSettings.customInfo));
			//// ToDo: CHANGE THSI BACK, TESTING ONLY
			proximitySensorSettings.objectInProximityValue = !OBJECT_IN_PROXIMITY_VALUE;

			LOG_ERROR("%d", proximitySensorSettings.pin);
			delay(200);

			m_proximitySensorsCheckpoints[index].SetSettings(proximitySensorSettings);
			m_proximitySensorsCheckpoints[index].SetListener(&TrackSection::ProximitySensorEventThunk, this);
			m_proximitySensorsCheckpoints[index].Setup();
			numberOfValidCheckpoints++;
		}
	}
	m_settings.numberOfCheckpoints = numberOfValidCheckpoints;
	m_numberOfCheckpoints = numberOfValidCheckpoints;
}

void TrackSection::SetListener(TrackSectionHandler handler, void* context)
{
	m_context = context;
	m_handler = handler;
}

void TrackSection::Update()
{
	m_proximitySensorEntrance.Update();
	m_proximitySensorExit.Update();

	for (uint8_t index = 0; index < m_numberOfCheckpoints; index++)
	{
		m_proximitySensorsCheckpoints[index].Update();
	}
}

void TrackSection::SendEvent(TrackSectionState state)
{
	TrackSectionEventInfo info;
	info.sectionId = m_sectionId;
	info.state = state;
	info.checkpointId = m_checkpointReached;
	info.numberOfCheckpoints = m_numberOfCheckpoints;

	SendEvent(info);
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
	m_state = TrackSectionState::UNKNOWN;
	SendEvent(state);
}

void TrackSection::OnProximityEvent(const ProximitySensorInfo& info)
{
	switch (m_state)
	{
	case TrackSectionState::UNKNOWN:
	{
		if (info.isObjectDetected == true)
		{
			if (info.inputPin == m_proximitySensorEntrance.GetPin())
			{
				ChangeState(TrackSectionState::TRAIN_ENTERING_SECTION);
				LOG_INFO("TrackSection: Change state TRAIN_ENTERING_SECTION");

			}
			else if (info.inputPin == m_proximitySensorExit.GetPin())
			{
				ChangeState(TrackSectionState::TRAIN_EXITING_SECTION);
				LOG_INFO("TrackSection: Change state TRAIN_EXITING_SECTION");
			}
			else // this is a checkpoint
			{
				m_checkpointReached = info.customInfo ? *(static_cast<uint8_t*>(info.customInfo)) : INVALID_PIN;
				ChangeState(TrackSectionState::TRAIN_REACHED_CHECKPOINT);
				LOG_INFO("TrackSection: Change state TRAIN_REACHED_CHECKPOINT, \"%d\"", (uint8_t)m_checkpointReached);
			}
		}

		break;
	}
	case TrackSectionState::TRAIN_OUTSIDE_SECTION:
	{
		break;
	}
	case TrackSectionState::TRAIN_ENTERING_SECTION:
	{
		break;
	}
	case TrackSectionState::TRAIN_FULLY_INSIDE_SECTION:
	{
		break;
	}
	case TrackSectionState::TRAIN_REACHED_CHECKPOINT:
	{
		break;
	}
	case TrackSectionState::TRAIN_EXITING_SECTION:
	{
		break;
	}
	case TrackSectionState::TRAIN_FULLY_EXITED_SECTION:
	{
		break;
	}
	}
}

void TrackSection::ProximitySensorEventThunk(void* context, const ProximitySensorInfo& info)
{
	static_cast<TrackSection*>(context)->OnProximityEvent(info);
}

