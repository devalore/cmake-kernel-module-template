#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include "char_device/char_device.h"

static int __init example_module_start(void)
{
	printk(KERN_INFO "Loading module: " MODULE_NAME "\n");
	char_device_install();
	return 0;
}

static void __exit example_module_end(void)
{
	printk(KERN_INFO "Stopping module: " MODULE_NAME "\n");
	char_device_remove();
}

module_init(example_module_start);
module_exit(example_module_end);

// Module License
MODULE_LICENSE("GPL");

// Module Author
MODULE_AUTHOR("Vitaly Bukhovsky");

// Module Description
MODULE_DESCRIPTION("Example Kernel Module");

// Module Version
MODULE_VERSION("0.1");
