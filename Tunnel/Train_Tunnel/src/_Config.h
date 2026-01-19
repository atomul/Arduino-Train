#pragma once

#include <Arduino.h>

#include <stdint.h>

#include "Utilities.h"

namespace config
{
	namespace control_center
	{
		//constexpr uint8_t PIN_BUTTON_TEST = 13;
		constexpr uint8_t PIN_BUTTON_TEST = 8;
	}

	//// Tunnel

	namespace tunnel
	{
		constexpr uint8_t PIN_LIGHTS_MOSFET = 8;
		constexpr uint8_t PIN_PROXIMITY_SENSOR_ENTRANCE = 3;
		constexpr uint8_t PIN_PROXIMITY_SENSOR_EXIT = 4;

		constexpr unsigned int PROXIMITY_SENSOR_THRESHOLD_VALUE = 300;
	};

	namespace track_sections
	{
		constexpr unsigned int PROXIMITY_SENSOR_THRESHOLD_VALUE = 300;

		constexpr uint8_t MAX_TRACK_SECTIONS = 4;
		constexpr uint8_t MAX_TRACK_SECTIONS_CHECKPOINTS = 3;
		constexpr uint8_t MAX_TRAINS = 2;

		constexpr uint8_t NUMBER_OF_TRACK_SECTIONS = 3;

		constexpr uint8_t PIN_RELAY_1X = 11;
		constexpr uint8_t PIN_RELAY_X2 = 12;

		/////////////////////////
		//// Track Section 1 ////
		/////////////////////////

		constexpr uint8_t PIN_TRACK_SECTION_1_H_BRIDGE_ENABLE = 9;
		constexpr uint8_t PIN_TRACK_SECTION_1_H_BRIDGE_INPUT_1 = 6;
		constexpr uint8_t PIN_TRACK_SECTION_1_H_BRIDGE_INPUT_2 = 7;
		
		//constexpr uint8_t PIN_TRACK_SECTION_1_PROXIMITY_SENSOR_ENTRANCE = A2;
		//constexpr uint8_t PIN_TRACK_SECTION_1_PROXIMITY_SENSOR_EXIT = A3;

		//constexpr uint8_t PIN_TRACK_SECTION_1_PROXIMITY_SENSOR_ENTRANCE = 2;
		//constexpr uint8_t PIN_TRACK_SECTION_1_PROXIMITY_SENSOR_EXIT = 13;

		constexpr uint8_t PIN_TRACK_SECTION_1_PROXIMITY_SENSOR_ENTRANCE = 3;
		constexpr uint8_t PIN_TRACK_SECTION_1_PROXIMITY_SENSOR_EXIT = 4;

		constexpr uint8_t NUMBER_OF_CHECKPOINTS_TRACK_SECTION_1 = 3;
		constexpr uint8_t PINS_TRACK_SECTION_1_PROXIMITY_SENSOR_CHECKPOINTS[NUMBER_OF_CHECKPOINTS_TRACK_SECTION_1] = {
			2, 5, 13
		};
		
		/////////////////////////
		//// Track Section 2 ////
		/////////////////////////

		constexpr uint8_t PIN_TRACK_SECTION_2_PROXIMITY_SENSOR_ENTRANCE = 2;
		constexpr uint8_t PIN_TRACK_SECTION_2_PROXIMITY_SENSOR_EXIT = 5;

		constexpr uint8_t NUMBER_OF_CHECKPOINTS_TRACK_SECTION_2 = 1;
		constexpr uint8_t PINS_TRACK_SECTION_2_PROXIMITY_SENSOR_CHECKPOINTS[NUMBER_OF_CHECKPOINTS_TRACK_SECTION_2] = {
			INVALID_PIN
		};

		/////////////////////////
		//// Track Section 3 ////
		/////////////////////////

		constexpr uint8_t PIN_TRACK_SECTION_3_H_BRIDGE_ENABLE = 10;
		constexpr uint8_t PIN_TRACK_SECTION_3_H_BRIDGE_INPUT_1 = A0;
		constexpr uint8_t PIN_TRACK_SECTION_3_H_BRIDGE_INPUT_2 = A1;

		constexpr uint8_t PIN_TRACK_SECTION_3_PROXIMITY_SENSOR_ENTRANCE = A4;
		constexpr uint8_t PIN_TRACK_SECTION_3_PROXIMITY_SENSOR_EXIT = A5;

		constexpr uint8_t NUMBER_OF_CHECKPOINTS_TRACK_SECTION_3 = 1;
		constexpr uint8_t PINS_TRACK_SECTION_3_PROXIMITY_SENSOR_CHECKPOINTS[NUMBER_OF_CHECKPOINTS_TRACK_SECTION_3] = {
			INVALID_PIN
		};

		/* MORE ARDUINO PINS
		MCP23S17

		MCP23017 (I�C, 16 GPIO) gives:
		  - Reliable silicon (Microchip)
		  - True GPIO (inputs + outputs)
		  - Interrupts
		  - Easy addressing (up to 8 chips = 128 pins)
		  - Simple libraries
		  - Plenty fast for relays, blocks, signals

		For:
		  -	Relays
		  - Signals
		  - Buttons
		  - Sensors
		    -> It s already overkill

		So �better� usually means:
		  - More pins per chip
		  - Faster bus
		  - Higher reliability / industrial
		  - Lower latency
		  - Less software overhead

		////////////////////////////////////////

		Option 1: MCP23S17 (SPI version of MCP23017)

		Better speed
		More deterministic timing

		Why it�s better
		- Uses SPI instead of I�C
		- 10�20� faster than I�C
		- No bus collisions
		- Better for time-critical IO

		Trade-offs
		- Uses more Arduino pins (CS per chip)
		- Slightly more wiring
		- Still 16 pins per chip

		When it�s worth it
		- Many fast-changing inputs
		- Synchronized outputs
		- Larger layouts with heavy IO traffic
		
		This is the most natural �upgrade� from MCP23017

		//*/


	};

	// https://www.optimusdigital.ro/ro/kituri/12264-set-de-fire-cu-izolaie-pvc-22awg-6-culori-10m-fiecare-0721248989765.html
	// https://www.bitmi.ro/electronica/breadboard-400-puncte-pentru-montaje-electronice-rapide-10633.html#addToWishlistPopup_1062
}