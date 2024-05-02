import gpiod
import subprocess # For executing external commands
import time, threading, datetime, os
import spidev

chip = gpiod.Chip('gpiochip4') # For RPi 5 the chip has to be "gpiochip4", for RPi 4 it is "gpiochip0"
led = chip.get_line(27) # Using GPIO27
led.request(consumer='blink', type=gpiod.LINE_REQ_DIR_OUT)

# SPI device setup
spi = spidev.SpiDev()
spi.open(0, 0)
spi.max_speed_hz = 1000000
spi.mode = 0

# For reading from ADC
def read_channel(channel):
    adc = spi.xfer2([1, (8 + channel) << 4, 0])
    data = ((adc[1] & 3) << 8) + adc[2]
    return data

light_channel = 0 # The channel which is being read from the ADC

def read_adc():
    light_level = read_channel(light_channel)
    return light_level

# These are for the test files
blinkTimes = 500
blinking_file = "Test504/pyBlinking.txt"
sensing_file = "Test504/pySensing.txt"
duration_file = "Test504/pyDuration.txt"

blinkState = 0 # for whether the LED has blinked or not
lastStateChangeTime = time.time() * 1000 # MS since last blink 
onDuration = 1000 # LED on for 1 second
offDuration = 1000 # LED off for 1 second
blinkRecorded = False # whether the blink has been sensed on not
 
# Example function of getting time from an NTP server. Not needed for the actual test. 
def ntp_sync():
	server = "time.google.com"
	command = ["sudo", "ntpdate", server]
	result = subprocess.run(command, capture_output=True, text=True)
	
	if result.returncode == 0:
		print("NTP synchronization likely successful.")
	else:
		print("NTP synchronization might have failed. (Error code: {})".format(result.returncode))
		print("Error output: {}".format(result.stderr))

# Function for the LED blinking thread
def blinking():
	global blinkState, lastStateChangeTime, blinkRecorded, blinkTimes # making the variables global so that they can be read
	i = 0
	while (i < blinkTimes):
		currentTime = time.time() * 1000 # gets current time
		if blinkState == 0: # if LED is off
			if currentTime - lastStateChangeTime >= offDuration: # if LED has been off for or more than a second
				printTime(i, blinking_file) # prints current time to file
				led.set_value(1) # LED on
				blinkState = 1 # LED has blinked
				lastStateChangeTime = currentTime
				blinkRecorded = False # blink hasn't been sensed yet
				i = i + 1
		elif blinkState == 1: # LED is on
			if currentTime - lastStateChangeTime >= onDuration: # if LED has been on for or more than a second
				led.set_value(0) # LED off
				blinkState = 0 # LED hasn't blinked
				lastStateChangeTime = currentTime
	time.sleep(1) # wait 1 second
	led.set_value(0) # truns LED off at the end of the thread

# Function for the sensing thread
def sensing():
	global blinkRecorded, blinkTimes # making the variables global so that they can be read
	i = 1
	while(i <= blinkTimes):
		if read_adc() > 700 and not blinkRecorded: # if sensor value is over a certain threshold and the blink hasn't been recorded yet
			printTime(i, sensing_file) # prints current time to file
			blinkRecorded = True # blink has been recorded
			i = i + 1

# Function for printing current time to a file
def printTime(i, fileName):
	if not os.path.exists(fileName):
		with open(fileName, 'w') as f:
			pass
	now = datetime.datetime.now()
	timeStr = now.strftime("%Y-%m-%d %H:%M:%S.%f")
	
	with open(fileName, "a") as file:
		file.write('%d) ' % i)
		file.write(f"{timeStr}\n")

if __name__ == '__main__':
	ntp_sync() # For time synchronization, not needed for test

	tblink = threading.Thread(target=blinking) # Sets up blinking thread
	tsense = threading.Thread(target=sensing) # Sets up sensing thread
	printTime(1, duration_file) # prints starting time to file 
	tblink.start() # starts blinking thread
	tsense.start() # starts sensing thread
	
	tblink.join() # blinking thread rejoins with the main program
	tsense.join() # sensing thread rejoins with the main program
	printTime(2, duration_file) # prints ending time to file 
