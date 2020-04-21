/*
**		    Standard I/O stuff for terminal program
**
*/
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <termio.h>
#include <gl.h>

#include "term.h"
#include "hostio.h"
#include "pxw.h"

#define ESC		0x1b



extern int		host;
extern int		infile;
extern int		hosttype;
extern int		fd;
extern int		replay[];
extern int		ttyd;
extern int		errno; 
extern px_status	outb;
extern px_bufs		pxl;
extern u_char		Blink_msgs;
extern u_char		Outbfound;

/*
**	global declarations
*/
FILE			*rlf = 0;
FILE			*wlf = 0;
rglft			frcv;
int			logfnum = 0;
unsigned 		wc;
unsigned char	 	*wp;
unsigned char		wbuf[WBUFFSIZE]; 
int 			rc;
unsigned char 		*rp;
unsigned char		rbuf[6148]; /* IBM LENGTH INCLUDED IN MESSAGE */
unsigned char		sbuf[4]; 
unsigned char		Breaksent = 0;

/*
**	Turn on/off the tracing of this module
*/
tr_host(flag)
{
	trace = flag;
}


hostloginit()
{
    char    buf[20];

    if (dflag[2]) {
	if (rlf)
	    fclose(rlf);
	if (wlf)
	    fclose(wlf);
        logfnum++;
	sprintf(buf, "RDDATA.%d", logfnum);
	if ((rlf = fopen(buf, "w")) == NULL)
	    oops("t3279: can't open %s\n\r", buf);
	sprintf(buf, "WRDATA.%d", logfnum);
	if ((wlf = fopen(buf, "w")) == NULL)
	    oops("t3279: can't open %s\n\r", buf);
	printf("opened RDDATA.%d ", logfnum);
    } else {
	if (rlf)
	    fclose(rlf);
	if (wlf)
	    fclose(wlf);
	rlf = wlf = 0;
	printf("closed RDDATA.%d ", logfnum);
    }
}
 

/*
**	initcom - intialize serial i/o or xns.
**
*/
initcom( medium )
register int medium;
{

    wc = 0;
    wp = wbuf;
    rc = 0;
    rp= rbuf;
    *sbuf = 0x05;		/* mark size of read */

	if (medium == I3270_COM)  {
		if (pxdopen() <= 0) {
			(void)printf("Cannot open '/dev/pxd':  errno = %d\n", errno);
			exit(1);
		}
	} 
}


closecom()
{
}


fillhostbuffer()
{

	register u_char n;
	u_char c, *cp;

	rp = rbuf;
	frcv.bodyaddr = rbuf;
	frcv.bodylen = sizeof(rbuf);
	if (!replay[2])
		(void)set_rglout_ptr(&frcv);
	else
		Outbfound = 0;
	if (Blink_msgs)
		lampoff(1);
	cp = &c;
	while (context == GRAPHICS && !(rc = rupdate3270())) {
		if (n =  read(ttyd, cp, 1)) {
#ifdef DEBUG
			DT("C %x ", c);
#endif DEBUG
			if (c == 0x7e) {
				kblamp(LAMP_LOCAL,1);
				while (!(n = read(ttyd, cp, 1)))
					;
#ifdef DEBUG
				DT("C %x ", c);
#endif DEBUG
				if (c == '.') {
					kblamp(LAMP_LOCAL,0);
					sendbreakchar();
					return -1;
				} else if (c == '%')
					percentesc();
				kblamp(LAMP_LOCAL,0);
			} else if (c == ESC) {	/* escape */
				while (!(n = read(ttyd, cp, 1)))
					;
#ifdef DEBUG
				DT("C %x ", c);
#endif DEBUG
				if (c == 0x7f) { /* ESC DEL graphics exit*/
					sendbreakchar();
					return -1;
				} else
					sendkbdchar(c & 0x7f);
			}
		}
	}
	if( rc == ERROR ) {
	    return -1;
	}
	if (context != GRAPHICS)
		return -1;
	if (dflag[2]) {
		sbuf[1] = (u_char)(rc >> 8);
		sbuf[2] = (u_char)rc;
		fwrite(sbuf, 1, 3, rlf);	/* save rc for replay */
		fwrite(rbuf, 1, rc, rlf);
		errno = 0;
	}
	
#ifdef DEBUG
DT("H%d ",rc);
#endif DEBUG
/*
DT(" H%d ",rc);
{
int i;
for ( i=0;i<rc;i++)
DT("%x ",rbuf[i]);
}
*/

	rc--;
	return *rp++;
}


flushhost()
{

	long i;
	if( wc == 0 )
		return;
	if (!replay[4]) {
		kill_outb();
		if((i = send_binary_str(wbuf,wc))) {
			tr_flush();
			DT("Flushbuffer: errno %d error %d writing IBM\n\r",errno, i);
		} else
			errno = 0;
	}
	if (dflag[2]) {
		fwrite(wbuf, 1, wc, wlf);
		errno = 0;
	}
/**/
#ifdef DEBUG
DT(" W%d\n",wc);
#endif DEBUG
/**/
	wp = wbuf;
	wc = 0;
}


sendkbdchar(c)
u_char c;
{
    u_char onechar;

    onechar = c & 0x7f;
#ifdef DEBUG
    DT("sendkbd %02x ",onechar);
#endif DEBUG
	send_binary_str(&onechar,1);
}


setlocalecho( value )
int value;
{
	return 1;
}



sendbreakchar()
{

	DT("sendbreakchar ");
	if (!replay[4]) {
		send_x_key(X_PA1);
		send_x_key(X_RESET);
		send_x_key(X_PA1);
		send_x_key(X_RESET);
	}
	xgexit();
	kill_outb();
	Breaksent++;
}


/*
 * Process the ~% escapes
 */
percentesc()
{
    char    c;
    struct termio   t;
    int     temp = 0;
    int	    tempcode = 0;
    char    buf[10];
    char    *type;

    if (read(ttyd, &c, 1) > 0) {
	switch (c) {

	    case 'M': 		/* ~%M<n> */
		c = '\0';
		read(ttyd, &c, 1);
		switch (c) {
		    case '0': 
			type = "60 Hz";
			goto sendcmd;
		    case '1': 
			type = "30 Hz";
			goto sendcmd;
		    case '2': 
			type = "NTSC";
		sendcmd:
#ifdef GL1
			bellring();
#else
			setmonitor(c - '0');
			fprintf(stderr, "monitor set to %s\n\r", type);
#endif
			break;
		    default: 
			bellring();
			break;
		}
		return;


	    case 'R': 		/* ~%R */
#ifndef GL1
		noport();
		foreground();
#endif
		ginit();
		xtpon();
		putscreenchar('\0');	/* force textport to appear */
		flushscreen();
		graphinited = 1;
		return;

	    default: 
		bellring();
		break;
	}
    }
}

/*
** reads for ascii replays */
rupdate3270()
{
	int toread, wasread;

	if (!replay[2]) {
		wasread = update3270();
		if (wasread == -1)
			DT("ftdone %d errno %d ",outb.ftdone,errno);
		return (wasread);
	} else {
		wasread = read(fd, rbuf, 3);
		if (*rbuf == 0x05) {
			toread = *(rbuf+1);
			toread <<= 8;
			toread += *(rbuf+2);
			wasread = read(fd, rbuf, toread);
			errno = 0;
		} else
			wasread = 0;
	}
	if (wasread > 0)
		return (wasread);
	else {
		context = TEXT;
		return 0;
	}
}

