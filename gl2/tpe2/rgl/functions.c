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

/********************** F U N C T I O N S . C *******************************
*
*   This module provides the interface to the PCOX board.
*
*	f13_diag	tests PCOX card onboard diagnostics
*	f13_loop	repeatedly tests PCOX onboard diagnostics
*	force_asc_string forces up to 256 ASCII char to host
*	force_ebx_string forces up to 256 EBCDIC char to host
*	reload		called by emulator after receiving a message
*	send_asc_string	sends up to 1900 char ASCII to host
*	send_binary_str sends up to 1900 char to host
*	send_direct	send a character direct to PCOX(untranslated)
*	send_ebx_string	sends up to 1900 char EBCDIC to host
*
*	the 3278 card uses a 3274 symbol set, not EBCDIC or ASCII
************************************************************************/

/*#include "gl.h"
#include "Vserial.h"
#include "chars.h"
#include "hostio.h"
#include "grioctl.h"*/
#include <sys/types.h>
#include <errno.h>
#include <Vioprotocl.h>
#include <Vio.h>
#include <Venviron.h>
#include "rpc.h"
#include "term.h"
#include "pxw.h"

#define ESC	0x1b
#define RETRIES 20
#ifdef M68020
#define GET1SEC	2000
#define GET4SEC 16000
#else
#define GET1SEC	500
#define GET4SEC 4000
#endif M68020


/*  External variables
*/
extern int	errno;
extern InstanceId fid;
extern outft	frcv;
extern		get_it();
extern u_char	getspot(); 		/* get a char from the display buffer */
extern px_status outb;
extern px_bufs	pxl;
extern rglft	rrcv;
extern long	writepxd();
extern u_short	Ack_flag, Cursor_addr;
extern u_char	Asc_xlat[];
extern u_char	Binxlat[];
extern dma_buffer DMa;
extern u_char	Ebx_xlat[];
extern char	Ft_type;
extern u_char	Pxd_debug;
extern u_char	Status_flags;
extern u_char	UDload;
extern x_key_table X_key_xlat[];
extern int 	rc;
extern u_char 	*rp;
extern int	readhost();

/*
**	Local variables
*/
char diag_table[] = {
	0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,
	12,12,13,13,14,14,15,15,16,16,17,17,18,18 /* memory patterns */
};
#define DIAG_LGTH (sizeof(diag_table)/sizeof(diag_table[0]))

static	u_char	buf[270];		/* 256 + room for header */
	u_char  s[4];
	u_char  Ebx_asc = 0;			/* EBCDIC = 0, ASCII = 1 */
Process_id	rhid;
tr_fnct(flag)
{
	trace = flag;
}


/********************** F 1 3 _ D I A G *********************************
*
*  FUNCTION:
*	Diagnostic functions
*
*  ENTRY:
*	Nothing
*   The ports on the PX are used:
*	read
*	SIGNAL - not read
*  	KREG - PCOX data char
*   write
*	SIGNAL - tells PCOX whate mode to be in
*	KREG - tx data to PCOX and/or 3274
*
*  RETURNS:
*	0    - timeout error
*	1-37 - data pattern error
*	38   - success
*
*  EXTERNALS USED:
*
************************************************************************/

u_short 
f13_diag()
{

	u_char c;
	register char i;

	(void)reset_signal(RUN_BIT|SIG1|SIG2|SIG3); 	/* invoke diag */
	delay(MS_100);
	(void)set_signal(RUN_BIT | SIG2); 	/* start nano diags */
	DMa.d_cnt = 0;
	for (i = 0; i < DIAG_LGTH; i++) {
		if ((char)(c = get_diag_byte()) == ERROR) 
			break;			/* timeout */
		if (c != diag_table[i])	/* now compare to diag_table */
			break; 			/* bad for now */
	}
	while ((u_char)(c = get_diag_byte()) != ERROR)
		;
	(void)reset_signal(RUN_BIT | SIG2);
	delay(MS_100);
	(void)set_signal(RUN_BIT);
	get_it();
	return(i);
}


