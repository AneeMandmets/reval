#include <stdio.h>
#include "ntp.c"
#include "delay.c"
#include "setup.c"
#include <stdlib.h>
#include <wiringPi.h>
#include <pthread.h>

void *blinking(void* i) {
	int *n = (int*)i;
	FILE *blinkingTimes;
	char* ntpTime;
	blinkingTimes = fopen("blinking.txt", "w");
	for (int j = *n; j > 0; j--) {
		ntpTime = NTP();
		fprintf(blinkingTimes, "%s\n", ntpTime);
		digitalWrite(17, HIGH);
		customDelay(1);
		digitalWrite(17, LOW);
		customDelay(1);
	}
	fclose(blinkingTimes);
	pthread_exit(NULL);
}

void *sensor(void* i){
	int *n = (int*)i;
	FILE *sensorTimes;
	char* ntpTime;
	sensorTimes = fopen("sensor.txt", "w");
	//printf("%d\n", &n);
	int j = *n;
	while(j > 0){
		if(analogRead(18) < 512){
			printf("%d HIGH\n", j);
			ntpTime = NTP();
			fprintf(sensorTimes, "%s\n", ntpTime);
			customDelay(2);
			j--;
		}	
	}
	fclose(sensorTimes);
	pthread_exit(NULL);
}


int main() {
	setUp();
	pthread_t idLED, idSensor;
	char* NTP_res = NTP();
	if (NTP_res == NULL) { // Checks for NTP connection
		return 1;     // returns 1 if can't establish connection
	} else {
		printf("Connected to NTP server! \nWaiting for 10 seconds\n");
	}
	customDelay(10);
	
	int testTimes = 10;
	pthread_create(&idLED, NULL, blinking, &testTimes);
	pthread_create(&idSensor, NULL, sensor, &testTimes);
	
	pthread_join(idLED, NULL);
	pthread_join(idSensor, NULL);
	
	//printf("%s\n", NTP_res);
	free(NTP_res);
	
	return 0;
}
