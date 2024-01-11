/*
#include "SwitchButton.h"

#include <Arduino.h>

SwitchButton::SwitchButton(uint8_t inputPin)
	: Button(inputPin)
	, m_isSwitchedOn(false)
{
}

SwitchButton::SwitchButton(uint8_t inputPin, unsigned long holdThreshold)
	: Button(inputPin, holdThreshold)
	, m_isSwitchedOn(false)
{

}

SwitchButton::~SwitchButton()
{
}

void SwitchButton::RegisterSwitchObserver(ISwitchButtonObserver * observer)
{
	m_switchButtonObserver = observer;
}

bool SwitchButton::IsSwitchedOn()
{
	return m_isSwitchedOn;
}

void SwitchButton::OnButtonClicksEvent()
{
	Button::OnButtonClicksEvent();
	
	return;
	if (m_numberOfClicks != 1)
	{
		return;
	}

	m_isSwitchedOn = !m_isSwitchedOn;

	if (m_switchButtonObserver)
	{
		if (IsSwitchedOn())
		{
			m_switchButtonObserver->OnSwitchOn(m_pin);
		}
		else
		{
			m_switchButtonObserver->OnSwitchOff(m_pin);
		}
	}
}
//*/