u_short 
f13_loop(times)
long times;
{

	register u_char c, i;
	register long j;

	(void)reset_signal(RUN_BIT|SIG1|SIG2|SIG3); 	/* invoke diag */
	delay(MS_100);
	messagef(" loop test %d times\r\n",times);
	(void)update3270();
	for (j = 0; j++ < times; ) {
		(void)set_signal(RUN_BIT | SIG2); 	/* start nano diags */
		DMa.d_cnt = 0;
		for (i = 0; i < DIAG_LGTH; i++) {
			if ((char)(c = get_diag_byte()) == ERROR) 
				break;			/* timeout */
			messagef("%02d ", c);
			if (c != diag_table[i])	/* same as diag_table */
				break; 			/* bad for now */
		}
		(void)reset_signal(RUN_BIT | SIG2);
		messagef("\r\n");
		while ((char)(c = get_diag_byte()) != ERROR)
			;
	}
	(void)reset_signal(RUN_BIT | SIG2);
	delay(MS_100);
	(void)set_signal(RUN_BIT);
	get_it();
	return(i);
}


/*
** read a byte, return ERROR if timeout
*/
get_diag_byte()
{

	register short tm;

	if (!DMa.d_cnt) {
		for (tm = 0; !DMa.d_cnt && tm++ < GET1SEC; ) {
			if ((DMa.d_cnt = pxdread(DMa.d_buf, sizeof(DMa.d_buf))))
				break;
		}
		if (errno || (DMa.d_cnt == ERROR)) {
			(void)messagef("f13_diag Cannot read '/dev/pxd': errno = %d\n"
			    , errno);
			exit(1);
		}
		if (!DMa.d_cnt)
			return (ERROR);	/* time out */
		DMa.d_rdp = DMa.d_buf;
	}
	DMa.d_cnt--;
	return ((u_char)*DMa.d_rdp++);
}



bin_encode(readptr,encodeptr)
u_char *readptr, *encodeptr;
{
	register u_char *rdp, *codeptr;
	register u_char c1, c2, c3;

	rdp = readptr;
	codeptr = encodeptr;
	c2 = *rdp++;
	c1 = c2 >> 2;
	*codeptr++ = Binxlat[c1];	/* top 6 first byte */
	c1 = c2 & 0x03;
	c1 = c1 << 4;
	c2 = *rdp++;
	c3 = c2 >> 4;
	c3 |= c1;
	*codeptr++ = Binxlat[c3];	/* bot 2 first byte, top 4 byte 2 */
	c1 = c2 & 0x0f;
	c1 = c1 << 2;
	c2 = *rdp++;
	c3 = c2 >> 6;
	c3 |= c1;
	*codeptr++ = Binxlat[c3];	/* bot 4 byte 2, top 2 third byte */
	c2 &= 0x3f;
	*codeptr = Binxlat[c2];		/* bot 6 third byte */
}


/*
**      reload file transfer
*/
reload(wasread)
long wasread;
{
	u_short ilen = 0x2f;
	u_short len;
	static u_char d = (u_char)0x80;

	if (UDload && context)
		rglmulator(wasread);
	pxl.pxt_buf.bufp = pxl.dma_buf;
	rrcv.bodyaddr = (u_char *)pxl.dma_buf;
	rrcv.bodylen = (long)PXDMASIZ-3;
	(void)set_rglout_ptr(&rrcv);
	setspot(25,d++);
	if (outb.length) {
		len = outb.length;
		setspot(ilen--,((len % 10) + 0x20));
		len /= 10;
		setspot(ilen--,((len % 10) + 0x20));
		len /= 10;
		setspot(ilen--,((len % 10) + 0x20));
		len /= 10;
		setspot(ilen--,((len % 10) + 0x20));
	}
}



