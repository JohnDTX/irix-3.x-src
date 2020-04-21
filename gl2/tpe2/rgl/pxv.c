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
* PXV Vkernel driver for CXI 3278 interface card via Multibus
*
*********************************************************************/

#include <types.h>
#include <process.h>
#include <memory.h>
#include <dm.h>
#include <errno.h>
#include <cpureg.h>
#include <px.h>
/*#include <comarea.h>*/
#include <Vioprotocl.h>
#include <Vpx.h>
#undef DEBUG

#define CONF_ALIVE	3
#define CONF_FAULTED	2


#define CMD_CURHI 0xe2
#define CMD_CURLO 0xe3
#define CMD_WRITE 0xc3
#define DATA1RXFER 0x1b		/* cent sign */
#define DATA2RXFER 0x1a		/* dollar sign */
#define END_WRITE 0xe6


/*	Exports:				*/
extern	SystemCode	Init_px();
extern	SystemCode	PxdCreate();
extern	SystemCode	PxdRead();
extern	SystemCode	PxdWrite();
extern	SystemCode	PxdModify();
extern	SystemCode	PxdQuery();
extern	SystemCode	PxdRelease();

/*	Imports:				*/
extern	Process	*Map_pid();
extern	Process	*Active;
extern	printf();

/*	Globals					*/

DeviceInstance	*PXDinstance = NULL;	/* device instance for driver */

u_char		Sbuf[PXDMASIZ];	/* driver dma screen buffer */
u_char		Wbuf[PXINDSIZ];	/* driver direct write 25x80 */

px_buf_t	pxk;		/* Dma buffer on PX card pointers */
px_buf_t	pxw;		/* Wbuf buffer in driver pointers */

short		pxd_exist = 0;	/* card in cage?? */
short		pxd_active = 0;	/* opened? */
long		pxd_nano;	/* current kb_nano value(non_zero only) */
long		bltlength;	/* last blt length */
ucaddr_t	lrdp;		/* last rdp error address */
long		curr_signal;	/* current signal value */
long		u_error = 0;	/* temp of return status */
char		lavail;		/* last read_avail status */
char		pxd_initted = 0;/* marks original hardware init */



/*
 *   Powerup-initialization and probe.
 * Test for PCOX card on the bus.
 * Record result in pxd_exist.
 */
SystemCode
Init_Px()
{
	int x;

	/*
	 * See if interface is present.
	 */
	pxd_exist = 0;
	if(!(x = probebyte(PXDMAADDR->BADDR1))) /* is DMA ctlr there? */
		return;	/*** DEBUG ***/
	pxd_exist = 1;
#ifdef PM1
	pxd_active = 0;			/* not open */
	return 1;
#endif PM1
}


/*
 * pxdopen is a "normal" character special device open except
 * that it rejects subsequent opens. Open inits the DMA hardware.
 *
 */
SystemCode
PxdCreate(req,desc)
CreateInstanceRequest *req;
DeviceInstance *desc;
{

#ifdef PM1
	if (!Init_Px()) {
		return NOT_FOUND;
	}
	if (req->filemode != FCREATE) {
		return MODE_NOT_SUPPORTED;
	}
#else
	pxd_exist = 1;
#endif PM1
	pxd_active = 1;
	rset_sig(SIG1 | SIG2 | SIG3);	/* may have been on */
	pxdelay();
	if (!pxd_initted++) {
		init_dma();
		i_sig();
	}
	init_buffer();
/*
 *** Initialize the device instance descriptor, can assume all fields
 *** are zero except for owner and id.
 */
	desc->readfunc = PxdRead;
	desc->writefunc = PxdWrite;
	desc->queryfunc = PxdQuery;
	desc->modifyfunc = PxdModify;
	desc->releasefunc = PxdRelease;
	desc->reader = 0;
	desc->writer = 0;
	PXDinstance = desc; /* record the instance */
	desc->type = (READABLE+WRITEABLE+VARIABLE_BLOCK); /*???*/
	desc->blocksize = PXDMASIZ; /* WHAT */
	Active->state = READY;
	return(OK);

}


/*
 * pxdclose turns off PCOX input with SIG1
 */
SystemCode
PxdRelease(req,desc)
IoRequest *req;
DeviceInstance *desc;
{
	pxd_active = 0;
	if(!pxd_exist)
		return CONF_FAULTED;
	return OK;
}



/*
 * Pxdread moves data from DMAADDR->Dma on PX to Sbuf in kernel
 */
