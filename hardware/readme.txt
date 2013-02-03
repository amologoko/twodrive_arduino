*************************************************************************************
*																					*
* 						SparkFun Pro Micro Arduino Addons							*
*																					*
* 								by: Jim Lindblom									*
* 								SparkFun Electronics								*
* 								date: June 19, 2012									*
*																					*
*************************************************************************************

Unzip the ProMicro folder into the "hardware" directory of your Arduino sketchbook.
If there is not a hardware directory in your sketchbook, make one.

If you don't know where your sketchbook is located, you can find it's location by
going to File > Preferences within Arduino.

For Windows XP users, the sketchbook is usually located in your "My Documents/Arduino"
directory. So once unzipped, this readme should reside in "My Documents/Arduino/hardware/ProMicro".
Windows 7, by default, is something like: C:\Users\<user name>\Arduino\hardware\ProMicro

Once unzipped, close any Arduino windows and reopen. The following boards should
be listed in the Tools > Board menu now:
SparkFun Pro Micro 5V/16MHz
SparkFun Pro Micro 3.3V/8MHz

IMPORTANT: Make sure you select the option that matches your board. If you program a 5V Pro Micro
with 3.3V code (or vice-versa), the USB will not work (PINDV bit of PLLCSR register
will be incorrectly set) on the board, and you'll probably have to resort to 
re-flashing the bootloader using an ISP.

*************************************************************************************
*																					*
*	 						Explanation of Files Included							*
*																					*
*************************************************************************************

This addon should work with no need to modify files. If you're curious what we've done
to the default Arduino files though, read on...

* bootloaders/caterina: This is a slightly modified version of the Caterina
bootloader, which is used in the Arduino Leonardo. Defines were modified so the VID
and PID were set to SparkFun-specific values (VID: 0x1B4F, PIDs: 0x9205/9206 and 0x9203/9204).
To compile this, you'll need to download LUFA, and point the makefile to that directory.

To compile the bootloader for a Pro Micro, you must first set F_CPU and 
PID near the top of the makefile. F_CPU should be either 16000000 or 8000000, respectively
for 5V and 3.3V boards.  PID should be either 9205 or 9203 for respectively.

This directory also includes caterina-promicro8.hex and caterina-promicro16.hex,
which are the compiled images for both 8MHz and 16MHz Pro Micro boards. They won't
be deleted after running a 'make clean'.

* cores/arduino: This is a mostly unchanged listing of all the core files required by
the ProMicro to compile under Arduino. The files with changes are:
	USBCore.cpp: added conditional statement for seting PLLCSR.
	
* variants/promicro: This is identical to the Leonardo pins_arduino.h.

* boards.txt: This file is what Arduino looks at when populating its Tools > Board menu.
There are two board entires in this file:
SparkFun Pro Micro 5V/16MHz -and-
SparkFun Pro Micro 3.3V/8MHz
This also defines a few variables such as clock frequency, USB VID and PID, fuses, and
bootloader location.


*************************************************************************************
*																					*
*	 						Questions/Comments/Concerns?							*
*																					*
*************************************************************************************

Please let us know if this gives you any problems. You can contact our tech support team
(techsupport@sparkfun.com), or post a comment on the product page and we'll try to get
back to you as quickly as possible.


Have fun!
-Jim (jim at sparkfun.com)