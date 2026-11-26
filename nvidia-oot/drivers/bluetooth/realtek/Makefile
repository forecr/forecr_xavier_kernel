ifeq ($(CONFIG_TEGRA_SYSTEM_TYPE_ACK),y)
	ccflags-y += -DTEGRA_SYSTEM_TYPE_ACK
endif

ifneq ($(KERNELRELEASE),)
ifeq ($(CONFIG_USB),y)
	obj-m := btrtlusb.o
	btrtlusb-y = btrtl_coex.o btrtl_misc.o btrtl_usb.o
endif
else
	PWD := $(shell pwd)
	KVER := $(shell uname -r)
	KDIR := /lib/modules/$(KVER)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	rm -rf *.o *.mod.c *.mod.o *.ko *.symvers *.order *.a

endif
