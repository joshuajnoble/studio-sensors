#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "3000" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 
spiDevice *spiDevice;

struct atmosphereData {
  char temp[2];
  char humidity[2];
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int connectAndSend(int argc, char *argv[])
{
    int sockfd, numbytes;  
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(hostname, PORT, &hints, &servinfo)) != 0) {
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

int readVolume( int pin )
{
  spiWriteRead( )
}


int readDHT(int pin, atmosphereData* aData) {
  int counter = 0;
  int laststate = HIGH;
  int j=0;

  // Set GPIO pin to output
  bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);

  bcm2835_gpio_write(pin, HIGH);
  usleep(500000);  // 500 ms
  bcm2835_gpio_write(pin, LOW);
  usleep(20000);

  bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);

  data[0] = data[1] = data[2] = data[3] = data[4] = 0;

  // wait for pin to drop?
  while (bcm2835_gpio_lev(pin) == 1) {
    usleep(1);
  }

  // read data!
  for (int i=0; i< MAXTIMINGS; i++) {
    counter = 0;
    while ( bcm2835_gpio_lev(pin) == laststate) {
      	counter++;
      	//nanosleep(1);		// overclocking might change this?
        if (counter == 1000) {
	         break;
         }
    }
    laststate = bcm2835_gpio_lev(pin);
    if (counter == 1000) break;
    bits[bitidx++] = counter;

    if ((i>3) && (i%2 == 0)) {
      // shove each bit into the storage bytes
      data[j/8] <<= 1;
      if (counter > 200)
        data[j/8] |= 1;
      j++;
    }
  }


#ifdef DEBUG
  for (int i=3; i<bitidx; i+=2) {
    printf("bit %d: %d\n", i-3, bits[i]);
    printf("bit %d: %d (%d)\n", i-2, bits[i+1], bits[i+1] > 200);
  }
#endif

  printf("Data (%d): 0x%x 0x%x 0x%x 0x%x 0x%x\n", j, data[0], data[1], data[2], data[3], data[4]);

  if ((j >= 39) && (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) ) 
  {
     // yay!
 
  	float f, h;
  	h = data[0] * 256 + data[1];
  	h /= 10;

  	f = (data[2] & 0x7F)* 256 + data[3];
    f /= 10.0;
    if (data[2] & 0x80)  f *= -1;
    //printf("Temp =  %.1f *C, Hum = %.1f \%\n", f, h);
    itoa(f, aData->temp);
    itoa(h, aData->humidity);
    return 1;
  }

  return 0;
}

void main()
{
  spiDevice = malloc(sizeof(spiDevice));
  spiDevice->mode = SPI_MODE_0;
  spiDevice->bitsPerWord = 8;
  spiDevice->speed = 1000000;
  spiDevice->spifd = -1;

  char adc08636 = "/dev/spidev0.0";

  if(spiOpen(spiDevice, adc08636, sizeof(adc08636)) < 0) {
    perror(" can't open spidev0.0");
  }

  atmosphereData dhtData;
  unsigned char dataDump[255];
  unsigned char spiData[2];
  int soundValue;

  while()
  {

    /*

      keep track of time,
      check sound, check movement
      if it's been long enough, throw it up to the sensor

     */

    readDHT( dhtPin, &dhtData );
    spiWriteRead( &spiDevice, )
    // we dont really care what we're sending b/c we're only reading
    soundValue = connectAndSend( &spiData[0], 2 );
    

  }


}
