#include "Button.h"

#include <Arduino.h>

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
