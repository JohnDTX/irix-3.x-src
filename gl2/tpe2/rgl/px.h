/**************************************************************************
 *									  *
 * 		 Copyright (C) 1984, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/*
 * these are pmII values replacing the pmI config.h values
 */
#ifdef PM1
#define MBIOBASE	MBIO
#define PXDMASIZ	0x3800 	/* 1ff800 reserved */
#else
#define PXDMASIZ	0x4000 	/* 8 2k bytewides on PX */
#endif

struct pxdma {			/* 8237 DMA controller on PX */
	char	BADDR0;
	char	BWRDCT0;	/* PX card has correct addresses on mb */
	char	BADDR1;
	char	BWRDCT1;
	char	BADDR2;
	char	BWRDCT2;
	char	BADDR3;
	char	BWRDCT3;

	char	COMMAND;	/* read for status */
	char	REQUEST;
	char	SINGLMASK;
	char	MODE;
	char	CLEARBYTE;
	char	DMACLEAR;	/* read for temporary register */
	char	CLEARMASK;
	char	MASKWRITE;
};

#define PXDMAADDR ((struct pxdma *)(MBIO + 0x7e00))
/*   the PX uses DMA1 for input from the PCOX */
#define SETMASK1	0x05
#define CLRMASK1	0x01
#define MODE0		0x48	/* single,incr,read,0 */
#define MODE1		0x55	/* single,incr,autoinit,write,1 */
#define MODE2		0x4a	/* single,incr,read,2 */
#define MODE3		0x4b	/* single,incr,read,3 */

struct pxddevice {		/* PCOX controller on PX */
	char	PXD_SIGNAL;
	char	PXD_KREG;
};
#define PXIOADDR ((struct pxddevice *)(MBIO + 0x7eee))

struct pxdm {
	char	Dma;
};
#define DMAADDR ((struct pxdm *)(MBIO + 0xc000))

#define RUN_BIT		0x1	/* signal port run bit */
#define SIG1		0x2	/* stop rx from px if set, write config */
#define SIG2		0x4	/* px diag enabled */
#define SIG3		0x8	/* write to nano buffer enabled */

#define TERMINAL_ID	0xe4	/* model 2 = 24 x 80 */

