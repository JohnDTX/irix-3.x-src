 /*************************************************************************
 *									  *
 * 		 Copyright (C) 1985, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************

	********* INTERNAL USE ONLY ***********
	***** REQUIRES NON_DISCLOSURE AGREEMENT ********
	***** DO NOT SHOW TO ANYONE WITHOUT A NON_DISCLOSURE AGREEMENT ****

/************************* P X D I O . C **********************************
*
*  MODULE DESCRIPTION:
*	I/O interface routines to the PCOX card
*
*  ENTRY POINTS:
*	get_screen()	- Force PCOX card to give new screen
*	outbread()	- Read an outbound message
*	pdelay()	- Provide a loop pdelay of n milliseconds
*	pxdclose()	- Close the pxd device
*	pxdioctl()	- process outb ioctls
*	pxdopen()	- Open the pxd device
*	pxdread()	- Read pxd Screen data
*	pxstat()	- Return TRUE if new data available from the 3274
*	send_pxd()	- Send a character to the 3274
*	send_dly()	- Send a character to the 3274 and delay 15 msecs
*
************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <gl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/times.h>
#include "term.h"
#include "pxw.h"

#define CMD_CURHI	(u_char)0xe2
#define CMD_CURLO	(u_char)0xe3
#define CMD_WRITE	(u_char)0xc3
#define ESPEC		(u_char)0xe0
#define SEVENF		0x7fffffff

#define LOGREAD		{ if (logflag) { logread(Countr);} }
#define LOGWRITE	{ if (logflag) { logwrite(c);} }

/*
**	Externals
*/
extern int		dflag[];
extern int		errno;
extern u_short		f13_diag();
extern long		get3270();
extern char		*inputfile;
extern px_bufs 		pxl;
extern int		replay[];
extern char		*sys_errlist[];
extern long		times();
extern u_char		Blink_msgs;
extern u_char		Display;
extern u_char		Display_xlat[];
extern dma_opr		DMa;
extern getput		Fst;
extern long		MaxRU;
extern u_char		Msg_proc;
extern u_char		Msgtype;
extern u_char		Manuflag;
extern u_char		Screen[];
extern u_char		Status_flags;


/*
**	Globals
*/
int			fd;			/* pxd device file descriptor */
px_status		outb;			/* outbound ft pointers */
int			tfd;			/* pxd write file descriptor */
struct tms		*Btm;
u_char			Buffer_is_mem = FALSE;
long			Countr;			/* pxdread actual read count */
u_char			F3174 = FALSE;		/* default is 3274 */
u_char			Ft_type = 0;
long			Kill_length = 0;	/* True msg length */
#ifdef juniper
long			Millibuzz = 1300;	/* magic to initialize */
#else
long			Millibuzz = 200;	/* magic to initialize */
#endif /* juniper */
u_char			Outbfound = 0;		/* pxdread cent,whatever */
int			Pxd_dma_size = PXDMASIZ;
u_char			Rbuf[4*PXDMASIZ];	/* driver read buffer */
u_char			Read_flag = FALSE;
u_char			Rows = ROWS;

/*
**	Local variables
*/
char			ident[] = "@(#) Pxd 3279 Version 1.9, GL2 WS";
u_char			curr_status;
px_buf_t		pxk;			/* read buffer pointers */
u_char			logflag = 0;
FILE			*logfptr;
px_buf_t		pxin;
u_char			Ibuf[4*PXDMASIZ];
FILE			*ilf = 0;
u_char			loginum = 0;
long			offset = 0;
u_char			buf6[8];
long			get_r1[1];
u_char			dtext[] = {"/dev/pxd"};


u_char ftxlat[] = {

	' ',' ',' ',' ',' ',' ',' ',' ',	/* 00-07 */
	' ',' ',' ',' ',' ',' ',' ',' ',	/* 08-0f */
       	' ',' ',' ',' ',' ',' ',' ',' ',	/* 10-17 */
       	' ',' ',' ',' ',' ',' ',' ',' ',	/* 18-1f */
       	'\064','\065','\066','\067','\070','\071','\072','\073', /* 20-27 */
	'\074','\075',' ',' ',' ',' ',' ',' ',	/* 28-2f */
       	' ',' ','\076','\077',' ',' ',' ',' ',	/* 30-37 */
       	' ',' ',' ',' ',' ',' ',' ',' ',	/* 38-3f */
       	' ',' ',' ',' ',' ',' ',' ',' ',	/* 40-47 */
       	' ',' ',' ',' ',' ',' ',' ',' ',	/* 48-4f */
       	' ',' ',' ',' ',' ',' ',' ',' ',	/* 50-57 */
       	' ',' ',' ',' ',' ',' ',' ',' ',	/* 58-5f */
       	' ',' ',' ',' ',' ',' ',' ',' ',	/* 60-67 */
	' ',' ',' ',' ',' ',' ',' ',' ',	/* 68-6f */
       	' ',' ',' ',' ',' ',' ',' ',' ',	/* 70-77 */
       	' ',' ',' ',' ',' ',' ',' ',' ',	/* 78-7f */
	'\032','\033','\034','\035','\036','\037','\040','\041', /* 80-87 */
	'\042','\043','\044','\045','\046','\047','\050','\051', /* 88-8f */
	'\052','\053','\054','\055','\056','\057','\060','\061', /* 90-97 */
	'\062','\063',' ',' ',' ',' ',' ',' ',	/* 98-9f */
	'\000','\001','\002','\003','\004','\005','\006','\007', /* a0,a7 */
	'\010','\011','\012','\013','\014','\015','\016','\017', /* a8-af */
	'\020','\021','\022','\023','\024','\025','\026','\027', /* b0-b7 */
	'\030','\031',' ',' ',' ',' ',' ',' ',	/* b8-bf */
       	' ',' ',' ',' ',' ',' ',' ',' ',	/* c0-c7 */
       	' ',' ',' ',' ',' ',' ',' ',' ',	/* c8-cf */
	' ',' ',' ',' ',' ',' ',' ',' ',	/* d0-d7 */
	' ',' ',' ',' ',' ',' ',' ',' ',	/* d8-df */
       	' ',' ',' ',' ',' ',' ',' ',' ',	/* e0-e7 */
       	' ',' ',' ',' ',' ',' ',' ',' ',	/* e8-ef */
	' ',' ',' ',' ',' ',' ',' ',' ',	/* f0-f7 */
	' ',' ',' ',' ',' ',' ',' ',' '		/* f8-ff */
};


/*
**	Turn on/off the tracing of this module
*/
tr_pxdio(flag)
{
	trace = flag;
}


/*
**	Close the pxd device
*/
pxdclose()
{
#ifdef DEBUG
	DT("Pxd: close");
#endif /* DEBUG */
	if (!replay[4]) {
		kill_all();
		send_x_key(X_RESET);
	}
	return (close(fd));
}


