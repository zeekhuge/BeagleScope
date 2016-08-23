    
## **Examples** 
The examples are based on the pru-software-support-package provided by TI. These examples were tested on 4.4.12-ti-r31 kernel version and are based on newer version of RPMsg kernel module, that uses ARM INTC in place of mailboxes. This new RPMsg is available in kernel version 4.4.12-ti-r31 and later.

### -firmware_examples
Contains examples that are only related to the firmware code for both or one of the PRUs.
###### -pru_blinky
Its the most basic program. A good post related to this example can be found [here](https://www.zeekhuge.me/post/ptp_blinky/). This program directory contains ''deploy.sh'' script that compiles the code, copies the firmware to /lib/firmware/am335x-pru1(0)-fw, and reboots pru1(0)-core. To get blinky on a particular pin:
1. Find the name of the board output pin, its muxed to, let say P8_46
2. Find the pru-core the pin is connected to. P8_46 is connected to pru1.
3. Open ''deploy.sh'' and you would see:
            
        #If changing these variables, make sure the given pin can be muxed to the given pru.
        HEADER=P8_
        PIN_NUMBER=45
        #PRU_CORE should be either 0 or 1
        PRU_CORE=1
4. Change it according to point number 1 and 2. So we need P8_46 which is on core PRU 1, so the modifications will look like:
            
        #If changing these variables, make sure the given pin can be muxed to the given pru.
        HEADER=P8_
        PIN_NUMBER=46
        #PRU_CORE should be either 0 or 1
        PRU_CORE=1
5. Execute the script and *Whola* !! - blinky will be up there!

###### -pru_pin_state_reader
The program by default will continuously monitor board pin number P8_45 and will print "CHANGED" to user-space, if the logical state of the pin changes.
To make changes in the pin being monitored, changes will have to be made in ''deploy.sh'' script (see [pru_blinky](#-pru_blinky)) and in the value of CHECK_BIT in the firmware code.

To get the program working just execute the ''deploy.sh'' script
        
    $ ./deploy.sh
then to see userspae messages :

    $ echo s >/dev/rpmsg-pru30 && cat /dev/rpmsg-pru30 
###### -pru_logic_replicate

The program is to show simultaneous input and output. The program replicates the logical state of one pin onto the other pin. Both of the pins should be connected to the same pru-core. 

To change the input and output pin, open the ''deploy.sh'' script and edit related variables. By default, the program takes input from P8_45 and outputs to P8_46. So you can see following variables in ''deploy.sh''

    #If changing these variables, make sure the given pin can be muxed to the given pru.  
    OUT_HEADER=P8_
    OUT_PIN_NUMBER=46
    IN_HEADER=P8_
    IN_PIN_NUMBER=45
    #PRU_CORE should be either 0 or 1
    PRU_CORE=1



###### -PRU_inline_asm_blinky 
Blinky using in-line assembly code. The example is to show how we can use in-line assembly along with the C code to do time critical things.
The main_pru1.c source file has a 'start' function that is defined inside the 'pru1-asm-blinky.asm' assembly file. The definition of this function is linked at compile time.

There are following make targets:
1. To compile code and generate the output firmware file in PRU_inline_asm_blinky/gen/ :

        $ make 
2. To compile and install the firmware on pru-core 1 (note - you will still have to config the required pin as prout, demonstrated [here](https://zeekhuge.github.io/post/a_handfull_of_commands_and_scripts_to_get_started_with_beagleboneblack/#starters:01d25bfd2399ec47b9c04f156786eab8) ) : 

        $ make install

###### -pru1_to_pru0_to_arm
The example demonstrates use of interrupts for inter-PRU communication along with the use of RPMsg. In the example uses PRU-1 generates an interrupt in every one second. Whenever PRU-2 gets this interrupt it sends a string message - "Interrupted" to userspace. PRU-1 generates system event 18 mapped to channel 1 which is further mapped to HOST1. 

There are following make targets:
1. To compile code and generate the output firmware file in pru1_to_pru0_to_arm/gen/ :

        $ make 
2. To compile and install the firmware on pru:

        $ make install


### -kernel_examples
Contains examples that have a loadable kernel module, and related pru firmware to demonstrate some concepts.

###### -n-blinky
The example contains a kernel module module/rpmsg_pru_parallel_example.c and a firmware for pru-1 firmware/main_pru1.c. 

The module is to demonstrate how to use rpsmg APIs to communicate with the PRUs. The module when probed, creates a character device at /dev/rpmsg_pru_parallel_example. Writing a number (0-9) to this device will toggle the board pins 27, 28, 29, 39, 40, 41, 42, 43, 44, 45 and 46 on P8 header, with a delay of about 1 second.

The firmware probes the rpmsg_pru_parallel_example driver and then waits for messages from the driver.

To get the example working:

Compile the source (cd to n-blinky) :
        
        $ make
        $ cd module
        $ sudo modprobe virtio_rpmsg_bus
        $ sudo insmod rpmsg_pru_parallel_example.ko
        $ cd ../firmware
        $ ./deploy.sh
This will probe the module and the char device rpmsg_pru_parallel_example would appear in /dev/. To get the parallel blinky blink 3 (or any number) times :

        $ echo 3 > /dev/rpmsg_pru_parallel_example
