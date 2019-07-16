#! /bin/bash

##############################################################################
#
# Copyright (C) 2016 Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
#
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#	* Redistributions of source code must retain the above copyright
#	  notice, this list of conditions and the following disclaimer.
#
#	* Redistributions in binary form must reproduce the above copyright
#	  notice, this list of conditions and the following disclaimer in the
#	  documentation and/or other materials provided with the
#	  distribution
#
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#############################################################################


# The script builds the pru_blinky project and configures the pinmuxing for $HEADER$PIN_NUM

#If changing these variables, make sure the given pin can be muxed to the given pru.  
HEADER=P8_
PIN_NUMBER=45
#PRU_CORE should be either 0 or 1
PRU_CORE=1

echo "*******************************************************"
echo "This must be compiled on the BEAGLEBONE BLACK itself"
echo "It was tested on 4.4.12-ti-r31 kernel version"
echo "The source code for blinky ie PRU_gpioToggle was taken from"
echo "pru-software-support-package and can be cloned from"
echo "git clone git://git.ti.com/pru-software-support-package/pru-software-support-package.git"
echo "******NOTE: use a resistor >470 ohms to connect to the LED, I have alredy made this mistake."
echo "To continue, press any key:"
read

echo "-Building project"
	cd PRU_gpioToggle
	make clean
	make

echo "-Placing the firmware"
	cp gen/*.out /lib/firmware/am335x-pru$PRU_CORE-fw

echo "-Configuring pinmux"
	config-pin -a $HEADER$PIN_NUMBER pruout
	config-pin -q $HEADER$PIN_NUMBER

echo "-Rebooting"
	if [ $PRU_CORE -eq 0 ]
	then
	    echo "Rebooting pru-core 0"
	    echo 'stop' > /sys/class/remoteproc/remoteproc1/state 2>/dev/null
	    echo "am335x-pru$PRU_CORE-fw" > /sys/class/remoteproc/remoteproc1/firmware
	    echo 'start' > /sys/class/remoteproc/remoteproc1/state
	else
	    echo "Rebooting pru-core 1"
	    echo 'stop' > /sys/class/remoteproc/remoteproc2/state 2>/dev/null
	    echo "am335x-pru$PRU_CORE-fw" > /sys/class/remoteproc/remoteproc2/firmware
	    echo 'start' > /sys/class/remoteproc/remoteproc2/state
	fi

echo "Done. Blikny must be up on pin $HEADER$PIN_NUMBER"
