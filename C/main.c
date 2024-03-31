#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <pthread.h>
#include <mcp3004.h>
#include <stdbool.h>

#define BASE 100
#define SPI_CHAN 0

int blinkState = 0;
unsigned long lastStateChangeTime = 0;
const unsigned long onDuration = 1000;  
const unsigned long offDuration = 1000; 

bool blinkRecorded = false;

const char* sensorFile = "Test502/sensor.txt";
const char* blinkFile = "Test502/blinking.txt";
const char* durationFile = "Test502/testDuration.txt";
int testTimes = 500;

void printTime(const char *filename, int i){
    struct timeval curTime;
    struct tm *localTime;
    char timeBuffer[80];

    gettimeofday(&curTime, NULL); 
    localTime = localtime(&curTime.tv_sec); 

    strftime(timeBuffer, 80, "%Y-%m-%d %H:%M:%S", localTime);

    // Open the file 
    FILE *fp = fopen(filename, "a"); // "a" for append mode
    if (fp == NULL) {
        perror("Error opening file");
        return;  // Exit if there's an error
    }

    // Write the time to the file
    fprintf(fp, "%d) %s.%06ld\n", i, timeBuffer, curTime.tv_usec);

    // Close the file
    fclose(fp); 
}

void printTimeTwo(const char *filename, char *str){
    struct timeval curTime;
    struct tm *localTime;
    char timeBuffer[80];

    gettimeofday(&curTime, NULL); 
    localTime = localtime(&curTime.tv_sec); 

    strftime(timeBuffer, 80, "%Y-%m-%d %H:%M:%S", localTime);

    // Open the file 
    FILE *fp = fopen(filename, "a"); // "a" for append mode
    if (fp == NULL) {
        perror("Error opening file");
        return;  // Exit if there's an error
    }

    // Write the time to the file
    fprintf(fp, "%s) %s.%06ld\n", str, timeBuffer, curTime.tv_usec);

    // Close the file
    fclose(fp); 
}

void *blinking(void* i) {
	int *n = (int*)i;
	int times = 1;
	int j = *n;
	
	while (j > 0) {
	    unsigned long currentTime = millis(); 
	    switch (blinkState) {
		case 0:
		    if (currentTime - lastStateChangeTime >= offDuration){
			//printf("LED: %d\n", currentTime);
			digitalWrite(2, HIGH);
			blinkState = 1;
			lastStateChangeTime = currentTime;
			blinkRecorded = false;
			printTime(blinkFile, times);
			++times;
			j--;
		    }
		    break;
		case 1:
		    if (currentTime - lastStateChangeTime >= onDuration) {
			digitalWrite(2, LOW);
			blinkState = 0;
			lastStateChangeTime = currentTime;
		    }
		    break; 
		}
	}
	pthread_exit(NULL);
}
void *sensor(void* i){
	int *n = (int*)i;
	//int adc;
	int times = 1;
	int j = *n;
	int bstart = analogRead(BASE); // taking first reading so that ADC can start its work before the actual loop.
	// customDelay(1);								// without this, the ADCs first reading is wrong
	//printf("%d\n", &n);
	while(j > 0){
	    //printf("%d\n", analogRead(BASE));
	    //customDelay(1000);
	    if(analogRead(BASE) > 800 && !blinkRecorded){ // arv muutub vastavalt valgusele 
		    printTime(sensorFile, times);
		    blinkRecorded = true;
		    //lastRecordedBlinkTime = millis();
		    //printf("Sensor: %d\n", lastRecordedBlinkTime);
		    j--;
		    ++times; 
	    }	
	}
	pthread_exit(NULL);
}

int main() {
    const char *ntpServer = "time.google.com";
    char command[50];

    // Build the ntpdate command
    sprintf(command, "sudo ntpdate %s", ntpServer);

    // Execute and get the return code
    int result = system(command); 

    if (result == 0) {
        printf("NTP synchronization likely successful.\n");
    } else {
        fprintf(stderr, "NTP synchronization might have failed. (Error code: %d)\n", result);
    }
    
    wiringPiSetup();
    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);
	
    pthread_t idLED, idSensor;
    mcp3004Setup(BASE, SPI_CHAN);
    delay(4);

    printTimeTwo(durationFile, "Algusaeg");
    pthread_create(&idLED, NULL, blinking, &testTimes);
    pthread_create(&idSensor, NULL, sensor, &testTimes);
	
    pthread_join(idLED, NULL);
    pthread_join(idSensor, NULL);
    printTimeTwo(durationFile, "Loppaeg");
    return 0;
}
