/**************************************************************************
 *									  *
 * 		 Copyright (C) 1985, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/********************************************************************
*
*
* PXD driver for CXI 3278 interface card via Multibus
*
*********************************************************************
*********************************************************************
*
* Modification history
*
********************************************************************/

#include "pxd.h"
#if NPXD > 0

#include <fcntl.h>
#include "../h/setjmp.h"
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/errno.h"
#include "../h/buf.h"
#include "../h/signal.h"
#include "../h/user.h"
#include "../h/iobuf.h"
#include "../h/file.h"
#include "../h/proc.h"
#include "../h/config.h"
#include "machine/cpureg.h"
#include "../multibus/mbvar.h"
#include "../multibus/pxdreg.h"

#define DEV_OFFSET 0x00ee
#define PXDDMA_ADDR 0xc000
#define PXDMEM_ADDR 0xfc0000
#define min(a,b) ( (a) <= (b) ? (a) : (b) )
#define MBIOBASE MBIO_VBASE

extern	short	beprint;
extern	int	*nofault;
extern	struct	user	u;	/* User process */

int	pxdprobe(), pxdattach(), pxdintr(), pxdstart();
struct  mb_device *pxdinfo[NPXD];
struct 	mb_ctlr *pxcinfo[NPXD];
struct	mb_driver	pxddriver = {
	pxdprobe, pxdattach, (int (*)())0, pxdstart, pxdintr,
	(char *(*)())0, "px", pxdinfo, "pxd", pxcinfo,
};

pxdattach()
{
	return CONF_ALIVE;
}

pxdstart()
{
}

pxdintr()
{
}



u_char		Sbuf[4*PXDMASIZ + 0x100];	/* temp buffer of read data */
u_char		Wbuf[PXINDSIZ];	/* driver direct write 25x80 */

px_buf_t	pxk;		/* Dma buffer on PX card pointers */
px_buf_t	pxw;		/* Wbuf buffer in driver pointers */

long		curr_signal = 0;/* current signal value */
short		pxd_active = 0;	/* opened? */
char		pxd_alive;	/* did it probe? */
short		pxd_buffer_as_mem = 0;	/* 64kb SRAM = 1 */
long		pxd_dma_cnt = 0;
long		pxd_dma_size = PXDMASIZ;
char		pxd_initted = 0;/* have we told 3274 and hardware init? */
long		pxd_nano = 0;	/* current kb_nano value(non_zero only) */
long		pxd_oldr1, pxd_oldrr1;		/* from unsigned to long wpc */

struct pxdma	*pxd_ioaddr;
struct pxddevice *pxd_dev;
u_char		*pxd_dma;



/*
 * Test for PCOX card on the bus.
 */
pxdprobe(reg)
int reg;
{
	u_char c, *p;
	jmp_buf env;
	short saved_be;
	int *saved_jb;
	jmp_buf jb;

	/* Make sure that all 68k references are byte swapped */
	pxd_ioaddr = (struct pxdma *)(MBIOBASE + reg);
	pxd_dev = (struct pxddevice *)(MBIOBASE + reg + (int)DEV_OFFSET);
	pxd_dma = (u_char *)(MBIOBASE + PXDDMA_ADDR);
	c = pxd_dev->PXD_KREG;
	pxd_alive = 1;
#ifdef IP2
	spl7();
	saved_be = beprint;
	beprint = 0;
	saved_jb = nofault;
	if (setjmp(jb) == 0) {
		nofault = jb;
		p = &pxd_dev->PXD_SIGNAL;
		p -= 0x1;		/* 7eed to remap RAM */
		*p = 1;			/* remap RAM to memory */
		p = (u_char *)(MBUF_VBASE + PXDMEM_ADDR);
		c = *p;
		pxd_buffer_as_mem = 1;
	} else {
		pxd_buffer_as_mem = 0;
	}
	nofault = saved_jb;
	beprint = saved_be;
	spl0();
#endif /* IP2 */
	if (!pxd_buffer_as_mem) {
		p = &pxd_dev->PXD_SIGNAL;
		p -= 0x2;		/* 7eec to remap RAM */
		*p = 1;			/* remap RAM to I/O */
		c = *pxd_dma;		/* probe buffer as I/O */
		pxd_dma_size = PXDMASIZ;
		printf("3174/3274 16K board ");
	} else {
#ifdef IP2
		pxd_dma = (u_char *)(MBUF_VBASE + PXDMEM_ADDR);
#endif /* IP2 */
		pxd_dma_size = 4 * PXDMASIZ;
		printf("3174/3274 64K board ");
	}
	return CONF_ALIVE;
}


