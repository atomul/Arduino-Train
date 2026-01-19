#include "Utilities.h"
#include <string.h>

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

bool IsThresholdReached(int value1, int value2, int threshold)
{
	int difference = 0;
	if (value1 > value2) difference = value1 - value2;
	else difference = value2 - value1;
	if (difference >= threshold)
	{
		return true;
	}

	return false;
}