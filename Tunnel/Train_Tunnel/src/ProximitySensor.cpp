#include "ProximitySensor.h"

#include <Arduino.h>

#include "macro-logger/MacroLogger.h"

ProximitySensor::ProximitySensor()
	: ProximitySensor(INVALID_PIN)
{
}
ProximitySensor::ProximitySensor(const ProximitySensorSettings& settings)
	: ProximitySensor(settings.pin, settings.threshold)
{
	m_objectInProximityValue = settings.objectInProximityValue;
}
ProximitySensor::ProximitySensor(uint8_t inputPin)
	: ProximitySensor(inputPin, 0)
{
}

ProximitySensor::ProximitySensor(uint8_t inputPin, unsigned int threshold)
	: m_pin(inputPin)
	, m_threshold(threshold)
	, m_objectDetected(false)
	, m_lastChangeTime(0)
	, m_lastTimeSaved(false)
	, m_objectInProximityValue(OBJECT_IN_PROXIMITY_VALUE)
	, m_customInfo(nullptr)
	, m_firstRead(true)
{
}

ProximitySensor::~ProximitySensor()
{
}

void ProximitySensor::SetSettings(const ProximitySensorSettings& settings)
{
	m_pin = settings.pin;
	m_threshold = settings.threshold;
	m_objectInProximityValue = settings.objectInProximityValue;
	m_customInfo = settings.customInfo;
}

void ProximitySensor::Setup()
{
	if (m_pin != INVALID_PIN)
	{
		pinMode(m_pin, INPUT);
	}
}

void ProximitySensor::Update()
{
	int sensorValue = digitalRead(m_pin);

	//LOG_TRACE("%d - %d\n", m_pin, sensorValue);

	bool hasObstacle = false;
	if (sensorValue == m_objectInProximityValue)
	{
		hasObstacle = true;
	}
	//LOG_TRACE("%d - %d\n", m_pin, hasObstacle);

	if (m_firstRead)
	{
		m_objectDetected = hasObstacle;
		m_firstRead = !m_firstRead;
		return;
	}

	unsigned long currentTime = millis();

	if (hasObstacle != m_objectDetected)
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
			m_objectDetected = hasObstacle;
			m_lastTimeSaved = false;

			SendEvent(m_objectDetected);
		}
	}
}

bool ProximitySensor::HasObstacle()
{
	return m_objectDetected;
}

void ProximitySensor::SetListener(ProximitySensorHandler handler, void* context)
{
	m_handler = handler;
	m_context = context;
}

void ProximitySensor::SendEvent(bool isObjectDetected)
{
	if (m_handler)
	{
		ProximitySensorInfo info;
		info.inputPin = m_pin;
		info.isObjectDetected = isObjectDetected;
		info.customInfo = m_customInfo;

		m_handler(m_context, info);
	}
}
