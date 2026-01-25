#pragma once

#ifndef UTILITIES.H
#define UTILITIES.H

#include <Arduino.h>

bool IsAnalogPin(uint8_t pin);
bool IsDigitalPin(uint8_t pin);
bool IsThresholdReached(int value1, int value2, int threshold);

//const uint8_t INVALID_PIN = 255;
static constexpr uint8_t INVALID_PIN = 0xFF;
static constexpr uint8_t INVALID_ID = 0xFF;
static constexpr uint8_t PIN_INVALID = 0xFF;

#endif