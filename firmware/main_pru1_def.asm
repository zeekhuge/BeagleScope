;*
;* Copyright (C) 2016 Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
;*
;* This file is as an example to show how to develope
;* and compile inline assembly code for PRUs
;*
;* This program is free software; you can redistribute it and/or modify
;* it under the terms of the GNU General Public License version 2 as
;* published by the Free Software Foundation.

	.cdecls "main_pru1.c"


;*******************************************************
; BLINK is just for debugging purpose while developing
; the code
;

BLINK	.macro
	CLK_TOGGLE
	DELAY_IMMEDIATE_2n 100000000
	CLK_TOGGLE
	DELAY_IMMEDIATE_2n 100000000
	.endm

;*******************************************************
; INIT : To do basic initialization on startup
;
; The macro performs initialization steps whenever the
; PRU is started.
;
; Steps that it performs are:
; 1.)Clearing the R30 register which will be used for
; clock generation.
; 2.)Clearing status of INT_P0_to_P1 interrup
;

INIT	.macro
	LDI32 R30, 0x00000000
	LDI32 R0, INT_P0_to_P1
	SBCO &R0, CONST_PRU_ICSS_INTC, SICR_offset, 4
	.endm

;*******************************************************
; NOP :	Null operation macro to produce a delay of one
;	cycle
;
; The macro essentially consumes one system cycle in
; subtracting 0 from the content of R0 register and put
; the result back in R0 register. This do not changes
; anything but just consumes up one single system clock
; cycle

NOP	.macro
	SUB	R0, R0, 0
	.endm

;*******************************************************
; DELAY_IMMEDIATE_2n:	The macro to cause a delay of
;			'cycles_2n' cycles.
;
; cycles_2n	: The number of cycles to delay. This
;		should be an immediate value and should
;		always be a multiple of 2.
;
; NOTE:	The least delay value that can be give to this
;	macro is 2 cycles

DELAY_IMMEDIATE_2n	.macro cycles_2n
			LDI32	R0, cycles_2n - 2
			QBEQ	$E2?, R0, 0
$M2?:			SUB	R0, R0, 2
			QBNE	$M2?, R0, 0
$E2?:
			.endm

;*****************************************************
; DELAY_IMMEDIATE_3n:	The macro to cause a delay of
;			'cycles_3n' cycles.
;
; cycles_3n	: The number of cycles to delay.
;		This should be an immediate value and
;		should always be a multiple of 3.
;
; NOTE:	The least delay value that can be given to the
; 	macro is 3

DELAY_IMMEDIATE_3n      .macro cycles_3n
                        LDI32   R0, cycles_3n - 3
                        NOP
                        QBEQ    $E3?, R0, 0
$M3?:                   SUB     R0, R0, 3
                        NOP
                        QBNE    $M3?, R0, 0
$E3?:
                        .endm

;********************************************************************
; DELAY_SAMPLE : To cause delay between samples
; The macro takes CYCLE_BTWN_SAMPLE value, given by PRU0
; and uses it to cause a delay between consecutive samples.
;
; Instruction: "SUB R0.wo, CYCLE_BTWN_SAMPLE, 3 + 2 + 2"
; The instrcution takes value from CYCLE_BTWN_SAMPLE value
; from SAMPLIG.CONFIG data and subtracts 3+2+2 cycles.
; 3 cycles      = already beign delayed between each sample
;               in SAMPLE_CYCLE_n macro either by DELAY_CYCLES_3n
;               macro, or by other macros
; 2 cycles      = being used by the SUB and QBEQ instructions
;               in the DELAY_SAMPLE macro
; 2 cycles      = being used by the 2 CLK_TOGGLE instructions
;               in TAKE_SAMPLE_8 macro
;
; NOTE: 1.) It turns out that the least value of CYCLE_BTWN_SAMPLE
;       can be 7. And hence least delay between 2 consecutive
;       samples will be 7 cycles.
;	2.) The value given in the SAMPLING_CONFIG_CYCLE_BTWN_SAMPLE
;	should always be odd, as subtracting 7 would then make it even
;	and it will be in an infinite loop if the value is even.
;

DELAY_SAMPLE    .macro
                SUB     R0, SAMPLING_CONFIG_CYCLE_BTWN_SAMPLE, 3 + 2 + 2
                QBEQ    $ES?, R0, 0
$MS?:           SUB     R0, R0, 2
                QBNE    $MS?, R0, 0
$ES?:
                .endm

;********************************************************************
; CHECK_INT_LOOP : To check and wait for the occurence of
; INT_P0_to_P1 interrupt. The macro keeps polling the status of
; INT_P0_to_P1 interrupt and ones the interrupt occurs, it invokes the
; MANAGE_INTERRUPT macro.
;
; The first 2 instructions LDI32, LBBO copies the data from SECR0
; register present in PRU ICSS INTC into R0 register.
; The 3rd instruction, QBBC checks the status of INT_P0_to_P1 bit in it.
;

CHECK_INT_LOOP	.macro
$A?:		LDI32	R1,SECR0
		LBBO	&R0, R1, 0, 4
		QBBC	$A?, R0, INT_P0_to_P1
		MANAGE_INTERRUPT
		.endm

;********************************************************************
; MANAGE_INTERRUPT : The macro that will be invoked on occurence of
; interrupt. The macro is incomplete as of now and just clears the
; status of INT_P0_to_P1. It then invokes BLINK macro.
;

MANAGE_INTERRUPT	.macro
			LDI32	R0, INT_P0_to_P1
			SBCO	&R0, CONST_PRU_ICSS_INTC, SICR_offset, 4
			BLINK
			.endm

;*******************************************************
;*******************************************************

CLK_TOGGLE	.macro
		XOR R30, R30, CLK_PIN
		.endm



;********************************************************
;*******************************************************

TAKE_SAMPLE_8	.macro RX
		CLK_TOGGLE
		MOV	RX, R31
		;CLK_TOGGLE
		.endm

;*********************************************************
;*********************************************************

SAMPLE_CYCLE_8	.macro

		TAKE_SAMPLE_8		BYTE_1

		DELAY_IMMEDIATE_3n 	3
		DELAY_SAMPLE

		TAKE_SAMPLE_8		BYTE_2

		DELAY_IMMEDIATE_3n 	3
		DELAY_SAMPLE

		TAKE_SAMPLE_8		BYTE_3

		DELAY_IMMEDIATE_3n	3
		DELAY_SAMPLE

		TAKE_SAMPLE_8		BYTE_4

		DELAY_IMMEDIATE_3n	3
		DELAY_SAMPLE

		TAKE_SAMPLE_8   	BYTE_5

		DELAY_IMMEDIATE_3n	3
                DELAY_SAMPLE

                TAKE_SAMPLE_8   	BYTE_6

		DELAY_IMMEDIATE_3n	3
                DELAY_SAMPLE

                TAKE_SAMPLE_8   	BYTE_7

		DELAY_IMMEDIATE_3n	3
                DELAY_SAMPLE

                TAKE_SAMPLE_8   	BYTE_8

		DELAY_IMMEDIATE_3n	3
                DELAY_SAMPLE

		TAKE_SAMPLE_8   	BYTE_9

		DELAY_IMMEDIATE_3n	3
            	DELAY_SAMPLE

		.endm

;*******************************************************
;*******************************************************

	.global main
main:
	INIT
again:
	CHECK_INT_LOOP
	JMP again
	HALT
