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

//#include "version.h"
#include "spkr-io.h"

MODULE_AUTHOR("Jorge Amoros , Antonio Perea");
MODULE_DESCRIPTION("PC Speaker beeper driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:pcspkr");
MODULE_LICENSE("Dual BSD/GPL");

struct dispositivo
{
	
	dev_t devTDispositivo;
	struct cdev dev;
	struct class *class;
	struct device *device;


	//sync

} disp;

static unsigned int minor = 0;
static unsigned int tamanio_buffer = PAGE_SIZE;
unsigned int buffer_limite = -1;




static int abrir(struct inode *inode, struct file *filep){
	return 0;
}

static int cerrar(struct inode *inode, struct file *filep){

	return 0;
}

static long ioctl_function(struct file *filep, unsigned int cmd, unsigned long arg){

	return 0;
}

static int spkr_fsync(struct file *filep,int dSyc){
	return 0;
}

static ssize_t escribir(struct file *filep, const char __user *buf, size_t count , loff_t *f_pos){


}

static struct file_operations fileop = {
	.owner = THIS_MODULE,
	.open = abrir,
	.release = cerrar,
	.write = escribir,
	.fsync = spkr_fsync,
	.unlocked_ioctl = ioctl_function
};

static int __init init_module(void)
{
	printk(KERN_INFO "Entering module\n");


	alloc_chrdev_region(&(disp.devTDispositivo),minor,1,"spkr");
	disp.dev.owner = THIS_MODULE;
	cdev_init(&(disp.dev), &fileop);
	cdev_add(&(disp.dev),disp.devTDispositivo,1);
	disp.class = class_create(THIS_MODULE,"speaker");
	disp.device = device_create(disp.class,NULL,disp.devTDispositivo,NULL,"intspkr");


	int mj, mn;
	mj=MAJOR(disp.devTDispositivo);
	mn=MINOR(disp.devTDispositivo);
	printk(KERN_INFO "-minor %d   -major %d \n",mn,mj);

	return 0;
}

static void __exit exit_module(void)
{
	printk(KERN_INFO "Exiting Module \n");

	device_destroy(disp.class,disp.devTDispositivo);
	class_destroy(disp.class);
	cdev_del(&(disp.dev));
	unregister_chrdev_region(disp.devTDispositivo,1);


}

module_init(init_module);
module_exit(exit_module);
