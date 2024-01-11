#include "ProximitySensor.h"

#include <Arduino.h>


#define FAR_VALUE HIGH
#define CLOSE_VALUE LOW
ProximitySensor::ProximitySensor(uint8_t inputPin)
	: ProximitySensor(inputPin, 0)
{
}

ProximitySensor::ProximitySensor(uint8_t inputPin, unsigned int threshold)
	: m_pin(inputPin)
	, m_threshold(threshold)
	, m_hasObstacle(false)
	, m_lastChangeTime(0)
	, m_lastTimeSaved(false)
{
	pinMode(m_pin, INPUT);
}

ProximitySensor::ProximitySensor(const ProximitySensor& rhs)
{
}

ProximitySensor::~ProximitySensor()
{
	delete m_proximityObserver;
	m_proximityObserver = NULL;
}

void ProximitySensor::Update()
{
	int sensorValue = digitalRead(m_pin);
	bool hasObstacle = false;
	if (sensorValue == CLOSE_VALUE)
	{
		hasObstacle = true;
	}

	unsigned long currentTime = millis();

	if (hasObstacle != m_hasObstacle)
	{
		if (m_lastTimeSaved == false)
		{
			m_lastChangeTime = currentTime;
			m_lastTimeSaved = true;
		}
		/*
		Serial.println("hasObstacle != m_hasObstacle");
		Serial.println(hasObstacle);
		Serial.println(m_hasObstacle);
		Serial.println("-----");
		//*/
		if ((currentTime - m_lastChangeTime) >= m_threshold)
		{
			m_hasObstacle = hasObstacle;
			m_lastTimeSaved = false;

			OnProximityChanged(m_hasObstacle);
		}
	}
}

bool ProximitySensor::HasObstacle()
{
	return m_hasObstacle;
}

void ProximitySensor::RegisterProximitySensorObserver(IProximitySensorObserver* observer)
{
	m_proximityObserver = observer;
}

void ProximitySensor::OnProximityChanged(bool hasObstacle)
{
	if (m_proximityObserver)
	{
		m_proximityObserver->OnProximityChanged(hasObstacle, m_pin);
	}
}
