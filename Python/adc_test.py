import spidev
import time

# Create a SPI interface from the spidev module
spi = spidev.SpiDev()

# Open the SPI device. Bus 0, device 0
spi.open(0, 0)

# Set the SPI speed and mode
spi.max_speed_hz = 5000
spi.mode = 0

# Function to read SPI data from MCP3008 chip
# Channel must be an integer 0-7
def read_channel(channel):
    adc = spi.xfer2([1, (8 + channel) << 4, 0])
    data = ((adc[1] & 3) << 8) + adc[2]
    return data

# Define sensor channels
light_channel = 0

# Define delay between readings
delay = 2

while True:
    # Read the light sensor data
    light_level = read_channel(light_channel)
    print("Light Level: {}".format(light_level))
    time.sleep(delay)
