#pragma once 

#include <stdint.h>
#include <Arduino.h>


class HBridge_L293D
{
public:
	struct HBridge_L293D_Channel_Settings
	{
		HBridge_L293D_Channel_Settings()
			: enable(-1)
			, input1(-1)
			, input2(-1)
		{
		}

		HBridge_L293D_Channel_Settings(uint8_t enable, uint8_t input1, uint8_t input2)
			: enable(enable)
			, input1(input1)
			, input2(input2)
		{
		}

		void SetupPins()
		{
			pinMode(enable, OUTPUT);
			pinMode(input1, OUTPUT);
			pinMode(input2, OUTPUT);
			Serial.print(enable);
			Serial.print(" ");
			Serial.print(input1);
			Serial.print(" ");
			Serial.print(input2);
			Serial.println(" ");
		}

		// used to PWM the motor, change the speed
		uint8_t enable; //pin 1 9

		/**
		* Used to change direction of the current
		* These pins should be set as oposite from each other
		* If both pins are HIGH or LOW, a motor hooked up to this will stop spinning
		*/
		uint8_t input1; //pin 2 10
		uint8_t input2; //pin 7 15
	};

	/**
	 *                _______
	 *  Enable 1  1 -|  |u|  |- 16 Vcc 1
	 *  Input A1  2 -|       |- 15 Input B1
	 * Output A1  3 -|       |- 14 Output B1
	 *       GND  4 -|       |- 13 GND
	 *       GND  5 -|       |- 12 GND
	 * Output A2  6 -|       |- 11 Output B2
	 *  Input A2  7 -|       |- 10 Input B2
	 *     Vcc 2  8 -|__|o|__|-  9 Enable 2
	 *
	 *  Vcc 1 = this is where the drive gets it's power. Connect to 5V board.
	 *  Vcc 2 = this is where the driver gets the external power that will be outputted to the motors. Connect the battery here.
	 */

	HBridge_L293D() {};
	~HBridge_L293D() {};

	void SetChannelSettings_Channel1(const HBridge_L293D::HBridge_L293D_Channel_Settings& channelSettings)
	{
		m_channelSettings_1 = channelSettings;
	}
	void SetChannelSettings_Channel2(const HBridge_L293D::HBridge_L293D_Channel_Settings& channelSettings)
	{
		m_channelSettings_2 = channelSettings;
	}

	HBridge_L293D::HBridge_L293D_Channel_Settings& GetChannelSettings_Channel1(const HBridge_L293D::HBridge_L293D_Channel_Settings& device1_pinSettigns) { return m_channelSettings_1; }
	HBridge_L293D::HBridge_L293D_Channel_Settings& GetChannelSettings_Channel2(const HBridge_L293D::HBridge_L293D_Channel_Settings& device1_pinSettigns) { return m_channelSettings_2; }
	
	HBridge_L293D_Channel_Settings m_channelSettings_1;
	HBridge_L293D_Channel_Settings m_channelSettings_2;
};

#pragma once

#include <stdint.h>

#include "HBridge.h"

class HBridgeMotorController
{
public:
	enum MotorDirection
	{
		MOTOR_DIRECTION_FORWARD,
		MOTOR_DIRECTION_BACKWARD
	};

	enum SpeedChangeBehavior
	{
		LINEAR,
		LOGARITMIC
	};

public:
	HBridgeMotorController();
	~HBridgeMotorController();

	void Update();

	void SetDirection(const MotorDirection direction);
	void SetSpeed(const uint32_t m_motorSpeed);

	void SetTargetSpeedPercentage(uint8_t speedPercentage, unsigned long timeInterval);
	void SetChangeSpeedAutomatically(bool shouldChangeSpeedAutomatically);
	
	uint8_t GetCurrentSpeedPercentage();

	bool IsEnabled();

	void Enable();
	void Disable();

	// ToDo 2PD: Create a struct for H-Bridge
	void AssignHBridgeChannel(const HBridge_L293D::HBridge_L293D_Channel_Settings& channelSettings);

	void MatchSpeed(const HBridgeMotorController& hBridge);

private:
	MotorDirection m_direction;
	uint32_t m_speed;
	bool m_isEnabled;
	
	SpeedChangeBehavior m_speedChangeBehavior;
	//uint8_t m_speedStepPercentage;

	bool m_changeSpeedAutomatically;
	unsigned int m_targetSpeedTimeInterval; //miliseconds
	unsigned int m_initialSpeedTimeStart;
	uint8_t m_initialSpeed;
	uint8_t m_targetSpeed;
	uint8_t m_currentSpeed;
	bool m_isIncreasingSpeed;
	uint8_t m_speedStep;

	HBridge_L293D::HBridge_L293D_Channel_Settings m_hBridgeChannelSettings;
};