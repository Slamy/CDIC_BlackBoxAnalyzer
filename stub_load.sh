set -e

# Compile
WINEPATH=D:/DOS/BIN wine D:/dos/bin/bmake.exe link_app

# Stop previous execution if already running by sending Ctrl+C
stty -F /dev/ttyUSB0 19200 raw
echo -ne '\x03' > /dev/ttyUSB0
# The CD-i will now reset

# And go. Hopefully in time!
wine cdilink.exe -port 5 -n -a 8000 -d build/cdictest.app -e

# Have a terminal
rm -f log_vmpeg
minicom -D /dev/ttyUSB0 -b 19200 -C log_vmpeg
