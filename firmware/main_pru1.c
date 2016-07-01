/*
 * Source Modified by Zubeen Tolani < ZeekHuge - zeekhuge@gmail.com >
 * Based on the examples distributed by TI
 *
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *	* Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 *
 *	* Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the
 *	  distribution.
 *
 *	* Neither the name of Texas Instruments Incorporated nor the names of
 *	  its contributors may be used to endorse or promote products derived
 *	  from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include "resource_table_pru1.h"
#include "common_pru_defs.h"

/*
 * Interrupts ( system events ) that will be used to interrupt the
 * two PRUs.
 * INT_P1_to_P0 : interrupt from PRU1 to PRU0
 * INT_P0_to_P1 : interrupt from PRU0 to PRU1
 */
#define INT_P1_to_P0 18
#define INT_P0_to_P1 19

/*
 * PRU ICSS INTC registers
 * CONST_PRU_ICSS_INTC	: The entry from the constant table that points
 *			to starting of PRU_ICSS INTC
 * PRU_ICSS_INTC	: The absolute local address of the PRU ICSS INTC
 * SICR_offset		: The offset of SICR register with resepect to the
 *			PRU ICSS INTC
 * SECR0		: Absolute local address of SECR0 register present
 *			in PRU ICSS INTC
 */
#define CONST_PRU_ICSS_INTC	C0
#define PRU_ICSS_INTC		0x00020000
#define SICR_offset		0x24
#define SECR0			PRU_ICSS_INTC + 0x280

/*
 * Clock Pin
 * The bit of the R30 register where Clock pin of the external device
 * is connected
 */
#define CLK_PIN 1<<1

/*
 * BYTE_N : Data byte RX.bm for X from 2 to 29 and
 * m from 0 to 1
 */
#define BYTE_1	R2.b0
#define BYTE_2  R2.b1
#define BYTE_3  R2.b2
#define BYTE_4  R2.b3
#define BYTE_5  R3.b0
#define BYTE_6  R3.b1
#define BYTE_7  R3.b2
#define BYTE_8  R3.b3
#define BYTE_9  R4.b0
#define BYTE_10 R4.b1
#define BYTE_11 R4.b2
#define BYTE_12 R4.b3
#define BYTE_13 R5.b0
#define BYTE_14 R5.b1
#define BYTE_15 R5.b2
#define BYTE_16 R5.b3
#define BYTE_17 R6.b0
#define BYTE_18 R6.b1
#define BYTE_19 R6.b2
#define BYTE_20 R6.b3
#define BYTE_21 R7.b0
#define BYTE_22 R7.b1
#define BYTE_23 R7.b2
#define BYTE_24 R7.b3
#define BYTE_25 R8.b0
#define BYTE_26 R8.b1
#define BYTE_27 R8.b2
#define BYTE_28 R8.b3
#define BYTE_29 R9.b0
#define BYTE_30 R9.b1
#define BYTE_31 R9.b2
#define BYTE_32 R9.b3
#define BYTE_33 R10.b0
#define BYTE_34 R10.b1
#define BYTE_35 R10.b2
#define BYTE_36 R10.b3
#define BYTE_37 R11.b0
#define BYTE_38 R11.b1
#define BYTE_39 R11.b2
#define BYTE_40 R11.b3
#define BYTE_41 R12.b0
#define BYTE_42 R12.b1
#define BYTE_43 R12.b2
#define BYTE_44 R12.b3
#define BYTE_45 R13.b0
#define BYTE_46 R13.b1
#define BYTE_47 R13.b2
#define BYTE_48 R13.b3
#define BYTE_49 R14.b0
#define BYTE_50 R14.b1
#define BYTE_51 R14.b2
#define BYTE_52 R14.b3
#define BYTE_53 R15.b0
#define BYTE_54 R15.b1
#define BYTE_55 R15.b2
#define BYTE_56 R15.b3
#define BYTE_57 R16.b0
#define BYTE_58 R16.b1
#define BYTE_59 R16.b2
#define BYTE_60 R16.b3
#define BYTE_61 R17.b0
#define BYTE_62 R17.b1
#define BYTE_63 R17.b2
#define BYTE_64 R17.b3
#define BYTE_65 R18.b0
#define BYTE_66 R18.b1
#define BYTE_67 R18.b2
#define BYTE_68 R18.b3
#define BYTE_69 R19.b0
#define BYTE_70 R19.b1
#define BYTE_71 R19.b2
#define BYTE_72 R19.b3
#define BYTE_73 R20.b0
#define BYTE_74 R20.b1
#define BYTE_75 R20.b2
#define BYTE_76 R20.b3
#define BYTE_77 R21.b0
#define BYTE_78 R21.b1
#define BYTE_79 R21.b2
#define BYTE_80 R21.b3
#define BYTE_81 R22.b0
#define BYTE_82 R22.b1
#define BYTE_83 R22.b2
#define BYTE_84 R22.b3
#define BYTE_85 R23.b0
#define BYTE_86 R23.b1
#define BYTE_87 R23.b2
#define BYTE_88 R23.b3
#define BYTE_89 R24.b0
#define BYTE_90 R24.b1
#define BYTE_91 R24.b2
#define BYTE_92 R24.b3
#define BYTE_93 R25.b0
#define BYTE_94 R25.b1
#define BYTE_95 R25.b2
#define BYTE_96 R25.b3
#define BYTE_97 R26.b0
#define BYTE_98 R26.b1
#define BYTE_99 R26.b2
#define BYTE_100 R26.b3
#define BYTE_101 R27.b0
#define BYTE_102 R27.b1
#define BYTE_103 R27.b2
#define BYTE_104 R27.b3
#define BYTE_105 R28.b0
#define BYTE_106 R28.b1
#define BYTE_107 R28.b2
#define BYTE_108 R28.b3

/*
 * SAMPLING_CONFIG_0: The first register that is used
 * to store the config data. This will also be the
 * starting point of the config data and hence the value
 * of SAMPLING_CONFIG_START.
 *
 * SAMPLING_CONFIG_1: Second register that is used to
 * store the config data
 *
 * SAMPLING_CONFIG_START: the register that will be the
 * starting point of the config data
 *
 * SAMPLING_CONFIG_LENGTH: Number of bytes of the sampling
 * data.
 */

#define SAMPLING_CONFIG_0		R28
#define SAMPLING_CONFIG_1		R29

#define SAMPLING_CONFIG_START		SAMPLING_CONFIG_0
#define SAMPLING_CONFIG_LENGTH		8

/*
 * CYCLE_BTWN_SAMPLE and SAMPLING_WIDTH
 * The two definitions to be used with
 * SAMPLING_CONFIG to get the config info
 * To get
 *	Delay cycles between consecutive
 *	sample = SAMPLING_CONFIG_CYCLE_BTWN_SAMPLE
 *
 *	Width of the sampling data =
 *		SAMPLING_CONFIG_SAMPLING_WIDTH
 */
#define SAMPLING_CONFIG_CYCLE_BTWN_SAMPLE	SAMPLING_CONFIG_0
#define SAMPLING_CONFIG_SAMPLING_WIDTH		SAMPLING_CONFIG_1.b2


register uint32_t __R30;
volatile register uint32_t __R31;

extern void main(void);
