obj-m += mychardev.o

all:
	make -C /lib/modules/6.6.40-xilinx-g2b7f6f70a62a/build/ M=$(PWD) modules

clean:
	make -C /lib/modules/6.6.40-xilinx-g2b7f6f70a62a/build/  M=$(PWD) clean
