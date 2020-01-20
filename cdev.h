#ifndef __CDEV__
#define __CDEV__

#include <linux/device.h>
#include <linux/module.h>
#include <linux/mutex.h>

#define MODULE_NAME "cdev" /* Device name as it appeas in /proc/devices */
#define DEVICE_NAME "cdev" /* Will probably be different, for this case I just kept name as the name.  */

/* Device struct 
 * ------------
 * Description: Device as it is represented in the kernel. Need the mutex to
 * prevent race conditions while userspace read/writes to the device. 
 * */
struct dev_device {
  struct cdev cdev;
  char buffer[256];
  struct mutex lock;
  size_t size;
};

#endif /* __CDEV__  */
