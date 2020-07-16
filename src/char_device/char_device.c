// Module
#include <linux/kernel.h>
#include <linux/module.h>

// Char device
#include <linux/device.h>
#include <linux/fs.h>

// kernel<->user space access
#include <asm/uaccess.h>

//////////////////////////////////////////////////////////////////////////////
/////////////////////////// Local Module Variables ///////////////////////////
//////////////////////////////////////////////////////////////////////////////

// The major device number that will be dynamically allocated
static int s_driver_major_num = -1;

// Device class structure - Used to create the device node on the FS
static struct class *s_driver_dev_class = NULL;

// Device structure (the actual device representation on the FS)
static struct device *s_driver_device = NULL;

//////////////////////////////////////////////////////////////////////////////
////////////////////////// File Operation Functions //////////////////////////
//////////////////////////////////////////////////////////////////////////////

static int char_device_open(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "Device open!\n");
	return 0;
}

static long char_device_ioctl(struct file *filp, unsigned int ioctl_num,
			      unsigned long ioctl_param)
{
	printk(KERN_INFO "Device ioctl #%d: %ld!\n", ioctl_num, ioctl_param);
	return 0;
}

static ssize_t char_device_read(struct file *filp, char *buff, size_t len,
				loff_t *offset)
{
	static int read_counter = 0;
	char temp_buff[] = "Hello from char device!\n";
	size_t temp_buff_len = strnlen(temp_buff, len);

	// Return the string every second read (to prevent infinite loop on `cat /dev/module0`)
	if (!read_counter) {
		// Copy the string to the user
		raw_copy_to_user(buff, temp_buff, temp_buff_len);
		read_counter = 1;
		return temp_buff_len;
	} else {
		read_counter = 0;
		return 0;
	}
}

static ssize_t char_device_write(struct file *filp, const char *buff,
				 size_t len, loff_t *offset)
{
	return len;
}

static int char_device_release(struct inode *inode, struct file *filp)
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////// Module API Functions ////////////////////////////
//////////////////////////////////////////////////////////////////////////////

// Device driver functions
static struct file_operations s_driver_ops = {
	.owner = THIS_MODULE,
	.open = char_device_open,
	.unlocked_ioctl = char_device_ioctl,
	.read = char_device_read,
	.write = char_device_write,
	.release = char_device_release,
};

int char_device_install(void)
{
	// Register the device
	if ((s_driver_major_num =
		     register_chrdev(0, MODULE_NAME, &s_driver_ops)) < 0) {
		printk(KERN_ERR "Failed registering the device (ERR - %d)!\n",
		       s_driver_major_num);

		return -1;
	}

	// Create class for the device
	if ((s_driver_dev_class = class_create(THIS_MODULE, MODULE_NAME)) ==
	    NULL) {
		printk(KERN_ERR "Failed creating class for the device!\n");

		return -1;
	}

	// Create the device on the FS (instead of using 'mknod')
	if ((s_driver_device = device_create(s_driver_dev_class, NULL,
					     MKDEV(s_driver_major_num, 0), NULL,
					     MODULE_NAME "%d", 0)) == NULL) {
		printk(KERN_ERR "Failed creating device!\n");

		return -1;
	}

	return 0;
}

void char_device_remove(void)
{
	// Ignore invalid device major values
	if (s_driver_major_num < 0)
		return;

	// Destroy the device
	device_destroy(s_driver_dev_class, MKDEV(s_driver_major_num, 0));

	// Destroy everything & unregister the device
	class_destroy(s_driver_dev_class);
	unregister_chrdev(s_driver_major_num, MODULE_NAME);
}

