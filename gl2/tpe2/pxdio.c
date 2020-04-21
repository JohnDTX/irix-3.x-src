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
*	send()		- Send a character to the 3274
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

#define CMD_CURHI (u_char)0xe2
#define CMD_CURLO (u_char)0xe3
#define CMD_WRITE (u_char)0xc3
#define ESPEC	(u_char)0xe0

#define LOGREAD	{ if(logflag == 1) { logread(countr);fwrite(Rbuf,1,countr,logfptr);errno=0; }  else if (logflag == 2) { logreada(countr);} }
#define LOGWRITE	{ if (logflag) { logwrite(c);} }

/*
**	Externals
*/
extern int	dflag[];
extern int	errno;
extern u_short	f13_diag();
extern char	*inputfile;
extern px_bufs 	pxl;
extern int	replay[];
extern char	*sys_errlist[];
extern long	times();
extern u_char	Blink_msgs;
extern u_char	Display_xlat[];
extern dma_buffer DMa;
#ifdef MEM
extern u_char	DMswitch;
#endif /* MEM */
extern u_char	Msg_proc;
extern u_char	Msgtype;
#ifdef MFG
extern u_char	Manuflag;
#endif /* MFG */
extern u_char	Status_flags;


/*
**	Globals
*/
int		fd;			/* pxd device file descriptor */
px_status	outb;			/* outbound ft pointers */
int		tfd;			/* pxd write file descriptor */
struct tms	*Btm;
u_char		Ft_type = 0;
long		Millibuzz = 123;	/* magic to initialize */
u_char		Outbfound = 0;		/* pxdread cent,buck with outb.ft */
u_char		Rows = ROWS;
u_char		Rbuf[PXDMASIZ];		/* driver read buffer */

/*
**	Local variables
*/
char ident[] = "@(#) Pxd 3279 Version 1.7, GL2 WS";
long		countr;			/* pxdread actual read count */
u_char		curr_status;
px_buf_t	pxk;			/* read buffer pointers */
u_char		logflag = 0;
FILE		*logfptr;
px_buf_t	pxin;
u_char		Ibuf[PXDMASIZ];
FILE		*ilf = 0;
u_char		loginum = 0;
long		offset = 0;
u_char		buf6[8];
u_char	dtext[] = {"/dev/pxd"};
#ifdef MEM
u_char	mtext[] = {"/dev/pxm"};
#endif /* MEM */


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
	if (!replay[4]) {
		force_open_init();
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
		get_buzz();
	if (!replay[4]) {
#ifdef MEM
		if (DMswitch)
			p = mtext;
		else
#endif /* MEM */
			p = dtext;
#ifdef MFG
		if (Manuflag) {
			printf("Doing minimal open\n");
			if ((fd = open(p, O_NDELAY)) <= 0) {
				perror("pxdopen error ");
				return (ERROR);
			}
			tfd = fd;
			return (38);
		}
#endif /*  MFG */
#ifdef DEBUG
		DT("Full %s open\n",p);
#endif /* DEBUG */
		if ((fd = open(p, O_RDWR)) <= 0) {
			perror("pxdopen error ");
			return (ERROR);
		}
		tfd = fd;
		kill_all();
		Status_flags = 0;
		DMa.d_cnt = 0;
		i = f13_diag();
		pdelay(MS_100);
		if (i < 38)
			printf("pxd:diag returned %d instead of 38 ",i);
		send_x_key(X_RESET);
		db_conopen();
		(void)update3270();
		get_screen();
	} else {
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
	}
	return (i);
}


/*
 * Pxdread moves data from the kernel to Rbuf
 */