/*
**	Open the pxd device
*/
pxdopen()
{
	u_char i = 38;
	u_char *p;

	if (errno == 9)
		errno = 0;
	if (Millibuzz == 123)
		(void)get_buzz();
	if (!replay[4]) {
		p = dtext;
		if (Manuflag) {
			printf("Doing minimal open\n");
			if ((fd = open(p, O_NDELAY)) <= 0) {
				perror("pxdopen error ");
				return (ERROR);
			}
			tfd = fd;
			db_conopen();
			return (38);
		}
		if ((fd = open(p, O_RDWR)) <= 0) {
			perror("pxdopen error ");
			return (ERROR);
		}
		p = (u_char *)&Fst;
		(void)read_open_init(&Fst);
		Buffer_is_mem = (*p == 1) ? TRUE : FALSE;
		Pxd_dma_size = (*p == 1) ? 4*PXDMASIZ : PXDMASIZ;
		tfd = fd;
		kill_all();
		Status_flags = 0;
		DMa.d_cnt = 0;
		if (Fst.mbchar == 1) {
			i = f13_diag();
			pdelay(MS_100);
			if (i < 38)
				printf("pxd:diag returned %d instead of 38 ",i);
		}
		db_conopen();
		if (!F3174 && MaxRU == DEFAULTRU)
			MaxRU = Buffer_is_mem ? 4*SMALL_DMA : SMALL_DMA;
		(void)get3270();
		get_screen();
	} else {
		if (!Display)
			lampoff(0x0f);
		printf("%s\n",inputfile);
		if ((fd = open(inputfile, O_RDONLY)) <= 0) {
			perror("replay open error ");
			return (ERROR);
		}
		if ((tfd = (int)fopen("REwrite", "w")) == 0) {
			perror("Rewrite open error ");
			return (ERROR);
		}
		Pxd_dma_size = 4*PXDMASIZ;
	}
	return (i);
}


/*
 * Pxdread moves data from the kernel to Rbuf
 */
long
pxdread()
{
	register u_char c1, c2;
	register long r1, r2;
	register u_char *kwrp, *rdp;
	extern char eat_3274();

	rdp = pxk.rdp;
	DMa.d_rdp = rdp;
	if (Outbfound) {
		outb.ftdone = ESTART;
		return (ERROR);
	}

	if ((pxk.wrp - pxk.rdp) <= 0) {	/* any previously read data? */
		errno = 0;
		Countr = replay[4] ? reread() : read(fd, Rbuf, sizeof(Rbuf));
		pxk.rdp = Rbuf;
		DMa.d_rdp = Rbuf;
		pxk.wrp = Rbuf + Countr;
		if (!Countr)
			return 0;
#ifdef DEBUG
		DT("#%x ",Countr);
#endif /* DEBUG */
		if (errno) {
			perror("pxd: pxdread error ");
#ifdef DEBUG
			DT("pxd: pxdread error %d ",errno);
			if (!replay[4]) {
				get_read_count(get_r1);
				DT("%x ",get_r1[0]);
			}
#endif /* DEBUG */
			return (ERROR);
		}
		Read_flag++;
 		LOGREAD
	}
	kwrp = pxk.wrp;			/* pronounced kwrap */
	rdp = pxk.rdp;
	r1 = 0;				/* read byte counter */
	r2 = kwrp - rdp;
	c1 = CENT;
/*
 * term reads character from Rbuf 
 */
dmaread:
	c2 = *rdp++;
	if (c2 == c1)
		if (*(rdp - 2) == CMD_WRITE || rdp == Rbuf + 1)
			goto rxfread;
rbufwrite:
	r1++;
	if (--r2)
		goto dmaread;

termout:
	pxk.rdp = rdp;
	return (r1);
/*
 * outb ft test for initialization because either:
 *	CMD_WRITE, cent and outb is enabled, or
 *	top of Rbuf, cent and outb is enabled
 */
rxfread:
	r1++;
	r2--;
	if (rdp >= kwrp) {
		(void)reread();
		kwrp = pxk.wrp;		/* after last char */
		r2 = kwrp - Rbuf;
		rdp = kwrp++ - 1;	/* last valid char */
		while (r2--)		/* move up 2 */
			*kwrp-- = *rdp--;
		*++rdp = CMD_WRITE;
		pxk.rdp = rdp;
		*++rdp = c2;		/* for emulint */
		pxk.wrp += 2;
		kwrp = pxk.wrp;		/* after last char */
		r2 = kwrp - ++rdp;
	}
	c2 = *rdp++;
	if (c2 == END_WRITE && outb.ft) {
		pxk.rdp = rdp;
		if (!(eat_3274())) {
			rdp = pxk.rdp;
			kwrp = pxk.wrp;
			r2 = kwrp - rdp;
			c2 = *rdp++;
			goto rbufwrite;
		}
		rdp = pxk.rdp;
		kwrp = pxk.wrp;
		r2 = kwrp - rdp;
		c2 = *rdp++;
	}
	if (r2 > 2)
		r2 = 2;		/* hasty exit after cent ? */
	if ((c2 != DATA2RXFER) && (c2 != DATA3RXFER) && (c2 != TEXT2RXFER)
		&& (c2 != TEXT3RXFER))
		goto rbufwrite;
	if (outb.ft == FALSE)
		goto rbufwrite;

	if (r1-- != 1)
		r1--;			/* CMD_WRITE, cent removed */
	Outbfound = c2;
	goto termout;
}


/*
 * pxdioctl does writes to signal and kbdata ports of px
 * the kb and signal routines are longs but use only a char
 * op	subroutine
 * 10	get_outb_status	read outb ft status
 * 12	kill_outb	disable outb ft
 * 16	set_rglout_ptr store outb ft addresses, max length
 * 17	set_rglcntu	for outb larger than max_len
 */
pxdioctl(dev, cmd, addr)
dev_t dev;
long *addr;
{

#ifdef DEBUG
	DT("i%d ",cmd);
#endif /* DEBUG */
	switch(cmd) {
	
	case 10:
		blt(addr,&outb,(int)sizeof(outb));
		break;
	case 12:
		if (outb.ftdone) {
			printf("pxd:kill_outb with ftdone = %d",outb.ftdone);
			outb.ftdone = 0;
		}
		Outbfound = outb.ft = outb.host_len = 0;
		break;
	case 16: /* contents of block is : */
		 /*   addr   u_char *bufp    = store body at */
		 /*   addr+4 long    max_len = limit of body length */

		outb.host_len = 0;
	case 17:
		outb.bufp = (u_char *)*addr;
		addr++;
		outb.max_len = *addr;
		outb.wrp = outb.bufp;
		Ft_type = outb.ft = MXFER;
		Outbfound = outb.ftdone = 0;
		break;
	default:
		beep();
	}
}


/*
**   Eat remaining characters after last of file xfer
*/
eat_trailing()
{
	register u_char c;
	register long count = ZAP_COUNT;
	register u_char *rdp;

	rdp = pxk.rdp;
	for (c = 0; c != END_WRITE && count; count-- ) {
		if (rdp >= pxk.wrp) {
			(void)reread();
			rdp = Rbuf;
		}
		c = *rdp++;		/* eat trailing stuff */
		if (!F3174 && c == CENT) {
			rdp--;
			printf("pxd:cent trailing\n");
#ifdef DEBUG
			if (replay[4])
				DT("cent trail offset %x ",offset);
#endif /* DEBUG */
			break;
		}
	}
	pxk.rdp = --rdp;
}


/*
 * eat_3274 removes the extraneous 3x74 commands from file transfers.
 *   called after read of END_WRITE and rdp at next position.
 *   read next bytes to see if write continues ;
 *   put in local buffer(buf6), then test position for continuation.
 * the usual sequences are:
 * e6 ca 14 ca 04 e2 xx e3 yy c3 after each segment
 *             e6 e2 00 e3 50 c3 after writing 1920 bytes, point home
 * e6 ca 14 ca 04 e2 00 e3 50 c3 1b 90 e6 for writing req ack in a file
 *
 *	returns	1 - still in file xfer, pxk.rdp after CMD_WRITE
 *		0 - rxfer done, pxk.rdp at position of CMD_WRITE
 */
