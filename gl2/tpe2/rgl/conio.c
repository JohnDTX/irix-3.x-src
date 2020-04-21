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

/********************** C O N I O . C ***********************************
*
*  MODULE DESCRIPTION:
*	This module contains all terminal dependent code
*
*  ENTRY POINTS:
*	backgrnd()	- Color the background
*	beep()		- Sound the bell
*	clear_screen()	- Clear terminal's screen
*	conclose()	- Reset the console flags
*	conopen()	- Initialize the console I/O code
*	db_conopen()	- Initialize the console without queuing the keys
*	disp_attrb()	- Display an attribute
*	end_status()	- Send end of user status info sequence to terminal
*	erase_cursor()	- Remove the cursor from the display
*	erase_eol()	- Erase to end of line
*	qkeys()		- Queue the iris console keyboard buttons
*	set_color()	- Specify color for subsequent text output
*	show_cursor()	- Display the cursor
*	start_status()	- Send load user status line command to terminal
*	strnout()	- Display string
*	unqkeys()	- Unqueue the iris console keyboard buttons
*	user_line()	- Enable/disable the display of the user's status line
*	xy(r,c)		- Cursor positioning
*
*  GLOBAL VARIABLES:
*	X_key_xlat	- translate table for special keys
*	L_x_key_xlat	- number of entries in above table
*
************************************************************************/


#include <sys/types.h>
#include <termio.h>
#include <fcntl.h>
#include <gl.h>
#include <device.h>
#include "pxw.h"

#define CTRL	'@'
#define	BS	('H' - CTRL)
#define ESC	0x1b
#define DEL	0x7f
#define HOME	('^' - CTRL)

#define D_ATTRB		0xc0		/* default attribute */

#define CTRL_BIT	0		/* keyboard status bits */
#define SHIFT_BIT	1
#define CAPS_LOCK_BIT	2

/*
**	Externals
*/
extern char	*getenv();
extern int	repainter();
extern int	strcmp();
extern char 	*strcpy();
extern char	*strncpy();
extern int	write();

/*
**	Global variables
*/
char	ttytype;

/*
**	Local variables
*/
static char c_scrn_str[6];	/* clear screen string */
static char end_s_str[6];	/* end of user's status */
static char e_eol_str[6];	/* erase to end of line */
static char strt_s_str[6];	/* user's status lead in */
static char u1_ln_str[6];	/* enable user's status line */
static char u2_ln_str[6];	/* disable user's status line */
static char xy_str[6];		/* cursor positioning string */
static Icoord Scrn_x, Scrn_y;	/* Current graphics position for cmov */
static Colorindex Ch_color;	/* Current character color */

