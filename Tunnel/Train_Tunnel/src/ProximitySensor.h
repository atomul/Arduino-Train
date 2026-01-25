#pragma once

#include <stdint.h>

#include "Utilities.h"

#define OBJECT_IN_PROXIMITY_VALUE LOW

class ProximitySensor;

class IProximitySensorObserver
{
	friend ProximitySensor;
public:
	IProximitySensorObserver() {};

private:
	virtual void OnProximityChanged(bool hasObstacle, uint8_t pin) = 0;
};

struct ProximitySensorSettings
{
	ProximitySensorSettings()
		: pin(INVALID_PIN)
		, threshold(0)
		, objectInProximityValue(OBJECT_IN_PROXIMITY_VALUE)
		, customInfo(nullptr)
	{}

	ProximitySensorSettings(uint8_t pin, unsigned int threshold, void* customInfo = nullptr)
		: pin(pin)
		, threshold(threshold)
		, customInfo(customInfo)
	{
	}

	ProximitySensorSettings(const ProximitySensorSettings& other)
		: pin(other.pin)
		, threshold(other.threshold)
		, objectInProximityValue(other.objectInProximityValue)
		, customInfo(other.customInfo)
	{
	}

	uint8_t pin;
	unsigned int threshold;
	bool objectInProximityValue;
	void* customInfo;
};

struct ProximitySensorInfo
{
	uint8_t inputPin;
	bool isObjectDetected;
	void* customInfo;
};

using ProximitySensorHandler = void(*)(void* context, const ProximitySensorInfo& info);

class ProximitySensor
{
public:
	ProximitySensor();
	ProximitySensor(const ProximitySensorSettings& settings);
	ProximitySensor(uint8_t inputPin);
	ProximitySensor(uint8_t inputPin, unsigned int threshold);
	~ProximitySensor();

	void SetSettings(const ProximitySensorSettings& settings);

	void Setup();

	virtual void Update();

	bool HasObstacle();
	bool IsObjectDetected();

	void SetListener(ProximitySensorHandler handler, void* context);

	uint8_t GetPin() { return m_pin; }

protected:
	virtual void SendEvent(bool isObjectDetected);

protected:
	uint8_t	m_pin;

	bool m_objectDetected;
	unsigned int m_threshold;
	unsigned long m_lastChangeTime;
	bool m_lastTimeSaved;

	void* m_context = nullptr;
	ProximitySensorHandler m_handler = nullptr;

	bool m_objectInProximityValue;
	void* m_customInfo = nullptr;

	bool m_firstRead;
};