SystemCode
PxdRead(req,desc)
IoRequest *req;
DeviceInstance *desc;
{
	register u_char c1,c2;
	register long r1;
	register ucaddr_t dmaend, kbufp, rdp;
	register u_char *p1, *wrp;
	Process *pd;

	desc->reader = Active->pid;
	dmaend = &DMAADDR->Dma + PXDMASIZ;
	r1 = min (req->bytecount,PXDMASIZ);
	kbufp = pxk.bufp;
	p1 = Sbuf + r1;			/* read limit */
	rdp = pxk.rdp;
	wrp = Sbuf;
/*
 * term reads character from Dma on PX
 */
dmaread:
	c2 = *rdp;
	if (c2 == (u_char)DMA_ZAP)
		goto termnow;		/* done??? */
/*
 * read previous byte to detect hardware overrun of rdp
 */
	c1 = (rdp != kbufp) ? (u_char)*(rdp - 1) : (u_char)DMA_ZAP;
	*rdp = (u_char)DMA_ZAP;			/* for overrun detection at next byte */
	if (++rdp >= dmaend) {
		if (rdp > dmaend)
			printf("RDP WAY too big ");
		rdp = kbufp;		/* circular buffer */
	}
/*
 * move a byte to the kernel's screen input buffer
 */
	*wrp++ = c2;
	if ((wrp < p1) && (c1 == (u_char)DMA_ZAP))
		goto dmaread;
	if (wrp == p1)
		goto termout;		/* read our requested count */
/*
 *   hardware overran rdp so restart dma process
 */
	rase1();			/* tell px no rx to prevent overrun */
	if (c1 != (u_char)DMA_ZAP) {
		lrdp = rdp;
		u_error = EZAP;
	} else {
		lrdp = rdp;
		u_error = ERBUF;
	}
	
	init_dma();
	rset_sig(SIG1);			/* reenable rx */
	desc->reader = NULL;
	req->shortbuffer[0] = u_error;
	req->bytecount = 0;
	return(OK);
termnow:
	c2 = *rdp;
	if (c2 != (u_char)DMA_ZAP)
		goto dmaread;
/*
 * test for data to move to dma_buf in user space.
 */
termout:
	pxk.rdp = rdp;
	r1 = wrp - Sbuf;
	if (r1) {
		blt(req->bufferptr,Sbuf,r1);
	}
	req->shortbuffer[0] = 0;
	req->bytecount = r1;
	bltlength = r1;
	desc->reader = NULL;
	return(OK);
}

/*
 * Pxdwrite moves data from user and direct writes to PCOX
 */
SystemCode
PxdWrite(req,desc)
IoRequest *req;
DeviceInstance *desc;
{
	register unsigned n;
	caddr_t u_base;
	register Process *pd;

	req->shortbuffer[0] = OK;
	/* check if write request is ok */
	desc->writer = Active->pid;
	pxw.rdp = pxw.bufp;
	/*
	 * test write count for validity
	 */
	if ((n = req->bytecount) == 0) {
		req->shortbuffer[0] = ENXIO;	/* 6 error in value */
		return(OK);
	}
	if (n >= PXINDSIZ) {
		req->shortbuffer[0] = EINVAL;	/* 22 error in value */
		return(OK);
	}
/* copyin call here to load Wbuf from user */
	blt(pxw.bufp,req->bufferptr, n);
	req->bufferptr += n;
/* set SIG3 and send to PCOX */
	set_sig(SIG3);			/* direct write signal */
	while (n--) {
		if (*pxw.rdp)
			pxd_nano = *pxw.rdp;
		PXIOADDR->PXD_KREG = *pxw.rdp++;
		dwdelay();
		}
	rset_sig(SIG3);			 /* direct write off again */
	desc->writer = NULL;
	return(OK);
}


/*
 * pxdioctl does writes to signal and kbdata ports of px
 * the kb and signal routines are longs but use only a char
 * op	subroutine
 * 6	read_avail	read data in Dma
 * 7	fetch_byte	read 1 char from kernel memory
 * 14	read_open_initted read pxd_initted
 * 15	test_dma_memory write/read static rams
 */
SystemCode
PxdQuery(req,desc)
struct PXDMQRequest *req;
DeviceInstance *desc;
{
	long cmd;
	register u_char c, *p;

	u_error = OK;
	desc->reader = Active->pid;
	cmd = req->mqreqcode;
	switch(cmd) {
	
	case	6:
		c = *pxk.rdp;
		req->junk[4] = c;
		if (c != (u_char)DMA_ZAP) {
			c = 1;
		} else {
			c = 0;
		}
		req->junk[0] = c;
		lavail = c;
		break;
	case	7:
		p = (u_char *)req->arg;
		req->junk[0] = *p;
		break;
	case	14:
		req->junk[0] = pxd_initted;
		break;
	case	15:
		c = test_dma_mem();
		req->junk[0] = c;
		break;
	default:
		u_error = ENOTTY;	/* 25 errno.h not a typewriter ?? */
	}
	desc->reader = NULL;
	return(u_error);
}


