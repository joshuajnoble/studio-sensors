 // Define hardware connections
#include "DHT.h"
#include <Adafruit_CC3000.h>
#include <SPI.h>

const char ID = '1';

////////////////////////////////////////////////////
// mic
////////////////////////////////////////////////////
#define MIC_ANALOG_IN A0
////////////////////////////////////////////////////

////////////////////////////////////////////////////
// temp/humid
////////////////////////////////////////////////////
#define DHTPIN 11     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE);
////////////////////////////////////////////////////

////////////////////////////////////////////////////
// light
////////////////////////////////////////////////////
volatile unsigned long cnt = 0;
unsigned long oldcnt = 0;
unsigned long t = 0;
unsigned long last;
unsigned long hz;
////////////////////////////////////////////////////

////////////////////////////////////////////////////
// wifi
////////////////////////////////////////////////////
// CC3000 configuration.
#define ADAFRUIT_CC3000_IRQ    3
#define ADAFRUIT_CC3000_VBAT   5
#define ADAFRUIT_CC3000_CS     10

// Wifi network configuration.
#define WLAN_SSID              "frogwirelessext"
#define WLAN_PASS              "friedolin"
#define WLAN_SECURITY          WLAN_SEC_WPA2
const uint8_t SERVER_IP[] = {162, 242, 237, 33};    // Logging server IP address.  Note that this
const uint8_t SERVER_PORT = 3000;                // Logging server listening port.
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT);
uint32_t ip;
prog_char URI[] PROGMEM = "http://162.242.237.33";
uint8_t loc;
////////////////////////////////////////////////////

/* 

we're looking for an IP that sends a few things:

light intensity, light, PIR, sound, temp, humid

?id=idl=122li=12&pir=true&sound=16&t=18&h=20

*/

char values[122];
uint8_t pirVal;

long lastSend;

#define PIR_PIN 6
//#define USING_SERIAL

void setup()
{
  
  #ifdef USING_SERIAL
  Serial.begin(115200);
  while(!Serial);
  #endif
  
  //  Configure LED pin as output
  pinMode(PIR_PIN, INPUT);
  dht.begin();
  
  #ifdef USING_SERIAL
  Serial.println( " dht begin "); 
  #endif
  
  // wifi
  if (!cc3000.begin()) {
    while(1);
  }
  #ifdef USING_SERIAL
  Serial.println(" cc3300 begin " );
  #endif

  // Connect to AP.
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    while(1);
  }
  
  #ifdef USING_SERIAL
  Serial.println(" connected " );
  #endif
  
  // Wait for DHCP to be complete
  while (!cc3000.checkDHCP()) {
    delay(100);
  }
  
  #ifdef USING_SERIAL
  Serial.println(" connected DHCP " );
  #endif
  
  pinMode(2, INPUT);            // Set pin 2 to input
  digitalWrite(2, HIGH);        // Turn on pullup resistor
  attachInterrupt(1, count_inc, RISING);
  
//  while (ip == 0) {
//    if (! cc3000.getHostByName("162.242.237.33", &ip)) {
//      Serial.println(F("Couldn't resolve!"));
//    }
//    delay(500);
//  }
  
  // Store the IP of the server.
  ip = cc3000.IP2U32(162, 242, 237, 33);
}

void loop()
{

  bool doSend = false;
  if(millis() - lastSend > 10000) 
  {
    doSend = true;
    lastSend = millis();
  }
  
  uint8_t tp = digitalRead(PIR_PIN);
  if(tp == LOW) {
    pirVal = tp;
  }
  
  // color of light
  //if (millis() - lastRead > 5) {
  if (millis() - last >= 1000) {
    last = millis();
    t = cnt;
    hz = t - oldcnt;
    oldcnt = t;
  }

  if(!doSend) {
    delay(10);
    return;
  }
  
  // Check the envelope input
  uint8_t soundValue = analogRead(MIC_ANALOG_IN); // 10< quiet 10-20 noise 20-30 noisy 30+ loud
  
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  uint8_t h = dht.readHumidity();
  uint8_t t = dht.readTemperature();
  
  //////////////////////////////////////////////////////////////////
  // q - why does this look like this?
  // a - to save space. We could save even more space by bit mashing
  // everything together but that just seems to be overkill, so we'll
  // stick with just not using the string 
  //////////////////////////////////////////////////////////////////
  
  memset(&values[0], 0, 122);
  
  // set our ID
  
  values[0] = ' ';
  values[1] = '/';
  values[2] = '?';
  values[3] = 'i';
  values[4] = '=';
  values[5] = ID;
  values[6] = '&';
  
  loc = 7;
  
  values[loc] = 'l';
  loc++;
  values[loc] = '=';
  loc++;
  
  itoa( hz, &values[loc], 16 );
  
  if(hz < 15 ) { loc+=1; }
  else if(hz < 254 ) { loc+=2; }
  else { loc+=3; }
  
  values[loc] = '&';
  loc++;
  values[loc] = 'm';
  loc++;
  values[loc] = '=';
  loc++;
  
  if(pirVal == LOW) {
    values[loc] = '1';
  } else {
    values[loc] = '0';
  }
  loc++;
  
  values[loc] = '&';
  loc++;
  values[loc] = 's';
  loc++;
  values[loc] = '=';
  loc++;
  
  itoa( soundValue, &values[loc], 16 );
  
  if(soundValue < 10 ) { loc+=1; }
  else { loc+=2; }
  
  values[loc] = '&';
  loc++;
  values[loc] = 't';
  loc++;
  values[loc] = '=';
  loc++;
  
  itoa( t, &values[loc], 16 );
  
  if(t < 10 ) { loc+=1; }
  else { loc+=2; }
  
  values[loc] = '&';
  loc++;
  values[loc] = 'h';
  loc++;
  values[loc] = '=';
  loc++;
  
  itoa( h, &values[loc], 16 );
  
  if(h < 16 ) { loc+=1; }
  else { loc+=2; }
  
  Adafruit_CC3000_Client server = cc3000.connectTCP(ip, 3000);
  
  if (server.connected()) {
    server.fastrprint(F("GET"));
    //server.fastrprint(F(" /index.html?i=1&m=1&t=1&h=2&s=2"));
    server.fastrprint(values);
    server.fastrprint(F(" HTTP/1.1\r\n"));
    server.fastrprint(F("\r\n"));
    server.fastrprint(F("Host: "));
    server.println();
  }
  
  
  server.close();
  //cc3000.disconnect();

  // pause for 1 second
  //delay(1000);
  
  pirVal = HIGH;
  
}

void count_inc() {
  cnt++;
}
