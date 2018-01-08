#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/init.h>		/* Needed for the macros */

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