long
pxdread(dest, destsize)
u_char *dest;
long destsize;
{
	register u_char c1, c2, ft_enabled;
	register long r1, r2;
	register u_char *kwrp, *rdp, *wrp;
	extern char eat_3274();

	rdp = pxk.rdp;
	if (Outbfound) {
		outb.ftdone = ESTART;
		return (ERROR);
	}

	if ((pxk.wrp - pxk.rdp) <= 0) {	/* any previously read data? */
		errno = 0;
		if (!replay[4])
			countr = read(fd, Rbuf, PXDMASIZ);
		else
			countr = reread();
		pxk.rdp = Rbuf;
		pxk.wrp = Rbuf + countr;
		if (!countr)
			return 0;
		if (errno) {
			printf("pxd: pxdread error %d ",errno);
			return (ERROR);
		}
 		LOGREAD
	}
	kwrp = pxk.wrp;			/* pronounced kwrap */
	rdp = pxk.rdp;
	wrp = dest;
	r1 = 0;				/* read byte counter */
	r2 = ((kwrp - rdp) < destsize) ? (kwrp - rdp) : destsize;
	c1 = CENT;
	ft_enabled = outb.ft;
/*
 * term reads character from Rbuf 
 */
dmaread:
	c2 = *rdp++;
	if (c2 == c1 && ft_enabled)
		if (*(rdp - 2) == CMD_WRITE || rdp == Rbuf + 1)
			goto rxfread;
rbufwrite:
	*wrp++ = c2;
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
	*wrp++ = c1;		/* store cent sign */
	r1++;
	r2--;
	if (rdp >= kwrp) {
		reread();
		kwrp = pxk.wrp;
		rdp = Rbuf;
		r2 = ((kwrp - rdp) < (destsize - r1)) ? (kwrp - rdp) : 
			(destsize - r1);
	}
	c2 = *rdp++;
	if (c2 == END_WRITE) {
		pxk.rdp = rdp;
		if (!(eat_3274())) {
			rdp = pxk.rdp;
			kwrp = pxk.wrp;
			r2 = ((kwrp - rdp) < (destsize - r1)) ? (kwrp - rdp) : 
				(destsize - r1);
			c2 = *rdp++;
			goto rbufwrite;
		}
		rdp = pxk.rdp;
		kwrp = pxk.wrp;
		r2 = ((kwrp - rdp) < (destsize - r1)) ? (kwrp - rdp) : 
			(destsize - r1);
		c2 = *rdp++;
	}
	if (c2 == (u_char)0x91)		/* cent r */
		if (r2 > 2)
			r2 = 2;		/* hasty exit after END_WRITE */
	if ((c2 != DATA2RXFER) && (c2 != DATA3RXFER) && (c2 != TEXT2RXFER)
		&& (c2 != TEXT3RXFER))
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
		if (outb.ftdone)
			printf("pxd:kill_outb with ftdone = %d",outb.ftdone);
		Outbfound = outb.ft = outb.host_len = 0;
		break;
	case 16:		/* contents of block is */
		/*   addr   u_char *bufp    = store body at */
		/*   addr+4 long    max_len = limit of body length */

		outb.host_len = 0;
	case 17:
		outb.bufp = (u_char *)*addr;
		addr++;
		outb.max_len = *addr;
		outb.wrp = outb.bufp;
		Ft_type = outb.ft = MXFER;
 		outb.r1save = 0;	/* 4:3 receipent */
 		outb.r2save = 3;	/* packing counter */
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
			reread();
			rdp = Rbuf;
		}
		c = *rdp++;		/* eat trailing stuff */
		if (c == CENT) {
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
 * eat_3274 removes the extraneous 3274 commands from file transfers.
 *   called after read of END_WRITE and rdp at next position.
 *   read next bytes to see if write continues ;
 *   put in local buffer(buf6), then test position for not in status line.
 *	   if not in status line, outb continues.
 *	   if in status line, rxfr is done.
 * the usual sequences are:
 * e6 ca 14 ca 04 e2 xx e3 yy c3 after each 256 byte segment
 *             e6 e2 00 e3 50 c3 after writing 1920 bytes, point home
 * e6 ca 14 ca 04 e2 00 e3 50 c3 1b 90 e6 for writing req ack in a file
 *
 *	returns	1 - still in file xfer, pxk.rdp after CMD_WRITE
 *		0 - rxfer done
 *		pxk.rdp at position of CMD_WRITE
 */
char
eat_3274()
{
	register long rereadctr;
	register u_char c1, c2 = 0;
	register u_char *rdp, *kwrp;

	rdp = pxk.rdp;
	kwrp = pxk.wrp;
	c1 = CMD_CURHI;
try5:
	if (c2 == CMD_WRITE) {
		pxk.rdp = rdp;
		return CNTU;
	}
	if (rdp >= kwrp) {
		rereadctr = reread();
		kwrp = pxk.wrp;
		rdp = Rbuf;
	}
	c2 = *rdp++;
try5a:
	if (c2 != c1)
		goto try5;	/* probably load control reg command, data */
	buf6[1] = c2;			/* e2, cursor high */

	if (rdp >= kwrp) {
		rereadctr = reread();
		kwrp = pxk.wrp;
		rdp = Rbuf;
	}
	c2 = *rdp++;
	if (c2 == c1)
		goto try5a;	/* was argument of cmd_cursor last time */
	buf6[2] = c2;			/* xx */

	if (rdp >= kwrp) {
		rereadctr = reread();
		kwrp = pxk.wrp;
		rdp = Rbuf;
	}
	c2 = *rdp++;
	if (c2 != CMD_CURLO)
		goto try5a;
	buf6[3] = c2;			/* e3, cursor low */

	if (rdp >= kwrp) {
		rereadctr = reread();
		kwrp = pxk.wrp;
		rdp = Rbuf;
	}
	c2 = *rdp++;
	buf6[4] = c2;			/* yy */

 	if (c2 == ESPEC) {		/* 3274 special character */
 		if (rdp >= kwrp) {
			rereadctr = reread();
 			kwrp = pxk.wrp;
 			rdp = Rbuf;
 		}
 		c2 = *rdp++;		/* ff or 00 */
 		c2 = 86;		/* why not ??? */
 		buf6[4] = c2;
 
	}

	if (rdp >= kwrp) {
		rereadctr = reread();
		kwrp = pxk.wrp;
		rdp = Rbuf;
	}
	c2 = *rdp++;
	buf6[5] = c2;			/* c3, CMD_WRITE */

	pxk.rdp = rdp;
	if (buf6[5] != CMD_WRITE)
		goto try5a;
	if (buf6[2] != 0)
		return (CNTU);		/* still pumping */
	if (buf6[4] >= COLS) {
		if (buf6[4] != COLS)
			return (CNTU);	/* still pumping */
		else {
			if (rdp >= kwrp) {
				rereadctr = reread();
				c2 = *pxk.rdp;
				if (c2 != CENT)
					return (CNTU);	/* still pumping */
#ifdef DEBUG
				DT("Cent start %d ",rereadctr);
#endif /* DEBUG */
				kwrp = pxk.wrp;		/* after last char */
				rdp = kwrp - 1;		/* last valid char */
				while (rereadctr--)	/* move up 1 */
					*kwrp-- = *rdp--;
				*++rdp = CMD_WRITE;	/* for emulint */
				pxk.rdp = rdp + 1;
			} else {
				c2 = *pxk.rdp;
				if (c2 != CENT)
					return (CNTU);	/* still pumping */
			}
		}
	}
	/* leave write cmd in buffer for next read to return */
	/* The termination cases are:
	*	CMD_WRITE,CENT found
	*	CMD_WRITE to buf6[4] < 80 (potential error)
	*/
	pxk.rdp--;			/* to buf6[5] */
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

	Btm = &tmp;
	if ((now = prev = times(Btm)) <= 0)
		perror("pxd:get_buzz ");
	while (now == prev)		/* sync with times */
		if ((now = times(Btm)) <= 0)
			perror("pxd:get_buzz ");
	prev = now;
	i = 0;
	while (now == prev) {		/* count 16.67 msec */
		for (tcount = 256 ; tcount--; )
			;
		if ((now = times(Btm)) <= 0)
			perror("pxd:get_buzz ");
		i++;
	}
	prev = now;
	Millibuzz = i * 256;		/* temp, real # is >> 4 */
	i = 0;
	while (now < prev + 8) {	/* count 8 * 16.67 msec */
		for (tcount =  Millibuzz ; tcount--; )
			;
		if ((now = times(Btm)) <= 0)
			perror("pxd:get_buzz ");
		i++;
	}
	Millibuzz = (Millibuzz >> 7) * i;
#ifdef juniper
	if (Millibuzz < 1200)
		Millibuzz = 1200;
#endif /* juniper */
#ifdef iris
	if (Millibuzz < 200)
		Millibuzz = 200;
#endif /* iris */
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
		if ((logfptr = fopen("RAWINP", "w")) == NULL)
			oops("t3279: can't open 'RAWINP'\n\r");
		
		setbuf(logfptr,0);	/* no buffering */
	} else if (flag == 3)
		pxin.wrp = Ibuf;
}