/*  DO NOT CHANGE THE ORDER IN THE TABLE BELOW WITHOUT CHANGING THE DEFINITIONS
**  IN THE INCLUDE FILE
*/
x_key_table X_key_xlat[] = {
	0263, 0, 0, 0, NORMAL, 0,	/* 00 num 3 -> reshow. KEEP IT AT 00! */
	'A'-CTRL, 0, 0, 0, NORMAL, 0x50,	/* 01 ctrl a -> attention */
	BS, 0, 0, 0, NORMAL, 0x31,		/* 02 backspace */
	0206, 0, 0, 0, NORMAL, 0x35,		/* 03 back_tab */
	0215, 0, 0, 0, ALT, 0x51,		/* 04 num enter -> clear */
	0312, 0, 0, 0, NORMAL, 0x13,		/* 05 c_down */
	0310, 0, 0, 0, NORMAL, 0x16,		/* 06 c_left */
	0314, 0, 0, 0, NORMAL, 0x1a,		/* 07 c_right */
	0313, 0, 0, 0, NORMAL, 0x0e,		/* 08 c_up */
	'S'-CTRL, 0, 0, 0, NORMAL, 0x51,	/* 09 ctrl s -> cur_select */
	DEL, 0, 0, 0, NORMAL, 0x0d,		/* 10 delete */
	'C'-CTRL, 0, 0, 0, ALT, 0x34,		/* 11 ctrl c -> dev_cancel */
	'U'-CTRL, 0, 0, 0, NORMAL, 0x5f,	/* 12 ctrl u -> dup */
	'\r', 0, 0, 0, NORMAL, 0x18,		/* 13 return -> enter */
	0261, 0, 0, 0, NORMAL, 0x55,		/* 14 num 1 -> erase_eof */
	0262, 0, 0, 0, ALT, 0x53,		/* 15 num 2 -> er_input */
	'F'-CTRL, 0, 0, 0, NORMAL, 0x5e,	/* 16 ctrl f -> f_mark */
	'\t', 0, 0, 0, NORMAL, 0x36,		/* 17 fwd_tab */
	0256, 0, 0, 0, ALT, 0x35,		/* 18 num . -> home */
	0214, 0, 0, 0, NORMAL, 0x0c,		/* 19 shift no scrl -> insert */
	'\n', 0, 0, 0, NORMAL, 0x08,		/* 20 new_line */
	0275, 0, 0, 0, ALT, 0x5f,		/* 21 ctrl pf1 -> pa1 */
	0276, 0, 0, 0, ALT, 0x5e,		/* 22 ctrl pf2 -> pa2 */
	0201, 0, 0, 0, ALT, 0x21,		/* 23 pfk1 -> pf1 */
	0202, 0, 0, 0, ALT, 0x22,		/* 24 pfk2 -> pf2 */
	0203, 0, 0, 0, ALT, 0x23,		/* 25 pfk3 -> pf3 */
	0204, 0, 0, 0, ALT, 0x24,		/* 26 pfk4 -> pf4 */
	0267, 0, 0, 0, ALT, 0x25,		/* 27 num 7 -> pf5 */
	0270, 0, 0, 0, ALT, 0x26,		/* 28 num 8 -> pf6 */
	0271, 0, 0, 0, ALT, 0x27,		/* 29 num 9 -> pf7 */
	0255, 0, 0, 0, ALT, 0x28,		/* 30 num - -> pf8 */
	0264, 0, 0, 0, ALT, 0x29,		/* 31 num 4 -> pf9 */
	0265, 0, 0, 0, ALT, 0x20,		/* 32 num 5 -> pf10 */
	0266, 0, 0, 0, ALT, 0x30,		/* 33 num 6 -> pf11 */
	0254, 0, 0, 0, ALT, 0x11,		/* 34 num , -> pf12 */
	0301, 0, 0, 0, NORMAL, 0x40,		/* 35 shift pfk1 -> pf13 */
	0302, 0, 0, 0, NORMAL, 0x41,		/* 36 shift pfk2 -> pf14 */
	0303, 0, 0, 0, NORMAL, 0x42,		/* 37 shift pfk3 -> pf15 */
	0304, 0, 0, 0, NORMAL, 0x43,		/* 38 shift pfk4 -> pf16 */
	0367, 0, 0, 0, NORMAL, 0x44,		/* 39 pf17 */
	0370, 0, 0, 0, NORMAL, 0x45,		/* 40 pf18 */
	0371, 0, 0, 0, NORMAL, 0x46,		/* 41 pf19 */
	0355, 0, 0, 0, NORMAL, 0x47,		/* 42 pf20 */
	0364, 0, 0, 0, NORMAL, 0x48,		/* 43 pf21 */
	0365, 0, 0, 0, NORMAL, 0x49,		/* 44 pf22 */
	0366, 0, 0, 0, NORMAL, 0x4a,		/* 45 pf23 */
	0354, 0, 0, 0, NORMAL, 0x4b,		/* 46 pf24 */
	0260, 0, 0, 0, NORMAL, 0x34,		/* 47 num 0 -> reset */
	'R'-CTRL, 0, 0, 0, ALT, 0x50,		/* 48 ctrl r -> sys_req */
	'T'-CTRL, 0, 0, 0, ALT, 0x57,		/* 49 ctrl t -> test */
	0, 0, 0, 0, NORMAL, 0x00,		/* 50 all_break */
	0, 0, 0, 0, ALT, 0x00,			/* 51 alt_make */
	0, 0, 0, 0, SHIFT, 0x00,		/* 52 shift_make */
};


