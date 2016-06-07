[![GPL Licence](https://badges.frapsoft.com/os/gpl/gpl-150x33.png?v=102)](https://opensource.org/licenses/GPL-2.0/) 
![Open Source Love](https://badges.frapsoft.com/os/v1/open-source-150x25.png?v=102)
[![Gitter](https://badges.gitter.im/beagleboard/beagle-gsoc.svg)](https://gitter.im/beagleboard/beagle-gsoc?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)

[![Travis](https://travis-ci.org/ZeekHuge/BeagleScope.svg)](https://travis-ci.org/ZeekHuge/BeagleScope)

    

# BeagleScope
The official repository for the GSoC-2016 project - [BeagleScope](https://zeekhuge.github.io/beaglescope.html). An introductory video to the project - [video](https://youtu.be/tdanTRSmq4E).

## What is BeagleScope ?
The project, as the part of GSoC-2016, aims to provide with software support to deploy the PRUSS subsystem on AM33xx processors, as a fast and parallel data acquisition units.The PRU cores being highly optimized for I/O operations and single cycle instruction execution (mostly), are an appropriate choice for bit-banging and offloading tasks. BeagleScope uses this very feature of PRUs to offload the parallel data transactions and fast data acquisition. This project provides:

1. Kernel support :	Kernel support will be in the form of an offload subsystem and a kernel module that will expose PRUs as 'parallel I/O bus'. The subsystem would manage all the transactions of the I/O unit, while the parallel bus unit will manage communication with the PRU core.
2. Firmware : The firmware will be the code for PRUs. The firmware would manage the communication on the PRU side and all the data I/O that PRU need to do. 

The overall software stack would allow developers to use PRUs for many cool applications. One of such applications is an Oscilloscope. This can be achieved using a fast ADC along with the developed software. This project will also try to develop a 20MHz oscilloscope as a test for the developed code. 

## This Repository

So here's a detail description of the contents of the repository.
#### - examples
It contains some pru-related-examples. The source code is mostly based on the TI's pru support package ( http://git.ti.com/pru-software-support-package/pru-software-support-package ) . The examples essentially show the configuration and steps that need to be followed in order to get started working with PRUs. All the examples contain a deploy.sh script that executes configuration steps.
These examples programs were tested and developed on 4.4.11-ti-r29 kernel version.
Some initial configuration steps :

    mkdir /usr/share/ti/cgt-pru/bin
    ln -s /usr/bin/clpru /usr/share/ti/cgt-pru/bin/clpru
    export PRU_CGT=/usr/share/ti/cgt-pru
    
##### ----pru_blinky
pru_blinky is a gpio toggle program . 
To use it, 
    
    edit the HEADER, PIN_NUMBER and PRU_CORE variables in deploy.sh
    execute deploy.sh
    
Note that the PIN that will be derived from HEADER and PIN_NUMBER should be mux-able to the PRU_CORE
    
##### ----pru_triggered_output
This example takes input from one pin and copies the logical state of that input pin to the output pin.
To use it,

    edit the OUT_HEADER, OUT_PIN_NUMBER, IN_HEADER, IN_PIN_NUMBER, PRU_CORE variables in deploy.sh
    execute deploy.sh

Note that the PINs that will be derived from IN/OUT_HEADER and IN/OUT_PIN_NUMBER must belong to same PRU_CORE and be mux-able to it.

## License
The kernel code is released under the [GPLv2](https://opensource.org/licenses/GPL-2.0/) license.
