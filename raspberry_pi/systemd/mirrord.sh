# note
# this is public repo so dont write the host and port in sh. Put them in .env and edit them
chromium-browser -noerrdialogs -kiosk -start_maximized  --disable-infobars --app=$host:$port  --ignore-certificate-errors-spki-list --ignore-ssl-errors --ignore-certificate-errors 2>/dev/null