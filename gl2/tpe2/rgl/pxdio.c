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

/************************* P X D I O . C **********************************
*
*  MODULE DESCRIPTION:
*	I/O interface routines to the PCOX card
*
*  ENTRY POINTS:
*	delay()		- Provide a loop delay of n milliseconds
*	get_screen()	- Force PCOX card to give new screen
*	initpxd()	- Initialize the pxd device, called by pxdopen
*	ioctl()		- Query/Modify replacement for UNIX ioctl calls
*	outbread()	- Read an outbound message
*	pxdioctl()	- Query/status of outb ft
*	pxdclose()	- Close the pxd device
*	pxdopen()	- Open the pxd device
*	pxstat()	- Return TRUE if new data available from the 3274
*	send()		- Send a character to the 3274
*	send_dly()	- Send a character to the 3274 and delay 15 msecs
*
*	4/9/85 baseline for boot and rgl versions
************************************************************************/

#include "gl.h"
#include "term.h"
#include "hostio.h"
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#ifdef GL2TERM
#include "/usr/srcgl2/vkernel/kernel/h/dm.h"
#else
#include "../../iris/src/V/iokernel/h/dm.h"
#endif GL2TERM
#include <Vioprotocl.h>
#include <Vio.h>
#include <Vdevtypes.h>
#include <Venviron.h>
#include <Vserial.h>
#include "rpc.h"
#include "grioctl.h"
#include "pxw.h"


#define CMD_CURHI 0xe2
#define CMD_CURLO 0xe3
#define CMD_WRITE 0xc3
#ifndef DEBUG
#define DEBUG
#endif
#define ZAP_COUNT 15000


/*
**	Externals
*/
extern int	close();
extern void	free();
extern u_short	f13_diag();
extern char	*getenv();
extern char	*gets();
extern		ioctl();
extern 		printf();
extern		print_dma();
extern int 	pxdclose();
extern InstanceId	pxdfid;
extern int	pxdopen();
extern		reload();
extern int	send_ebx_str();
extern int 	start, end;
extern 		usropen();
extern dma_buffer DMa;
extern char	File_xfer;
extern u_char	Status_flags;


/*
**	Globals
*/
int		errno, Lerrno;
InstanceId	fid;			/* pxd device file descriptor */
char		File_xfer = 0;
char		Ft_type = 0;
long		Nbytecount = PXDMASIZ-1;
long		Old_time = 0;
u_char		Outbfound = 0;		/* pxdread cent,buck with outb.ft */
u_char		Pxd_debug = 0;
u_char		Rbuf[PXDMASIZ];		/* driver read buffer */
char		Rows = ROWS;


/* 
** IBM3270 Modify Query Message Format.
 */

typedef struct {
	SystemCode		requestcode;
	InstanceId 		fileid;
	unsigned 		mqreqcode;
	unsigned		arg;
	unsigned char		junk[18];
}PXDMQRequest ;


/*
**	Locals
*/
u_char		b6[6];
long		countq,countr;	/* pxdread actual read count */
getput		*fst;
char		mfbuf[80];
px_status	outb;		/* outbound ft pointers */
px_buf_t	pxk;		/* read buffer pointers */
unsigned	retry = 1500;
PXDMQRequest	rq1;
char		s[80];
Process_id	serverid;


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
	kill_outb();
	send_x_key(X_RESET);
	return(closepxd());
}


/*
**	Open the pxd device
*/
pxdopen()
{
	register char i = 38;
	getput fsst;

	if (initpxd() != OK)
		return (ERROR);
	kill_outb();
	Status_flags = 0;
	DMa.d_cnt = 0;
	(void)read_open_init(&fsst);
	if (!fsst.mbchar) {
		(void)update3270();
		i = f13_diag();
		delay(MS_100);
		if (i != 38)
			messagef("f13 returned %d ",i);
	}
	send_x_key(X_RESET);
	(void)update3270();
	get_screen();
	db_conopen();
	File_xfer = 1;
	return (i);
}


/*
 * Pxdread moves data from the kernel to Rbuf and then to Sbuf
 */