static char *battrb_str[] = {
	"",	/* reverse screen background */
	""	/* normal screen background */
};

static char *attrb_str[] = {
	"",	/* normal video */
	"",	/* invisible */
	"",	/* intensified */
	"",	/* invisible intensified */
	"",	/* protected */
	"",	/* protected invisible */
	"",	/* protected intensified */
	""	/* protected invisible intensified */
};

/*
**	Global variables
*/

#define	L_X_KEY_XLAT	sizeof(X_key_xlat)/sizeof(X_key_xlat[0])
char	L_x_key_xlat =	L_X_KEY_XLAT;


/*
**	Turn on/off the trace of this module
*/
tr_conio(flag)
{
	trace = flag;
}


/*
**	Return a color for the given attribute
*/
attrb2col(attrb)
u_char attrb;
{
	static char col_tbl[] = {
		GREEN, GREEN, RED, BLACK, CYAN, CYAN, WHITE, BLACK
	};

	return (col_tbl[attrb = attrb >> 3 & 0x04 | attrb >> 2 & 0x3]);
}


/*
**	Background attribute
*/
backgrnd(attrb)
u_char attrb;
{
	Ch_color = (Colorindex)attrb2col(attrb);
	color(Ch_color);
}


/*
**	Sound the bell
*/
beep()
{
	ringbell();
}


