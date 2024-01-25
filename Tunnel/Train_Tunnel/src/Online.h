//#include <Arduino.h>
//#include <stdint.h>

namespace Online
{

////////////////////////////////////////////////////////////////////////////////
//// Utilities
////////////////////////////////////////////////////////////////////////////////

bool IsAnalogPin(uint8_t pin);
bool IsDigitalPin(uint8_t pin);
bool IsThresholdReached(int value1, int value2, int threshold);

bool IsAnalogPin(uint8_t pin)
{
	if (pin >= A0 && pin <= A7) return true;

	return false;
}

bool IsDigitalPin(uint8_t pin)
{
	if (pin >= 0 && pin <= 13) return true;

	return false;
}

bool IsThresholdReached(int value1, int value2, int threshold)
{
	int difference = 0;
	if (value1 > value2) difference = value1 - value2;
	else difference = value2 - value1;
	if (difference >= threshold)
	{
		return true;
	}

	return false;
}


////////////////////////////////////////////////////////////////////////////////
//// BUTTON
////////////////////////////////////////////////////////////////////////////////

enum class BUTTON_EVENT_TYPE : uint8_t
{
	BUTTON_CLICKS,
	BUTTON_DOWN,
	BUTTON_UP,
	BUTTON_HOLD,
	BUTTON_HOLD_RELEASE,
	BUTTON_SWITCH
};

enum class BUTTON_SWITCH_STATE : uint8_t
{
	BUTTON_SWITCH_STATE_UNDEFINED,
	BUTTON_SWITCH_STATE_ON,
	BUTTON_SWITCH_STATE_OFF
};

struct ButtonEventInfo
{
	BUTTON_EVENT_TYPE m_buttonEventType;
	uint8_t m_numberOfClicks;
	BUTTON_SWITCH_STATE m_switchState;

	ButtonEventInfo(BUTTON_EVENT_TYPE buttonEventType)
		: m_buttonEventType(buttonEventType)
		, m_numberOfClicks(0)
		, m_switchState(BUTTON_SWITCH_STATE::BUTTON_SWITCH_STATE_UNDEFINED)
	{
	}
};

class Button;

class IButtonObserver
{
	friend Button;
public:
	IButtonObserver() {};

private:
	/*
	virtual void OnButtonClicks(uint32_t buttonId, uint8_t numberOfClicks) = 0;
	virtual void OnButtonDown(uint32_t buttonId) = 0;
	virtual void OnButtonHold(uint32_t buttonId) = 0;
	virtual void OnButtonReleasedEvent(uint32_t buttonId) = 0;
	virtual void OnButtonUp(uint32_t buttonId) = 0;
	//*/
	virtual void OnButtonEvent(uint32_t buttonId, const ButtonEventInfo& buttonEventInfo) = 0;
};

class Button
{
public:
	Button(uint8_t inputPin);
	Button(uint8_t inputPin, unsigned long holdThreshold);
	Button(const Button& rhs);
	~Button();

	virtual void Update();

	bool IsPressed();
	bool WasPressed();

	virtual void RegisterButtonObserver(IButtonObserver* observer);

protected:
	/*
	virtual void OnButtonClicksEvent();
	virtual void OnButtonDownEvent();
	virtual void OnButtonUpEvent();
	virtual void OnButtonHoldEvent();
	virtual void OnButtonReleasedEvent();
	//*/

	void SendEvent(const ButtonEventInfo& buttonEventInfo);

protected:
	uint8_t	m_pin;

	int m_pressState;
	int m_previousPressState;

	unsigned long m_holdThreshold;
	unsigned long m_initialHoldTime;
	bool m_isHolding = false;

	unsigned long m_lastClick;
	unsigned long m_timeBetweenClicksThreshold;
	uint8_t m_numberOfClicks;
	bool m_needToSendClicks;

	IButtonObserver* m_buttonObserver;

