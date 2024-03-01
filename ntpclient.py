import socket, struct, sys, time

NTP_SERVER = 'ntp.ttu.ee'
TIME1970 = 2208988800

def sntp_client():
    client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    data = b'\x1b' + 47 * b'\0'
    client.sendto(data, (NTP_SERVER, 123))
    data, address = client.recvfrom(1024)
    if data: 
        print('Response received from:', address)
        seconds, fraction = struct.unpack('!LL', data[32:40])
        seconds -= TIME1970
        fraction /= 2**32
        print('\tTime = %s.%s' % (time.ctime(seconds), str(fraction)[2:]))
        # Adjust the fraction to represent milliseconds
        milliseconds = round(fraction * 1000)
        print('\tFractional part = %d milliseconds' % milliseconds)

if __name__ == '__main__':
    sntp_client()
