#include "spi.h"
#include <iostream>

spiConfig config;

int main()
{



  config.mode = SPI_MODE_0;
  config.bitsPerWord = 8;
  config.speed = 1000000;
  config.spifd = -1;

  char adc08636[] = "/dev/spidev0.0";

  if(spiOpen(&config, &adc08636[0], sizeof(adc08636)) < 0) {
    std::cout << " can't open spidev0.0" << std::endl;
  }
}
