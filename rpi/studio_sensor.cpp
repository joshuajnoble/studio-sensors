#include <stdio.h>
#include <iostream>
#include <errno.h>
#include <string>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sstream>
#include <arpa/inet.h>
#include "spi.h"
#include <string.h>
#include <time.h>
#include <sys/time.h>

extern "C" { 
   #include "pi_mmio.h"
   #include "common_dht_read.h"
}
#define PORT "3000" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 
spiConfig spidata;

#define LIGHT_PIN 15
#define PIR_PIN 11
#define DHT_MAXCOUNT 32000
#define DHT_PULSES 41
#define DHT_PIN 23

using namespace std;

int pirValue;
bool pirTriggered;
time_t  timev;
unsigned long lastReadTime;
unsigned int lightCount;

struct atmosphereData {
  float temperature;
  float humidity;
};

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int connectAndSend( string hostname )
{
    int sockfd, numbytes;  
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(hostname.c_str(), PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 0;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        //exit(1);
	return 0;
    }

    buf[numbytes] = '\0';

    printf("client: received '%s'\n",buf);

    close(sockfd);
   return 1;
}


int readDHT(atmosphereData *aData)
{
   // Store the count that each DHT bit pulse is low and high.
  // Make sure array is initialized to start at zero.
  int pulseCounts[DHT_PULSES*2] = {0};

  // Set pin to output.
  pi_mmio_set_output(DHT_PIN);

  // Bump up process priority and change scheduler to try to try to make process more 'real time'.
  set_max_priority();

  // Set pin high for ~500 milliseconds.
  pi_mmio_set_high(DHT_PIN);
  sleep_milliseconds(500);

  // The next calls are timing critical and care should be taken
  // to ensure no unnecssary work is done below.

  // Set pin low for ~20 milliseconds.
  pi_mmio_set_low(DHT_PIN);
  busy_wait_milliseconds(20);

  // Set pin at input.
  pi_mmio_set_input(DHT_PIN);
  // Need a very short delay before reading pins or else value is sometimes still low.
  for (volatile int i = 0; i < 50; ++i) {
  }

  // Wait for DHT to pull pin low.
  uint32_t count = 0;
  while (pi_mmio_input(DHT_PIN)) {
    if (++count >= DHT_MAXCOUNT) {
      // Timeout waiting for response.
      set_default_priority();
      return DHT_ERROR_TIMEOUT;
    }
  }

  // Record pulse widths for the expected result bits.
  for (int i=0; i < DHT_PULSES*2; i+=2) {
    // Count how long pin is low and store in pulseCounts[i]
    while (!pi_mmio_input(DHT_PIN)) {
      if (++pulseCounts[i] >= DHT_MAXCOUNT) {
        // Timeout waiting for response.
        set_default_priority();
        return DHT_ERROR_TIMEOUT;
      }
    }
    // Count how long pin is high and store in pulseCounts[i+1]
    while (pi_mmio_input(DHT_PIN)) {
      if (++pulseCounts[i+1] >= DHT_MAXCOUNT) {
        // Timeout waiting for response.
        set_default_priority();
        return DHT_ERROR_TIMEOUT;
      }
    }
  }

  // Done with timing critical code, now interpret the results.

  // Drop back to normal priority.
  set_default_priority();

  // Compute the average low pulse width to use as a 50 microsecond reference threshold.
  // Ignore the first two readings because they are a constant 80 microsecond pulse.
  uint32_t threshold = 0;
  for (int i=2; i < DHT_PULSES*2; i+=2) {
    threshold += pulseCounts[i];
  }
  threshold /= DHT_PULSES-1;

  // Interpret each high pulse as a 0 or 1 by comparing it to the 50us reference.
  // If the count is less than 50us it must be a ~28us 0 pulse, and if it's higher
  // then it must be a ~70us 1 pulse.
  uint8_t data[5] = {0};
  for (int i=3; i < DHT_PULSES*2; i+=2) {
    int index = (i-3)/16;
    data[index] <<= 1;
    if (pulseCounts[i] >= threshold) {
      // One bit for long pulse.
      data[index] |= 1;
    }
    // Else zero bit for short pulse.
  }

  // Useful debug info:
  //printf("Data: 0x%x 0x%x 0x%x 0x%x 0x%x\n", data[0], data[1], data[2], data[3], data[4]);

  // Verify checksum of received data.
  if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
      // Calculate humidity and temp for DHT22 sensor.
      aData->humidity = (data[0] * 256 + data[1]) / 10.0f;
      aData->temperature = ((data[2] & 0x7F) * 256 + data[3]) / 10.0f;
      if (data[2] & 0x80) {
        aData->temperature *= -1.0f;
      }
    return DHT_SUCCESS;
  }
  else {
    return DHT_ERROR_CHECKSUM;
  }
}

int main()
{

  // turn everything on
  if (!pi_mmio_init() < 0)
  {
	return -1;
  }
  else
  {
    std::cout << " pi_mmio initialized " << std::endl;
  }

  pirTriggered = false;

  // start up dht sensor
  // Set GPIO dhtPin to output
  std::cout << " about to call set input " << std::endl;
  pi_mmio_set_input(DHT_PIN);

  std::cout << " DHT pin set " << std::endl;

  // startup spi
  spidata.mode = SPI_MODE_0;
  spidata.bitsPerWord = 8;
  spidata.speed = 1000000;
  spidata.spifd = -1;

  char adc08636[] = "/dev/spidev0.0";

  std::cout << " about to call spiOpen " << std::endl;

  if(spiOpen(&spidata, &adc08636[0], sizeof(adc08636)) < 0) {
    std::cout << " can't open spidev0.0" << std::endl;
    return -1;
  }
	

  atmosphereData dhtData;
  unsigned char dataDump[255];
  unsigned char spiData[2];
  int soundValue;

  // Set RPI pin P1-15 to be an input
  pi_mmio_set_input(LIGHT_PIN);  

  // Set RPI pin P1-15 to be an input
  pi_mmio_set_input(PIR_PIN);

  std::cout << " starting loop " << std::endl;

  while(true)
  {

    // keep track of time,
    // check sound, check movement
    // if it's been long enough, throw it up to the sensor

    // check for event
    if (pi_mmio_input(LIGHT_PIN))
    {
      lightCount++;
    }

    stringstream ss;

    // Read some data
    uint8_t inPirValue = pi_mmio_input(PIR_PIN);

    if(inPirValue != pirValue)
    {
      pirTriggered = true;
    }

    time(&timev);

    ////// if we're ready to read //////
    if( timev - lastReadTime > 60 ) // one minute
    {
      std::cout << " it's been a minute " << std::endl;
      lastReadTime = timev;
    }
    else
    {
      continue; // hasn't been a minute? bail
    }
/*
    readDHT( &dhtData );

    ss << "t="<< dhtData.temperature << "&";

    ss << "h="<< dhtData.humidity << "&";
    soundValue = spiWriteRead( &spidata, &spiData[0], 2 );
    ss << "s="<< soundValue << "&";
    ss << "li=" << lightCount;
    ss << "m=" << (int) pirTriggered;
    // we dont really care what we're sending b/c we're only reading
    string hostname = "162.242.237.33";
    connectAndSend( hostname );
*/  

  }

  return 1;
}
