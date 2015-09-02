obj-m += chardevFile.o

all: module 

test: 
	gcc test.c -o test


module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f test


i ins insmod:
	sudo dmesg -C
	sudo insmod chardevFile.ko
	sudo mknod /dev/chardevFile c `cat /proc/devices | grep chardevFile | awk '{print $$1}'` 0
	sudo chmod 666 /dev/chardevFile /tmp/chardevFile

r rm rmm rmmod:
	sudo rm -f /dev/chardevFile
	sudo rmmod chardevFile

