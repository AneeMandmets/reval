#include <stdio.h>
#include <unistd.h>
#include <gpiod.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <stdint.h>

#define CONSUMER "Consumer"
#define GPIOCHIP 4
#define GPIOPIN 27
#define SPI_DEVICE "/dev/spidev0.0"

int blinkState = 0; // for whether the LED has blinked or not
unsigned long lastStateChangeTime = 0; // MS since last blink 
const unsigned long onDuration = 1000;  // LED on for 1 second
const unsigned long offDuration = 1000; // LED off for 1 second

bool blinkRecorded = false; // whether the blink has been sensed on not

struct timespec start_time;

// These are for the test files
const char* sensorFile = "Test504/sensor.txt";
const char* blinkFile = "Test504/blinking.txt";
const char* durationFile = "Test504/testDuration.txt";
int testTimes = 500;

// Gets the start time at the start of the test
void record_start_time() { 
    clock_gettime(CLOCK_REALTIME, &start_time);
}

// Gets current time and converts it to milliseconds
unsigned long millis() {
    struct timespec current_time;
    clock_gettime(CLOCK_REALTIME, &current_time);
    return (current_time.tv_sec - start_time.tv_sec) * 1000 + (current_time.tv_nsec - start_time.tv_nsec) / 1.0e6;
}

// For reading ADC values
int read_adc(int fd, uint8_t channel) {
    uint8_t tx[] = {1, (8 + channel) << 4, 0}; // data to be sent
    uint8_t rx[3];  // data to be received

    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = sizeof(tx),
        .delay_usecs = 0,
        .speed_hz = 1000000,
        .bits_per_word = 8,
    };

    if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) == -1) {
        perror("Failed to read from the SPI bus");
        return -1;
    }

    return ((rx[1] & 3) << 8) + rx[2];
}

// For printing the blinking times and sensing times
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

// For printing the start and end times
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

// Function for the LED blinking thread
void *blinking(void* i) {
    /* PIN as output setup */
    struct gpiod_chip *chip; 
    struct gpiod_line *line;
    int ret;

    chip = gpiod_chip_open_by_number(GPIOCHIP);
    if (!chip) {
        perror("Open chip failed\n");
       exit(-1);
    }

    line = gpiod_chip_get_line(chip, GPIOPIN);
    if (!line) {
        perror("Get line failed\n");
        gpiod_chip_close(chip);
        exit(-1);
    }

    ret = gpiod_line_request_output(line, CONSUMER, 0);
    if (ret < 0) {
        perror("Request line as output failed\n");
        gpiod_chip_close(chip);
        exit(-1);
    }
    gpiod_line_set_value(line, 0);

    /* Blinking logic */
	int *n = (int*)i; // how many times the LED should blink
	int times = 1;
	int j = *n;
	
	while (j > 0) {
	    unsigned long currentTime = millis(); // gets the current time
	    switch (blinkState) { 
            case 0: // if the LED hasn't blinked yet
                if (currentTime - lastStateChangeTime >= offDuration){ // if LED has been off for or more than a second
                    printTime(blinkFile, times); // prints the time to a file
                    gpiod_line_set_value(line, 1); // turns LED on
                    blinkState = 1; // LED has blinked 
                    lastStateChangeTime = currentTime; // last state change time to current time
                    blinkRecorded = false; // the blink hasn't been sensed yet
                    ++times;
                    j--;
                }
                break;
            case 1:
                if (currentTime - lastStateChangeTime >= onDuration) { // if LED has been on for or more than a second
                    gpiod_line_set_value(line, 0); // turns off LED
                    blinkState = 0; // LED is turned off
                    lastStateChangeTime = currentTime; // last state change time to current time
                }
                break; 
            }
	}
    gpiod_line_release(line); // releases the used line so that no errors occur for next test
    gpiod_chip_close(chip); // releases the used chip so that no errors occur for next test
	pthread_exit(NULL); // exits the thread so that the program can stop
}

// Function for the sensing thread
void *sensor(void* i){
    /* SPI device setup */
	int fd = open(SPI_DEVICE, O_RDWR);
	if (fd < 0) {
	    perror("Failed to open the SPI device");
	}

    /* Sensing logic */
	int *n = (int*)i; // how many times the LED is expected to blink
	int times = 1;
	int j = *n;
	while(j > 0){
	    if(read_adc(fd, 0) > 700 && !blinkRecorded){ // if the ADC value is over a certain threshold
		    printTime(sensorFile, times); // prints current time to file
		    blinkRecorded = true; // the blink has been sensed
		    j--;
		    ++times; 
	    }	
	}
	close(fd); // closes file
	pthread_exit(NULL); // exits the thread so that the program can stop
}

int main() {
    /* Example of getting time from an NTP server. Not needed for the actual test. */
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
    /* End of NTP synchronization */

    pthread_t idLED, idSensor;
    record_start_time(); // Gets the test start time
    printTimeTwo(durationFile, "Algusaeg"); // Prints start time to the file
    pthread_create(&idLED, NULL, blinking, &testTimes); // Creates blinking thread
    pthread_create(&idSensor, NULL, sensor, &testTimes);  // Creates sensing thread
	
    pthread_join(idLED, NULL); // Starts blinking thread
    pthread_join(idSensor, NULL); // Starts sensing thread
    printTimeTwo(durationFile, "Loppaeg"); // Prints end time to the file

    return 0;
}