char
eat_3274()
{
	register long rereadctr;
	register u_char e2, c, c3;
	register u_char *rdp, *kwrp, *wrp;
	int clear_3x74;

	rdp = pxk.rdp;
	kwrp = pxk.wrp;
	e2 = CMD_CURHI;
	c = 0;
	c3 = CMD_WRITE;
	clear_3x74 = FALSE;
try5:
	if (c == c3) {
		pxk.rdp = rdp;
		return CNTU;
	}
	if (rdp >= kwrp) {
		rereadctr = reread();
		kwrp = pxk.wrp;
		rdp = Rbuf;
	}
	c = *rdp++;
try5a:
	if (c != e2)
		goto try5;	/* probably load control reg command, data */
	buf6[1] = c;					/* e2, cursor high */

	if (rdp >= kwrp) {
		rereadctr = reread();
		kwrp = pxk.wrp;
		rdp = Rbuf;
	}
	c = *rdp++;
	if (c == e2)
		goto try5a;	/* was argument of cmd_cursor last time */
	buf6[2] = c;					/* xx */

	if (rdp >= kwrp) {
		rereadctr = reread();
		kwrp = pxk.wrp;
		rdp = Rbuf;
	}
	c = *rdp++;
	if (c != CMD_CURLO)
		goto try5a;
	buf6[3] = c;					/* e3, cursor low */

	if (rdp >= kwrp) {
		rereadctr = reread();
		kwrp = pxk.wrp;
		rdp = Rbuf;
	}
	c = *rdp++;
	buf6[4] = c;					/* yy */

 	if (c == ESPEC) {			/* 3274 special character */
 		if (rdp >= kwrp) {
			rereadctr = reread();
 			kwrp = pxk.wrp;
 			rdp = Rbuf;
 		}
 		c = *rdp++;				/* ff or 00 */
 		buf6[4] = ESPEC;
	}

	if (rdp >= kwrp) {
		rereadctr = reread();
		kwrp = pxk.wrp;
		rdp = Rbuf;
	}
	c = *rdp++;
	buf6[5] = c;					/* c3, CMD_WRITE */

	pxk.rdp = rdp;
	if (buf6[5] != c3) {
		if (c == (u_char)0xc6)
			clear_3x74 = TRUE;
		goto try5a;
	}
	if (buf6[2] != 0) {
		if (clear_3x74 == TRUE) {
#ifdef DEBUG
			DT("eating %02x%02x ",buf6[2],buf6[4]);
#endif /* DEBUG */
			c = buf6[2];
			goto try5a;
		} else
			return (CNTU);			/* still pumping */
	}
	if (buf6[4] >= COLS) {
		if (buf6[4] != COLS)
			return (CNTU);			/* still pumping */
		else {
			clear_3x74 = FALSE;
			e2 = 0;				/* test fix reads */
			rereadctr = 0;
			if (rdp >= kwrp) {
				rereadctr = reread();
				kwrp = pxk.wrp;
				e2 = 1;			/* must fix reads */
			}
			c = *pxk.rdp;
			if (c != CENT)
				return (CNTU);		/* still pumping */
			if (rdp+1 >= kwrp) {
				rereadctr = reread();
				kwrp = pxk.wrp;
				e2 = 2;			/* must fix reads */
				c = *pxk.rdp;
				if (c == DATA3RXFER || c == TEXT3RXFER) {
					pxk.rdp++;
					goto test_for_cent_c;
				}
			} else
				c = *(pxk.rdp+1);
			if (c == DATA3RXFER || c == TEXT3RXFER) {
				pxk.rdp += 2;
test_for_cent_c:
				if (c == DATA3RXFER) {
					rdp = pxk.rdp;
					if (rdp >= kwrp) {
						rereadctr = reread();
						kwrp = pxk.wrp;
						rdp = Rbuf;
					}
					c = *rdp;
					if (!c) {	/* eat 14 nulls */
						e2 = 14;
						do {
							if (rdp >= kwrp) {
								rereadctr = reread();
								kwrp = pxk.wrp;
								rdp = Rbuf;
							}
							c = *rdp++;
						} while (--e2);
					}
					pxk.rdp = rdp;
				}
				return (CNTU);
			}
#ifdef DEBUG
			DT("Cent %02x&",c);
#endif /* DEBUG */
			if (e2) {
				rdp = kwrp - 1;		/* last valid char */
				if (e2 == 2)
					kwrp++;		/* move up 2 */
				pxk.wrp = kwrp + 1;
				while (rereadctr--)	/* move up */
					*kwrp-- = *rdp--;
	/* leave write cmd in buffer for getby_nowait to return */
				*++rdp = c3;		/* for emulint */
				pxk.rdp = rdp + 1;	/* adjust decs */
				kwrp = pxk.wrp;
				if (e2 == 2)
					*pxk.rdp = CENT;
#ifdef DEBUG
				DT("%d %02x %x ",e2,c,pxk.wrp-Rbuf);
#endif /* DEBUG */
			}
		}
	} else {
	/*
	** update the status line for Sure_reset and others
	*/
		wrp = Screen;
		wrp += buf6[4];
		do {
			if (rdp >= kwrp) {
				rereadctr = reread();
				kwrp = pxk.wrp;
				rdp = Rbuf;
			}
			c = *rdp++;
			*wrp++ = c;
		} while (c != END_WRITE);
		goto try5;
	}
	/* leave write cmd in buffer for next read to return 
	** The termination case is: CMD_WRITE,CENT (not c or m) found
	*/
	if (c == KILLIT) {
	/* test and read true length if present */
		if ((pxk.rdp + 2) >= kwrp) {
			rereadctr = reread();
			kwrp = pxk.wrp;
			c = *pxk.rdp;
			if (c != (u_char)0xbe)		/* ';' */
				goto load_fake_ck;
			pxk.rdp++;
			goto get_true_length;
		} else
			c = *(pxk.rdp + 2);
		if (c != (u_char)0xbe)			/* ';' */
			goto adjust;
		for (e2 = 3; e2; e2--) {		/* ck; */
			if (pxk.rdp >= kwrp) {
				rereadctr = reread();
				kwrp = pxk.wrp;
			}
			pxk.rdp++;
		}
get_true_length:
		Kill_length = 0;
		for (e2 = 9; e2; e2--) {
			if (pxk.rdp >= kwrp) {
				rereadctr = reread();
				kwrp = pxk.wrp;
			}
			c = *pxk.rdp++;
			DT("%02x=",c);
			if ((c & (u_char)0xf0) != 0x20) { /* not numb */
#ifdef DEBUG
				DT(" c %02x BAD ",c);
#endif /* DEBUG */
				Kill_length = 0;
				goto adjust;
			}
			Kill_length += c - 0x20;
			if (e2 != 1)
				Kill_length *= 10;
		}
#ifdef DEBUG
		DT("Kill_length %d ",Kill_length);
#endif /* DEBUG */
		/* now load fake ck for emulint */
load_fake_ck:
		if (pxk.rdp < Rbuf + 3) {
			pxk.wrp = Rbuf + 4;		/* jam it in anyway */
			pxk.rdp = Rbuf + 3;
		}
		*pxk.rdp-- = 0;
		*pxk.rdp-- = KILLIT;
		*pxk.rdp = CENT;
		*(pxk.rdp-1)= CMD_WRITE;
	}
adjust:
	pxk.rdp--;				/* to CMD_WRITE */
	return 0;
}


