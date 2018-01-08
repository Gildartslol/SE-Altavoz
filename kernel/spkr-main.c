#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/init.h>		/* Needed for the macros */

#include <linux/version.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/kfifo.h>
#include <linux/ioctl.h>
#include <asm/uaccess.h>

#include "version.h"
#include "spkr-io.h"

MODULE_LICENSE("Dual BSD/GPL");

static int __init init_module(void)
{
	printk(KERN_INFO "Hello, world 2\n");
	return 0;
}

static void __exit exit_module(void)
{
	printk(KERN_INFO "Goodbye, world 2\n");
}

module_init(init_module);
module_exit(exit_module);
