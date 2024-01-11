#pragma once

#include <stdint.h>

class ProximitySensor;

class IProximitySensorObserver
{
	friend ProximitySensor;
public:
	IProximitySensorObserver() {};

private:
	virtual void OnProximityChanged(bool hasObstacle, uint8_t pin) = 0;
};

class ProximitySensor
{
public:
	ProximitySensor(uint8_t inputPin);
	ProximitySensor(uint8_t inputPin, unsigned int threshold);
	ProximitySensor(const ProximitySensor& rhs);
	~ProximitySensor();

	virtual void Update();

	bool HasObstacle();

	virtual void RegisterProximitySensorObserver(IProximitySensorObserver* observer);

	uint8_t GetPin() { return m_pin; }

protected:
	virtual void OnProximityChanged(bool hasObstacle);

protected:
	uint8_t	m_pin;

	bool m_hasObstacle;
	unsigned int m_threshold;
	unsigned long m_lastChangeTime;
	bool m_lastTimeSaved;

	IProximitySensorObserver* m_proximityObserver;
};