long
pxdread(dest, destsize)
u_char *dest;
long destsize;
{
	register u_char c1,c2;
	register long minlen, r1, r2;
	long hostlen, z2;
	register u_char *kwrp, *rdp, *wrp;
	long minlen_save;

	rdp = pxk.rdp;
	if (Outbfound) {
		outb.ftdone = ESTART;
		return (ERROR);
	}

	if ((pxk.wrp - pxk.rdp) <= 0) {	/* any previously read data? */
		pxk.rdp = Rbuf;		/* No, read data buffer */
 		if (countr)
 			countq = countr;
		countr = readpxd();
		pxk.wrp = Rbuf + countr;
		if (!countr)
			return 0;
		if (errno || (countr == ERROR)) {
			return ERROR;
		}
	}
	kwrp = pxk.wrp;			/* pronounced kwrap */
	rdp = pxk.rdp;
	wrp = dest;
 	r1 = 0;				/* read byte counter */
	r2 = ((kwrp - rdp) < destsize) ? (kwrp - rdp) : destsize;
/*
 * term reads character from Rbuf 
 */
dmaread:
	c2 = *rdp++;
	if (c2 == (u_char)DATA1RXFER) {
		if (*(rdp - 2) == (u_char)CMD_WRITE && outb.ft)
			goto rxfread;
		if ((Rbuf + 1) == rdp && outb.ft)
			goto rxfread;	/* assume CMD_WRITE ended prev read */
	}
rbufwrite:
	*wrp++ = c2;
	r1++;
	if (--r2)
		goto dmaread;

termout:
	pxk.rdp = rdp;
	return(r1);
/*
 * outb ft test for initialization because either:
 *	CMD_WRITE, cent and outb is enabled, or
 *	top of Rbuf, cent and outb is enabled
 */
rxfread:
	*wrp++ = (u_char)DATA1RXFER;		/* store cent sign */
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
	if ((c2 != (u_char)DATA2RXFER) && (c2 != (u_char)DATA3RXFER))
		goto rbufwrite;
	
	if (r1-- != 1)
		r1--;			/* CMD_WRITE, cent removed */
	Outbfound = 1;
	goto termout;
}


/*
 * pxdioctl does writes to signal and kbdata ports of px
 * the kb and signal routines are longs but use only a char
 * op	subroutine
 * 10	get_outb_status	read outb ft status
 * 12	kill_outb	disable outb ft
 * 16	set_rglout_ptr store outb ft address, max length
 */
/*VARARGS*/
pxdioctl(dev, cmd, addr)
dev_t dev;
long *addr;
{

	switch(cmd) {
	
	case	10:
		blt(addr,&outb,sizeof(outb));
		break;
	case	12:
		outb.ft = 0;
		break;
	case	16:		/* contents of block is */
		/*   addr   u_char *bufp    = store body at */
		/*   addr+4 long    max_len = limit of body length */

		outb.bufp = (u_char *)*addr;
		addr++;
		outb.max_len = *addr;
		outb.wrp = outb.headrp;
		Ft_type = outb.ft = FT_RGL;
		outb.ftdone = outb.host_len = 0;
 		outb.r1save = 0;	/* 4:3 receipent */
 		outb.r2save = 3;	/* packing counter */
		break;
	default:
		ringbell();
	}
}


/*
** delay t msecs
*/
delay(t)
register  t;
{
	register long u;

#ifdef M68020
	u = t * 8192;
#else
	u = t * 1024;
#endif M68020
	while(u--)
		;
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
	for (c = 0; c != (u_char)END_WRITE && count; count-- ) {
		if (rdp >= pxk.wrp) {
			reread();
			rdp = Rbuf;
		}
		c = *rdp++;		/* eat trailing stuff */
		if (c == (u_char)DATA1RXFER) {
			rdp--;
			messagef("pxd:cent trailing\n");
			break;
		}
	}
	pxk.rdp = --rdp;
}


/*
 * eat_3274 removes the extraneous 3274 commands from file transfers.
 *   called after read of END_WRITE 
 *   read next bytes to see if write continues ;
 *   put in local buffer(b6), then test position for not in status line.
 *	   if not in status line, outb continues.
 *	   if in status line, rxfr is done, copy b6 to prev wrp.
 * the usual sequences are:
 * e6 ca 14 ca 04 e2 xx e3 yy c3 after each 256 byte segment
 *             e6 e2 00 e3 50 c3 after writing 1920 bytes, point home
 * e6 ca 14 ca 04 e2 00 e3 50 c3 1b 90 e6 for writing req ack in a file
 *
 *	returns	1 - still in file xfer
 *		0 - rxfer done
 */
