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

#include <linux/limits.h>

//#include "version.h"
#include "spkr-io.h"

MODULE_AUTHOR("Jorge Amoros , Antonio Perea");
MODULE_DESCRIPTION("PC Speaker beeper driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:pcspkr");
MODULE_LICENSE("Dual BSD/GPL");

static int const CHAR_BIT = 8;

struct dispositivo
{
	
	dev_t devTDispositivo;
	struct cdev dev;
	struct class *class;
	struct device *device;


	//sync

	rwlock_t lock_escritura;
	unsigned long flagsEscritura;

	spinlock_t lock_escritura_buffer;
	unsigned long flags_escritura_buffer;

	spinlock_t lock_kfifo;
	unsigned long flags_lock_kfifo;

	wait_queue_head_t lista_bloq;

	//modulo
	struct kfifo cola_fifo;
	struct timer_list contador;
	int activo;
	int resetearColaFifo;
	int silencio;


	int terminado;

} disp;

static unsigned int minor = 0;
static unsigned int tamanio_buffer = PAGE_SIZE;
unsigned int limite_buffer = -1;

module_param(minor,int, S_IRUGO);
module_param(tamanio_buffer, int, S_IRUGO);
module_param(limite_buffer, int, S_IRUGO);

static int abrir(struct inode *inode, struct file *descriptor){

	if((descriptor->f_mode & O_ACCMODE) == FMODE_READ){

		printk(KERN_INFO "Abriendo modulo\n");

	}else{
			if(((descriptor->f_mode & O_ACCMODE) == FMODE_WRITE && (write_trylock(&(disp.lock_escritura))==0))){
				return -EBUSY;
			}
		printk(KERN_INFO "Abriendo module\n");
	}

	
	return 0;
}

static int cerrar(struct inode *inode, struct file *descriptor){

	if((descriptor->f_mode & O_ACCMODE) == FMODE_READ){

		printk(KERN_INFO "Close module\n");

	}else{
			if((descriptor->f_mode & O_ACCMODE) == FMODE_WRITE)
				write_unlock_irqrestore(&(disp.lock_escritura),disp.flagsEscritura);
			
	}



	printk(KERN_INFO "Close Module \n");
	return 0;
}

static long ioctl_function(struct file *descriptor, unsigned int cmd, unsigned long arg){

	return 0;
}

static int spkr_fsync(struct file *descriptor, loff_t start, loff_t end, int datasync){
	return 0;
}

void sonando(unsigned long countAux){

			unsigned char sonido[4];
			unsigned int frec, ms;
			int tamanio = 4;
			disp.activo = 1;

			//spkr_off();
			printk(KERN_INFO "Speaker OFF");		
			if(disp.resetearColaFifo == 0){

				if(kfifo_len(&(disp.cola_fifo)) >= 4 ){


					// no se necesita extra locking para un lector y un escritor.
					int i  = kfifo_out(&(disp.cola_fifo),sonido,tamanio);
					frec = (unsigned char)sonido[0] << CHAR_BIT | (unsigned char)sonido[1];
					ms = (unsigned char)sonido[2] << CHAR_BIT | (unsigned char)sonido[3];

					printk(KERN_INFO "Primero %s  Segundo %s",sonido[0],sonido[1]);		
					printk(KERN_INFO "Tercero %s  Cuarto %s",sonido[2],sonido[3]);
					printk(KERN_INFO "Frecuencia %d  Tiempo %d",frec,ms);

					disp.contador.data = countAux;
					disp.contador.expires = jiffies + msecs_to_jiffies(ms);

					
					if(frec != 0){

							if(!disp.silencio){
									spkr_on();
									printk(KERN_INFO "Speaker ON");	
							}
					}

					add_timer(&(disp.contador));
					//comprobar que hay mas sonidos

					if(kfifo_avail(&(disp.cola_fifo)) >= countAux){
						wake_up_interruptible(&(disp.lista_bloq));
					}
					if(kfifo_avail(&(disp.cola_fifo)) >= limite_buffer){
						wake_up_interruptible(&(disp.lista_bloq));
					}

				}else{
					disp.activo = 0;
					if(countAux == 0){

						disp.resetearColaFifo = 0;
						disp.terminado = 1;
						// fsync
					}
				}

			}else{


				kfifo_reset_out(&(disp.cola_fifo));
				disp.resetearColaFifo = 0;
				disp.activo = 0;
				// if auxCount == 0...

			}


}

