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
 **************************************************************************

	********* INTERNAL USE ONLY ***********
	***** REQUIRES NON_DISCLOSURE AGREEMENT ********
	***** DO NOT SHOW TO ANYONE WITHOUT A NON_DISCLOSURE AGREEMENT ****


/********************** F U N C T I O N S . C *****************************
*
*	This module provides the interface to the PCOX board.
*
*	f13_diag	tests PCOX card onboard diagnostics
*	f13_loop	repeatedly tests PCOX onboard diagnostics
*	force_asc_string forces up to HUNDRED ASCII char to host
*	force_ebx_string forces up to HUNDRED EBCDIC char to host
*	reload		called by emulator after receiving a message
*	send_asc_string	sends up to 1900 char ASCII to host
*	send_ebx_string	sends up to 1900 char EBCDIC to host
*	send_binary_str sends up to 1900 char to host
*	send_direct	send a character direct to PCOX(untranslated)
*
*	the 3278 card uses a 3274 symbol set, not EBCDIC or ASCII
************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include "term.h"
#include "rpc.h"
#include "pxw.h"

#define GET_ONE_SEC	2
#define HOST_AK		125			/* 125 seconds of get3270 */
#define HUNDRED		256
#define KEY_INHIBIT	(u_char)0x04
#define MS_5		5
#define NOACK 		(u_char)0x90		/* q */
#define NSHOTS		1
#define RETRIES 	20
#define SEG_ERR		-6

/*  External variables
*/
extern int		fd;
extern int		errno;
extern long		get3270();
extern			get_it();
extern u_char		getspot();		/* get char from Screen buffer*/
extern FILE		*mlf;
extern px_status 	outb;
extern long		pxdread();
extern px_buf_t		pxk;			/* read buffer pointers */
extern px_bufs 		pxl;
extern int 		rc;
extern int		replay[];
extern FILE		*rlf;
extern unsigned char 	*rp;
extern rglft		rrcv;
extern int		tfd;			/**** RFUNC ***/
extern char		*sys_errlist[];
extern int		sys_nerr;
extern u_short		Ack_flag;
extern u_char		FxAsc_xlat[];           /* File Transfer  wpc */
extern u_char		Asc2ebc[];
extern u_char		Binxlat[];
extern u_char		Blink_msgs;
extern u_char		Buffer_is_mem;
extern u_char		Control_ctr;
extern u_char		Control_reg;
extern u_char		Curhi_ctr;
extern u_char		Clear_flag;
extern u_short		Cursor_addr;
extern u_char		Display_xlat[];
extern dma_opr		DMa;
extern u_char		Ebx_xlat[];
extern u_char		Ebc2asc[];
extern u_char		F3174;
extern long		Millibuzz;
extern u_char		Msg_proc;
extern u_char		Outbfound;
extern int		Pxd_dma_size;
extern u_char		Rbuf[];
extern u_char		Status_flags;
extern u_char		Want_ack;

/*
**	Global variables
*/
struct stat		Fstat;
long			Host_ack = HOST_AK;
long			MaxRU = DEFAULTRU;
u_char			Msgtype = 0;
u_char			Record_sep = (u_char)0xe1;
long			Tm;

/*
**	Local variables
*/
u_char			buf[370];	/* coded 256 + room for header*/
int			lfd, dlfd = ERROR;
u_char			opendiag = 0;
u_char			lbuffer[2048];
u_char			mbuf[4]; 
u_char			nbuffer[2048];	/* double buffered reads */
u_char  		pathname[80];
u_char			ptext[] = {".TEXT"};
u_char  		s[4];
u_short			scursor_addr;
u_char			send_type;

u_char diag_table[] = {
	0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,
	12,12,13,13,14,14,15,15,16,16,17,17,18,18,
	19,19,20,20,21,21,22,			/* test pattern */
};
/*#define D_LGTH (sizeof(diag_table)/sizeof(diag_table[0]))*/
#define D_LGTH	38
char			diag_lgth = D_LGTH;


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
*   read
*	SIGNAL - not read
*  	KREG - PCOX data char
*   write
*	SIGNAL - tells PCOX what mode to be in
*	KREG - tx data to PCOX and/or 3274
*
*  RETURNS:
*	0    - timeout error
*	1-37 - data pattern error
*	>= 38   - success
*
*  EXTERNALS USED:
*
************************************************************************/

u_short 
f13_diag()
{

	extern u_char get_diag_byte();
	register u_char c, i;
	register long j;

	DMa.d_cnt = 0;
	(void)reset_signal(RUN_BIT|SIG1|SIG2|SIG3); 	/* invoke diag */
	for (j = 3; j ; )
		j = DMA_ZAP == get_diag_byte() ? --j : 3;
	(void)set_signal(RUN_BIT | SIG2); 	/* start nano diags */
	DMa.d_cnt = 0;
	for (i = 0; i < diag_lgth; i++) {
		if ((c = get_diag_byte()) == DMA_ZAP) {
			printf(" pxd:timeout buzz %d ",Millibuzz);
			break;			/* timeout */
		}
		if (opendiag)
			printf("%02d ", c);
		if (c != diag_table[i])	{	/* diag_table has pattern */
			printf("pxd: i %d c %02x is not zero ",i, c);
			printf ("%02x %02x %02x %02x %02x %02x %02x ",
			(u_char)*DMa.d_rdp++,
			(u_char)*DMa.d_rdp++,
			(u_char)*DMa.d_rdp++,
			(u_char)*DMa.d_rdp++,
			(u_char)*DMa.d_rdp++,
			(u_char)*DMa.d_rdp++,
			(u_char)*DMa.d_rdp++);
			break; 			/* bad for now */
		}
	}
	while (get_diag_byte() != DMA_ZAP)
		;
	(void)reset_signal(RUN_BIT | SIG2);
	pdelay(MS_100);
	(void)set_signal(RUN_BIT);
	get_it();
	normalize_kb();
	return(i);
}

u_short 
f13_loop(count,prtflag)
long count;
char prtflag;
{

	extern u_char get_diag_byte();
	register u_char c, i;
	register long j;

	DMa.d_cnt = 0;
	(void)reset_signal(RUN_BIT|SIG1|SIG2|SIG3); 	/* invoke diag */
	for (j = 3; j ; )
		j = DMA_ZAP == get_diag_byte() ? --j : 3;
	if (prtflag)
		printf("loop test ");
	for (j = 0; j++ < count; ) {
		(void)set_signal(RUN_BIT | SIG2); 	/* start nano diags */
		DMa.d_cnt = 0;
		for (i = 0; i < diag_lgth; i++) {
			if ((c = get_diag_byte()) == DMA_ZAP) {
				printf(" pxd:timeout ");
				break;			/* timeout */
			}
			if (c != diag_table[i]) {	/* diag_table has pattern */
				if (c != (u_char)0xfe)
					printf(" count %d read %02d should be %02d ",
					j,c,diag_table[i]);
				break; 			/* bad for now */
			}
		}
		(void)reset_signal(RUN_BIT | SIG2);
		if (prtflag)
			printf(".");
		while ((char)(c = get_diag_byte()) != DMA_ZAP)
			;
	}
	(void)reset_signal(RUN_BIT | SIG2);
	pdelay(MS_100);
	(void)set_signal(RUN_BIT);
	get_it();
	normalize_kb();
	return(i);
}


/*
** read a byte, return ERROR if timeout
*/
u_char
get_diag_byte()
{

	if (!DMa.d_cnt) {
		DMa.d_rdp = Rbuf;
		errno = 0;
		for (Tm = Millibuzz; !DMa.d_cnt && Tm--; ) {
			if (DMa.d_cnt = pxdread())
				break;
		}
		if (errno || (DMa.d_cnt == ERROR)) {
			(void)printf("get_diag_byte read '/dev/pxd': errno = %d\n"
			    , errno);
			exit(1);
		}
		if (!DMa.d_cnt)
			return (DMA_ZAP);	/* time out */
	}
	DMa.d_cnt--;
	return ((u_char)*DMa.d_rdp++);
}

