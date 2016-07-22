# firmware
PRU firmware source code for BeagleScope project.
The firmware is used to perform high speed input data sampling. 

### Features -
- RAW_READ mode - To read a single sample.
- BLOCK_READ mode - To sample continuously and send data to the kernel in block of 44 bytes
- Configurable duty cycle
- Configurable sampling frequency
- P8_46 used as the clock pin

### Compiling the source
To compile :

    $ make
To compile and install the firmware on the PRUs :

    $ make install
To configure P8_46 as output and mux it to PRU

    $ config-pin P8_46 pruout
    
### Working

The firmware takes 1 x 12 byte long message from the userspace through '/dev/rpmsg_pru30' char file. This message is actually the configuration data. The message is composed of :
* CYCLE_BTWN_SAMPLE    -  4 byte long
* CYCLE_BEFORE_SAMPLE  - 2 byte long
* CYCLE_AFTER_SAMPLE   - 2 byte long
* MISC_CONFIG_DATA - 4 byte long

##### The sampling process goes as ( assume each point to consume only one cycle ):
* Clock pin is pulled high
* Wait for CYCLE_BEFORE_SAMPLE cycles
* Input data is sampled through the GPIs
* Wait for CYCL_AFTER_SAMPLE cycles
* Clock pin is pulled low
* House keeping instruction 1
* House keeping instruction 2
* Interrupt check
* Wait for CYCLE_BTWN_SAMPLE cycles

The instructions above are executed inside a loop.

Thus as the above demo of sampling process suggest, the total time period is = CYCLE_BEFORE_SAMPLE + CYCLE_AFTER_SAMPLE + CYCLE_BTWN_SAMPLE + 5

##### Important points
* The least value of CYCLE_BEFORE_SAMPLE, CYCLE_AFTER_SAMPLE and CYCLE_BTWN_SAMPLE can be 1
* The value of CYCLE_BEFORE_SAMPLE, CYCLE_AFTER_SAMPLE and CYCLE_BTWN_SAMPLE should always be an odd number.

### Testing the firmware
To test the firmware, configuration data needs to be send to the PRUs from kernel space using the character file '/dev/rpmsg_pru30'.

This can be a bit difficult/confusing because of the way raw data is send thorough character device files.

* Assume :
    * CYCLE_BTWN_SAMPLE = 0x00989681 = 00989681 
    * CYCLE_BEFORE_SAMPLE = 0xabcd = abcd
    * CYCLE_AFTER_SAMPLE = 0x0103 = 0103

* Then swap their bytes
    * CYCLE_BTWN_SAMPLE = 81969800 
    * CYCLE_AFTER_SAMPLE = cdab
    * CYCLE_BEFORE_SAMPLE = 0301

* Format suitable to be used with echo
    * CYCLE_BTWN_SAMPLE = \x81\x96\x98\x00 
    * CYCLE_AFTER_SAMPLE = \xcd\xab
    * CYCLE_BEFORE_SAMPLE = \x03\x01

* write them to the char device file using the sequence

        CYCLE_BTWN_SAMPLE CYCLE_AFTER_SAMPLE CYCLE_BEFORE_SAMPLE  

    ie, for this example it will be :
    
        \x81\x96\x98\x00\ xcd\xab\ x03\x01
        
    appending the constant MISC_CONFIG_DATA the message to be send becomes (notice the spaces have been removed) :

        \x81\x96\x98\x00\xcd\xab\x03\x01\x01\x00\x00\x80
        
    To send it execute following command:
    
        echo -e '\x81\x96\x98\x00\xcd\xab\x03\x01\x01\x00\x00\x80' > /dev/rpmsg_pru30 && xxd -c 4 /dev/rpmsg_pru30
