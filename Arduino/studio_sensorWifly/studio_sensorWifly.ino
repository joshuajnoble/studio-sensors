// Define hardware connections
#include "DHT.h"
#include <WiFlyHQ.h>

#include <SoftwareSerial.h>
SoftwareSerial wifiSerial(8,9);


WiFly wifly;

//////// what ID are we?
const char ID = '6';

const unsigned long SEND_INTERVAL = 60000; // 60s
const long SOUND_INTERVAL = 3000; // 3s
const long PIR_WAIT = 30000;

const unsigned long connectTimeout  = 1000L; // Max time to wait for server connection
unsigned long wifiTimeout;

const int WIFI_POWER_PIN = 12;

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

// Wifi network configuration.
#define WLAN_SSID              "frogwirelessext"
#define WLAN_PASS              "friedolin"
#define WLAN_SECURITY          WLAN_SEC_WPA2
const uint8_t SERVER_IP[] = { 162, 242, 237, 33};    // Logging server IP address.  Note that this
const char site[] = "http://162.242.237.33";
//const uint8_t SERVER_PORT = 3000;                // Logging server listening port.
uint32_t ip;
uint8_t loc;

//String readings;

////////////////////////////////////////////////////

/* 
 
 we're looking for an IP that sends a few things:
 
 light = l
 PIR = m
 sound = s
 temp = t
 humidity = h
 
 ?i=1&l=122&m=1&s=16&t=18&h=30
 
 */

// average sound readings
long lastSoundReading;
uint8_t soundReadings[20];
uint8_t soundReadingIndex;

//char values[122];
uint8_t pirVal;

long lastSend;

#define PIR_PIN 6
//#define USING_SERIAL

String out = "";

void setup()
{

#ifdef USING_SERIAL
  Serial.begin(115200);
  while(!Serial);
#endif
  
  pinMode(WIFI_POWER_PIN, OUTPUT);
  digitalWrite(WIFI_POWER_PIN, HIGH);

  //  Configure PIR pin as output
  pinMode(PIR_PIN, INPUT);
  dht.begin();

  pinMode(2, INPUT);            // Set pin 2 to input
  digitalWrite(2, HIGH);        // Turn on pullup resistor
  attachInterrupt(1, count_inc, RISING);

  soundReadingIndex = 0;
  lastSend = 0;
  // calibrate PIR
  delay(PIR_WAIT);
  
  
  wifiSerial.begin(9600);
  if (!wifly.begin(&wifiSerial, &Serial)) {
      Serial.println("Failed to start wifly");
  }

  /* Join wifi network if not already associated */
  if (!wifly.isAssociated()) {
	/* Setup the WiFly to connect to a wifi network */
	Serial.println("Joining network");
	wifly.setSSID(WLAN_SSID);
	wifly.setPassphrase(WLAN_PASS);
	wifly.enableDHCP();

	if (wifly.join()) {
	    Serial.println("Joined wifi network");
	} else {
	    Serial.println("Failed to join wifi network");
	    //terminal();
	}
  } else {
      Serial.println("Already joined network");
  }

}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////

void loop()
{

  bool doSend = false;
  if(millis() - lastSend > SEND_INTERVAL) 
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

  if(millis() - lastSoundReading > SOUND_INTERVAL)
  {
    lastSoundReading = millis();
    if(soundReadingIndex > 19)
    {
      soundReadingIndex = 0; 
    }
    soundReadings[soundReadingIndex] = analogRead(MIC_ANALOG_IN);
    soundReadingIndex++;
  }

  if(!doSend) {
    delay(50);
    return;
  }

#ifdef USING_SERIAL
  Serial.println(" sending " );
#endif

  // Check the envelope input
  uint32_t volume = 0;
  for( int i = 0; i < 20; i++ )
  {
    volume += soundReadings[i];
  }
  uint8_t soundValue = volume / 20; // 10< quiet 10-20 noise 20-30 noisy 30+ loud

  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  uint8_t h = dht.readHumidity();
  uint8_t temp = dht.readTemperature();

  //////////////////////////////////////////////////////////////////
  // q - why does this look like this?
  // a - to save space. We could save even more space by bit mashing
  // everything together but that just seems to be overkill, so we'll
  // stick with just not using the string 
  //////////////////////////////////////////////////////////////////

  //memset(&values[0], 0, 122);

  // set our ID
  
  out += "/?i=";
  out += ID;
  out += "&l=";
  out += hz;
  out += "&m=";
  
  if(pirVal == LOW) {
    out += "1";
  } 
  else {
    out += "0";
  }
  out += "&s=";
  out += volume;
  out += "&t=";
  out += temp;
  out += "&h=";
  out += h;

  wifiTimeout = millis();

  // have we connected?
  boolean waitingToConnect = true;

#ifdef USING_SERIAL
  Serial.print(" connected? " );
  Serial.println(cc3000.checkConnected());
#endif

#ifdef USING_SERIAL
  Serial.print(" DHCP? " );
  Serial.println(cc3000.checkDHCP());
#endif

 if (wifly.open(site, 80)) 
 {
    Serial.print("Connected to ");
    Serial.println(site);

    /* Send the request */
    wifly.println("GET / HTTP/1.0");
    wifly.println(out);
    } else {
        Serial.println("Failed to connect");
    }
  pirVal = HIGH;

}

void count_inc() {
  cnt++;
}

