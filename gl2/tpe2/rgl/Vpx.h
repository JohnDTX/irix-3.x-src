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
 * Vkernel IBM 3270 bus manifests, structs, etc.
 */

#ifndef VIBM3270
#define VIBM3270

#include "Vio.h"
#include "Vdevtypes.h"


/* 
** IBM3270 Modify Query Message Format.
 */

struct PXDMQRequest {
	SystemCode	requestcode;
	InstanceId 	fileid;
	unsigned 	mqreqcode;
	unsigned	arg;
	unsigned char	junk[18];
};

typedef struct {		/* buffer */
	caddr_t		rdp;	/* unload pointer */
	caddr_t		wrp;	/* load pointer */
	caddr_t		bufp;	/* pointer to buffer */
} px_buf_t;

typedef struct {		/* px driver status (all pointers) */
	caddr_t		wrp;	/* load pointer */
	caddr_t		bufp;	/* pointer to buffer */
	unsigned	ft;	/* flag for in ft */
	unsigned	host_len; /* working length */
	unsigned	length;	/* host length request */
	caddr_t		headrp;	/* file header store addr */
	unsigned	max_len; /* max received length allowable */
} px_status;

typedef int (*fptr_t)();

#endif VIBM3270

#define true		1
#define false		0
#define YES		1
#define NO		0
#define DMA_ZAP		0xef
#define END_WRITE	0xe6
#define min(a,b) ( (a) <= (b) ? (a) : (b) )

/*  direct write mode settings (indicates last command sent to nano)
*/
#define DIR_MODE_SET	1	/* set cursor address */
#define DIR_MODE_WD	2	/* writing data to buffer */
#define DIR_MODE_WA	3	/* writing attribute to buffer */
#define EACK		90	/* part of outb received */
#define EDONE		91	/* all of an outb file received */
#define ELENGTHBAD	92	/* host length negative or in error */
#define ECOPYOUT	93	/* copyout returned an error */
#define ERBUF		94	/* overrun in kernal rx copyout buf */
#define ERZAP		95	/* reread failure or outb length not found */
#define E74LEN		96	/* length ended by 3274 (not at home) */
#define EZAP		97	/* overrun on PXD hardware, kernel discovered */
#define ESTART		98	/* pxdread detected start of message */
#define ALT_MAKE	0x4f
#define ALT_BREAK	0xcf
#define SHIFT_BREAK	0xcd
#define CLEAR		0x51
#define PA1		0x5f
#define PA2		0x5e

#define PXINDSIZ	(25*80)
#define PXBUFSIZ	0x1000
#define COPYSIZ		2	/* copyin, copyout granularity */
#define ZAP_COUNT	500000	/* 1 sec wait for data from dma */
