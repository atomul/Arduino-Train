#pragma once

#include <stdint.h>

class PhotoresistorSensor
{
public:
	enum class PHOTORESISTOR_EVENT_MODE : uint8_t
	{
		DISABLED,
		AND_MODE,
		OR_MODE
	};
	enum class PHOTORESISTOR_LIGHT_CHANGE : uint8_t
	{
		BRIGHTENED,
		DARKENED
	};

	class IPhotoresistorSensorObserver
	{
		friend class PhotoresistorSensor;

	public:
		struct PhotoresistorSensorEventInfo
		{
			uint8_t pin;
			unsigned short int luminosity;
			PHOTORESISTOR_LIGHT_CHANGE lightChangeType;
		};

		IPhotoresistorSensorObserver() {};

	private:
		virtual void OnLuminosityChanged(const PhotoresistorSensorEventInfo& eventInfo) = 0;
	};

	struct PhotoresistorSensorSettings
	{
		PhotoresistorSensorSettings()
			: pin(-1)
			, lightDifferenceThreshold(0)
			, readTimeInterval(0)
			, eventMode(PHOTORESISTOR_EVENT_MODE::DISABLED)
		{}
		uint8_t pin;
		unsigned short int lightDifferenceThreshold;
		unsigned long readTimeInterval;
		PHOTORESISTOR_EVENT_MODE eventMode;
	};

	PhotoresistorSensor(const PhotoresistorSensorSettings& photoresistorSettings);
	~PhotoresistorSensor();
	;
	virtual void Update();

	virtual void RegisterPhotoresistorSensorObserver(IPhotoresistorSensorObserver* observer)
	{
		m_photoresistorSensorObserver = observer;
	}
	
	unsigned short int ReadSensorValue();

	void ChangeSettings_LightDifferenceThreshold(unsigned short int lightDifferenceThreshold)
	{
		m_settings.lightDifferenceThreshold = lightDifferenceThreshold;
	}

	uint8_t GetPin() { return m_settings.pin; }

protected:
	PhotoresistorSensorSettings m_settings;

	unsigned short int m_lastSavedSensorValue;
	unsigned long m_lastSavedTime;

	IPhotoresistorSensorObserver* m_photoresistorSensorObserver;
};

