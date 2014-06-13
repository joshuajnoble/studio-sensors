#include <iostream>
#include <bcm2835.h>

#define MAXTIMINGS 100
int dhtPin;

struct atmosphereData
{
  char temp[2];
  char humidity[2];
};

int readDHT(atmosphereData* aData) {
  int counter = 0;
  int laststate = HIGH;
  int j=0;

  // read data!
  for (int i=0; i< MAXTIMINGS; i++) {
    counter = 0;
    while ( bcm2835_gpio_lev(dhtPin) == laststate) {
      	counter++;
      	//nanosleep(1);		// overclocking might change this?
        if (counter == 1000) {
	         break;
         }
    }
    laststate = bcm2835_gpio_lev(dhtPin);
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
	atmosphereData dhtData;

	  // Set GPIO dhtPin to output
	bcm2835_gpio_fsel(dhtPin, BCM2835_GPIO_FSEL_OUTP);

	bcm2835_gpio_write(dhtPin, HIGH);
	usleep(500000);  // 500 ms
	bcm2835_gpio_write(dhtPin, LOW);
	usleep(20000);

	bcm2835_gpio_fsel(dhtPin, BCM2835_GPIO_FSEL_INPT);

	data[0] = data[1] = data[2] = data[3] = data[4] = 0;

	while(true)
	{
		// wait for dhtPin to drop
		if(bcm2835_gpio_lev(dhtPin) == 0) 
		{
			readDHT( 10, &dhtData );
			std::cout << dhtData.temp << " " dhtData.humidity << std::endl;
			bcm2835_delay(1000);
		}
	}
	return 1;
}