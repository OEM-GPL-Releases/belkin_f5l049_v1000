#!/bin/sh

echo -n "$$" > /var/run/led_wps_pin.pid
sleep $1
echo -n "100 20 6 3 0 0 0" > /proc/silex/led/6