eat_3274()
{
	register u_char c2;
	register u_char *rdp, *kwrp;

	rdp = pxk.rdp;
	kwrp = pxk.wrp;
try5:
	if (rdp >= kwrp) {
		reread();
		kwrp = pxk.wrp;
		rdp = Rbuf;
	}
	c2 = *rdp++;
try5a:
	if (c2 != (u_char)CMD_CURHI)
		goto try5;	/* probably load control reg command, data */
	b6[1] = c2;			/* e2, cursor high */

	if (rdp >= kwrp) {
		reread();
		kwrp = pxk.wrp;
		rdp = Rbuf;
	}
	c2 = *rdp++;
	if (c2 == (u_char)CMD_CURHI)
		goto try5a;	/* was argument of cmd_cursor last time */
	b6[2] = c2;			/* xx */

	if (rdp >= kwrp) {
		reread();
		kwrp = pxk.wrp;
		rdp = Rbuf;
	}
	c2 = *rdp++;
	if (c2 != (u_char)CMD_CURLO)
		goto try5a;
	b6[3] = c2;			/* e3, cursor low */

	if (rdp >= kwrp) {
		reread();
		kwrp = pxk.wrp;
		rdp = Rbuf;
	}
	c2 = *rdp++;
	b6[4] = c2;			/* yy */

	if (rdp >= kwrp) {
		reread();
		kwrp = pxk.wrp;
		rdp = Rbuf;
	}
	c2 = *rdp++;
	b6[5] = c2;			/* c3, CMD_WRITE */

	pxk.rdp = rdp;
	if (b6[5] == (u_char)0xce)
		goto tdone;
	if (b6[2] != 0) {
		if (b6[5] != (u_char)CMD_WRITE)
			goto try5a;
		if (*rdp != (u_char)DATA1RXFER)
			return(1);		/* still pumping */
		goto tdone;
	}
	if (b6[4] >= (u_char)80) {
		if (b6[4] != (u_char)80)
			return(1);		/* still pumping */
		else {
			if (b6[5] != (u_char)CMD_WRITE)
				goto try5a;
			if (*rdp != (u_char)DATA1RXFER)
				return(1);	/* still pumping */
		}
	}
	/* leave write cmd in buffer for next read to return */
	/* The termination cases are:
	*	b6[5] = ce (status update)
	*	CMD_WRITE,DATA1RXFER found (potential error discovered)
	*		eathi if write in screen (error)
	*		eatho if write at home (cent-q or error)
	*	CMD_WRITE to b6[4] < 80 (status update)
	*/
tdone:
	pxk.rdp--;				/* to b6[5] */
	return(0);
}


