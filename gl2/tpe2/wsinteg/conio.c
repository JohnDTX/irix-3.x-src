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
*	get_key()	- Waits for a keyboard keystroke
*	get_key_changes() - Gets changes to 3270 function key definitions
*	get_presentation() - updates keyboard and display ttytype choices
*	get_rc()	- Read .t3279rc here or $HOME
*	get_state_changes() - get graphics button mapping to keystrokes
*	prinit()	- Print individual keystroke
*	print_3274_key_codes() - print 3174/3274 interface code for pf key
*	print_keys()	- Print 3270 pf key to keystroke mapping
*	qkeys()		- Queue the iris console keyboard buttons
*	rdcon() 	- Menu 8 updates display, keyboard choices, maps
*	set_color()	- Specify color for subsequent text output
*	show_cursor()	- Display the cursor
*	start_status()	- Send load user status line command to terminal
*	strnout()	- Display string
*	unqkeys()	- Unqueue the iris console keyboard buttons
*	user_line()	- Enable/disable the display of the user's status line
*	xy(r,c)		- Cursor positioning
*	update_ttytype()- Update ttytype from TERM of environment
*
*  GLOBAL VARIABLES:
*	X_key_codes	- 3270 codes for special keys
*	X_key_xlat	- translate table for special keys
*	L_x_key_xlat	- number of entries in above table
*
************************************************************************/

#include <sys/types.h>
/*#include <sys/termio.h>*/
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
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


#define CTL		'@'
#define	BS		('H' - CTL)
#define CONFIGFILE	".t3279rc"
#define ETCCONFIGFILE	"/usr/etc/t3279rc"	/* shipped rc file  wpc */
#define ESC		0x1b
#define DEL		0x7f

#define D_ATTRB		(u_char)0xc0	/* default attribute */

#define CTRL_BIT	0		/* keyboard status bits */
#define SHIFT_BIT	1
#define CAPS_LOCK_BIT	2


/*
**	Externals
*/
extern int		errno;
extern char		*concat();
extern char		*getenv();
extern int		strcmp();
extern char 		*strcpy();
extern char		*strncpy();
extern char		*sys_errlist[];
extern int		sys_nerr;
extern int		write();
extern u_char		F3174;
extern u_char		Force_show_screen;
extern long		MaxRU;
extern u_char		Record_sep;

/*
**	Global variables
*/
u_char			Display;	/* 0 - graphic, 1 - text, 2 - text/att*/
u_char			Keyboard;	/* 0 - device, 1 - tty, 2 - tty/pad*/
u_char			ttytype = 0;	/* default is wsiris */

/*
**	Local variables
*/
u_char			kb_changed = 0;
u_char			kb_status = 0;
u_char			kstate[MAXKBDBUT+1] = { 0 };
u_char			battrb;		/* 0 if normal , 0x8 if intensified */
int			ttyp = 0;
u_char			c_scrn_str[8];	/* clear screen string */
u_char			end_s_str[6];	/* end of user's status */
u_char			e_eol_str[6];	/* erase to end of line */
u_char			strt_s_str[10];	/* user's status lead in */
u_char			u1_ln_str[6];	/* enable user's status line */
u_char			u2_ln_str[6];	/* disable user's status line */
u_char			xy_str[10];	/* cursor positioning string */
Icoord			Scrn_x, Scrn_y;	/* Current graphics position for cmov */
Colorindex		Ch_color;	/* Current character color */
Icoord			Offst_x, Offst_y; /* Current graphics position for cmov */


/*  DO NOT CHANGE THE ORDER IN THE TABLE BELOW WITHOUT CHANGING THE DEFINITIONS
**  IN THE INCLUDE FILE PXW.H (e.g. X_CLEAR)
*/
x_key_table Iris_key_xlat[] = {
	0xb3, 0, 0, 0, 			/* 00 pad 3 -> reshow. KEEP IT AT 00! */
	'A'-CTL, 0, 0, 0,		/* 01 ctrl a -> attention */
	']', 0, 0, 0,			/* 02 bar */
	BS, 0, 0, 0,			/* 03 backspace */
	0x86, 0, 0, 0,			/* 04 back_tab */
	'[', 0, 0, 0,			/* 05 cent */
	0x8d, 0, 0, 0,			/* 06 pad enter -> clear */
	0xca, 0, 0, 0,			/* 07 c_down */
	0xc8, 0, 0, 0,			/* 08 c_left */
	0xcc, 0, 0, 0,			/* 09 c_right */
	0xcb, 0, 0, 0,			/* 10 c_up */
	'X'-CTL, 0, 0, 0,		/* 11 ctrl s -> cur_select */
	DEL, 0, 0, 0,			/* 12 delete */
	'N'-CTL, 0, 0, 0,		/* 13 ctrl c -> dev_cancel */
	'U'-CTL, 0, 0, 0,		/* 14 ctrl u -> dup */
	'M'-CTL, 0, 0, 0,		/* 15 return -> enter */
	0xb1, 0, 0, 0,			/* 16 pad 1 -> erase_eof */
	0xb2, 0, 0, 0,			/* 17 pad 2 -> er_input */
	'F'-CTL, 0, 0, 0,		/* 18 ctrl f -> f_mark */
	'I'-CTL, 0, 0, 0,		/* 19 fwd_tab */
	0xae, 0, 0, 0,			/* 20 pad . -> home */
	'P'-CTL, 0, 0, 0,		/* 21 insert */
	'J'-CTL, 0, 0, 0,		/* 22 new_line */
	0xbd, 0, 0, 0,			/* 23 ctrl pf1 -> pa1 */
	0xbe, 0, 0, 0,			/* 24 ctrl pf2 -> pa2 */
	0x81, 0, 0, 0,			/* 25 pfk1 -> pf1 */
	0x82, 0, 0, 0,			/* 26 pfk2 -> pf2 */
	0x83, 0, 0, 0,			/* 27 pfk3 -> pf3 */
	0x84, 0, 0, 0,			/* 28 pfk4 -> pf4 */
	0xb7, 0, 0, 0,			/* 29 pad 7 -> pf5 */
	0xb8, 0, 0, 0,			/* 30 pad 8 -> pf6 */
	0xb9, 0, 0, 0,			/* 31 pad 9 -> pf7 */
	0xad, 0, 0, 0,			/* 32 pad - -> pf8 */
	0xb4, 0, 0, 0,			/* 33 pad 4 -> pf9 */
	0xb5, 0, 0, 0,			/* 34 pad 5 -> pf10 */
	0xb6, 0, 0, 0,			/* 35 pad 6 -> pf11 */
	0xac, 0, 0, 0,			/* 36 pad , -> pf12 */
	0xc1, 0, 0, 0,			/* 37 shift pfk1 -> pf13 */
	0xc2, 0, 0, 0,			/* 38 shift pfk2 -> pf14 */
	0xc3, 0, 0, 0,			/* 39 shift pfk3 -> pf15 */
	0xc4, 0, 0, 0,			/* 40 shift pfk4 -> pf16 */
	0xf7, 0, 0, 0,			/* 41 pf17 */
	0xf8, 0, 0, 0,			/* 42 pf18 */
	0xf9, 0, 0, 0,			/* 43 pf19 */
	0xed, 0, 0, 0,			/* 44 pf20 */
	0xf4, 0, 0, 0,			/* 45 pf21 */
	0xf5, 0, 0, 0,			/* 46 pf22 */
	0xf6, 0, 0, 0,			/* 47 pf23 */
	0xec, 0, 0, 0,			/* 48 pf24 */
	0xb0, 0, 0, 0,			/* 49 pad 0 -> reset */
	'Y'-CTL, 0, 0, 0,		/* 50 ctrl r -> sys_req */
	'T'-CTL, 0, 0, 0,		/* 51 ctrl t -> test */
	0, 0, 0, 0,			/* 52 alt_break */
	0, 0, 0, 0,			/* 53 alt_make */
	0, 0, 0, 0,			/* 54 shift_make */
	0, 0, 0, 0,			/* 55 debug_dp */
	0, 0, 0, 0,			/* 56 disconnect */
	0, 0, 0, 0,			/* 57 fix_comm */
	0, 0, 0, 0,			/* 58 interrupt */
	0, 0, 0, 0,			/* 59 pause */
	0, 0, 0, 0,			/* 60 restore_dp */
	0, 0, 0, 0,			/* 61 version_dp */
	3, 0, 0, 0,			/* 62 exit_term */
	4, 0, 0, 0,			/* 63 exit_rgl */
	0, 0, 0, 0,			/* 64 MaxRU */
	0, 0, 0, 0,			/* 65 ttytype */
};