	bool m_isSwitchedOn;
};

Button::Button(uint8_t inputPin)
	: Button(inputPin, 0)
{
}

Button::Button(uint8_t inputPin, unsigned long holdThreshold)
	: m_pressState(0)
	, m_previousPressState(0)
	, m_initialHoldTime(0)
	, m_holdThreshold(holdThreshold)
	, m_lastClick(0)
	, m_numberOfClicks(0)
	, m_timeBetweenClicksThreshold(500)
	, m_needToSendClicks(false)
	, m_isSwitchedOn(false)
{
	m_pin = inputPin;
	pinMode(m_pin, INPUT);
}

Button::~Button()
{
}

void Button::Update()
{
	bool sentHold = false;
	m_pressState = digitalRead(m_pin);
	unsigned long currentTime = millis();
	if (IsPressed())
	{
		// it was pressed earlier and still holding
		if (WasPressed())
		{
			if ((currentTime - m_initialHoldTime) >= m_holdThreshold)
			{
				m_isHolding = true;

				SendEvent(ButtonEventInfo(BUTTON_EVENT_TYPE::BUTTON_HOLD));
			}
		}
		// it was just pressed
		else
		{
			m_initialHoldTime = currentTime;

			SendEvent(ButtonEventInfo(BUTTON_EVENT_TYPE::BUTTON_DOWN));
		}
	}
	else
	{
		m_initialHoldTime = currentTime;

		// if it was pressed earlier and not it isn't it was just released
		if (WasPressed())
		{
			SendEvent(ButtonEventInfo(BUTTON_EVENT_TYPE::BUTTON_UP));

			//Sometimes after holding button, it only sends 1 click
			/*
			m_lastClick = currentTime;
			m_needToSendClicks = true;
			m_numberOfClicks++;
			//*/

			if (m_isHolding == true)
			{
				SendEvent(ButtonEventInfo(BUTTON_EVENT_TYPE::BUTTON_HOLD_RELEASE));
				m_isHolding = false;
				sentHold == true;
				m_needToSendClicks = false; // if it's a hold it's not a click anymore
				m_numberOfClicks = 0;
			}
			///*
			else
			{
				m_lastClick = currentTime;
				m_needToSendClicks = true;
				m_numberOfClicks++;
			}
			//*/
		}
	}

	if (m_needToSendClicks)
	{
		if ((currentTime - m_lastClick) >= m_timeBetweenClicksThreshold)
		{
			ButtonEventInfo buttonEventInfo = ButtonEventInfo(BUTTON_EVENT_TYPE::BUTTON_CLICKS);
			buttonEventInfo.m_numberOfClicks = m_numberOfClicks;
			Serial.println("Button clicks: " + String(m_numberOfClicks) + " " + String(currentTime - m_lastClick) + " " + String(currentTime) + " " + String(m_lastClick));
			SendEvent(buttonEventInfo);

			if (m_numberOfClicks == 1)
			{
				m_isSwitchedOn = !m_isSwitchedOn;

				ButtonEventInfo buttonEventInfo = ButtonEventInfo(BUTTON_EVENT_TYPE::BUTTON_SWITCH);
				buttonEventInfo.m_switchState = ((m_isSwitchedOn) ? BUTTON_SWITCH_STATE::BUTTON_SWITCH_STATE_ON : BUTTON_SWITCH_STATE::BUTTON_SWITCH_STATE_OFF);
				SendEvent(buttonEventInfo);
			}

			m_numberOfClicks = 0;
			m_needToSendClicks = false;
			m_lastClick = currentTime;
		}
	}

	m_previousPressState = m_pressState;
}

void Button::SendEvent(const ButtonEventInfo& buttonEventInfo)
{
	if (m_buttonObserver)
	{
		m_buttonObserver->OnButtonEvent(m_pin, buttonEventInfo);
	}
}

void Button::RegisterButtonObserver(IButtonObserver* observer)
{
	m_buttonObserver = observer;
}

bool Button::IsPressed() { return m_pressState; }
bool Button::WasPressed() { return m_previousPressState; }

////////////////////////////////////////////////////////////////////////////////
//// Proximity Sensor
////////////////////////////////////////////////////////////////////////////////

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


////////////////////////////////////////////////////////////////////////////////
//// Photoresistor sensor
////////////////////////////////////////////////////////////////////////////////

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

