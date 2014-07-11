#pragma once

#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>


struct spiConfig {

	unsigned char mode;
	unsigned char bitsPerWord;
	unsigned int speed;
	int spifd;

};

int spiOpen(spiConfig *spic, char *spiDevice, int length);
int spiClose(spiConfig *spic );
int spiWriteRead( spiConfig *spic, unsigned char * data, int length );