/*********************** S E N D _ S T R *****************************
*  OPERATION:
/*
**	Sure_reset is sure the 3278 keyboard is reset
**		returns length if file is received
**		returns ERROR if reset failed
*/
long
Sure_reset()
{
	u_short retries = 0,shots = 3;
	long out_cnt;

	if ((out_cnt = get3270()))
		return (out_cnt);
	if (getspot(8) == 0) {
		DT("reset found unnecessary\n");
		return 0;
	}
	for (retries = 0; retries < 10; ++retries) {
		send_x_key(X_RESET);
		delay(MS_15);
		if ((out_cnt = get3270()))
			return (out_cnt);
		if (getspot(8) == 0)
			break;
		/* test for ALT problem ?+ at status line */
		if ((getspot(8)) && getspot(0x0a) != 0xb2) {
			DT("?+ error in status \n");
			send_x_key(X_ALT_MAKE);
			send_x_key(X_RESET);
			delay(MS_15);
			if ((out_cnt = get3270()))
				return (out_cnt);
			if (getspot(8) == 0)
				break;
			send_x_key(X_SHIFT);
		}
	}
	if (retries >= 10)
		return (ERROR);
	return 0;
}


/*
**	Sure_aid resets keyboard, sends aid, and verifies 3274 locks kb after
**		returns count if file is received
**		returns ERROR if cannot verify aid sent
*/
long
Sure_x_aid(c)
u_char c;
{
	long out_cnt;
	register short tm;

	if ((out_cnt = Sure_reset()))
		return (out_cnt);
	send_x_key (c);
	/*  wait 1 sec for 3274 reply */
	for (tm = 0; tm++ < GET1SEC; ) {
		if ((out_cnt = get3270()))
			return (out_cnt);
		if (getspot(8))
			return 0;
	}
	return (ERROR);
}




