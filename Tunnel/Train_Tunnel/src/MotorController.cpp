#include "MotorController.h"

#include <Arduino.h>

#include "macro-logger/MacroLogger.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

MotorController::MotorController()
{
	// TODO: initalize m_channelRuntime
}

////////////////////////////////////////////////////////////////////////////////

void MotorController::SetTargetSpeedPercentage(uint8_t channelIndex, uint8_t speedPercentage, unsigned long timeInterval)
{
	if (channelIndex >= m_settings.noChannels)
	{
		return;
	}

	if (speedPercentage > 100) { speedPercentage = 100; }
	else if (speedPercentage < 0) { speedPercentage = 0; }

	ChannelRuntime& currentChannelRuntime = m_channelRuntime[channelIndex];
	currentChannelRuntime.initialSpeed = currentChannelRuntime.currentSpeed;
	currentChannelRuntime.targetSpeed = map(speedPercentage, 0, 100, 0, 255);
	currentChannelRuntime.startTimeMs = millis();
	currentChannelRuntime.targetTimeDurationMs = timeInterval;
	
	if (currentChannelRuntime.currentSpeed != currentChannelRuntime.targetSpeed)
	{
		if (currentChannelRuntime.currentSpeed < currentChannelRuntime.targetSpeed)
		{
			currentChannelRuntime.isIncreasingSpeed = true;
		}
		else
		{
			currentChannelRuntime.isIncreasingSpeed = true;
		}
	}
	else
	{
		// ???
	}
	//currentChannelRuntime.stopMode = ? 
	//currentChannelRuntime.currentSpeed = ? 
	
	//if (currentChannelRuntime.timeDurationMs != 0)

	m_settings.channels[channelIndex].pin_in1 = 1;
}

////////////////////////////////////////////////////////////////////////////////

void MotorController::Update()
{
	uint32_t currentTime = millis();

	uint32_t debugPrintLastElapsedTime = 0;
	bool shouldDebugPrint = false;

	if (debugPrintLastElapsedTime - currentTime >= 200)
	{
		shouldDebugPrint = true;
		debugPrintLastElapsedTime = currentTime;
	}

	for (int i = 0; i < m_settings.noChannels; i++)
	{
		ChannelRuntime& currentChannelRuntime = m_channelRuntime[i];

		uint32_t elapsedTime = currentTime - currentChannelRuntime.startTimeMs;
		
		// clamp
		if (elapsedTime > currentChannelRuntime.targetTimeDurationMs)
		{
			elapsedTime = currentChannelRuntime.targetTimeDurationMs;
		}

		if (
			(
				currentChannelRuntime.isIncreasingSpeed == true && 
				currentChannelRuntime.currentSpeed < currentChannelRuntime.targetSpeed
				) ||
			(
				currentChannelRuntime.isIncreasingSpeed == false &&
				currentChannelRuntime.currentSpeed > currentChannelRuntime.targetSpeed
				)
			)
		{
			if (m_settings.speedChangeAlgorithm == SpeedChangeAlgorithm::Linear)
			{
				uint8_t nextSpeed =
					map(
						elapsedTime,
						0,
						currentChannelRuntime.targetTimeDurationMs,
						currentChannelRuntime.initialSpeed,
						currentChannelRuntime.targetSpeed
					);

				currentChannelRuntime.currentSpeed = nextSpeed;

				if (shouldDebugPrint)
				{

				}
			}

			analogWrite(m_settings.channels[i].pin_speed, currentChannelRuntime.currentSpeed);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

MotorController::MotorChannel::MotorChannel(MotorController& owner, uint8_t channelId)
	: m_owner(owner)
	, m_channelId(channelId)
{
}

void MotorController::MotorChannel::SetTargetSpeedPercentage(uint8_t channelIndex, uint8_t speedPercentage, unsigned long timeInterval)
{
	m_owner.SetTargetSpeedPercentage(m_channelId, speedPercentage, timeInterval);
}
