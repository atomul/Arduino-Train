#pragma once 

#include <stdint.h>
#include <Arduino.h>
#include "Utilities.h"

class MotorController
{
	class MotorChannel
	{
	public:
		void SetTargetSpeedPercentage(uint8_t channelIndex, uint8_t speedPercentage, unsigned long timeInterval = 0);
	
	private:
		friend class MotorController;
		MotorChannel(MotorController& owner, uint8_t channelId);

	private:
		MotorController& m_owner; // this owns the MotorController
		uint8_t m_channelId; // this is unique in the context of this Motor Controller, not unique across all motor channels
	};

	static constexpr uint8_t MAX_CHANNELS = 2;

	struct DriverSettings
	{
		// low power mode
		// used by TB6612FNG
		uint8_t pin_standby = UNUSED;

		// TB6612FNG activates when STBY is high
		bool active_when_standby_high = true;
	};

	struct ChannelSettings
	{		
		// PWN pin
		// L293D - enable pin
		// TB6612FNG -  PWMA, PWMB
		uint8_t pin_speed = UNUSED;

		/**
		* Used to change:, coast or break
		* - direction: one pin HIGH, the other LOW
		* - coast: both pins LOW
		* - break: both pins HIGH
		*/
		uint8_t pin_in1 = UNUSED;
		uint8_t pin_in2 = UNUSED;
	};

	enum class StopMode : uint8_t
	{
		Coast,
		Brake
	};

	enum class Direction
	{
		Forward,
		Reverse
	};

	enum class SpeedChangeAlgorithm
	{
		Linear,
		Logarithmic,
		Exponential,
	};

	struct MotorControllerSettings
	{
		ChannelSettings channels[MAX_CHANNELS];
		DriverSettings driver;
		uint8_t noChannels = 0;
		Direction direction = Direction::Forward;
		SpeedChangeAlgorithm speedChangeAlgorithm = SpeedChangeAlgorithm::Linear;
	};

	struct ChannelRuntime
	{
		uint8_t currentSpeed = 0;
		uint8_t initialSpeed = 0;
		uint8_t targetSpeed = 0;

		uint32_t startTimeMs = 0;
		uint32_t targetTimeDurationMs = 0;

		bool isIncreasingSpeed = false;

		Direction direction = Direction::Forward;
		StopMode stopMode = StopMode::Coast;
	};

public:

	MotorController();

	void Update();

	void SetTargetSpeedPercentage(uint8_t channelIndex, uint8_t speedPercentage, unsigned long timeInterval = 0);


private:

	MotorControllerSettings m_settings;
	ChannelRuntime m_channelRuntime[MAX_CHANNELS];
};