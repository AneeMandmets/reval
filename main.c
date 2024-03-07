#include <stdio.h>
#include "ntp.c"
#include "delay.c"

int main() {
	if (NTP() != 0) { // Checks for NTP connection
		return 1;     // returns 1 if can't establish connection
	}
	customDelay(10);
	printf("10 seconds have passed!\n");
	NTP();
	
	return 0;
}