/*
**	Convert a button to an ascii character based on the state of the kbd
*/
u_char 
but2c(button,kb_status)
Device button;
char kb_status;
{
	static u_char button_xlat[5][83] = {
		{
		/* un-shifted, no control
		 */
		0,       		/* break */
		'D'-CTRL,		/* setup */
		0,			/* control */
		0,			/* caps lock */
		0,			/* right shift */
		0,			/* left shift */
		0x1b,			/* escape */
		'1', '\t', 'q', 'a', 's',
		0,			/* no scrl */
		'2', '3', 'w', 'e', 'd',
		'f', 'z', 'x', '4', '5',
		'r', 't', 'g', 'h', 'c',
		'v', '6', '7', 'y', 'u',
		'j', 'k', 'b', 'n', '8',
		'9', 'i', 'o', 'l', ';',
		'm', ',', '0', '-', 'p',
		'[', '\'', '\r', '.', '/',
		'=', '`', ']', '\\',
		'\261',			/* pad 1 */
		'\260',			/* pad 0 */
		'\n', '\b',
		0x7f,			/* del */
		'\264',			/* pad 4 */
		'\262',			/* pad 2 */
		'\263',			/* pad 3 */
		'\256',			/* pad . */
		'\267',			/* pad 7 */
		'\270',			/* pad 8 */
		'\265',			/* pad 5 */
		'\266',			/* pad 6 */
		'\202', '\201', '\310', '\312',	/* PF2, PF1, LT, DN */
		'\271',			/* pad 9 */
		'\255',			/* pad - */
		'\254',			/* pad , */
		'\204', '\203', '\314', '\313',	/* PF4, PF3, RT, UP */
		'\215',			/* pad enter */
		' ',
		},

		{
		/* control
		 */
		0,			/* break */
		0,			/* setup */
		0,			/* control */
		0,			/* caps lock */
		0,			/* right shift */
		0,			/* left shift */
		0x1b,			/* escape */
		'\201', '\t', 'Q'-CTRL, 'A'-CTRL, 'S'-CTRL,
		0,			/* no scrl */
		'\202', '\203', 'W'-CTRL, 'E'-CTRL, '@'-CTRL,
		'F'-CTRL, 'Z'-CTRL, 'X'-CTRL, '\204', '\267',
		'R'-CTRL, 'T'-CTRL, 'G'-CTRL, 'H'-CTRL, '@'-CTRL,
		'V'-CTRL, '\270', '\271', 'Y'-CTRL, 'U'-CTRL,
		'J'-CTRL, 'K'-CTRL, 'B'-CTRL, 'N'-CTRL, '\255',
		'\264', 'I'-CTRL, 'O'-CTRL, 'L'-CTRL, ';',
		'M'-CTRL, ',', '\265', '\266', 'P'-CTRL,
		'['-CTRL, '\'', '\r', '.', '/'-CTRL,
		'\254','^'-CTRL, ']'-CTRL, '\\'-CTRL,
		'\261',			/* pad 1 */
		'\260',			/* pad 0 */
		'\n', '\b',
		0x1f,			/* del */
		'\264',			/* pad 4 */
		'\262',			/* pad 2 */
		'\263',			/* pad 3 */
		'\256',			/* pad . */
		'\267',			/* pad 7 */
		'\270',			/* pad 8 */
		'\265',			/* pad 5 */
		'\266',			/* pad 6 */
		'\276', '\275', '\310', '\312',	/* PF2, PF1, LT, DN */
		'\271',			/* pad 9 */
		'\255',			/* pad - */
		'\254',			/* pad , */
 		'\341', '\340', '\314', '\313',	/* PF4, PF3, RT, UP */
		'\215',			/* pad enter */
		' ',
		},

		{
		/* shifted
		 */
		0,			/* break */
		0,			/* setup */
		0,			/* control */
		0,			/* caps lock */
		0,			/* right shift */
		0,			/* left shift */
		0x1b,			/* escape */
		'!', '\206', 'Q', 'A', 'S',
		'\214',			/* no scrl */
		'@', '#', 'W', 'E', 'D',
		'F', 'Z', 'X', '$', '%',
		'R', 'T', 'G', 'H', 'C',
		'V', '^', '&', 'Y', 'U',
		'J', 'K', 'B', 'N', '*',
		'(', 'I', 'O', 'L', ':',
		'M', '<', ')', '_', 'P',
		'{', '"', '\r', '>', '?',
		'+', '~', '}', '|',
		'\261',			/* pad 1 */
		'\260',			/* pad 0 */
		'\n', '\b',
		'\315',			/* del */
		'\364',			/* pad 4 */
		'\262',			/* pad 2 */
		'\263',			/* pad 3 */
		'\256',			/* pad . */
		'\367',			/* pad 7 */
		'\370',			/* pad 8 */
		'\365',			/* pad 5 */
		'\366',			/* pad 6 */
		'\302', '\301', '\310', '\312',	/* PF2, PF1, LT, DN */
		'\371',			/* pad 9 */
		'\355',			/* pad - */
		'\354',			/* pad , */
		'\304', '\303', '\314', '\313',	/* PF4, PF3, RT, UP */
		'\215',			/* pad enter */
		' ',
		},

		{
		/* caps lock
		 */
		0,       		/* break */
		0,			/* setup */
		0,			/* control */
		0,			/* caps lock */
		0,			/* right shift */
		0,			/* left shift */
		0x1b,			/* escape */
		'1', '\t', 'Q', 'A', 'S',
		0,			/* no scrl */
		'2', '3', 'W', 'E', 'D',
		'F', 'Z', 'X', '4', '5',
		'R', 'T', 'G', 'H', 'C',
		'V', '6', '7', 'Y', 'U',
		'J', 'K', 'B', 'N', '8',
		'9', 'I', 'O', 'L', ';',
		'M', ',', '0', '-', 'P',
		'[', '\'', '\r', '.', '/',
		'=', '`', ']', '\\',
		'\261',			/* pad 1 */
		'\260',			/* pad 0 */
		'\n', '\b',
		0x7f,			/* del */
		'\264',			/* pad 4 */
		'\262',			/* pad 2 */
		'\263',			/* pad 3 */
		'\256',			/* pad . */
		'\267',			/* pad 7 */
		'\270',			/* pad 8 */
		'\265',			/* pad 5 */
		'\266',			/* pad 6 */
		'\202', '\201', '\310', '\312',	/* PF2, PF1, LT, DN */
		'\271',			/* pad 9 */
		'\255',			/* pad - */
		'\254',			/* pad , */
		'\204', '\203', '\314', '\313',	/* PF4, PF3, RT, UP */
		'\215',			/* pad enter */
		' ',
		},

		{
		/* shifted & caps lock
		 */
		0,			/* break */
		0,			/* setup */
		0,			/* control */
		0,			/* caps lock */
		0,			/* right shift */
		0,			/* left shift */
		0x1b,			/* escape */
		'!', '\206', 'q', 'a', 's',
		'\214',			/* no scrl */
		'@', '#', 'w', 'e', 'd',
		'f', 'z', 'x', '$', '%',
		'r', 't', 'g', 'h', 'c',
		'v', '^', '&', 'y', 'u',
		'j', 'k', 'b', 'n', '*',
		'(', 'i', 'o', 'l', ':',
		'm', ',', ')', '_', 'p',
		'{', '"', '\r', '>', '?',
		'+', '~', '}', '|',
		'\261',			/* pad 1 */
		'\260',			/* pad 0 */
		'\n', '\b',
		'\315',			/* del */
		'\364',			/* pad 4 */
		'\262',			/* pad 2 */
		'\263',			/* pad 3 */
		'\256',			/* pad . */
		'\367',			/* pad 7 */
		'\370',			/* pad 8 */
		'\365',			/* pad 5 */
		'\366',			/* pad 6 */
		'\302', '\301', '\310', '\312',	/* PF2, PF1, LT, DN */
		'\371',			/* pad 9 */
		'\355',			/* pad - */
		'\354',			/* pad , */
		'\304', '\303', '\314', '\313',	/* PF4, PF3, RT, UP */
		'\215',			/* pad enter */
		' ',
		}
	};
	register char i;

	if (kb_status & (1<<CTRL_BIT))
		i = 1;
 	else if ((kb_status & ((1<<SHIFT_BIT) | (1<<CAPS_LOCK_BIT))) ==
 			((1<<SHIFT_BIT) | (1<<CAPS_LOCK_BIT)))
		i = 4;
	else if (kb_status & (1<<SHIFT_BIT))
		i = 2;
	else if (kb_status & (1<<CAPS_LOCK_BIT))
		i = 3;
	else
		i = 0;
	return (button_xlat[i][button-BREAKKEY]);
}


