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
*
************************************************************************/

#include <sys/types.h>
#include <signal.h>
#include "ctype.h"
#include <errno.h>
#include "term.h"
#include "pxw.h"

#define K_CENT		0x5b
#define K_EOF		'D' - '@'	/* return on ^D */
#define K_RESTART	'G' - '@'



/*
**	Externals
*/
extern u_short	fid;
extern void	free();
extern void	longjmp();
extern char	*malloc();
extern px_status outb;
extern long	pxstat();
extern x_key_table X_key_xlat[];
extern int	errno;
extern char	File_xfer;
extern char	Ft_type;
extern char	L_x_key_xlat;
extern u_char	Outbfound;
extern char	Rows;
extern u_char	Status_flags;
extern u_char	UDload;

/*
**	Global variables
*/

outft		frcv;
px_bufs 	pxl;
rglft		rrcv;
u_char		usrbuf[PXDMASIZ];

/*
**	Local variables
*/

#ifndef GL2TERM
static jmp_buf	jbuf;
#endif GL2TERM
px_status 	status;

struct key_table {
	u_char k_mode;		/* mode: NORMAL, SHIFT, ALT, DONTK */
	u_char k_code;
} Key_xlat[] = {
/* space */		{NORMAL,	0x10,},
/*  ! */		{SHIFT,		0x1b,},
/*  " */		{SHIFT,		0x12,},
/*  # */		{SHIFT,		0x23,},
/*  $ */		{SHIFT,		0x24,},
/*  % */		{SHIFT,		0x25,},
/*  & */		{SHIFT,		0x27,},
/*  ' */		{NORMAL,	0x12,},
/*  ( */		{SHIFT,		0x29,},
/*  ) */		{SHIFT,		0x20,},
/*  * */		{SHIFT,		0x28,},
/*  + */		{SHIFT,		0x11,},
/*  , */		{NORMAL,	0x33,},
/*  - */		{NORMAL,	0x30,},
/*  . */		{NORMAL,	0x32,},
/*  / */		{NORMAL,	0x14,},
/*  0 */		{NORMAL,	0x20,},
/*  1 */		{NORMAL,	0x21,},
/*  2 */		{NORMAL,	0x22,},
/*  3 */		{NORMAL,	0x23,},
/*  4 */		{NORMAL,	0x24,},
/*  5 */		{NORMAL,	0x25,},
/*  6 */		{NORMAL,	0x26,},
/*  7 */		{NORMAL,	0x27,},
/*  8 */		{NORMAL,	0x28,},
/*  9 */		{NORMAL,	0x29,},
/*  : */		{SHIFT,		0x7e,},
/*  ; */		{NORMAL,	0x7e,},
/*  < */		{NORMAL,	0x09,},
/*  = */		{NORMAL,	0x11,},
/*  > */		{SHIFT,		0x09,},
/*  ? */		{SHIFT,		0x14,},
/*  @ */		{SHIFT,		0x22,},
/*  A */		{SHIFT,		0x60,},
/*  B */		{SHIFT,		0x61,},
/*  C */		{SHIFT,		0x62,},
/*  D */		{SHIFT,		0x63,},
/*  E */		{SHIFT,		0x64,},
/*  F */		{SHIFT,		0x65,},
/*  G */		{SHIFT,		0x66,},
/*  H */		{SHIFT,		0x67,},
/*  I */		{SHIFT,		0x68,},
/*  J */		{SHIFT,		0x69,},
/*  K */		{SHIFT,		0x6a,},
/*  L */		{SHIFT,		0x6b,},
/*  M */		{SHIFT,		0x6c,},
/*  N */		{SHIFT,		0x6d,},
/*  O */		{SHIFT,		0x6e,},
/*  P */		{SHIFT,		0x6f,},
/*  Q */		{SHIFT,		0x70,},
/*  R */		{SHIFT,		0x71,},
/*  S */		{SHIFT,		0x72,},
/*  T */		{SHIFT,		0x73,},
/*  U */		{SHIFT,		0x74,},
/*  V */		{SHIFT,		0x75,},
/*  W */		{SHIFT,		0x76,},
/*  X */		{SHIFT,		0x77,},
/*  Y */		{SHIFT,		0x78,},
/*  Z */		{SHIFT,		0x79,},
/*  cents([) */		{NORMAL,	0x1b,},
/*  \ */		{NORMAL,	0x15,},
/*  vert bar(]) */	{SHIFT,		0x21,},
/*  logic not(^) */	{SHIFT,		0x26,},
/*  _ */		{SHIFT,		0x30,},
/*  ` */		{NORMAL,	0x3d,},
/*  a */		{NORMAL,	0x60,},
/*  b */		{NORMAL,	0x61,},
/*  c */		{NORMAL,	0x62,},
/*  d */		{NORMAL,	0x63,},
/*  e */		{NORMAL,	0x64,},
/*  f */		{NORMAL,	0x65,},
/*  g */		{NORMAL,	0x66,},
/*  h */		{NORMAL,	0x67,},
/*  i */		{NORMAL,	0x68,},
/*  j */		{NORMAL,	0x69,},
/*  k */		{NORMAL,	0x6a,},
/*  l */		{NORMAL,	0x6b,},
/*  m */		{NORMAL,	0x6c,},
/*  n */		{NORMAL,	0x6d,},
/*  o */		{NORMAL,	0x6e,},
/*  p */		{NORMAL,	0x6f,},
/*  q */		{NORMAL,	0x70,},
/*  r */		{NORMAL,	0x71,},
/*  s */		{NORMAL,	0x72,},
/*  t */		{NORMAL,	0x73,},
/*  u */		{NORMAL,	0x74,},
/*  v */		{NORMAL,	0x75,},
/*  w */		{NORMAL,	0x76,},
/*  x */		{NORMAL,	0x77,},
/*  y */		{NORMAL,	0x78,},
/*  z */		{NORMAL,	0x79,},
/*  { */		{NORMAL,	0x0f,},
/*  | */		{SHIFT,		0x15,},
/*  } */		{SHIFT,		0x0f,},
/*  ~ */		{SHIFT,		0x3d,},
};


