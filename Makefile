##################################################
# Makefile for the BeagleScop project
##################################################

SUBDIRS=driver/

KDIR?=/lib/modules/$(shell uname -r)/build

.PHONY: all
all: 
	@cd $(SUBDIRS) && $(MAKE)

.PHONY: clean
clean:
	@cd $(SUBDIRS) && $(MAKE) clean

travis_test: 
	cd .travis_test && $(MAKE)
