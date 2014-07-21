//
// compiled for g++
// 

extern "C" {
	#include "/usr/local/include/bcm2835.h"
}
#include <iostream>
#include <time.h>

int lightCount;
#define PIN RPI_V2_GPIO_P1_26


long int last_heartbeat;
long int heartbeat_difference;
struct timespec gettime_now; 

int main()
{

	std::cout << " tsl 235 " << std::endl;


    if (!bcm2835_init())
		return 1;


	lightCount = 0;

	// Set RPI pin P1-15 to be an input
	bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_INPT);
	
	std::cout << " ok fsel ok " << std::endl;

	// enable rising edge
	bcm2835_gpio_ren(PIN);
	
	std::cout << " tsl 235 " << std::endl;

	while(true)
	{
	    // check for event
	    if (bcm2835_gpio_eds(PIN))
	    {
	      lightCount++;
	      // Now clear the eds flag by setting it to 1
	      bcm2835_gpio_set_eds(PIN);
		}
	 
		clock_gettime(CLOCK_REALTIME, &gettime_now);
		heartbeat_difference = gettime_now.tv_nsec - last_heartbeat;		//Get nS value
		
		if (heartbeat_difference < 0) {
			heartbeat_difference += 1000000000;				//(Rolls over every 1 second)
		}

		if (heartbeat_difference > 1000000) {
			lightCount = 0;
			last_heartbeat += 1000000;
		}
	}
	return 1;
}