/*
**	Force the PCOX card to send a fresh panel
*/
get_it()
{
	(void)raise1();
	delay(MS_100);
	(void)reset_signal(SIG1);
	delay(MS_100);
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


/*
 *	Outbread reads the outb message into
 *   the rxfer address of the set_outb_ptr ioctl request.
 *   This is a forked read, with file transfers beginning with
 *   cent sign, dollar sign. The 16 byte header is stored at
 *   one location, and the body is appended at another, allowing
 *   applications to have a large body built of many segments
 */
long
outbread()
{
	register u_char c1,c2;
	register long minlen, r1, r2;
	long hostlen, z2;
	register u_char *kwrp, *rdp, *wrp;
	long minlen_save;

/*
 * prepare for outb ft as we have CMD_WRITE, cent, buck, and enable
 */
	rdp = pxk.rdp;
	if (!outb.host_len) {
		readlen();		/* first frame includes length */
		rdp = pxk.rdp;
		hostlen = outb.host_len - 3;
		kwrp = pxk.wrp;
 		r1 = 0;			/* 4:3 receipent */
 		r2 = 3;			/* packing counter */
		wrp = outb.bufp; 
	} else {			/* continuation frame */
		hostlen = outb.host_len;/* working length */
		kwrp = pxk.wrp;
 		r1 = outb.r1save;
 		r2 = outb.r2save;
		wrp = outb.wrp;
	}
	outb.ftdone = EACK;		/* default for ft */
	minlen = (hostlen < outb.max_len) ? hostlen : outb.max_len;
	minlen_save = minlen;
	if (hostlen <= 0) {
		outb.ftdone = ELENGTHBAD;
		goto clearout;		/* host said neg bytes */
	}
	goto rxfr;

/*
 * read next bytes to see if write continues ;
 */
try4:
	pxk.rdp = rdp;
	z2 = eat_3274();
	kwrp = pxk.wrp;
	rdp = pxk.rdp;
	if (!z2) {
		hostlen -= minlen_save - minlen;
		if (hostlen <= 0) {
			hostlen = 0;
			outb.ft = 0;
			outb.ftdone = EDONE;
 			outb.r1save = 0;
 			outb.r2save = 3;
 		} else {
 			outb.r1save = r1;
 			outb.r2save = r2;
		}
		goto done_rxfer;
	}

/*
 *   main outb loop must be fast so change carefully and check adb
 */
rxfr:
	if (rdp >= kwrp) {
		reread();
		kwrp = pxk.wrp;
		rdp = Rbuf;
	}
	c2 = *rdp++;
	if (c2 == (u_char)END_WRITE)
		goto try4;
	r1 = r1 << 6;
	r1 = r1 | ((unsigned)ftxlat[c2] & 0x3f);
	if (r2--)
		goto rxfr;

	r2 = r1 >> 8;
	c1 = (u_char)((r2 >> 8) & 0xff);
	*wrp++ = c1;
	c1 = (u_char)(r2 & 0xff);
	*wrp++ = c1;
	c1 = (u_char)(r1 & 0xff);
	*wrp++ = c1;
	r1 = 0;
	r2 = 3;
	minlen -= 3;
	if (minlen > 0)
		goto rxfr;
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
 * copyout Wbuf remainder to ioctl address.
 * clear xfer flags, save regs, return
 */
done_rxfer:
	outb.wrp = wrp;
	if ((outb.ftdone != EDONE) && (outb.ftdone != EACK)) { 

clearout:
		outb.ft = outb.host_len = 0;
		Outbfound = 0;
		pxk.rdp = rdp;
		return ERROR;
	}
	outb.host_len = hostlen;	/* working length remaining */
	Outbfound = 0;
	pxk.rdp = rdp;
	if (outb.ftdone == EDONE)
		return (outb.length);
	return (ERROR);
}

printRbuf(rdpp,count)
u_char *rdpp;
long count;
{
	register u_char *bufp, *wrp;
	register long i;

	printf("\n\tRbuf");	
	wrp = bufp = rdpp;
	wrp += count;
	i = 23;
	while ( bufp < wrp ) {
		i++;
		if (i%8 == 0)
			(void)printf(" ");
		if (i%24 == 0)
			(void)printf("\n\t");
		(void)printf("%02x ", *bufp++);
	}
}


/*
**	Return YES if PCOX dma address moved since last call
*/
pxstat()
{

	long curr_data = 0;

	if (pxk.wrp - pxk.rdp > 0) {
		return 1;
	}
	if ((read_avail(&curr_data)) == ERROR)
		return 0;
	return (curr_data);
}


/*
** read outb length which is 4:3 coded
*/
readlen()
{
	register u_char c1,c2;
	register long r1, r2, r3;
	register u_char *rdp, *wrp, *kwrp;
	long z2;
	long r1temp;

	outb.wrp = outb.headrp;
	outb.wrp = outb.bufp;
	rdp = pxk.rdp;
	wrp = outb.wrp;
	kwrp = pxk.wrp;
	r1 = 0;
	r2 = 3;
/**/
/*** now read first four message bytes to go into length ***/
/**/
	goto rxfr;
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
		/*** partial return ***/
		outb.ftdone = ERZAP;
		goto done_rxfer;
	}
rxfr:
	if (rdp >= kwrp) {
		reread();
		kwrp = pxk.wrp;
		rdp = Rbuf;
	}
	c2 = *rdp++;
	if (c2 == (u_char)END_WRITE)
		goto try4;
	r1 = r1 << 6;
	r1 = r1 | ((unsigned)ftxlat[c2] & 0x3f);
	if (r2) {
		r2--;
		goto rxfr;

	} else {
		r1temp = r1 & 0xffffff;
		r2 = r1 >> 8;
		c1 = (u_char)((r2 >> 8) & 0xff);
		*wrp++ = c1;
		c1 = (u_char)(r2 & 0xff);
		*wrp++ = c1;
		c1 = (u_char)(r1 & 0xff);
		*wrp++ = c1;
		r1 = 0;
		r2 = 3;
	}
done_rxfer:
	outb.length = outb.host_len = r1temp;
	outb.wrp = wrp;
	pxk.rdp = rdp;
}


