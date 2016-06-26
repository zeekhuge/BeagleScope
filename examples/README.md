    
## **Examples** 
The examples are based on the pru-software-support-package provided by TI. These examples were tested on 4.4.11-ti-r29 kernel version and are based on older version of RPMsg kernel module, that used mailboxes. The newer RPMsg, that uses ARM INTC in place of mailboxes, is available in kernel version 4.4.12-ti-r31 and later.

### -firmware_examples
Contains examples that are only related to the firmware code for both or one of the PRUs.
###### -pru_blinky
Its the most basic program. This program directory contains ''deploy.sh'' script that compiles the code, copies the firmware to /lib/firmware/am335x-pru1(0)-fw, and reboots pru1(0)-core. To get blinky on a particular pin:
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

        make 
2. To compile and install the firmware on pru-core 1 (note - you will still have to config the required pin as prout, demonstrated [here](https://zeekhuge.github.io/post/a_handfull_of_commands_and_scripts_to_get_started_with_beagleboneblack/#starters:01d25bfd2399ec47b9c04f156786eab8) ) : 

        make install


### -kernel_examples
Contains examples that have a loadable kernel module, and related pru firmware to demonstrate some concepts.