/*********************** S E N D _ S T R *****************************
*  OPERATION:
*	reset, write string direct, enter, test for ACK1, if ACK0 repeat all
*	   If reset fails, send a PA1 to show host we are alive
*	   Write string direct - 256 byte segments, test Acks, repeat if Nak(s)
*
*  RETURNS:
*	-1 - file receive error (Errno tells why)
*	0 - string written (normal exit)
*	1 - length 0 or negative
*	2 - no response from host processor after enter
*	3 - length > 1900
*	5 - host is not connected to us (4Ac)
*	6 - write RETRIES exceeded
*	7 - direct write failed
*	n>16 - file received
*
*  EXTERNALS USED:
*	 Ebx_asc
*
************************************************************************/
long
send_str(str, len)
u_char *str;
long len;
{

	u_char *bpsave, *string, *sstring;
	register u_char *bp;
	register int length, slength;
	register long tm;
	u_short scursor_addr, shots, status;
	register long out_cnt, retries;

	if (len <= 0) {
		DT("EXIT from send_str with status = 1\n");
		return (1);
	} else if (len > 1900) {
		DT("EXIT from send_str with status = 3\n");
		return (3);
	}
	Status_flags &= ~(R_FLAG | S_FLAG | W_FLAG);
	if ((out_cnt = Sure_reset()) > 0) {
		return (out_cnt);
	} else if (out_cnt == ERROR) {
		if ((out_cnt = Sure_x_aid(X_ENTER)) > 0)
			return (out_cnt);
		delay(MS_100);
		DT("EXIT from send_str with status = 5\n");
		return (5);
	}
	Status_flags |= W_FLAG;
	shots = 3;
again:


	string = str;
	if ((out_cnt = Sure_reset())) {
		Status_flags &= ~(R_FLAG | S_FLAG | W_FLAG);
		return (out_cnt);
	}
	send_x_key(X_FTAB);
	send_x_key(X_ER_EOF);
	delay(MS_15);	/* time for nano to erase screen */
	if ((out_cnt = get3270())) {
		Status_flags &= ~(R_FLAG | S_FLAG | W_FLAG);
		return (out_cnt);
	}
	Cursor_addr = 0x50;	/* home */
	scursor_addr = Cursor_addr;
	bp = buf;
	*bp++ = DIR_CMD_SETH;
	*bp++ = 0;
	*bp++ = DIR_CMD_SETL;
	*bp++ = 0x50;
	*bp++ = DIR_CMD_WD;
	if (!Ebx_asc) {
		if (*str != EDOLLAR) {
			*bp++ = DOLLAR;
			*bp++ = CENT;
			scursor_addr += 2;
		} else
			bp--;
	} else {
		if (Ebx_asc == 1) {
			if (*str != ADOLLAR) {
				*bp++ = DOLLAR;
				*bp++ = CENT;
				scursor_addr +=2;
			} else
				bp--;
		} else {
			*bp++ = DOLLAR;
			*bp++ = F3274;
		/*** LEN + 3 ENCODE HERE ***/
			out_cnt = len + 3;
			s[2] = (u_char)(out_cnt & 0xff);
			out_cnt = out_cnt >> 4;
			out_cnt = out_cnt >> 4;
			s[1] = (u_char)(out_cnt & 0xff);
			out_cnt = out_cnt >> 4;
			out_cnt = out_cnt >> 4;
			s[0] = (u_char)(out_cnt & 0xff);
			bin_encode(s, bp);
			bp += 4;
			scursor_addr += 6;

		}
	}
	slength = len;
wsegment:
	bpsave = bp;
	sstring = str;
	for (retries = status = 0; status != 1 && retries < RETRIES;
	    ++retries) {
		length = (slength <= 256) ? slength : 256;
		while (length > 0) {
			*bp++ = DIR_CMD_SETH;
			*bp++ = scursor_addr >> 8;
			*bp++ = scursor_addr & 0x80 ? DIR_CMD_SETLX : DIR_CMD_SETL;
			*bp++ = scursor_addr & 0x7f;
			*bp++ = DIR_CMD_WD;
			if (Ebx_asc != 2) {
				if (!Ebx_asc) {
					for (; length-- && bp < buf + sizeof(buf); )
						*bp++ = Ebx_xlat[*str++ - 0x40];
				} else {
					for (; length-- && bp < buf + sizeof(buf); )
						*bp++ = Asc_xlat[*str++];
				}
			} else {
				/***** HERE IS BINARY 4:3 ENCODE****/
				for (; length > 0 && bp < buf + sizeof(buf); ) {
					bin_encode(str, bp);
					str += 3;
					bp += 4;
					length -= 3;
				}
			}
		}
		if (writepxd(fid, buf, bp - buf)==ERROR) {
			Status_flags &= ~(R_FLAG | S_FLAG | W_FLAG);
			return (7);
		}
		Ack_flag = 0;
		if ((out_cnt =get3270())) {
			Status_flags &= ~(R_FLAG | S_FLAG | W_FLAG);
			return (out_cnt);
		}
		status = (Ack_flag==0) ?		2 :
			 (Ack_flag==bp - buf) ?		1 :
							8;
		str = sstring;
		bp = bpsave;
	}
	if (retries >= RETRIES) {
		Status_flags &= ~(R_FLAG | S_FLAG | W_FLAG);
		return (6);
	}
	slength = (slength > 256) ? slength - 256 : 0;
	if (slength) {
		str += 256;
		scursor_addr += 256;
		bp = buf;
		goto wsegment;
	}
	str = string;
	Status_flags &= ~(R_FLAG | S_FLAG);
	send_x_key (X_ENTER);
	lampon(2);
	/*  wait 4 sec for host reply */
	for (tm = 0; tm++ < GET4SEC && (!(Status_flags & (R_FLAG | S_FLAG))); ) {
		if (out_cnt = get3270()) {
			Status_flags &= ~(R_FLAG | S_FLAG | W_FLAG);
			lampoff(2);
			return(out_cnt);
		}
	}
	lampoff(2);
	if (!(Status_flags & R_FLAG))
		if (--shots) {
			Status_flags &= ~(R_FLAG | S_FLAG);
			send_x_key(X_RESET);
			DT("Take another shot, no cent r\n");
			goto again;
		} else {
			Status_flags &= ~(R_FLAG | S_FLAG | W_FLAG);
			DT("EXIT from send_str with no host action\n");
			return(2);
		}
	DP0("ebx_w-");
	send_ack();
	Status_flags &= ~(R_FLAG | S_FLAG | W_FLAG);
	DT("EXIT from send_str with status = 0\n");
	return (0);
}

