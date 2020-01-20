obj-m += cdev.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -Wall -Werror -O2 -o userspace_test userspace_test.c

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
