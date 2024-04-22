#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <MCP3XXX.h>

#define CS_PIN 7
#define CLOCK_PIN 6
#define MOSI_PIN 5
#define MISO_PIN 4

MCP3008 adc;

unsigned long prevLedTime = millis();
long LEDinterval = 1000;
int led = LOW;

unsigned long prevSenseTime = millis();
int printInterval = 1500;


// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

unsigned int localPort = 8888;       // local port to listen for UDP packets

const char timeServer[] = "time.nist.gov"; // time.nist.gov NTP server

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

void setup() {
  // You can use Ethernet.init(pin) to configure the CS pin
  Ethernet.init(10);  // Most Arduino shields

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // start Ethernet and UDP
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // no point in carrying on, so do nothing forevermore:
    while (true) {
      delay(1);
    }
  }
  Udp.begin(localPort);
  pinMode(8, OUTPUT);
  adc.begin(CS_PIN, MOSI_PIN, MISO_PIN, CLOCK_PIN);
}

void loop() { // Using "multithreading" for blinking and sensing
  unsigned long currentTime = millis();

  // Blinking the LED
  if (currentTime - prevLedTime > LEDinterval) {
    prevLedTime = currentTime;

    if (led) {
      led = LOW;  
    } else {
      led = HIGH;  
    }

    digitalWrite(8, led);
  }
  if (adc.analogRead(0) > 700) {
    if (currentTime - prevSenseTime > printInterval){
      prevSenseTime = currentTime;
      Serial.println("Sensed");  
      ntp();
    }
  }   
  // Printing sensor values
  /*if (currentTime - prevSenseTime > printInterval){
    prevSenseTime = currentTime;
    Serial.println(adc.analogRead(0));  
  }*/
}


void ntp() {
  sendNTPpacket(timeServer); // send an NTP packet to a time server

  // wait to see if a reply is available
  //delay(1000);
  if (Udp.parsePacket()) {
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    // the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, extract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    //Serial.print("Seconds since Jan 1 1900 = ");
    //Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL - 2 * 60 * 60; // +2 hours
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:

    // Converting time to milliseconds:
    
    unsigned long long epochmillis = epoch * 1000ULL;

    // split epochMillis into two parts
    unsigned long upper = (unsigned long) (epochmillis / 1000000ULL);
    unsigned long lower = (unsigned long) (epochmillis % 1000000ULL);

    // create buffers for the string parts
    char upperStr[11];
    char lowerStr[7];

    // print the parts to the string buffers
    sprintf(upperStr, "%lu", upper);
    sprintf(lowerStr, "%06lu", lower);

    // concatenate the string parts
    char epochMillisStr[18];
    strcpy(epochMillisStr, upperStr);
    strcat(epochMillisStr, lowerStr);

    // print Unix time in milliseconds:
    Serial.println(epochMillisStr);
    
    //Serial.println(epoch);


    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    Serial.print(':');
    if (((epoch % 3600) / 60) < 10) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ((epoch % 60) < 10) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch % 60); // print the second
  }
  // wait ten seconds before asking for the time again
  // delay(10000);
  Ethernet.maintain();
}

// send an NTP request to the time server at the given address
void sendNTPpacket(const char * address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); // NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
