/**
 * @file
 *
 * Addtional customize for inflate.
 * 
 * Copyright (C) 2008 - 2009 silex technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __KERNEL__
#define __KERNEL__
#endif

#include <linux/autoconf.h>
#include <asm/io.h>
#include <asm/cache.h>
#include <ar7100.h>

/*
 * gzip declarations
 */

#define OF(args)  args
#define STATIC static

#undef memset
#undef memcpy
#undef memcmp
#undef puts
#define memzero(s, n)     memset ((s), 0, (n))

typedef unsigned char  uch;
typedef unsigned short ush;
typedef unsigned long  ulg;

#define WSIZE 0x8000			/* Window size must be at least 32k, */
								/* and a power of two */

static uch *inbuf;				/* input buffer */
static uch window[WSIZE];		/* Sliding window buffer */

static unsigned insize = 0;		/* valid bytes in inbuf */
static unsigned inptr = 0;		/* index of next byte to be processed in inbuf */
static unsigned outcnt = 0;		/* bytes in output buffer */

/* gzip flag byte */
#define ASCII_FLAG   0x01		/* bit 0 set: file probably ASCII text */
#define CONTINUATION 0x02		/* bit 1 set: continuation of multi-part gzip file */
#define EXTRA_FIELD  0x04		/* bit 2 set: extra field present */
#define ORIG_NAME    0x08		/* bit 3 set: original file name present */
#define COMMENT      0x10		/* bit 4 set: file comment present */
#define ENCRYPTED    0x20		/* bit 5 set: file is encrypted */
#define RESERVED     0xC0		/* bit 6,7:   reserved */

#define get_byte() (inptr < insize ? inbuf[inptr++] : fill_inbuf())
		
/* Diagnostic functions */
#ifdef DEBUG
#  define Assert(cond,msg) {if(!(cond)) error(msg);}
#  define Trace(x) fprintf x
#  define Tracev(x) {if (verbose) fprintf x ;}
#  define Tracevv(x) {if (verbose>1) fprintf x ;}
#  define Tracec(c,x) {if (verbose && (c)) fprintf x ;}
#  define Tracecv(c,x) {if (verbose>1 && (c)) fprintf x ;}
#else
#  define Assert(cond,msg)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#endif

static int  fill_inbuf(void);
static void flush_window(void);
static void error(char *m);
static void gzip_mark(void **);
static void gzip_release(void **);
  
/*
 * This is set up by the setup-routine at boot-time
 */
static unsigned char *real_mode; /* Pointer to real-mode data */

#define EXT_MEM_K   (*(unsigned short *)(real_mode + 0x2))
#ifndef STANDARD_MEMORY_BIOS_CALL
#define ALT_MEM_K   (*(unsigned long *)(real_mode + 0x1e0))
#endif
#define SCREEN_INFO (*(struct screen_info *)(real_mode+0))

static long bytes_out = 0;
static uch *output_data = (char *) 0x80002000;
static unsigned long output_ptr = 0;

static void *malloc(int size);
static void free(void *where);
static void error(char *m);
static void gzip_mark(void **);
static void gzip_release(void **);
 
static void puts(const char *);
void print_hex(unsigned long data, const char *comment);


/* This extern is in build/*.lds */
extern int end;                 /* Need customize for your target */
extern char input_data[];       /* from *.lds */
extern int input_len;           /* from *.lds */

/* Work memory range  */
unsigned long free_mem_ptr = (unsigned long)&end;
unsigned long free_mem_end_ptr = (unsigned long)0x85000000;

#include "inflate.c"

static void *malloc(int size)
{
	void *p;

	if (size <0) error("Malloc error\n");
	if (free_mem_ptr <= 0) error("Memory error\n");

	free_mem_ptr = (free_mem_ptr + 7) & ~7;	/* Align */

	p = (void *)free_mem_ptr;
	free_mem_ptr += size;

	if (free_mem_ptr >= free_mem_end_ptr)
		error("\nOut of memory\n");

	return p;
}

static void free(void *where)
{	/* Don't care */
}

static void gzip_mark(void **ptr)
{
	*ptr = (void *) free_mem_ptr;
}

static void gzip_release(void **ptr)
{
	free_mem_ptr = (long) *ptr;
}
 
static inline void slow_down(void)
{      
	int k;

	for (k=0; k<100000; k++);
}

void puts(const char *s)
{
	while( *s != '\0' ){ 
		ar7100_reg_wr_nf((AR7100_UART_BASE + 0), *s);
		slow_down();
		s++;
	}
}

void print_hex(unsigned long data, const char *comment)
{
	int i;
	char buf[64];
	char *ptr = buf;
	unsigned long work;

	/* Insert comment first */
	while(*comment != '\0')
		*ptr++ = *comment++;

	*ptr++ = '>';

	for(i=7; i>=0; i--) {
		work = data & (0xf << (i*4));
		work = work >> (i*4);

		if(work >= 0 && work <= 9)
			*ptr++ = (char)(work + '0');
		else if(work >= 10 && work <= 15)
			*ptr++ = (char)(work + 'A' - 10);
		else
			puts("Error\n");
	}
	*ptr++ = '\n';
	*ptr = '\0';
	puts(buf);
}

void* memset(void* s, int c, size_t n)
{
	int i;
	char *ss = (char*)s;

	for (i=0;i<n;i++) ss[i] = c;
	return s;
}

void* memcpy(void* __dest, __const void* __src, size_t __n)
{
	int i;
	char *d = (char *)__dest, *s = (char *)__src;

	for (i=0;i<__n;i++) d[i] = s[i];
	return __dest;
}

int memcmp(__const void* __dest, __const void* __src, size_t __n)
{
    int i;
    char *d = (char *)__dest, *s = (char *)__src;

    for (i=0;i<__n;i++){
	if( d[i] > s[i] )
	  return 1;
	if( d[i] < s[i] )
	  return -1;
    }

    return 0;
}

/* ===========================================================================
 * Fill the input buffer. This is called only when the buffer is empty
 * and at least one byte is really needed.
 */
static int fill_inbuf(void)
{
	if (insize != 0) {
		error("ran out of input data\n");
	}

	inbuf = input_data;
	insize = input_len;
	inptr = 1;
	return inbuf[0];
}

/* ===========================================================================
 * Write the output window window[0..outcnt-1] and update crc and bytes_out.
 * (Used for the decompressed data only.)
 */
static void flush_window(void)
{
    ulg c = crc;         /* temporary variable */
    unsigned n;
    uch *in, *out, ch;
    
    in = window;
    out = &output_data[output_ptr]; 
    for (n = 0; n < outcnt; n++) {
	    ch = *out++ = *in++;
	    c = crc_32_tab[((int)c ^ ch) & 0xff] ^ (c >> 8);
    }
    crc = c;
    bytes_out += (ulg)outcnt;
    output_ptr += (ulg)outcnt;
    outcnt = 0;
}

static void error(char *x)
{
    puts("\n\n");
    puts(x);
    puts("\n\n -- System halted");
}

#define STACK_SIZE (4096)

long user_stack [STACK_SIZE];
long *stack_start = &(user_stack[STACK_SIZE - 1]);
static int a = 10;

int decompress_kernel( )
{
	makecrc();
	puts("Uncompressing Linux... ");
	gunzip();
	puts("Ok, booting the kernel.\n");
}

