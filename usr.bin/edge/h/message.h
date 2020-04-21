/*
 * Standard message types
 *
 * $Source: /d2/3.7/src/usr.bin/edge/h/RCS/message.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:00 $
 */

/* bleh */
#define	UNIXSIG		0x00004000	/* base for unix signal messages */

#define	KBMSG		0x00010000	/* from keyboard process */
#define	KBBINDKEY	0x00010001	/* bind a key */
#define	KBGETBINDING	0x00010002	/* get key binding */
#define	DEADPROC	0x00020000	/* from a dead process */
#define	SHELL_REDRAW	0x00030000	/* redraw shell */
#define	SCROLL		0x00040000	/* scroll message from scroll bar */
