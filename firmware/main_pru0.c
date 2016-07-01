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
#include "resource_table_pru1.h"
#include "common_pru_defs.h"

volatile register uint32_t __R30;
volatile register uint32_t __R31;

void main(void)
{

	while (1) {

		__R31 = ( (1 << 5) | (INT_P0_to_P1));
		__delay_cycles(200000000);
	}
}

