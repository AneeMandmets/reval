#include <wiringPi.h>

// https://pinout.xyz/pinout/wiringpi

int setUp() {
	wiringPiSetupGpio();
	pinMode(0, OUTPUT); // LED pin
	pinMode(1, INPUT);  // Sensor pin
	
}
