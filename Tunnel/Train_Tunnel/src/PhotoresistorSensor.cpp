#include "PhotoresistorSensor.h"

#include <Arduino.h>

#include "Utilities.h"

#define FAR_VALUE HIGH
#define CLOSE_VALUE LOW
PhotoresistorSensor::PhotoresistorSensor(const PhotoresistorSensorSettings& photoresistorSettings)
	: m_settings(photoresistorSettings)
	, m_lastSavedTime(0)
	, m_lastSavedSensorValue(0)
{
	pinMode(m_settings.pin, INPUT);
}

PhotoresistorSensor::~PhotoresistorSensor()
{
	delete m_photoresistorSensorObserver;
	m_photoresistorSensorObserver = NULL;
}

unsigned short int PhotoresistorSensor::ReadSensorValue()
{
	return analogRead(m_settings.pin);
}

void PhotoresistorSensor::Update()
{
	if (m_settings.eventMode == PhotoresistorSensor::PHOTORESISTOR_EVENT_MODE::DISABLED)
	{
		return;
	}

	unsigned short int sensorValue = 0;
	unsigned long currentTime = 0;
	bool timeIntervalReached = false;
	bool lightDifferenceThresholdReached = false;
	bool sendEvent = false;

	if (m_settings.readTimeInterval > 0)
	{
		currentTime = millis();
		if (currentTime - m_lastSavedTime >= m_settings.readTimeInterval)
		{
			timeIntervalReached = true;
		}
	}
	else
	{
		timeIntervalReached = true;
	}

	sensorValue = ReadSensorValue();
	if (m_lastSavedSensorValue == 0)
	{
		m_lastSavedSensorValue = sensorValue;
	}

	if (m_settings.lightDifferenceThreshold > 0)
	{
		if (IsThresholdReached(sensorValue, m_lastSavedSensorValue, m_settings.lightDifferenceThreshold))
		{
			lightDifferenceThresholdReached = true;
		}
	}
	else
	{
		lightDifferenceThresholdReached = true;
	}

	if (m_settings.eventMode == PhotoresistorSensor::PHOTORESISTOR_EVENT_MODE::AND_MODE)
	{
		if (timeIntervalReached && lightDifferenceThresholdReached)
		{
			sendEvent = true;
		}
	}
	else if (m_settings.eventMode == PhotoresistorSensor::PHOTORESISTOR_EVENT_MODE::OR_MODE)
	{
		if (timeIntervalReached || lightDifferenceThresholdReached)
		{
			sendEvent = true;
		}
	}

	if (sendEvent)
	{
		IPhotoresistorSensorObserver::PhotoresistorSensorEventInfo eventInfo;
		eventInfo.pin = m_settings.pin;
		eventInfo.luminosity = sensorValue;
		eventInfo.lightChangeType = (sensorValue > m_lastSavedSensorValue) ? PHOTORESISTOR_LIGHT_CHANGE::DARKENED : PHOTORESISTOR_LIGHT_CHANGE::BRIGHTENED;
		m_photoresistorSensorObserver->OnLuminosityChanged(eventInfo);
		m_lastSavedSensorValue = sensorValue;
		m_lastSavedTime = currentTime;
	}
}