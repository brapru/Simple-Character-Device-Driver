# Simple Character Device Driver

My attempt at a simple linux kernel module. When loaded, the cdev.ko module
creates a /dev/cdev device. Userspace can then read and write to the device.
There is a simple userspace program with independent read and write functions
to test the module. 

## Getting Started

These instructions will get you a copy of the project up and running on your
local machine for development and testing purposes.

### Build

To compile simply run:
```
make
```

To insert the module in the kernel:
```
sudo insmod cdev.ko
``` 

### Check Success

To confirm the module loaded correctly and created devices:
```
  $ lsmod | grep cdev
  cdev                   16384  0
  $ dmesg 
  [26080.388784] cdev: Initializing module
  [26080.388786] cdev: Major and Minor numbers successfully assigned.
                 Major: 240       Minor: 0
  [26080.388810] cdev: Module class_create() success.
  [26080.388947] cdev: Device successfully added.
  $ ls -la /dev | grep cdev
  crw-------   1 root root    240,   0 Jan 20 09:59 cdev
```

### Userspace Test

Run the accompanying userspace executable to ensure reading and writing to the
device work as expected. Note it takes the /dev/cdev as an argument. 
```
  $ sudo ./userspace_test
  Usage: ./userspace_test <device>

  $ sudo ./userspace_test /dev/cdev
  [-] Quick test for the cdev.ko module.
  
  [-] Opening /dev/cdev to write...
  [*] Enter text to write to /dev/cdev:
          [*] char device test
  
  [*] Press ENTER to read from file.
  
  [-] Opening /dev/cdev to read...
  [-] Output from /dev/cdev:
          [*] char device test
```

### Remove Module

Simply remove the module by:
```
  $ sudo rmmod cdev
```

## References

Jonathan Corbet, Alessandro Rubini, Greg Kroah-Hartman. "Chapter 3: Char
Drivers." *Linux Device Drivers* O'Reilly Media Inc, 2005.