/*
**	Set Millibuzz by timing 9 * 16.67 msec in ioctls
*/
get_buzz()
{
	time_t now,prev;
	register long i, tcount;
	struct tms tmp;
	int j = 0, loop_counter = 0;

	Btm = &tmp;
repeat:
	if ((now = prev = (SEVENF & times(Btm))) == SEVENF) {
		perror("pxd:get_buzz1 ");
	}
	while (now == prev)		/* sync with times */
		if ((now = (SEVENF & times(Btm))) == SEVENF)
			perror("pxd:get_buzz2 ");
	prev = now;
	i = 0;
	while (now == prev) {		/* count 16.67 msec */
		for (tcount = 256 ; tcount--; )
			;
		if ((now = (SEVENF & times(Btm))) == SEVENF)
			perror("pxd:get_buzz3 ");
		i++;
	}
	prev = now;
	Millibuzz = i * 256;		/* temp, real # is >> 4 */
	i = 0;
	while (now < prev + 8) {	/* count 8 * 16.67 msec */
		for (tcount =  Millibuzz ; tcount--; )
			;
		if ((now = (SEVENF & times(Btm))) == SEVENF)
			perror("pxd:get_buzz4 ");
		i++;
	}
	Millibuzz = (Millibuzz >> 7) * i;
	j++;
	loop_counter++;
#ifdef juniper
	if (loop_counter > 3)
		Millibuzz = 1300;	/* other UNIX jobs eating cpu */
	if (Millibuzz < 1300)
		goto repeat;
#endif /* juniper */
#ifdef iris
	if (loop_counter > 3)
		Millibuzz = 200;	/* other UNIX jobs eating cpu */
	if (Millibuzz < 200)
		goto repeat;
	else 
		Millibuzz *= 2;
#endif /* iris */
	return (j);
}


/*
**	Force the PCOX card to send a fresh panel
*/
get_it()
{
	(void)raise1();
	pdelay(MS_100);
	(void)reset_signal(SIG1);
	pdelay(MS_100);
}


/*
**	Force the PCOX card to send a fresh panel, then input 
**      and mark for display.
*/
get_screen()
{
	get_it();
	(void)emulint();
	repaint(0, Rows - 1);
}


kill_all()
{
	Msg_proc = Msgtype = 0;
	kill_outb();
}


logenable(flag)
{

	logflag = flag - 1;
	if (flag == 2) {
		if ((logfptr = fopen("RAWINP", "w")) == NULL) {
			logflag = 0;
			printf("Cannot open RAWINP\r");
		} else
			setbuf (logfptr, (char *)0);
	} else if (flag == 3)
		pxin.wrp = Ibuf;
}


logread(r1)
{
	u_char bufr[4];
	register u_char *rdp, *wrp, *kwrp;
 
	if (logflag == 1) {
		bufr[0] = DMA_ZAP;
		bufr[1] = (u_char)(r1 >> 16);
		bufr[2] = (u_char)(r1 >> 8);
		bufr[3] = (u_char)r1;
		fwrite(bufr, 1, 4, logfptr);
		errno = 0;
		fwrite(Rbuf,1,Countr,logfptr);
		errno=0;
		return;
	}  else if (logflag != 2)
		return;
#ifdef DEBUG
	DT("=%x ",r1);
#endif /* DEBUG */
	rdp = Rbuf;
	wrp = pxin.wrp;
	kwrp = Ibuf + sizeof(Ibuf);
	*wrp++ = DMA_ZAP;
	if (wrp >= kwrp)
		wrp = Ibuf;
	*wrp++ = (u_char)(r1 >> 16);
	if (wrp >= kwrp)
		wrp = Ibuf;
	*wrp++ = (u_char)(r1 >> 8);
	if (wrp >= kwrp)
		wrp = Ibuf;
	*wrp++ = (u_char)r1;
	if (wrp >= kwrp)
		wrp = Ibuf;
	while (r1--) {
		*wrp++ = *rdp++;
		if (wrp >= kwrp)
			wrp = Ibuf;
	}
	pxin.wrp = wrp;
}

logwrite(c)
u_char c;
{
	register u_char *wrp, *kwrp;
	u_char bufr[4];
 
	if (logflag == 1) {
		bufr[0] = DMA_ZAP;
		bufr[1] = DMA_ZAP;
		bufr[2] = (u_char)c;
		fwrite(bufr, 1, 3, logfptr);
		errno = 0;
	} else if (logflag == 2) {
		wrp = pxin.wrp;
		kwrp = Ibuf + sizeof(Ibuf);
		*wrp++ = DMA_ZAP;
		if (wrp >= kwrp)
			wrp = Ibuf;
		*wrp++ = DMA_ZAP;
		if (wrp >= kwrp)
			wrp = Ibuf;
		*wrp++ = (u_char)c;
		if (wrp >= kwrp)
			wrp = Ibuf;
		pxin.wrp = wrp;
	}
}


logstr(str, count)
register u_char *str;
int count;
{
	register u_char *wrp, *kwrp;
	u_char bufr[4];
	short scount;

	scount = (short)count;
	if (logflag == 1) {
		bufr[0] = DMA_ZAP;
		bufr[1] = DMA_ZAP;
		bufr[2] = DMA_ZAP;
		fwrite (bufr,1,3,logfptr);
		errno = 0;
		bufr[0] = (u_char)(scount >> 8);
		bufr[1] = (u_char)scount;
		fwrite (bufr,1,2,logfptr);
		errno = 0;
		fwrite (str,1,count,logfptr);
		errno = 0;
	} else if (logflag == 2) {
		wrp = pxin.wrp;
		kwrp = Ibuf + sizeof(Ibuf);
		*wrp++ = DMA_ZAP;
		if (wrp >= kwrp)
			wrp = Ibuf;
		*wrp++ = DMA_ZAP;
		if (wrp >= kwrp)
			wrp = Ibuf;
		*wrp++ = DMA_ZAP;
		if (wrp >= kwrp)
			wrp = Ibuf;
		*wrp++ = (u_char)(scount >> 8);
		if (wrp >= kwrp)
			wrp = Ibuf;
		*wrp++ = (u_char)scount;
		if (wrp >= kwrp)
			wrp = Ibuf;
		do {
			*wrp++ = *str++;
			if (wrp >= kwrp)
				wrp = Ibuf;
		} while (--count);
		pxin.wrp = wrp;
	}
}

logdump()
{
	u_char *bufp, bufr[64];
	int ic;

	if (logflag != 2)
		return;
	if (ilf)
	    fclose(ilf);
        loginum++;
	if (dflag[4]) {
		strcpy((char *)bufr,inputfile);
		bufp = bufr + dflag[4];
		sprintf(bufp, ".%d", loginum);
	} else
		sprintf(bufr, "ILOG.%d", loginum);
	printf(bufr);
	printf(" ");
	if ((ilf = fopen(bufr, "w")) == NULL) {
	    oops("t3279: can't open %s\n\r", bufr);
	    return;
	}
	setbuf(ilf,0);	/* no buffering */
	ic = sizeof(Ibuf) - (pxin.wrp - Ibuf);
	if (ic)
		fwrite(pxin.wrp, 1, ic, ilf);
	ic = pxin.wrp - Ibuf;
	if (ic)
		fwrite(Ibuf, 1, ic, ilf);
	fclose(ilf);
	ilf = 0;
}


/*
 *	Outbread reads the outb message into
 *   the rxfer address of the set_outb_ptr ioctl request.
 *   This read can process $, c binary messages and t, m
 *   text messages from the host.
 */