/*
 * pxdioctl does writes to signal and kbdata ports of px
 * the kb and signal routines are longs but use only a char
 * op	subroutine
 * 0	kb_nan		output char to KREG
 * 1	set_sig		set bits, output to SIGNAL 
 * 2	rset_sig	reset bits, output to SIGNAL
 * 3	forc_sig	output to SIGNAL
 * 5	i_sig		init SIG and KREG ports
 * 8	store_byte	store char
 * 11	rase1		halt PCOX DMA by raising SIG1
 * 13	force_open_init clears pxd_initted
 */
SystemCode
PxdModify(req,desc)
struct PXDMQRequest *req;
DeviceInstance *desc;
{
	long addr,cmd;
	register u_char c, *p;
	register int i;

	u_error = OK;
	desc->writer = Active->pid;
	cmd = req->mqreqcode;
	addr = req->arg;
	switch(cmd) {
	
	case	0:
		PXIOADDR->PXD_KREG = addr;
		pxd_nano = addr;
		break;
	case	1:
		PXIOADDR->PXD_SIGNAL = curr_signal |= addr;
		break;
	case	2:
		PXIOADDR->PXD_SIGNAL = curr_signal &= ~addr;
		break;
	case	3:
		PXIOADDR->PXD_SIGNAL = curr_signal = addr;
		break;
	case	5:
		i_sig();
		break;
	case	8:
		p = (u_char *)req->arg;
		c = req->junk[0];
		*p = c;
		break;
	case	11:
		rase1();
		break;
	case	13:
		pxd_initted = 0;
		break;
	default:
		u_error = ENOTTY;	/* 25 errno.h not a typewriter ?? */
	}
	desc->writer = NULL;
	return(u_error);
}

/*
 * delay 5 usec for px direct write
 */
dwdelay()
{
	register unsigned t;

	for(t=2; t--;)
		;
}

/*
 *   init_buffer loads write pointer block at open time
 */
init_buffer()
{

	pxw.rdp = pxw.wrp = pxw.bufp = Wbuf;
}

/*
 * init_dma loads dma buffer with dma_zaps and restarts pointers
 */
init_dma()
{
	register u_char zero = 0;

	PXDMAADDR->DMACLEAR = zero;	/* MASK OFF ALL */
	PXDMAADDR->BADDR1 = zero;
	PXDMAADDR->BADDR1 = zero;	/* base of dma is zero to 8237 */
	PXDMAADDR->BWRDCT1 = (PXDMASIZ-1) & 0xff;	   /* lo of (count-1) */
	PXDMAADDR->BWRDCT1 = (((PXDMASIZ-1) >> 8) & 0xff); /* 16kb dma in use */
	PXDMAADDR->MODE = MODE0;	/* channel 0 mode */
	PXDMAADDR->MODE = MODE1;	/* channel 1 mode */
	PXDMAADDR->MODE = MODE2;	/* channel 2 mode */
	PXDMAADDR->MODE = MODE3;	/* channel 3 mode */
	init_rx();
	PXDMAADDR->SINGLMASK = CLRMASK1;	/* enable DMA1 */
}

/*
 * init_rx loads dma buffer with dma_zaps and restarts rx pointers
 */
init_rx()
{
	register unsigned i;
	register ucaddr_t wrp;

	PXDMAADDR->SINGLMASK = SETMASK1;	/* disable DMA1 */
	wrp = pxk.rdp = pxk.wrp = pxk.bufp = &DMAADDR->Dma;
	for (i = 0; i < PXDMASIZ; i++)
		*wrp++ = (u_char)DMA_ZAP;
	PXDMAADDR->SINGLMASK = CLRMASK1;	/* enable DMA1 */
	
}

/*
 * i_sig initializes the signal port of the px card 
 * and also does init_shift from
 * px11src restart, emulator_10 code
 */
i_sig()
{
	set_sig(curr_signal = RUN_BIT | SIG1);
	kb_nan(TERMINAL_ID);		/* terminal_id for 24x80 */
	pxdelay();			/* wait 100 msec for px init */

	rset_sig(SIG1);			/* wait for buffer write */
	pxdelay();

	kb_nan(0xcd);			/* shift-break of init_shift */
	kbdelay();
	kb_nan(0xcf);			/* alt-break of init_shift */
	kbdelay();
}