	void Update();
	void RegisterPhotoresistorSensorObserver(IPhotoresistorSensorObserver* observer);

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

////////////////////////////////////////////////////////////////////////////////

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

void PhotoresistorSensor::RegisterPhotoresistorSensorObserver(IPhotoresistorSensorObserver* observer)
{
	m_photoresistorSensorObserver = observer;
}

////////////////////////////////////////////////////////////////////////////////
//// Throttle
////////////////////////////////////////////////////////////////////////////////

class Throttle;

class IThrottleObserver
{
	friend Throttle;

public:
	IThrottleObserver() {}

private:
	virtual void OnThrottleChange(uint32_t, uint32_t) = 0;
};

class Throttle
{
public:
	Throttle(uint8_t pin, uint32_t threshold = 0);

	void Update();

	void RegisterObserver(IThrottleObserver* observer);

private:
	uint8_t m_pin;
	uint32_t m_value;

	uint32_t m_threshold;

	IThrottleObserver* m_observer;
};

////////////////////////////////////////////////////////////////////////////////

Throttle::Throttle(uint8_t pin, uint32_t threshold)
	: m_pin(pin)
	, m_value(0)
	, m_threshold(threshold)
{
}

void Throttle::Update()
{
	uint32_t value = analogRead(m_pin);

	if ((m_value - value) > m_threshold)
	{
		m_value = value;
		m_observer->OnThrottleChange(m_value, m_pin);
	}
}

void Throttle::RegisterObserver(IThrottleObserver* observer)
{
	m_observer = observer;
}

////////////////////////////////////////////////////////////////////////////////
//// Tunnel
////////////////////////////////////////////////////////////////////////////////

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

	enum class TRAIN_TUNNEL_STATE : uint8_t
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

////////////////////////////////////////////////////////////////////////////////

Tunnel::Tunnel(const TunnelSettings& tunnelSettings)
	: m_proximitySensorEntrance(NULL)
	, m_proximitySensorExit(NULL)
	, m_shouldMonitorTunnelEntrances(false)
	, m_tunnelObserver(NULL)
	, m_trainTunnelState(TRAIN_TUNNEL_STATE::TRAIN_OUTSIDE_TUNNEL)
	, m_trainNearTunnelEntrance(false)
	, m_trainNearTunnelExit(false)
	, m_trainPassedTunnelExit(false)
{
	m_tunnelSettings = tunnelSettings;

	m_proximitySensorEntrance = new ProximitySensor(tunnelSettings.pin_proximitySensor_entrance, tunnelSettings.proximitySensorThresholdValue);
	m_proximitySensorEntrance->RegisterProximitySensorObserver(this);

	m_proximitySensorExit = new ProximitySensor(tunnelSettings.pin_proximitySensor_exit, tunnelSettings.proximitySensorThresholdValue);
	m_proximitySensorExit->RegisterProximitySensorObserver(this);

	m_photoresistorSensorEntrance = new PhotoresistorSensor(tunnelSettings.photoresistorSensorSettings_entrance);
	m_photoresistorSensorExit = new PhotoresistorSensor(tunnelSettings.photoresistorSensorSettings_exit);

	m_photoresistorSensorEntrance->RegisterPhotoresistorSensorObserver(this);
	m_photoresistorSensorExit->RegisterPhotoresistorSensorObserver(this);

	pinMode(tunnelSettings.pin_mosfet_gate_lights, OUTPUT);
}

bool Tunnel::TunnelSettings::AreSettingsValid()
{
	return true;
}

void Tunnel::Update()
{
	if (m_shouldMonitorTunnelEntrances)
	{
		switch (m_tunnelSettings.trainDetectionMode)
		{
		case TRAIN_DETECTION_MODE::PROXIMITY_SENSORS:
		{
			m_proximitySensorEntrance->Update();
			m_proximitySensorExit->Update();
			break;
		}
		case TRAIN_DETECTION_MODE::PHOTORESISTOR_SENSORS:
		{
			m_photoresistorSensorEntrance->Update();
			m_photoresistorSensorExit->Update();
			break;
		}
		}

		HandleTrainTunnelState();
	}
}

void Tunnel::HandleTrainTunnelState()
{
	switch (m_trainTunnelState)
	{
	case TRAIN_TUNNEL_STATE::TRAIN_OUTSIDE_TUNNEL:
	{
		// should I just set in "OnProximityChanged" a variable m_trainApproachingTunnelEntrance = true and test it out here and change the state here so all logic is done here?
		if (m_trainNearTunnelEntrance)
		{
			m_trainNearTunnelEntrance = false;
			ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_APROACHING_TUNNEL_ENTRANCE);
		}

		break;
	}
	case TRAIN_TUNNEL_STATE::TRAIN_APROACHING_TUNNEL_ENTRANCE:
	{
		if (m_tunnelSettings.lightMode == TUNNEL_LIGHT_MODE::AUTOMATIC_TRAIN_DETECTION)
		{
			TurnOnLights();
		}
		ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_IN_TUNNEL);
		break;
	}
	case TRAIN_TUNNEL_STATE::TRAIN_IN_TUNNEL:
	{
		if (m_trainNearTunnelExit)
		{
			m_trainNearTunnelExit = false;
			ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_EXITING_TUNNEL);
		}


