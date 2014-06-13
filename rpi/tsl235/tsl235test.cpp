
// compiled for g++
// 

#include <iostream>
#include <bcm2835.h>
#include <time.h>

int lightCount;
const int LIGHT_PIN = 15;

long int last_heartbeat;
long int heartbeat_difference;
struct timespec gettime_now; 

int main()
{

	lightCount = 0;

	// Set RPI pin P1-15 to be an input
	bcm2835_gpio_fsel(LIGHT_PIN, BCM2835_GPIO_FSEL_INPT);
	// enable rising edge
	bcm2835_gpio_ren(LIGHT_PIN);

	while(true)
	{
	    // check for event
	    if (bcm2835_gpio_eds(LIGHT_PIN))
	    {
	      lightCount++;
	      // Now clear the eds flag by setting it to 1
	      bcm2835_gpio_set_eds(LIGHT_PIN);
		}
	 
		clock_gettime(CLOCK_REALTIME, &gettime_now);
		heartbeat_difference = gettime_now.tv_nsec - last_heartbeat;		//Get nS value
		
		if (heartbeat_difference < 0) {
			heartbeat_difference += 1000000000;				//(Rolls over every 1 second)
		}

		if (heartbeat_difference > 1000000) {
			std::cout << lightCount << std::endl;
			lightCount = 0;
			last_heartbeat += 1000000;
		}
	}
	return 1;
}