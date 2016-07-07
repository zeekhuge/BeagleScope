#
# Makefile written by Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
# Copyright (C) 2016 Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
#
# Makefile to compile rpmsg_pru_parallel_example.c
#

obj-m += beaglescope_driver.o

KDIR?=/lib/modules/$(shell uname -r)/build
INSTALL?=/lib/modules/$(shell uname -r)/kerenl/drivers/testing
OP=$(patsubst %.o,%.ko,$(obj-m))
MODNAME=$(patsubst %.o,%,$(obj-m))

.PHONY: all
all:
	@make -C $(KDIR) M=$(PWD) modules

.PHONY: install
install: all
	@mkdir -p $(INSTALL)
	@echo "  -	copying $(OP) driver to $(INSTALL)"
	@cp $(OP) $(INSTALL)
	@echo "  -	executing depmod -a"
	@$(shell depmod -a)

.PHONY: clean
clean:
	@make -C $(KDIR) M=$(PWD) clean
	