/*
** pxdopen is a "normal" character special device open except
** that it rejects subsequent opens. Open inits the DMA hardware.
**
*/
/* ARGSUSED */
pxdopen(dev, flag)
dev_t dev;
int flag;
{

	if (!pxd_alive) {
		u.u_error = ENXIO;		/* 6 no such device */
		return;
	}
	if (pxd_active) {
		u.u_error = EBUSY;		/* 16 already open */
		return;
	}
	if (flag == O_RDWR + 1) {
		pxd_active = 1;
		rset_sig(SIG1 | SIG2 | SIG3);	/* may have been on */
		pxdelay();
		if (!pxd_initted++) {
			init_dma();
			i_sig();
		}
	} else {
		printf("pxd:kernel minimal open\n");
		pxk.rdp = pxk.wrp = pxk.bufp = pxd_dma;
	}
}


/*
 * pxdclose disables PCOX input 
 */
/* ARGSUSED */
pxdclose(dev)
	dev_t dev;
{
	pxd_active = 0;
	rase1();				/* tell PCOX no input */
}


/*
 * Pxdread moves data from PX to Sbuf in kernel
 * because the hardware inverts A0, preventing word or long reads of PX.
 */
pxdread()
{
	register u_char c1;
	long tmp_pxd_cnt;		/* unsigned to long wpc */
	register unsigned short r1, r2, rr1, rr2;
	struct user *up;
	register u_char *rdp, *wrp;

	up = &u;
/*
 * term detects overrun from Dma on PX
 */
dmaread:
	c1 = (pxk.rdp != pxk.bufp) ? (u_char)*(pxk.rdp - 1) : (u_char)*(pxd_dma + pxd_dma_size - 1);
	if (c1 != DMA_ZAP) {
/*
 *   overran input so restart dma process
 */
		rase1();		/* tell px no rx to prevent overrun */
		up->u_error = EZAP;
		init_dma();
		rset_sig(SIG1);				/* reenable rx */
		pxdelay();
		return;
	}
once_more:
	spl7();
	rr2 = (u_char)pxd_ioaddr->BWRDCT1;	/* down from pxd_dma_size */
	rr1 = (u_char)pxd_ioaddr->BWRDCT1;
	r2 = (u_char)pxd_ioaddr->BADDR1;	/* up to pxd_dma_size */
	r1 = (u_char)pxd_ioaddr->BADDR1;
	spl0();
	r1 <<= 8;
	r1 |= r2;
	if (r1 == pxd_dma_cnt)
		return;			/* no new data */
	rr1 <<= 8;
	rr1 |= rr2;
	pxd_oldrr1 = rr1 + r1;		/* for reg integrity check */
	tmp_pxd_cnt = r1;		/* after last new byte */
	if (pxd_oldrr1 < pxd_dma_size - 3 || pxd_oldrr1 > pxd_dma_size + 1)
		goto once_more;
	pxk.rdp = pxd_dma + r1;		/* where next byte will go */
	rdp = pxd_dma + pxd_dma_cnt;	/* new byte is here */
	wrp = Sbuf;
	if (r1 > pxd_dma_cnt) {
		r1 = (u_short)((long)r1 - pxd_dma_cnt); /* sign bit? wpc */
		r2 = r1;
		do {
			*wrp++ = *rdp++;
		} while (--r2);
		*(--rdp) = DMA_ZAP;
docopy:
		wrp = Sbuf;
		pxd_oldr1 = pxd_dma_cnt;	/* for ioctl to check */
		while (r1) {
			r2 = min (r1, NBPG);
			if (up->u_error == 0) {
				if (copyout(wrp,up->u_base,r2)) {
					up->u_error = ECOPYOUT;	/* copyout failed */
				} else {
					up->u_base += r2;
					wrp += r2;
					up->u_count -= r2;
					r1 -= r2;
				}
			}
		}
	} else {
		r1 = pxd_dma_size - pxd_dma_cnt;
		r2 = r1;
		do {
			*wrp++ = *rdp++;
		} while (--r2);
		*(--rdp) = DMA_ZAP;
		r1 += tmp_pxd_cnt;
		if (tmp_pxd_cnt) {
			rdp = pxd_dma;
			r2 = tmp_pxd_cnt;
			do {
				*wrp++ = *rdp++;
			} while (--r2);
			*(--rdp) = DMA_ZAP;
		}
		goto docopy;
	}
	pxd_dma_cnt = tmp_pxd_cnt;
	if (up->u_error)
		init_dma();
}


