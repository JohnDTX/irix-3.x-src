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


/************************ E M U L A T O R . C ****************************
*
*  MODULE DESCRIPTION:
*
*	This module contains the code for the 3279 emulation
*
*
*  ENTRY POINTS:
*
*	db_emulator()		- Debug emulator entry point
*	emulator()		- 3279 emulator entry point
*	get3270()		- Updater for write to host
*	send_shift_alt()	- Send shift/alt make/break as req'd
*	send_key()		- Send a key to the 3274
*	send_x_key()		- Send an aid to the 3274
*	update3270()		- Updater for file rxfer
*	usropen()		- Malloc pxl file xfer memory
*
************************************************************************/

#include <sys/types.h>
#include <sys/signal.h>
#include <setjmp.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include "term.h"
#include "pxw.h"

#define K_BREAK		(u_char)0x85	/* \205 is BREAK */
#define K_EOF		'D' - '@'	/* return on ^D */
#define K_RESTART	'G' - '@'

typedef int (*pfi)();


/*
**	Externals
*/
extern int		fd;
extern void		longjmp();
extern px_status	outb;
extern long		outbread();
extern pfi		signal();
extern u_char		Keyboard;
extern key_table	Key_xlat[];
extern u_char		L_x_key_xlat;
extern u_char		Msg_proc;
extern u_char		Outbfound;
extern u_char		Read_flag;
extern u_char		Rows;
extern u_char		Want_ack;
extern key_table	X_key_codes[];
extern x_key_table	X_key_xlat[];

/*
**	Global variables
*/

outft			frcv;
px_bufs 		pxl;
rglft			rrcv;
u_char			usrbuf[4*PXDMASIZ];
u_char			Noread = 0;

/*
**	Local variables
*/

jmp_buf			jbuf;
/*long			hup_intr();*/


/*
**	Trace this module
*/
tr_emulator(flag)
{
	trace = flag;
}


/*
**	Debug menu entrance
*/
db_emulator()
{
	u_char stats;

	if (!Keyboard)
		qkeys(1);
	else
		conopen(1);
	normalize_kb();
	clear_screen();
	get_screen();
	show_screen();
	stats = dblator();
	conclose(1);
	return (stats);
}


/*
**	Debugging emulator
*/
dblator()
{
	u_char buf[5];
	u_char c, centflag = 0;
	register char i, j, k;
	register long partial;

	for (i = -1, j = 0;;) {
		if (conin(&c)) {
			if (c == K_EOF || c == K_BREAK)
			    break;
			else if (c == K_RESTART) {
			    restart_emulator();
			    i = -1; j = 0;
			} else if (i < 0) {
			    /* default state */
			    if (c==0||((i = xlat_idx(c, 0)) < 0 && isprint(c))){
				/* printable, NULL or not in special xlat tbl */
				if (centflag) {
					rrcv.bodyaddr = outb.bufp;
					rrcv.bodylen = outb.max_len;
					(void)set_rglout_ptr(&rrcv);
					centflag = 0;
				}
				send_key(c);
			    } else
				/* 1st char matched at row i, save it */
				buf[j++] = c;
			} else {
			    /* j chars in buf matched so far */
			    /* save input char */
			    buf[j++] = c;
			    if (c != X_key_xlat[i].k_stroke[j-1]) {
				/* input ch doesn't match next ch in seq, */
				/* search for another row in the table */
				if ((i = new_x_row(buf, i, j)) < 0) {
				    /* search failed, flush buffer */
				    for (k = 0; k < j; k++)
					send_key(buf[k]);
				    j = 0;
				}
			    }
			}
			if (i >= 0 && X_key_xlat[i].k_stroke[j] == 0) {
			    /* matched chars form a complete seq */
#ifdef DEBUG
			    DT("Kbd sequence match at entry # %d\n", i);
#endif /* DEBUG */
			    if (i != 0) {
				if(i == X_CENT) {
					if(outb.ft)
						centflag = 1;
					(void)kill_outb();
				}
			    	send_x_key(i);
			    } else {
			        repaint(0, Rows - 1);
				show_screen();
			    }
			    i = -1;  j = 0;
			}
		}
		if (emulint()) {
			if (outb.ftdone == ESTART) {
				if ((partial = outbread()) > 0) {
					if (Msg_proc != RGLXFER)
						show_msg();
					reload(partial);
				} else
					reload(0);
			}
			if (Want_ack) {
				Want_ack = 0;
				send_ack();
			}
			show_status();
		} else if (Read_flag) {
			show_screen();
			Read_flag = 0;
		}
	}
	send_shift_alt(NORMAL);
	if (c == K_BREAK)
		return 1;
	return 0;
}


