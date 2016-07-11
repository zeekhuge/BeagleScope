;*
;* Copyright (C) 2016 Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
;*
;* The code is a part of BeagleScope Project.
;*
;* This program is free software; you can redistribute it and/or
;* modify it under the terms of the GNU General Public License version
;* 2 as published by the Free Software Foundation.

	.cdecls "main_pru1.c"


;********************************************************************
; BLINK is just for debugging purpose while developing the code.
;

BLINK	.macro
	CLK_TOGGLE
	DELAY_IMMEDIATE_2n 100000000
	CLK_TOGGLE
	DELAY_IMMEDIATE_2n 100000000
	.endm

;********************************************************************
; INIT : To do basic initialization on startup
;
; The macro performs initialization steps whenever the PRU is
; started.
;
; Steps that it performs are:
; 1.)Clearing the R30 register which will be used for clock
; generation.
; 2.)Clearing status of INT_P0_to_P1 interrupt
;

INIT	.macro
	LDI32 R30, 0x00000000
	LDI32 R0, INT_P0_to_P1
	SBCO &R0, CONST_PRU_ICSS_INTC, SICR_offset, 4
	.endm

;********************************************************************
; NOP :	Null operation macro to produce a delay of one
;	cycle
;
; The macro essentially consumes one system cycle in subtracting 0
; from the content of R0 register and put the result back in R0
; register. This do not changes anything but just consumes up one
; single system clock cycle.
;

NOP	.macro
	SUB	R0, R0, 0
	.endm

;********************************************************************
; DELAY_IMMEDIATE_2n : The macro to cause a delay of 'cycles_2n'
; cycles.
;
; cycles_2n : The number of cycles to delay. This should be an
; immediate value and should always be a multiple of 2.
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

;********************************************************************
; DELAY_IMMEDIATE_3n : The macro to cause a delay of 'cycles_3n'
; cycles.
;
; cycles_3n	: The number of cycles to delay. This should be an
; immediate value and should always be a multiple of 3.
;
; NOTE:	The least delay value that can be given to the macro is 3
;

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
; DELAY_SAMPLE : To cause delay between samples. The macro takes
; CYCLE_BTWN_SAMPLE value, given by PRU0 and uses it to cause a delay
; between consecutive samples.
;
; Instruction: "SUB R0.wo, CYCLE_BTWN_SAMPLE, 1 + 2 + 2 + 2"
; 	The instruction takes value from CYCLE_BTWN_SAMPLE value from
; SAMPLIG.CONFIG data and subtracts 1+2+2+2 cycles.
;
; 1 cycle = used for the CHECK_INT macro
;
; 2 cycles = already being delayed between each sample in
; SAMPLE_CYCLE_n macro either by DELAY_CYCLES_2n  macro, or by other
; macros
;
; 2 cycles = being used by the SUB and QBEQ instructions in the
; DELAY_SAMPLE macro
;
; 2 cycles = being used by the 2 CLK_TOGGLE instructions in
; TAKE_SAMPLE_8 macro
;
; NOTE: 1.) It turns out that the least value of CYCLE_BTWN_SAMPLE 
; can be 7. And hence least delay between 2 consecutive  samples will
; be 7 cycles.
; 2.) The value given in the SAMPLING_CONFIG_CYCLE_BTWN_SAMPLE should
; always be odd, as subtracting 7 would then make it even and it will
; be in an infinite loop if the value is even.
;

DELAY_SAMPLE    .macro
                SUB     R0, SAMPLING_CONFIG_CYCLE_BTWN_SAMPLE, 1 + 2 + 2 + 2
                QBEQ    $ES?, R0, 0
$MS?:           SUB     R0, R0, 2
                QBNE    $MS?, R0, 0
$ES?:
                .endm

;********************************************************************
; CHECK_INT : A macro to do a quick check of the status of the
; INT_P0_to_P1 interrupt.
;
; If there in an interrupt to be serviced, the control JMPs to
; 'manage_interrupt' tag in main, that further services the interrupt.
;
; If no interrupt is detected, the JMP statement is bypassed and the
; program flow is continued.
;

CHECK_INT	.macro
		QBBC	$CI1?, R31, HOST_PRU0_TO_PRU1_CB
		JMP	manage_interrupt
$CI1?:
		.endm

;********************************************************************
; CHECK_INT_LOOP : To check and wait for the occurrence of
; INT_P0_to_P1 interrupt. The macro keeps polling the status of
; INT_P0_to_P1 interrupt and once the interrupt occurs, control moves
; to the next instruction. Therefore, control after CHECK_INT_LOOP
; must be moved to 'manage_interrupt' tag in main.
;
; This macro checks the status of HOST_PRU0_TO_PRU1_CB in R31
; register. This bit is hardwired to HOST_PRU0_TO_PRU1(Host0 or Host1)
; and is set whenever INT_P0_to_P1 occurs.
;

