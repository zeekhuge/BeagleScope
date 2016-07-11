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
 * Register Usage
 * The PRU comprises of 32 registers that can be used for various
 * purposes. This section of the file describes and declares usage of
 * the register.
 *
 * USED_IN_CODE_N : These registers are used inside the code to help
 * perform various operations, and should not be used for a specific
 * purpose.
 *
 * UNUSED_FOR_NOW_N : The registers are not being used for any
 * purpose and are free to be used for other operations/tasks.
 *
 * BYTE_N : Registers reserved to store sampled data
 *
 * SAMPLING_CONFIG_N : Register used to save sampling configuration
 * data
 *
 * CONTROL_REGISTER_N : These register are not meant for normal use.
 * They are control register and have very specific usage.
 */
#define USED_IN_CODE_0		R0 // Register R0 and R1 are being
#define USED_IN_CODE_1		R1 // used in the code
#define UNUSED_FOR_NOW_1	R2
#define UNUSED_FOR_NOW_2	R3
#define UNUSED_FOR_NOW_3	R4
#define UNUSED_FOR_NOW_4	R5
#define BYTE_1			R6.b0
#define BYTE_2			R6.b1
#define BYTE_3			R6.b2
#define BYTE_4			R6.b3
#define BYTE_5			R7.b0
#define BYTE_6			R7.b1
#define BYTE_7			R7.b2
#define BYTE_8			R7.b3
#define BYTE_9			R8.b0
#define BYTE_10			R8.b1
#define BYTE_11			R8.b2
#define BYTE_12			R8.b3
#define BYTE_13			R9.b0
#define BYTE_14			R9.b1
#define BYTE_15			R9.b2
#define BYTE_16			R9.b3
#define BYTE_17			R10.b0
#define BYTE_18			R10.b1
#define BYTE_19			R10.b2
#define BYTE_20			R10.b3
#define BYTE_21			R11.b0
#define BYTE_22			R11.b1
#define BYTE_23			R11.b2
#define BYTE_24			R11.b3
#define BYTE_25			R12.b0
#define BYTE_26			R12.b1
#define BYTE_27			R12.b2
#define BYTE_28			R12.b3
#define BYTE_29			R13.b0
#define BYTE_30			R13.b1
#define BYTE_31			R13.b2
#define BYTE_32			R13.b3
#define BYTE_33			R14.b0
#define BYTE_34			R14.b1
#define BYTE_35			R14.b2
#define BYTE_36			R14.b3
#define BYTE_37			R15.b0
#define BYTE_38			R15.b1
#define BYTE_39			R15.b2
#define BYTE_40			R15.b3
#define BYTE_41			R16.b0
#define BYTE_42			R16.b1
#define BYTE_43			R16.b2
#define BYTE_44			R16.b3
#define BYTE_45			R17.b0
#define BYTE_46			R17.b1
#define BYTE_47			R17.b2
#define BYTE_48			R17.b3
#define BYTE_49			R18.b0
#define BYTE_50			R18.b1
#define BYTE_51			R18.b2
#define BYTE_52			R18.b3
#define BYTE_53			R19.b0
#define BYTE_54			R19.b1
#define BYTE_55			R19.b2
#define BYTE_56			R19.b3
#define BYTE_57			R20.b0
#define BYTE_58			R20.b1
#define BYTE_59			R20.b2
#define BYTE_60			R20.b3
#define BYTE_61			R21.b0
#define BYTE_62			R21.b1
#define BYTE_63			R21.b2
#define BYTE_64			R21.b3
#define BYTE_65			R22.b0
#define BYTE_66			R22.b1
#define BYTE_67			R22.b2
#define BYTE_68			R22.b3
#define BYTE_69			R23.b0
#define BYTE_70			R23.b1
#define BYTE_71			R23.b2
#define BYTE_72			R23.b3
#define BYTE_73			R24.b0
#define BYTE_74			R24.b1
#define BYTE_75			R24.b2
#define BYTE_76			R24.b3
#define BYTE_77			R25.b0
#define BYTE_78			R25.b1
#define BYTE_79			R25.b2
#define BYTE_80			R25.b3
#define BYTE_81			R26.b0
#define BYTE_82			R26.b1
#define BYTE_83			R26.b2
#define BYTE_84			R26.b3
#define SAMPLING_CONFIG_0	R27
#define SAMPLING_CONFIG_1	R28
#define SAMPLING_CONFIG_2	R29
#define CONTROL_REGISTER_0	R30
#define CONTROL_REGISTER_1	R31

/*
 * SAMPLING_CONFIG_START: the register that will be the
 * starting point of the config data. Register SAMPLING_CONFIG_0
 * and SAMPLING_CONFIG_1 are used to save the sampling data, with
 * SAMPLING_CONFIG_0 being the lower register, and hence the value
 * of SAMPLING_CONFIG_START.
 *
 * SAMPLING_CONFIG_LENGTH: Number of bytes of the sampling
 * data.
 */
#define SAMPLING_CONFIG_START		SAMPLING_CONFIG_0
#define SAMPLING_CONFIG_LENGTH		3 * 4

/*
 * CYCLE_BTWN_SAMPLE and SAMPLING_WIDTH
 * The two definitions to be used with
 * SAMPLING_CONFIG to get the config info
 * To get
 *	Delay cycles between consecutive
 *	sample = CYCLE_BTWN_SAMPLE
 *
 *	Delay cycles after the CLK_PIN is HIGH and
 *	before the sample is taken = CYCLE_BEFORE_SAMPLE
 *
 *	Delay cycles after the sample is taken and before
 *	the CLK_PIN is LOW = CYCLE_AFTER_SAMPLE
 *
 *	The regiter that contains some miscellaneous
 *	sampling configuration data = MISC_CONFIG_DATA
 *
 *	Width of the sampling data =
 *		SAMPLING_WIDTH
 *
 *	Start or Stop sampling instruction =
 *		SAMPLING_START_BIT is the bit
 *		number in SAMPLING_CONFIG_1 register
 *		and instructs to start sampling if is
 *		set. Otherwise sampling should not be
 *		done
 */
#define CYCLE_BTWN_SAMPLE	SAMPLING_CONFIG_0
#define CYCLE_BEFORE_SAMPLE	SAMPLING_CONFIG_1.w0
#define CYCLE_AFTER_SAMPLE	SAMPLING_CONFIG_1.w2
#define MISC_CONFIG_DATA	SAMPLING_CONFIG_1
#define SAMPLING_WIDTH		MISC_CONFIG_DATA.b2
#define SAMPLING_START_BIT	31

register uint32_t __R30;
volatile register uint32_t __R31;

extern void main(void);
