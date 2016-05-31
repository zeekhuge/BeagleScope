##################################################
# Makefile for the BeagleScop project
##################################################

SUBDIRS= .travis_test/
KDIR?=/lib/modules/$(shell uname -r)/build

.PHONY: all
all: travis_test


.PHONY: clean
clean:
	cd $(SUBDIRS) && $(MAKE) clean

travis_test: 
	cd .travis_test && $(MAKE)
