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
#include <sys/termio.h>
#include <fcntl.h>
#define DC4
#define UC4
#include <gl2/globals.h>
#include <gl2/gltypes.h>
#include <gl2/immed.h>
#include <gl2/imsetup.h>
#include <gl.h>
#include <device.h>
#include "term.h"
#include "pxw.h"

#undef  CTRL
#define CTRL	'@'
#define	BS	('H' - CTRL)
#define ESC	0x1b
#define DEL	0x7f


#define CTRL_BIT	0		/* keyboard status bits */
#define SHIFT_BIT	1
#define CAPS_LOCK_BIT	2

/*
**	Externals
*/
extern int	errno;
extern char	*getenv();
extern int	strcmp();
extern char 	*strcpy();
extern char	*strncpy();
extern int	write();
extern u_char	F3174;

/*
**	Global variables
*/
u_char		Display;		/* 0 - graphic, 1 - text, 2 - text/att*/
u_char		Keyboard;		/* 0 - device, 1 - tty, 2 - tty/pad*/
u_char	ttytype;
int	ttyp = 0;

/*
**	Local variables
*/
u_char c_scrn_str[6];		/* clear screen string */
u_char end_s_str[6];		/* end of user's status */
u_char e_eol_str[6];		/* erase to end of line */
u_char strt_s_str[6];		/* user's status lead in */
u_char u1_ln_str[6];		/* enable user's status line */
u_char u2_ln_str[6];		/* disable user's status line */
u_char xy_str[6];		/* cursor positioning string */
Icoord Scrn_x, Scrn_y;		/* Current graphics position for cmov */
Colorindex Ch_color;		/* Current character color */

