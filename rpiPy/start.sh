#!/bin/bash

sudo pigpiod
python studio_sensors.py > studio_sensors_log