/*  DO NOT CHANGE THE ORDER IN THE TABLE BELOW WITHOUT CHANGING THE DEFINITIONS
**  IN THE INCLUDE FILE
*/
x_key_table R2_4_key_xlat[] = {
	ESC, '?', 's', 0,		/* 00 reshow. KEEP IT AT 00! */
	'A'-CTL, 0, 0, 0,		/* 01 attention */
	']', 0, 0, 0,			/* 02 bar */
	BS, 0, 0, 0,			/* 03 backspace */
	ESC, 'I', 0, 0,			/* 04 back_tab */
	'[', 0, 0, 0,			/* 05 cent */
	ESC, '?', 'M', 0,		/* 06 pad enter -> clear */
	ESC, 'B', 0, 0,			/* 07 c_down */
	ESC, 'D', 0, 0,			/* 08 c_left */
	ESC, 'C', 0, 0,			/* 09 c_right */
	ESC, 'A', 0, 0,			/* 10 c_up */
	'X'-CTL, 0, 0, 0, 		/* 11 cur_select */
	DEL, 0, 0, 0,			/* 12 delete */
	'N'-CTL, 0, 0, 0,		/* 13 dev_cancel */
	'U'-CTL, 0, 0, 0,		/* 14 dup */
	'\r', 0, 0, 0,			/* 15 return -> enter */
	ESC, '?', 'q', 0,		/* 16 pad 1 -> erase_eof */
	ESC, '?', 'r', 0,		/* 17 pad 2 -> er_input */
	'F'-CTL, 0, 0, 0,		/* 18 f_mark */
	'\t', 0, 0, 0,			/* 19 fwd_tab */
	ESC, '?', 'n', 0,		/* 20 pad . -> home */
	'P'-CTL, 0, 0, 0,		/* 21 insert */
	'\n', 0, 0, 0,			/* 22 new_line */
	ESC, ESC, 'P', 0,		/* 23 esc pf1 -> pa1 */
	ESC, ESC, 'Q', 0,		/* 24 esc pf2 -> pa2 */
	ESC, 'P', 0, 0,			/* 25 pf1 */
	ESC, 'Q', 0, 0,			/* 26 pf2 */
	ESC, 'R', 0, 0,			/* 27 pf3 */
	ESC, 'S', 0, 0,			/* 28 pf4 */
	ESC, '?', 'w', 0,		/* 29 pad 7 -> pf5 */
	ESC, '?', 'x', 0,		/* 30 pad 8 -> pf6 */
	ESC, '?', 'y', 0,		/* 31 pad 9 -> pf7 */
	ESC, '?', 'm', 0,		/* 32 pad - -> pf8 */
	ESC, '?', 't', 0,		/* 33 pad 4 -> pf9 */
	ESC, '?', 'u', 0,		/* 34 pad 5 -> pf10 */
	ESC, '?', 'v', 0,		/* 35 pad 6 -> pf11 */
	ESC, '?', 'l', 0,		/* 36 pad , -> pf12 */
	0, 0, 0, 0,			/* 37 pf13 */
	0, 0, 0, 0,			/* 38 pf14 */
	0, 0, 0, 0,			/* 39 pf15 */
	0, 0, 0, 0,			/* 40 pf16 */
	0, 0, 0, 0,			/* 41 pf17 */
	0, 0, 0, 0,			/* 42 pf18 */
	0, 0, 0, 0,			/* 43 pf19 */
	0, 0, 0, 0,			/* 44 pf20 */
	0, 0, 0, 0,			/* 45 pf21 */
	0, 0, 0, 0,			/* 46 pf22 */
	0, 0, 0, 0,			/* 47 pf23 */
	0, 0, 0, 0,			/* 48 pf24 */
	ESC, '?', 'p', 0,		/* 49 pad 0 -> reset */
	'Y'-CTL, 0, 0, 0,		/* 50 sys_req */
	'T'-CTL, 0, 0, 0,		/* 51 test */
	0, 0, 0, 0,			/* 52 alt_break */
	0, 0, 0, 0,			/* 53 alt_make */
	0, 0, 0, 0,			/* 54 shift_make */
	0, 0, 0, 0,			/* 55 debug_dp */
	0, 0, 0, 0,			/* 56 disconnect */
	0, 0, 0, 0,			/* 57 fix_comm */
	0, 0, 0, 0,			/* 58 interrupt */
	0, 0, 0, 0,			/* 59 pause */
	0, 0, 0, 0,			/* 60 restore_dp */
	0, 0, 0, 0,			/* 61 version_dp */
	3, 0, 0, 0,			/* 62 exit_term */
	4, 0, 0, 0,			/* 63 exit_rgl */
	0, 0, 0, 0,			/* 64 MaxRU */
	0, 0, 0, 0,			/* 65 ttytype */
};

/*  DO NOT CHANGE THE ORDER IN THE TABLE BELOW WITHOUT CHANGING THE DEFINITIONS
**  IN THE INCLUDE FILE
*/
x_key_table R2_5_key_xlat[] = {
	ESC, '?', 'S', 0,		/* 00 reshow. KEEP IT AT 00! */
	'A'-CTL, 0, 0, 0,		/* 01 attention */
	']', 0, 0, 0,			/* 02 bar */
	BS, 0, 0, 0,			/* 03 backspace */
	ESC, 'I', 0, 0,			/* 04 back_tab */
	'[', 0, 0, 0,			/* 05 cent */
	ESC, '?', 'M', 0,		/* 06 pad enter -> clear */
	ESC, 'B', 0, 0,			/* 07 c_down */
	ESC, 'D', 0, 0,			/* 08 c_left */
	ESC, 'C', 0, 0,			/* 09 c_right */
	ESC, 'A', 0, 0,			/* 10 c_up */
	'X'-CTL, 0, 0, 0,		/* 11 cur_select */
	DEL, 0, 0, 0,			/* 12 delete */
	'N'-CTL, 0, 0, 0,		/* 13 dev_cancel */
	'U'-CTL, 0, 0, 0,		/* 14 dup */
	'\r', 0, 0, 0,			/* 15 return -> enter */
	ESC, '?', 'l', 0,		/* 16 pad , -> erase_eof */
	ESC, '?', 'L', 0,		/* 17 shift pad , -> er_input */
	'F'-CTL, 0, 0, 0,		/* 18 f_mark */
	'\t', 0, 0, 0,			/* 19 fwd_tab */
	ESC, BS, 0, 0,			/* 20 ESC bs  -> home */
	ESC, '?', 'n', 0,		/* 21 pad '.' -> insert */
	'\n', 0, 0, 0,			/* 22 new_line */
	ESC, 'S', 0, 0,			/* 23 pf4 -> pa1 */
	ESC, '?', 'm', 0,		/* 24 pad - -> pa2 */
	ESC, 'P', 0, 0,			/* 25 pf1 */
	ESC, 'Q', 0, 0,			/* 26 pf2 */
	ESC, 'R', 0, 0,			/* 27 pf3 */
	ESC, '?', 'w', 0,		/* 28 pad 7 -> pf4 */
	ESC, '?', 'x', 0,		/* 29 pad 8 -> pf5 */
	ESC, '?', 'y', 0,		/* 30 pad 9 -> pf6 */
	ESC, '?', 't', 0,		/* 31 pad 4 -> pf7 */
	ESC, '?', 'u', 0,		/* 32 pad 5 -> pf8 */
	ESC, '?', 'v', 0,		/* 33 pad 6 -> pf9 */
	ESC, '?', 'q', 0,		/* 34 pad 1 -> pf10 */
	ESC, '?', 'r', 0,		/* 35 pad 2 -> pf11 */
	ESC, '?', 's', 0,		/* 36 pad 3 -> pf12 */
	0, 0, 0, 0,			/* 37 pf13 */
	0, 0, 0, 0,			/* 38 pf14 */
	0, 0, 0, 0,			/* 39 pf15 */
	0, 0, 0, 0,			/* 40 pf16 */
	0, 0, 0, 0,			/* 41 pf17 */
	0, 0, 0, 0,			/* 42 pf18 */
	0, 0, 0, 0,			/* 43 pf19 */
	0, 0, 0, 0,			/* 44 pf20 */
	0, 0, 0, 0,			/* 45 pf21 */
	0, 0, 0, 0,			/* 46 pf22 */
	0, 0, 0, 0,			/* 47 pf23 */
	0, 0, 0, 0,			/* 48 pf24 */
	ESC, '?', 'p', 0,		/* 49 pad 0 -> reset */
	'Y'-CTL, 0, 0, 0,		/* 50 sys_req */
	'T'-CTL, 0, 0, 0,		/* 51 test */
	0, 0, 0, 0,			/* 52 alt_break */
	0, 0, 0, 0,			/* 53 alt_make */
	0, 0, 0, 0,			/* 54 shift_make */
	0, 0, 0, 0,			/* 55 debug_dp */
	0, 0, 0, 0,			/* 56 disconnect */
	0, 0, 0, 0,			/* 57 fix_comm */
	0, 0, 0, 0,			/* 58 interrupt */
	0, 0, 0, 0,			/* 59 pause */
	0, 0, 0, 0,			/* 60 restore_dp */
	0, 0, 0, 0,			/* 61 version_dp */
	3, 0, 0, 0,			/* 62 exit_term */
	4, 0, 0, 0,			/* 63 exit_rgl */
	0, 0, 0, 0,			/* 64 MaxRU */
	0, 0, 0, 0,			/* 65 ttytype */
};