/*  DO NOT CHANGE THE ORDER IN THE TABLE BELOW WITHOUT CHANGING THE DEFINITIONS
**  IN THE INCLUDE FILE
*/
x_key_table X_key_xlat[] = {
	0xb3, 0, 0, 0, NORMAL, 0,	/* 00 num 3 -> reshow. KEEP IT AT 00! */
	'A'-CTRL, 0, 0, 0, NORMAL, 0x50,	/* 01 ctrl a -> attention */
	BS, 0, 0, 0, NORMAL, 0x31,		/* 02 backspace */
	0x86, 0, 0, 0, NORMAL, 0x35,		/* 03 back_tab */
	0x8d, 0, 0, 0, ALT, 0x51,		/* 04 num enter -> clear */
	0xca, 0, 0, 0, NORMAL, 0x13,		/* 05 c_down */
	0xc8, 0, 0, 0, NORMAL, 0x16,		/* 06 c_left */
	0xcc, 0, 0, 0, NORMAL, 0x1a,		/* 07 c_right */
	0xcb, 0, 0, 0, NORMAL, 0x0e,		/* 08 c_up */
	'X'-CTRL, 0, 0, 0, NORMAL, 0x51,	/* 09 ctrl x -> cur_select */
	DEL, 0, 0, 0, NORMAL, 0x0d,		/* 10 delete */
	'N'-CTRL, 0, 0, 0, ALT, 0x34,		/* 11 ctrl n -> dev_cancel */
	'U'-CTRL, 0, 0, 0, NORMAL, 0x5f,	/* 12 ctrl u -> dup */
	'M'-CTRL, 0, 0, 0, NORMAL, 0x18,	/* 13 return -> enter */
	0xb1, 0, 0, 0, NORMAL, 0x55,		/* 14 num 1 -> erase_eof */
	0xb2, 0, 0, 0, ALT, 0x53,		/* 15 num 2 -> er_input */
	'F'-CTRL, 0, 0, 0, NORMAL, 0x5e,	/* 16 ctrl f -> f_mark */
	'I'-CTRL, 0, 0, 0, NORMAL, 0x36,	/* 17 fwd_tab */
	0xae, 0, 0, 0, ALT, 0x35,		/* 18 num . -> home */
	'P'-CTRL, 0, 0, 0, NORMAL, 0x0c,	/* 19 ctrl p -> insert */
	'J'-CTRL, 0, 0, 0, NORMAL, 0x08,	/* 20 new_line */
	0xbd, 0, 0, 0, ALT, 0x5f,		/* 21 ctrl pf1 -> pa1 */
	0xbe, 0, 0, 0, ALT, 0x5e,		/* 22 ctrl pf2 -> pa2 */
	0x81, 0, 0, 0, ALT, 0x21,		/* 23 pfk1 -> pf1 */
	0x82, 0, 0, 0, ALT, 0x22,		/* 24 pfk2 -> pf2 */
	0x83, 0, 0, 0, ALT, 0x23,		/* 25 pfk3 -> pf3 */
	0x84, 0, 0, 0, ALT, 0x24,		/* 26 pfk4 -> pf4 */
	0xb7, 0, 0, 0, ALT, 0x25,		/* 27 num 7 -> pf5 */
	0xb8, 0, 0, 0, ALT, 0x26,		/* 28 num 8 -> pf6 */
	0xb9, 0, 0, 0, ALT, 0x27,		/* 29 num 9 -> pf7 */
	0xad, 0, 0, 0, ALT, 0x28,		/* 30 num - -> pf8 */
	0xb4, 0, 0, 0, ALT, 0x29,		/* 31 num 4 -> pf9 */
	0xb5, 0, 0, 0, ALT, 0x20,		/* 32 num 5 -> pf10 */
	0xb6, 0, 0, 0, ALT, 0x30,		/* 33 num 6 -> pf11 */
	0xac, 0, 0, 0, ALT, 0x11,		/* 34 num , -> pf12 */
	0xc1, 0, 0, 0, NORMAL, 0x40,		/* 35 shift pfk1 -> pf13 */
	0xc2, 0, 0, 0, NORMAL, 0x41,		/* 36 shift pfk2 -> pf14 */
	0xc3, 0, 0, 0, NORMAL, 0x42,		/* 37 shift pfk3 -> pf15 */
	0xc4, 0, 0, 0, NORMAL, 0x43,		/* 38 shift pfk4 -> pf16 */
	0xf7, 0, 0, 0, NORMAL, 0x44,		/* 39 pf17 */
	0xf8, 0, 0, 0, NORMAL, 0x45,		/* 40 pf18 */
	0xf9, 0, 0, 0, NORMAL, 0x46,		/* 41 pf19 */
	0xed, 0, 0, 0, NORMAL, 0x47,		/* 42 pf20 */
	0xf4, 0, 0, 0, NORMAL, 0x48,		/* 43 pf21 */
	0xf5, 0, 0, 0, NORMAL, 0x49,		/* 44 pf22 */
	0xf6, 0, 0, 0, NORMAL, 0x4a,		/* 45 pf23 */
	0xec, 0, 0, 0, NORMAL, 0x4b,		/* 46 pf24 */
	0xb0, 0, 0, 0, NORMAL, 0x34,		/* 47 num 0 -> reset */
	'R'-CTRL, 0, 0, 0, ALT, 0x50,		/* 48 ctrl r -> sys_req */
	'T'-CTRL, 0, 0, 0, ALT, 0x57,		/* 49 ctrl t -> test */
	0, 0, 0, 0, NORMAL, 0x00,		/* 50 alt_break */
	0, 0, 0, 0, ALT, 0x00,			/* 51 alt_make */
	0, 0, 0, 0, SHIFT, 0x00,		/* 52 shift_make */
};
#define	L_X_KEY_XLAT	sizeof(X_key_xlat)/sizeof(X_key_xlat[0])
u_char	L_x_key_xlat =	L_X_KEY_XLAT;

