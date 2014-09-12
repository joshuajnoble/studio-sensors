#include "spi.h"
#include <iostream>
#include <time.h>
#include <sys/time.h>


spiConfig config;
int average[64];

int main()
{

  int channel = 1;

  config.mode = SPI_MODE_0;
  config.bitsPerWord = 8;
  config.speed = 500000;
  config.spifd = -1;

  char adc08636[] = "/dev/spidev0.0";

  if(spiOpen(&config, &adc08636[0], sizeof(adc08636)) < 0) 
  {
    std::cout << " can't open spidev0.0" << std::endl;
  }

  int count = 0;

  while( true )
  {
		unsigned char data[3];
		data[0] = 1;
		data[1] = 0b10000000 | (((channel & 7) << 4));
		data[2] = 0;

		int result = spiWriteRead( &config, &data[0], 3 );
		
		#ifdef AVERAGING
		average[count] = result;
		count++;

		if(count > 63)
		{
			count = 0;
		}

		int averaged = 0;
		for( int i = 0; i < 64; i++) {
			averaged += average[i];
		}
		
		averaged /= 64;
        std::cout << averaged << std::endl;
		#else
		std::cout << result << std::endl;
		#endif		

		//sleep(2);
		usleep(500000);
  }
  
  return 1;

}