/*
 * Pxdwrite moves data from user and direct writes to PCOX
 */
pxdwrite()
{
	register struct user *up;
	register unsigned n;

	up = &u;
	pxw.rdp = Wbuf;
	/*
	 * test write count for validity
	 */
	if ((n = up->u_count) == 0)
		return;				/* just return on count of 0 */
	if (n >= PXINDSIZ) {
		up->u_error = EINVAL;		/* 22 error in value */
		return;
		}
/* copyin call here to load Wbuf from user */
	if (copyin(up->u_base,Wbuf, n)) {
		up->u_error = EFAULT;		/* 14 bad address */
		return;
		}
	up->u_base += n;
	up->u_count = 0;
/* set SIG3 and send to PCOX */
	set_sig(SIG3);				/* direct write signal */
	while (n--) {
		if (*pxw.rdp)
			pxd_nano = *pxw.rdp;
		pxd_dev->PXD_KREG = *pxw.rdp++;
		dwdelay();
		}
	rset_sig(SIG3);				/* direct write off again */
}


/*
 * pxdioctl does writes to signal and kbdata ports of px
 * the kb and signal routines are longs but use only a char
 * op	subroutine
 * 0	kb_nan		output char to KREG
 * 1	set_sig		set bits, output to SIGNAL 
 * 2	rset_sig	reset bits, output to SIGNAL
 * 3	forc_sig	output to SIGNAL
 * 4	get_cur		read SIGNAL, KREG data
 * 5	i_sig		init SIGNAL, KREG
 * 6	read_avail	read data in Dma
 * 7	fetch_byte	read char
 * 8	store_byte	store char
 * 11	rase1		halt PCOX DMA by raising SIG1
 * 13	force_open_init	pxdopen restart hardware 
 * 14	get_open_initted return initted status
 * 15	test_dma_mem	pxd static memory test	
 * 16	fetch_oldr1	prev dma addr
 */
/*ARGSUSED*/
pxdioctl(dev, cmd, addr, flag)
dev_t dev;
long addr;
{
	register struct user *up;
	u_char dma_addr;
	register u_char c, *p;
	register unsigned r1;
	short saved_be;
	int *saved_jb;
	jmp_buf jb;

	up = &u;
	switch(cmd) {
	
	case	0:
		pxd_dev->PXD_KREG = addr;
		pxd_nano = addr;
		break;
	case	1:
		pxd_dev->PXD_SIGNAL = curr_signal |= addr;
		break;
	case	2:
		pxd_dev->PXD_SIGNAL = curr_signal &= ~addr;
		break;
	case	3:
		pxd_dev->PXD_SIGNAL = curr_signal = addr;
		break;
	case	4:
		dma_addr = (u_char)curr_signal;
		if (subyte((caddr_t)addr , dma_addr))	/* to user */
			up->u_error = EFAULT;	/* 14 bad address */
		dma_addr = (u_char)pxd_nano;
		if (subyte((caddr_t)addr+1 , dma_addr))	/* to user */
			up->u_error = EFAULT;	/* 14 bad address */
		break;
	case	5:
		i_sig();
		break;
	case	6:
		spl7();
		c = pxd_ioaddr->BADDR1;
		r1 = pxd_ioaddr->BADDR1;
		spl0();
		r1 <<= 8;
		r1 |= c;
		r1 &= 0xffff;
		c =  (r1 == pxd_dma_cnt) ? 0 : 1;
		if (subyte((caddr_t)addr,c))
			up->u_error = EFAULT;	/* 14 bad address */
		break;
	case	7:
		if ((p = (u_char *)fuword((caddr_t)addr))==(u_char *)-1)
			up->u_error = EFAULT;	/* 14 bad address */
		spl7();
		saved_be = beprint;
		beprint = 0;
		saved_jb = nofault;
		if (setjmp(jb) == 0) {
			nofault = jb;
			c = *p;
		} else
			u.u_error = ENXIO;	/* 6 no such device */
		nofault = saved_jb;
		beprint = saved_be;
		spl0();
		if (subyte((caddr_t)addr+4,c))
			up->u_error = EFAULT;	/* 14 bad address */
		break;
	case	8:
		if ((p = (u_char *)fuword((caddr_t)addr))==(u_char *)-1)
			up->u_error = EFAULT;	/* 14 bad address */
		if ((long)(c = fubyte((caddr_t)addr+4))==-1L)
			up->u_error = EFAULT;	/* 14 bad address */
		spl7();
		saved_be = beprint;
		beprint = 0;
		saved_jb = nofault;
		if (setjmp(jb) == 0) {
			nofault = jb;
			*p = c;
		} else
			u.u_error = ENXIO;	/* 6 no such device */
		nofault = saved_jb;
		beprint = saved_be;
		spl0();
		break;
	case	11:
		rase1();
		break;
	case	13:
		pxd_initted = 0;
		break;
	case	14:
		if (subyte((caddr_t)addr,pxd_buffer_as_mem))
			up->u_error = EFAULT;	/* 14 bad address */
		if (subyte((caddr_t)addr+4,pxd_initted))
			up->u_error = EFAULT;	/* 14 bad address */
		break;
	case	15:
		c = test_dma_mem();
		if (subyte((caddr_t)addr+4,c))
			up->u_error = EFAULT;	/* 14 bad address */
		break;
	case	16:
		r1 = pxd_oldr1;
		for (c = 4; c--; ) {
			dma_addr = (u_char)r1;
			r1 >>= 8;
			if (subyte((caddr_t)(addr+c),dma_addr)) {
				up->u_error = EFAULT;
				break;
			}
		}
		break;
	default:
		up->u_error = ENOTTY;		/* 25 not a typewriter ?? */
	}
}