char tb3[] = {"Key_codes"};
key_table Key_xlat[] = {
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
key_table X_key_codes[] = {
	NORMAL, 0,		/* 00 reshow. KEEP IT AT 00! */
	NORMAL, 0x50,		/* 01 attention */
	SHIFT, 0x21,		/* 02 bar */
	NORMAL, 0x31,		/* 03 backspace */
	NORMAL, 0x35,		/* 04 back_tab */
	NORMAL, 0x1b,		/* 05 cent */
	ALT, 0x51,		/* 06 clear */
	NORMAL, 0x13,		/* 07 c_down */
	NORMAL, 0x16,		/* 08 c_left */
	NORMAL, 0x1a,		/* 09 c_right */
	NORMAL, 0x0e,		/* 10 c_up */
	NORMAL, 0x51,		/* 11 cur_select */
	NORMAL, 0x0d,		/* 12 delete */
	ALT, 0x34,		/* 13 dev_cancel */
	NORMAL, 0x5f,		/* 14 dup */
	NORMAL, 0x18,		/* 15 enter */
	NORMAL, 0x55,		/* 16 erase_eof */
	ALT, 0x53,		/* 17 er_input */
	NORMAL, 0x5e,		/* 18 f_mark */
	NORMAL, 0x36,		/* 19 fwd_tab */
	ALT, 0x35,		/* 20 home */
	NORMAL, 0x0c,		/* 21 insert */
	NORMAL, 0x08,		/* 22 new_line */
	ALT, 0x5f,		/* 23 pa1 */
	ALT, 0x5e,		/* 24 pa2 */
	ALT, 0x21,		/* 25 pf1 */
	ALT, 0x22,		/* 26 pf2 */
	ALT, 0x23,		/* 27 pf3 */
	ALT, 0x24,		/* 28 pf4 */
	ALT, 0x25,		/* 29 pf5 */
	ALT, 0x26,		/* 30 pf6 */
	ALT, 0x27,		/* 31 pf7 */
	ALT, 0x28,		/* 32 pf8 */
	ALT, 0x29,		/* 33 pf9 */
	ALT, 0x20,		/* 34 pf10 */
	ALT, 0x30,		/* 35 pf11 */
	ALT, 0x11,		/* 36 pf12 */
	NORMAL, 0x40,		/* 37 pf13 */
	NORMAL, 0x41,		/* 38 pf14 */
	NORMAL, 0x42,		/* 39 pf15 */
	NORMAL, 0x43,		/* 40 pf16 */
	NORMAL, 0x44,		/* 41 pf17 */
	NORMAL, 0x45,		/* 42 pf18 */
	NORMAL, 0x46,		/* 43 pf19 */
	NORMAL, 0x47,		/* 44 pf20 */
	NORMAL, 0x48,		/* 45 pf21 */
	NORMAL, 0x49,		/* 46 pf22 */
	NORMAL, 0x4a,		/* 47 pf23 */
	NORMAL, 0x4b,		/* 48 pf24 */
	NORMAL, 0x34,		/* 49 reset */
	ALT, 0x50,		/* 50 sys_req */
	ALT, 0x57,		/* 51 test */
	NORMAL, 0x00,		/* 52 alt_break */
	ALT, 0x00,		/* 53 alt_make */
	SHIFT, 0x00,		/* 54 shift_make */
	SHIFT|ALT, 0x00,	/* 55 debug_dp */
	SHIFT|ALT, 0x00,	/* 56 disconnect */
	SHIFT|ALT, 0x00,	/* 57 fix_comm */
	SHIFT|ALT, 0x00,	/* 58 interrupt */
	SHIFT|ALT, 0x00,	/* 59 pause */
	SHIFT|ALT, 0x00,	/* 60 restore_dp */
	SHIFT|ALT, 0x00,	/* 61 version_dp */
	SHIFT|ALT, 0x00,	/* 62 exit_term */
	SHIFT|ALT, 0x00,	/* 63 exit_rgl */
	SHIFT|ALT, 0x00,	/* 64 ttytype */
};


u_char button_xlat[5][83] = {
	{
	/* un-shifted, no control
	 */
	0x85,      		/* break */
	0,			/* setup */
	0,			/* control */
	0,			/* caps lock */
	0,			/* right shift */
	0,			/* left shift */
	0x1b,			/* escape */
	'1', 'I'-CTRL, 'q', 'a', 's',
	0,			/* no scrl */
	'2', '3', 'w', 'e', 'd',
	'f', 'z', 'x', '4', '5',
	'r', 't', 'g', 'h', 'c',
	'v', '6', '7', 'y', 'u',
	'j', 'k', 'b', 'n', '8',
	'9', 'i', 'o', 'l', ';',
	'm', ',', '0', '-', 'p',
	'[', '\'', 'M'-CTRL, '.', '/',
	'=', '`', ']', '\\',
	0xb1,			/* pad 1 */
	0xb0,			/* pad 0 */
	'J'-CTRL, BS,
	0x7f,			/* del */
	0xb4,			/* pad 4 */
	0xb2,			/* pad 2 */
	0xb3,			/* pad 3 */
	0xae,			/* pad . */
	0xb7,			/* pad 7 */
	0xb8,			/* pad 8 */
	0xb5,			/* pad 5 */
	0xb6,			/* pad 6 */
	0x82, 0x81, 0xc8, 0xca,	/* PF2, PF1, LT, DN */
	0xb9,			/* pad 9 */
	0xad,			/* pad - */
	0xac,			/* pad , */
	0x84, 0x83, 0xcc, 0xcb,	/* PF4, PF3, RT, UP */
	0x8d,			/* pad enter */
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
	'1', 'I'-CTRL, 'Q'-CTRL, 'A'-CTRL, 'S'-CTRL,
	0,			/* no scrl */
	'2', '3', 'W'-CTRL, 'E'-CTRL, 'D'-CTRL,
	'F'-CTRL, 'Z'-CTRL, 'X'-CTRL, '4', '5',
	'R'-CTRL, 'T'-CTRL, 'G'-CTRL, 'H'-CTRL, '@'-CTRL,
	'V'-CTRL, '6', '7', 'Y'-CTRL, 'U'-CTRL,
	'J'-CTRL, 'K'-CTRL, 'B'-CTRL, 'N'-CTRL, '8',
	'9', 'I'-CTRL, 'O'-CTRL, 'L'-CTRL, ';',
	'M'-CTRL, ',', '0', '_'-CTRL, 'P'-CTRL,
	'['-CTRL, '\'', 'M'-CTRL, '.', '/',
	'=', '`', ']'-CTRL, '\\'-CTRL,
	0xb1,			/* pad 1 */
	0xb0,			/* pad 0 */
	'J'-CTRL, BS,
	0x7f,			/* del */
	0xb4,			/* pad 4 */
	0xb2,			/* pad 2 */
	0xb3,			/* pad 3 */
	0xae,			/* pad . */
	0xb7,			/* pad 7 */
	0xb8,			/* pad 8 */
	0xb5,			/* pad 5 */
	0xb6,			/* pad 6 */
	0xbe, 0xbd, 0xc8, 0xca,	/* PF2, PF1, LT, DN */
	0xb9,			/* pad 9 */
	0xad,			/* pad - */
	0xac,			/* pad , */
	0xe1, 0xe0, 0xcc, 0xcb,	/* PF4, PF3, RT, UP */
	0x8d,			/* pad enter */
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
	'!', 0x86, 'Q', 'A', 'S',
	'P'-CTRL,			/* no scrl */
	'@', '#', 'W', 'E', 'D',
	'F', 'Z', 'X', '$', '%',
	'R', 'T', 'G', 'H', 'C',
	'V', '^', '&', 'Y', 'U',
	'J', 'K', 'B', 'N', '*',
	'(', 'I', 'O', 'L', ':',
	'M', '<', ')', '_', 'P',
	'{', '"', 'J'-CTRL, '>', '?',
	'+', '~', '}', '|',
	0xb1,			/* pad 1 */
	0xb0,			/* pad 0 */
	'J'-CTRL, BS,
	0xcd,			/* del */
	0xf4,			/* pad 4 */
	0xb2,			/* pad 2 */
	0xb3,			/* pad 3 */
	0xae,			/* pad . */
	0xf7,			/* pad 7 */
	0xf8,			/* pad 8 */
	0xf5,			/* pad 5 */
	0xf6,			/* pad 6 */
	0xc2, 0xc1, 0xc8, 0xca,	/* PF2, PF1, LT, DN */
	0xf9,			/* pad 9 */
	0xed,			/* pad - */
	0xec,			/* pad , */
	0xc4, 0xc3, 0xcc, 0xcb,	/* PF4, PF3, RT, UP */
	0x8d,			/* pad enter */
	' ',
	},

	{
	/* caps lock
	 */
	0,			/* break */
	0,			/* setup */
	0,			/* control */
	0,			/* caps lock */
	0,			/* right shift */
	0,			/* left shift */
	0x1b,			/* escape */
	'1', 'I'-CTRL, 'Q', 'A', 'S',
	0,			/* no scrl */
	'2', '3', 'W', 'E', 'D',
	'F', 'Z', 'X', '4', '5',
	'R', 'T', 'G', 'H', 'C',
	'V', '6', '7', 'Y', 'U',
	'J', 'K', 'B', 'N', '8',
	'9', 'I', 'O', 'L', ';',
	'M', ',', '0', '-', 'P',
	'[', '\'', 'M'-CTRL, '.', '/',
	'=', '`', ']', '\\',
	0xb1,			/* pad 1 */
	0xb0,			/* pad 0 */
	'J'-CTRL, BS,
	0x7f,			/* del */
	0xb4,			/* pad 4 */
	0xb2,			/* pad 2 */
	0xb3,			/* pad 3 */
	0xae,			/* pad . */
	0xb7,			/* pad 7 */
	0xb8,			/* pad 8 */
	0xb5,			/* pad 5 */
	0xb6,			/* pad 6 */
	0x82, 0x81, 0xc8, 0xca,	/* PF2, PF1, LT, DN */
	0xb9,			/* pad 9 */
	0xad,			/* pad - */
	0xac,			/* pad , */
	0x84, 0x83, 0xcc, 0xcb,	/* PF4, PF3, RT, UP */
	0x8d,			/* pad enter */
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
	'!', 0x86, 'q', 'a', 's',
	'P'-CTRL,			/* no scrl */
	'@', '#', 'w', 'e', 'd',
	'f', 'z', 'x', '$', '%',
	'r', 't', 'g', 'h', 'c',
	'v', '^', '&', 'y', 'u',
	'j', 'k', 'b', 'n', '*',
	'(', 'i', 'o', 'l', ':',
	'm', ',', ')', '_', 'p',
	'{', '"', 'J'-CTRL, '>', '?',
	'+', '~', '}', '|',
	0xb1,			/* pad 1 */
	0xb0,			/* pad 0 */
	'J'-CTRL, BS,
	0xcd,			/* del */
	0xf4,			/* pad 4 */
	0xb2,			/* pad 2 */
	0xb3,			/* pad 3 */
	0xae,			/* pad . */
	0xf7,			/* pad 7 */
	0xf8,			/* pad 8 */
	0xf5,			/* pad 5 */
	0xf6,			/* pad 6 */
	0xc2, 0xc1, 0xc8, 0xca,	/* PF2, PF1, LT, DN */
	0xf9,			/* pad 9 */
	0xed,			/* pad - */
	0xec,			/* pad , */
	0xc4, 0xc3, 0xcc, 0xcb,	/* PF4, PF3, RT, UP */
	0x8d,			/* pad enter */
	' ',
	}
};

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
char *battrb_str[] = {
	"",	/* reverse screen background */
	""	/* normal screen background */
};

char *attrb_str[] = {
	"",	/* normal video */
	"",	/* invisible */
	"",	/* intensified */
	"",	/* invisible intensified */
	"",	/* protected */
	"",	/* protected invisible */
	"",	/* protected intensified */
	""	/* protected invisible intensified */
};

u_char kb_status = 0;
u_char kstate[MAXKBDBUT+1] = { 0 };


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
register u_char attrb;
{
	static u_char col_tbl[] = {
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
	im_setup;
	
	Ch_color = (Colorindex)attrb2col(attrb);
	im_color(Ch_color);
#ifdef DEBUG
	DT("back color %d ",Ch_color);
#endif /* DEBUG */
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
u_char kb_status;
{
	register u_char i;

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
#ifdef DEBUG
	DT("button_xlst[%d][%d] -> %03o %02x\n", i,button-BREAKKEY,
	    button_xlat[i][button-BREAKKEY], button_xlat[i][button-BREAKKEY]);
#endif /* DEBUG */
	return (button_xlat[i][button-BREAKKEY]);
}


/*
**	Clear terminal's screen
*/
clear_screen()
{
	im_setup;

#ifdef DEBUG
	DT("Clear_screen ");
#endif /* DEBUG */
	im_color(BLACK);
/*		rectfi((Icoord)152,(Icoord)214,(Icoord)874,(Icoord)573);*/
	im_rectfi((Icoord)0,(Icoord)0,(Icoord)722,(Icoord)375);
	im_color(WHITE);
/*		rectfi((Icoord)152,(Icoord)226,(Icoord)874,(Icoord)573);*/
	im_rectfi((Icoord)0,(Icoord)11,(Icoord)722,(Icoord)375);
	xy(1,1);
}


/*
**	Reset the console flags
*/
conclose(flag)
char flag;
{
#ifdef DEBUG
	DT("Conclose ");
#endif /* DEBUG */
	if (flag) {
		unqkeys();
		gflush();
		gexit();
	}
}


/*
**	Get a character from the console.  Return 0 if none avail yet else 1.
*/
conin(cp)
u_char *cp;
{
	short data;
	register Device button;

	while ((button = qtest(&data)) != NULLDEV && (button < BUT1 ||
	    button > LEFTMOUSE))
		button = qread(&data);	/* gobble up unknown input */
	if (errno == 9)
		errno = 0;
	if (button == NULLDEV)
		return 0;		/* no input from KEYBD yet */
	switch (button = qread(&data))
	{
	case CTRLKEY:
		if (data)
			kb_status |= (1<<CTRL_BIT);
		else 
			kb_status ^= 1<<CTRL_BIT;
		return 0;
	case CAPSLOCKKEY:
		if (data) {
			kstate[CAPSLOCKKEY] ^= 1;
			if (kstate[CAPSLOCKKEY])
				kb_status |= (1<<CAPS_LOCK_BIT);
			else
				kb_status ^= 1<<CAPS_LOCK_BIT;
		}
		return 0;
	case RIGHTSHIFTKEY:
	case LEFTSHIFTKEY:
		if (data)
			kb_status |= (1<<SHIFT_BIT);
		else
			kb_status ^= 1<<SHIFT_BIT;
#ifdef DEBUG
		DT("SHIFTKEY -> kb_status = %x\n", kb_status &
		    (1<<SHIFT_BIT));
#endif /* DEBUG */
		return 0;
	case RIGHTMOUSE:
		if (data == 0)
			return 0;
		*cp = (u_char)0xfa;
		return 1;
	case MIDDLEMOUSE:
		if (data == 0)
			return 0;
		*cp = (u_char)0xfb;
		return 1;
	case LEFTMOUSE:
		if (data == 0)
			return 0;
		*cp = (u_char)0xfc;
		return 1;
	default:
		kstate[button] = (u_char)data;
		if (data == 0)
			return 0;
		*cp = but2c(button,kb_status);
#ifdef DEBUG
		DT("CONIN:  %02x\n", *cp);
#endif /* DEBUG */
		return 1;
	}
}


/*
**	Initialize the console I/O code
*/
conopen(flag)
u_char flag;
{
	register Device button;

	kb_status = 0;
	if (flag)
		qkeys(flag);
	else
		if (zflag[1])
			gbegin();
		else
			ginit();
}


/*
**	Display a character
*/
conout(c)
u_char c;
{
	im_setup;
	static char str[] = " ";

	color(BLACK);
/*	rectfi(Scrn_x,Scrn_y-2,Scrn_x+8,Scrn_y+13);*/
	im_rectfi(Scrn_x,Scrn_y-2,Scrn_x+8,Scrn_y+12);
	im_color(Ch_color);
	im_cmov2i(Scrn_x,Scrn_y);
	str[0] = (char)c;
	charstr(str);
	Scrn_x += 9;
}


/*
**	Initialize the console I/O code
*/
db_conopen()
{
	register u_char i, j;

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

do_conopen() {};

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
	im_setup;

#ifdef DEBUG
	DT("DISP_ATTRB %02x -> %d\n", attrb, attrb2col(attrb));
#endif /* DEBUG */
	conout(' ');
	im_color(Ch_color = (Colorindex)attrb2col(attrb));
#ifdef DEBUG
	DT("disp color %d ",Ch_color);
#endif /* DEBUG */
}


/*
**	Send end of user status info sequence to terminal
*/
end_status()
{
}


/*
**	Un-display the cursor on the iris terminal
*/
erase_cursor(r,c)
u_char r, c;
{
	im_setup;
	register Icoord x,y;

	im_color(BLACK);
/*		x = 9*(c-1)+(Icoord)152;
		y = 16*(33-r)+(Icoord)81;*/
	x = 9*(c-1);
	y = 15*(25-r);
	im_rectfi(x,y-2,x+8,y+12);
}


/*
**	Erase to end of line
*/
erase_eol()
{
	im_setup;

	im_color(BLACK);
/*		rectfi(Scrn_x,Scrn_y-2,(Icoord)874,Scrn_y+13);*/
	im_rectfi(Scrn_x,Scrn_y-2,(Icoord)722,Scrn_y+12);
}


/*
**	Normalize 3274 keyboard state from unknown state
*/
normalize_kb() 
{
#ifdef DEBUG
	DT("norm kb\n");
#endif /* DEBUG */
	send_x_key(X_RESET);
	pdelay(MS_15);
}


/*
**	Print keyboard mapping
*/
print_keys()
{
	printf("\r\n PF 1-12 are pad PF1-4 789-456, and CTRL 1234567890-= on keyboard");
	printf("\r\n EXIT EMULATION is CTRL-d 		EXIT GRAPHICS is ~. or ESC DEL");
	printf("\r\n ATTN CTRL-a    BACK TAB shift TAB    BAR ']'   CANCEL CTRL-n    CENT '['");
	printf("\r\n CLEAR pad 'ENTER'   DUP CTRL-u   ERASE EOF pad '1'   ERASE INPUT pad '2'");
	printf("\r\n FM CTRL-f    HOME pad '.'    INSERT CTRL-p    PA1,2  CTRL-PF1,2");
	printf("\r\n RESET pad '0'    SYS REQ CTRL-r    CURSR SEL CTRL-x    TEST CTRL-t");
}

/*
**	Queue all the keys on the iris console
*/
qkeys(flag)
{
	register Device button;

	prefsize(722,375);
	foreground(1);
	getport(trms[0].tt_name);
	winattach();
	for (button=BUT1; button<=MAXKBDBUT; ++button)
		qdevice(button);
	qreset();
}

rdcon() {};

/*
**	Set the color for subsequent characters
*/
set_color(attrb)
u_char attrb;
{
	im_setup;

	im_color(Ch_color = (Colorindex)attrb2col(attrb));
#ifdef DEBUG
	DT("set color %d ",Ch_color);
#endif /* DEBUG */
}


/*
**	Display the cursor on the iris terminal
*/
show_cursor(c)
u_char c;
{
	im_setup;
	static char str[] = " ";

	im_color(WHITE);
/*	rectfi(Scrn_x,Scrn_y-2,Scrn_x+8,Scrn_y+13);*/
	im_rectfi(Scrn_x,Scrn_y-2,Scrn_x+8,Scrn_y+12);
	im_color(BLACK);
	im_cmov2i(Scrn_x,Scrn_y);
	str[0] = (char)c;
	charstr(str);
}


/*
**	Lead in for user's status on line 25
*/
start_status()
{
	im_setup;

	xy(25,1);
	im_color(Ch_color = WHITE);
}


/*
**	Display string of given length
*/
strnout(s,len)
char *s;
{
	im_setup;
	char buf[81];

	im_color(BLACK);
/*	rectfi(Scrn_x,Scrn_y-2,Scrn_x+9*len,Scrn_y+13);*/
	im_rectfi(Scrn_x,Scrn_y-2,Scrn_x+9*len,Scrn_y+12);
	im_color(Ch_color);
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
	register Device button;

	for (button=BUT1; button<=MAXKBDBUT; ++button)
		unqdevice(button);
	qreset();
}


/*
**	Enable/disable the display of the user's status line on line 25
**	(call it for tvi950 only!)
*/
user_line(flag)
{
}


/*
**	Cursor positioning
*/
xy(r,c)
u_char r, c;
{
	im_setup;

/*	cmov2i(Scrn_x = 9*(c-1)+152, Scrn_y = 16*(33-r)+81);*/
	im_cmov2i(Scrn_x = 9*(c-1), Scrn_y = 15*(25-r));
}

set_3174()
{
	register u_char *p;

	p = (u_char *)X_key_xlat;
	p += sizeof(X_key_xlat) - 1;
	*p = F3174;
}

rgltest()
{
	short *indext;
	unsigned short *colort, *wtmt;
	char *bolley;

	ginit();
	clear();
	cursoff();
	getcursor(indext, colort, wtmt, bolley);
	printf(" b is %x ",bolley);
	curson();
	getcursor(indext, colort, wtmt, bolley);
	printf(" b is %x ",bolley);
	cursoff();
	gexit();
}