/*
**	Clear terminal's screen
*/
clear_screen()
{
	color(BLACK);
/*	rectfi(142,74,880,692);*/
	rectfi((Icoord)148,(Icoord)196,(Icoord)878,(Icoord)577);
	color(WHITE);
	rectfi((Icoord)148,(Icoord)196,(Icoord)878,(Icoord)577);
	xy(1,1);
}


/*
**	Reset the console flags
*/
conclose(flag)
u_char flag;
{
		unqkeys();
		gflush();
}


/*
**	Get a character from the console.  Return 0 if none avail yet else 1.
*/
conin(cp)
u_char *cp;
{
	static char kb_status = 0;
	static char kstate[MAXKBDBUT+1] = { 0 };
	short data;
	register Device button;

	while ((button = qtest(&data)) != NULLDEV && (button < BUT0 ||
 	    button > LEFTMOUSE))
		button = qread(&data);	/* gobble up unknown input */
	if (button == NULLDEV)
		return 0;		/* no input from KEYBD yet */
	switch (button = qread(&data))
	{
	case CTRLKEY:
		if (data)
			kb_status |= (1<<CTRL_BIT);
		else 
			kb_status &= !(1<<CTRL_BIT);
		return 0;
	case CAPSLOCKKEY:
		kstate[CAPSLOCKKEY] ^= 1;
		if (kstate[CAPSLOCKKEY])
			kb_status ^= (1<<CAPS_LOCK_BIT);
		return 0;
	case RIGHTSHIFTKEY:
	case LEFTSHIFTKEY:
		if (data)
			kb_status |= (1<<SHIFT_BIT);
		else
			kb_status &= !(1<<SHIFT_BIT);
		return 0;
	case RIGHTMOUSE:
		if (data == 0)		/* key released */
			return 0;
		*cp = '\372';
		return 1;
	case MIDDLEMOUSE:
		if (data == 0)		/* key released */
			return 0;
		*cp = '\373';
		return 1;
	case LEFTMOUSE:
		if (data == 0)		/* key released */
			return 0;
		*cp = '\374';
		return 1;
	default:
		kstate[button] = (char)data;
		if (data == 0)		/* key released */
			return 0;
		*cp = but2c(button,kb_status);
/*		if (*cp == (u_char)'\205')
			return 0;	/* ignore BREAK */
		DT("CONIN:  %02x\n", *cp);
		return 1;
	}
}


