	obj-m := spkr.o
	spkr-objs := spkr-main.o spkr-io.o
default:
	make -C $(KERNEL_PATH) M=$(PWD) modules

clean:
	make -C $(KERNEL_PATH) M=$(PWD) clean
