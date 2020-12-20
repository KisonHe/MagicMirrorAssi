# note
# this is public repo so dont write the host and port in sh. Put them in .env and edit them
# launch chromium
chromium-browser -noerrdialogs -kiosk -start_maximized  --disable-infobars --app=$host:$port  --ignore-certificate-errors-spki-list --ignore-ssl-errors --ignore-certificate-errors 2>/dev/null


# force refresh page around a time
# [TODO]

# shutdown gracely if need to, in case schools cuts power.
# [TODO]
# put args in .env

# https://www.raspberrypi.org/documentation/raspbian/applications/vcgencmd.md
vcgencmd display_power 0 # turn off 
vcgencmd display_power 1 # turn on