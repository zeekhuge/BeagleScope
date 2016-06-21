# 
# Copyright (c) 2016 Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
# Makefile to make PRU_inlineASM_blinky project
#

# PRU_CGT environment variable must point to the TI PRU compiler directory. E.g.:
#(Linux) export PRU_CGT=/home/jason/ti/ccs_v6_1_0/ccsv6/tools/compiler/ti-cgt-pru_2.1.0
#(Windows) set PRU_CGT=C:/TI/ccs_v6_0_1/ccsv6/tools/compiler/ti-cgt-pru_2.1.0

ifndef PRU_CGT
define ERROR_BODY

************************************************************
PRU_CGT environment variable is not set. Examples given:
(Linux) export PRU_CGT=/home/jason/ti/ccs_v6_1_0/ccsv6/tools/compiler/ti-cgt-pru_2.1.0
(Windows) set PRU_CGT=C:/TI/ccs_v6_0_1/ccsv6/tools/compiler/ti-cgt-pru_2.1.0
************************************************************

endef
$(error $(ERROR_BODY))
endif


LINKER_COMMAND_FILE=./AM335x_PRU.cmd
LIBS=--library=../../lib/rpmsg_lib.lib
INCLUDE=--include_path=../../include --include_path=../../include/am335x
STACK_SIZE=0x100
HEAP_SIZE=0x100



CFLAGS=-v3 -O2 --display_error_number --endian=little --hardware_mac=on --obj_directory=$(GEN_DIR) --pp_directory=$(GEN_DIR) -ppd -ppa
LFLAGS=--reread_libs --warn_sections --stack_size=$(STACK_SIZE) --heap_size=$(HEAP_SIZE)

GEN_DIR=gen
TARGET=$(GEN_DIR)/pru1_inline_asm_blinky.out


.PHONY: all
all: $(TARGET)

$(TARGET): $(GEN_DIR)/main.object $(GEN_DIR)/pru1-asm-blinky.object 
	@echo 'LD	$@'
	@$(PRU_CGT)/bin/lnkpru -i$(PRU_CGT)/lib -i$(PRU_CGT)/include $(LFLAGS) -o $(TARGET) $(GEN_DIR)/main.object $(LINKER_COMMAND_FILE) --library=libc.a $(LIBS) $^
	@echo '*Generated firmware - $(TARGET)'


$(GEN_DIR)/main.object: main.c 
	@mkdir -p $(GEN_DIR)
	@echo 'CC	$<'
	@$(PRU_CGT)/bin/clpru --include_path=$(PRU_CGT)/include $(INCLUDE) $(CFLAGS) -fe $@ $<

$(GEN_DIR)/pru1-asm-blinky.object: pru1-asm-blinky.asm
	@mkdir -p $(GEN_DIR)
	@echo 'CC	$<'
	@$(PRU_CGT)/bin/clpru --include_path=$(PRU_CGT)/include $(INCLUDE) $(CFLAGS) -fe $@ $<

.PHONY: install copy_fw reboot_pru_1
install: $(TARGET) copy_fw reboot_pru_1

copy_fw: $(TARGET)
	@echo '*placing firmare at /lib/firmware/am335x_pru1_fw'
	@cp $(TARGET) /lib/firmware/am335x_pru1_fw

reboot_pru_1:
	@echo '*rebooting pru core 1'
	@echo "4a338000.pru1"  > /sys/bus/platform/drivers/pru-rproc/unbind 2> /dev/null
	@echo "4a338000.pru1" > /sys/bus/platform/drivers/pru-rproc/bind
	@echo '**pru core 1 is now loaded with $(TARGET)'	

.PHONY: clean
clean:
	@echo 'CLEAN	.'
	@rm -rf $(GEN_DIR)
