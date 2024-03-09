#include <stdio.h>
#include "ntp.c"
#include "delay.c"
#include "setup.c"
#include <stdlib.h>

int main() {
	setUp();
	char* NTP_res = NTP();
	if (NTP_res == NULL) { // Checks for NTP connection
		return 1;     // returns 1 if can't establish connection
	} else {
		printf("Connected to NTP server! \nWaiting for 10 seconds\n");
	}
	customDelay(10);
	
	printf("%s\n", NTP_res);
	free(NTP_res);
	
	return 0;
}