		break;
	}
	case TRAIN_TUNNEL_STATE::TRAIN_EXITING_TUNNEL:
	{
		if (m_trainPassedTunnelExit)
		{
			m_trainPassedTunnelExit = false;
			ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_EXITED_TUNNEL);
		}

		break;
	}
	case TRAIN_TUNNEL_STATE::TRAIN_EXITED_TUNNEL:
	{
		if (m_tunnelSettings.lightMode == TUNNEL_LIGHT_MODE::AUTOMATIC_TRAIN_DETECTION)
		{
			TurnOffLights();
		}
		ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_OUTSIDE_TUNNEL);

		break;
	}
	}
}

void Tunnel::ToggleSettings_TrainDetectionMode()
{
	if (m_tunnelSettings.trainDetectionMode == TRAIN_DETECTION_MODE::PROXIMITY_SENSORS)
	{
		ChangeSettings_TrainDetectionMode(TRAIN_DETECTION_MODE::PHOTORESISTOR_SENSORS);
	}
	else if (m_tunnelSettings.trainDetectionMode == TRAIN_DETECTION_MODE::PHOTORESISTOR_SENSORS)
	{
		ChangeSettings_TrainDetectionMode(TRAIN_DETECTION_MODE::PROXIMITY_SENSORS);
	}
}

void Tunnel::ChangeSettings_TrainDetectionMode(TRAIN_DETECTION_MODE trainDetectionMode)
{
	if (m_tunnelSettings.trainDetectionMode != trainDetectionMode)
	{
		m_tunnelSettings.trainDetectionMode = trainDetectionMode;
		if (m_tunnelSettings.trainDetectionMode == TRAIN_DETECTION_MODE::PROXIMITY_SENSORS)
		{
			Serial.println("Train detection mode changed to PROXIMITY sensors");
		}
		else if (m_tunnelSettings.trainDetectionMode == TRAIN_DETECTION_MODE::PHOTORESISTOR_SENSORS)
		{
			Serial.println("Train detection mode changed to PHOTORESISTOR sensors");
		}

		ChangeTunnelStateState(TRAIN_TUNNEL_STATE::TRAIN_OUTSIDE_TUNNEL);
	}
	else
	{
		Serial.println("Train detection mode NOT changed. Already in this mode.");
	}
}

void Tunnel::ChangeSettings_TunnelLightMode(TUNNEL_LIGHT_MODE lightMode)
{
	m_tunnelSettings.lightMode = lightMode;
	if (lightMode == TUNNEL_LIGHT_MODE::AUTOMATIC_TRAIN_DETECTION)
	{
		Serial.println("Tunnel light detection changed to \"automatic light detection\" mode");
	}
	else if (lightMode == TUNNEL_LIGHT_MODE::MANUAL)
	{
		Serial.println("Tunnel light detection changed to \"manual\" mode");
	}
}

void Tunnel::ChangeSettings_PhotoresistorEntrance_LightDifferenceThreshold(unsigned short int lightDifferenceThreshold)
{
	m_photoresistorSensorEntrance->ChangeSettings_LightDifferenceThreshold(lightDifferenceThreshold);
}

