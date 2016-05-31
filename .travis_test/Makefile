
# Makefile for travis_test.ko

obj-m += travis_test.o
	

KDIR?=/lib/modules/$(shell uname -r)/build


travis_test: travis_test.c
	make -C $(KDIR) M=$(PWD) modules	

.PHONY: clean
clean:
	make -C $(KDIR) M=$(PWD) clean