long
outbread()
{
	extern long readbin(), readtext();

/*
 * prepare for outb ft as we have CMD_WRITE, cent, (buck, c, t, m)
 */
	if (Blink_msgs)
		lampon(1);
#ifdef DEBUG
	DT("O %02x ",*pxk.rdp);
#endif /* DEBUG */
	if (Outbfound == DATA2RXFER || Outbfound == DATA3RXFER)
		return(readbin());
	else if (Outbfound == TEXT2RXFER || Outbfound == TEXT3RXFER)
		return(readtext());
	else if (!outb.host_len) {
		if (Outbfound != DATA2RXFER && Outbfound != TEXT2RXFER) {
			outb.ftdone = ECNTU+1;
			goto clearout;
		}
	} else {			/* continuation frame */
#ifdef DEBUG
	DT("Outb %02x host_len %d\n",Outbfound,outb.host_len);
#endif /* DEBUG */
		outb.ftdone = ECNTU;
	}
clearout:
	outb.ft = outb.host_len = 0;
	Outbfound = 0;
	if (Blink_msgs)
		lampoff(1);
#ifdef DEBUG
	DT("pxd: outbread error %d ",outb.ftdone);
#endif /* DEBUG */
	return (ERROR);
}


/*
** delay t msecs
*/
pdelay(t)
long t;
{
	register long u;

	u = Millibuzz * t;
	while (u--)
		;
}


/*
**	Return YES if PCOX dma address moved since last call
*/
pxstat()
{

	register int i;

	if (pxk.wrp - pxk.rdp > 0)
		return CNTU;
	if (replay[1])
		return CNTU;
	if (i = read_avail(&curr_status))
		printf("pxd:pxstat read_avail %d",i);
	return (curr_status);
}


/*
 *	readbin reads the outb message into
 *   the rxfer address of the set_outb_ptr ioctl request.
 *   This read can process $, c binary messages from the host.
 */
long
readbin()
{
	register u_char c1,c2;
	register u_char *dispxlate, *kwrp, *rdp, *wrp;
	register long minlen, r1, r2;
	char reading_len = 1, z2;
	long hostlen, minlen_save;

/*
 * prepare for outb ft as we have CMD_WRITE, cent, (buck, c)
 */
	rdp = pxk.rdp;
	if (!outb.host_len) {
		if (Outbfound != DATA2RXFER) {
			outb.ftdone = ECNTU+1;
			goto clearout;
		}
		readlen();		/* first frame includes length */
 		if (outb.host_len == 0) {
 			eat_trailing();
 			rdp = pxk.rdp;
 			goto clearout;	/* part of rays 12 byte trailer */
 		}
		if (Outbfound == DATA2RXFER) {
			if (outb.host_len <= 12) {
				minlen_save = hostlen = outb.host_len;
				minlen = 0;
				goto done_screen;
			}
			hostlen = outb.host_len - 3;	/* length itself 3 */
			kwrp = pxk.wrp;
			rdp = pxk.rdp;
			minlen = (hostlen < outb.max_len) ? hostlen : outb.max_len;
			minlen_save = minlen;
			minlen -= 9;			/* readlen wrote 9 */
			wrp = outb.wrp; 
			goto now_init_xfer;
		} 
	} else {			/* continuation frame */
		reading_len = 0;
		if (Outbfound != DATA3RXFER) {
#ifdef DEBUG
	DT("hostlen %d Outb %02x host_len %d\n",hostlen,Outbfound,outb.host_len);
#endif /* DEBUG */
			outb.ftdone = ECNTU;
			goto clearout;
		}
		kwrp = pxk.wrp;
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp;
		if (!c2) {
			r2 = 14;
			do {
				if (rdp >= kwrp) {
					(void)reread();
					kwrp = pxk.wrp;
					rdp = Rbuf;
				}
				c2 = *rdp++;
			} while (--r2);
		}
		pxk.rdp = rdp;
		hostlen = outb.host_len;/* working length */
		wrp = outb.wrp;
	}
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (hostlen <= 0) {
		if (outb.ftdone == ESTART)
			outb.ftdone = ELENGTHBAD;
 		eat_trailing();
		printf("pxd:hostlen %x ft %d ",hostlen,outb.ft);
		rdp = pxk.rdp;
		goto clearout;		/* host said neg bytes */
	}
	minlen = (hostlen < outb.max_len) ? hostlen : outb.max_len;
	minlen_save = minlen;
now_init_xfer:
	c1 = END_WRITE;
	dispxlate = ftxlat;
	outb.ftdone = EACK;		/* default for ft */
	goto rxfr;
/*
 * read next bytes to see if write continues ;
 */
try4a:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+16 <= kwrp)
			goto rxsta;
		else
			goto rxfra;
	}
	goto try4;

try4b:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+15 <= kwrp)
			goto rxstb;
		else
			goto rxfrb;
	}
	goto try4;

try4c:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+14 <= kwrp)
			goto rxstc;
		else
			goto rxfrc;
	}
	goto try4;

try4d:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+13 <= kwrp)
			goto rxstd;
		else
			goto rxfrd;
	}
	goto try4;
	
try4e:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+12 <= kwrp)
			goto rxste;
		else
			goto rxfre;
	}
	goto try4;

try4f:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+11 <= kwrp)
			goto rxstf;
		else
			goto rxfrf;
	}
	goto try4;

try4g:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+10 <= kwrp)
			goto rxstg;
		else
			goto rxfrg;
	}
	goto try4;

try4h:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+9 <= kwrp)
			goto rxsth;
		else
			goto rxfrh;
	}
	goto try4;
	
try4i:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+8 <= kwrp)
			goto rxsti;
		else
			goto rxfri;
	}
	goto try4;

try4j:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+7 <= kwrp)
			goto rxstj;
		else
			goto rxfrj;
	}
	goto try4;

try4k:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+6 <= kwrp)
			goto rxstk;
		else
			goto rxfrk;
	}
	goto try4;

try4l:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+5 <= kwrp)
			goto rxstl;
		else
			goto rxfrl;
	}
	goto try4;
	
try4m:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+4 <= kwrp)
			goto rxstm;
		else
			goto rxfrm;
	}
	goto try4;

try4n:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+3 <= kwrp)
			goto rxstn;
		else
			goto rxfrn;
	}
	goto try4;

try4o:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+2 <= kwrp)
			goto rxsto;
		else
			goto rxfro;
	}
	goto try4;

try4p:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+1 <= kwrp)
			goto rxstp;
		else
			goto rxfrp;
	}
	
try4:
#ifdef DEBUG
	DT("hostlen %d minlen_save %d minlen %d\n",hostlen,minlen_save,minlen);
#endif /* DEBUG */
	if (Kill_length && (outb.length != Kill_length)) {
#ifdef DEBUG
	DT("Kill %d length %d host_len %d minlen_save %d minlen %d ",
	Kill_length,outb.length,outb.host_len,minlen_save,minlen);
#endif /* DEBUG */
		Kill_length += 3;		/* length field */
		Kill_length += outb.host_len;
		Kill_length -= outb.length;
		Kill_length -= minlen_save;
		minlen = -Kill_length;
		outb.ftdone = EDONE;
#ifdef DEBUG
	DT("new minlen %d \n",minlen);
#endif /* DEBUG */
	}
	hostlen -= minlen_save - minlen;
	if (hostlen <= 0 || hostlen == -minlen) {
		hostlen = 0;
		outb.ft = 0;
		outb.ftdone = EDONE;
	}
	goto done_rxfer;

/*
 *   main outb loop must be fast so change carefully and check adb for speed
 */