/*
 * kb_nan writes data to px card
 */
/*ARGSUSED*/
kb_nan(ldata)
long ldata;
{
	PXIOADDR->PXD_KREG = (u_char)ldata;
	pxd_nano = ldata;
}

/*
 * delay 15 msec for px make or break   
 */
kbdelay()
{
	register unsigned t;

	for(t=7500; t; t--)
		;
}

/*
 * delay 100 msec for px init of self   
 */
pxdelay()
{
	register unsigned t;

	for(t=50000; t; t--)
		;
}

/*
 * rase1 sets SIG1 to stop dma from PCOX 
 */
rase1()
{
	set_sig(SIG1);
	kb_nan(TERMINAL_ID);
}


/*
 * rset_sig removes bits from the current signal port
 */
/*ARGSUSED*/
rset_sig(lsignal)
long lsignal;
{
	curr_signal &= ~lsignal;
	PXIOADDR->PXD_SIGNAL = (u_char)curr_signal;
}

/*
 * set_sig adds bits to the current signal
 */
/*ARGSUSED*/
set_sig(lsignal)
long lsignal;
{
	curr_signal |= lsignal;
	PXIOADDR->PXD_SIGNAL = (u_char)curr_signal;
}


/*
 * test_dma_mem loads dma buffer with test data
 */
test_dma_mem()
{
	register unsigned i;
	register ucaddr_t rdp, wrp;
	register u_char address;
	char results = 8;

	PXDMAADDR->SINGLMASK = SETMASK1;	/* disable DMA1 */
	wrp = pxk.rdp = pxk.wrp = pxk.bufp = &DMAADDR->Dma;
	rdp = wrp = pxk.bufp = &DMAADDR->Dma;
	for (i = 0; i < PXDMASIZ; i++)
		*wrp++ = (u_char)0xff;
	for (i = 0; i < PXDMASIZ; i++)
		if (*rdp++ != (u_char)0xff) {
			printf("wrote ff read %02x at %x\n",
				(*(rdp-1)),(u_char)(rdp-pxk.bufp)-1);
			results = 1;
			break;
		}
	for (wrp = pxk.bufp, i = 0; i < PXDMASIZ; i++)
		*wrp++ = (u_char)0xaa;
	for (rdp = pxk.bufp, i = 0; i < PXDMASIZ; i++)
		if (*rdp++ != (u_char)0xaa) {
			printf("wrote aa read %02x at %x\n",
				(*(rdp-1)),(u_char)(rdp-pxk.bufp)-1);
			results = 1;
			break;
		}
	for (wrp = pxk.bufp, i = 0; i < PXDMASIZ; i++) {
		address = (u_char)(wrp && 0xff);
		*wrp++ = address;
	}
	for (rdp = pxk.bufp, i = 0; i < PXDMASIZ; i++) {
		address = (u_char)(rdp && 0xff);
		if (*rdp++ != address) {
			printf("wrote %02x read %02x at %x\n",
				address, (*(rdp-1)),(u_char)(rdp-pxk.bufp)-1);
			results = 1;
			break;
		}
	}
	for (wrp = pxk.bufp, i = 0; i < PXDMASIZ; i++) {
		address = (u_char)!(wrp && 0xff);
		*wrp++ = address;
	}
	for (rdp = pxk.bufp, i = 0; i < PXDMASIZ; i++) {
		address = (u_char)!(rdp && 0xff);
		if (*rdp++ != address) {
			printf("wrote %02x read %02x at %x\n",
				address, (*(rdp-1)),(u_char)(rdp-pxk.bufp)-1);
			results = 1;
			break;
		}
	}
	for (wrp = pxk.bufp, i = 0; i < PXDMASIZ; i++)
		*wrp++ = 0x55;
	for (rdp = pxk.bufp, i = 0; i < PXDMASIZ; i++)
		if (*rdp++ != 0x55) {
			printf("wrote 55 read %02x at %x\n",
				(*(rdp-1)),(rdp-pxk.bufp)-1);
			results = 1;
			break;
		}

	for (wrp = pxk.bufp, i = 0; i < PXDMASIZ; i++)
		*wrp++ = (u_char)DMA_ZAP;
	PXDMAADDR->SINGLMASK = CLRMASK1;	/* enable DMA1 */
	if (results != 8)
		results = i % (PXDMASIZ / 8);
	return (results);
	
}

