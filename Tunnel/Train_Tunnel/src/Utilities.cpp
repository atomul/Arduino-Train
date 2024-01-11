#include "Utilities.h"

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