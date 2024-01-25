/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the Uno and
  Leonardo, it is attached to digital pin 13. If you're unsure what
  pin the on-board LED is connected to on your Arduino model, check
  the documentation at http://arduino.cc

  This example code is in the public domain.

  modified 8 May 2014
  by Scott Fitzgerald
 */

//#include <Servo.h>
//#include "src/Tank.h"
#include "src/Train.h"
#include "src/Online.h"

//Tank* k_tank;
Train* k_train;

Online::Train* k_train2 = new Online::Train();

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.println("");
	Serial.println("--------------------");
	Serial.println("Starting program.");
	Serial.println("--------------------");

	//k_tank = new Tank();
	k_train = new Train();

	k_train2 = new Online::Train();
}

// the loop function runs over and over again forever
void loop() {
	//k_tank->Update();
	k_train->Update();

	k_train2->Update();
}