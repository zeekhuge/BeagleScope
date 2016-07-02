/*
 * Copyright (C) 2016 Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
 *
 * The code is developed as a part of BeagleScope project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdint.h>
#include <pru_cfg.h>
#include <pru_intc.h>
#include "resource_table_pru0.h"
#include "common_pru_defs.h"

volatile register uint32_t __R30;
volatile register uint32_t __R31;

void main(void)
{
	/* pointer to starting of 12KB shared RAM */
	int32_t *ptr_to_shared_mem;
	ptr_to_shared_mem = (int32_t *) SHARED_MEM_ADDR;

	/* Writing to EISR regoster to enable interrupt */
	CT_INTC.EISR_bit.EN_SET_IDX = INT_P0_to_P1;

	/* Writing data that is to be shared to PRU1, in shared
	   RAM */
	*(ptr_to_shared_mem) = 10000001;
	*(ptr_to_shared_mem + 1 ) = 1<<31 ;

	/* Generating system event INT_P0_to_P1 */
	__R31 = ( (1 << 5) | (INT_P0_to_P1 - 16));

	__halt();
}

