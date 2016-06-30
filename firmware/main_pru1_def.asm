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
;*******************************************************

DELAY_CYCLES	.macro cycles
		LDI32	R0, cycles - 1 
		QBEQ	$E?, R0, 0
$M?:		SUB	R0, R0, 1
		QBNE	$M?, R0, 0
$E?:	
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