rxfr:	if (rdp+16 <= kwrp) {			/* don't need individual tests*/
rxsta:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4a;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
rxstb:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4b;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
rxstc:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4c;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
rxstd:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4d;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
		r2 = r1 >> 8;
		*wrp++ = r2 >> 8;
		*wrp++ = r2;
		*wrp++ = r1;
		minlen -= 3;
rxste:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4e;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
rxstf:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4f;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
rxstg:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4g;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
rxsth:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4h;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
		r2 = r1 >> 8;
		*wrp++ = r2 >> 8;
		*wrp++ = r2;
		*wrp++ = r1;
		minlen -= 3;
rxsti:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4i;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
rxstj:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4j;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
rxstk:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4k;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
rxstl:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4l;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
		r2 = r1 >> 8;
		*wrp++ = r2 >> 8;
		*wrp++ = r2;
		*wrp++ = r1;
		minlen -= 3;
		if (minlen <= 0)
			goto done_screen;
rxstm:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4m;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
rxstn:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4n;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
rxsto:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4o;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
rxstp:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4p;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
		r2 = r1 >> 8;
		*wrp++ = r2 >> 8;
		*wrp++ = r2;
		*wrp++ = r1;
		minlen -= 3;
	} else {
	
rxfra:						/* test for reread each byte */
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4a;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
rxfrb:
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4b;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
rxfrc:
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4c;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
rxfrd:
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4d;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
		r2 = r1 >> 8;
		*wrp++ = r2 >> 8;
		*wrp++ = r2;
		*wrp++ = r1;
		minlen -= 3;

rxfre:						/* test for reread each byte */
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4e;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
rxfrf:
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4f;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
rxfrg:
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4g;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
rxfrh:
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4h;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
		r2 = r1 >> 8;
		*wrp++ = r2 >> 8;
		*wrp++ = r2;
		*wrp++ = r1;
		minlen -= 3;

rxfri:						/* test for reread each byte */
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4i;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
rxfrj:
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4j;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
rxfrk:
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4k;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
rxfrl:
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4l;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
		r2 = r1 >> 8;
		*wrp++ = r2 >> 8;
		*wrp++ = r2;
		*wrp++ = r1;
		minlen -= 3;
		if (minlen <= 0)
			goto done_screen;

rxfrm:						/* test for reread each byte */
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4m;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
rxfrn:
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4n;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
rxfro:
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4o;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
rxfrp:
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4p;
		r1 <<= 6;
		r1 |= *(dispxlate + c2);
		r2 = r1 >> 8;
		*wrp++ = r2 >> 8;
		*wrp++ = r2;
		*wrp++ = r1;
		minlen -= 3;

	}
	if (minlen > 0)
		goto rxfr;

done_screen:
	hostlen -= minlen_save - minlen;
	if (hostlen <= 0) {
		hostlen = 0;
		pxk.rdp = rdp;
		eat_trailing();
		rdp = pxk.rdp;
		outb.ftdone = EDONE;
		outb.ft = 0;
	}
/*
 * clear xfer flags, save regs, return
 */
done_rxfer:
	pxk.rdp = rdp;
	outb.wrp = wrp;
	if ((outb.ftdone != EDONE) && (outb.ftdone != EACK)) { 

clearout:
		outb.ft = outb.host_len = 0;
		Outbfound = 0;
		pxk.rdp = rdp;
		if (Blink_msgs)
			lampoff(1);
#ifdef DEBUG
		DT("pxd: outbread error %d ",outb.ftdone);
#endif /* DEBUG */
		return (ERROR);
	}
	if (Blink_msgs)
		lampoff(1);
	outb.host_len = hostlen;	/* working length remaining */
#ifdef DEBUG
	DT("ftdone %d rl %d Kl %d O %02x ",outb.ftdone,reading_len,Kill_length,Outbfound);
#endif /* DEBUG */
	if (outb.ftdone == EDONE) {
		if (reading_len) {
			if (Kill_length) {
				Kill_length = 0;
				return (minlen_save - minlen);
			} else
				return (outb.length);
		} else {
			Kill_length = 0;
			return (minlen_save - minlen);
		}
	}
	if (reading_len)
		minlen_save += 3; /* bin length field */
	return (minlen_save - minlen);	/* partial read */

}

/*
** read outb length which is packed 4:3 coded for binary or
**   six 3274 characters for text
*/
readlen()
{
	register u_char c1,c2;
	register long r1, r2, r3;
	register u_char *dispxlate, *rdp, *wrp, *kwrp;
	char z2;

	outb.wrp = outb.bufp;
	rdp = pxk.rdp;
	wrp = outb.wrp;
	kwrp = pxk.wrp;
	r1 = 0;
	r3 = 3;			/* put length + 9 data at wrp */
	c1 = END_WRITE;
	dispxlate = ftxlat;
	if (Outbfound == DATA2RXFER) {
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp;
		if (!c2) {
			r2 = 14;
			do {
				if (rdp >= kwrp) {
					(void)reread();
					kwrp = pxk.wrp;
					rdp = Rbuf;
				}
				c2 = *rdp++;
			} while (--r2);
		}
		r2 = 3;
		goto rxfr;		/* binary outb */
	}
	r2 = 5;
	goto txtfer;
/*
 * read next bytes to see if write continues ;
 */
try3:
	pxk.rdp = rdp;
	outb.wrp = wrp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (!z2) {
		outb.ftdone = ERZAP;
#ifdef DEBUG
		DT("readlen try3 zapped ");
#endif /* DEBUG */
		goto done_rxfer;
	}
txtfer:					/* 6 char length */
	if (rdp >= kwrp) {
		(void)reread();
		kwrp = pxk.wrp;
		rdp = Rbuf;
	}
	c2 = *rdp++;
	if (c2 == c1)
		goto try3;
	r1 <<= 4;
	c2 -= 0x20;			/* zero is 0x20 */
	if (c2 > 9)
		c2 -= 0x76;		/* A is 0xA0 */
	r1 |= c2;
	if (r2--)
		goto txtfer;

	outb.length = outb.host_len = r1;
	goto done_rxfer;

/*
 * read next bytes to see if write continues
 */
try4:
	pxk.rdp = rdp;
	outb.wrp = wrp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (!z2) {
		outb.ftdone = ERZAP;
#ifdef DEBUG
		DT("readlen try4 rdp %x offset %x ",pxk.rdp,offset);
#endif /* DEBUG */
		goto done_rxfer;
	}
rxfr:
	if (rdp >= kwrp) {
		(void)reread();
		kwrp = pxk.wrp;
		rdp = Rbuf;
	}
	c2 = *rdp++;
	if (c2 == c1)
		goto try4;
	r1 <<= 6;
	r1 |= *(dispxlate + c2);
	if (r2--)
		goto rxfr;

	r2 = r1 >> 8;
	*wrp++ = r2 >> 8;
	*wrp++ = r2;
	*wrp++ = r1;
	if (r3 == 3) {
		outb.length = outb.host_len = r1;
#ifdef DEBUG
		DT("readlen %x ",r1);
#endif /* DEBUG */
		wrp -= 3;
	}
	r1 = 0;
	r2 = 3;
	if (r3--)
		goto rxfr;

done_rxfer:
	outb.wrp = wrp;
	pxk.rdp = rdp;
}


/*
 *	readtext reads the outb message into
 *   the rxfer address of the set_outb_ptr ioctl request.
 *   This read can process t, m text messages from the host.
 */
