#include <stdio.h>
#include "ntp.c"
#include "delay.c"
#include "setup.c"
#include <stdlib.h>
#include <wiringPi.h>
#include <pthread.h>
#include <mcp3004.h>

#define BASE 100
#define SPI_CHAN 1
 
void *blinking(void* i) {
	int *n = (int*)i;
	FILE *blinkingTimes;
	char* ntpTime;
	blinkingTimes = fopen("blinking.txt", "w");
	for (int j = *n; j > 0; j--) {
		ntpTime = NTP();
		fprintf(blinkingTimes, "%s\n", ntpTime);
		digitalWrite(0, HIGH);
		customDelay(1);
		digitalWrite(0, LOW);
		customDelay(1);
	}
	fclose(blinkingTimes);
	pthread_exit(NULL);
}

void *sensor(void* i){
	int *n = (int*)i;
	FILE *sensorTimes;
	char* ntpTime;
	//int adc;
	sensorTimes = fopen("sensor.txt", "w");
	//printf("%d\n", &n);
	int j = *n;
	while(j > 0){
		//printf("%d\n", analogRead(BASE));
		//customDelay(2);
		if(analogRead(BASE) < 700){ // arv muutub vastavalt valgusele 
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
	//setUp();
	wiringPiSetup();
	pinMode(0, OUTPUT);
	
	pthread_t idLED, idSensor;
	mcp3004Setup(BASE, SPI_CHAN);
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