/*
**	Turn on/off the tracing of this module
**	Dummy to keep lint happy, no tracing (DT) here
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
	char stats;

	qkeys();
	cursoff();
	normalize_kb();
	clear_screen();
	get_screen();
	show_screen();
	stats = dblator();
	conclose(1);
	return(stats);
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
		    if (c==K_EOF) {
			break;
		    }
		    else if (c==K_RESTART) {
			restart_emulator();
			i = -1; j = 0;
		    } else if (i<0) {
			/* default state */
			if (istviprint(c) || c==0 || (i = xlat_idx(c, 0)) < 0){
			    /* printable, NULL or not in special xlat tbl */
			    if(c == K_CENT){
				if (outb.ft)
				    centflag = 1;
				(void)kill_outb();
			    } else {
				if (centflag) {
					rrcv.bodyaddr = outb.bufp;
					rrcv.bodylen = outb.max_len;
					(void)set_rglout_ptr(&rrcv);
					    centflag = 0;
				}
			    }
			    send_key(c);
			} else {
			    /* 1st char matched at row i, save it */
			    buf[j++] = c;
			}
		    } else {
			/* j chars in buf matched so far */
			/* save input char */
			buf[j++] = c;
			if (c!=X_key_xlat[i].k_stroke[j-1]) {
			    /* input ch doesn't match next ch in seq, */
			    /* search for another row in the table */
			    if ((i = new_x_row(buf, i, j)) < 0) {
				/* search failed, flush buffer */
				for (k=0; k<j; k++)
				    send_key(buf[k]);
				j = 0;
			    }
			}
		    }
		    if (i >= 0 && X_key_xlat[i].k_stroke[j]==0) {
			/* matched chars form a complete seq */
			if (i!=0) {
			    send_x_key(i);
			} else {
			    repaint(0, Rows - 1);
			    show_screen();
			}
			i = -1;  j = 0;
		    }
		}
		if (pxstat()) {
		    if (emulint()) {
			if (outb.ftdone == ESTART) {
				if ((partial = outbread()) > 0) {
				    show_msg();
				    outb.ftdone = 0;
				    reload(partial);
				} else {
				    if (outb.ftdone != EACK)
					    reload(0);
				    else
					outb.ftdone = 0;
				}
			}
			show_status();
		    } else {
			show_screen();
		    }
		}
	}
	send_x_key(X_RESET);
	return (0);
}


