#
# Makefile written by Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
# Copyright (C) 2016 Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
#
# Makefile to compile rpmsg_pru_parallel_example.c
#

obj-m += rpmsg_pru_parallel_example.o

KDIR?=/lib/modules/$(shell uname -r)/build


.PHONY: all
all:
	@echo "**Starting to compile module source"
	make -C $(KDIR) M=$(PWD) modules
	@echo "**Module compilation complete"

.PHONY: clean
clean:
	@echo "**Starting to clean modules"
	make -C $(KDIR) M=$(PWD) clean
	@echo "**Clean complete"
