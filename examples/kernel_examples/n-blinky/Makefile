#
# Makefile written by Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
# Copyright (C) 2016 Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
#
# Makefile to compile n-blinky
#

DIR	=module firmware
#DIR	+=firmware

all:
	for dir in $(DIR); do\
		cd $$dir && $(MAKE);\
	done

clean: 
	for dir in $(DIR); do\
		cd $$dir && $(MAKE) clean;\
	done