istviprint(c)
u_char c;
{
	if ((c < 0x7f) && (c >= 0x20))
		return 1;
	return 0;
}


/*
**	Terminal reader
*/
get3270()
{

	register long partial;

	if (emulint()) {
		while (Outbfound || outb.ftdone) {
			partial = outbread();
			if (partial > 0) {
				outb.ftdone = 0;
				return (partial-3);
			}
			if (outb.ftdone == EACK) {
				Outbfound = 0;
				for (partial = 50000; partial-- && (!errno &&
				(!outb.ftdone || (outb.ftdone == EACK ))); ) {
					outb.ftdone = 0;
					if (emulint())
						break;
				}
			} else {
				return (ERROR);
			}
		}
	} else
		if (errno)
			return (ERROR);
	return (0);
}



/*
**	Terminal emulator
*/
emulator()
{
	u_char c;
	register char stats = 0;

/*
**	Allocate dma and screen buffers
*/
	usropen();
	conopen(0);
	clear_screen();
	File_xfer = 0;
	get_screen();
	show_screen();
	stats  = dblator();
/*
**	Free dma and screen buffers
*/
	usrclose();
	conclose(0);
	return (stats);
}


/*
**	Return a new row # in the x translate tbl where the j chars
**	in the buffer buf match.
*/
new_x_row(buf, i, j)
u_char buf[];
char i, j;
{
	register char k;

	while ((i = xlat_idx(buf[0], ++i)) >= 0) {
	    for (k=1; k<j; ++k)
		if (buf[k]!=X_key_xlat[i].k_stroke[k])
		    break;
	    if (k>=j)
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
	if (c < (u_char)' ' || c > (u_char)'~')
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
	static u_char state = NORMAL;	/* NORMAL, SHIFT, ALT */

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
		break;
	default:
		delay(MS_15);
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
	send_shift_alt(X_key_xlat[i].k_mode);
	send_dly(X_key_xlat[i].k_code);
}


/*
**   Show_msg displays the message in hex on line 25 at a safe spot
*/
show_msg()
{

	static u_char d = (u_char)0x80;
	register u_char c, nibble;
	u_char *str;
	register u_short index, sholength;

	if (context == GRAPHICS)
		return;
	setspot(25,d++);
	str = pxl.dma_buf;
	for (index = 0x32,sholength = 8; sholength--; ) {
		setspot(index++, 0x10); 
		c = *str++;
		nibble = c >> 4;
		nibble &= (u_char)0x0f;
		if (nibble <= 9)
			nibble += (u_char)0x20;
		else
			nibble += (u_char)0x76;
		setspot(index++, nibble);
		nibble = c & (u_char)0x0f;
		if (nibble <= 9)
			nibble += (u_char)0x20;
		else
			nibble += (u_char)0x76;
		setspot(index++, nibble);
	}
}


/*
**	Terminal updater
*/
update3270()
{
	register long got;
	if (pxstat() || outb.ftdone) {
		errno = 0;		/* ignore callers problems */
		got = get3270();
		return (got);
	}
	return (0);
}


/*
**	Allocate dma and screen buffers
*/
usropen()
{
/*	if ((pxl.dma_buf = (u_char *)malloc(PXDMASIZ))==(u_char *)0) {
		 lampon(0x0e);
		exit(1);
	}*/
	pxl.dma_buf = usrbuf;
	pxl.pxt_buf.bufp = pxl.dma_buf;
}


/*
**	Free dma and screen buffers
*/
usrclose()
{
/*	free((char *)pxl.dma_buf);*/
}


/*
**	Return index of c in table X_key_xlat starting from position i.
**	If c is not in the table return -1.
*/
xlat_idx(c, i)
register u_char c;
register char i;
{
	for (; i<L_x_key_xlat; i++)
		if (c==X_key_xlat[i].k_stroke[0])
			return (i);
	return (-1);
}
