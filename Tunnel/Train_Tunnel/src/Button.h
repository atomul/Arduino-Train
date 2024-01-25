#pragma once

#include <stdint.h>

class Button;
class SwitchButton;

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

	uint8_t GetPin() { return m_pin; }

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