static ssize_t escribir(struct file *descriptor, const char __user *buf, size_t count , loff_t *f_pos){
	printk(KERN_INFO "Write Module \n");



	unsigned int countAux = count;
	unsigned int despl = 0;
	unsigned int copiado;

	//seccion critica
	spin_lock_irqsave(&(disp.lock_escritura_buffer),disp.flags_escritura_buffer);
	

	disp.terminado = 0;

	while(countAux > 0 ){

		int isFull = kfifo_is_full(&(disp.cola_fifo));
		// Bloqueamos el proceso hasta que se cumpla la condicion, que la fifo no esté llena.
		if(wait_event_interruptible(disp.lista_bloq, !isFull) != 0 ){
				// el bloqueo queda cancelado debido a que devuelve diferente de 0. Soltamos el lock y devolvemos error.
			spin_unlock_irqrestore(&(disp.lock_escritura_buffer),disp.flags_escritura_buffer);
			return -ERESTARTSYS;

		}

		//kfifo_from_user(fifo,from,len,copied)
		if(kfifo_from_user(&(disp.cola_fifo),buf+despl,countAux, &copiado)!=0){
			//algun error, volvemos a liberar el lock
			spin_unlock_irqrestore(&(disp.lock_escritura_buffer),disp.flags_escritura_buffer);	
			return -EFAULT;
		}

		//actualizamos

		despl += copiado;
		countAux -= copiado;
		printk(KERN_INFO "-copiado %d  -desplazamiento %d  -porCopiar %d \n",copiado,despl,countAux);	
		if(!disp.activo)
			sonando(countAux);
			
	}

	spin_unlock_irqrestore(&(disp.lock_escritura_buffer),disp.flags_escritura_buffer);

	return count;

}

static struct file_operations fileop = {
	.owner = THIS_MODULE,
	.open = abrir,
	.release = cerrar,
	.write = escribir,
	.fsync = spkr_fsync,
	.unlocked_ioctl = ioctl_function
};



void setUpDispositivo(void){

	alloc_chrdev_region(&(disp.devTDispositivo),minor,1,"spkr");
	disp.dev.owner = THIS_MODULE;
	cdev_init(&(disp.dev), &fileop);
	cdev_add(&(disp.dev),disp.devTDispositivo,1);
	disp.class = class_create(THIS_MODULE,"speaker");
	disp.device = device_create(disp.class,NULL,disp.devTDispositivo,NULL,"intspkr");
}

void setUpVariablesSync(void){
	rwlock_init(&(disp.lock_escritura));
	spin_lock_init(&(disp.lock_escritura_buffer));
	spin_lock_init(&(disp.lock_kfifo));
	//cola de espera
	init_waitqueue_head(&disp.lista_bloq);

}

void setUpTemporales(void){


	init_timer(&(disp.contador));
	disp.contador.function = sonando;

	disp.activo = 0;
	disp.silencio = 0;

}

int setUpFifo(void){

	disp.resetearColaFifo = 0;

	int error = kfifo_alloc(&(disp.cola_fifo),tamanio_buffer,GFP_KERNEL);
	return error;

}

void setUpPruebas(void){


	int mj, mn;
	mj=MAJOR(disp.devTDispositivo);
	mn=MINOR(disp.devTDispositivo);
	printk(KERN_INFO "-minor %d   -major %d \n",mn,mj);

}


static int __init setUp(void)
{
	printk(KERN_INFO "Entering module\n");
	int error;
	error = setUpFifo();
	if(error != 0)
		return -ENOMEM;

	//dispositivo
	setUpDispositivo();
	
	//sync variables
	setUpVariablesSync();


	//temporales

	setUpTemporales();

	//Pruebas
	setUpPruebas();

	

	return 0;
}



static void __exit setDown(void)
{
	printk(KERN_INFO "Exiting Module \n");

	device_destroy(disp.class,disp.devTDispositivo);
	class_destroy(disp.class);
	cdev_del(&(disp.dev));
	unregister_chrdev_region(disp.devTDispositivo,1);


}

module_init(setUp);
module_exit(setDown);