char *iris_scrn_strs[] = {
	"\004\033H\033J",		/* C_SCRN */
	"\000",				/* END_S */
	"\002\033K",			/* E_EOL */
	"\004\033Y< ",			/* STRT_S */
	"\000",				/* U1_LN */
	"\000",				/* U2_LN */
	"\004\033Y",			/* XY */
	"\000",				/* reverse screen background */
	"\000",				/* normal screen background */
	"\000",				/* normal video */
	"\000",				/* invisible */
	"\000",				/* intensified */
	"\000",				/* invisible intensified */
	"\000",				/* protected */
	"\000",				/* protected invisible */
	"\000",				/* protected intensified */
	"\000"				/* protected invisible intensified */
};

char *r2_3_scrn_strs[] = {
	"\002\033v",			/* C_SCRN */
	"\002\033K",			/* END_S */
	"\002\033K",			/* E_EOL */
	"\004\033Y8 ",			/* STRT_S */
	"\000",				/* U1_LN */
	"\000",				/* U2_LN */
	"\004\033Y  ",			/* XY */
	"\001 ",			/* reverse screen background */
	"\001 ",			/* normal screen background */
	"\001 ",			/* normal video */
	"\001 ",			/* invisible */
	"\001 ",			/* intensified */
	"\001 ",			/* invisible intensified */
	"\001 ",			/* protected */
	"\001 ",			/* protected invisible */
	"\001 ",			/* protected intensified */
	"\001 "				/* protected invisible intensified */
};

char *r2_4_scrn_strs[] = {
	"\002\033v",			/* C_SCRN */
	"\000",				/* END_S */
	"\002\033K",			/* E_EOL */
	"\004\033Y8 ",			/* STRT_S */
	"\000",				/* U1_LN */
	"\000",				/* U2_LN */
	"\004\033Y  ",			/* XY */
	"\004\0339  ",			/* reverse screen background */
	"\004\0330  ",			/* normal screen background */
	"\004\0330  ",			/* normal video */
	"\001 ",			/* invisible */
	"\004\0339  ",			/* intensified */
	"\001 ",			/* invisible intensified */
	"\004\0330  ",			/* protected */
	"\001 ",			/* protected invisible */
	"\004\0339  ",			/* protected intensified */
	"\001 "				/* protected invisible intensified */
};

char *ansi_scrn_strs[] = {
	"\006\033[H\033[J",		/* C_SCRN */
	"\000",				/* END_S */
	"\003\033[K",			/* E_EOL */
	"\000",				/* STRT_S */
	"\000",				/* U1_LN */
	"\000",				/* U2_LN */
	"\010\033[01;01H",		/* XY */
	"\000",				/* reverse screen background */
	"\000",				/* normal screen background */
	"\005\033[0m ",			/* normal video */
	"\001 ",			/* invisible */
	"\005\033[1m ",			/* intensified */
	"\001 ",			/* invisible intensified */
	"\005\033[0m ",			/* protected */
	"\001 ",			/* protected invisible */
	"\005\033[1m ",			/* protected intensified */
	"\001 "				/* protected invisible intensified */
};

static char keypad_alt[] = { ESC,"=" };
static char keypad_num[] = { ESC,">" };
#define	L_X_KEY_XLAT	sizeof(Iris_key_xlat)/sizeof(Iris_key_xlat[0])
x_key_table	X_key_xlat[L_X_KEY_XLAT];
 

char *configfile;
char keyfile[] = { CONFIGFILE };
struct {
	char *k_name;
	x_key_table *k_ptr;
} kl[] = {
	{"redraw",X_key_xlat},
	{"attn",&X_key_xlat[1]},
	{"bar",&X_key_xlat[2]},
	{"bs",&X_key_xlat[3]},
	{"btab",&X_key_xlat[4]},
	{"cent",&X_key_xlat[5]},
	{"clear",&X_key_xlat[6]},
	{"c_dn",&X_key_xlat[7]},
	{"c_lf",&X_key_xlat[8]},
	{"c_rt",&X_key_xlat[9]},
	{"c_up",&X_key_xlat[10]},
	{"c_sel",&X_key_xlat[11]},
	{"del",&X_key_xlat[12]},
	{"cancel",&X_key_xlat[13]},
	{"dup",&X_key_xlat[14]},
	{"enter",&X_key_xlat[15]},
	{"er_eof",&X_key_xlat[16]},
	{"erinp",&X_key_xlat[17]},
	{"fm",&X_key_xlat[18]},
	{"ftab",&X_key_xlat[19]},
	{"home",&X_key_xlat[20]},
	{"insert",&X_key_xlat[21]},
	{"newline",&X_key_xlat[22]},
	{"pa1",&X_key_xlat[23]},
	{"pa2",&X_key_xlat[24]},
	{"pf1",&X_key_xlat[25]},
	{"pf2",&X_key_xlat[26]},
	{"pf3",&X_key_xlat[27]},
	{"pf4",&X_key_xlat[28]},
	{"pf5",&X_key_xlat[29]},
	{"pf6",&X_key_xlat[30]},
	{"pf7",&X_key_xlat[31]},
	{"pf8",&X_key_xlat[32]},
	{"pf9",&X_key_xlat[33]},
	{"pf10",&X_key_xlat[34]},
	{"pf11",&X_key_xlat[35]},
	{"pf12",&X_key_xlat[36]},
	{"pf13",&X_key_xlat[37]},
	{"pf14",&X_key_xlat[38]},
	{"pf15",&X_key_xlat[39]},
	{"pf16",&X_key_xlat[40]},
	{"pf17",&X_key_xlat[41]},
	{"pf18",&X_key_xlat[42]},
	{"pf19",&X_key_xlat[43]},
	{"pf20",&X_key_xlat[44]},
	{"pf21",&X_key_xlat[45]},
	{"pf22",&X_key_xlat[46]},
	{"pf23",&X_key_xlat[47]},
	{"pf24",&X_key_xlat[48]},
	{"reset",&X_key_xlat[49]},
	{"sys_req",&X_key_xlat[50]},
	{"test",&X_key_xlat[51]},
	{"alt_m",&X_key_xlat[52]},
	{"alt_b",&X_key_xlat[53]},
	{"shift",&X_key_xlat[54]},
	{"debug_dp",&X_key_xlat[55]},
	{"disconnect",&X_key_xlat[56]},
	{"fix_comm",&X_key_xlat[57]},
	{"interrupt",&X_key_xlat[58]},
	{"pause",&X_key_xlat[59]},
	{"restore_dp",&X_key_xlat[60]},
	{"version_dp",&X_key_xlat[61]},
	{"exit_term",&X_key_xlat[62]},
	{"exit_rgl",&X_key_xlat[63]},
	{"F3174",&X_key_xlat[64]},
	{"NULL",X_key_xlat},
};
#define N_KL (sizeof(kl) / sizeof(kl[0]))

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
	NORMAL, 0x00,		/* 55 debug_dp */
	NORMAL, 0x00,		/* 56 disconnect */
	NORMAL, 0x00,		/* 57 fix_comm */
	NORMAL, 0x00,		/* 58 interrupt */
	NORMAL, 0x00,		/* 59 pause */
	NORMAL, 0x00,		/* 60 restore_dp */
	NORMAL, 0x00,		/* 61 version_dp */
	NORMAL, 0x00,		/* 62 exit_term */
	NORMAL, 0x00,		/* 63 exit_rgl */
	NORMAL, 0x00,		/* 64 MaxRU */
	NORMAL, 0x00,	/* 65 ttytype */
};


