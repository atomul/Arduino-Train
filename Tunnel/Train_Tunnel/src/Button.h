#pragma once

#include <Arduino.h>

#include <stdint.h>

class Button;
class SwitchButton;
class String;

enum class BUTTON_EVENT_TYPE : uint16_t
{
	BUTTON_CLICKS,
	BUTTON_DOWN,
	BUTTON_UP,
	BUTTON_HOLD,
	BUTTON_HOLD_RELEASE,
	BUTTON_SWITCH

	//BUTTON_ALL = BUTTON_CLICKS | BUTTON_DOWN | BUTTON_UP | BUTTON_HOLD | BUTTON_HOLD_RELEASE | BUTTON_SWITCH
};

enum BUTTON_EVENT_MASK : uint16_t {
	EVENT_NONE = 0,

	EVENT_CLICKS = 1 << 0,
	EVENT_DOWN = 1 << 1,
	EVENT_UP = 1 << 2,
	EVENT_HOLD = 1 << 3,
	EVENT_HOLD_RELEASE = 1 << 4,
	EVENT_SWITCH = 1 << 5,

	EVENT_ALL = EVENT_CLICKS | EVENT_DOWN | EVENT_UP | EVENT_HOLD | EVENT_HOLD_RELEASE | EVENT_SWITCH
	//EVENT_ALL = 0xFFFF
};

static uint16_t EventTypeToMask(BUTTON_EVENT_TYPE type)
{
	switch (type)
	{
	case BUTTON_EVENT_TYPE::BUTTON_CLICKS:       return EVENT_CLICKS;
	case BUTTON_EVENT_TYPE::BUTTON_DOWN:         return EVENT_DOWN;
	case BUTTON_EVENT_TYPE::BUTTON_UP:           return EVENT_UP;
	case BUTTON_EVENT_TYPE::BUTTON_HOLD:         return EVENT_HOLD;
	case BUTTON_EVENT_TYPE::BUTTON_HOLD_RELEASE: return EVENT_HOLD_RELEASE;
	case BUTTON_EVENT_TYPE::BUTTON_SWITCH:       return EVENT_SWITCH;
	default:                                     return EVENT_NONE;
	}
}

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

const __FlashStringHelper* ButtonEventTypeToText(BUTTON_EVENT_TYPE type);
//void PrintButtonEventType(BUTTON_EVENT_TYPE type);
//String ButtonEventTypeToString(const BUTTON_EVENT_TYPE& type);

/*
class IButtonObserver
{
	friend Button;
public:
	IButtonObserver() {};

private:
	virtual void OnButtonEvent(uint32_t buttonId, const ButtonEventInfo& buttonEventInfo) = 0;
};
//*/

using ButtonHandler = void(*)(void* context, uint8_t buttonId, const ButtonEventInfo& buttonEventInfo);

struct ButtonSettings
{
	ButtonSettings()
		: holdThreshold(3000)
		, timeBetweenClicksThreshold(500)
		, enabledEvents(BUTTON_EVENT_MASK::EVENT_ALL)
	{
	}

	uint16_t holdThreshold;
	uint16_t timeBetweenClicksThreshold;
	uint16_t enabledEvents;
};

class Button
{
public:
	Button(uint8_t inputPin);
	Button(uint8_t inputPin, const ButtonSettings& settings);

	~Button();

	void Setup();

	uint8_t GetPin() { return m_pin; }

	void SetSettings(const ButtonSettings& settings);

	void Update();

	bool IsPressed();
	bool WasPressed();

	//void RegisterButtonObserver(IButtonObserver* observer);
	
	void SetListener(ButtonHandler handler, void* context);

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

	uint16_t m_holdThreshold;

	unsigned long m_initialHoldTime;
	bool m_isHolding = false;

	unsigned long m_lastClick;
	
	uint16_t m_timeBetweenClicksThreshold;
	uint8_t m_numberOfClicks;
	bool m_needToSendClicks;

	uint16_t m_eventMask;

	//IButtonObserver* m_buttonObserver;

	bool m_isSwitchedOn;

	void* m_context = nullptr;
	ButtonHandler m_handler = nullptr;
};

