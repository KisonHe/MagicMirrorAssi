#!/bin/bash
export DISPLAY=:0 #change this to auto detect later

# note
# Put host and port in .env and edit them
# launch chromium
#/usr/bin/chromium-browser -noerrdialogs -kiosk -start_maximized  --disable-infobars --app=$host:$port  --ignore-certificate-errors-spki-list --ignore-ssl-errors --ignore-certificate-errors 2>/dev/null
/usr/bin/chromium-browser -noerrdialogs -kiosk -start_maximized  --disable-infobars --app=$host:$port  2>/dev/null

# force refresh page around a time
# [TODO]

# shutdown gracely, in case schools cuts power.
# shutdown -h 23:00

# https://www.raspberrypi.org/documentation/raspbian/applications/vcgencmd.md
# vcgencmd display_power 0 # turn off 
# vcgencmd display_power 1 # turn on