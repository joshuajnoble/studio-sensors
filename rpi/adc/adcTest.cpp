#include <stdio.h>
#include "spi.h"


int main()
{

	if(spiOpen(spiConfig, &adc08636[0], sizeof(adc08636))) < 0) {
		std::cout <<  " can't open " << std::endl;

		getch(); // hang this here
		return 0;
	}

	std::cout <<  " can't open " << std::endl;

	spiConfig = (spiConfig*) malloc(sizeof(spiConfig));
	spiConfig->mode = SPI_MODE_0;
	spiConfig->bitsPerWord = 8;
	spiConfig->speed = 1000000;
	spiConfig->spifd = -1;

	while()
	{	
		soundValue = spiWriteRead( spiConfig, &spiData[0], 2 );
		delay(1000);
	}

	return 1;
}