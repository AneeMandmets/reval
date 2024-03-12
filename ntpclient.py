import socket, struct, sys, time, threading
#import RPi.GPIO as GPIO

NTP_SERVER = 'ntp.ttu.ee'
TIME1970 = 2208988800
ledpin = 17
sensorpin = 18

def sntp_client(filename, i): 
    client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    data = b'\x1b' + 47 * b'\0'
    client.sendto(data, (NTP_SERVER, 123))
    data, address = client.recvfrom(1024)
    file = open(filename, "a")
    if data: 
        print('Response received from:', address)
        seconds, fraction = struct.unpack('!LL', data[32:40])
        seconds -= TIME1970
        seconds += 2 * 60 * 60
        fraction /= 2**32
        file.write('%d) ' % i)
        t = time.gmtime(seconds)
        file.write('%s.%s\n' % (time.strftime("%H:%M:%S", t), str(fraction)[2:]))

        # print('\tTime = %s.%s' % (time.strftime("%H:%M:%S", t), str(fraction)[2:]))
        # Adjust the fraction to represent milliseconds
        # milliseconds = round(fraction * 1000)
        # print('\tFractional part = %d milliseconds' % milliseconds)

def blinking(blinkTimes): # LED PIN 17
    blinking_file = "pyBlinking.txt"
    i = 0
    while (i < blinkTimes):
        i = i + 1
        GPIO.output(ledpin, GPIO.HIGH)
        sntp_client(blinking_file, i)
        time.sleep(1)
        GPIO.output(ledpin, GPIO.LOW)
        time.sleep(1)

def sensing(): # Sensor PIN 18
    sensing_file = "pySensing.txt"
    i = 1
    while(1):
        if GPIO.input(sensorpin):
            sntp_client(sensing_file, i)
            time.sleep(1)
            i = i + 1


if __name__ == '__main__':
    # Set input and output pins
    GPIO.setup(17, GPIO.OUT)
    GPIO.setup(18, GPIO.IN)
    # sntp_client()

    tblink = threading.Thread(target=blinking, args=(10,))
    tsense = threading.Thread(target=sensing)

    tblink.join()
    tsense.join()