long
readtext()
{
	register u_char c1,c2;
	register u_char *dispxlate, *kwrp, *rdp, *wrp;
	register long minlen, r1, r2;
	char reading_len = 1, z2;
	long hostlen, minlen_save;

	rdp = pxk.rdp;
	if (!outb.host_len) {
		if (Outbfound != TEXT2RXFER) {
			outb.ftdone = ECNTU+1;
			goto clearout;
		}
		readlen();		/* first frame includes length */
 		if (outb.host_len == 0) {
 			eat_trailing();
 			rdp = pxk.rdp;
 			goto clearout;	/* part of rays 12 byte trailer */
 		}
		hostlen = outb.host_len - 6;	/* length itself 6 */
		wrp = outb.wrp; 
	} else {			/* continuation frame */
		reading_len = 0;
		if (Outbfound != TEXT3RXFER) {
#ifdef DEBUG
	DT("hostlen %d Outb %02x host_len %d\n",hostlen,Outbfound,outb.host_len);
#endif /* DEBUG */
			outb.ftdone = ECNTU;
			goto clearout;
		}
		hostlen = outb.host_len;/* working length */
		wrp = outb.wrp;
	}
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (hostlen <= 0) {
		if (outb.ftdone == ESTART)
			outb.ftdone = ELENGTHBAD;
 		eat_trailing();
		printf("pxd:hostlen %x ft %d ",hostlen,outb.ft);
		rdp = pxk.rdp;
		goto clearout;		/* host said neg bytes */
	}
	minlen = (hostlen < outb.max_len) ? hostlen : outb.max_len;
	minlen_save = minlen;
now_init_xfer:
	c1 = END_WRITE;
	dispxlate = ftxlat;
	outb.ftdone = EACK;		/* default for ft */
	dispxlate = Display_xlat;
	goto txtfer;
/*
 * read next bytes to see if text continues ;
 */

try4a:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+16 <= kwrp && minlen > 15)
			goto rxsta;
		else
			goto rxfra;
	}
	goto try4;

try4b:
	pxk.rdp = rdp;
	minlen -= 1;
	if (minlen <= 0)
		goto try4;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	minlen += 1;
	if (z2) {
		if (rdp+15 <= kwrp && minlen > 14)
			goto rxstb;
		else
			goto rxfrb;
	}
	goto try4;

try4c:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+14 <= kwrp && minlen > 13)
			goto rxstc;
		else
			goto rxfrc;
	}
	minlen -= 2;
	goto try4;

try4d:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+13 <= kwrp && minlen > 12)
			goto rxstd;
		else
			goto rxfrd;
	}
	minlen -= 3;
	goto try4;
	
try4e:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+12 <= kwrp && minlen > 11)
			goto rxste;
		else
			goto rxfre;
	}
	minlen -= 4;
	goto try4;

try4f:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+11 <= kwrp && minlen > 10)
			goto rxstf;
		else
			goto rxfrf;
	}
	minlen -= 5;
	goto try4;

try4g:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+10 <= kwrp && minlen > 9)
			goto rxstg;
		else
			goto rxfrg;
	}
	minlen -= 6;
	goto try4;

try4h:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+9 <= kwrp && minlen > 8)
			goto rxsth;
		else
			goto rxfrh;
	}
	minlen -= 7;
	goto try4;
	
try4i:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+8 <= kwrp && minlen > 7)
			goto rxsti;
		else
			goto rxfri;
	}
	minlen -= 8;
	goto try4;

try4j:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+7 <= kwrp && minlen > 6)
			goto rxstj;
		else
			goto rxfrj;
	}
	minlen -= 9;
	goto try4;

try4k:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+6 <= kwrp && minlen > 5)
			goto rxstk;
		else
			goto rxfrk;
	}
	minlen -= 10;
	goto try4;

try4l:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+5 <= kwrp && minlen > 4)
			goto rxstl;
		else
			goto rxfrl;
	}
	minlen -= 11;
	goto try4;
	
try4m:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+4 <= kwrp && minlen > 3)
			goto rxstm;
		else
			goto rxfrm;
	}
	minlen -= 12;
	goto try4;

try4n:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+3 <= kwrp && minlen > 2)
			goto rxstn;
		else
			goto rxfrn;
	}
	minlen -= 13;
	goto try4;

try4o:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+2 <= kwrp && minlen > 1)
			goto rxsto;
		else
			goto rxfro;
	}
	minlen -= 14;
	goto try4;

try4p:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2) {
		if (rdp+1 <= kwrp && minlen)
			goto rxstp;
		else
			goto rxfrp;
	}
	minlen -= 15;
	
try4:
#ifdef DEBUG
	DT("hostlen %d minlen_save %d minlen %d\n",hostlen,minlen_save,minlen);
#endif /* DEBUG */

	if (Kill_length && (outb.length != Kill_length)) {
#ifdef DEBUG
	DT("Kill %d length %d host_len %d minlen_save %d minlen %d ",
	Kill_length,outb.length,outb.host_len,minlen_save,minlen);
#endif /* DEBUG */
		Kill_length += 6;		/* length field */
		Kill_length += outb.host_len;
		Kill_length -= outb.length;
		Kill_length -= minlen_save;
		minlen = -Kill_length;
		outb.ftdone = EDONE;
#ifdef DEBUG
	DT("new minlen %d \n",minlen);
#endif /* DEBUG */
	}
	hostlen -= minlen_save - minlen;
	if (hostlen <= 0 || hostlen == -minlen) {
		hostlen = 0;
		outb.ft = 0;
		outb.ftdone = EDONE;
	}
	goto done_rxfer;

/*
 *   main text loop must be fast so change carefully and check adb for speed
 */
txtfer:	if (rdp+16 <= kwrp && minlen >= 16) {			/* don't need individual tests*/
rxsta:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4a;
		*wrp++ = *(dispxlate + c2);
rxstb:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4b;
		*wrp++ = *(dispxlate + c2);
rxstc:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4c;
		*wrp++ = *(dispxlate + c2);
rxstd:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4d;
		*wrp++ = *(dispxlate + c2);
rxste:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4e;
		*wrp++ = *(dispxlate + c2);
rxstf:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4f;
		*wrp++ = *(dispxlate + c2);
rxstg:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4g;
		*wrp++ = *(dispxlate + c2);
rxsth:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4h;
		*wrp++ = *(dispxlate + c2);
rxsti:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4i;
		*wrp++ = *(dispxlate + c2);
rxstj:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4j;
		*wrp++ = *(dispxlate + c2);
rxstk:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4k;
		*wrp++ = *(dispxlate + c2);
rxstl:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4l;
		*wrp++ = *(dispxlate + c2);
rxstm:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4m;
		*wrp++ = *(dispxlate + c2);
rxstn:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4n;
		*wrp++ = *(dispxlate + c2);
rxsto:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4o;
		*wrp++ = *(dispxlate + c2);
rxstp:
		c2 = *rdp++;
		if (c2 == c1)
			goto try4p;
		*wrp++ = *(dispxlate + c2);
		minlen -= 16;
		if (minlen <= 0)
			goto done_screen;
	} else {
	
rxfra:						/* test for reread each byte */
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4a;
		*wrp++ = *(dispxlate + c2);
rxfrb:
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4b;
		*wrp++ = *(dispxlate + c2);
rxfrc:
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4c;
		*wrp++ = *(dispxlate + c2);
rxfrd:
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4d;
		*wrp++ = *(dispxlate + c2);

rxfre:						/* test for reread each byte */
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4e;
		*wrp++ = *(dispxlate + c2);
rxfrf:
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4f;
		*wrp++ = *(dispxlate + c2);
rxfrg:
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4g;
		*wrp++ = *(dispxlate + c2);
rxfrh:
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4h;
		*wrp++ = *(dispxlate + c2);

rxfri:						/* test for reread each byte */
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4i;
		*wrp++ = *(dispxlate + c2);
rxfrj:
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4j;
		*wrp++ = *(dispxlate + c2);
rxfrk:
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4k;
		*wrp++ = *(dispxlate + c2);
rxfrl:
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4l;
		*wrp++ = *(dispxlate + c2);

rxfrm:						/* test for reread each byte */
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4m;
		*wrp++ = *(dispxlate + c2);
rxfrn:
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4n;
		*wrp++ = *(dispxlate + c2);
rxfro:
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4o;
		*wrp++ = *(dispxlate + c2);
rxfrp:
		if (rdp >= kwrp) {
			(void)reread();
			kwrp = pxk.wrp;
			rdp = Rbuf;
		}
		c2 = *rdp++;
		if (c2 == c1)
			goto try4p;
		*wrp++ = *(dispxlate + c2);
		minlen -= 16;
	}
	if (minlen > 0)
		goto txtfer;