/*
**
** reread tries another read for the outb guys when they exhaust their data
*/
reread()
{
	register long retry;

	countq = countr;
	for (retry=ZAP_COUNT,countr=0; retry-- && !countr; )
		countr = readpxd();
	if (errno || (countr == ERROR)) {
		messagef("reread: error; errno %d",errno);
		return 0 ;
	}
	if (countr) {
		pxk.wrp = Rbuf + countr;
		return (countr);
	}
	outb.ftdone = ERZAP;
	messagef("reread:150000 bad reads after %x",countq);
	return 0;
}


send(c)
u_char c;
{

	if (!c)
		return;
	(void)kb_nano(c);
}


send_dly(c)
u_char c;
{

	if (!c)
		return;
	(void)kb_nano(c);
	delay(MS_15);
}


/*
**   aborts or stops causing dump if possible
*/
stopnow()
{
#ifndef GL2TERM
	abort();
#else
	messagef("blew it, am stopping ");
#endif GL2TERM
}


closepxd()
{
	Process_id serverid;
	union message {
		IoRequest  	req;
		IoReply  	reply;
	} msg;
	union message *mp;

	mp = &msg;
	serverid = GetPid(DEVICE_SERVER,LOCAL_PID);

	mp->req.requestcode = RELEASE_INSTANCE;
	mp->req.fileid = fid;

	if ( Send(mp,serverid) == 0 ) {
		messagef("pxdclose Send failure ");
		return(ERROR);
	}

	if ( mp->reply.replycode) {
		messagef("pxdclose replycode failure %x",
			mp->reply.replycode);
		return(ERROR);
	}
	return(OK);
}


initpxd()
{
	union message {
		CreateInstanceRequest	req;
		CreateInstanceReply	reply;
	} msg;
	union message *mp;

	mp = &msg;

	mp->req.requestcode = CREATE_INSTANCE;
	mp->req.type = IBMPXD;
	mp->req.filemode = FCREATE;
	mp->req.filename = "shit";	/* need a valid ptr	*/

	serverid = GetPid(DEVICE_SERVER,LOCAL_PID);

	if ( Send(mp,serverid) == 0 ) {
		messagef("create Send returned 0");	/*** DEBUG ***/
		return(ERROR);
	}

	if ( mp->reply.replycode != OK ) {
		messagef("create Replycode %x not OK",mp->reply.replycode);
		return(ERROR);
	}	/*** DEBUG ***/

	fid = mp->reply.fileid;
	initserial();

	return(OK);
}


/*VARARGS*/
ioctl(dev, cmd, addr)
dev_t dev;
long addr;
{
	PXDMQRequest *rq;
	long i;

	rq = &rq1;
	rq->fileid = fid;
	rq->requestcode = MODIFY_FILE;
	gflush();

	rq->mqreqcode = (unsigned)cmd;
	serverid = GetPid(DEVICE_SERVER,LOCAL_PID);

	switch(cmd) {

	case	0:	/* kb_nano(c) */
	case	1:	/* set_signal(c) */
	case	2:	/* reset_signal(c) */
	case	3:	/* force_signal(c) */
		rq->arg = (u_char)addr;
		if ( Send(rq,serverid ) == 0 ) {
			messagef("ioctl Send error ");
			return(ERROR);
		}
		if ( rq->requestcode != OK ) {
			messagef("ioctl Send reqcode error cmd %d",cmd);
			return(ERROR);
		}
		break;
	case	6:	/* read_avail */
		rq->requestcode = QUERY_FILE;
		if ( Send(rq,serverid ) == 0 ) {
			messagef("read_avail Send error ");
			return(ERROR);
		}
		if ( rq->requestcode != OK ) {
			messagef("read_avail error ");
			return(ERROR);
		}
		fst = (getput *)addr;
		fst->mbaddr = (long)rq->junk[0];
		break;
	case	7:	/* fetch_byte(addr) */
	case	14:	/* read_open_initted(addr) */
	case	15:	/* test_dma_memory(addr) */
		fst = (getput *)addr;
		rq->arg = (unsigned)fst->mbaddr;
		rq->requestcode = QUERY_FILE;
		if ( Send(rq,serverid ) == 0 ) {
			return(ERROR);
		}
		if ( rq->requestcode != OK ) {
			return(ERROR);
		}
		fst->mbchar = rq->junk[0];
		break;
	case	8:	/* store_byte(addr) */
		fst = (getput *)addr;
		rq->arg = (unsigned)fst->mbaddr;
		rq->junk[0] = fst->mbchar;
		if ( Send(rq,serverid ) == 0 ) {
			return(ERROR);
		}
		if ( rq->requestcode != OK ) {
			return(ERROR);
		}
		break;
	case	5:	/* init_signal */
	case	11:	/* raise1 */
	case	13:	/* force_open_init */
		if ( Send(rq,serverid ) == 0 ) {
			return(ERROR);
		}
		if ( rq->requestcode != OK ) {
			return(ERROR);
		}
		break;
	default:
		messagef(" default in ioctl for %d ",cmd);
		for (i=1; i++ < 100000;)
			;
		ringbell();
		return(ERROR);
	}
	return(OK);
}