/*
 * delay 5 usec for px direct write
 */
dwdelay()
{
	register unsigned t;

#ifdef juniper
	for(t=13; t--;)
#else
	for(t=3; t--;)
#endif juniper
		;
}




/*
 * init_dma loads dma buffer with dma_zaps and restarts pointers
 */
init_dma()
{
	register zero = 0;

	pxd_ioaddr->DMACLEAR = zero;		/* MASK OFF ALL */
	pxd_ioaddr->BADDR1 = zero;
	pxd_ioaddr->BADDR1 = zero;		/* base of dma is zero to 8237*/
	pxd_ioaddr->BWRDCT1 = 0xff;	   	/* lo of (count-1) */
	pxd_ioaddr->BWRDCT1 = (((pxd_dma_size-1) >> 8) & 0xff); /*16kb/64kb */
	pxd_ioaddr->MODE = MODE0;		/* channel 0 mode */
	pxd_ioaddr->MODE = MODE1;		/* channel 1 mode */
	pxd_ioaddr->MODE = MODE2;		/* channel 2 mode */
	pxd_ioaddr->MODE = MODE3;		/* channel 3 mode */
	init_rx();
	pxd_ioaddr->SINGLMASK = CLRMASK1;	/* enable DMA1 */
}


/*
 * init_rx restarts rx pointers
 */
init_rx()
{
	register unsigned i;
	register u_char *wrp;

	pxd_ioaddr->SINGLMASK = SETMASK1;	/* disable DMA1 */
	wrp = pxk.rdp = pxk.wrp = pxk.bufp = pxd_dma;
	wrp += pxd_dma_size;
	*--wrp = DMA_ZAP;			/* a good end */
	pxd_dma_cnt = 0;			/* init this value  wpc */
	pxd_ioaddr->SINGLMASK = CLRMASK1;	/* enable DMA0 */
	
}


/*
 * i_sig initializes the signal port of the px card 
 * and also does init_shift from
 * px11src restart, emulator_10 code
 */
i_sig()
{
	set_sig(curr_signal = RUN_BIT | SIG1);
	kb_nan(TERMINAL_ID);			/* terminal_id for 24x80 */
	pxdelay();				/* wait 100 msec for px init */

	rset_sig(SIG1);				/* wait for buffer write */
	pxdelay();

	kb_nan(0xcd);				/* shift-break of init_shift */
	kbdelay();
	kb_nan(0xcf);				/* alt-break of init_shift */
	kbdelay();
	rase1();
}


/*
 * kb_nan writes data to px card
 */
/*ARGSUSED*/
kb_nan(ldata)
long ldata;
{
	pxd_dev->PXD_KREG = ldata;
	pxd_nano = ldata;
}


/*
 * delay 15 msec for px make or break   
 */