f14_diag()
{
	register u_char c, i;
	register long j;
	u_char tbuf[23*80 - 2];
	extern long load_segment();

	for (j = 0; j < sizeof(tbuf); j++)
		tbuf[j] = (u_char)('A'+(j % 62));
	send_type = 1;
	Cursor_addr = COLS;			/* forced home */
	scursor_addr = Cursor_addr;
	if ((j = load_segment(tbuf, buf, HUNDRED, sizeof(tbuf))) != 0) {
		printf("pxd:error in load_segment %d ",j);
		return (j);
	}
	get_screen();
	for (j = 0; j < sizeof(tbuf); j++) {
		i = Display_xlat[getspot(j + COLS)];
		c = (u_char)('A' + (j % 62));
		if (c != i) {
			printf("pxd:direct write function failed\n");
			printf("addr %x is %02x should be %02x\n",j+COLS,i,c);
			break;
		}
	}
	return (sizeof(tbuf)-j);

}

f14_loop(countr,prtflag)
long countr;
char prtflag;
{
	int i = 0;

	if (prtflag)
		printf(" direct write test ");
	while (countr--) {
		if (prtflag && !(i = f14_diag()))
			printf(".");
	}
	if (!i)					/* proper operation */
		i = 38;				/* no trailing dump */
	return (i);
}

bin_encode(readptr,encodeptr,binlength)
register u_char *readptr, *encodeptr;
register short binlength;
{
	register u_char c1, c2, c3;
	register short encodectr = 0;

	while (binlength > 0) {
		c2 = *readptr++;
		c1 = c2 >> 2;
		*encodeptr++ = Binxlat[c1];	/* top 6 first byte */
		c1 = c2 & 0x03;
		c1 <<= 4;
		c2 = *readptr++;
		c3 = c2 >> 4;
		c3 |= c1;
		*encodeptr++ = Binxlat[c3];
		c1 = c2 & 0x0f;
		c1 <<= 2;
		c2 = *readptr++;
		c3 = c2 >> 6;
		c3 |= c1;
		*encodeptr++ = Binxlat[c3];
		c2 &= 0x3f;
		*encodeptr++ = Binxlat[c2];	/* bot 6 third byte */
		binlength -= 3;
		encodectr += 4;
	}
	return (encodectr);
}

long
load_segment(str, bp, seglen, slength)
register u_char *bp, *str;
long seglen;
register short slength;
{
	register u_char *bpmax;
	register short length;
 	u_char status, *bpsave, *segstring;
	long out_cnt, retries, returnval;
	
	Control_ctr = 0;
	bpmax = buf + sizeof(buf);
wsegment:
	bpsave = bp;
	segstring = str;
	for (retries = status = 0; status != 1 && retries < RETRIES;
	    ++retries) {
		length = (slength <= seglen) ? slength : seglen;
/*#ifdef DEBUG
		DT("segment length %x ",length);
#endif /* DEBUG */
		while (length > 0) {
			*bp++ = DIR_CMD_SETH;
/*#ifdef DEBUG
			DT("cursor addr %x ",scursor_addr);
#endif /* DEBUG */
			*bp++ = scursor_addr >> 8;
			*bp++ = scursor_addr & 0x80 ? DIR_CMD_SETLX : DIR_CMD_SETL;
			*bp++ = scursor_addr & 0x7f;
			*bp++ = DIR_CMD_WD;
			if (send_type != 2) {
				if (!send_type) {
					while (length-- && bp < bpmax)
						*bp++ = Ebx_xlat[*str++];
				} else {
					while (length-- && bp < bpmax)
						*bp++ = FxAsc_xlat[*str++];/*wpc
						File Transfer above  wpc */
				}
			} else {
				bp += bin_encode(str, bp, length);
				length = 0;
			}
		}
		errno = 0;
		logstr(buf, bp - buf);
		if (write(tfd, buf, bp - buf) == ERROR) {
#ifdef DEBUG
			DT("write error %d\n",errno);
#endif /* DEBUG */
			returnval = -7;
			goto returnerr;
		}
		Ack_flag = 0;
		if (out_cnt = get3270())
			goto returnmsg;
#ifdef DEBUG
/*		DT("Ack_flag = %x\n", Ack_flag);*/
		if (Ack_flag != bp - buf) {
			DT("LOAD SEGMENT ERROR, Cursor_addr %x\n",Cursor_addr);
		}
#endif /* DEBUG */
		status = (Ack_flag == 0) ?		2 :
			 (Ack_flag == bp - buf) ?	1 :
							8;
		str = segstring;
		bp = bpsave;
		if (replay[4])
			status = 1;
		if (Control_ctr || Cursor_addr != COLS) {
			returnval = SEG_ERR;
			if (Msg_proc == 0 && S_FLAG) {	/* exit on got_killed */
#ifdef DEBUG
				logwrite((u_char)0xa0);
				logdump();
				DT("error %02x status %02x\n",getspot(10),getspot(12));
#endif /* DEBUG */
				returnval = -7;
			}
			goto returnerr;
		}
	}
	if (retries >= RETRIES) {
#ifdef DEBUG
		DT("Failed after %d retries\n", RETRIES);
#endif /* DEBUG */
		returnval = -6;
		goto returnerr;
	}
	slength = (slength > seglen) ? slength - seglen : 0;
	if (slength) {
/*#ifdef DEBUG
		DT("slength %x\n",slength);
#endif /* DEBUG */
		str += seglen;
		scursor_addr += HUNDRED;
		bp = buf;
		goto wsegment;
	}
	return 0;
returnmsg:
	Status_flags &= ~(R_FLAG | S_FLAG | W_FLAG);
	return (out_cnt);
returnerr:
	Status_flags &= ~(R_FLAG | S_FLAG | W_FLAG);
#ifdef DEBUG
	DT("return from load_segment with err %d\n",returnval);
#endif /* DEBUG */
	return (returnval);
}


/*
**      reload file transfer
*/
reload(wasread)
long wasread;
{
	u_short ilen = 27;
	u_short len;
	static u_char d = 'p';

	if (!Msg_proc)
		test_xfer_type(wasread);
	if (Msg_proc == RGLXFER) {
		rglmulator(wasread);		/* do graphics */
		rrcv.bodyaddr = (u_char *)pxl.dma_buf;
		rrcv.bodylen = Pxd_dma_size-3;
		(void)set_rglout_ptr(&rrcv);
	} else if (Msg_proc == FXFER) {
		rrcv.bodyaddr = (u_char *)pxl.dma_buf;
		rrcv.bodylen = Pxd_dma_size-3;
		if ((updown(wasread)) == CNTU) /* do file xfer */
			(void)set_rglcntu(&rrcv);
		else
			(void)set_rglout_ptr(&rrcv);
		return;
	} else {
		if (dflag[2]) {
#ifdef DEBUG
			DT("M%x ",wasread);
#endif /* DEBUG */
			mbuf[0] = DMA_ZAP;
			mbuf[1] = (u_char)(wasread >> 16);
			mbuf[2] = (u_char)(wasread >> 8);
			mbuf[3] = (u_char)wasread;
			fwrite(mbuf, 1, 4, mlf);	/* save for replay */
			fwrite(pxl.dma_buf, 1, wasread, mlf);
			errno = 0;
		}
		rrcv.bodyaddr = (u_char *)pxl.dma_buf;
		rrcv.bodylen = Pxd_dma_size-3;
		if (outb.host_len)
			(void)set_rglcntu(&rrcv);
		else
			(void)set_rglout_ptr(&rrcv);
		Msg_proc = MXFER;
		setspot(18,d++);
		if (d > (u_char)0xfd)
			d = 7;
		if (outb.length) {
			len = outb.length;
			setspot(ilen--,((len % 10) + 0x20));
			len /= 10;
			setspot(ilen--,((len % 10) + 0x20));
			len /= 10;
			setspot(ilen--,((len % 10) + 0x20));
			len /= 10;
			setspot(ilen,((len % 10) + 0x20));
		}
	}
}