/*
**	Initialize the console I/O code
*/
conopen(flag)
u_char flag;
{
	register button;

		ginit();
		cursoff();
		color(7);
		Ready(Create(3, repainter, 500), 0);
		for (button=BUT0; button<=MAXKBDBUT; ++button)
#ifdef GL2
			qdevice(button);
#else
			qbutton(button);
#endif GL2

	qreset();
#ifndef GL2TERM
	normalize_kb();
#endif GL2TERM
}


/*
**	Display a character
*/
conout(c)
u_char c;
{
	static char str[] = " ";

	color(BLACK);
	rectfi(Scrn_x,Scrn_y-2,Scrn_x+8,Scrn_y+12);
	color(Ch_color);
	cmov2i(Scrn_x,Scrn_y);
	str[0] = c;
	charstr(str);
	Scrn_x += 9;
}


/*
**	Initialize the console I/O code
*/
db_conopen()
{
	register char i, j;
	static char *iris_scrn_strs[] = {
		"\004\033H\033J",	/* C_SCRN */
		"\000",			/* END_S */
		"\002\033K",		/* E_EOL */
		"\004\033Y< ",		/* STRT_S */
		"\000",			/* U1_LN */
		"\000",			/* U2_LN */
		"\004\033Y",		/* XY */
		"",		/* reverse screen background */
		"",		/* normal screen background */
		"",		/* normal video */
		"",		/* invisible */
		"",		/* intensified */
		"",		/* invisible intensified */
		"",		/* protected */
		"",		/* protected invisible */
		"",		/* protected intensified */
		""		/* protected invisible intensified */
	};
	static struct {
		char *tt_name;
		char **scrn_strs;
		x_key_table *key_xlat;
	} trms[] = {
		"iris",
		iris_scrn_strs,
		X_key_xlat
	};

	ttytype = 0;			/* default = iris console */
	/* THE FOLLOWING CODE SHOULD REALLY USE THE TERMCAP FILE */
	(void)strcpy(c_scrn_str, trms[ttytype].scrn_strs[0]);
	(void)strcpy(end_s_str, trms[ttytype].scrn_strs[1]);
	(void)strcpy(e_eol_str, trms[ttytype].scrn_strs[2]);
	(void)strcpy(strt_s_str, trms[ttytype].scrn_strs[3]);
	(void)strcpy(u1_ln_str, trms[ttytype].scrn_strs[4]);
	(void)strcpy(u2_ln_str, trms[ttytype].scrn_strs[5]);
	(void)strcpy(xy_str, trms[ttytype].scrn_strs[6]);
	battrb_str[0] = trms[ttytype].scrn_strs[7];
	battrb_str[1] = trms[ttytype].scrn_strs[8];
	for (i = 0, j = 9; i < 8; i++, j++)
		attrb_str[i] = trms[ttytype].scrn_strs[j];
	normalize_kb();
}