void Tunnel::ChangeSettings_PhotoresistorExit_LightDifferenceThreshold(unsigned short int lightDifferenceThreshold)
{
	m_photoresistorSensorExit->ChangeSettings_LightDifferenceThreshold(lightDifferenceThreshold);
}

void Tunnel::ChangeTunnelStateState(TRAIN_TUNNEL_STATE tunnelState)
{
	m_trainTunnelState = tunnelState;
	String printStr = "Tunnel state changed: \"";

	switch (tunnelState)
	{
	case TRAIN_TUNNEL_STATE::TRAIN_OUTSIDE_TUNNEL:
	{
		printStr = printStr + "TRAIN_OUTSIDE_TUNNEL";
		break;
	}
	case TRAIN_TUNNEL_STATE::TRAIN_APROACHING_TUNNEL_ENTRANCE:
	{
		printStr = printStr + "TRAIN_APROACHING_TUNNEL_ENTRANCE";
		break;
	}
	case TRAIN_TUNNEL_STATE::TRAIN_IN_TUNNEL:
	{
		printStr = printStr + "TRAIN_IN_TUNNEL";
		break;
	}
	case TRAIN_TUNNEL_STATE::TRAIN_EXITING_TUNNEL:
	{
		printStr = printStr + "TRAIN_EXITING_TUNNEL";
		break;
	}
	case TRAIN_TUNNEL_STATE::TRAIN_EXITED_TUNNEL:
	{
		printStr = printStr + "TRAIN_EXITED_TUNNEL";
		break;
	}
	}

	TunnelEventInfo eventInfo(TUNNEL_EVENT_TYPE::TRAIN_TUNNEL_STATE_EVENT);
	eventInfo.state = m_trainTunnelState;
	SendEvent(eventInfo);

	printStr = printStr + "\"";
	Serial.println(printStr);
}

void Tunnel::OnProximityChanged(bool hasObstacle, uint8_t pin)
{
	if (m_tunnelSettings.trainDetectionMode != TRAIN_DETECTION_MODE::PROXIMITY_SENSORS)
	{
		return;
	}

	switch (m_trainTunnelState)
	{
	case TRAIN_TUNNEL_STATE::TRAIN_OUTSIDE_TUNNEL:
	{
		if (pin == m_proximitySensorEntrance->GetPin())
		{
			if (hasObstacle)
			{
				m_trainNearTunnelEntrance = true;
			}
		}

		break;
	}
	case TRAIN_TUNNEL_STATE::TRAIN_IN_TUNNEL:
	{
		if (pin == m_proximitySensorExit->GetPin())
		{
			if (hasObstacle)
			{
				m_trainNearTunnelExit = true;
			}
		}

		break;
	}
	case TRAIN_TUNNEL_STATE::TRAIN_EXITING_TUNNEL:
	{
		if (pin == m_proximitySensorExit->GetPin())
		{
			if (!hasObstacle)
			{
				m_trainPassedTunnelExit = true;
			}
		}

		break;
	}
	}
}

void Tunnel::OnLuminosityChanged(const PhotoresistorSensor::IPhotoresistorSensorObserver::PhotoresistorSensorEventInfo& eventInfo)
{
	if (m_tunnelSettings.trainDetectionMode != TRAIN_DETECTION_MODE::PHOTORESISTOR_SENSORS)
	{
		return;
	}

	switch (m_trainTunnelState)
	{
	case TRAIN_TUNNEL_STATE::TRAIN_OUTSIDE_TUNNEL:
	{
		if (eventInfo.pin == m_photoresistorSensorEntrance->GetPin())
		{
			Serial.println(" " + String(eventInfo.pin) + " " + String((int)eventInfo.lightChangeType));
			if (eventInfo.lightChangeType == PhotoresistorSensor::PHOTORESISTOR_LIGHT_CHANGE::DARKENED)
			{
				m_trainNearTunnelEntrance = true;
			}
		}

		break;
	}
	case TRAIN_TUNNEL_STATE::TRAIN_IN_TUNNEL:
	{
		if (eventInfo.pin == m_photoresistorSensorExit->GetPin())
		{
			if (eventInfo.lightChangeType == PhotoresistorSensor::PHOTORESISTOR_LIGHT_CHANGE::DARKENED)
			{
				m_trainNearTunnelExit = true;
			}
		}

		break;
	}
	case TRAIN_TUNNEL_STATE::TRAIN_EXITING_TUNNEL:
	{
		if (eventInfo.pin == m_photoresistorSensorExit->GetPin())
		{
			if (eventInfo.lightChangeType == PhotoresistorSensor::PHOTORESISTOR_LIGHT_CHANGE::BRIGHTENED)
			{
				m_trainPassedTunnelExit = true;
			}
		}

		break;
	}
	}
}