kbdelay()
{
	register long u;

#ifdef juniper
	u = 15 * 1300;
#else
	u = 15 * 277;
#endif juniper
	for(; u; u--)
		;
}


mv_fast(from, to, count)
register u_char *from, *to;
register unsigned count;
{
/*	asm("jra .L999");
	asm(".L20998:"); 
	asm("movb a5@+,a4@+");
	asm(".L999:");
	asm("dbf  d7,.L20998");
	asm("movb	#0xef,a5@");*/
	do {
		*to++ = *from++;
	} while (--count);
}

/*
 * delay 100 msec for px init of self   
 */
pxdelay()
{
	register unsigned u;

#ifdef juniper
	u = 100 * 1300;
#else
	u = 100 * 277;
#endif juniper
	for(; u; u--)
		;
}


/*
 * rase1 sets SIG1 to stop dma from PCOX 
 */
rase1()
{
	set_sig(RUN_BIT | SIG1);		/* force run on   wpc*/
	kb_nan(TERMINAL_ID);			/* terminal_id for 24x80 */
	pxdelay();				/* wait 100 msec for px init */
}


/*
 * rset_sig removes bits from the current signal port
 */
/*ARGSUSED*/
rset_sig(lsignal)
long lsignal;
{
	pxd_dev->PXD_SIGNAL = curr_signal &= ~lsignal;
}


/*
 * set_sig adds bits to the current signal
 */
/*ARGSUSED*/
set_sig(lsignal)
long lsignal;
{
	pxd_dev->PXD_SIGNAL = curr_signal |= lsignal;
}


/*
 * test_dma_mem loads dma buffer with test data
 */
test_dma_mem()
{
	register unsigned i;
	register u_char *rdp, *wrp;
	register u_char address;
	char results = 8;

	pxd_ioaddr->SINGLMASK = SETMASK1;	/* disable DMA1 */
	rdp = wrp = pxk.bufp = pxd_dma;
	for (i = 0; i < pxd_dma_size; i++)
		*wrp++ = (u_char)0xff;
	for (i = 0; i < pxd_dma_size; i++)
		if (*rdp++ != (u_char)0xff) {
			printf("wrote ff read %02x at %x\n",
				(*(rdp-1)),rdp-1);
			results = 1;
			break;
		}
	for (wrp = pxk.bufp, i = 0; i < pxd_dma_size; i++)
		*wrp++ = (u_char)0xaa;
	for (rdp = pxk.bufp, i = 0; i < pxd_dma_size; i++)
		if (*rdp++ != (u_char)0xaa) {
			printf("wrote aa read %02x at %x\n",
				(*(rdp-1)),rdp-1);
			results = 1;
			break;
		}
	for (wrp = pxk.bufp, i = 0; i < pxd_dma_size; i++) {
		address = (u_char)(wrp && 0xff);
		*wrp++ = address;
	}
	for (rdp = pxk.bufp, i = 0; i < pxd_dma_size; i++) {
		address = (u_char)(rdp && 0xff);
		if (*rdp++ != address) {
			printf("wrote %02x read %02x at %x\n",
				address, (*(rdp-1)),rdp-1);
			results = 1;
			break;
		}
	}
	for (wrp = pxk.bufp, i = 0; i < pxd_dma_size; i++) {
		address = (u_char)!(wrp && 0xff);
		*wrp++ = address;
	}
	for (rdp = pxk.bufp, i = 0; i < pxd_dma_size; i++) {
		address = (u_char)!(rdp && 0xff);
		if (*rdp++ != address) {
			printf("wrote %02x read %02x at %x\n",
				address, (*(rdp-1)),rdp-1);
			results = 1;
			break;
		}
	}
	for (wrp = pxk.bufp, i = 0; i < pxd_dma_size; i++)
		*wrp++ = 0x55;
	for (rdp = pxk.bufp, i = 0; i < pxd_dma_size; i++)
		if (*rdp++ != 0x55) {
			printf("wrote 55 read %02x at %x\n",
				(*(rdp-1)),rdp-1);
			results = 1;
			break;
		}

	for (wrp = pxk.bufp, i = 0; i < pxd_dma_size; i++)
		*wrp++ = DMA_ZAP;
	pxd_ioaddr->SINGLMASK = CLRMASK1;	/* enable DMA0 */
	if (results != 8)
		results = i % (pxd_dma_size / 8);
	return (results);
	
}
#endif
