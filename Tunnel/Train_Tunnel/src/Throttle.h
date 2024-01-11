#pragma once

#include <stdint.h>

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