void Tunnel::TurnOnLights()
{
	digitalWrite(m_tunnelSettings.pin_mosfet_gate_lights, HIGH);
	Serial.println("Lights ON");
}

void Tunnel::TurnOffLights()
{
	digitalWrite(m_tunnelSettings.pin_mosfet_gate_lights, LOW);
	Serial.println("Lights OFF");
}

void Tunnel::StartStopMonitoringTunnelEntrances(bool shouldMonitorTunnelEntrances)
{
	m_shouldMonitorTunnelEntrances = shouldMonitorTunnelEntrances;
	if (m_shouldMonitorTunnelEntrances)
	{
		Serial.println("Tunnel setting changed: START monitoring");
	}
	else
	{
		Serial.println("Tunnel setting changed: STOP monitoring");
	}
}

void Tunnel::RegisterTunnelObserver(ITunnelObserver* tunnelObserver)
{
	m_tunnelObserver = tunnelObserver;
}


void Tunnel::SendEvent(const TunnelEventInfo& eventInfo)
{
	if (m_tunnelObserver)
	{
		m_tunnelObserver->OnTunnelEvent(eventInfo);
	}
}


////////////////////////////////////////////////////////////////////////////////
//// Control Center
////////////////////////////////////////////////////////////////////////////////

class IControlCenterObserver
{
public:
	IControlCenterObserver() {};

	virtual void OnSwitchLightsOn() = 0;
	virtual void OnSwitchLightsOff() = 0;

	virtual void OnSwitchTunnelDetectionOn() = 0;
	virtual void OnSwitchTunnelDetectionOff() = 0;

	virtual void OnLightsOverrideChanged(bool areLightsOverridden) = 0;
	virtual void OnLightSensitivityChanged(unsigned short int lightDifferenceThreshold) = 0;
	virtual void OnChangeTrainDetectionMode() = 0;
};

class ControlCenter
	: public IButtonObserver
	, public IThrottleObserver
{
public:
	ControlCenter();
	~ControlCenter();

	void Update();

	void RegisterObserver(IControlCenterObserver* observer);

private:

	// Button Interface
	void OnButtonEvent(uint32_t buttonId, const ButtonEventInfo& buttonEventInfo) override final;

	// Throttle Interface
	void OnThrottleChange(uint32_t speed, uint32_t throttleId) override final;

private:
	Button* m_buttonSettings;
	Button* m_buttonEnableTunnelDetection;
	Throttle* m_throttle;

	IControlCenterObserver* m_observer;

	bool m_areLightsOverridden;
	bool m_isTunnelDetectionOn;
};

////////////////////////////////////////////////////////////////////////////////

const uint8_t k_pin_button_settings = 2;
const unsigned long k_button_enable_holdThreshold = 3000;

const uint8_t k_pin_throttle_light_sensitivity = A2;
const uint8_t k_throttle_threshold = 2;

const uint8_t k_pin_button_toggle_tunnel_detection = 5;

bool IsButtonEnable(uint8_t buttonPin)
{
	return (k_pin_button_settings == buttonPin);
}

ControlCenter::ControlCenter()
{
	m_areLightsOverridden = false;
	m_isTunnelDetectionOn = false;

	m_buttonSettings = new Button(k_pin_button_settings, k_button_enable_holdThreshold);
	m_buttonEnableTunnelDetection = new Button(k_pin_button_toggle_tunnel_detection, 60000);
	m_throttle = new Throttle(k_pin_throttle_light_sensitivity, k_throttle_threshold);

	m_buttonSettings->RegisterButtonObserver(this);
	m_buttonEnableTunnelDetection->RegisterButtonObserver(this);
	m_throttle->RegisterObserver(this);
}

