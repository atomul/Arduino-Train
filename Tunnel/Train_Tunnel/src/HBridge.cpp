#include "HBridge.h"

#include <Arduino.h>

HBridgeMotorController::HBridgeMotorController()
	: m_direction(MOTOR_DIRECTION_FORWARD)
	, m_speed(0)
	, m_isEnabled(false)
	, m_speedChangeBehavior(SpeedChangeBehavior::LINEAR)
	, m_targetSpeed(0)
	, m_initialSpeed(0)
	, m_targetSpeedTimeInterval(1000)
	, m_changeSpeedAutomatically(false)
	, m_isIncreasingSpeed(false)
	, m_speedStep(0)
	, m_initialSpeedTimeStart(0)
{}

HBridgeMotorController::~HBridgeMotorController()
{}

void HBridgeMotorController::Update()
{
	if (IsEnabled())
	{
		if (m_changeSpeedAutomatically == true)
		{
			unsigned long currentTime = millis();
			unsigned long elapsedTime = currentTime - m_initialSpeedTimeStart;

			// clamp it for safety
			if (elapsedTime > m_targetSpeedTimeInterval)
			{
				elapsedTime = m_targetSpeedTimeInterval;
			}

			static unsigned long lastElapsedTime = 0;

			bool shouldPrint = false;
			if (elapsedTime - lastElapsedTime >= 0)
			{
				lastElapsedTime = elapsedTime;
				shouldPrint = true;
			}

			if (
				m_isIncreasingSpeed && m_speed < m_targetSpeed ||
				!m_isIncreasingSpeed && m_targetSpeed < m_speed)
			{		
				if (m_speedChangeBehavior == SpeedChangeBehavior::LINEAR)
				{
					uint8_t nextSpeed = map(elapsedTime, 0, m_targetSpeedTimeInterval, m_initialSpeed, m_targetSpeed);
					SetSpeed(nextSpeed);

					if (shouldPrint)
					{
						Serial.print(elapsedTime);
						Serial.print(" ");							
						Serial.print(0);
						Serial.print(" ");
						Serial.print(m_targetSpeedTimeInterval);
						Serial.print(" ");
						Serial.print(m_initialSpeed);
						Serial.print(" ");
						Serial.print(m_targetSpeed);
						Serial.print(" -> ");
						Serial.print(nextSpeed);
						Serial.println(" ");
						//Serial.println("==========================");
					}
				}
			}
		}

		if (m_direction == MOTOR_DIRECTION_FORWARD)
		{
			digitalWrite(m_hBridgeChannelSettings.input1, HIGH);
			digitalWrite(m_hBridgeChannelSettings.input2, LOW);
		}
		else
		{
			digitalWrite(m_hBridgeChannelSettings.input1, LOW);
			digitalWrite(m_hBridgeChannelSettings.input2, HIGH);
		}

		analogWrite(m_hBridgeChannelSettings.enable, m_speed);
		//Serial.println(m_speed);

		//Serial.print("Speed: ");
		//Serial.print(m_speed);
		//Serial.print("\n");
	}
}

void HBridgeMotorController::SetDirection(const MotorDirection direction) { m_direction = direction; }
void HBridgeMotorController::SetSpeed(const uint32_t speed) { m_speed = speed; }
void HBridgeMotorController::SetTargetSpeedPercentage(uint8_t speedPercentage, unsigned long timeInterval)
{
	Serial.print("SetTargetSpeedPercentage ");
	Serial.print(speedPercentage);
	Serial.print(" ");
	Serial.print(timeInterval);
	Serial.println(" ");
	
	m_targetSpeedTimeInterval = timeInterval;
	m_initialSpeed = m_speed;
	m_targetSpeed = map(speedPercentage, 0, 100, 0, 255);
	if (m_targetSpeed != m_speed)
	{
		m_initialSpeedTimeStart = millis();

		//m_targetSpeedPercentage = speedPercentage;
		m_changeSpeedAutomatically = true;
		//int m_speedDifferencePercentage = 0;
		if (m_speed < m_targetSpeed)
		{
			m_isIncreasingSpeed = true;
			//m_speedDifferencePercentage = m_targetSpeedPercentage - m_currentSpeedPercentage;
		}
		else
		{
			m_isIncreasingSpeed = false;
			//m_speedDifferencePercentage = m_currentSpeedPercentage - m_targetSpeedPercentage;
		}

		/*
		if (m_speedChangeBehavior == SpeedChangeBehavior::LINEAR)
		{
			m_speedStep =
		}
		//*/
	}
}
void HBridgeMotorController::SetChangeSpeedAutomatically(bool shouldChangeSpeedAutomatically)
{
	m_changeSpeedAutomatically = shouldChangeSpeedAutomatically;
}
uint8_t HBridgeMotorController::GetCurrentSpeedPercentage()
{
	// rounded instead of floored
	return (uint8_t)((m_speed * 100UL + 127) / 255);
}
bool HBridgeMotorController::IsEnabled() { return m_isEnabled; }

void HBridgeMotorController::Enable() { m_isEnabled = true; }

void HBridgeMotorController::Disable()
{
	analogWrite(m_hBridgeChannelSettings.enable, 0);
	Serial.print("SPEED DISABLED: ");
	Serial.println(m_speed);
	m_isEnabled = false;
}

void HBridgeMotorController::AssignHBridgeChannel(const HBridge_L293D::HBridge_L293D_Channel_Settings& channelSettings)
{
	m_hBridgeChannelSettings = channelSettings;
	m_hBridgeChannelSettings.SetupPins();
}

void HBridgeMotorController::MatchSpeed(const HBridgeMotorController& hBridge)
{
	m_direction = hBridge.m_direction;
	m_speed = hBridge.m_speed;
	m_isEnabled = hBridge.m_isEnabled;
	m_speedChangeBehavior = hBridge.m_speedChangeBehavior;
	m_changeSpeedAutomatically = hBridge.m_changeSpeedAutomatically;
	m_targetSpeedTimeInterval = hBridge.m_targetSpeedTimeInterval;
	m_initialSpeedTimeStart = hBridge.m_initialSpeedTimeStart;
	m_initialSpeed = hBridge.m_initialSpeed;
	m_targetSpeed = hBridge.m_targetSpeed;
	m_currentSpeed = hBridge.m_currentSpeed;
	m_isIncreasingSpeed = hBridge.m_isIncreasingSpeed;
	m_speedStep = hBridge.m_speedStep;
}
