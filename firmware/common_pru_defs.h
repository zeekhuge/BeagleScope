
/*
 * Copyright (C) 2016 Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
 *
 * The code is developed as a part of BeagleScope project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _COMMON_PRU_DEFS_
#define _COMMON_PRU_DEFS_

/*
 * Interrupts ( system events ) that will be used to interrupt the
 * two PRUs.
 * INT_P1_to_P0 : interrupt from PRU1 to PRU0
 * INT_P0_to_P1 : interrupt from PRU0 to PRU1
 */
#define INT_P1_to_P0 18
#define INT_P0_to_P1 19

/* Interrupt mappings
 * CHNL_PRU0_TO_PRU1 : The INTC channel to which the INT_P0_to_P1 interrupt
 * 		is be mapped.
 * HOST_PRU0_TO_PRU1 : The Host to which CHNL_PRU0_TO_PRU1 channel is
 * 		mapped. PRU1 will be checking status of this host to check
 * 		the occurence of INT_P0_to_P1 interrupt. It is recommended
 *		to map channel N to Host N.
 */
#define CHNL_PRU0_TO_PRU1	1
#define HOST_PRU0_TO_PRU1	1

/*
 * HOST0 and HOST1 check bits
 * HOST_PRU0_TO_PRU1_CB : The check bit to check HOST_PRU0_TO_PRU1 interrupt
 *		Since the value of HOST_PRU0_TO_PRU1 is 1, HOST_PRU0_TO_PRU1_CB
 *		has the value 31
 */
#define HOST_PRU0_TO_PRU1_CB	31

/* Address of the external peripherals
 * SHARED_MEM_ADDR : Absolute local address of the 12 KB shared RAM that will be
 * 		used to communicate sampling configuration data between the two
 * 		PRUs
 */
#define SHARED_MEM_ADDR 0x00010000

#endif /* _COMMON_PRU_DEFS_ */
