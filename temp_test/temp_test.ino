#include "DHT.h"
#include <EthernetUdp.h>
#define DHTPIN 2     // what digital pin we're connected to
#define DHTTYPE DHT22   
DHT dht(DHTPIN, DHTTYPE);
#include <SPI.h>
#include <Ethernet.h>
//-------------------------------------------------------------------------------
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; //Setting MAC Address
char server[] = "api.pushingbox.com"; //pushingbox API server
IPAddress ip(10,18,0,148); //Arduino IP address. Only used when DHCP is turned off.
EthernetClient client; //define 'client' as object
String data; //GET query with data
float suhu; //suhu (bahasa Indonesia) means temperature
boolean koneksi = false;

unsigned int localPort = 8888;       // local port to listen for UDP packets

char timeServer[] = "time.nist.gov"; // time.nist.gov NTP server

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;
//------------------------------------------------------------------------------
void setup() {

  pinMode(7,OUTPUT);
  
  Serial.begin(9600);
  dht.begin();
  if (Ethernet.begin(mac) == 0) {
  Serial.println("Failed to configure Ethernet using DHCP");
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);
  }
  Udp.begin(localPort);
  delay(1000);
}
//------------------------------------------------------------------------------
void loop(){
 //---------------//
  float f = dht.readTemperature(true);
  float h = dht.readHumidity();
   float hif = dht.computeHeatIndex(f, h);
   data+="";
  data+="GET /pushingbox?devid=vFE7764CDE169915&temp="; //GET request query to pushingbox API
  data+=f;
  data+="&hum=";
  data+=h;
  data+="&index=";
  data+=hif;
  data+=" HTTP/1.1";

  
   Serial.println("connecting...");
   if (client.connect(server, 80)) {
     sendData();  
     koneksi = true; //connected = true
   }
   else{
     Serial.println("connection failed");
   }
  // loop
  while(koneksi){
    if (client.available()) {
    char c = client.read(); //save http header to c
    Serial.print(c); //print http header to serial monitor
    }
    if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
          Serial.print("Temperature Sent :");
          Serial.println(f); //print sent value to serial monitor
    client.stop(); 
          koneksi = false; 
          data = ""; //data reset
    }
  }

  //------------------TIME Check -------------------
sendNTPpacket(timeServer); // send an NTP packet to a time server
int Hour = 0;
  // wait to see if a reply is available
  delay(1000);
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
    

    
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:


    // print the hour, minute and second:
    int Hour = ((epoch  % 86400L) / 3600) -4;
    if( Hour <0){
      Hour = 24+Hour;
    }
    Serial.print("The Eastern time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.println(Hour); // print the hour (86400 equals secs per day)
    
  }
   Ethernet.maintain();
   




if ((Hour >7) && (Hour <19))
{
 Serial.println("In Work Time, It is going to check the temp!");
if (f>85){
  digitalWrite(7,HIGH);
  Serial.println("Motor Run");
}
else{
  digitalWrite(7,LOW);
}


}
else{
  digitalWrite(7,LOW);
}
delay(120000);
}

void sendData(){
  Serial.println("connected");
  client.println(data);
  client.println("Host: api.pushingbox.com");
  client.println("Connection: close");
  client.println();
}
void sendNTPpacket(char* address) {
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
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}



