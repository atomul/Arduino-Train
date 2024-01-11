#include <Arduino.h>

#include "ControlCenter.h"
#include "Button.h"

const uint8_t k_pin_button_enable = 2;
const unsigned long k_button_enable_holdThreshold = 3000;

const uint8_t k_pin_throttle_light_sensitivity = A2;
const uint8_t k_throttle_threshold = 2;

bool IsButtonEnable(uint8_t buttonPin)
{
	return (k_pin_button_enable == buttonPin);
}

ControlCenter::ControlCenter()
{
	m_areLightsOverridden = false;

	m_button = new Button(k_pin_button_enable, k_button_enable_holdThreshold);
	m_throttle = new Throttle(k_pin_throttle_light_sensitivity, k_throttle_threshold);
	
	m_button->RegisterButtonObserver(this);
	m_throttle->RegisterObserver(this);
}

ControlCenter::~ControlCenter()
{
	delete m_button;
	m_button = NULL;

	delete m_throttle;
	m_throttle = NULL;
}

void ControlCenter::Update()
{
	m_button->Update();
	m_throttle->Update();
}

void ControlCenter::RegisterObserver(IControlCenterObserver * observer)
{
	m_observer = observer;
}

void ControlCenter::OnButtonEvent(uint32_t buttonId, const ButtonEventInfo& buttonEventInfo)
{
	switch (buttonEventInfo.m_buttonEventType)
	{
		case BUTTON_EVENT_TYPE::BUTTON_SWITCH:
		{
			if (buttonId == k_pin_button_enable)
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
			break;
		}
		case BUTTON_EVENT_TYPE::BUTTON_CLICKS:
		{
			if (buttonId == k_pin_button_enable)
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
			if (buttonId == k_pin_button_enable)
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