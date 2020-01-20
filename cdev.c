#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>

/* Includes Macros and struct definitions */
#include "cdev.h"

MODULE_LICENSE("GPL"); /* So gcc will quit yelling at me  */
MODULE_AUTHOR("brapru");
MODULE_DESCRIPTION("Simple Linux Kernel character driver to read and write.");

/* Function Declaration  */
static void cleanup_cdev(void);
int dev_open(struct inode *inode, struct file *file);
int dev_release(struct inode *inode, struct file *file);
ssize_t dev_read(struct file *file, char __user *user_buf, size_t count, loff_t *offset);
ssize_t dev_write(struct file *file, const char __user *user_buf, size_t count, loff_t *offset);

static struct file_operations fops = {
	.owner = 	  THIS_MODULE,
	.open =			dev_open,
	.release =	dev_release,
	.read = 		dev_read,
	.write = 		dev_write,
};

/* Define all global variables here */
static dev_t dev;
static int err, major, minor;
static struct class *dev_class = NULL;
static struct dev_device dev_device;

/* Function: init_cdev
 * -------------------
 * Description: Handles initialization of module:
 *  - Kernel dynamically assigns major number
 *  - Creates a device class in order to create device nodes in sysfs
 *  - Creates devices and device nodes. If success, can see in /dev/
 *
 * Returns: Returns zero if successful, or negative int if error occurs during
 * building process. If returns zero, the module will be loaded, and will wait
 * for open, release, read, write function calls. 
 */
static int __init init_cdev(void){

  printk(KERN_INFO "%s: Initializing module", MODULE_NAME);
 
  /* Ask the kernel to dynamically assign us a major number */
  err = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME); 

  if (err < 0) {
    printk(KERN_ALERT "%s: Cannot assign device number.\n", MODULE_NAME);
    return err;
  }
  
  major = MAJOR(dev);
  minor = MINOR(dev);
  printk(KERN_INFO "%s: Major and Minor numbers successfully assigned.\nMajor: %d\tMinor: %d\n", MODULE_NAME, major, minor);
  
  /* Appears in /sys/class */
  dev_class = class_create(THIS_MODULE, DEVICE_NAME);
  if (dev_class == NULL){
    printk(KERN_ALERT "%s: Failed to create the class.\n", MODULE_NAME);
    err = -1;
    goto FAIL;
  }
  printk(KERN_INFO "%s: Module class_create() success.\n", MODULE_NAME);
 
  /* Initialize and create the cdev device */
  if (device_create(dev_class, NULL, MKDEV(major, minor), NULL, DEVICE_NAME) == NULL){
    printk(KERN_ALERT "%s: Failed to create the device.\n", MODULE_NAME);
    err = -1;
    goto FAIL;
  }

  cdev_init(&dev_device.cdev, &fops);
  dev_device.cdev.owner = THIS_MODULE;
 
  err = cdev_add(&dev_device.cdev, MKDEV(major, minor), 1);
  if (err < 0){
    printk(KERN_ALERT "%s: Failed to add the device.\n", MODULE_NAME);
    err = -1;
    goto FAIL;
  }
  
  printk(KERN_INFO "%s: Device successfully added.\n", MODULE_NAME);
  
  return 0; /* Success */

FAIL:
  cleanup_cdev();
  return err; 

}

/* Function: cleanup_cdev
 * -------------------
 * Description: Cleanup routine for this module. Ensures that all resources
 * from init_cdev are appropriately torn down.
 */
static void cleanup_cdev(void){

  device_destroy(dev_class, MKDEV(major, minor)); 
  class_unregister(dev_class);
  class_destroy(dev_class); 
  unregister_chrdev_region(dev, 1);
  printk(KERN_INFO "%s: Cleaned up module.\n", MODULE_NAME);

}

/* Function: dev_open
 * -------------------
 * Description: Simple function that is called each time the device is opened.
 *
 * Params:
 *  @inode: Pointer to the current inode, which contains the cdev structure set
 *  up in init_cdev. 
 *  @file: Pointer to current file descriptor struct after open syscall.
 *
 * Returns: Will always succeed, returns with 0.
 *
 */
int dev_open(struct inode *inode, struct file *file){
  /* Set device information  */ 
  struct dev_device *dev = container_of(inode->i_cdev, struct dev_device, cdev); 
  
  /* Validate the access to the data  */
  file->private_data = dev;

  if(!mutex_trylock(&dev->lock)){
    printk(KERN_ALERT "%s: Device already in use.", MODULE_NAME); 
    return -EBUSY;
  }
  
  printk(KERN_INFO "%s: /dev/%s opened", MODULE_NAME, DEVICE_NAME); 
  return 0; /* Success */
}

/* Function: dev_release
 * -------------------
 * Description: Whenever the device is closed/released by the userspace
 * program. 
 *
 * Params:
 *  @inode: Pointer to the current inode, which contains the cdev structure set
 *  up in init_cdev. 
 *  @file: Pointer to current file descriptor struct after open syscall.
 * 
 * Returns:
 *
 */

int dev_release(struct inode *inode, struct file *file){
  struct dev_device *dev = file->private_data;
  mutex_unlock(&dev->lock);

  printk(KERN_INFO "%s: /dev/%s released", MODULE_NAME, DEVICE_NAME); 
  return 0;
}

/* Function: dev_read
 * -------------------
 * Description: Called whenever the device is being read from user space (i.e.
 * data is transfered from device to user space. Uses copy_to_user function to
 * send the buffer string to user space.
 *
 * Params:
 *  @file: File pointer
 *  @user_buf: Points to the empty user buffer where the newly read data will
 *  be placed.
 *  @count: Size of the requested data transfer.
 *  @offset: File position the user is accessing.
 *
 * Returns:
 */
ssize_t dev_read(struct file *file, char __user *user_buf, size_t count, loff_t *offset){
  
  struct dev_device *dev = file->private_data;
  //ssize_t retval = -ENOMEM;
  ssize_t retval = 0;

  if (*offset >= dev->size){
    return retval;
  }
  if (*offset + count > dev->size){
    count = dev->size - *offset;
  }
  
  
  if (copy_to_user(user_buf, (dev->buffer + *offset), count)) {
    printk(KERN_ALERT "%s: Failed to copy /dev/%s data to user space.\n", MODULE_NAME, DEVICE_NAME);
    return -EFAULT;
  }
 
  *offset += count;
  retval = count;
  printk(KERN_INFO "%s: Sent %zu chars to user space.\n", MODULE_NAME, retval);

  return retval;

}

/* Function: dev_write
 * -------------------
 * Description: Called whenever user space writes to the device (i.e. data
 * transfered to device).
 *
 * Params:
 *  @file: File Pointer
 *  @user_buf: Points to the user buffer holding the data to be transfered.
 *  @count: Size of the requested data transfer.
 *  @offset: File position the user is accessing.
 *
 * Returns: The number of bytes requested to be read from the device.
 *
 */
ssize_t dev_write(struct file *file, const char __user *user_buf, size_t count, loff_t *offset){
  
  struct dev_device *dev = file->private_data;
  ssize_t retval = -ENOMEM;

  if (copy_from_user(dev->buffer, user_buf, count)){
    printk(KERN_ALERT "%s: Failed to copy user space data to /dev/%s.\n", MODULE_NAME, DEVICE_NAME);
    return -EFAULT;
  }
  
  *offset += count;
  retval = count;

  if (dev->size < *offset){
    dev->size = *offset;
  }

  printk(KERN_INFO "%s: Received %zu chars from user space\n", MODULE_NAME, retval);
  return retval;

}

module_init(init_cdev);
module_exit(cleanup_cdev);