long
readpxd()
{
	Process_id serverid;
	union message {
		IoRequest  	req;
		IoReply  	reply;
	} msg;
	union message *mp;
	register u_char c2;
	register u_char *rdp;

	mp = &msg;
	serverid = GetPid(DEVICE_SERVER,LOCAL_PID);

	mp->req.requestcode = READ_INSTANCE;
	mp->req.fileid = fid;
	mp->req.bufferptr = (char *)Rbuf;
	mp->req.bytecount = PXDMASIZ;
	errno = 0;

	if ( Send(mp,serverid) == 0 ) {
		messagef("readpxd Send failure ");
		return(ERROR);
	}

	if ( mp->reply.replycode != OK ) {
		messagef("readpxd replycode failure ");
		return(ERROR);
	}
	Nbytecount = mp->reply.bytecount;

	errno = (u_short)mp->reply.shortbuffer[0];
	if (errno) {
		messagef("\nreadpxd: err %d",errno);
		Lerrno = errno;
		return(ERROR);
	}
/*	printf("-%x",Nbytecount);	/***/
	return(Nbytecount);
}


writepxd(fd,bufptr,count)
InstanceId fd;
char *bufptr;
int count;
{
	Process_id serverid;
	union message {
		Message		msg;
		IoRequest  	req;
		IoReply  	reply;
	} msg;
	union message *mp;

	mp = &msg;
	serverid = GetPid(DEVICE_SERVER,LOCAL_PID);

	mp->req.requestcode = WRITE_INSTANCE;
	mp->req.fileid = fd;
	mp->req.bufferptr = bufptr;
	mp->req.bytecount = count;

	if ( Send(&msg,serverid) == 0 ) {
	}

	if ( msg.reply.shortbuffer[0] != OK ) {
		return(ERROR);
	}
	return(OK);
}


/*
** these routines are from textctl, and are modified to
** work on GL1 or GL2 terminals.
*/
unsigned char scrbuff[SCRBUFFSIZE];
unsigned char *swp;
int	swc;

flushscreen()
{
#ifdef GL2TERM
	text_write(textfile,scrbuff,swc);
#else
    write(textfile,scrbuff,swc);
#endif GL2TERM
    swc = 0;
    swp = scrbuff;
}



putscrchar( onechar )
char onechar;
{
    putscreenchar(onechar);
}

message( s )
register char *s;
{
    while( *s )
	putscrchar( *s++ );
    flushscreen();
}

/*VARARGS*/
message1( s, v )
register char *s;
unsigned v;
{
    char sv[20];
    char *svp = sv;
    sprintf(sv, "%x", v);
    while( *s )
	putscrchar( *s++ );
    while( *svp )
	putscrchar( *svp++ );
    flushscreen();
}

/*VARARGS*/
messagef( s, a,b,c,d,e,f,g,h,i,j )
register char *s;
{
    char *svp = mfbuf;
    sprintf(svp, s,a,b,c,d,e,f,g,h,i,j);
    while( *svp )
	putscrchar( *svp++ );
    flushscreen();
}



ttyinit()
{
#ifdef GL2TERM
	textfile = text_open("console",2);
#else
	textfile = open("console",2); 
	gl_attachtx(0, 0);
#endif GL2TERM
	textport(0,1023,0,767);
	swc = 0;
	swp = scrbuff;
}

ttynewline()
{
}

ttyclear()
{
}

localprim()
{
}

blkqread()
{
}

#ifdef PM1
gdownload()
{
}
lastone()
{
}
bogus()
{
}
xexit()
{
}
#endif PM1


#ifndef RGL
initserial()
{
}
#endif RGL


repainter()
{
	while(1) {
		Delay(0, 10);
		grioctl(GR_SAFE);
	}
}


#ifndef RGL

irisinit()
{
}

switchtographics()
{
}


#endif RGL