/*
**	Terminal emulator
*/
emulator()
{
	u_char stats = 0;
	pfi hi, ii, qi;
	extern hup_intr();

#ifdef DEBUG
	if ((hi = signal(SIGHUP, SIG_IGN)) == (pfi)-1)
		DT("signal() error\n");
	if ((ii = signal(SIGINT, SIG_IGN)) == (pfi)-1)
		DT("signal() error\n");
	if ((qi = signal(SIGQUIT, SIG_IGN)) == (pfi)-1)
		DT("signal() error\n");
#endif /* DEBUG */
/*
**	Allocate dma and screen buffers
*/
	conopen(1);
	normalize_kb();
	clear_screen();
	get_screen();
	show_screen();
	if (setjmp(jbuf) == 0) {
#ifdef DEBUG
		if (signal(SIGINT, hup_intr) == (pfi)-1)
			DT("signal() error\n");
#endif /* DEBUG */
		stats  = dblator();
#ifdef DEBUG
		if (signal(SIGINT, SIG_IGN) == (pfi)-1)
			DT("signal() error\n");
#endif /* DEBUG */
	}
/*
**	Free dma and screen buffers
*/
	conclose(1);
#ifdef DEBUG
	if (signal(SIGHUP, hi) == (pfi)-1)
		DT("signal() error\n");
	if (signal(SIGINT, ii) == (pfi)-1)
		DT("signal() error\n");
	if (signal(SIGQUIT, qi) == (pfi)-1)
		DT("signal() error\n");
#endif /* DEBUG */
	return (stats);
}


/*
**	Terminal reader
*/
long
get3270()
{
	register long partial;

	if (emulint() || Outbfound) {    /* wpc add || Outbfound because 
	emulint will require the services of pxdread.  But if pxdread,
	which reads data from the dma controller, gets only exactly two
	bytes of data and they are begin write and cent (outbound control char)
	c3 and 1b or 45, then pxdread will set the Outbfound flag and throw
	away these two bytes -- meaning returning zero data to emulint.
	Without adding the check for Outbfound flag, the following codes will
	not be exercised.  This condition will result in outbread not being
	called and message will be lost, which may require a retry.  This will 
	not happen in file transfer but will happen in conversation mode.
	This will happen more frequently in slower mainframes or links. wpc */

		if (Outbfound || outb.ftdone == ESTART || outb.ftdone == EACK) {
			partial = outbread();
			if (partial > 0) {
				outb.ftdone = 0;
				Noread = 1;
				emulint();	/* read cent q */
				if (Want_ack) {
					Want_ack = 0;
					send_ack();
				}
				Noread = 0;
				return (partial);
			}
		} else if (Want_ack != 0) {
			Want_ack = 0;
			send_ack();
		}
	} else
		if (errno)
			return (ERROR);
	return (0);
}


/*
**	BREAK interrupt service routine
*/
hup_intr(sig)
{
	lampon(8);
#ifdef DEBUG
	if (signal(SIGINT,SIG_IGN) == (pfi)-1)
		DT("signal() error\n");
#endif /* DEBUG */
	unqkeys();
	lampoff(8);
#ifdef DEBUG
	DT("Signal %d detected\n", sig);
#endif /* DEBUG */
	longjmp(jbuf,1);
}


