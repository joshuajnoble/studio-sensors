#include <stdio.h>
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
#define DHT_PIN 12

using namespace std;

int pirValue;
bool pirTriggered;
time_t  timev;
unsigned long lastReadTime;
unsigned int lightCount;

int main()
{

  // turn everything on
  if (!pi_mmio_init() < 0)
  {
    std::cout << " can't start " << std::endl;
  	return -1;
  }

  // Set RPI pin P1-15 to be an input
  pi_mmio_set_input(LIGHT_PIN);  

  // Set RPI pin P1-15 to be an input
  pi_mmio_set_input(PIR_PIN);

  return 1;
}