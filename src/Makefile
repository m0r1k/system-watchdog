.PHONY: default prep build install insmod rmmod dmesg clean

TARGET  = system_watchdog
KDIR    = /lib/modules/`uname -r`/build

obj-m           += $(TARGET).o
$(TARGET)-objs  := system_main.o system_netfilter.o system_thread1.o system_restart.o system_sysfs.o
PWD             := $(shell pwd)

default: build

prep:
	apt install -y linux-headers-$(shell uname -r)

build:
	make -C $(KDIR) M=$(PWD) modules

install: build
	make -C $(KDIR) M=$(PWD) modules_install
	depmod -a

insmod: build
	insmod ./$(TARGET).ko mgmt_if_name=enp4s0

rmmod:
	rmmod $(TARGET)

dmesg:
	dmesg

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f *.o *.ko *.mod *.mod.c modules.order Module.symvers

