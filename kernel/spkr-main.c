#include <linux/init.h>		/* Needed for the macros */
#include <linux/module.h>	/* Needed by all modules */

#include <linux/version.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/kfifo.h>
#include <linux/ioctl.h>
#include <asm/uaccess.h>

#include "spkr-io.h"

MODULE_LICENSE("Dual BSD/GPL");

 int __init setUp(void)
{
	printk(KERN_INFO "Hello, world 2\n");
	return 0;
}

 void __exit setDown(void)
{
	printk(KERN_INFO "Goodbye, world 2\n");
}

module_init(setUp);
module_exit(setDown);
