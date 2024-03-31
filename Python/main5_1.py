from gpiozero import MCP3008, LED
import subprocess # For executing external commands
import time, threading, datetime, os

led = LED(27)
analog_input = MCP3008(channel=0)

blinkTimes = 250;
blinking_file = "Test254/pyBlinking.txt"
sensing_file = "Test254/pySensing.txt"
duration_file = "Test254/pyDuration.txt"

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
		led.on()
		printTime(i, blinking_file)
		time.sleep(0.9)
		led.off()
		time.sleep(1.1)

	
def sensing():
	i = 1
	while(i <= blinkTimes):
		if analog_input.value * 1024 > 700:
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