/*
**	Sure_reset is sure the 3278 keyboard is reset
**		returns length if file is received
**		returns ERROR if reset failed
*/
long
Sure_reset()
{
	register u_char retries;
	register long loop_cntr, out_cnt;

	if (outb.ftdone) {
#ifdef DEBUG
		DT("Sure_reset called with ftdone %d\n",outb.ftdone);
#endif /* DEBUG */
		outb.ftdone = 0;
	}
	if (out_cnt = get3270())
		return (out_cnt);
	if (getspot(8) == 0 && (!(Control_reg & KEY_INHIBIT)))
		return 0;
	if (replay[1])
		return 0;
	if (outb.ftdone) {
#ifdef DEBUG
		DT("Sure_reset reads ftdone %d\n",outb.ftdone);
#endif /* DEBUG */
		outb.ftdone = 0;
	}
	if (Control_reg & KEY_INHIBIT)
		send_x_key(X_RESET);
	for (retries = 0; retries < RETRIES; ++retries) {  /* use RETRIES wpc */
#ifdef DEBUG
		DT("Sure_reset %02x ",getspot(10));
#endif /* DEBUG */
		for (loop_cntr = Millibuzz * (2*GET_ONE_SEC); loop_cntr-- ; ) {
			if (out_cnt = get3270())
				return (out_cnt);
			if (getspot(8) == 0 && (!(Control_reg & KEY_INHIBIT)))
				return 0;
			if (outb.ftdone) {
#ifdef DEBUG
				DT("Sure_reset reads ftdone %d\n",outb.ftdone);
#endif /* DEBUG */
				outb.ftdone = 0;
			}
			/* test for ALT problem ?+ at status line */
			if (getspot(8) 
				&& (!(Control_reg & KEY_INHIBIT)) /* unlocked */
				&& getspot(10) != (u_char)0xb2 /* SYSTEM */
				&& getspot(10) != (u_char)0xf4) { /* () */
				if (Blink_msgs)
					lampon(8);
#ifdef DEBUG
				logwrite((u_char)0xa1);
				logdump();
				DT("error %02x inhibited status\n",getspot(10));
#endif /* DEBUG */
				send_x_key(X_ALT_MAKE);
				if (Blink_msgs)
					lampoff(8);
				break;
			}
		}
		send_x_nowait(X_RESET);
	}
	logwrite((u_char)0xa2);
	logdump();
#ifdef DEBUG
	DT("Sure_reset %d retries with %02x status \n",RETRIES,getspot(10)); 
	/* above was hard coded 10, now use RETRIES wpc */
#endif /* DEBUG */
	return (ERROR);
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

	if ((out_cnt = Sure_reset()))
		return (out_cnt);
	send_x_key (c);
	/*  wait 1 sec for 3274 reply */
	for (Tm = Millibuzz * GET_ONE_SEC; Tm-- ; ) {
		if (out_cnt = get3270())
			return (out_cnt);
		if (getspot(8))
			return 0;
	}
	return (ERROR);
}


/*
**  send_protocol is a stripped version of send_str for sending ascii
**  strings, and not waiting for a cent r acknowledgement. It is used to
**  avoid the X ?+ from a busy 3274.
*/
long
send_protocol(str, len)
register u_char *str;
short len;
{
	extern long prep_write();
	register u_char *bp;
	register short slength;
	long out_cnt, returnval, seglen, rshots;
	
	if (replay[1])
		return (0);
	errno = 0;				/* ignore callers problems */
	if (errno == 9) {
		errno = 0;
	}
	rshots = NSHOTS + 1;			/* thy will be done */
reset_again:
	Status_flags &= ~(R_FLAG | S_FLAG | W_FLAG);
	if ((out_cnt = Sure_reset()) > 0) {
		goto returnmsg;
	} else if (out_cnt == ERROR) {
		if ((out_cnt = Sure_x_aid(X_ENTER)) > 0)
			goto returnmsg;
		returnval = 5;
		goto returnerr;
	}
again:
	if ((out_cnt = prep_write()) > 0)
		goto returnmsg;
	else if (getspot(8)) {
#ifdef DEBUG
		DT("resetting again %02x ",getspot(10));
#endif /* DEBUG */
		if (rshots-- > 0)
			goto reset_again;
		returnval = 4;
		goto returnerr;
	}
	Cursor_addr = COLS;			/* forced home */
	scursor_addr = Cursor_addr;
	bp = buf;
	*bp++ = DIR_CMD_SETH;
/*#ifdef DEBUG
	DT("cursor addr %x ",Cursor_addr);
#endif /* DEBUG */
	*bp++ = 0;
	*bp++ = DIR_CMD_SETL;
	*bp++ = COLS;
	*bp++ = DIR_CMD_WD;
	seglen = HUNDRED;
	send_type = 1;			/* ascii protocol */
	bp--;
	slength = len;
	Tm = load_segment(str, bp, seglen, slength);
	if (Tm > 0) {
		out_cnt = Tm;
		goto returnmsg;
	} else if (Tm < 0) {
		if (Tm == SEG_ERR) {
			rshots--;
			if ((out_cnt = Sure_reset()) > 0)
				goto returnmsg;
			goto again;
		}
		returnval = -Tm;
		goto returnerr;
	}
	Status_flags &= ~(R_FLAG | S_FLAG);
	send_x_nowait(X_ENTER);
#ifdef DEBUG
	DT("ebx_w-");
#endif /* DEBUG */
	Status_flags &= ~(R_FLAG | S_FLAG | W_FLAG);
	return 0;
returnmsg:
	Status_flags &= ~(R_FLAG | S_FLAG | W_FLAG);
	return (out_cnt);
returnerr:
	Status_flags &= ~(R_FLAG | S_FLAG | W_FLAG);
#ifdef DEBUG
	DT("return from send_protocol with err %d\n",returnval);
#endif /* DEBUG */
	return (returnval);
}