CHECK_INT_LOOP	.macro
$A?:		QBBC	$A?, R31, HOST_PRU0_TO_PRU1_CB
		.endm

;********************************************************************
; MANAGE_INTERRUPT : The macro that will be invoked on occurrence of
; interrupt.
;
; The first 2 instructions of the macro clears the status of
; INT_P0_to_P1 interrupt, due to which this macro got invoked.
;
; Next 2 instructions loads the data from shared RAM into
; SAMPLING_CONFIG_0 and SAMPLING_CONFIG_1 registers. The sampling
; config data in the shared RAM was placed by PRU0.
;

MANAGE_INTERRUPT	.macro
			LDI32	R0, INT_P0_to_P1
			SBCO	&R0, CONST_PRU_ICSS_INTC, SICR_offset, 4
			LDI32	R1, SHARED_MEM_ADDR
			LBBO	&SAMPLING_CONFIG_START, R1, 0, SAMPLING_CONFIG_LENGTH
			.endm

;********************************************************************
; TRANSFER_AND_TELL : The macro transfers the sampled data that has
; been acquired using SAMPLE_CYCLE_8 macro, to the bank and then
; interrupts PRU0.
;
; The macro takes one argument 'bank'.It is the bank id number where
; the sampled data needs to be transfered.
;

TRANSFER_AND_TELL	.macro bank
			XOUT	bank, &DATA_START_REGISTER, DATA_SIZE
			LDI	R31.w0, R31_P1_to_P0
			.endm


;********************************************************************
; CLK_TOGGLE : This macro toggles the pin connected to the CLK_PIN
; bit of the R30 register. Each toggle takes 1 cycle.
;

CLK_TOGGLE	.macro
		XOR R30, R30, CLK_PIN
		.endm

;********************************************************************
; TAKE_SAMPLE_8 : The macro takes data input from the R31 register,
; ie the GPI pins and stores the data into RX register.
; The macro also toggles the clock, required to sample the data,
; provided the clock pin is at 0 before the SAMPLE_CYCLE_8 starts.
;
; The Rx register is where the sampled data is to be kept.
;
; For testing purposes one can comment out the 'MOV RX,R31' and one
; of the 'CLK_TOGGLE' instructions and remove ';' from 
; "LDI Rx, FAKE_DATA" instruction.
; Further connect an LED or Oscilloscope to the CLK_PIN. This will
; result in sending FAKE_DATA to the PRU0 which will be further sent
; to the kernel. Also, the  CLK_PIN will be toggled each time a sample
; is taken, and hence the sampling frequency can be tested.
;

TAKE_SAMPLE_8	.macro RX
		CLK_TOGGLE
		MOV	RX, R31
		;LDI	RX, FAKE_DATA
		CLK_TOGGLE
		.endm

;********************************************************************
; SAMPLE_CYCLE_8 : The macro is a complete sampling cycle that takes
; one sample using TAKE_SAMPLE_8 macro, delays for < delay by
; CHECK_INT macro + 2 + delay caused by DELAY_SAMPLE > cycles and
; then takes another sample.
;
; The delay after the last sample of the cycle is
; < delay by DELAY_SAMPLE macro >. This is because in main,
; SAMPLE_CYCLE_8 is followed by TRANSFER_AND_TELL and a CHECK_INT
; macro or JMP instruction, both of which take one cycle. Thus
; maintaining a delay of < 3 + delay by DELAY_SAMPLE macro > cycles
; between each sample.
;
; In all, this macro takes 44 samples of 1byte each.
;

SAMPLE_CYCLE_8	.macro
		TAKE_SAMPLE_8           BYTE_1

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_2

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_3

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_4

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_5

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_6

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_7

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_8

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_9

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_10

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_11

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_12

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_13

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_14

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_15

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_16

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_17

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_18

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_19

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_20

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_21

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_22

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_23

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_24

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_25

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_26

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_27

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_28

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_29

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_30

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_31

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_32

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_33

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_34

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_35

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_36

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_37

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_38

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_39

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_40

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_41

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_42

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_43

		CHECK_INT
		DELAY_IMMEDIATE_2n      2
		DELAY_SAMPLE

		TAKE_SAMPLE_8           BYTE_44

		DELAY_SAMPLE

		.endm

;*******************************************************
;*******************************************************

	.global main
main:
	INIT

int_loop:
	CHECK_INT_LOOP

manage_interrupt:
	MANAGE_INTERRUPT
	QBBC	int_loop, SAMPLING_CONFIG_1, SAMPLING_CONFIG_START_BIT

sample_1:
	SAMPLE_CYCLE_8
	CHECK_INT
	TRANSFER_AND_TELL SP_BANK_0

sample_2:
	SAMPLE_CYCLE_8
	CHECK_INT
	TRANSFER_AND_TELL SP_BANK_1

sample_3:
	SAMPLE_CYCLE_8
	TRANSFER_AND_TELL SP_BANK_2
	JMP sample_1

	HALT
