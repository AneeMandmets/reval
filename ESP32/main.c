#define LED_PIN 2
#define SENSOR_PIN 35

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(SENSOR_PIN, INPUT);
  Serial.begin(115200);
  analogReadResolution(12);  // Set the sample  bits and resolution
}
int i = 500;
bool ending = false;

void loop() {
  while (i >= 0){
    digitalWrite(LED_PIN, HIGH); // Turn the LED on
    unsigned long startTime = micros(); // Get the start time
    Serial.print("Blinked: ");
    Serial.print(startTime);
    // Wait for the sensor to detect the light
    while (analogRead(SENSOR_PIN) < 1500); // Adjust the threshold as needed
    unsigned long endTime = micros(); // Get the end time
    Serial.print(" Sensed: ");
    Serial.print(endTime);
    delay(1000);
    digitalWrite(LED_PIN, LOW); // Turn the LED off
    unsigned long elapsedTime = endTime - startTime;
    Serial.print(" Difference: ");
    Serial.println(elapsedTime);
    i--;
    delay(1000); // Wait for a second before the next blink 
  }
    if (!ending){
      Serial.println(micros());
      ending = true;  
    }
}
