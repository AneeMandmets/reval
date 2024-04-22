#include <Ethernet.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // replace with your Ethernet shield's MAC address
IPAddress ip(192, 168, 137, 177); // replace with your desired IP
EthernetClient client;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    Serial.println("waiting"); // wait for serial port to connect.
  }

  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(mac, ip);
  }
  delay(1000);
}

void loop() {
  checkInternetConnection();
  delay(60000); // check every minute
}

void checkInternetConnection() {
  Serial.println("\nChecking internet connection...");
  if (client.connect("www.google.com", 80)) {
    Serial.println("Internet connection is active");
    client.stop();
  } else {
    Serial.println("Internet connection is not active");
  }
}
