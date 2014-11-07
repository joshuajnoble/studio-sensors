#!/bin/bash
sudo modprobe spi_bcm2708
sudo pigpiod

STUDIO=SEA
ZONE=0

sudo python /home/pi/frog-studio-sensors/studio_sensors.py -s $STUDIO -z $ZONE > studio_sensors_log&
