#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sstream>
#include <arpa/inet.h>
#include <ctime>
#include "spi.h"

extern "C" {
   #include "bcm2835.h"
}

#define PORT "3000" // the port client will be connecting to 
#define MAXDATASIZE 100 // max number of bytes we can get at once 
spiConfig *spiConfig;

#define LIGHT_PIN 15
#define PIR_PIN 11
#define MAXTIMINGS 100
#define DHT_PIN 12;

using namespace std;

int pirValue;
bool pirTriggered;
time_t  timev;
unsigned long lastReadTime;

struct atmosphereData {
  char temp[2];
  char humidity[2];
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

    memset(&hints, 0, sizeof hints);
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
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';

    printf("client: received '%s'\n",buf);

    close(sockfd);
}


int readDHT(atmosphereData* aData)
{
  int counter = 0;
  int laststate = HIGH;
  int j = 0;
  bcm2835_gpio_write(DHT_PIN, HIGH);
  usleep(500000);
  bcm2835_gpio_write(DHT_PIN, LOW);
  usleep(20000);
  bcm2835_gpio_fsel(DHT_PIN, BCM2835_GPIO_FSEL_INPT);

  data[0] = data[1] = data[2] = data[3] = data[4] = 0;

  // wait for dhtPin to drop
  while(bcm2835_gpio_lev(dhtPin) != 0) 
  {
    bcm2835_delay(1000);
  }

  // read data!
  for (int i=0; i< MAXTIMINGS; i++) {
    counter = 0;
    while ( bcm2835_gpio_lev(DHT_PIN) == laststate) {
        counter++;
        //nanosleep(1);   // overclocking might change this?
        if (counter == 1000) {
           break;
         }
    }
    laststate = bcm2835_gpio_lev(DHT_PIN);
    if (counter == 1000) break;
    bits[bitidx++] = counter;

    if ((i>3) && (i%2 == 0)) {
      // shove each bit into the storage bytes
      data[j/8] <<= 1;
      if (counter > 200) {
        data[j/8] |= 1;
      }
      j++;
    }
  }
}

int main()
{

  // turn everything on
  if (!bcm2835_init())
  {
	return 1;
  }

  pirTriggered = false;

  // start up dht sensor
  // Set GPIO dhtPin to output
  bcm2835_gpio_fsel(DHT_PIN, BCM2835_GPIO_FSEL_OUTP);

  // startup spi
  spiConfig->mode = SPI_MODE_0;
  spiConfig->bitsPerWord = 8;
  spiConfig->speed = 1000000;
  spiConfig->spifd = -1;

  const char adc08636[] = "/dev/spidev0.0";

  if(spiOpen(spiConfig, &adc08636[0], sizeof(adc08636))) < 0) {
    perror(" can't open spidev0.0");
  }

  atmosphereData dhtData;
  unsigned char dataDump[255];
  unsigned char spiData[2];
  int soundValue;

  // Set RPI pin P1-15 to be an input
  bcm2835_gpio_fsel(LIGHT_PIN, BCM2835_GPIO_FSEL_INPT);
  // enable rising edge
  bcm2835_gpio_ren(LIGHT_PIN);

  // Set RPI pin P1-15 to be an input
  bcm2835_gpio_fsel(PIR_PIN, BCM2835_GPIO_FSEL_INPT);
  //  with a pullup
  bcm2835_gpio_set_pud(PIR_PIN, BCM2835_GPIO_PUD_UP);

  while()
  {

    /*

      keep track of time,
      check sound, check movement
      if it's been long enough, throw it up to the sensor
     */

    // check for event
    if (bcm2835_gpio_eds(LIGHT_PIN))
    {
      lightCount++;
      // Now clear the eds flag by setting it to 1
      bcm2835_gpio_set_eds(LIGHT_PIN);
    }

    stringstream ss;

    // Read some data
    uint8_t inPirValue = bcm2835_gpio_lev(PIR_PIN);

    if(inPirValue != pirValue)
    {
      pirTriggered = true;
    }

    time(&timev);

    ////// if we're ready to read //////
    if( timev.time - lastReadTime > 60 ) // one minute
    {
      lastReadTime = timev.time;
    }
    else
    {
      continue; // hasn't been a minute? bail
    }

    readDHT( dhtPin, &dhtData );

    ss << "t="<< dhtData.temp << "&";

    ss << "h="<< dhtData.humidity << "&";
    soundValue = spiWriteRead( spiConfig, &spiData[0], 2 );
    ss << "s="<< soundValue << "&";
    ss << "li=" << lightCount;

    // we dont really care what we're sending b/c we're only reading
    string hostname = "162.242.237.33";
    connectAndSend( hostname );
    

  }

  bcm2835_close();
  return 1;
}
