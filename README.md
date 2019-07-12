[![GPL Licence](https://badges.frapsoft.com/os/gpl/gpl-150x33.png?v=102)](https://opensource.org/licenses/GPL-2.0/) 
![Open Source Love](https://badges.frapsoft.com/os/v1/open-source-150x25.png?v=102)
[![Gitter](https://badges.gitter.im/beagleboard/beagle-gsoc.svg)](https://gitter.im/beagleboard/beagle-gsoc?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)

[![Travis](https://travis-ci.org/ZeekHuge/BeagleScope.svg)](https://travis-ci.org/ZeekHuge/BeagleScope)

[![HitCount](http://hits.dwyl.io/zeekhuge/BeagleScope.svg)](http://hits.dwyl.io/zeekhuge/BeagleScope)



# BeagleScope
The official repository for the GSoC-2016 project - [BeagleScope](https://zeekhuge.github.io/beaglescope.html). An introductory video to the project - [video](https://youtu.be/tdanTRSmq4E).

**Note - The PRU support on the kernel side changes more often than it should (of course to make it more advanced and add better support). I'll try to keep the repo up to date (PRs welcomed). To use the repository, look into the available tags (they represent supported/tested kernel version). If the kernel version you are looking for is not present, try using the latest/closest kernel version among those which are available, or even better, make the required changes and submit a PR. (PRs ARE AWESOME)**

**Note 2 - The default branch for this repository has been changed to `master`, from the `port_to_4.4.12-ti-r31+` branch. This change might break the `git pull origin` command, as it fetches the default branch. The easiest way to make it work is delete your local repo, and clone this repo from scratch.**

---
**Poject has examples to explain use of remoteproc drivers and RPMSG framework for the PRUs.** :https://github.com/ZeekHuge/BeagleScope/tree/master/examples

(Just a side note - You'll probably like https://www.zeekhuge.me)

---

## What is BeagleScope ?
The project, as the part of GSoC-2016, aims to provide with software support to deploy the PRUSS subsystem on AM33xx processors, as a fast and parallel data acquisition units. The project is a good example of using generic PRU-remoteproc drivers and the RPMSG framework for PRUs. The PRU cores being highly optimized for I/O operations and single cycle instruction execution (mostly), are an appropriate choice for bit-banging and offloading tasks. BeagleScope uses this very feature of PRUs to offload the parallel data transactions and fast data acquisition. This project provides:

1. Kernel support :	Kernel support will be in the form of an offload subsystem and a kernel module that will expose PRUs as 'parallel I/O bus'. The subsystem would manage all the transactions of the I/O unit, while the parallel bus unit will manage communication with the PRU core.
2. Firmware : The firmware will be the code for PRUs. The firmware would manage the communication on the PRU side and all the data I/O that PRU need to do. 

The overall software stack would allow developers to use PRUs for many cool applications. One of such applications is an Oscilloscope. This can be achieved using a fast ADC along with the developed software. This project will also try to develop a 20MHz oscilloscope as a test for the developed code. 

---

## Using this repo 
The repo has tags representing the kernel version this code has been tested upon. To be able to use this repo, you will have to find the kernel version running on your board using command `uname -r`. Once you know the kernel version, look into the tags (command : `git tag`) and find for the tag name closest to your kernel version. Once decide on what tag to use, checkout the repo on the tag (command `git checkout <tag-name>`). Now you can use the repo (and it will probably work).

---

## Contents

- #### docs/
    The direcotry contains the documents and notes that have been used or created along the main development process.

- #### driver/
    The directory contains all the kernel linux related code for the main project. It contains following driver source
        
    1. parallel_interface
    2. pi-bus 
    3. beaglescope_driver

- #### dtc/
    The directory contains the source for the device tree overlays that need to be loaded for the software stack to work correctly.

- #### examples/
    Directory that contains the examples that have been developed while learning PRU programming.

- #### firmware/
    The directory contains the PRU firmware that needs to be compiled and installed to booted onto the PRUs to serve as a parallel interfacing bus.

---

## Build the source on BBB/BBG itself

- Clone this repo.

    `$ git clone https://github.com/ZeekHuge/BeagleScope.git`
- Install linux header files

    `$ sudo apt-get install linux-headers-$(uname -r)`
- Enter into BeagleScope directory
    
    `$ cd BeagleScope`
    - To compile firmwae, enter into 'firmware' directory
        
        `$ cd firmware`
        - To compile the firmware
            
            `$ make`
        - To boot the PRUs with this firmware
            
            `$ make install`
    - To compiel drivers, enter into the 'driver' directory
        
        `$ cd driver`
        - To compile the drivers
            
            `$ make`
        - To load the drivers

            `$ make load`
    - To compile the device tree overlay, enter into the 'dtc' directory
    
        `$ cd dtc`
        - To compile the dts
        `$ dtc -O dtb /lib/firmware/PI-TEST-00A0.dtbo -b 0 -@ PI-TEST-00A0.dts`
        
        - To overlay the dts once its compiled
        '$ echo PI-TEST > /sys/devices/platform/bone_capemgr/slots'

**If you want to cross compile the source, rather than compiling it on the board itself, most of the things remain same and you can use the 'Kernel Development' section of [this post](https://www.zeekhuge.me/post/a_handfull_of_commands_and_scripts_to_get_started_with_beagleboneblack/) for the setup part.**

---

## How to use releases:
To use a release you will have to compile its source.
- [Release v0.1](https://github.com/ZeekHuge/BeagleScope/releases)
    - Clone this repo.

        `$ git clone https://github.com/ZeekHuge/BeagleScope.git`
    - Enter into the `BeagleScope` directory.
    
        `$ cd BeagleScope`
    - Create a new branch using tag v0.1.
        
        `$ git checkout v0.1 -b use_branch`
    - Ignore the README of that branch and compile the source using following make targets.
        - To compile firmware on the beaglebone black/green itself:
            
            - `$ cd firmware`
            - `$ make` - To compile the firmware.
            - `$ make install` - To compile as well as install the firmware.
        
        - To comiple the driver on the beaglebone black/green itslef:

            - `$ cd driver`
            - `$ make` - To compile the driver.
            - `$ make load` - To compile as well as load the driver.

**If you want to cross compile the source, rather than compiling it on the board itself, most of the things remain same and you can use the 'Kernel Development' section of [this post](https://www.zeekhuge.me/post/a_handfull_of_commands_and_scripts_to_get_started_with_beagleboneblack/) for the setup part.**

---

## Want this fast ? Help me develope it.
See the [Quickstart](https://github.com/ZeekHuge/BeagleScope/blob/port_to_4.4.12-ti-r31%2B/quickstart.md) to get started quickly and contribute to the project.

---
## License
The kernel code is released under the [GPLv2](https://opensource.org/licenses/GPL-2.0/) license.

