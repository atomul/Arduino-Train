#pragma once

#include <Arduino.h>

#include "ProximitySensor.h"
#include "PhotoresistorSensor.h"

class Tunnel
	: IProximitySensorObserver
	, PhotoresistorSensor::IPhotoresistorSensorObserver
{
public:
	enum class TUNNEL_LIGHT_MODE : uint8_t
	{
		AUTOMATIC_TRAIN_DETECTION,
		AUTOMATIC_AMBIENT_LIGHT_DETECTION,
		MANUAL
	};

	enum class TUNNEL_EVENT_TYPE : uint8_t
	{
		TRAIN_TUNNEL_STATE_EVENT
	};

	enum class TRAIN_TUNNEL_STATE: uint8_t
	{
		TRAIN_OUTSIDE_TUNNEL,
		TRAIN_APROACHING_TUNNEL_ENTRANCE,
		TRAIN_IN_TUNNEL,
		TRAIN_STUCK_IN_TUNNEL,
		TRAIN_EXITING_TUNNEL,
		TRAIN_EXITED_TUNNEL
	};

	enum class TRAIN_DETECTION_MODE : uint8_t
	{
		PHOTORESISTOR_SENSORS,
		PROXIMITY_SENSORS
	};

	struct TunnelSettings
	{
		TunnelSettings()
			: pin_mosfet_gate_lights(-1)

			, photoresistorSensorSettings_entrance(PhotoresistorSensor::PhotoresistorSensorSettings())
			, photoresistorSensorSettings_exit(PhotoresistorSensor::PhotoresistorSensorSettings())

			, pin_proximitySensor_entrance(-1)
			, pin_proximitySensor_exit(-1)
			, proximitySensorThresholdValue(0)

			, trainDetectionMode(TRAIN_DETECTION_MODE::PROXIMITY_SENSORS)
			, lightMode(TUNNEL_LIGHT_MODE::AUTOMATIC_TRAIN_DETECTION)
		{}

		bool AreSettingsValid();

		uint8_t pin_mosfet_gate_lights;
		
		PhotoresistorSensor::PhotoresistorSensorSettings photoresistorSensorSettings_entrance;
		PhotoresistorSensor::PhotoresistorSensorSettings photoresistorSensorSettings_exit;

		uint8_t pin_proximitySensor_entrance;
		uint8_t pin_proximitySensor_exit;
		unsigned int proximitySensorThresholdValue;


		TRAIN_DETECTION_MODE trainDetectionMode;
		TUNNEL_LIGHT_MODE lightMode;
	};

	struct TunnelEventInfo
	{
		TunnelEventInfo(TUNNEL_EVENT_TYPE eventType)
			: eventType(eventType)
		{
		}
		TUNNEL_EVENT_TYPE eventType;
		TRAIN_TUNNEL_STATE state;
	};

	class ITunnelObserver
	{
		friend Tunnel;
	public:
		ITunnelObserver() {};

	private:
		virtual void OnTunnelEvent(const TunnelEventInfo& eventInfo) = 0;
	};

public:
	Tunnel(const TunnelSettings& settings);
	
	void Update();
	void HandleTrainTunnelState();

	virtual void RegisterTunnelObserver(ITunnelObserver* tunnelObserver);

	void TurnOnLights();
	void TurnOffLights();
	
	void StartStopMonitoringTunnelEntrances(bool shouldMonitorTunnelEntrances);

	// Proximity Sensor inferface
	//PhotoresistorSensor::IPhotoresistorSensorObserver::PhotoresistorSensorEventInfo a;
	void OnProximityChanged(bool hasObstacle, uint8_t pin);
	void OnLuminosityChanged(const PhotoresistorSensor::IPhotoresistorSensorObserver::PhotoresistorSensorEventInfo& eventInfo);

	void ToggleSettings_TrainDetectionMode();
	void ChangeSettings_TrainDetectionMode(TRAIN_DETECTION_MODE trainDetectionMode);
	void ChangeSettings_TunnelLightMode(TUNNEL_LIGHT_MODE lightMode);
	void ChangeSettings_PhotoresistorEntrance_LightDifferenceThreshold(unsigned short int lightDifferenceThreshold);
	void ChangeSettings_PhotoresistorExit_LightDifferenceThreshold(unsigned short int lightDifferenceThreshold);

protected:
	void SendEvent(const TunnelEventInfo& eventInfo);

	void ChangeTunnelStateState(TRAIN_TUNNEL_STATE tunnel_state);

	

private:
	bool m_shouldMonitorTunnelEntrances;
	TunnelSettings m_tunnelSettings;

	bool m_trainNearTunnelEntrance;
	bool m_trainNearTunnelExit;
	bool m_trainPassedTunnelExit;

	ProximitySensor* m_proximitySensorEntrance;
	ProximitySensor* m_proximitySensorExit;

	PhotoresistorSensor* m_photoresistorSensorEntrance;
	PhotoresistorSensor* m_photoresistorSensorExit;

	ITunnelObserver* m_tunnelObserver;

	TRAIN_TUNNEL_STATE m_trainTunnelState;
};