ifneq ($(KERNELRELEASE),)
obj-m:=switcolor.o
else
 #generate the path
CURRENT_PATH:=$(shell pwd)
 #the absolute path
#LINUX_KERNEL_PATH:=/lib/modules/$(shell uname -r)/build
LINUX_KERNEL_PATH:= /work/a31/a31_sdk/lichee/linux-3.3/
 #complie object
default:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules
clean:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean
endif
