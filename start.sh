#!/bin/bash
sudo modprobe spi_bcm2708
sudo pigpiod

sudo python /home/pi/studio-sensors/studio_sensors.py > studio_sensors_log&