/*********************** S E N D _ S T R *****************************
*  OPERATION:
*	start with reset, enter to gain the screen from host, then
*	   If reset fails, send a PA1 to show host we are alive
*	reset, write string direct, enter, test for ACK1, if ACK0 repeat all
*	   Write string direct - HUNDRED byte segments, test Acks, repeat if
*		Nak(s) or Ack count is wrong
*
*  RETURNS:
*	ERROR - file receive error (outb.ftdone tells why)
*	0 - string written (normal exit)
*	1 - length > 1900 or 0 or negative
*	2 - no response from host processor after enter
*	3 - host rejected because input buffer is full
*	4 - reset retries exceeded
*	5 - host is not connected to us (4Ac)
*	6 - write RETRIES exceeded
*	7 - direct write failed
*	n>16 - file received
*
*  EXTERNALS USED:
*	 send_type
*
************************************************************************/
long
send_str(str, len)
register u_char *str;
short len;
{
	extern long prep_write();
	register u_char *bp;
	register short slength;
 	u_char *string;
	long out_cnt, returnval, seglen, rshots, shots;
	
	if (replay[1])
		return (0);
#ifdef DEBUG
	DT("ENTER Send_str Len = %x\n", len);
#endif /* DEBUG */
	errno = 0;				/* ignore callers problems */
	if (len <= 0 || len > 1900) {
#ifdef DEBUG
		DT("EXIT from send_str with status = 1\n");
#endif /* DEBUG */
		return (1);
	}
	if (errno == 9) {
#ifdef DEBUG
		DT("send_str with errno 9\n");
#endif /* DEBUG */
		errno = 0;
	}
	rshots = NSHOTS + 1;
reset_again:
	Status_flags &= ~(R_FLAG | S_FLAG | W_FLAG);
	if ((out_cnt = Sure_reset()) > 0) {
		goto returnmsg;
	} else if (out_cnt == ERROR) {
		if ((out_cnt = Sure_x_aid(X_ENTER)) > 0)
			goto returnmsg;
		returnval = 5;
		goto returnerr;
	}
	shots = NSHOTS;
again:

	string = str;
	if ((out_cnt = prep_write()) > 0)
		goto returnmsg;
	else if (getspot(8)) {
#ifdef DEBUG
		DT("resetting again %02x ",getspot(10));
#endif /* DEBUG */
		if (rshots-- > 0)
			goto reset_again;
		returnval = 4;
		goto returnerr;
	}
	Cursor_addr = COLS;			/* forced home */
	scursor_addr = Cursor_addr;
	bp = buf;
	*bp++ = DIR_CMD_SETH;
/*#ifdef DEBUG
	DT("cursor addr %x ",Cursor_addr);
#endif /* DEBUG */
	*bp++ = 0;
	*bp++ = DIR_CMD_SETL;
	*bp++ = COLS;
	*bp++ = DIR_CMD_WD;
	seglen = HUNDRED;
	if (send_type != 2) {
		if ((send_type == 0 && *str == EDOLLAR) ||   /* 0x65 now wpc */
			(send_type == 1 && *str == ADOLLAR)) /* 0x80 now wpc */
			bp--;
		else {
			*bp++ = DOLLAR;
			*bp++ = CENT;
			scursor_addr += 2;
		}
	} else {
		*bp++ = DOLLAR;
		*bp++ = F3274;
		Tm = len + 3;
		s[2] = (u_char)Tm;
		Tm >>= 8;
		s[1] = (u_char)Tm;
		Tm >>= 8;
		s[0] = (u_char)Tm;
		bp += bin_encode(s, bp, 3);
		scursor_addr += 6;
		seglen = (HUNDRED / 4) * 3;

	}
	slength = len;
	Tm = load_segment(str, bp, seglen, slength);
	if (Tm > 0) {
		out_cnt = Tm;
		goto returnmsg;
	} else if (Tm < 0) {
		if (Tm == SEG_ERR) {
			rshots--;
			if ((out_cnt = Sure_reset()) > 0)
				goto returnmsg;
			goto again;
		}
		returnval = -Tm;
		goto returnerr;
	}
	str = string;
	Status_flags &= ~(R_FLAG | S_FLAG);
	if (Blink_msgs)
		lampon(2);
	send_x_nowait(X_ENTER);
	Status_flags |= W_FLAG;
	if (replay[4])
		Status_flags |= R_FLAG;
	/*  wait for host reply */
	for (Tm = Millibuzz * Host_ack; Tm-- && (!(Status_flags & (R_FLAG | S_FLAG)));) {
		if (out_cnt = get3270()) {
			if (Blink_msgs)
				lampoff(2);
			goto returnmsg;
		}
	}
	if (Blink_msgs)
		lampoff(2);
	if (outb.ftdone == EACKNAK)
		outb.ftdone = 0;
	if (!(Status_flags & R_FLAG)) {
		logwrite((u_char)0xa3);
		logdump();
		if (--shots > 0) {
			if ((out_cnt = Sure_reset())) {
returnmsg:
				Status_flags &= ~(R_FLAG | S_FLAG | W_FLAG);
				return (out_cnt);
			}
#ifdef DEBUG
			if (Status_flags & S_FLAG) {
				DT("Take another shot, got S_FLAG\n");
			} else {
				DT("Take another shot, got no cent r\n");
			}
#endif /* DEBUG */
			Status_flags &= ~(R_FLAG | S_FLAG | W_FLAG);
			goto again;
		} else {
#ifdef DEBUG
			DT("EXIT from send_str with no host action\n");
#endif /* DEBUG */
			if (Status_flags & S_FLAG) {
				returnval = 3;
				goto returnerr;
			} else {
				returnval = 2;
returnerr:
				Status_flags &= ~(R_FLAG | S_FLAG | W_FLAG);
#ifdef DEBUG
				DT("return from send_str with err %d\n",returnval);
#endif /* DEBUG */
				return (returnval);
			}
		}
	}
#ifdef DEBUG
	DT("ebx_w-");
#endif /* DEBUG */
	Status_flags &= ~(R_FLAG | S_FLAG | W_FLAG);
	return 0;
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
*	Set send_type to ASCII and call send_str.
*
*  RETURNS:
*	ERROR - file receive error (outb.ftdone tells why)
*	n>16 - file received
*
*  EXTERNALS USED:
*	send_type, send_str
*
************************************************************************/

long
send_asc_str(str, len)
u_char *str;
short len;
{

	send_type = 1;
	return (send_str(str, len));
}

long
send_binary_str(str, len)
u_char *str;
short len;
{

	send_type = 2;
	return (send_str(str, len));
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
*	Set send_type to ASCII and call send_str.
*
*  RETURNS:
*	ERROR - file receive error (outb.ftdone tells why)
*	n>16 - file received
*
*  EXTERNALS USED:
*	send_type, send_str
*
************************************************************************/

long
send_ebx_str(str, len)
u_char *str;
short len;
{

	send_type = 0;
	return (send_str(str, len));
}


long
force_str(str, len)
u_char *str;
short len;
{

	register u_char *bp;
	register long out_cnt;

#ifdef DEBUG
	DT("Len = %x\n", len);
#endif /* DEBUG */
	if (len <= 0) {
#ifdef DEBUG
		DT("EXIT from force_str with status = 1\n");
#endif /* DEBUG */
		return (1);
	} else if (len > HUNDRED) {
#ifdef DEBUG
		DT("EXIT from force_str with status = 3\n");
#endif /* DEBUG */
		return (3);
	}
	if (replay[1])
		return 0;
	if (out_cnt = get3270())
		return (out_cnt);
	Cursor_addr = COLS;			/* home */
	while (len > 0) {
		bp = buf;
		*bp++ = DIR_CMD_SETH;
		*bp++ = 0;
		*bp++ = DIR_CMD_SETL;
		*bp++ = COLS;
		*bp++ = DIR_CMD_WD;
		if (send_type) {
			while (len-- )
				*bp++ = FxAsc_xlat[*str++]; /* File Trx wpc */
		} else {
			while (len-- )
				*bp++ = Ebx_xlat[*str++];
		}
	}
	send_x_key(X_RESET);
	(void)prep_write();
	if (write(tfd, buf, bp - buf) == ERROR)
			return (7);
#ifdef DEBUG
	DT("Returned from write with positive status\n");
#endif /* DEBUG */
	send_x_key(X_ENTER);
	return 0;
}


long
force_asc_str(str, len)
u_char *str;
long len;
{
	send_type = 1;
	return (force_str(str, len));
}
long
force_ebx_str(str, len)
u_char *str;
long len;
{
	send_type = 0;
	return (force_str(str, len));
}


format_down_reply(filelength) 
register long filelength;
{
	register u_char *where = pxl.dma_buf;
	register u_char c, count;
	register long size;

	*where++ = ADOLLAR;  /* use 0x80 so FxAsc_xlat gives 0x46  wpc */
	*where++ = 't';
	*where++ = '0';
	*where++ = '0';
	*where++ = '0';
	*where++ = '0';
	*where++ = '2';
	*where = '7';
	where += 8;
	*where-- = ';';
	size = filelength;
	count = 6;
	do {
		c = size & 0x0f;		/* convert hex */
		if (c > 9)
			c += 7;			/* to letter */
		c += '0';
		*where-- = c;
		size >>= 4;
	} while (--count);
	*where = 'D';
	where += 14;
	*where-- = ';';
	size = Buffer_is_mem ? 4*SMALL_DMA : SMALL_DMA;
	count = 6;
	do {
		c = '0' + size % 10;		/* convert to decimal */
		size /= 10;
		*where-- = c;
	} while (--count);
	where += 13;
	size = MaxRU;
	count = 6;
	do {
		c = '0' + size % 10;		/* convert to decimal */
		size /= 10;
		*where-- = c;
	} while (--count);
	where += 10;
	size = Record_sep;
	count = 3;
	do {
		c = '0' + size % 10;		/* convert to decimal */
		size /= 10;
		*where-- = c;
	} while (--count);
	*where = ';';
}


format_reply(size, filelength)
register long size, filelength;
{
	register u_char *where = pxl.dma_buf;
	register u_char c;
	register long count, total;

	total = size;
	while (size-- )
		*(where+size+8) = *(where+size);
	*where++ = ADOLLAR;      /* use 0x80 so FxAsc_xlat gives 0x46 wpc */
	*where = 't';
	where += 6;
	size = total + 6;			/* include this length */
	for (count = 6; count--; ) {
		c = size & 0x0f;		/* convert hex */
		if (c > 9)
			c += 7;			/* to letter */
		c += '0';
		*where-- = c;
		size >>= 4;
	}
	where += 13;
	size = filelength;
	for (count = 6; count--; ) {
		c = size & 0x0f;		/* convert hex */
		if (c > 9)
			c += 7;			/* to letter */
		c += '0';
		*where-- = c;
		size >>= 4;
	}
}

long
get_length(msgpointer)
register u_char *msgpointer;
{
	register char c;
	register long filelength = 0;

	if (Outbfound == DATA2RXFER) {		/* binary */
		filelength = *msgpointer;
		filelength <<= 8;
		filelength |= *++msgpointer;
		filelength <<= 8;
		filelength |= *++msgpointer;
	} else {				/* text */
		for (c = 5; c-- ; ) {
			if (*msgpointer > '9')
				*msgpointer -= 7;
			filelength |= *msgpointer - '0';
			filelength <<= 4;
			msgpointer++;
		}
		if (*msgpointer > '9')
			*msgpointer -= 7;
		filelength |= *msgpointer - '0';
	}
	return (filelength);
}


got_killed()
{
#ifdef DEBUG
	DT("got a cent k ");
#endif /* DEBUG */
	if (dlfd > 0) {
		close(dlfd);
		outb.host_len = 0;
#ifdef DEBUG
		DT("dlfd %d\n",dlfd);
#endif /* DEBUG */
	}
	dlfd = ERROR;
	if (Msg_proc == FXFER) {
		printf("file xfer terminated by host ");
		Want_ack = 0;
		Status_flags |= S_FLAG;
		Msg_proc = 0;
	}
	Msgtype = 0;
}



/*
**	Prep_text prepares a text file for upload by creating an expanded
**	version with an uppercase name. It expands tabs to 8 cols, and
**	works on the basis of 80 col mainframe records.
*/
prep_text(ufile,type)
char *ufile,type;
{
	register u_char *bp, *path, *str;
	register short flen, length, slength;
	u_char c, nbuf[COLS];
	int incomment, iread, iwrite, jread, readcount, uppr;
	long llength;

	incomment = iread = iwrite = readcount = uppr = 0;
	if (type == 'f') {
		printf(" (fortran conversion) ");
		incomment = 0;		/* was 1; now 0 = will convert wpc */
		uppr++;
	}
	if ((lfd = (stat(ufile, &Fstat))) < 0) {
		printf(" File open failed\n");
		return;
	}
	llength = Fstat.st_size;
	if ((lfd = (open(ufile, O_RDONLY,"r"))) <= 0) {
		printf(" File open failed\n");
		return;
	}
	bp = (u_char *)ufile;
	for (slength = COLS; slength && *bp; bp++, slength--) {
		if (*bp == '/')
			readcount++;
		else if (*bp == '.')
			iwrite = 1;
	}
	bp = (u_char *)ufile;
	path = nbuf;
	length = COLS - slength;
	while (length--) {
		if (readcount) {
			if (*bp == '/')
				readcount--;
			bp++;
		} else {
			if (islower(*bp))
				*path =  _toupper(*bp);
			else {
				*path = *bp;
				if (*bp == '.' && iread < 9)
					iread = -1; /* . doesn't count */
			}
			bp++; iread++; path++;
		}
	}
	if (iwrite == 0) {		/* no . in name */
		bp = ptext;
		do {
/*		while (length < sizeof(ptext)) {*/
			*path++ = *bp++;
			length++;
		} while (*bp);
	}
	*path = 0;

	path = nbuf;
	if (iread >= 9) {
		printf("move %s to a name with 8 chars for VM/CMS upload",
		path);
	} else
		printf("file %s ",path);
	dlfd = ERROR;
	if ((dlfd = (open(path, (O_RDWR|O_CREAT),0x1b6))) <= 0) {
		printf("Creat of %s ", path);
		(void)perror("failed\n");
		return;
	}

	length = 0;
	while (llength) {
		if ((iread = read(lfd, lbuffer, sizeof(lbuffer))) <= 0) {
			if (iread) {
				printf ("read %s ",ufile);
				(void)perror ("failed ");
			}
			printf("read 0 llength %d ",llength);
			break;
		}
#ifdef DEBUG
		DT("r%d ",iread);
#endif /* DEBUG */
		jread = iread;
		bp = lbuffer;
		str = nbuffer;
		while (iread--) {
			if (*bp == '\t') {
				flen = length % 8 ? (8 - (length % 8)) : 8;
				while (flen--) {
					*str++ = ' ';
					length = ++length % COLS ? length : 0;
					if (str - nbuffer >= sizeof(nbuffer)) {
						if ((iwrite = write(dlfd,nbuffer, str - nbuffer)) < 0) {
#ifdef DEBUG
							DT("bad write %d ",errno);
#endif /* DEBUG */
							(void)perror("write failed ");
							break;
						}
#ifdef DEBUG
						DT("w%d\n",str-nbuffer);
#endif /* DEBUG */
						str = nbuffer;
					}
				}
				bp++;
			} else if (*bp == '\n' || *bp == '\f') {
				incomment = 0;
				flen = COLS - (length % COLS);
				if (!(length == 0 && isprint(*(bp-1)))) {
					while (flen-- > 0) {
						*str++ = ' ';
						if (str - nbuffer >= sizeof(nbuffer)) {
							if ((iwrite = write(dlfd,nbuffer, str - nbuffer)) < 0) {
#ifdef DEBUG
								DT("bad write %d ",errno);
#endif /* DEBUG */
								(void)perror("write failed ");
								break;
							}
#ifdef DEBUG
							DT("w%d\n",str-nbuffer);
							DT("f-%d l+%d ",flen,length);
#endif /* DEBUG */
							str = nbuffer;
						}
					}
				}
				bp++;
				length = 0;
			} else if (uppr) {
				c = *bp;
				if (!incomment && *(bp-1) == '\n' &&
					(c == 'c' || c == 'C')) {
					c = 'C';
					incomment = 1;
				}
				if (!incomment && islower(c))
					c -= ' ';
				*str++ = c;
				bp++; length++;
			} else {
				*str++ = *bp++;
				length++;
			}
			length = length % COLS ? length : 0;
			if (str - nbuffer >= sizeof(nbuffer)) {
				if ((iwrite = write(dlfd,nbuffer, str - nbuffer)) < 0) {
#ifdef DEBUG
					DT("bad write %d ",errno);
#endif /* DEBUG */
					(void)perror("write failed ");
					break;
				}
#ifdef DEBUG
				DT("w%d\n",str-nbuffer);
#endif /* DEBUG */
				str = nbuffer;
			}
		}
		if ((iwrite = write(dlfd,nbuffer, str - nbuffer)) < 0) {
#ifdef DEBUG
			DT("bad write %d ",errno);
#endif /* DEBUG */
			(void)perror("write failed ");
			break;
		}
#ifdef DEBUG
		DT("w-%d\n",iwrite);
#endif /* DEBUG */
		llength -= jread;
		if (llength <= 0)
			length = 0;
	}
#ifdef DEBUG
	DT(" done ");
#endif /* DEBUG */
	printf(" done ");
	if (dlfd > 0)
		close(dlfd);
}


long
prep_write()
{
	long loop_cntr, out_cnt;
	int shots = NSHOTS + 1;

prep_again:
	do {
		if (out_cnt = get3270())
			return (out_cnt);
		if (Msg_proc == 0)		/* got_killed */
			return 1;
	} while (Control_reg & KEY_INHIBIT);
	send_x_nowait(X_ER_EOF);
	Clear_flag = 0;
	Curhi_ctr = 0;
	for (loop_cntr = Millibuzz * GET_ONE_SEC; loop_cntr-- ; ) {
		if (out_cnt = get3270())
			return (out_cnt);
		if (Msg_proc == 0)		/* got_killed */
			return 1;
		if (Curhi_ctr) {
			if (shots--)
				goto prep_again;
		}
		if (Clear_flag && Cursor_addr == COLS)
			return 0;
		if (outb.ftdone) {
#ifdef DEBUG
			DT("Prep_write reads ftdone %d\n",outb.ftdone);
#endif /* DEBUG */
			outb.ftdone = 0;
		}
	}
#ifdef DEBUG
	DT("Prep_write %d no Clear_flag!!\n",Millibuzz * GET_ONE_SEC);
#endif /* DEBUG */
	return 0;				/* go for it */
}


rglmulator(wasread)
long wasread;
{

	register u_char cls, cms;
	extern u_char sbuf[];

	rp = pxl.dma_buf;
	while (!*rp) {
		rp++;
		if (!--wasread)
			return;
	}
	rc = wasread - 3;			/* 3 byte length starts messge*/
	if (*rp != TESC)
		return;
	cls = *(rp+1);
	cms = *(rp+2);
	/* test for slowcom, fastcom, ginit */
	if (!((cms == ' ' && (cls == ' ' || cls == '!')) || /* slow, fast */
		(cms == '!' && cls == '3')))	/* xginit */
		return;
#ifdef DEBUG
	DT("RGL ");
#endif /* DEBUG */
	conclose(1);
	if (dflag[2]) {
		hostloginit();
		sbuf[1] = (u_char)(rc >> 8);
		sbuf[2] = (u_char)rc;
		fwrite(sbuf, 1, 3, rlf);	/* save rc for replay */
		fwrite(rp, 1, rc, rlf);
		errno = 0;
	}
	context = TEXT;
	readhost();
	conopen(1);
#ifdef DEBUG
	DT("RGL done ");
#endif /* DEBUG */
	Msg_proc = 0;
}


/*
 *   send ack to host
 */
send_ack()
{
	long loop_cntr;

	if (Msg_proc != FXFER || Msgtype) {
#ifdef DEBUG
		DT("SEND_ACK-");
#endif /* DEBUG */
		if (!F3174) {
			send_x_nowait(X_ER_EOF);
			Clear_flag = 0;
			for (loop_cntr = Millibuzz * GET_ONE_SEC; loop_cntr-- ; ) {
				emulint();
				if (Clear_flag)
					break;
			}
			Clear_flag = 0;
#ifdef DEBUG
			if (loop_cntr > 0)
				DT("%d+",(Millibuzz )- loop_cntr);
			else
				DT("NO Clear_flag-");
#endif /* DEBUG */
		}
		send_x_nowait(X_ENTER);
		Control_reg |= KEY_INHIBIT;
	}
}


/*
**
**	Send_big_file reads and sends the named path file to the host
**	by the screen full.
**	1. Find path (return 9 if path not found)
**	2. Get size of path (return 9 if size is zero or erroneous)
**	3. Open path for read (return 8 if open fails)
**	4. Store buck,f, encoded length or buck, t, text length
**	5. Read up to 1434 bytes from path (return 10 if read error)
**	6. Encode data 4:3 for trasnsmission
**	7. Direct write up to HUNDRED bytes in screen
**	8. Verify direct write
**	9. Add buck,q if more data will follow
**	10. Send Enter with encoded data
**	11. If more remains, read into double buffer
**		 (return 10 if read error)
**	12. If reply is S_FLAG, repeat 6,7,8,9 up to NSHOTS shots
**		(return 2 if NSHOTS all fail)
**	13. If reply is R_FLAG, continue
**	14. Return if no bytes read (done)
**	15. Store buck,c at home
**	16. go to 6
**
*  RETURNS:
*	ERROR - file receive error (outb.ftdone tells why)
*	0 - string written (normal exit)
*	1 - file length 0
*	2 - no response from host processor after enter
*	5 - host is not connected to us (4Ac)
*	6 - write RETRIES exceeded
*	7 - direct write failed
*	8 - open of path failed
*	9 - path not readable
*	10 - disk read error
*	n>16 - file received
*/
long
send_big_file(path)
char *path;
{
	register u_char *bp, *str;
	register short length, slength;
	int iread, lntoggle = 0, readcount = 0, readnext = 1;
	u_char retries, *nstr, *string;
	long len, llength, out_cnt, returnval, seglen, sendlength = 0, shots;
	extern long prep_write();

	if (replay[1])
		return (0);
#ifdef DEBUG
	DT("ENTER send_big_file sending type %x\n", send_type);
#endif /* DEBUG */
	if ((len = (stat(path, &Fstat))) < 0) {
		printf(" stat of send file failed, status %d errno %d\n",len,errno);
#ifdef DEBUG
		DT("bad status %d errno %d\n",len,errno);
#endif /* DEBUG */
		returnval = 9;
		goto returnerr;
	}
	llength = Fstat.st_size;
	if (!Fstat.st_size) {
		printf("zero length file ");
#ifdef DEBUG
		DT("zero length file ");
#endif /* DEBUG */
		returnval = 9;
		goto returnerr;
	}
#ifdef DEBUG
	DT("size is %d\n",llength);
#endif /* DEBUG */
	if ((lfd = (open(path, O_RDONLY,"r"))) <= 0) {
		printf(" file open failed ");
#ifdef DEBUG
		DT("EXIT from send_big_file with status = 8\n");
#endif /* DEBUG */
		returnval = 8;
		goto returnerr;
	}
/* 1912 = 1920 - $t length */
/* 1434 = 1920 - $f 4:3 coding overhead (2 unused bytes at end of screen) */
	if (send_type != 2) {
		if ((iread = read(lfd, lbuffer, 1912)) <= 0) {
			printf(" disk read error, errno %d ",errno);
			returnval = 10;
			goto returnerr;
		}
	} else {
		if ((iread = read(lfd, lbuffer, 1434)) <= 0) {
			printf(" disk read error, errno %d ",errno);
			returnval = 10;
			goto returnerr;
		}
	}
#ifdef DEBUG
	readcount++;
	DT("read %d bytes from path\n",iread);
#endif /* DEBUG */
	str = lbuffer;
	len = (llength < iread) ? llength : iread;
	llength -= iread;

reset_again:
	Status_flags &= ~(R_FLAG | S_FLAG | W_FLAG);
#ifdef DEBUG
	DT("reset_again\n");
#endif /* DEBUG */
	if ((out_cnt = Sure_reset()) > 0) {
		goto returnmsg;
	} else if (out_cnt == ERROR) {
		if ((out_cnt = Sure_x_aid(X_ENTER)) > 0) {
			goto returnmsg;
		}
		printf(" failed to reset keyboard ");
#ifdef DEBUG
		DT("EXIT from send_big_file with status = 5\n");
#endif /* DEBUG */
		returnval = 5;
		goto returnerr;
	}
	shots = NSHOTS;
again:

	string = str;				/* data source */
	if ((out_cnt = prep_write()) > 0) {
		goto returnmsg;
	} else if (getspot(8))
		goto reset_again;
	Cursor_addr = COLS;			/* forced home */
	scursor_addr = Cursor_addr;
	bp = buf;
	*bp++ = DIR_CMD_SETH;
/*#ifdef DEBUG
	DT("cursor addr %x\n",Cursor_addr);
#endif /* DEBUG */
	*bp++ = 0;
	*bp++ = DIR_CMD_SETL;
	*bp++ = COLS;
	*bp++ = DIR_CMD_WD;
	if (!sendlength++) {
		seglen = HUNDRED;
		if (send_type != 2) {
			if (send_type == 1 && *str == ADOLLAR) /* 0x80 wpc */
				bp--;
			else {
				*bp++ = DOLLAR;
				*bp++ = TEXT2RXFER;
				bp += 5;
				out_cnt = llength + iread + 6;
				for (length = 6; length--; ) {
					retries = out_cnt & 0x0f;
					if (retries > 9)
						retries += 7;	/* to letter */
					retries += 0x30;
					*bp-- = FxAsc_xlat[retries]; /*  wpc */
					out_cnt >>= 4;
				}
				bp += 7;
				scursor_addr += 8;
			}
		} else {
			*bp++ = DOLLAR;
			*bp++ = F3274;
			Tm = llength + iread + 3;
			s[2] = (u_char)Tm;
			Tm >>= 8;
			s[1] = (u_char)Tm;
			Tm >>= 8;
			s[0] = (u_char)Tm;
			bp += bin_encode(s, bp, 3);
			scursor_addr += 6;
			seglen = 192;
		}
	} else {
		*bp++ = DOLLAR;
		if (send_type == 2)
			*bp++ = DATA3RXFER;
		else
			*bp++ = TEXT3RXFER;
		scursor_addr += 2;
	}
	slength = len;
	Tm = load_segment(str, bp, seglen, slength);
	if (Tm > 0) {
		out_cnt = Tm;
		goto returnmsg;
	} else if (Tm < 0) {
		if (Tm == SEG_ERR) {
			shots--;
			if ((out_cnt = Sure_reset()) > 0)
				goto returnmsg;
			goto again;
		}
		returnval = -Tm;
		goto returnerr;
	}
	str = string;
	Status_flags &= ~(R_FLAG | S_FLAG);
	if (Blink_msgs)
		lampon(2);
	send_x_nowait(X_ENTER);
	Status_flags |= W_FLAG;
#ifdef DEBUG
	DT("sent ENTER key ");
#endif /* DEBUG */
	if (replay[1])
		Status_flags |= R_FLAG;
	/*  wait Host_ack sec for host reply */
	for (Tm = Millibuzz * Host_ack; Tm-- && (!(Status_flags & (R_FLAG | S_FLAG)));) {
		if (llength && readnext) {
			if (!lntoggle) {
/* 1918 = 1920 - $m */
/* 1437 = (1920 - ($c + 2 spare bytes)) * 3/4 */
				if (send_type != 2) {
					if ((iread = read(lfd, nbuffer, 1918)) <= 0) {
						printf(" disk read error, errno %d ",errno);
						returnval = 10;
						goto returnerr;
					}
				} else {
					if ((iread = read(lfd, nbuffer, 1437)) <= 0) {
						printf(" disk read error, errno %d ",errno);
						returnval = 10;
						goto returnerr;
					}
				}
				nstr = nbuffer;
				lntoggle = 1;
			} else {
				if (send_type != 2) {
					if ((iread = read(lfd, lbuffer, 1918)) <= 0) {
						printf(" disk read error, errno %d ",errno);
						returnval = 10;
						goto returnerr;
					}
				} else {
					if ((iread = read(lfd, lbuffer, 1437)) <= 0) {
						printf(" disk read error, errno %d ",errno);
						returnval = 10;
						goto returnerr;
					}
				}
				nstr = lbuffer;
				lntoggle = 0;
			}
#ifdef DEBUG
			readcount++;
			DT("read(#%d) x%x bytes x%x remains\n",readcount,iread,llength);
#endif /* DEBUG */
			readnext = 0;
		}
		if (out_cnt = get3270()) {
			if (Blink_msgs)
				lampoff(2);
			goto returnmsg;
		}
	}
	if (Blink_msgs)
		lampoff(2);
	if (outb.ftdone == EACKNAK)
		outb.ftdone = 0;
	if (!(Status_flags & R_FLAG)) {
		logwrite((u_char)0xa4);
		logdump();
		if (--shots > 0) {
#ifdef DEBUG
			if (Status_flags & S_FLAG) {
				if (Msgtype == 0) {
					returnval = 2;
					goto returnerr;
				}
				DT("Take another(%d( shot, got S_FLAG\n",shots);
			} else
				DT("Take another(%d) shot, didn't get R_FLAG \n",shots);
#endif /* DEBUG */
			Status_flags &= ~(R_FLAG | S_FLAG);
			if (sendlength == 1)
				sendlength = 0;
			if ((out_cnt = Sure_reset())) {
				goto returnmsg;
			}
			goto again;
		} else {
			printf("%d bytes remained with no host reply ",llength);
			returnval = 2;
returnerr:
			Status_flags &= ~(R_FLAG | S_FLAG | W_FLAG);
#ifdef DEBUG
			DT("EXIT from send_str with no host action(%d)\n",
				returnval);
#endif /* DEBUG */
			return(returnval);
		}
	}
#ifdef DEBUG
	DT("ebx_w\n");
#endif /* DEBUG */
	llength -= iread;
	if (llength >= 0 ) {
		readnext = 1;
		len = iread;
		str = nstr;
		if ((out_cnt = Sure_reset())) {
returnmsg:
			Status_flags &= ~(R_FLAG | S_FLAG | W_FLAG);
			return (out_cnt);
		}
		Status_flags &= ~(R_FLAG | S_FLAG);
		shots = NSHOTS;
		goto again;
	}
	close(lfd);
	Status_flags &= ~(R_FLAG | S_FLAG | W_FLAG);
#ifdef DEBUG
	DT("EXIT send_big_file\n");
#endif /* DEBUG */
	return 0;
}


test_xfer_type(wasread)
long wasread;
{

	register u_char cls, cms;
	extern u_char sbuf[];

#ifdef DEBUG
	DT("test_xfer %d \n", wasread);
#endif /* DEBUG */
	rp = pxl.dma_buf;
	while (!*rp) {
		rp++;
		if (!--wasread) {
#ifdef DEBUG
			DT("EXIT on all nulls in msg\n");
#endif /* DEBUG */
			return;
		}
	}
#ifdef DEBUG
	DT("to FXFER \n");
#endif /* DEBUG */
	Msg_proc = FXFER;
	rc = wasread - 3;			/* 3 byte length starts messge*/
	if (*rp != TESC)
		return;
	cls = *(rp+1);
	cms = *(rp+2);
	/* test for slowcom, fastcom, ginit */
	if (!((cms == ' ' && (cls == ' ' || cls == '!')) || /* slow, fast */
		(cms == '!' && cls == '3')))	/* xginit */
		return;
	Msg_proc = RGLXFER;
#ifdef DEBUG
	DT("to RGLXFER \n");
#endif /* DEBUG */
}


/*
**	updown does file transfer under the t3279 emulation.
**	It is called by reload with a received message.
**	The messages all begin direction(c),length(ccc)
**
*/
updown(wassent)
long wassent;
{
	u_char *filename, *msgpointer;
	u_char *path;
	u_char filedir;
	static u_char adollark[3] = {ADOLLAR, 'k', '\0'}; /* o tilde k wpc */
	static u_char type_of_xfer;
	static long filelength;
	long bort, wassaved;

#ifdef DEBUG
	DT("wassent %d ",wassent);
#endif /* DEBUG */
	if (!wassent) {
		Msg_proc = 0;
		return 0;
	}
	msgpointer = pxl.dma_buf;
	filedir = *msgpointer;
#ifdef DEBUG
	DT("Msgtype %d\n", Msgtype);
#endif /* DEBUG */
	if (Outbfound == DATA2RXFER || Outbfound == DATA3RXFER) {
		filename = pxl.dma_buf+4;
	} else {
		filename = pxl.dma_buf+7;
	}
	switch(Msgtype)
	{
	case 0:					/* Transfer Request? */
		type_of_xfer = filedir;
		/* convert name to ASCII and open the file */
		path = pathname;
		wassaved = wassent;
		wassent -= 8;
		if (Outbfound == DATA2RXFER) {
			while (--wassent > 0) {
				*path++ = (u_char)Ebc2asc[*filename++];
#ifdef DEBUG
				DT("%c",*(path-1));
#endif /* DEBUG */
				if (*(path-1) == ' ')
					*(path-1) = 0;
			}
		} else {
			wassent -= 5;
			while (--wassent > 0) {
				*path++ = *filename++;
#ifdef DEBUG
				DT("%c",*(path-1));
#endif /* DEBUG */
			}
		}
		*path = 0;     
		printf("%s ",pathname);
		path = pathname;
		/* if download, get filelength */
		if (filedir == 0 || filedir == 'd') {
#ifdef DEBUG
			DT(" download ");
#endif /* DEBUG */
			filelength = get_length(msgpointer + 1);
#ifdef DEBUG
			DT("length %d ",filelength);
#endif /* DEBUG */
			if (!filelength)
				goto returnerrormsg;
#ifdef DEBUG
			if (Outbfound == DATA2RXFER)
				DT("binary ");
			else
				DT("text ");
#endif /* DEBUG */
			format_down_reply(filelength);
			send_type = 1;
			if ((dlfd = (creat(path, 0666))) <= 0) {
#ifdef DEBUG
				DT("Creat failed ");
#endif /* DEBUG */
				goto returnerrormsg;
			}
			outb.ftdone = 0;
			(void)kill_outb();
			if (*(pxk.rdp+6) == CENT && *(pxk.rdp+7) == KILLIT)
				*(pxk.rdp+7) = (u_char)0x88;
			else if (*(pxk.rdp+1) == CENT && *(pxk.rdp+2) == KILLIT)
				*(pxk.rdp+2) = (u_char)0x88;
			if (lfd = send_str(pxl.dma_buf, 33)) {
#ifdef DEBUG
				DT("send conversed %d\n",lfd);
#endif /* DEBUG */
				goto returnerrormsg;
			}
#ifdef DEBUG
			DT("Transfer reply sent - ");
#endif /* DEBUG */
			Msgtype = 3;
		} else if (filedir == 1 || filedir == 'u') {
#ifdef DEBUG
			DT(" upload ");
#endif /* DEBUG */
			if ((lfd = (stat(path, &Fstat))) < 0) {
#ifdef DEBUG
				DT("path bad status errno %d\n",errno);
#endif /* DEBUG */
				goto returnerrormsg;
			}
			if (Outbfound == DATA2RXFER) {
				send_type = 2;
				filelength = Fstat.st_size;
				msgpointer += 4;
				*msgpointer-- = 0;
				*msgpointer-- = filelength & 0xff;
				filelength >>= 8;
				*msgpointer-- = filelength & 0xff;
				filelength >>= 8;
				*msgpointer = filelength & 0xff;
			} else {
				send_type = 1;
				filelength = Fstat.st_size;
				format_reply(wassaved, filelength);
				wassaved += 2;	/* was 8; 6 too many wpc */
			}
#ifdef DEBUG
			DT("%x bytes in file\n",filelength);
#endif /* DEBUG */
			if (*(pxk.rdp+6) == CENT && *(pxk.rdp+7) == KILLIT)
				*(pxk.rdp+7) = (u_char)0x88;
			else if (*(pxk.rdp+1) == CENT && *(pxk.rdp+2) == KILLIT)
				*(pxk.rdp+2) = (u_char)0x88;
			outb.ftdone = 0;
			(void)kill_outb();
			if ((lfd = send_str(pxl.dma_buf, wassaved))) {
#ifdef DEBUG
				DT("send conversed %d\n",lfd);
#endif /* DEBUG */
				goto killed_out;
			}
#ifdef DEBUG
			DT("Transfer reply sent - ");
#endif /* DEBUG */
			Msgtype = ERROR;
			filelength = send_big_file(path);
			if (filelength)
				goto returnerrormsg;
#ifdef DEBUG
			DT("File sent - ");
#endif /* DEBUG */
			goto senddone;
		}
		break;
	case 3:					/* Receive start of a file */
#ifdef DEBUG
		DT("start of rcv file %d length %d ",filelength,outb.length);
#endif /* DEBUG */
		if (Outbfound == DATA2RXFER) {
#ifdef DEBUG
			DT("binary ");
#endif /* DEBUG */
			bort = 3;
		} else {
#ifdef DEBUG
			DT("text ");
#endif /* DEBUG */
			bort = 6;
		}
		if (wassent - bort > filelength)
			wassent = filelength + bort;
		if ((lfd = write(dlfd, msgpointer, wassent - bort)) < 0)
			goto returnerrormsg;
#ifdef DEBUG
		DT("write %d ",lfd);
#endif /* DEBUG */
		filelength -= wassent;		/* didn't write length field */
		if (filelength <= 0)
			goto rxdone;
#ifdef DEBUG
		DT("filelength left %d\n",filelength);
#endif /* DEBUG */
		Msgtype = 4;
		return CNTU;
		break;
	case 4:					/* Receive part of a file */
#ifdef DEBUG
		DT("rcv part of file msg ");
#endif /* DEBUG */
		if (wassent > filelength)
			wassent = filelength;
		if ((lfd = write(dlfd, msgpointer, wassent)) < 0)
			goto returnerrormsg;
		filelength -= wassent;
#ifdef DEBUG
		DT("write %d left %d\n",lfd,filelength);
#endif /* DEBUG */
		if (filelength <= 0)
			goto rxdone;
		return CNTU;
		break;
	case 5:					/* File Transfer Complete */
senddone:
#ifdef DEBUG
		DT("upload done msg\n");
#endif /* DEBUG */
		/* was send_protocol("$k",2); change to 0x80 ADOLLAR  wpc */
		send_protocol(adollark,2);   /* static o tilde k ADOLLAR wpc */
rxdone:
		printf("done ");
		if (Blink_msgs)
			printf("\007");
#ifdef DEBUG
		DT("file xfer done\n");
#endif /* DEBUG */
		close(dlfd);
		dlfd = ERROR;
		Msgtype = 0;
		Msg_proc = 0;
		break;
	default:
returnerrormsg:
#ifdef DEBUG
		DT("return error msg ");
#endif /* DEBUG */
		filename = pxl.dma_buf+4;
		if (dlfd > 0)
			close(dlfd);
		dlfd = ERROR;
		Msgtype = 0;
		
		if (filelength == 1) {		/* got_killed in prep_write */
			outb.ftdone = 0;
			(void)kill_outb();
			Msg_proc = 0;
			filelength = 0;
			goto killed_out;
		}
		filelength = 0;
		msgpointer = pxl.dma_buf;
		if (type_of_xfer < 4) {
			*msgpointer++ = 0;
			*msgpointer++ = 0;
			*msgpointer++ = 0;
			*msgpointer++ = 0;
		} else {
			*msgpointer++ = '0';
			*msgpointer++ = '0';
			*msgpointer++ = '0';
			*msgpointer++ = '0';
			*msgpointer++ = '0';
			*msgpointer++ = '0';
		}
#ifdef DEBUG
		DT("errno %d ",errno);
#endif /* DEBUG */
		if (errno) {
			(void)perror("updown error ");
			path = (u_char *)sys_errlist[errno];
			/* append perror to message and reply */
			while (*path) {
#ifdef DEBUG
				DT("%c",*path);
#endif /* DEBUG */
				*msgpointer++ = (u_char)Asc2ebc[*path++];
			}
		}
		outb.ftdone = 0;
		(void)kill_outb();
		Msg_proc = 0;
		send_type = type_of_xfer > 4 ? 0 : 2 ;
		send_str(pxl.dma_buf, msgpointer - pxl.dma_buf);
killed_out:
		printf("file xfer killed ");
		get_screen();
		break;
	}
#ifdef DEBUG
	DT("UPDOWN return\n ");
#endif /* DEBUG */
	return 0;
}

