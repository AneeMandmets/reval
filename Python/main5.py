import socket, struct, sys, time, threading
import gpiod

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
        print("blinked")
        led_line.set_value(1)
        sntp_client(blinking_file, i)
        time.sleep(1)
        led_line.set_value(0)
        time.sleep(1)
    led_line.release()
    

def sensing(blinkTimes): # Sensor PIN 18
    sensing_file = "pySensing.txt"
    i = 1
    while(i <= blinkTimes):
        if sensor_line.get_value() == 0:
            sntp_client(sensing_file, i)
            print("sensed")
            time.sleep(2)
            i = i + 1
    sensor_line.release()


if __name__ == '__main__':
    # Set input and output pins
    chip = gpiod.Chip('gpiochip4')
    led_line = chip.get_line(ledpin)
    sensor_line = chip.get_line(sensorpin)
    led_line.request(consumer="LED", type=gpiod.LINE_REQ_DIR_OUT)
    sensor_line.request(consumer="sensor", type=gpiod.LINE_REQ_DIR_IN)
    # sntp_client()
    i = 10
    tblink = threading.Thread(target=blinking, args=(i,))
    tsense = threading.Thread(target=sensing, args=(i,))
    
    tblink.start()
    tsense.start()

    tblink.join()
    tsense.join()