struct {
	char *tt_name;
	char **scrn_strs;
	x_key_table *key_xlat;
	short display;		/* 0 graphic, 1 text, 2 text w/attributes */
	short keyboard;		/* 0 device, 1 stdin, 2 stdin with alt pf */
} trms[] = { {
	"wsiris",
	iris_scrn_strs,
	Iris_key_xlat, 0, 0
}, {
	"v50am",
	r2_3_scrn_strs,
	R2_4_key_xlat, 1, 1
}, {
	"iris",
	r2_4_scrn_strs,
	R2_4_key_xlat, 1, 2
}, {
	"vt52",
	r2_3_scrn_strs,
	R2_5_key_xlat, 1, 2
}, {
	"ansi",
	ansi_scrn_strs,
	Iris_key_xlat, 2, 2
}, {
	"vt100",
	ansi_scrn_strs,
	R2_5_key_xlat, 2, 1
} };
#define N_TRMS (sizeof(trms) / sizeof(trms[0]))

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
	'1', 'I'-CTL, 'q', 'a', 's',
	0,			/* no scrl */
	'2', '3', 'w', 'e', 'd',
	'f', 'z', 'x', '4', '5',
	'r', 't', 'g', 'h', 'c',
	'v', '6', '7', 'y', 'u',
	'j', 'k', 'b', 'n', '8',
	'9', 'i', 'o', 'l', ';',
	'm', ',', '0', '-', 'p',
	'[', '\'', 'M'-CTL, '.', '/',
	'=', '`', ']', '\\',
	0xb1,			/* pad 1 */
	0xb0,			/* pad 0 */
	'J'-CTL, BS,
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
	'1', 'I'-CTL, 'Q'-CTL, 'A'-CTL, 'S'-CTL,
	0,			/* no scrl */
	'2', '3', 'W'-CTL, 'E'-CTL, 'D'-CTL,
	'F'-CTL, 'Z'-CTL, 'X'-CTL, '4', '5',
	'R'-CTL, 'T'-CTL, 'G'-CTL, 'H'-CTL, '@'-CTL,
	'V'-CTL, '6', '7', 'Y'-CTL, 'U'-CTL,
	'J'-CTL, 'K'-CTL, 'B'-CTL, 'N'-CTL, '8',
	'9', 'I'-CTL, 'O'-CTL, 'L'-CTL, ';',
	'M'-CTL, ',', '0', '_'-CTL, 'P'-CTL,
	'['-CTL, '\'', 'M'-CTL, '.', '/',
	'=', '`', ']'-CTL, '\\'-CTL,
	0xb1,			/* pad 1 */
	0xb0,			/* pad 0 */
	'J'-CTL, BS,
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
	'P'-CTL,		/* no scrl */
	'@', '#', 'W', 'E', 'D',
	'F', 'Z', 'X', '$', '%',
	'R', 'T', 'G', 'H', 'C',
	'V', '^', '&', 'Y', 'U',
	'J', 'K', 'B', 'N', '*',
	'(', 'I', 'O', 'L', ':',
	'M', '<', ')', '_', 'P',
	'{', '"', 'J'-CTL, '>', '?',
	'+', '~', '}', '|',
	0xb1,			/* pad 1 */
	0xb0,			/* pad 0 */
	'J'-CTL, BS,
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
	'1', 'I'-CTL, 'Q', 'A', 'S',
	0,			/* no scrl */
	'2', '3', 'W', 'E', 'D',
	'F', 'Z', 'X', '4', '5',
	'R', 'T', 'G', 'H', 'C',
	'V', '6', '7', 'Y', 'U',
	'J', 'K', 'B', 'N', '8',
	'9', 'I', 'O', 'L', ';',
	'M', ',', '0', '-', 'P',
	'[', '\'', 'M'-CTL, '.', '/',
	'=', '`', ']', '\\',
	0xb1,			/* pad 1 */
	0xb0,			/* pad 0 */
	'J'-CTL, BS,
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
	'P'-CTL,		/* no scrl */
	'@', '#', 'w', 'e', 'd',
	'f', 'z', 'x', '$', '%',
	'r', 't', 'g', 'h', 'c',
	'v', '^', '&', 'y', 'u',
	'j', 'k', 'b', 'n', '*',
	'(', 'i', 'o', 'l', ':',
	'm', ',', ')', '_', 'p',
	'{', '"', 'J'-CTL, '>', '?',
	'+', '~', '}', '|',
	0xb1,			/* pad 1 */
	0xb0,			/* pad 0 */
	'J'-CTL, BS,
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

struct {
	char *k_name;
} kn[] = {
	{"normal    "},
	{"control   "},
	{"shift     "},
	{"CAPS      "},
	{"shift CAPS"},
};

u_char		buf[PXBUFSIZ];		/* file read buffer */
Device		button;
char		*evaluep;
u_char		fbuf[10];		/* field input buffer */
int		kfile;
u_char		pbuf[10];

/*
**	Global variables
*/

u_char	L_x_key_xlat =	L_X_KEY_XLAT;


/*
**	Turn on/off the trace of this module
*/
tr_conio(tflag)
{
	trace = tflag;
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
register u_char attrb;
{
	im_setup;
	register u_char battr_index;

	if (Display) {
		battrb = attrb & 0x08;
		battr_index = battrb ? 0 : 1;
#ifdef DEBUG
		DT("BACKGRND %02x index = %d ", attrb, battr_index);
#endif /* DEBUG */
		if (*battrb_str[battr_index])
			(void)write (ttyp, battrb_str[battr_index]+1, *battrb_str[battr_index]);
	} else {
#ifdef DEBUG
		DT("BACKGRND %02x -> %d\n", attrb, attrb2col(attrb));
#endif /* DEBUG */
		Ch_color = attrb2col(attrb);
		im_color(Ch_color);
	}
}

/*
**	Sound the bell
*/
beep()
{
	static u_char bell = '\007';

	if (Display)
		(void)write (ttyp, &bell, 1);
	else
		ringbell();
}

/*
**	Convert a button to an ascii character based on the state of the kbd
*/
u_char
but2c(Button,kb_status)
Device Button;
register u_char kb_status;
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
	DT("button_xlst[%d][%d] -> %02x %c\n", i,Button-BREAKKEY,
	    button_xlat[i][Button-BREAKKEY], button_xlat[i][Button-BREAKKEY]);
#endif /* DEBUG */
	return (button_xlat[i][Button-BREAKKEY]);
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
	if (Display)
		(void)write(ttyp, c_scrn_str + 1, c_scrn_str[0]);
	else {
#ifdef DEBUG
		DT("Clear_screen ");
#endif /* DEBUG */
		im_color(BLACK);
		im_rectfi(Offst_x,Offst_y,Offst_x+722,Offst_y+373);
		im_color(WHITE);
		im_rectfi(Offst_x,Offst_y+12,Offst_x+722,Offst_y+373);
		xy(1,1);
	}
}

/*
**	Reset the console flags
*/
conclose(flag)
{
#ifdef DEBUG
	DT("CONCLOSE ");
#endif /* DEBUG */
	if (Keyboard) {
#ifdef DEBUG
		DT("restore ttyp ");
#endif /* DEBUG */
		restoremode(ttyp);
		if (Keyboard == 2) {
#ifdef DEBUG
			DT("keypad_num ");
#endif /* DEBUG */
			(void)write(ttyp, keypad_num, sizeof(keypad_num));
		}
	} else {
		lampoff(0x0f);
		if (flag) {
			kb_status = 0;
			unqkeys();
			gflush();
			gexit();
		}
	}
}

/*
**	Get a character from the console.  Return 0 if none avail yet else 1.
*/
conin(cp)
register u_char *cp;
{
	register n;
	short data;

	if (Keyboard) {
		if (n =  read(ttyp, cp, 1)) {
#ifdef DEBUG
			if (*cp > ' ' && *cp <= '~') {
				DT("CONIN: %02x %c\n", *cp,*cp);
			} else {
				DT("CONIN: %02x\n", *cp);
			}
#endif /* DEBUG */
		}
		return (n);
	} else {
	 	while ((button = qtest(&data)) != NULLDEV && (button < BUT0 ||
	 	    button > LEFTMOUSE)) {
	 		button = qread(&data);	/* gobble up unknown input */
			if (button == REDRAW) {
				repaint(1,24);
				Force_show_screen = TRUE;
				show_screen();
				Force_show_screen = FALSE;
			}
		}
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
	 		return 0;
	 	case RIGHTMOUSE:
	 		if (!data)
				return 0;
			*cp = (u_char)0xfa;
			return CNTU;
		case MIDDLEMOUSE:
			if (!data)
				return 0;
			*cp = (u_char)0xfb;
			return CNTU;
		case LEFTMOUSE:
			if (!data)
				return 0;
			*cp = (u_char)0xfc;
			return CNTU;
		default:
			kstate[button] = (u_char)data;
			if (data == 0)		/* key released */
				return 0;
			*cp = but2c(button,kb_status);
#ifdef DEBUG
			DT("CONIN:%02x ", *cp);
#endif /* DEBUG */
			return CNTU;
		}
	}
}

/*
**	Initialize the console I/O code
*/
conopen(flag)
{

#ifdef DEBUG
	DT("CONOPEN ttytype %d ",ttytype);
#endif /* DEBUG */
	if (Keyboard) {
		if (!ttyp) {
#ifdef DEBUG
			DT("Open /dev/tty ");
#endif /* DEBUG */
			if ((ttyp = open("/dev/tty", O_RDWR | O_NDELAY, 0))<=0) {
				if (errno < sys_nerr)
					(void)perror("pxd:conopen '/dev/tty' error ");
				else
					(void)printf("Cannot open '/dev/tty' - errno = %d\n", errno);
				exit(1);
			}
		}
#ifdef DEBUG
		DT("Rawmode ");
#endif /* DEBUG */
		rawmode(ttyp);
		if (Keyboard == 2) {
#ifdef DEBUG
			DT("keypad_alt ");
#endif /* DEBUG */
			(void)write(ttyp, keypad_alt, sizeof(keypad_alt));
		}
	} else {			/* iris keyboard */
		kb_status = 0;
		if (flag)
			qkeys(flag);
		else if (Display == 0)
			if (zflag[1])
				gbegin();
			else
				ginit();
		lampoff(0x0f);
	}
}

/*
**	Display a character
*/
conout(c)
u_char c;
{
	im_setup;
	static char str[] = " ";

	if (Display)
		(void)write (ttyp, &c, 1);
	else {
		im_color(BLACK);
		im_rectfi(Scrn_x+Offst_x,Scrn_y+Offst_y-2, Scrn_x+Offst_x+8,Scrn_y+Offst_y+12);
		im_color(Ch_color);
		im_cmov2i(Scrn_x+Offst_x,Scrn_y+Offst_y);
		str[0] = (char)c;
		charstr(str);
		Scrn_x += 9;

	}
}

