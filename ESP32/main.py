from machine import Pin, ADC
import time

# Initialize the LED and photoresistor
led = Pin(2, Pin.OUT)
photoresistor = ADC(Pin(35))
photoresistor.atten(ADC.ATTN_11DB)  # Configure the attenuation for full range

# Define the threshold for sensing the blink
threshold = 2000  # Adjust this value based on your photoresistor's sensitivity
i = 500
while i > 0:
    # Turn the LED on and record the time
    led.value(1)
    time_on = time.ticks_us()
    
    # Wait for the photoresistor to sense the LED
    while photoresistor.read() <= threshold:
        pass  # Busy-wait until the photoresistor senses the LED
    # Calculate the time difference
    time_sensed = time.ticks_us()
    time_difference = time_sensed - time_on
    
    # Print the times and the time difference
    print("LED on at", time_on, "us")
    print("Blink sensed at", time_sensed, "us")
    print("Time difference:", time_difference, "us")
    
    time.sleep(1)  # Keep the LED on for 1 second
    
    # Turn the LED off
    led.value(0)
    time.sleep(1)  # Keep the LED off for 1 second
    i = i - 1
