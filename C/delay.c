#include <stdio.h>
#include <time.h>

void customDelay (int seconds) {
	int stopTime = time(0) + seconds;
	while (time(0) < stopTime);
}
