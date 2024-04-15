import gpiod
import subprocess # For executing external commands
import time, threading, datetime, os
import spidev

chip = gpiod.Chip('gpiochip0')
led = chip.get_line(27)
led.request(consumer='blink', type=gpiod.LINE_REQ_DIR_OUT)

spi = spidev.SpiDev()
spi.open(0, 0)
spi.max_speed_hz = 5000
spi.mode = 0

def read_channel(channel):
    adc = spi.xfer2([1, (8 + channel) << 4, 0])
    data = ((adc[1] & 3) << 8) + adc[2]
    return data

light_channel = 0

def read_adc():
    light_level = read_channel(light_channel)
    return light_level

blinkTimes = 50;
blinking_file = "Test52/pyBlinking.txt"
sensing_file = "Test52/pySensing.txt"
duration_file = "Test52/pyDuration.txt"

def ntp_sync():
	server = "time.google.com"
	command = ["sudo", "ntpdate", server]
	result = subprocess.run(command, capture_output=True, text=True)
	
	if result.returncode == 0:
		print("NTP synchronization likely successful.")
	else:
		print("NTP synchronization might have failed. (Error code: {})".format(result.returncode))
		print("Error output: {}".format(result.stderr))

def blinking():
	i = 0
	while (i < blinkTimes):
		i = i + 1
		printTime(i, blinking_file)
		led.set_value(1)
		time.sleep(0.9)
		led.set_value(0)
		time.sleep(1.1)

def sensing():
	i = 1
	while(i <= blinkTimes):
		if read_adc() > 700:
			printTime(i, sensing_file)
			time.sleep(1)
			i = i + 1

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
	ntp_sync()

	tblink = threading.Thread(target=blinking)
	tsense = threading.Thread(target=sensing)
	printTime(1, duration_file)
	tblink.start()
	tsense.start()
	
	tblink.join()
	tsense.join()
	printTime(2, duration_file)