db_conopen()
{
	register long ru_size;
	register u_char *p;

	if (ttytype == 0) {
		update_ttytype();
		Display = trms[ttytype].display;
		Keyboard = trms[ttytype].keyboard;
		get_rc();
	}
	do_conopen();
	p = (u_char *)X_key_xlat + ((L_X_KEY_XLAT-2) * sizeof(X_key_xlat[0]));
	*p++ = Record_sep;
	p++;
	ru_size = MaxRU;
	*p++ = ru_size >> 8;
	*p++ = ru_size;
	*p++ = Display;
	*p++ = Keyboard;
	*p++ = ttytype;
	*p = F3174;
	if (ismex())
		Offst_x = Offst_y = 0;
	else {
		Offst_x = 150;
		Offst_y = 390;
	}
}



/*
**	Initialize the console I/O code
*/
do_conopen()
{
	register u_char i, j;

#ifdef DEBUG
	DT("DO_conopen ttytype is %d\n",ttytype);
#endif /* DEBUG */
	/* THE FOLLOWING CODE SHOULD REALLY USE THE TERMCAP FILE */
	strcpy(c_scrn_str, trms[ttytype].scrn_strs[0]);
	strcpy(end_s_str, trms[ttytype].scrn_strs[1]);
	strcpy(e_eol_str, trms[ttytype].scrn_strs[2]);
	strcpy(strt_s_str, trms[ttytype].scrn_strs[3]);
	strcpy(u1_ln_str, trms[ttytype].scrn_strs[4]);
	strcpy(u2_ln_str, trms[ttytype].scrn_strs[5]);
	strcpy(xy_str, trms[ttytype].scrn_strs[6]);
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
register u_char attrb;
{
	im_setup;

	if (Display) {
		attrb ^= battrb;
		attrb = attrb >> 3 & 0x04 | attrb >> 2 & 0x3;
#ifdef DEBUG
		DT(" Attribute no %d->%s. ", attrb, attrb_str[attrb]+2);
#endif /* DEBUG */
		(void)write (ttyp, attrb_str[attrb]+1, *attrb_str[attrb]);
	} else {
#ifdef DEBUG
		DT("DISP_ATTRB %02x -> %d\n", attrb, attrb2col(attrb));
#endif /* DEBUG */
		conout(' ');
		im_color(Ch_color = (Colorindex)attrb2col(attrb));
	}
}

/*
**	Send end of user status info sequence to terminal
*/
end_status()
{
	if (Display && end_s_str[0])
		(void)write(ttyp, end_s_str + 1, end_s_str[0]);
}

/*
**	Un-display the cursor on the iris terminal
*/
erase_cursor(r,c)
register u_char r, c;
{
	im_setup;
	register Icoord x,y;

	if (Display == 0) {
		im_color(BLACK);
		x = 9*(c-1) + Offst_x;
		y = 15*(25-r) + Offst_y;
		im_rectfi(x,y-2,x+8,y+12);
	}
}

/*
**	Erase to end of line
*/
erase_eol()
{
	im_setup;

	if (Display)
		(void)write(ttyp, e_eol_str + 1, e_eol_str[0]);
	else {
		im_color(BLACK);
		im_rectfi(Scrn_x+Offst_x,Scrn_y+Offst_y-2, Offst_x+722,Scrn_y+Offst_y+12);
	}
}

/*
**	Normalize 3274 keyboard state from unknown state
*/
normalize_kb() 
{
#ifdef DEBUG
	DT("norm kb\n");
#endif /* DEBUG */
	send_x_key(X_ALT_MAKE);
	send_x_key(X_RESET);
	pdelay(MS_15);
}

/*
**	Print keyboard mapping
*/
print_keymap()
{
	if (Keyboard == 0) {
		printf("\r\n PF 1-12 are pad PF1-4 789-456,  on keyboard");
		printf("\r\n EXIT EMULATION is CTRL-d 		EXIT GRAPHICS is ~. or ESC DEL");
		printf("\r\n ATTN CTRL-a    BACK TAB shift TAB    BAR ']'   CANCEL CTRL-n    CENT '['");
		printf("\r\n CLEAR pad 'ENTER'   DUP CTRL-u   ERASE EOF pad '1'   ERASE INPUT pad '2'");
		printf("\r\n FM CTRL-f    HOME pad '.'    INSERT CTRL-p    PA1,2 are CTRL-PF1,2");
		printf("\r\n RESET pad '0'    SYS REQ CTRL-r    CURSR SEL CTRL-x    TEST CTRL-t");
	} else  if (Keyboard == 1) {
		printf("\n 	      PF 1-12 are ESC 1234567890-=");
		printf("\n	EXIT EMULATION is CTRL-d  EXIT GRAPHICS is ESC DEL");
		printf("\n ATTN CTRL-a    BAR ']'    CANCEL CTRL-n    CENT '['");
		printf("\n CLEAR is ESC ENTER   CURSR SEL CTRL-x   DUP CTRL-u");
		printf("\n ERASE EOF is ESC '/'   FM CTRL-f    INSERT CTRL-p");
		printf("\n PA1/2 are ESC PF1/2   RESET CTRL-r    SYS REQ CTRL-y       TEST CTRL-t");
	} else if (Keyboard == 2) {
		printf("\n 	      PF 1-12 are pad PF1-4 789-456,");
		printf("\n	EXIT EMULATION is CTRL-d  EXIT GRAPHICS is ESC DEL");
		printf("\n ATTN CTRL-a    BAR ']'    CANCEL CTRL-n    CENT '['");
		printf("\n CLEAR pad 'ENTER'   CURSR SEL CTRL-x   DUP CTRL-u   ERASE EOF pad '1'");
		printf("\n ERASE INPUT pad '2'   FM CTRL-f    HOME pad '.'    INSERT CTRL-p");
		printf("\n PA1/2 are ESC PF1/2   RESET pad '0'    SYS REQ CTRL-y       TEST CTRL-t");
	}
}

/*
**	Queue all the keys on the iris console
*/
qkeys(flag)
{

	if (Keyboard == 0) {
#ifdef DEBUG
		DT("Queue keys\n");
#endif /* DEBUG */
		if (Display == 0)
			prefsize(722,375);
		else {
			prefsize(42,35);
			prefposition(722,375);
		}
		foreground();
		if (flag == 2)
			noport();
		getport(trms[0].tt_name);
		wintitle("3270 Emulation");
		winattach();
		for (button=BUT1; button<=MAXKBDBUT; ++button)
			qdevice(button);
		qreset();
	} else {
#ifdef DEBUG
		DT("Q Open /dev/tty ");
#endif /* DEBUG */
		if ((ttyp = open("/dev/tty", O_RDWR | O_NDELAY, 0))<=0) {
			if (errno < sys_nerr)
				(void)perror("pxd:qkeys '/dev/tty' error ");
			else
				(void)printf("Cannot open '/dev/tty' - errno = %d\n", errno);
			exit(1);
		}
#ifdef DEBUG
		DT("Rawmode ");
#endif /* DEBUG */
		rawmode(ttyp);
	}
#ifdef DEBUG
	DT("Queue keys\n");
#endif /* DEBUG */
}

/*
**	Set the color for subsequent characters
*/
set_color(attrb)
u_char attrb;
{
	im_setup;

	if (Display == 0)
		im_color(Ch_color = (Colorindex)attrb2col(attrb));
}

/*
**	Display the cursor on the iris terminal
*/
show_cursor(c)
u_char c;
{
	im_setup;
	static char str[] = " ";

	if (Display == 0) {
		im_color(WHITE);
		im_rectfi(Scrn_x+Offst_x,Scrn_y+Offst_y-2, Scrn_x+Offst_x+8,Scrn_y+Offst_y+12);
		im_color(BLACK);
		im_cmov2i(Scrn_x+Offst_x,Scrn_y+Offst_y);
		str[0] = (char)c;
		charstr(str);
	}
}

/*
**	Lead in for user's status on line 25
*/
start_status()
{
	im_setup;

	if (Display) {
		if (strt_s_str[0] != 0)
			(void)write(ttyp, strt_s_str+1, strt_s_str[0]);
	} else {
		xy(25,1);
		im_color(Ch_color = WHITE);
	}
}

/*
**	Display string of given length
*/
strnout(s,len)
register u_char *s;
{
	im_setup;

	if (Display) {
#ifdef DEBUG
		DT("s%d ", len);
#endif /* DEBUG */
		(void)write (ttyp, s, len);
	} else {
		im_color(BLACK);
		im_rectfi(Scrn_x+Offst_x,Scrn_y+Offst_y-2, Scrn_x+Offst_x+9*len,Scrn_y+Offst_y+12);
		im_color(Ch_color);
		(void)strncpy(buf, s, len);
		buf[len] = '\0';
		charstr(buf);
		Scrn_x += 9 * len;
	}
}


/*
**	Unqueue all the keys on the iris console
*/
unqkeys()
{

	if (Keyboard == 0) {
#ifdef DEBUG
		DT("Unqueue keys\n");
#endif /* DEBUG */
		for (button=BUT1; button<=MAXKBDBUT; ++button)
			unqdevice(button);
		qreset();
	}
}


/*
**	Cursor positioning
*/
xy(r,c)
register u_char r, c;
{
	im_setup; 

	if (Display) {
#ifdef DEBUG
		DT("rc %d,%d ",r,c);
#endif /* DEBUG */
		if (xy_str[0] == 4) {
			xy_str[3] = (char)r + 31;
			xy_str[4] = (char)c + 31;
		} else {
			xy_str[3] = (r/10) + '0';
			xy_str[4] = (r%10) + '0';
			xy_str[6] = (c/10) + '0';
			xy_str[7] = (c%10) + '0';
		}
		(void)write(ttyp, xy_str+1, xy_str[0]);
	} else {
		im_cmov2i((Scrn_x = 9*(c-1))+Offst_x, (Scrn_y = 15*(25-r))+Offst_y);
	}
}

/*
** THE REMAINING CODE IMPLIMENTS MENU 8, THE KEYBOARD AND DISPLAY CUSTOMIZER
*/
u_char
get_key()
{
	u_char c;

	errno = 0;
	while (!conin(&c))
		;
	return(c);
}


get_rc()
{
	register u_char *p, *x_key_ptr;
	register int i, n;
	char *home;

	if ((kfile = open(keyfile, O_RDWR)) > 0) {
		configfile = keyfile;
		if ((i = read(kfile,pbuf,8)) <= 0) {
			printf("Read zero bytes from %s\r\n",configfile);
			goto home_read;
		}
		if ((evaluep = getenv("TERM")) != NULL) {
			if (strcmp(evaluep, pbuf) == 0)
				goto read_buf;
		}
		goto home_read;
	} else {

home_read:
		(void)perror(".t3279rc open failed");
		if ((home = concat(getenv("HOME"), "/"))
		&& (configfile = concat(home, CONFIGFILE))
	 	&& (kfile = open(configfile, O_RDWR)) <= 0) {
			printf("%s open failed \r\n",configfile);
			(void)perror("****** open error ****** ");
			goto etc_read;
		}
	}
	if ((i = read(kfile,pbuf,8)) <= 0) {
		printf("Read zero bytes from %s\r\n",configfile);
		goto etc_read;
	}
	if ((evaluep = getenv("TERM"))!=NULL) {
		if (strcmp(evaluep, pbuf) != 0) {
			printf(" %s TERM is %s, so not used\r\n",configfile,pbuf);
			configfile = keyfile;	/* revert to .t3279rc   wpc */
			goto etc_read;
		}
	}
	goto read_buf;

etc_read:
	if ((kfile = open(ETCCONFIGFILE, O_RDONLY)) <= 0) { /* read_only  wpc */
		(void)perror("/usr/etc/t3279rc open failed ");
		goto skip_read;
	}
	
	if ((i = read(kfile,pbuf,8)) <= 0) {
		printf("Read zero bytes from %s\r\n",ETCCONFIGFILE);
		goto skip_read;
	}
	if ((evaluep = getenv("TERM"))!=NULL) {
		if (strcmp(evaluep, pbuf) != 0) {
			printf(" %s TERM is %s, so not used\r\n",ETCCONFIGFILE,pbuf);
			goto skip_read;
		}
	}

read_buf:
	if ((i = read(kfile, buf, sizeof(buf))) <= 0) {
		printf("Read zero bytes from %s\r\n",configfile);
		goto skip_read;
	}
#ifdef DEBUG
	DT("moving %s to current values ",CONFIGFILE);
#endif /* DEBUG */
	x_key_ptr = fbuf;
	p = buf + sizeof(X_key_xlat) + sizeof(Key_xlat);
	if (kfile && i) {
		if (*p++ || *p) {	/* ! packed.key file */
			printf("%s is NOT a %s type file\r\n",configfile,CONFIGFILE);
skip_read:
			blt((char *)X_key_xlat, (char *)trms[ttytype].key_xlat,
			    sizeof(X_key_xlat));
		} else {
			n = sizeof(X_key_xlat);
			p = (u_char *)X_key_xlat;
			x_key_ptr = buf;
			while (n--)
				*p++ = *x_key_ptr++;
			p--;
			F3174 = *p--;
			ttytype = *p--;
			Keyboard = *p--;
			Display = *p--;
			n = 0;
			n = *(p-1);
			n <<= 8;
			n += *p;
			MaxRU = n;
			Record_sep = *(p-3);
			kb_changed++;
			n = sizeof(Key_xlat);
			p = (u_char *)Key_xlat;
			while (n--)
				*p++ = *x_key_ptr++;
			n = sizeof(X_key_codes);
			p = (u_char *)X_key_codes;
			while (n--)
				*p++ = *x_key_ptr++;
			if (!Keyboard) {
				n = sizeof(button_xlat);
				p = (u_char *)button_xlat;
				while (n--)
					*p++ = *x_key_ptr++;
			}
		}
	}
	close(kfile);
	errno = 0;
}


u_char
get_yn()
{
	u_char c, c1 = 'n';
	int n = 0;

get_a_key:
	errno = 0;
	while (!conin(&c))
		;
	n++;
	if (c >= ' ' && c < 0x7f)
		printf("%c",c);
	if (c == BS) {
		if (n > 1) {
			printf("\b \b");
			n -= 2;		/* backup one */
		} else
			n = 0;		/* ignore */
	}
	if (c == '\r')
		return(c1);
	else
		c1 = c;
	goto get_a_key;
}

/*
**	Menu 8 updates display choice, keyboard choice, keystroke definition
**	3270 function definitions and 3174/3274 interface codes
*/
rdcon()
{
	register u_char c, *p, *x_key_ptr;
	register int i;

/*wpc	get_rc();            no need to get default value        wpc */
	get_presentation();
	get_key_changes();
/*wpc	get_3270_changes();  controller and maxRU changes        wpc */
	printf("\r\n Do you want to save tables as '%s' (y/n) <n>: ",configfile);
	c = get_yn();
	conclose(1); 
	if (c != 'y' && c != 'Y') {
		printf("\r\n no changes saved");
		return;
	}
	if ((kfile = creat(configfile,0666))<=0) {
		(void)perror(".t3279rc ");
		return;
	} 

	p = pbuf;
	x_key_ptr = (u_char *)evaluep;
	for(i = 0; i < 8; i++)
		*p++ = *x_key_ptr++;
	if ((i = write(kfile,pbuf,8)) <= 0) {
		printf("\r\nWrote zero bytes to %s file\n",configfile);
		(void)perror(".t3279rc ");
		return;
	}
	if ((i = write(kfile,X_key_xlat,sizeof(X_key_xlat))) <= 0) {
		(void)perror(".t3279rc ");
		return;
	}
	if ((i = write(kfile,Key_xlat,sizeof(Key_xlat))) <= 0) {
		(void)perror(".t3279rc ");
		return;
	}
	if ((i = write(kfile,X_key_codes,sizeof(X_key_codes))) <= 0) {
		(void)perror(".t3279rc ");
		return;
	}
	if (!Keyboard)
		if ((i = write(kfile,button_xlat,sizeof(button_xlat))) <= 0) {
			(void)perror(".t3279rc ");
			return;
		}
	close(kfile);
}


/*
**  get the 3174/3274 parameters updated 
*/
get_3270_changes()
{
	register u_char c;
	register int index;
	int i;
	extern u_char get_yn(), get_key();

	c = F3174 ? '1' : '2' ;
	printf("\r\n\tThe controller interface is 3%c74",c);
	printf("\r\n Do you want to change controller model (y/n) <n>: ");
	c = get_yn();
	if (c != 'y' && c != 'Y')
		goto get_ru;
	i = F3174 ? 3274 : 3174 ;
	printf("\r\n Enter the model (3174/3274) <%d>: ", i);
	errno = 0; i = 0;
	while ((c = get_key()) != '\r') {
		fbuf[i++] = c;
		if (c >= ' ' && c < 0x7f)
			printf("%c",c);
		if (c == BS) {
			if (i > 1) {
				printf("\b \b");
				i -= 2;		/* backup one */
			} else
				i = 0;		/* ignore */
		}
	}
	if (i == 0)
		F3174 = F3174 ? 0 : 1 ;
	else if (fbuf[1] == '2')
		F3174 = 0;
	else
		F3174 = 1;
	set_3174();
	c = F3174 ? '1' : '2' ;
	printf("\r\n\tThe controller interface is now 3%c74",c);
get_ru:
	printf("\r\n\tThe current max RU size is %d",MaxRU);
	index = F3174 ? (ROWS-1)*COLS : 4*SMALL_DMA;
	printf("\r\n\tThe largest max RU size is %d", index);
	printf("\r\n Do you want to change RU size (y/n) <n>: ");
	c = get_yn();
	if (c != 'y' && c != 'Y')
		return;
	printf("\r\n Enter the max RU");
	if (F3174)
		printf(" modulo 128 required ");
	else
		printf(" modulo 16 required ");
	printf("(1024/1536/1920");
	if (!F3174)
		printf("/15216/60864");
	printf(") <%d>: ",DEFAULTRU);
	errno = 0; i = 0;
	while ((c = get_key()) != '\r') {
		fbuf[i++] = c;
		if (c >= ' ' && c < 0x7f)
			printf("%c",c);
		if (c == BS) {
			if (i > 1) {
				printf("\b \b");
				i -= 2;		/* backup one */
			} else
				i = 0;		/* ignore */
		}
	}
	fbuf[i] = 0;
	if (i == 0)
		MaxRU = DEFAULTRU;
	else {
		sscanf(fbuf,"%d",&i);
		if (F3174)
			MaxRU = i > index ? index : i & 0xffffff80;
		else
			MaxRU = i > index ? index : i & 0xfffffff0;
	}
	set_3174();
	printf("\r\n\tThe max RU size is now %d",MaxRU);
}


/*
**	Ask for changes to 3270 function key definitions
*/
get_key_changes()
{
	register u_char c, *x_key_ptr;
	register int i, index;

	kb_changed++;			/* force detailed list of keys */
	system("clear");                /* clear screen            wpc */
	printf(" Current 3270 key definitions:\r\n\n");        /*  wpc */
	print_keys();
	kb_changed--;
	printf("\r\n\n Do you want to change any of the above 3270 keys (y/n) <n>: ");
	c = get_yn();
	if (c != 'y' && c != 'Y') {
		system("clear");
		goto get_codes;
		}
next_stroke:
	(void)printf("\r\n Type name of 3270 key to change: ");	
	errno = 0; i = 0;
	while ((c = get_key()) != '\r') {
		fbuf[i++] = c;
		if (c >= ' ' && c < 0x7f)
			printf("%c",c);
		if (c == BS) {
			if (i > 1) {
				printf("\b \b");
				i -= 2;		/* backup one */
			} else
				i = 0;		/* ignore */
		}
	}
	if (i == 0) {
		printf("\r\n");
		print_keys();
		printf("\r\n Are you done with 3270 key changes (y/n) <n>: ");
		c = get_yn();
		if (c == 'y' || c == 'Y') {
			printf("\r\n");
			goto get_codes;
		}
		goto next_stroke;
	}
	fbuf[i] = 0;
	/*
	** add i = 1 logic to edit Key_table
	*/
	if (i == 1) {
		printf("\r\n 3270 key name error %s\r\n",fbuf);
		print_keys();
		goto next_stroke;
	}
	for (index = 0; index < N_KL-2; index++) {
		if (!strcmp(fbuf,kl[index].k_name))
			break;
	}
	if (index == N_KL-2) {
		printf("\r\n 3270 key name error %s\r\n",fbuf);
		print_keys();
		goto next_stroke;
	}
	printf("\r\n\t          The current definition: ");
	x_key_ptr = (u_char *)kl[index].k_ptr;
	i = 3;
	do {
		c = *x_key_ptr++;
		if (c == 0)
			break;
		prinit(c);
		printf(" ");
	} while (i-- > 0);
	printf("\r\n     Stroke new key(s) for %s -> ",kl[index].k_name);
	errno = 0; i = 0;
	x_key_ptr = (u_char *)kl[index].k_ptr;
	do {
		c = get_key();
		if (c == '\r')
			break;
		fbuf[i++] = c;
		prinit(c);
		printf(" ");
	} while (i < 4 );
	if (i == 0) {
		printf("\r\n Do you want to use RETRN key as value (y/n) ? <n>: ");
		c = get_yn();
		if (c == 'y' || c == 'Y') {
			*x_key_ptr++ = '\r';
			*x_key_ptr++ = 0;
			*x_key_ptr++ = 0;
			*x_key_ptr++ = 0;
			kb_changed++;
		} else {
			printf(" no change made ");
		}
	} else {
		c = 0;
		while (i--)
			*x_key_ptr++ = fbuf[c++];
		for (i = 4 - c; i; i--)
			*x_key_ptr++ = 0;
		kb_changed++;
	}
	goto next_stroke;

get_codes:
	printf("\r\n Current 3270 scan codes:\r\n");    /* use heading wpc*/
	print_codes();
	printf("\r\n\n Do you want to change 3270 scan codes (y/n) <n>: ");
	c = get_yn();
	if (c != 'y' && c != 'Y')
		return;

next_key:
	(void)printf("\r\n Type name of 3270 key to change: ");	
	errno = 0; i = 0;
	while ((c = get_key()) != '\r') {
		fbuf[i++] = c;
		if (c >= ' ' && c < 0x7f)
			printf("%c",c);
		if (c == BS) {
			if (i > 1) {
				printf("\b \b");
				i -= 2;		/* backup one */
			} else
				i = 0;		/* ignore */
		}
	}
	if (i == 0) {
		printf("\r\n");
		print_codes();
		printf("\r\n Are you done with 3270 scan code changes (y/n) <n>: ");
		c = get_yn();
		if (c == 'y' || c == 'Y')
			return;
		goto next_key;
	}
	fbuf[i] = 0;
	/*
	** add i = 1 logic to edit Key_table
	*/
	if (i == 1) {
		if (fbuf[0] == 0 || !isprint(fbuf[0])) {
			printf("\r\n 3270 key name error %s\r\n",fbuf);
			print_codes();
			goto next_key;
		}
		printf("\r\n\t The 3174/3274 '%c' key code is ", fbuf[0]);
		x_key_ptr = (u_char *)Key_xlat;
		x_key_ptr += (fbuf[0] - ' ') * sizeof(Key_xlat[0]);
		goto show_code;
	}
	for (index = 0; index < N_KL-14; index++) {
		if (!strcmp(fbuf,kl[index].k_name))
			break;
	}
	if (index == N_KL-14) {
		printf("\r\n 3270 key name error %s\r\n",fbuf);
		print_codes();
		goto next_key;
	}
	printf("\r\n\t The 3174/3274 '%s' key code is ", kl[index].k_name);
	x_key_ptr = (u_char *)X_key_codes;
	x_key_ptr += index * sizeof(X_key_codes[0]);

show_code:
	print_3274_key_codes(x_key_ptr);

get_code:
	printf("\r\n Type n(ormal) or s(hift) or a(lt) or k(ana),\n\r followed by new 3270 scan code for %s -> ",kl[index].k_name);
	c = get_key();
	if (c == 'N' || c == 'n') {
		printf("NORMAL x");
		*x_key_ptr++ = NORMAL;
	} else if (c == 'S' || c == 's') {
		printf("SHIFT x");
		*x_key_ptr++ = SHIFT;
	} else if (c == 'A' || c == 'a') {
		printf("ALT x");
		*x_key_ptr++ = ALT;
	} else if (c == 'K' || c == 'k') {
		printf("KANA x");
		*x_key_ptr++ = DONTK;
	} else if (c == '\r') {
		printf(" no change made ");
		goto next_key;
	} else {
		printf(" * not n, s, a, or k ...  try again\n");
		goto get_code;
	}
	errno = 0; i = 0;
	do {
		c = get_key();
		if (c == '\r')
			break;
		fbuf[i++] = c;
		prinit(c);
	} while (i < 2 );
	fbuf[i] = 0;
	if (i == 1) {
		fbuf[1] = fbuf[0];
		fbuf[0] = 0;
	}
	c = fbuf[1] > '9' ? fbuf[1] - 7 : fbuf[1];
	c &= 0x0f;
	fbuf[0] = fbuf[0] > '9' ? fbuf[0] - 7 : fbuf[0];
	fbuf[0] <<= 4;
	fbuf[0] &= 0xf0;
	c |= fbuf[0];
	*x_key_ptr = c;
	goto next_key;
}


/*
**	Get_presentation gets keyboard and display ttytype
*/
get_presentation()
{
	register u_char c, *p;
	int disp_changed = 0;

	update_ttytype();
	conopen(2);
	setbuf (stdout, (char *)0);
	p = (u_char *)X_key_xlat + (L_X_KEY_XLAT-1) * sizeof(X_key_xlat[0]);
	printf("\r\n\n Type the RETRN key for default selection or to end typing");
	printf("\r\n\n\t *** DISPLAY TERMINAL *** \r\n\n\t");
	printf("0 - graphics terminal \r\n\t");
	printf("1 - textport terminal \r\n\t");
	printf("2 - textport terminal with attributes capability \r\n\n");
	printf("Current display terminal is: %d \r\n\n",Display);
	printf("Do you want to change display from %d (y/n) <n>: ",Display);
	c = get_yn();
	if (c == 'y' || c == 'Y') {
		printf("\r\nEnter new display value (0,1,2) <%d> ",Display);
		c = get_key();
		if (c != '\r' && c != '\n') {
			prinit(c);
			if (c >= '0' && c < '3') {
				Display = c - '0';
				printf("\r\n Display changed to %d",Display);
				*p = Display;
				disp_changed++;
			} else
				printf("\r\n No change made");
		}
	}
	printf("\r\n");
	*p++ = Display;
	if (Display || disp_changed) {
		printf("\r\n\t *** KEYBOARD *** \r\n\n\t");
		printf("0 - graphics keyboard \r\n\t");
		printf("1 - stdin keyboard \r\n\t");
		printf("2 - stdin keyboard with keypad \r\n\n");
		printf(" Current keyboard is: %d \r\n\n",Keyboard);
		printf(" Do you want to change keyboard from %d (y/n) <n>: ",Keyboard);
		c = get_yn();
		if (c == 'y' || c == 'Y') {
			printf("\r\n Enter new keyboard value (0,1,2) <%d> ",Keyboard);
			c = get_key();
			if (c != '\r' && c != '\n') {
				prinit(c);
				if (c >= '0' && c < '3') {
					conclose(1);
					Keyboard = c - '0';
					conopen(2); /* was 1, now 2 noport wpc*/
					setbuf (stdout, (char *)0);
					printf("\r\n Keyboard changed to %d",Keyboard);
				} else
					printf("\r\n No change made");
			}
		}
		printf("\r\n");
	}
	*p++ = Keyboard;
	*p = ttytype;
	set_3174();
	if (Keyboard == 0) {
		printf("\r\n\tThe graphic keyboard (gl2/device.h) is selected");
		printf("\r\n\n\n Want to change keyboard button definitions (y/n) <n>: ");
		c = get_yn();
		if (c == 'y' || c == 'Y')
			get_state_changes();
		else
			printf("\r\n");
	}
}


print_3274_key_codes(p)
register u_char *p;
{
	register u_char c;

	c = *p++;
	if (c == NORMAL)
		printf("NORMAL ");
	else if (c == SHIFT)
		printf("SHIFT ");
	else if (c == ALT)
		printf("ALT ");
	else if (c == DONTK)
		printf("KANA ");
	else {
		printf("Current entry BAD, must change\n");
		return;
	}
	c = *p;
	printf("x%02x ",c & 0x7f);
}


/*
**	Print 3270 metakey code mapping
*/
print_codes()
{
	register int index, index1, kline, strokes;
	register u_char c, c1, *k, *p, *p1;

	printf("\r\n");
	kline = strokes = 0;
	for (index = 0; index < L_X_KEY_XLAT - 14; index++) {
		p = (u_char *)X_key_codes +(index * sizeof(X_key_codes[0]));
		printf(" %s ", kl[index].k_name);
		c = *p++;
		if (c == NORMAL)
			printf("N ");
		else if (c == SHIFT)
			printf("S ");
		else if (c == ALT)
			printf("A ");
		else if (c == DONTK)
			printf("K ");
		else {
			printf("Current entry BAD, must change\r\n");
			kline = 0;
		}
		c = *p;
		printf("x%02x ",c & 0x7f);
		p1 = p = (u_char *)X_key_codes + (index * sizeof(X_key_codes[0]));
		for (index1 = 0; index1 < index ; index1++) {
			k = (u_char *)X_key_codes + (index1 * sizeof(X_key_codes[0]));
			p = p1;
			c = *p++;
			c1 = *k++;
			if (c1 == c) {
				c = *p++;
				c1 = *k++;
			}
			if (c1 == c)
				printf("*");
		}
		if (++kline > 4) {
			kline = 0;
			printf("\r\n");
		} else
			printf("  ");
	}
}


/*
**	Print 3270 function keyboard mapping
*/
print_keys()
{
	register int index, index1, kline, strokes;
	register u_char c, c1, *k, *p, *p1;

	if (kb_changed == 0 && Keyboard < 3) {
		print_keymap();
		return;
	}
	kline = strokes = 0;
	for (index = 0; index < L_X_KEY_XLAT - 2; index++) {
		p = (u_char *)X_key_xlat +(index * sizeof(X_key_xlat[0]));
		c = *p++;
		if (c != 0) {
			printf(" %s ", kl[index].k_name);
			prinit(c);
			strokes++;
			do {
				if (!(c = *p++))
					break;
				else {
					printf(",");
					prinit(c);
				}
			} while (++strokes < 4);
			strokes = 0;
			p1 = p = (u_char *)X_key_xlat + (index * sizeof(X_key_xlat[0]));
			for (index1 = 0; index1 < index ; index1++) {
				k = (u_char *)X_key_xlat + (index1 * sizeof(X_key_xlat[0]));
				p = p1;
				c = *p++;
				c1 = *k++;
				if (c1 == c) {
					do {
						c = *p++;
						if (!(c1 = *k++))
							break;
						else if (c1 != c)
							break;
					} while (strokes++ < 4);
					strokes = 0;
				}
				if (c1 == c)
					printf("*");
			}
			if (++kline > 4) {
				kline = 0;
				printf("\r\n");
			} else
				printf("  ");
		}
	}
}


prinit(c)
register u_char c;
{
	if (c == 0x1b)
		printf("ESC");
	else if (c == 0x7f)
		printf("DEL");
	else if (c > (u_char)0x7f)
		printf("x80+%02x",c & 0x7f);
	else if (c >= ' ')
		printf("%c",c);
	else if (c && c < 0x1b)
		printf("CTRL-%c", c + '`');
	else
		printf("CTRL-%c", c + '@');
/*	printf("x%02x",c);*/
}



/*
**	get graphics keyboard state changes
*/
get_state_changes()
{
	register u_char c, *x_key_ptr;
	register int i, j, n;
	register char index;

next_key:
	(void)printf("\r\n Press the key (button) that needs to be changed : ");
	errno = 0; i = 0;
	c = get_key();
	if (button == RETKEY) {
		printf("\r\n Do you want to change RETRN button (y/n) <n>: ");
		c = get_yn();
		if (c == 'y' || c == 'Y')
			goto proc_button;
		printf("\r\n Are you done with button changes (y/n) <n>: ");
		c = get_yn();
		if (c == 'y' || c == 'Y') {
			printf("\r\n");
			return;
		}
		goto next_key;
	} else {
proc_button:
		printf(" button %d (device.h)",button);
	}
	i = button - BUTOFFSET;
	n = 0;
	for (index = 0; index < 5; index++) {
		printf("\r\n %s ", kn[index].k_name);
		prinit(button_xlat[index][i]);
		printf("\r\n Enter the new two digit hex value for this key ");
		printf("\r\n (do not preface with x or 0x): ");
		c = get_key();
		if (button != RETKEY) {
			printf(" x");
			if (button == XKEY)
				c = get_key();
			for (n = 0; button != RETKEY; n++) {
				printf("%c",c);
				fbuf[n] = c;
				c = get_key();
			}
			fbuf[n] = 0;
			if (n == 1) {
				fbuf[1] = fbuf[0];
				fbuf[0] = 0;
			}
			c = fbuf[1] > '9' ? fbuf[1] - 7 : fbuf[1];
			c &= 0x0f;
			fbuf[0] = fbuf[0] > '9' ? fbuf[0] - 7 : fbuf[0];
			fbuf[0] <<= 4;
			fbuf[0] &= 0xf0;
			c |= fbuf[0];
			button_xlat[index][i] = c;
		}
		/*
		** now change 3270 code
		*/
		c = button_xlat[index][i];
		if (c==0||((j = xlat_idx(c, 0)) < 0 && isprint(c))){
			/* NULL or not in X_key_xlat tbl and printable*/
/*			x_key_ptr = (u_char *)Key_xlat;
			x_key_ptr += (c - ' ') * sizeof(X_key_codes[0]);
			printf(" 3270 key code ");
			print_3274_key_codes(x_key_ptr);*/
		} else if (j >= 0 && X_key_xlat[j].k_stroke[1]==0) {
			    /* matched button is a complete seq */
try_key_name:
			printf(" %s -> ",kl[j].k_name);
			n = 0;
			while ((c = get_key()) != '\r') {
				fbuf[n++] = c;
				if (c >= ' ' && c < 0x7f)
					printf("%c",c);
				if (c == BS) {
					if (n > 1) {
						printf("\b \b");
						n -= 2;		/* backup one */
					} else
						n = 0;		/* ignore */
				}
			}
			if (n) {
				fbuf[n] = 0;
				for (n = 0; n < N_KL-2; n++) {
					if (!strcmp(fbuf,kl[n].k_name))
						break;
				}
				if (n == N_KL-2) {
					printf("\r\n 3270 key name error %s\r\n",fbuf);
					print_keys();
					printf("\r\n");
					goto try_key_name;
				} else {
					X_key_xlat[j].k_stroke[0] = 0; /* remove old */
					X_key_xlat[n].k_stroke[0] = button_xlat[index][i];
					X_key_xlat[n].k_stroke[1] = 0; /* install new */
				}
			}
		}
	}
	goto next_key;
}


set_3174()
{
	register u_char *p;
	register long ru_size;

	p = (u_char *)X_key_xlat;
	p += sizeof(X_key_xlat) - 1;
	*p = F3174;
	p -= 5;
	ru_size = MaxRU;
	*p++ = ru_size >> 8;
	*p = ru_size;
	*(p - 3) = Record_sep;
}


update_ttytype()
{
	if ((evaluep = getenv("TERM")) != NULL) {
		if (*evaluep == 'r')
			ttytype = 1;	/* rwsiris same as iris, not wsiris */
					/* and will get to be 2 */
		for (; ttytype < N_TRMS; ++ttytype)
			if (strcmp(evaluep, trms[ttytype].tt_name) == 0)
				break;
		if (ttytype >= N_TRMS)
			ttytype = 2;
#ifdef DEBUG
	DT("ttytype %d ",ttytype);
#endif /* DEBUG */
	}
}
