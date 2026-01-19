#include <Arduino.h>

#include "Button.h"

#include "macro-logger/MacroLogger.h"

Button::Button(uint8_t inputPin)
	: Button(inputPin, ButtonSettings())
{
}

Button::Button(uint8_t inputPin, const ButtonSettings& settings)
	: m_pin(inputPin)
	, m_holdThreshold(settings.holdThreshold)
	, m_timeBetweenClicksThreshold(settings.timeBetweenClicksThreshold)
	, m_pressState(0)
	, m_previousPressState(0)
	, m_initialHoldTime(0)
	, m_lastClick(0)
	, m_numberOfClicks(0)
	, m_needToSendClicks(false)
	, m_isSwitchedOn(false)
	, m_eventMask(BUTTON_EVENT_MASK::EVENT_ALL)
{	
}

Button::~Button()
{
}

void Button::Setup()
{
	pinMode(m_pin, INPUT);
}

void Button::SetSettings(const ButtonSettings& settings)
{
	m_holdThreshold = settings.holdThreshold;
	m_timeBetweenClicksThreshold = settings.timeBetweenClicksThreshold;
	m_eventMask = settings.enabledEvents;
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
				if (m_isHolding == false)
				{
					m_isHolding = true;
					SendEvent(ButtonEventInfo(BUTTON_EVENT_TYPE::BUTTON_HOLD));
				}

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
			//Serial.println("Button clicks: " + String(m_numberOfClicks) + " " + String(currentTime - m_lastClick) + " " + String(currentTime) + " " + String(m_lastClick));
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
	uint16_t mask = EventTypeToMask(buttonEventInfo.m_buttonEventType);
	//LOG_TRACE("\"%d\" - \"%d\" - \"%d\"", (uint16_t)buttonEventInfo.m_buttonEventType, m_eventMask, ((uint16_t)buttonEventInfo.m_buttonEventType & m_eventMask));
	if ((m_eventMask & mask) == 0)
	{
		return;
	}

	//if (m_buttonObserver)
	if (m_handler)
	{
		{
			//m_buttonObserver->OnButtonEvent(m_pin, buttonEventInfo);
			m_handler(m_context, m_pin, buttonEventInfo);
		}
	}
}

/*
void Button::RegisterButtonObserver(IButtonObserver* observer)
{
	m_buttonObserver = observer;
}
//*/

void Button::SetListener(ButtonHandler handler, void* context)
{
	m_handler = handler;
	m_context = context;
}

bool Button::IsPressed() { return m_pressState; }
bool Button::WasPressed() { return m_previousPressState; }

const __FlashStringHelper* ButtonEventTypeToText(BUTTON_EVENT_TYPE type)
{
	switch (type)
	{
	case BUTTON_EVENT_TYPE::BUTTON_CLICKS:       return F("BUTTON_CLICKS");
	case BUTTON_EVENT_TYPE::BUTTON_DOWN:         return F("BUTTON_DOWN");
	case BUTTON_EVENT_TYPE::BUTTON_UP:           return F("BUTTON_UP");
	case BUTTON_EVENT_TYPE::BUTTON_HOLD:         return F("BUTTON_HOLD");
	case BUTTON_EVENT_TYPE::BUTTON_HOLD_RELEASE: return F("BUTTON_HOLD_RELEASE");
	case BUTTON_EVENT_TYPE::BUTTON_SWITCH:       return F("BUTTON_SWITCH");
	default:                                     return F("Unknown");
	}
}

/*
void PrintButtonEventType(BUTTON_EVENT_TYPE type)
{
	switch (type)
	{
	case BUTTON_EVENT_TYPE::BUTTON_CLICKS:
		Serial.print(F("BUTTON_CLICKS"));
		break;
	case BUTTON_EVENT_TYPE::BUTTON_DOWN:
		Serial.print(F("BUTTON_DOWN"));
		break;
	case BUTTON_EVENT_TYPE::BUTTON_UP:
		Serial.print(F("BUTTON_UP"));
		break;
	case BUTTON_EVENT_TYPE::BUTTON_HOLD:
		Serial.print(F("BUTTON_HOLD"));
		break;
	case BUTTON_EVENT_TYPE::BUTTON_HOLD_RELEASE:
		Serial.print(F("BUTTON_HOLD_RELEASE"));
		break;
	case BUTTON_EVENT_TYPE::BUTTON_SWITCH:
		Serial.print(F("BUTTON_SWITCH"));
		break;
	default:
		Serial.print(F("Unknown"));
		break;
	}
}
//*/

/*
String ButtonEventTypeToString(const BUTTON_EVENT_TYPE& type)
{
	switch (type)
	{
	case BUTTON_EVENT_TYPE::BUTTON_CLICKS:
		return "BUTTON_CLICKS";
	case BUTTON_EVENT_TYPE::BUTTON_DOWN:
		return "BUTTON_DOWN";
	case BUTTON_EVENT_TYPE::BUTTON_UP:
		return "BUTTON_UP";
	case BUTTON_EVENT_TYPE::BUTTON_HOLD:
		return "BUTTON_HOLD";
	case BUTTON_EVENT_TYPE::BUTTON_HOLD_RELEASE:
		return "BUTTON_HOLD_RELEASE";
	case BUTTON_EVENT_TYPE::BUTTON_SWITCH:
		return "BUTTON_SWITCH";
	default:
		return "Unknown";
	}
}
//*/