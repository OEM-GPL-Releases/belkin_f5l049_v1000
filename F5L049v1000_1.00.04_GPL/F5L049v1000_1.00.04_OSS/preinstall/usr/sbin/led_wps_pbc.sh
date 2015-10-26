#!/bin/sh

echo -n "$$" > /var/run/led_wps_pbc.pid
sleep $1
echo -n "100 1F 6 3 0 0 0" > /proc/silex/led/6
