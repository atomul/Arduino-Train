#pragma once

#include <stdint.h>

#include "ControlCenter_old.h"
#include "ProximitySensor.h"
#include "Tunnel.h"

class Tank
	: public IControlCenterObserver
	, public IProximitySensorObserver
{
public:
	Tank();
	~Tank();

	void Update();

	// Remote Interface
	void OnSwitchLightsOn();
	void OnSwitchLightsOff();
	void OnLightSensitivityChanged(unsigned short int lightDifferenceThreshold);
	void OnLightsOverrideChanged(bool areLightsOverridden);

	void OnChangeTrainDetectionMode();

	// Proximity Sensor inferface
	void OnProximityChanged(bool hasObstacle, uint8_t pin);

private:
	enum class TRAIN_TUNNEL_STATE : uint8_t
	{
		TRAIN_OUTSIDE_TUNNEL,
		TRAIN_APROACHING_TUNNEL_ENTRANCE,
		TRAIN_IN_TUNNEL,
		TRAIN_EXITING_TUNNEL,
		TRAIN_EXITED_TUNNEL,
	};

	enum class LIGHTS_MODE : uint8_t
	{
		LIGHTS_ON,
		LIGHTS_OFF
	};

	enum class TRAIN_DETECTION_MODE : uint8_t
	{
		TRAIN_DETECTION_MODE_PHOTORESISTORS,
		TRAIN_DETECTION_MODE_PROXIMITY_SENSORS
	};

private:
	
	void SetupRemote();
	void TryToggleLights(LIGHTS_MODE mode);
	void ToggleLights(LIGHTS_MODE mode);
	
	void ChangeTunnelStateState(TRAIN_TUNNEL_STATE tunnel_state);

	ControlCenter* m_controlCenter;

	bool m_isEnabled;
	int m_threshholdValue;

	unsigned long m_startReadTime;

	int m_lastEntranceValue;
	int m_lastExitValue;

	ProximitySensor* m_proximitySensoEntrance;
	ProximitySensor* m_proximitySensoExit;
	
	unsigned long m_startPrintTime;
	bool m_areLightsOverridden;
	//int m_savedEntranceValue;
	//int m_savedExitValue;

	TRAIN_TUNNEL_STATE m_train_tunnel_state;
	TRAIN_DETECTION_MODE m_train_detection_mode;
};

