
/*
 * Prom setup file for ar7100
 */

#include <linux/init.h>
#include <linux/autoconf.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/bootmem.h>

#include <asm/bootinfo.h>
#include <asm/addrspace.h>
#include "ar7100.h"


#define SERIAL_BASE   AR7100_UART_BASE
#define SER_DATA      0x0

static volatile unsigned long * const com1 = (unsigned long *)SERIAL_BASE;
int __ath_flash_size;

#ifdef CONFIG_EARLY_PRINTK
static inline void slow_down(void)
{
    int k;
    for (k=0; k<100000; k++);
}

void prom_putchar(const unsigned char c)
{
	ar7100_reg_wr_nf((SERIAL_BASE + SER_DATA), c);
	slow_down();
}
#endif /* CONFIG_EARLY_PRINTK */

void __init prom_init(void)
{
	int memsz = 0x2000000, argc = fw_arg0, i;
	char **arg = (char**) fw_arg1;

	printk ("flash_size passed from bootloader = %ld\n", fw_arg3);
	__ath_flash_size = fw_arg3;

#warning "prom_init ignore kernel argment."
#if 0
	/* 
	 * if user passes kernel args, ignore the default one 
	 */
	if (argc > 1) {
		arcs_cmdline[0] = '\0';

		for (i = 1; i < argc; i++) 
			printk("arg %d: %s\n", i, arg[i]);

		/* 
		 * arg[0] is "g", the rest is boot parameters 
		 */
		for (i = 1; i < argc; i++) {
			if (strlen(arcs_cmdline) + strlen(arg[i] + 1) >= sizeof(arcs_cmdline))
				break;
			strcat(arcs_cmdline, arg[i]);
			strcat(arcs_cmdline, " ");
		}
	}
#endif

	mips_machtype  = MACH_ATHEROS_AP81;

	/*
	 * By default, use all available memory.  You can override this
	 * to use, say, 8MB by specifying "mem=8M" as an argument on the
	 * linux bootup command line.
	 */
	 add_memory_region(0, memsz, BOOT_MEM_RAM);
}

void __init prom_free_prom_memory(void)
{
}