/*
**	Display an attribute
**
**	Attrb from 3274:
**		 7                     0
**		-------------------------
**		|  |  |  |  |  |  |  |  |
**		-------------------------
**		        ^     ^  ^
**			|     |  |________ invisible if intensified
**			|     |___________ intensified
**			|_________________ protected
*/
disp_attrb(attrb)
u_char attrb;
{
	conout(' ');
	color(Ch_color = (Colorindex)attrb2col(attrb));
}


/*
**	Send end of user status info sequence to terminal
*/
end_status()
{
	color(BLACK);
	rectfi((Icoord)873,(Icoord)199,(Icoord)874,(Icoord)213);
}


/*
**	Un-display the cursor on the iris terminal
*/
erase_cursor(r,c)
{
	register Icoord x,y;

	color(BLACK);
/*	x = 9*(c-1)+152;*/
	x = 9*(c-1)+(Icoord)152;
/*	y = 15*(40-r)+85;*/
	y = 15*(33-r)+(Icoord)81;
	rectfi(x,y-2,x+8,y+12);
}


/*
**	Erase to end of line
*/
erase_eol()
{
	color(BLACK);
/*	rectfi(Scrn_x,Scrn_y-2,880-1,Scrn_y+12);*/
	rectfi(Scrn_x,Scrn_y-2,(Icoord)874,Scrn_y+12);
}


/*
**	Normalize 3274 keyboard state from unknown state
*/
normalize_kb() 
{
	send_x_key(X_ALT_MAKE);
	send_x_key(X_SHIFT);
	send_x_key(X_RESET);
	delay(MS_15);
}


/*
**	Queue all the keys on the iris console
*/
qkeys()
{
	register button;

	for (button=BUT0; button<=MAXKBDBUT; ++button)
#ifdef GL2
		qdevice(button);
#else
		qbutton(button);
#endif GL2
	qreset();
}


/*
**	Set the color for subsequent characters
*/
set_color(attrb)
u_char attrb;
{

	color(Ch_color = (Colorindex)attrb2col(attrb));
}


/*
**	Display the cursor on the iris terminal
*/
show_cursor(c)
u_char c;
{
	static char str[] = " ";

	color(WHITE);
	rectfi(Scrn_x,Scrn_y-2,Scrn_x+8,Scrn_y+12);
	color(BLACK);
	cmov2i(Scrn_x,Scrn_y);
	str[0] = c;
	charstr(str);
}


/*
**	Lead in for user's status on line 25
*/
start_status()
{
	xy(25,1);
	color(Ch_color = WHITE);
	return (1);
}


/*
**	Display string of given length
*/
strnout(s,len)
u_char *s;
{
	char buf[81];

	color(BLACK);
	rectfi(Scrn_x,Scrn_y-2,Scrn_x+9*len,Scrn_y+12);
	color(Ch_color);
	(void)strncpy(buf, s, len);
	buf[len] = '\0';
	charstr(buf);
	Scrn_x += 9 * len;
}



/*
**	Unqueue all the keys on the iris console
*/
unqkeys()
{
	register button;

	for (button=BUT0; button<=MAXKBDBUT; ++button)
#ifdef GL2
		unqdevice(button);
#else
		unqbutton(button);
#endif GL2
	qreset();
}


/*
**	Enable/disable the display of the user's status line on line 25
**	(call it for tvi950 only!)
*/
user_line(flag)
	char flag;		/* 1 - enable;  0 - disable */
{
	flag++;
}


/*
**	Cursor positioning
*/
xy(r,c)
char r, c;
{
/*	cmov2i(Scrn_x = 9*(c-1)+151, Scrn_y = 15*(40-r)+85);*/
	cmov2i(Scrn_x = 9*(c-1)+152, Scrn_y = 15*(33-r)+81);
}

