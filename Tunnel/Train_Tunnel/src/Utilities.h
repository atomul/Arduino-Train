#pragma once

#ifndef UTILITIES.H
#define UTILITIES.H

#include <Arduino.h>

bool IsAnalogPin(uint8_t pin);
bool IsDigitalPin(uint8_t pin);
bool IsThresholdReached(int value1, int value2, int threshold);

#endif