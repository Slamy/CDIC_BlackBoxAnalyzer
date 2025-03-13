# CDIC Black Box Analyzer

The goal of this project is a black box analysis of the IMS66490 CD-Interface Controller (CDIC).

[Findings of these project can be found here](doc)

## Current state of community efforts

The datasheet of this chip is not public, resulting into a lack of information.
The author of the closed source [CD-i Emulator](https://www.cdiemu.org/) already provided [some info based on reverse engineering the CDIC driver](https://github.com/cdifan/cdichips/blob/master/ims66490cdic.md).
An open source implementation of the CDIC is partially based on these findings and is available in the [CD-i emulation core of MAME](https://github.com/mamedev/mame/blob/master/src/mame/philips/cdicdic.cpp).

CDIC emulation is still plagued with issues:
* Sometimes audio is not played
* ADPCM audio tracks play one audio sector to late after the audio map is used

## Compiling under Linux

The previous Windows oriented build system was changed to building on Linux.
If building for Windows is desired, the previous build system needs to be restored and tested on a Windows machine.

### Prerequisites

A platform to run this application on.
Not every CD-i uses a CDIC. The intended platform is the "Mono I" hardware:
* CD-i Emulator
* A real Philips CD-i with Mono I PCB (eg. 210/05)
* MAME is currently not supported as the UART emulation is missing for printing

Dosbox is required for the MSDOS based mastering tools.
Wine is required to execute the Win32 based compiler and linker.
Copy cdilink.exe to this folder for starting the application on a CD-i.
A [Philips CD-i to PC Null-Modem Cable](http://www.icdia.co.uk/docs/cdi_nullmodem.jpg)

Use winecfg to mount cdi-sdk as drive D:

### Compiling for serial stub

	WINEPATH=D:/DOS/BIN wine D:/dos/bin/bmake.exe link_app

### Compiling for CD booting

Keep in mind that this application is designed to be started with
already existing CDs in mind. I don't recommend to burn this as a CD
as you will have to change the software very often.

Please use this only with emulators that lack a serial port emulation!
Ensure to add a delay at the start and change the CD during execution.

	WINEPATH=D:/DOS/BIN wine D:/dos/bin/bmake.exe link_cd

### Starting via serial stub on real CD-i

Also starts a minicom terminal for test output

	wine cdilink.exe -port 5 -n -a 8000 -d build/cdictest.app -e && minicom -D /dev/ttyUSB0 -b 9600

There is also a script for quality of life during development.
It compiles the code, stops the current instance of the test
and starts the next one using the stub loader.
This has the advantage that the CD-i must not be manually power cycled
for the next run.

	./stub_load.sh

### Cleanup

	WINEPATH=D:/DOS/BIN wine D:/dos/bin/bmake.exe clean

### Mastering into CDI/TOC file

	dosbox master.bat -exit

The resulting image can be loaded into cdiemu

### Compiling, mastering and CUE/BIN conversion in one step

This approach is crude and might not work on all machines.
It makes use of xdotool to automate button presses.

	./make_image.sh

### Start image on MAME

	mame cdimono1 -cdrom disk/CDICTEST.CUE

### Start image on cdiemu

This also shows the UART window for printed outputs

	wine wcdiemu-v053b8.exe CDICTEST.CDI -playcdi -start -term uart

### Copy to MiSTer

	scp disk/CDICTEST.CUE disk/CDICTEST.BIN root@mister:/media/fat/games/CD-i

## Resources

* The project was started as a fork of the game [Nobelia](https://github.com/TwBurn/Nobelia).
  Thanks go out to TwBurn for providing the source code to help getting into CD-i development
* Some [register descriptions by cdifan](https://github.com/cdifan/cdichips/blob/master/ims66490cdic.md)
* [CDIC code of MAME](https://github.com/mamedev/mame/blob/master/src/mame/philips/cdicdic.cpp)
