#
# Makefile written by Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
# Copyright (C) 2016 Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
#
#

obj-m += beaglescope_driver.o

KDIR?=/lib/modules/$(shell uname -r)/build
INSTALL?=/lib/modules/$(shell uname -r)/kernel/drivers/testing
OP=$(patsubst %.o,%.ko,$(obj-m))
MODNAME=$(patsubst %.o,%,$(obj-m))

.PHONY: all driver
all: driver

driver:
	@make -C $(KDIR) M=$(PWD) modules

.PHONY: install install-driver install-testing

install: all
	@mkdir -p $(INSTALL)
	@for file in $(OP) ; do					 \
	echo "  -	copying $$file driver to $(INSTALL)"    ; \
	cp $$file $(INSTALL)					; \
	done
	@echo "  -	executing depmod -a"
	@depmod -a

.PHONY: load
load: install
	@for file in $(MODNAME) ; do		  \
	echo "  -	rmmod $$file"		; \
	rmmod $$file | true			; \
	done
	@for file in $(MODNAME) ; do		  \
	echo "  -	modprobe $$file"	; \
	modprobe $$file				; \
	done

.PHONY: clean
clean:
	@make -C $(KDIR) M=$(PWD) clean
	@cd testing && $(MAKE) clean
