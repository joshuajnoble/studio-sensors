
#include "pi_dht_read.h"

#include <sys/time.h>
#include <time.h>
#include <iostream>
#include <unistd.h>


int main()
{

	int sensor, pin;
	float humidity, temperature;

	sensor = 22;
	pin = 18;

	while(true) {
    	sleep(5);
	    int ret = pi_dht_read( sensor, pin, &humidity, &temperature );
		std::cout << ret << " " << humidity << " " << temperature << std::endl;
	}


}
