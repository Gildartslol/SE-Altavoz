#include <linux/version.h>
#include <linux/init.h>
#include <linux/spinlock.h>
//#include <asm/io.h>
//#include <sys/io.h>
#include <linux/io.h>
#include "spkr-io.h"

#ifdef V3_1
#include <linux/i8253.h>
#endif

#ifdef V3_0
extern raw_spinlock_t i8225_lock
#endif


#define SPKR_REGISTRO_CONTROL 0x43
#define SPKR_REGISTRO_DATOS 0x42
#define REGISTRO_CONTROL_SISTEMA 0x61


DEFINE_SPINLOCK(spkrSpinLock);
unsigned long spkrFlags;

void set_spkr_frecuency(unsigned int frecuency){

	unsigned long flags;
	unsigned int frec = 1000;
	if (frecuency > 20 && frecuency < 32767)
		frec = PIT_TICK_RATE / frecuency;

	printk(KERN_INFO "FRECUENCIA FIJADA EN %d\n",frecuency);	
	
	outb_p(0xb6,SPKR_REGISTRO_CONTROL);

	raw_spin_lock_irqsave(&i8253_lock,flags);
	outb_p(frec & 0xff,SPKR_REGISTRO_DATOS);
	outb_p(frec >> 8,SPKR_REGISTRO_DATOS);
	raw_spin_unlock_irqrestore(&i8253_lock,flags);

}

void spkr_on(void){

	spin_lock_irqsave(&spkrSpinLock,spkrFlags);
	outb_p(inb_p(REGISTRO_CONTROL_SISTEMA) | 3, REGISTRO_CONTROL_SISTEMA);
	spin_unlock_irqrestore(&spkrSpinLock,spkrFlags);

}


void spkr_off(void){

	spin_lock_irqsave(&spkrSpinLock,spkrFlags);
	outb(inb_p(REGISTRO_CONTROL_SISTEMA) & 0xFC, REGISTRO_CONTROL_SISTEMA);
	spin_unlock_irqrestore(&spkrSpinLock,spkrFlags);
}