done_screen:
	hostlen -= minlen_save - minlen;
	if (hostlen <= 0) {
		hostlen = 0;
		pxk.rdp = rdp;
		eat_trailing();
		rdp = pxk.rdp;
		outb.ftdone = EDONE;
		outb.ft = 0;
	}
/*
 * clear xfer flags, save regs, return
 */
done_rxfer:
	pxk.rdp = rdp;
	outb.wrp = wrp;
	if ((outb.ftdone != EDONE) && (outb.ftdone != EACK)) { 

clearout:
		outb.ft = outb.host_len = 0;
		Outbfound = 0;
		pxk.rdp = rdp;
		if (Blink_msgs)
			lampoff(1);
#ifdef DEBUG
		DT("pxd: outbread error %d ",outb.ftdone);
#endif /* DEBUG */
		return (ERROR);
	}
	if (Blink_msgs)
		lampoff(1);
	outb.host_len = hostlen;	/* working length remaining */
#ifdef DEBUG
	DT("ftdone %d rl %d Kl %d O %02x ",outb.ftdone,reading_len,Kill_length,Outbfound);
#endif /* DEBUG */
	if (outb.ftdone == EDONE) {
		if (reading_len) {
			if (Kill_length) {
				Kill_length = 0;
				return (minlen_save - minlen);
			} else
				return (outb.length);
		} else {
			Kill_length = 0;
			return (minlen_save - minlen);
		}
	}
	if (reading_len)
		minlen_save += 6; /* text length field */
	return (minlen_save - minlen);	/* partial read */

}


/*
**
** reread tries another read for the outb guys when they exhaust their data
*/
reread()
{
#ifdef DEBUG
	static rcount = 0;
#endif /* DEBUG */
	register char lampflag = 0;
	register long retry;

	errno = 0;
	pxk.rdp = Rbuf;
	if (!replay[1]) {
		for (retry = ZAP_COUNT, Countr = 0; retry-- && !Countr; ) {
			Countr = read(fd, Rbuf, sizeof(Rbuf));
			if (Blink_msgs) {
				if (retry % 2048 == 0) {
					lampflag = 1;
					lampon(4);
				} else if (retry % 2048 == 1024) {
					lampflag = 0;
					lampoff(4);
				}
			}
		}
	} else {
get_4:
		Countr = read(fd, Rbuf, 4);
		if (errno || (Countr == ERROR)) {
			perror("pxd: reread file error ");
#ifdef DEBUG
			DT("pxd: reread %d disk error %d ",fd,errno);
#endif /* DEBUG */
			goto show_out;
		} else if (!Countr)
			goto out_of_data;

test_4:
		if (*Rbuf == DMA_ZAP) {
			if (*(Rbuf+1) != DMA_ZAP) {	/* logread */
				retry = *(Rbuf+1);
				retry <<= 8;
				retry += *(Rbuf+2);
				retry <<= 8;
				retry += *(Rbuf+3);
				Countr = read(fd, Rbuf, retry);
				if (errno || (Countr == ERROR)) {
					printf("pxd: reread file %d count %x",fd,retry);
					perror(" ");
#ifdef DEBUG
					DT("pxd: reread %x disk error %d ",retry,errno);
#endif /* DEBUG */
					goto show_out;
				} else if (!Countr)
					goto out_of_data;
			} else if (*(Rbuf+2) != DMA_ZAP) { /* logwrite */
				Countr = read(fd, Rbuf+1, 3);
				if (errno || (Countr == ERROR)) {
					perror("pxd: reread file error ");
#ifdef DEBUG
					DT("pxd: reread %d disk error %d ",fd,errno);
#endif /* DEBUG */
					goto show_out;
				} else if (!Countr)
					goto out_of_data;
				goto test_4;
			} else {			/* logstr */
				retry = *(Rbuf+3);
				retry <<= 8;
				Countr = read(fd, Rbuf, 1);
				if (errno || (Countr == ERROR)) {
					perror("pxd: reread file error ");
#ifdef DEBUG
					DT("pxd: reread %d disk error %d ",fd,errno);
#endif /* DEBUG */
					goto show_out;
				} else if (!Countr)
					goto out_of_data;
				retry += *Rbuf;		/* scount */
				Countr = read(fd, Rbuf, retry);
				if (errno || (Countr == ERROR)) {
					printf("pxd: reread file %d count %x ",fd,retry);
					perror(" ");
#ifdef DEBUG
					DT("pxd: reread %x disk error %d ",retry,errno);
#endif /* DEBUG */
					goto show_out;
				} else if (!Countr) {
out_of_data:
					outb.ftdone = EZAP;
show_out:
					if (!Display)
						lampon(0x0f);
					return 0;
				}
				goto get_4;
			}
		} else {
			if (replay[1] && !Display)
				lampon(0x0f);
			return 0;
		}
	}
	if (Blink_msgs && lampflag)
		lampoff(4);
	if (errno || (Countr == ERROR)) {
		if (replay[1] && !Display)
			lampon(0x0f);
		perror("pxd: reread ");
		return 0;
	}
#ifdef DEBUG
	DT("@%x ",Countr);
#endif /* DEBUG */
	if (Countr) {
		Read_flag++;
 		LOGREAD
		pxk.wrp = Rbuf + Countr;
#ifdef DEBUG1
		offset += Countr + 4;
		DT("-%x ",Countr);
		if (rcount++ >= 16) {
			rcount = 0;
			DT("\n");
		}
#endif /* DEBUG1 */
		return (Countr);
	}
	outb.ftdone = ERZAP;
#ifdef DEBUG
	DT("REREAD null reads ");
#endif /* DEBUG */
	logwrite((u_char)0xb0);
	logdump();
	if (!Display)
		lampon(0x0f);
	printf("pxd:reread had %d null reads ",ZAP_COUNT);
	Msgtype = 0;
	Outbfound = 0;
	return 0;
}


send_pxd(c)
u_char c;
{
#ifdef DEBUG
	DT("Send %02x ", c);
#endif /* DEBUG */
	errno = 0;
	if (!c)
		return;
	if (!c || c == (u_char)0xa1)
		DT("we fell in with %02x ",c);
	if (!replay[4]) {
		LOGWRITE
		(void)kb_nano(c);
	} else
		return;
	if (errno) {
		perror("pxd:send_pxd error ");
		errno = 0;
	}
}


send_dly(c)
u_char c;
{
#ifdef DEBUG
	static char crcount = 6;

	DT("Send_dly %02x ", c);
	if (--crcount == 0) {
		DT("\n");
		crcount = 6;
	}
#endif /* DEBUG */
	errno = 0;
	if (!c)
		return;
	if (!c || c == (u_char)0xa1)
		DT("we fell in with %02x ",c);
	if (!replay[4]){
		LOGWRITE
		(void)kb_nano(c);
	} else
		return;
	if (errno) {
		(void)perror("pxd:send_dly error ");
		errno = 0;
	}
	pdelay(MS_15);
}

