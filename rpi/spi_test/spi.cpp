#include "spi.h"


int spiOpen(spiConfig *spic, char *spiDevice, int length)
{

	int statusVal = -1;
	spic->spifd = open(spiDevice, O_RDWR);
	if(spic->spifd < 0)	// did we open the device?
	{
		perror("could not open SPI device");
		return -1;
	}

	statusVal = ioctl(spic->spifd, SPI_IOC_WR_MODE, &(spic->mode));

	if(statusVal < 0)
	{
		perror("couldn't set SPI write mode");
		return -1;
	}

	statusVal = ioctl(spic->spifd, SPI_IOC_RD_MODE, &(spic->mode));
	if(statusVal < 0)
	{
		perror("couldn't set SPI read mode");
		return -1;
	}

	statusVal = ioctl(spic->spifd, SPI_IOC_WR_BITS_PER_WORD, &(spic->bitsPerWord));
	if(statusVal < 0)
	{
		perror("couldn't set SPI bitsPerWord write");
		return -1;
	}

	statusVal = ioctl(spic->spifd, SPI_IOC_RD_BITS_PER_WORD, &(spic->bitsPerWord));
	if(statusVal < 0)
	{
		perror("couldn't set SPI bitsPerWord read");
		return -1;
	}

	statusVal = ioctl(spic->spifd, SPI_IOC_WR_MAX_SPEED_HZ, &(spic->speed));
	if(statusVal < 0)
	{
		perror("couldn't set SPI bitsPerWord read");
		return -1;
	}

	statusVal = ioctl(spic->spifd, SPI_IOC_RD_MAX_SPEED_HZ, &(spic->speed));
	if(statusVal < 0)
	{
		perror("couldn't set SPI bitsPerWord write");
		return -1;
	}

	return statusVal;
}

int spiClose(spiConfig *spic)
{
	int statusVal = -1;
	statusVal = close(spic->spifd);
	if(statusVal < 0)
	{
		perror(" couldn't close device ");
		return -1; // should probbaly exit program??
	}

	return statusVal;
}

// returns 0-1023
int spiWriteRead( spiConfig *spic, unsigned char * data, int length )
{
	struct spi_ioc_transfer spi[length];
	int i = 0;
	int retVal = -1;

	for( i = 0; i < length; i++)
	{

		spi[i].tx_buf = (unsigned long) (data+i);
		spi[i].rx_buf = (unsigned long) (data+i);
		spi[i].len = sizeof(*(data+i));
		spi[i].delay_usecs = 0;
		spi[i].speed_hz = spic->speed;
		spi[i].bits_per_word = spic->bitsPerWord;
		spi[i].cs_change = 0;
	}

	retVal = ioctl( spic->spifd, SPI_IOC_MESSAGE(length), &spi);
	if(retVal < 0)
	{
		perror(" can't transmit spi data");
	}
	return ((data[0] << 7) | (data[1] >> 1)) & 0x3FF; // get a nice number
}

