obj-m += mychardev.o

KDIR := /lib/modules/6.6.40-xilinx-g2b7f6f70a62a/build
PWD  := $(shell pwd)

# Build kernel module + test_client with 'make all'
all: mychardev.ko test_client test_numbers

# Kernel module build rule
mychardev.ko:
	make -C $(KDIR) M=$(PWD) modules

# User-space test_client build rule
test_client: test_client.c
	$(CC) -Wall -O2 -o test_client test_client.c

test_numbers: test_numbers.c
	$(CC) -Wall -O2 -o test_numbers test_numbers.c

uio: uio.c
	$(CC) -Wall -O2 -o uio uio.c

# Clean both kernel and userspace objects
clean:
	make -C $(KDIR) M=$(PWD) clean
	$(RM) test_client
	$(RM) test_numbers
	$(RM) uio

# Reload the kernel module: remove + insert
reload: all
	sudo rmmod mychardev || true
	sudo insmod mychardev.ko

.PHONY: all clean reload