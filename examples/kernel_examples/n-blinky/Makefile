#
# Copyright (C) 2016 Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
#
# Makefile to compile n-blinky
#

.PHONY:all
all: firmware module

.PHONY:firmware
firmware:
	cd firmware; make

.PHONY:module
module: 
	cd module; make

.PHONY:clean
clean: 
	cd firmware; $(MAKE) clean
	cd module; $(MAKE) clean	

