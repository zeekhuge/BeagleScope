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
	LDI32 TEMP_VARIABLE_0, INT_P0_to_P1
	SBCO &TEMP_VARIABLE_0, CONST_PRU_ICSS_INTC, SICR_offset, 4
	.endm


;********************************************************************
; SINGLE_STEP_MODE : To change from continuous to single_step_mode
;
; The macro does the required configuration to change the mode of PRU
; from continuous mode to single_step mode.
;

SINGLE_STEP_MODE	.macro
			LDI32	TEMP_VARIABLE_0, PRUSS_PRU_CTRL_START
			LBBO	&R2, TEMP_VARIABLE_0, CONTROL_REG, 4
			OR	R2, R2.b1, 1
			SBBO	&R2, TEMP_VARIABLE_0, CONTROL_REG, 4
			.endm

;********************************************************************
; NOP :	Null operation macro to produce a delay of one
;	cycle
;
; The macro essentially consumes one system cycle in subtracting 0
; from the content of TEMP_VARIABLE_0 register and put the result back in TEMP_VARIABLE_0
; register. This do not changes anything but just consumes up one
; single system clock cycle.
;

NOP	.macro
	SUB	TEMP_VARIABLE_0, TEMP_VARIABLE_0, 0
	.endm

;********************************************************************
; DELAY_2 : The macro to cause a delay of '2' cycles.
;

DELAY_2	.macro
	NOP
	NOP
	.endm

;********************************************************************
; THE_DELAY : The macro to cause a configurable delay

; cycles : The register that contains the number of cycles to be
; delayed. The content of the register will be unaltered. To do this
; the macro uses TEMP_VARIABLE_0 as the output register for the first
; SUB instruction.
;
; The macro can produce a delay of only odd number of cycles.
; Therefore the value contained in the 'cycles' should always be an
; odd number (1, 3, 5, ..)
;

THE_DELAY	.macro cycles
		QBEQ	$ED?, cycles, 1
		SUB	TEMP_VARIABLE_0, cycles, 3
		QBEQ	$ED?, TEMP_VARIABLE_0, 0
$MD?		SUB	TEMP_VARIABLE_0, TEMP_VARIABLE_0, 2
		QBNE	$MD?, TEMP_VARIABLE_0, 0
$ED?
		.endm

;********************************************************************
; DELAY_SAMPLE : To cause delay between samples. The macro takes
; CYCLE_BTWN_SAMPLE value, given by PRU0 and uses it to cause a delay
; between consecutive samples.
;
; Instruction: "SUB TEMP_VARIABLE_0, CYCLE_BTWN_SAMPLE, 1 + 2 + 2 + 2"
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
                SUB     TEMP_VARIABLE_0, CYCLE_BTWN_SAMPLE, 1 + 2 + 2 + 2
                QBEQ    $ES?, TEMP_VARIABLE_0, 0
$MS?:           SUB     TEMP_VARIABLE_0, TEMP_VARIABLE_0, 2
                QBNE    $MS?, TEMP_VARIABLE_0, 0
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
			LDI32	TEMP_VARIABLE_0, INT_P0_to_P1
			SBCO	&TEMP_VARIABLE_0, CONST_PRU_ICSS_INTC, SICR_offset, 4
			LDI32	TEMP_VARIABLE_1, SHARED_MEM_ADDR
			LBBO	&SAMPLING_CONFIG_START, TEMP_VARIABLE_1, 0, SAMPLING_CONFIG_LENGTH
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
; one sample using, delays for some cycles and then takes another
; sample. The delay it causes are not optimized yet.
;
; The macro uses MVI instruction and the MVI_POINTER register to move
; 1 byte from R31.b0 to the address pointed to by MVI_POINTER in a
; loop.
; The value of MVI_POINTER also gets incremented every time the MVI
; instruction is executed.
;
; THE_DELAY macro follows all the CLK_TOGGLE previous to the MVIB or
; MOV instruction. And All the MOV or MVIB instruction that take data
; sample are also followed by THE_DELAY macro. The delay that they
; produce is of CYCLE_BEFORE_SAMPLE and CYCLE_AFTER_SAMPLE cycles
; respectively. The values of these register come from the
; configuration data send by the user. Thus these delays can be used
; to produce a variable clock duty cycle.
;
; After one $SC1? loop, the two more sample taking steps are performed
; separately. The Delay of 2 cycles that we have in every sampling
; step, is used to reset the values of COUNTER_REG and MVI_POINTER and
; to execute TRANSFER_AND_TELL macro in these two separate sample
; taking steps.
;
; The macro transfer this data to only one bank, given by the BANK_ID
; variable.
;
; In all, this macro takes 44 samples of 1byte at each and transfers
; that data to the scratch pad bank given by BANK_ID variable
;

SAMPLE_CYCLE_8	.macro BANK_ID

		;****** Initialization required for this macro
		LDI	COUNTER_REG, 0
		LDI	MVI_POINTER, &DATA_START_REGISTER

		;****** SC1 loop starts
