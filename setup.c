#include <wiringPi.h>

// https://pinout.xyz/pinout/wiringpi

int setUp() {
	wiringPiSetupGpio();
	pinMode(0, OUTPUT);
	pinMode(1, INPUT);
	
}
