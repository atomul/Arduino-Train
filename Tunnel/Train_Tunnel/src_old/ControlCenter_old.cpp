#include <Arduino.h>

#include "ControlCenter_old.h"
#include "Button.h"

const uint8_t k_pin_button_settings = 2;
uint16_t k_button_enable_holdThreshold = 3000;

//const uint8_t k_pin_throttle_light_sensitivity = A2; // decomissioned
//const uint8_t k_throttle_threshold = 2;

const uint8_t k_pin_button_toggle_tunnel_detection = 5;

const uint8_t k_pin_button_test = 13;

bool IsButtonEnable(uint8_t buttonPin)
{
	return (k_pin_button_settings == buttonPin);
}

ControlCenter::ControlCenter()
{
	m_areLightsOverridden = false;
	m_isTunnelDetectionOn = false;

	m_buttonSettings = new Button(k_pin_button_settings);

	Serial.println(F("ControlCenter new btn settings"));
	Serial.flush();

	delay(1000);

	//m_buttonEnableTunnelDetection = new Button(k_pin_button_toggle_tunnel_detection, 60000);

	Serial.println(F("ControlCenter new btn tunnel detection"));
	Serial.flush();

	delay(1000);
	//m_buttonTest = new Button(k_pin_button_test, 60000);

	Serial.println(F("ControlCenter new btn test"));
	Serial.flush();
	delay(1000);

	//m_throttle = new Throttle(k_pin_throttle_light_sensitivity, k_throttle_threshold);
	
	
}

void ControlCenter::Setup()
{
	m_buttonSettings->RegisterButtonObserver(this);
	/*
	m_buttonEnableTunnelDetection->RegisterButtonObserver(this);
	m_buttonTest->RegisterButtonObserver(this);
	//*/

	//m_throttle->RegisterObserver(this);
}

ControlCenter::~ControlCenter()
{
	delete m_buttonSettings;
	m_buttonSettings = NULL;

	delete m_buttonEnableTunnelDetection;
	m_buttonEnableTunnelDetection = NULL;

	delete m_buttonTest;
	m_buttonTest = NULL;

	//delete m_throttle;
	//m_throttle = NULL;
}

void ControlCenter::Update()
{
	m_buttonSettings->Update();
	//m_buttonEnableTunnelDetection->Update();
	//m_buttonTest->Update();
	//m_throttle->Update();
}

void ControlCenter::RegisterObserver(IControlCenterObserver * observer)
{
	m_observer = observer;
}

void ControlCenter::OnButtonEvent(uint32_t buttonId, const ButtonEventInfo& buttonEventInfo)
{
	return;
	//Serial.println("Button event: " + ButtonEventTypeToString(buttonEventInfo.m_buttonEventType) + " ID: " + (int)buttonId + " clicks: " + (int)buttonEventInfo.m_numberOfClicks);

	Serial.print(F("Button event: "));
	PrintButtonEventType(buttonEventInfo.m_buttonEventType);
	Serial.print(F(" ID: "));
	Serial.print(buttonId);
	Serial.print(F(" clicks: "));
	Serial.println(buttonEventInfo.m_numberOfClicks);

	switch (buttonEventInfo.m_buttonEventType)
	{
		case BUTTON_EVENT_TYPE::BUTTON_SWITCH:
		{
			if (buttonId == m_buttonSettings->GetPin())
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
			else if (buttonId == m_buttonEnableTunnelDetection->GetPin())
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
			if (buttonId == m_buttonTest->GetPin())
			{
				if (buttonEventInfo.m_numberOfClicks == 1)
				{
					m_observer->OnTestButtonClick();
				}
			}

			else if (buttonId == m_buttonSettings->GetPin())
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

/*
void ControlCenter::OnThrottleChange(uint32_t value, uint32_t throttleId)
{
	if (throttleId == k_pin_throttle_light_sensitivity)
	{
		m_observer->OnLightSensitivityChanged(value);
	}
}
//*/