ControlCenter::~ControlCenter()
{
	delete m_buttonSettings;
	m_buttonSettings = NULL;

	delete m_buttonEnableTunnelDetection;
	m_buttonEnableTunnelDetection = NULL;

	delete m_throttle;
	m_throttle = NULL;
}

void ControlCenter::Update()
{
	m_buttonSettings->Update();
	m_buttonEnableTunnelDetection->Update();
	m_throttle->Update();
}

void ControlCenter::RegisterObserver(IControlCenterObserver* observer)
{
	m_observer = observer;
}

void ControlCenter::OnButtonEvent(uint32_t buttonId, const ButtonEventInfo& buttonEventInfo)
{
	switch (buttonEventInfo.m_buttonEventType)
	{
	case BUTTON_EVENT_TYPE::BUTTON_SWITCH:
	{
		if (buttonId == k_pin_button_settings)
		{
			if (m_areLightsOverridden)
			{
				if (buttonEventInfo.m_switchState == BUTTON_SWITCH_STATE::BUTTON_SWITCH_STATE_ON)
				{
					m_observer->OnSwitchLightsOn();
				}
				else if (buttonEventInfo.m_switchState == BUTTON_SWITCH_STATE::BUTTON_SWITCH_STATE_OFF)
				{
					m_observer->OnSwitchLightsOff();
				}
			}
		}
		else if (buttonId == k_pin_button_toggle_tunnel_detection)
		{
			m_isTunnelDetectionOn = !m_isTunnelDetectionOn;
			if (m_isTunnelDetectionOn)
			{
				m_observer->OnSwitchTunnelDetectionOn();
			}
			else
			{
				m_observer->OnSwitchTunnelDetectionOff();
			}
		}
		break;
	}
	case BUTTON_EVENT_TYPE::BUTTON_CLICKS:
	{
		if (buttonId == k_pin_button_settings)
		{
			if (buttonEventInfo.m_numberOfClicks == 2)
			{
				m_observer->OnChangeTrainDetectionMode();
			}
		}

		break;
	}
	case BUTTON_EVENT_TYPE::BUTTON_HOLD_RELEASE:
	{
		if (buttonId == k_pin_button_settings)
		{
			m_areLightsOverridden = !m_areLightsOverridden;
			m_observer->OnLightsOverrideChanged(m_areLightsOverridden);
		}
		break;
	}
	}
}

void ControlCenter::OnThrottleChange(uint32_t value, uint32_t throttleId)
{
	if (throttleId == k_pin_throttle_light_sensitivity)
	{
		m_observer->OnLightSensitivityChanged(value);
	}
}

////////////////////////////////////////////////////////////////////////////////
//// Train
////////////////////////////////////////////////////////////////////////////////

class Train
	: public IControlCenterObserver
	, public Tunnel::ITunnelObserver
{
public:
	Train();
	~Train();

	void SetupControlCenter();
	void SetupTunnel();

	void Update();

	// Control Center Observer Interface
	void OnSwitchLightsOn() override;
	void OnSwitchLightsOff() override;

	void OnSwitchTunnelDetectionOn() override;
	void OnSwitchTunnelDetectionOff() override;

	void OnLightsOverrideChanged(bool areLightsOverridden) override;
	void OnLightSensitivityChanged(unsigned short int lightDifferenceThreshold) override;
	void OnChangeTrainDetectionMode() override;

	// Tunnel Observer Interface
	void OnTunnelEvent(const Tunnel::TunnelEventInfo& eventInfo) override;

private:
	ControlCenter* m_controlCenter;
	Tunnel* m_tunnel;

	unsigned short int lastLightDifferenceThresholdPrinted = 0;
};

////////////////////////////////////////////////////////////////////////////////

Train::Train()
{
	SetupControlCenter();
	SetupTunnel();

	lastLightDifferenceThresholdPrinted = 0;
}