$SC1?:
		CLK_TOGGLE
		THE_DELAY CYCLE_BEFORE_SAMPLE
		MVIB 	*MVI_POINTER++, R31.b0
		THE_DELAY CYCLE_AFTER_SAMPLE
		CLK_TOGGLE
		CHECK_INT
		THE_DELAY CYCLE_BTWN_SAMPLE
		ADD	COUNTER_REG, COUNTER_REG, 1
		QBNE 	$SC1?, COUNTER_REG, 44-2
		;****** SC1 loop ends

		;****** Separate sample taking step 1 starts
		CLK_TOGGLE
		THE_DELAY CYCLE_BEFORE_SAMPLE
		MOV	BYTE_43, R31.b0
		THE_DELAY CYCLE_AFTER_SAMPLE
		CLK_TOGGLE
		CHECK_INT
		LDI	COUNTER_REG, 0
		LDI	MVI_POINTER, &DATA_START_REGISTER
		THE_DELAY CYCLE_BTWN_SAMPLE
		;****** Separate sample taking step 1 ends

		;****** Separate sample taking step 2 starts
		CLK_TOGGLE
		THE_DELAY CYCLE_BEFORE_SAMPLE
		MOV	BYTE_44, R31.b0
		THE_DELAY CYCLE_AFTER_SAMPLE
		CLK_TOGGLE
		THE_DELAY CYCLE_BTWN_SAMPLE
		CHECK_INT
		TRANSFER_AND_TELL SP_BANK_0
		;****** Separate sample taking step 2 ends

		;****** SC2 loop starts
$SC2?:
		CLK_TOGGLE
		THE_DELAY CYCLE_BEFORE_SAMPLE
		MVIB 	*MVI_POINTER++, R31.b0
		THE_DELAY CYCLE_AFTER_SAMPLE
		CLK_TOGGLE
		CHECK_INT
		THE_DELAY CYCLE_BTWN_SAMPLE
		ADD	COUNTER_REG, COUNTER_REG, 1
		QBNE 	$SC2?, COUNTER_REG, 44-2
		;****** SC2 loop ends

		;****** Separate sample taking step 1 starts
		CLK_TOGGLE
		THE_DELAY CYCLE_BEFORE_SAMPLE
		MOV	BYTE_43, R31.b0
		THE_DELAY CYCLE_AFTER_SAMPLE
		CLK_TOGGLE
		CHECK_INT
		LDI	COUNTER_REG, 0
		LDI	MVI_POINTER, &DATA_START_REGISTER
		THE_DELAY CYCLE_BTWN_SAMPLE
		;****** Separate sample taking step 1 ends

		;****** Separate sample taking step 2 starts
		CLK_TOGGLE
		THE_DELAY CYCLE_BEFORE_SAMPLE
		MOV	BYTE_44, R31.b0
		THE_DELAY CYCLE_AFTER_SAMPLE
		CLK_TOGGLE
		THE_DELAY CYCLE_BTWN_SAMPLE
		CHECK_INT
		TRANSFER_AND_TELL SP_BANK_1
		;****** Separate sample taking step 2 ends

		;****** SC3 loop starts
$SC3?:
		CLK_TOGGLE
		THE_DELAY CYCLE_BEFORE_SAMPLE
		MVIB 	*MVI_POINTER++, R31.b0
		THE_DELAY CYCLE_AFTER_SAMPLE
		CLK_TOGGLE
		CHECK_INT
		THE_DELAY CYCLE_BTWN_SAMPLE
		ADD	COUNTER_REG, COUNTER_REG, 1
		QBNE 	$SC3?, COUNTER_REG, 44-2
		;****** SC3 loop ends

		;****** Separate sample taking step 1 starts
		CLK_TOGGLE
		THE_DELAY CYCLE_BEFORE_SAMPLE
		MOV	BYTE_43, R31.b0
		THE_DELAY CYCLE_AFTER_SAMPLE
		CLK_TOGGLE
		CHECK_INT
		LDI	COUNTER_REG, 0
		LDI	MVI_POINTER, &DATA_START_REGISTER
		THE_DELAY CYCLE_BTWN_SAMPLE
		;****** Separate sample taking step 1 ends

		;****** Separate sample taking step 2 starts
		CLK_TOGGLE
		THE_DELAY CYCLE_BEFORE_SAMPLE
		MOV	BYTE_44, R31.b0
		THE_DELAY CYCLE_AFTER_SAMPLE
		CLK_TOGGLE
		THE_DELAY CYCLE_BTWN_SAMPLE
		TRANSFER_AND_TELL SP_BANK_2
		;****** Separate sample taking step 2 ends

		; JMP instruction to form an infinite loop
		; the CHECK_INT step was removed from the above
		; sample taking step to compensate for cycle
		; usage by this JMP instruction
		JMP $SC1?
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
	QBEQ	sample_start, READ_MODE, BLOCK_READ

take_raw:
	TAKE_SAMPLE_8 BYTE_1
	TRANSFER_AND_TELL SP_BANK_0
	JMP int_loop

sample_start:
	QBBC int_loop, MISC_CONFIG_DATA, SAMPLING_START_BIT
sample_loop:
	SAMPLE_CYCLE_8 SP_BANK_0
	HALT
