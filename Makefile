CONFIG_MODULE_SIG = n
TARGET_MODULE := bench_kernel_all

obj-m := $(TARGET_MODULE).o
bench_kernel_all-objs := \
				bench_kernel.o \
				xvmalloc.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all: bench_usr
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	$(RM) bench_usr out

load:
	sudo insmod $(TARGET_MODULE).ko
	
unload:
	sudo rmmod $(TARGET_MODULE) || true >/dev/null

bench_usr: bench_usr.c
	$(CC) -o $@ $^

PRINTF = env printf
PASS_COLOR = \e[32;01m
NO_COLOR = \e[0m
pass = $(PRINTF) "$(PASS_COLOR)$1 Passed [-]$(NO_COLOR)\n"

run: all
	$(MAKE) unload
	$(MAKE) load
	sudo taskset -c 5 ./bench_usr
	$(MAKE) unload