void Train::SetupControlCenter()
{
	m_controlCenter = new ControlCenter();
	m_controlCenter->RegisterObserver(this);
}

void Train::SetupTunnel()
{
	Tunnel::TunnelSettings tunnelSettings;

	tunnelSettings.trainDetectionMode = Tunnel::TRAIN_DETECTION_MODE::PROXIMITY_SENSORS;
	tunnelSettings.lightMode = Tunnel::TUNNEL_LIGHT_MODE::AUTOMATIC_TRAIN_DETECTION;

	tunnelSettings.pin_mosfet_gate_lights = 9;

	tunnelSettings.pin_proximitySensor_entrance = 3;
	tunnelSettings.pin_proximitySensor_exit = 4;
	tunnelSettings.proximitySensorThresholdValue = 300;

	PhotoresistorSensor::PhotoresistorSensorSettings photoresistorSettings;
	photoresistorSettings.pin = A0;
	photoresistorSettings.eventMode = PhotoresistorSensor::PHOTORESISTOR_EVENT_MODE::AND_MODE;
	photoresistorSettings.lightDifferenceThreshold = 300;
	photoresistorSettings.readTimeInterval = 100;
	tunnelSettings.photoresistorSensorSettings_entrance = photoresistorSettings;

	photoresistorSettings.pin = A1;
	tunnelSettings.photoresistorSensorSettings_exit = photoresistorSettings;

	m_tunnel = new Tunnel(tunnelSettings);
	m_tunnel->RegisterTunnelObserver(this);
	m_tunnel->StartStopMonitoringTunnelEntrances(false);
}

void Train::Update()
{
	m_tunnel->Update();
	m_controlCenter->Update();
}

void Train::OnSwitchLightsOn()
{
	m_tunnel->TurnOnLights();
}

void Train::OnSwitchLightsOff()
{
	m_tunnel->TurnOffLights();
}

void Train::OnSwitchTunnelDetectionOn()
{
	m_tunnel->StartStopMonitoringTunnelEntrances(true);
}

void Train::OnSwitchTunnelDetectionOff()
{
	m_tunnel->StartStopMonitoringTunnelEntrances(false);
}

void Train::OnLightsOverrideChanged(bool areLightsOverridden)
{
	if (areLightsOverridden)
	{
		m_tunnel->ChangeSettings_TunnelLightMode(Tunnel::TUNNEL_LIGHT_MODE::MANUAL);
	}
	else
	{
		m_tunnel->ChangeSettings_TunnelLightMode(Tunnel::TUNNEL_LIGHT_MODE::AUTOMATIC_TRAIN_DETECTION);
	}
}

void Train::OnLightSensitivityChanged(unsigned short int lightDifferenceThreshold)
{
	unsigned short int difference = 0;
	if (lastLightDifferenceThresholdPrinted > lightDifferenceThreshold) difference = lastLightDifferenceThresholdPrinted - lightDifferenceThreshold;
	else difference = lightDifferenceThreshold - lastLightDifferenceThresholdPrinted;
	if (difference >= 50)
	{
		String str = "Tunnel settings - light sensitivity:\"" + String(lightDifferenceThreshold) + String("\"");
		Serial.println(str);
		lastLightDifferenceThresholdPrinted = lightDifferenceThreshold;
	}

	m_tunnel->ChangeSettings_PhotoresistorEntrance_LightDifferenceThreshold(lightDifferenceThreshold);
	m_tunnel->ChangeSettings_PhotoresistorExit_LightDifferenceThreshold(lightDifferenceThreshold);
}

void Train::OnChangeTrainDetectionMode()
{
	m_tunnel->ToggleSettings_TrainDetectionMode();
}

void Train::OnTunnelEvent(const Tunnel::TunnelEventInfo& eventInfo)
{
	// do nothing
}

Train::~Train()
{
}

////////////////////////////////////////////////////////////////////////////////
//// MAIN
////////////////////////////////////////////////////////////////////////////////

Train* k_train;

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.println("");
	Serial.println("--------------------");
	Serial.println("Starting program.");
	Serial.println("--------------------");

	k_train = new Train();
}

// the loop function runs over and over again forever
void loop() {
	k_train->Update();
}
}