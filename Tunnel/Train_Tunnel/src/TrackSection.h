#pragma once

#include <Arduino.h>

#include "Utilities.h"
#include "_Config.h"
#include "macro-logger/MacroLogger.h"

#include "HBridge.h"
#include "ProximitySensor.h"

// A X B
enum class TrackSectionRole : uint8_t
{
	Normal, // A, B, ...
	Overlap // X
};

enum class TrackSectionState : uint8_t
{
	UNKNOWN = 0,
	TRAIN_OUTSIDE_SECTION,
	TRAIN_ENTERING_SECTION,
	TRAIN_FULLY_INSIDE_SECTION,
	TRAIN_REACHED_CHECKPOINT,
	TRAIN_EXITING_SECTION,
	TRAIN_FULLY_EXITED_SECTION
};

struct TrackSectionEventInfo
{
	TrackSectionEventInfo()
		: sectionId(0)
		, state(TrackSectionState::UNKNOWN)
		, checkpointId(0)
		, numberOfCheckpoints(0)
	{}

	uint8_t sectionId;
	TrackSectionState state;
	uint8_t checkpointId;
	uint8_t numberOfCheckpoints;
};

struct TrackSectionSettings
{
	TrackSectionSettings()
		: sectionId(0)
		, pinProximitySensorEntrance(INVALID_PIN) // -1 = 255 in this case
		, pinProximitySensorExit(INVALID_PIN)
		, numberOfCheckpoints(0)
		, trackSectionRole(TrackSectionRole::Normal)
	{
		for (uint8_t i = 0; i < config::track_sections::MAX_TRACK_SECTIONS_CHECKPOINTS; ++i) {
			pinsCheckpoints[i] = INVALID_PIN;
		}
	}

	TrackSectionSettings(const TrackSectionSettings& other)
		: sectionId(other.sectionId)
		, hBridgeChannelSettings(other.hBridgeChannelSettings)
		, pinProximitySensorEntrance(other.pinProximitySensorEntrance)
		, pinProximitySensorExit(other.pinProximitySensorExit)
		, numberOfCheckpoints(other.numberOfCheckpoints)
		, trackSectionRole(other.trackSectionRole)
	{
		for (uint8_t i = 0; i < config::track_sections::MAX_TRACK_SECTIONS_CHECKPOINTS; ++i)
		{
			pinsCheckpoints[i] = other.pinsCheckpoints[i];
		}
	}

	void DebugPrint() const
	{
		LOG_TRACE("sectionId: %d", sectionId);
		LOG_TRACE("pinProximitySensorEntrance: %d", pinProximitySensorEntrance);
		LOG_TRACE("pinProximitySensorExit: %d", pinProximitySensorExit);
		LOG_TRACE("numberOfCheckpoints: %d", numberOfCheckpoints);
		LOG_TRACE("trackSectionRole: %d", (uint8_t)trackSectionRole);
		for (int i = 0; i < numberOfCheckpoints; i++)
		{
			LOG_TRACE("pinsCheckpoints[%d] = \"%d\"", i, pinsCheckpoints[i]);
		}
	}

	uint8_t sectionId;
	HBridge_L293D::HBridge_L293D_Channel_Settings hBridgeChannelSettings;
	uint8_t pinProximitySensorEntrance;
	uint8_t pinProximitySensorExit;
	uint8_t pinsCheckpoints[config::track_sections::MAX_TRACK_SECTIONS_CHECKPOINTS];
	uint8_t numberOfCheckpoints;
	TrackSectionRole trackSectionRole;
};

using TrackSectionHandler = void(*)(void* context, const TrackSectionEventInfo& info);

class TrackSection
{
public:
	TrackSection();
	~TrackSection();

	void Setup(const TrackSectionSettings& settings);
	void Update();

	void SendEvent(const TrackSectionEventInfo& info);
	void SendEvent(TrackSectionState state);

	void ChangeState(TrackSectionState state);

	void OnProximityEvent(const ProximitySensorInfo& info);
	static void ProximitySensorEventThunk(void* context, const ProximitySensorInfo& info);

	void SetListener(TrackSectionHandler handler, void* context);

private:
	bool m_isOccupied;
	bool m_isReserved;
	bool m_isTrainFullyInside;
	uint8_t m_checkpointReached;

	TrackSectionState m_state;

	uint8_t m_sectionId;
	TrackSectionSettings m_settings;
	HBridgeMotorController m_hBridgeController;

	ProximitySensor m_proximitySensorEntrance;
	ProximitySensor m_proximitySensorExit;

	uint8_t m_numberOfCheckpoints;
	ProximitySensor m_proximitySensorsCheckpoints[config::track_sections::MAX_TRACK_SECTIONS_CHECKPOINTS];
	uint8_t m_proximitySensorsIds[config::track_sections::MAX_TRACK_SECTIONS_CHECKPOINTS];

	void* m_context = nullptr;
	TrackSectionHandler m_handler = nullptr;
};