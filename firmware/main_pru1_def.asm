;*
;* Copyright (C) 2016 Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
;*
;* This file is as an example to show how to develope
;* and compile inline assembly code for PRUs
;*
;* This program is free software; you can redistribute it and/or modify
;* it under the terms of the GNU General Public License version 2 as
;* published by the Free Software Foundation.

	.asg 19, INT_P0_TO_P1
	.asg 18, INT_P1_TO_P0
	
	.cdecls "main_pru1.c"

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
                        LDI32   R0, cycles - 3
                        NOP
                        QBEQ    $E3?, R0, 0
$M3?:                   SUB     R0, R0, 3
                        NOP
                        QBNE    $M3?, R0, 0
$E3?:
                        .endm

;*****************************************************
;*****************************************************

DELAY_SAMPLE	.macro time
 
		.endm
;******************************************************
;******************************************************

INIT	.macro
	LDI32 R30, 0x00000000
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

SAMPLE_CYCLE_8	.macro bit
		TAKE_SAMPLE_8	BYTE_1 
		DELAY_SAMPLE 	CYCLE_BTWN_SAMPLE
		TAKE_SAMPLE_8	BYTE_2
		DELAY_SAMPLE	CYCLE_BTWN_SAMPLE
		TAKE_SAMPLE_8	BYTE_3
		DELAY_SAMPLE	CYCLE_BTWN_SAMPLE
		TAKE_SAMPLE_8	BYTE_4
		DELAY_SAMPLE	CYCLE_BTWN_SAMPLE
		TAKE_SAMPLE_8   BYTE_5
                DELAY_SAMPLE    CYCLE_BTWN_SAMPLE
                TAKE_SAMPLE_8   BYTE_6
                DELAY_SAMPLE    CYCLE_BTWN_SAMPLE
                TAKE_SAMPLE_8   BYTE_7
                DELAY_SAMPLE    CYCLE_BTWN_SAMPLE
                TAKE_SAMPLE_8   BYTE_8
                DELAY_SAMPLE    CYCLE_BTWN_SAMPLE
		TAKE_SAMPLE_8   BYTE_9
                DELAY_SAMPLE    CYCLE_BTWN_SAMPLE
                TAKE_SAMPLE_8   BYTE_10
                DELAY_SAMPLE    CYCLE_BTWN_SAMPLE
                TAKE_SAMPLE_8   BYTE_11
                DELAY_SAMPLE    CYCLE_BTWN_SAMPLE
                TAKE_SAMPLE_8   BYTE_12
                DELAY_SAMPLE    CYCLE_BTWN_SAMPLE - 1 
		.endm

;*******************************************************
;*******************************************************

MANAGE_INTERRUPT	.macro
			LDI32	SICR,			INT_P0_TO_P1
			LBCO	SAMPLING_CONFIG,	SHARED_MSG,0,4	
			.endm	

	.global main
main:
	LDI32	SAMPLING_CONFIG,	100000
	SAMPLE_CYCLE_8
	HALT
