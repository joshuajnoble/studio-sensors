//
// compiled for g++
// 

extern "C" {
	#include <bcm2835.h>
}
#include <iostream>
#include <time.h>

int lightCount;
#define PIN RPI_V2_GPIO_P1_26


long int last_heartbeat;
long int heartbeat_difference;
struct timespec gettime_now, gettime_last;

unsigned long int time_diff( struct timespec start, struct timespec stop )
{

	unsigned long int r1;
	r1 = (unsigned long int) (stop.tv_sec - start.tv_sec * 1000000000 + (stop.tv_nsec - start.tv_nsec));
	return r1;

}

int main()
{

	std::cout << " tsl 235 " << std::endl;

/*
    if (pi_mmio_init() != 0) {
		std::cout << " can't init pi mmio lib " << std::endl;
		return 1;
	}

	// enable rising edge
    pi_mmio_set_input( 7 );

	while(true)
	{
		//while( pi_mmio_input(7) != 0) {
			// wait for low;
		//}
		while( pi_mmio_input(7) == 0)
		{
			// in low
		}
		// now gettime to get rising edge
		clock_gettime(CLOCK_REALTIME, &gettime_last);
		while( pi_mmio_input(7) != 0)
		{
			// now waiting for falling edge to get whole cycle
			
		}
 		clock_gettime(CLOCK_REALTIME, &gettime_now);

		unsigned long int diff = time_diff(gettime_last, gettime_now); 
		std::cout << " diff " << diff << std::endl;

		sleep(1);	
	}
*/

	if(bcm2835_init())
	{
		bcm2835_gpio_afen( PIN );
		bcm2835_gpio_set_eds(PIN);
		int sample_count = 10;
		while( 1 )
		{

			clock_gettime(CLOCK_REALTIME, &gettime_last);
			for( int i = 0; i < sample_count; i++ )
			{
					while( bcm2835_gpio_eds(PIN) != HIGH )
					{
					}
					
					bcm2835_gpio_set_eds(PIN);
			}
			clock_gettime(CLOCK_REALTIME, &gettime_now);
			unsigned long int diff = time_diff(gettime_last, gettime_now); 
	   		std::cout << " diff in mhz " << diff << std::endl;
	   		//std::cout << " diff in mhz " << diff << std::endl;
			
			sleep(2);
	
		}

	}
	else
	{
		std::cout << " NO BCM ALL IS LOST " << std::endl;
	}

	return 1;
}