/**********************  S E N D _ A S C _ S T R *************************
*
*  FUNCTION:
*	Send ASCII string to PCOX using direct writes and the appropriate
*	aids
*
*  ENTRY:
*	str - pointer to string
*	len - string length
*
*  OPERATION:
*	Set Ebx_asc to ASCII and call send_str.
*
*  RETURNS:
*	-1 - file receive error (Errno tells why)
*	0 - string written (normal exit)
*	1 - length 0 or negative
*	2 - no response from host processor after enter
*	3 - length > 1900
*	5 - host is not connected to us (4Ac)
*	6 - write RETRIES exceeded
*	7 - direct write failed
*	n>16 - file received
*
*  EXTERNALS USED:
*	Ebx_asc, send_str
*
************************************************************************/

long
send_asc_str(str, len)
u_char *str;
long len;
{

	Ebx_asc = 1;
	return (send_str(str, len));
}

long
send_binary_str(str, len)
u_char *str;
long len;
{

	Ebx_asc = 2;
	return (send_str(str, len+3));
}
/**********************  S E N D _ E B X _ S T R *************************
*
*  FUNCTION:
*	Send EBCDIC string to PCOX using direct writes and the appropriate
*	aids
*
*  ENTRY:
*	str - pointer to string
*	len - string length
*
*  OPERATION:
*	Set Ebx_asc to ASCII and call send_str.
*
*  RETURNS:
*	-1 - file receive error (Errno tells why)
*	0 - string written (normal exit)
*	1 - length 0 or negative
*	2 - no response from host processor after enter
*	3 - length > 1900
*	5 - host is not connected to us (4Ac)
*	6 - write RETRIES exceeded
*	7 - direct write failed
*	n>16 - file received
*
*  EXTERNALS USED:
*	Ebx_asc, send_str
*
************************************************************************/

long
send_ebx_str(str, len)
u_char *str;
long len;
{

	Ebx_asc = 0;
	return (send_str(str, len));
}
long
force_str(str, len)
u_char *str;
long len;
{

	register u_char *bp;
	register long out_cnt;

	if (len <= 0) {
		DT("EXIT from force_str with status = 1\n");
		return (1);
	} else if (len > 256) {
		DT("EXIT from force_str with status = 3\n");
		return (3);
	}
	if ((out_cnt = get3270()))
		return (out_cnt);
	Cursor_addr = 0x50;		/* home */
	while (len > 0) {
		bp = buf;
		*bp++ = DIR_CMD_SETH;
		*bp++ = 0;
		*bp++ = DIR_CMD_SETL;
		*bp++ = 0x50;
		*bp++ = DIR_CMD_WD;
		if (Ebx_asc) {
			for (; len-- && bp < buf + sizeof(buf); )
				*bp++ = Asc_xlat[*str++];
		} else {
			for (; len-- && bp < buf + sizeof(buf); )
				*bp++ = Ebx_xlat[*str++ - 0x40];
		}
	}
	send_x_key(X_RESET);
	send_x_key(X_FTAB);
	send_x_key(X_ER_EOF);
	delay (MS_15);
	if (writepxd(fid, buf, bp - buf)==ERROR)
			return (7);
	DT("write OK \n");
	send_x_key (X_PA1);
	DT("EXIT from force_str with status = 0\n");
	return (0);
}
long
force_asc_str(str, len)
u_char *str;
long len;
{
	Ebx_asc = 1;
	return (force_str(str, len));
}
long
force_ebx_str(str, len)
u_char *str;
long len;
{
	Ebx_asc = 0;
	return (force_str(str, len));
}


rglmulator(wasread)
long wasread;
{

	rp = pxl.dma_buf;
	rc = wasread - 3;	/* 3 byte length starts message */
	unqkeys();
#ifdef PM1
	context = GRAPHICS;
#endif PM1
	readhost();
#ifdef PM1
	context = TEXT;
#endif PM1
	qkeys();
}

