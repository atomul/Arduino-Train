#include "Throttle.h"

#include <Arduino.h>

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

void Throttle::RegisterObserver(IThrottleObserver * observer)
{
	m_observer = observer;
}
