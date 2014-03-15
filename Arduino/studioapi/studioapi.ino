 // Define hardware connections
#include "DHT.h"
#include <Adafruit_CC3000.h>
#include <SPI.h>

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
const uint8_t SERVER_IP[] = {192, 168, 1, 105};    // Logging server IP address.  Note that this
const uint8_t SERVER_PORT = 3000;                // Logging server listening port.
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT);
uint32_t ip;
prog_char URI[] PROGMEM = "http://162.17.13.11:3000";
uint8_t loc;
////////////////////////////////////////////////////

/* 

we're looking for an IP that sends a few things:

light intensity, light, PIR, sound, temp, humid

?l=122li=12&pir=true&sound=16&t=18&h=20

*/

char values[122];
uint8_t pirVal;

long lastSend;

#define PIR_PIN 6


void setup()
{
  
  Serial.begin(115200);
  while(!Serial);
  
  //  Configure LED pin as output
  pinMode(PIR_PIN, INPUT);
  dht.begin();
  
  Serial.println( " dht begin "); 
  
  // wifi
  if (!cc3000.begin()) {
    while(1);
  }
  
  Serial.println(" cc3300 begin " );

  // Connect to AP.
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    while(1);
  }
  
  Serial.println(" connected " );
  
  // Wait for DHCP to be complete
  while (!cc3000.checkDHCP()) {
    delay(100);
  }
  
  Serial.println(" connected DHCP " );
  
  pinMode(2, INPUT);            // Set pin 2 to input
  digitalWrite(2, HIGH);        // Turn on pullup resistor
  attachInterrupt(2, count_inc, RISING);
  
  // Store the IP of the server.
  ip = cc3000.IP2U32(SERVER_IP[0], SERVER_IP[1], SERVER_IP[2], SERVER_IP[3]);
}

void loop()
{

  bool doSend = false;
  if(millis() - lastSend > 1000) 
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

  //Adafruit_CC3000_Client server = cc3000.connectTCP(ip, SERVER_PORT);
  
  
  //////////////////////////////////////////////////////////////////
  // q - why does this look like this?
  // a - to save space. We could save even more space by bit mashing
  // everything together but that just seems to be overkill, so we'll
  // stick with just not using the string 
  //////////////////////////////////////////////////////////////////
  
  memset(&values[0], 0, 122);
  
  loc = 0;
  
  values[loc] = 'l';
  loc++;
  values[loc] = '=';
  loc++;
  
  itoa( hz, &values[loc], 16 );
  
  if(hz < 16 ) { loc+=1; }
  else if(hz < 255 ) { loc+=2; }
  else { loc+=3; }
  
  values[loc] = '&';
  loc++;
  values[loc] = 'p';
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
  
  //if (server.connected()) {
  //  server.println(&values[0]);
  //}
  
  
  //server.close();
  Serial.println(values);

  // pause for 1 second
  //delay(1000);
  
  pirVal = HIGH;
  
}

void count_inc() {
  cnt++;
}
