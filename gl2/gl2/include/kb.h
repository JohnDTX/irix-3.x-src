#ifndef KBDEF
#define KBDEF
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
 **************************************************************************/

/*
 * Keyboard interface structure between user and system:
 *	- user issues a grioctl(GR_[SG]ETKBD, &a_kbd)
 */
struct	kbd {
	short	k_beepmask;		/* mask of beep bits */
	short	k_ledmask;		/* mask of led bits */
};

#define KB_US	0			/* US */
#define KB_GS	1			/* GS (intl) */
#define KB_DBG	2			/* DEBUG */

#define WHO_RU_0	0x00		/* 2-byte request */
#define WHO_RU_1	0x10

/*
 * kdb responses to WHO ARE YOU
 */
#define KB_IRIS_STD	"\252"		/* IRIS 3000 kbd */
#define KB_IRIS_ISO	"\156\001"	/* IRIS GS kbd */
#define KB_4D_STD	"\156\377"	/* not defined yet */

/* some special keys (keycodes from the keyboard) */
#define	KEY_CTRL	0x02
#define	KEY_RIGHT_SHIFT	0x04
#define	KEY_LEFT_SHIFT	0x05
#define	KEY_CAPSLOCK	0x03
#define	KEY_SETUP	0x01
/* the following only apply to the international keyboard. */
#define KEY_NUMLOCK	0x6a
#define KEY_RIGHT_CTRL	0x55
#define KEY_RIGHT_ALT	0x54
#define KEY_LEFT_ALT	0x53

/* constants for the iris keyboard */
#define	KBD_BEEPCMD	0x00		/* low bit clear --> beep commands */
#define	KBD_LEDCMD	0x01		/* low bit set --> led commands */
#define	KBD_RESET	0x10		/* reset the keyboard */
#define	KBD_REPLY	0xAA		/* reply from keyboard after reset */

/* beep commands */
#define	KBD_CLICK	0x08		/* key click: active low */
#define	KBD_SHORTBEEP	0x02		/* do a short beep */
#define	KBD_LONGBEEP	0x04		/* do a long beep */

/* led commands */
#define	KBD_LED0	0x00
#define	KBD_LED1	0x02		/* led0 and led1 are inverses */
#define	KBD_LED2	0x04
#define	KBD_LED3	0x08
#define	KBD_LED4	0x10
#define	KBD_LED5	0x20
#define	KBD_LED6	0x40
#define	KBD_LED8	0x80

/* defaults */
#define	KBD_BEEPDEFAULT	(KBD_BEEPCMD|KBD_CLICK|KBD_SHORTBEEP)
#define	KBD_LEDDEFAULT	(KBD_LEDCMD|KBD_LED0)

#ifdef	KERNEL
/* soft keyboard states */
#define STATE_LEFTSHIFT		1
#define STATE_RIGHTSHIFT	2
#define STATE_CTRL		4
#define STATE_CAPSLOCK		8
/* the following only apply to the international keyboard. */
#define STATE_RIGHTCTRL		16
#define STATE_RIGHTALT		32
#define STATE_LEFTALT		64
#define STATE_NUMLOCK		128
#endif

/* convert scancode from keyboard into keycode and up/down value */
#define KB_SCANCTOKEYC(c)   ((c) & 0x7f)
#define KB_SCANCTOUPDN(c)   (!((c) & 0x80))

/* map keycode to GL device */
#define KB_DEVOFF1	    1 
#define KB_DEVOFF2	    60
#define KB_KEYCTODEV(c)	    ((((c) + KB_DEVOFF1) <= MAXKBDBUT) \
				    ? ((c) + KB_DEVOFF1) \
				    : ((c) + KB_DEVOFF2))

/* map device to keycode */
#define KB_DEVTOKEYC(d)	    ((d) - ((ISXKEYBD(d))?KB_DEVOFF2:KB_DEVOFF1))

#endif KBDEF