logread(r1)
{
	u_char bufr[4];
 
	if (logflag != 1)
		return;
 	bufr[0] = DMA_ZAP;
 	bufr[1] = (u_char)(r1 >> 16);
 	bufr[2] = (u_char)(r1 >> 8);
 	bufr[3] = (u_char)r1;
	fwrite(bufr, 1, 4, logfptr);
	errno = 0;
}
 

logreada(r1)
{
	register u_char *rdp, *wrp, *kwrp;
 
	if (logflag != 2)
		return;
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
	if ((ilf = fopen(bufr, "w")) == NULL)
	    oops("t3279: can't open %s\n\r", bufr);
	setbuf(ilf,0);	/* no buffering */
	ic = sizeof(Ibuf) - (pxin.wrp - Ibuf);
	if (ic)
		fwrite(Ibuf, 1, ic, ilf);
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
	register u_char c1,c2;
	register u_char *dispxlate, *kwrp, *rdp, *wrp;
	register long minlen, r1, r2;
	char reading_len = 1, z2;
	long hostlen, minlen_save;

/*
 * prepare for outb ft as we have CMD_WRITE, cent, (buck, c, t, m)
 */
	if (Blink_msgs)
		lampon(1);
	rdp = pxk.rdp;
	if (!outb.host_len) {
		if (Outbfound != DATA2RXFER && Outbfound != TEXT2RXFER) {
			outb.ftdone = ECNTU+1;
			goto clearout;
		}
		readlen();		/* first frame includes length */
 		if (outb.host_len == 0) {
 			eat_trailing();
 			rdp = pxk.rdp;
 			goto clearout;	/* part of rays 12 byte trailer */
 		}
		if (Outbfound == DATA2RXFER)
			hostlen = outb.host_len - 3;
		else
			hostlen = outb.host_len - 6;
		r1 = 0;			/* 4:3 receipent */
		r2 = 3;			/* packing counter */
		wrp = outb.bufp; 
	} else {			/* continuation frame */
		reading_len = 0;
		if (Outbfound != DATA3RXFER && Outbfound != TEXT3RXFER) {
			outb.ftdone = ECNTU;
			goto clearout;
		}
		hostlen = outb.host_len;/* working length */
		r1 = outb.r1save;
		r2 = outb.r2save;
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
	outb.ftdone = EACK;		/* default for ft */
	minlen = (hostlen < outb.max_len) ? hostlen : outb.max_len;
	minlen_save = minlen;
	c1 = END_WRITE;
	dispxlate = ftxlat;
	if (Outbfound == DATA2RXFER || Outbfound == DATA3RXFER)
		goto rxfr;
	dispxlate = Display_xlat;
	goto txtfer;
/*
 * read next bytes to see if text continues ;
 */
try3:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2)
		goto txtfer;

#ifdef DEBUG
	DT("hostlen %d minlen_save %d minlen %d\n",hostlen,minlen_save,minlen);
#endif /* DEBUG */
	hostlen -= minlen_save - minlen;
	if (hostlen <= 0 || hostlen == -minlen) {
		hostlen = 0;
		outb.ft = 0;
		outb.ftdone = EDONE;
#ifdef DEBUG
		if (r1 || r2 != 3)
			DT("forcing %02x to 0, %02x to 3 in try4\n",r1,r2);
#endif /* DEBUG */
		outb.r1save = 0;
		outb.r2save = 3;
	}
	goto done_rxfer;

/*
 *   main text loop must be fast so change carefully and check adb for speed
 */
txtfer:
	if (rdp >= kwrp) {
		reread();
		kwrp = pxk.wrp;
		rdp = Rbuf;
	}
	c2 = *rdp++;
	if (c2 == c1)
		goto try3;

	*wrp++ = *(dispxlate + c2);
	if (--minlen > 0)
		goto txtfer;

	goto done_screen;
/*
 * read next bytes to see if write continues ;
 */
try4:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (z2)
		goto rxfr;

#ifdef DEBUG
	DT("hostlen %d minlen_save %d minlen %d\n",hostlen,minlen_save,minlen);
#endif /* DEBUG */
	hostlen -= minlen_save - minlen;
	if (hostlen <= 0 || hostlen == -minlen) {
		hostlen = 0;
		outb.ft = 0;
		outb.ftdone = EDONE;
		outb.r1save = 0;
		outb.r2save = 3;
	} else {
		outb.r1save = r1;
		outb.r2save = r2;
#ifdef DEBUG
		if (r1 || r2 != 3)
			DT("forcing %02x to 0, %02x to 3 in try4\n",r1,r2);
#endif /* DEBUG */
		outb.r1save = 0;
		outb.r2save = 3;
	}
	goto done_rxfer;

/*
 *   main outb loop must be fast so change carefully and check adb for speed
 */
rxfr:
	if (rdp >= kwrp) {
		reread();
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
	r1 = 0;
	r2 = 3;
	minlen -= 3;
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
	if (outb.ftdone == EDONE)
		if (reading_len)
			return (outb.length);
		else
			return (minlen_save - minlen);
	if (reading_len)
		if (Outbfound == DATA2RXFER)
			minlen_save += 3; /* bin length field */
		else
			minlen_save += 6; /* text length field */
	return (minlen_save - minlen);	/* partial read */

}


/*
** delay t msecs
*/
pdelay(t)
long t;
{
	register long u;

	u = Millibuzz * t;
	while(u--)
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
** read outb length which is packed 4:3 coded for binary or
**   six 3274 characters for text
*/
readlen()
{
	register u_char c1,c2;
	register long r1, r2;
	register u_char *dispxlate, *rdp, *wrp, *kwrp;
	char z2;

	outb.wrp = outb.bufp;
	rdp = pxk.rdp;
	wrp = outb.wrp;
	kwrp = pxk.wrp;
	r1 = 0;
	r2 = 3;
	c1 = END_WRITE;
	dispxlate = ftxlat;
	if (Outbfound == DATA2RXFER)
		goto rxfr;		/* binary outb */
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
		reread();
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

	goto loadlength;

/*
 * read next bytes to see if write continues ;
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
		reread();
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

loadlength:
	r2 = r1 >> 8;
	*wrp++ = r2 >> 8;
	*wrp++ = r2;
	*wrp++ = r1;

done_rxfer:
	outb.length = outb.host_len = r1;
	outb.wrp = wrp;
	pxk.rdp = rdp;
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
	long prev_countr;

	errno = 0;
	pxk.rdp = Rbuf;
	prev_countr = countr;
	if (!replay[1]) {
		for (retry = ZAP_COUNT, countr = 0; retry-- && !countr; ) {
			countr = read(fd, Rbuf, PXDMASIZ);
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
		countr = read(fd, Rbuf, 4);
		if (errno || (countr == ERROR)) {
			printf("pxd: reread disk error %d ",errno);
			lampon(0x0f);
			return 0 ;
		}
		if (*Rbuf == DMA_ZAP) {
			retry = *(Rbuf+1);
			retry <<= 8;
			retry += *(Rbuf+2);
			retry <<= 8;
			retry += *(Rbuf+3);
			countr = read(fd, Rbuf, retry);
		} else {
			if (replay[1])
				lampon(0x0f);
			return 0;
		}
	}
	if (Blink_msgs && lampflag)
		lampoff(4);
	if (errno || (countr == ERROR)) {
		if (replay[1])
			lampon(0x0f);
		printf("pxd: reread error %d ",errno);
		return 0;
	}
	if (countr) {
 		LOGREAD
		pxk.wrp = Rbuf + countr;
#ifdef DEBUG
		offset += countr + 4;
		DT("-%x ",countr);
		if (rcount++ >= 16) {
			rcount = 0;
			DT("\n");
		}
#endif /* DEBUG */
		return (countr);
	}
	outb.ftdone = ERZAP;
#ifdef DEBUG
	DT("REREAD null reads ");
#endif /* DEBUG */
	logwrite((u_char)0xb0);
	logdump();
	lampon(0x0f);
	if (*(Rbuf + prev_countr - 3) == (u_char)0xc6)
		printf("PROBABLE MAINFRAME CHANNEL ERROR, got CLEAR from 3274 during a channel write\n");
	else
		printf("pxd:reread had %d null reads ",ZAP_COUNT);
	Msgtype = 0;
	Outbfound = 0;
	send_x_key(X_CLEAR);	/* bye bye birdie */
	return 0;
}


send(c)
u_char c;
{
#ifdef DEBUG
	DT("Send %02x ", c);
#endif /* DEBUG */
	errno = 0;
	if (!c)
		return;
	if (!replay[4]) {
		LOGWRITE
		(void)kb_nano(c);
	}
	if (errno) {
		perror("pxd:send error ");
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
	if (!replay[4]){
		LOGWRITE
		(void)kb_nano(c);
	}
	if (errno) {
		perror("pxd:send_dly error ");
		errno = 0;
	}
	pdelay(MS_15);
}

