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

## This Branch - "port_to_4.4.12-ti-r31+" 
In kernel version 4.4.12-ti-r31, the RPMsg framework used ARM INTC in place of Mailboxes to generate kicks from, and to PRUs. This was a significant change and the code needed to be modified to work with this new RPMsg framework.
This branch has the code that works with kernel later than 4.4.12-ti-r31. To get examples for older version, checkout the [master branch](https://github.com/ZeekHuge/BeagleScope/tree/master).

## firmware/
firmware directory contains BeagleScope firmware code for PRUs (under developement).
BeagleScope uses these two PRU cores to sample data and transfer it to the Main Processing Unit (MPU).
##### Current state -
1. Samples 44 bytes of data in one cycle.
2. Transfers data to PRU0 after each cycle.
3. Utilizes all 3 scratch pad banks in cycle starting from bank0.
4. Sampling frequency can be configured from userspace.
5. Sampling can be stopped and started from userspace.

##### Testing the code - 
For testing pupose, the code has a FAKE_DATA constant defined in ''common_pru_defs.h'' file. The value of this data is used as the value of input sample. Further, the functioning of the clock pin also changes. In normal working condition, clock pin is first signaled HIGH, a sample is taken, and the clock pin is brought back to low.  While testing, the clock pin toggles whenever a sample is taken.

To start testing :
1. Clone the repository
2. Move into BeagleScope/firmware/ directory
3. Apply test.patch

        $ patch -p2 < test.patch
4. Build and install the code

        $ make install
At this point, a character device file, rpmsg-pru30 must appear at /dev/rpmsg-pru30. This file will be used to write configuration settings to PRU.
So, we need to send 2 messages.
 * The first message will be the number of cycles to delay between each sample (this value should always be even)
 * The second message should have its 32nd bit set ( 1<<31 ) to start sampling and 32nd bit cleared, to stop sampling

For example, keeping a delay of 100000001 cycles
* 10000001 = 0x00989681
* To send this data above character device file, take byte at a time, starting with the most significant byte
* The message for 0x00989681 turns out to be '\x81\x96\x98\x00'

To send this message :
        
        $ echo -e '\x81\x96\x98\x00' >/dev/rpmsg_pru30
further, the second message we need to send is 1<<31 , which turns out to be 0x80000000. So the message for this is '\x00\x00\x00\x80'
To send this message and see the output data from the pru:

        $ echo -e '\x00\x00\x00\x80' >/dev/rpmsg_pru30 && xxd -c 4 /dev/rpmsg_pru30 

PRU will first echo the message back, and then start printing sets of 44 FAKE_DATA.

To stop sampling :

Send anything as the first message

        $ echo -e '\x81\x96\x98\x00' >/dev/rpmsg_pru30
and then send this as the 2nd message :

        $ echo -e '\x00\x00\x00\x00' >/dev/rpmsg_pru30


## License
The kernel code is released under the [GPLv2](https://opensource.org/licenses/GPL-2.0/) license.

