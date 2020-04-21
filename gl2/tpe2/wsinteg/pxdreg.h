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


#define DMA_ZAP		(u_char)0xef

#define ECOPYOUT	93
#define ERBUF		94
#define EZAP		95

/*
 * these are pmII values replacing the pmI config.h values
 */

struct pxdma {			/* 8237 DMA controller on PX */
	u_char	BADDR0;		/* 1 */
	u_char	BWRDCT0;	/* 0 PX card has correct addresses on mb */
	u_char	BADDR1;		/* 3 */
	u_char	BWRDCT1;	/* 2 */
	u_char	BADDR2;		/* 5 */
	u_char	BWRDCT2;	/* 4 */
	u_char	BADDR3;		/* 7 */
	u_char	BWRDCT3;	/* 6 */

	u_char	COMMAND; 	/* 9 read for status */
	u_char	REQUEST;	/* 8 */
	u_char	SINGLMASK;	/* b */
	u_char	MODE;		/* a */
	u_char	CLEARBYTE;	/* d */
	u_char	DMACLEAR;	/* c read for temporary register */
	u_char	CLEARMASK;	/* f */
	u_char	MASKWRITE;	/* e */
};

struct pxddevice {		/* PCOX controller on PX */
	u_char	PXD_SIGNAL;	/* 1 */
	u_char	PXD_KREG;	/* 0 */
};

/*   the PX uses DMA1 for input from the PCOX */
#define SETMASK1	0x05
#define CLRMASK1	0x01
#define MODE0		0x48	/* single,incr,read,0 */
#define MODE1		0x55	/* single,incr,autoinit,write,1 */
#define MODE2		0x4a	/* single,incr,read,2 */
#define MODE3		0x4b	/* single,incr,read,3 */

#define RUN_BIT		0x1	/* signal port run bit */
#define SIG1		0x2	/* stop rx from px if set, write config */
#define SIG2		0x4	/* px diag enabled */
#define SIG3		0x8	/* write to nano buffer enabled */

#define TERMINAL_ID	0xe4	/* model 2 = 24 x 80 */

typedef struct {		/* buffer */
    u_char	*rdp;		/* unload pointer */
    u_char	*wrp;		/* load pointer */
    u_char	*bufp;		/* pointer to buffer */
} px_buf_t;

#define PXINDSIZ	(25*80)
#define PXDMASIZ	0x4000 	/* 8 2k bytewides on PX */
#define COPYSIZ		4096	/* copyin, copyout granularity */

typedef int (*fptr_t)();