/*
**	Return a new row # in the x translate tbl where the j chars
**	in the buffer buf match.
*/
new_x_row(buf, i, j)
u_char buf[];
register char i, j;
{
	register char k;

	while ((i = xlat_idx(buf[0], ++i)) >= 0) {
		for (k = 1; k < j; ++k)
			if (buf[k] != X_key_xlat[i].k_stroke[k])
				break;
		if (k >= j)
			break;
	}
	return (i);
}


/*
**	Restart emulator - executed when ^G is pressed
*/
restart_emulator()
{
	get_screen();
	show_screen();
}


/*
**	Send the make/break codes followed by the key code for
**	the given character.
*/
send_key(c)
u_char c;
{
#ifdef DEBUG
	DT("send_key %02x\n",c);
#endif /* DEBUG */
	if (c < ' ' || c > '~')
		return;
	send_shift_alt(Key_xlat[c -= ' '].k_mode);
	send_dly(Key_xlat[c].k_code);
}


/*
**	Send shift/alt make/break as required
*/
send_shift_alt(mode)
u_char mode;
{
	static u_char state = NORMAL;		/* NORMAL, SHIFT, ALT */

	switch (state)
	{
	case NORMAL:
		if (mode==SHIFT)
			send_dly(SHIFT_MAKE);
		else if (mode==ALT)
			send_dly(ALT_MAKE);
		break;
	case SHIFT:
		if (mode==NORMAL)
			send_dly(SHIFT_BREAK);
		else if (mode==ALT) {
			send_dly(SHIFT_BREAK);
			send_dly(ALT_MAKE);
		}
		break;
	case ALT:
		if (mode==NORMAL)
			send_dly(ALT_BREAK);
		else if (mode==SHIFT) {
			send_dly(ALT_BREAK);
			send_dly(SHIFT_MAKE);
		}
	default:
		break;
	}
	state = mode;
}


/*
**	Send the make/break codes followed by the key code for
**	the given row in the x translate tbl.
*/
send_x_key(i)
u_char i;
{
#ifdef DEBUG
	DT("send_x_key %02x\n",i);
#endif /* DEBUG */
	send_shift_alt(X_key_codes[i].k_mode);
	send_dly(X_key_codes[i].k_code);
}


send_x_nowait(i)
u_char i;
{
#ifdef DEBUG
	DT("send_x_nowait %02x\n",i);
#endif /* DEBUG */
	send_shift_alt(X_key_codes[i].k_mode);
	send_pxd(X_key_codes[i].k_code);
}


/*
**   Show_msg displays the message in hex on line 25 at a safe spot
*/
show_msg()
{

	register u_char c, index, length, nibble;
	u_char *str;

	if (outb.ftdone != EDONE)
		return;
#ifdef DEBUG
	DT("show_msg* ");
#endif /* DEBUG */
	str = pxl.dma_buf;
	for (index = 0x32, length = 8; length--; ) {
		setspot(index++, 0x10); 
		c = *str++;
		nibble = c >> 4;
		nibble &= 0x0f;
		/* convert to 3274 codes */
		if (nibble <= 9)
			nibble += 0x20;
		else
			nibble += 0x76;
		setspot(index++, nibble);
		nibble = c & 0x0f;
		if (nibble <= 9)
			nibble += 0x20;
		else
			nibble += 0x76;
		setspot(index++, nibble);
	}
}

/*
**	Terminal updater
*/
long
update3270()
{
	register long got;

	if (pxstat() || outb.ftdone) {
		errno = 0;		/* ignore callers problems */
		got = get3270();
		return (got);
	}
	return 0;
}


/*
**	Allocate dma and screen buffers
*/
usropen()
{
	pxl.dma_buf = usrbuf;
	pxl.pxt_buf.bufp = pxl.dma_buf;
}


/*
**	Return index of c in table X_key_xlat starting from position i.
**	If c is not in the table return -1.
*/
xlat_idx(c, i)
register u_char c;
register char i;
{
	for (; i < L_x_key_xlat; i++)
		if (c == X_key_xlat[i].k_stroke[0])
			return (i);
	return (ERROR);
}


