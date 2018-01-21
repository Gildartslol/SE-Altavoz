void set_spkr_frecuency(unsigned int frecuency);
void spkr_on(void);
void spkr_off(void);


#if LINUX_VERSION_CODE == KERNEL_VERSION(3,0,0)
	#define V3_0
#elif LINUX_VERSION_CODE > KERNEL_VERSION(3,0,0)
	#define V3_1
#endif
