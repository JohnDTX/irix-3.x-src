/*
 * Global data
 *
 * $Source: /d2/3.7/src/usr.bin/edge/common/RCS/gsh.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:45:53 $
 */

/*
 * Flags and values setup by argument processing
 */
extern	char	flag_shell;		/* use a specific shell, not default */
extern	int	flag_hold;		/* hold output after shell exits */
extern	char	*flag_font;		/* font to use */
extern	char	flag_position;		/* set position */
extern	char	flag_signal;		/* send SIGWINCH on redraw */
extern	int	flag_rows, flag_cols;	/* arguments selection rows && cols */
extern	char	flag_script;		/* script input/output to a file */
extern	char	flag_debug;		/* enable debugging */
extern	char	flag_blink;		/* blink cursor */

#ifdef notdef
extern	int	pty_num;		/* pty number we are using */
#endif
extern	char	**shell_argv;		/* arguments for user shell */
extern	char	**global_argv;		/* global version of argv */

extern	short	charwidth, charheight, chardescender;
extern	char	title[];
extern	int	child_pid;		/* child process id */
extern	char	*fontlib;		/* path to font library dir */

/* location of the default font library directory */
#define	FONTLIB		"/usr/lib/gl2/fonts"

/* gunk */
#ifndef	MAX
#define	MAX(a, b)	(((a) > (b)) ? (a) : (b))
#endif
#ifndef	MIN
#define	MIN(a, b)	(((a) < (b)) ? (a) : (b))
#endif
#ifndef	NULL
#define NULL	0
#endif

/*
 * Scroll bar stuff.
 */
#define	SCROLLBAR_WIDTH		13

/*
 * Borders.
 */
#define	TFXWIDTH	1
#define	TFYWIDTH	1
#define	FRAMEXWIDTH	0
#define	FRAMEYWIDTH	0
#define	LINXWIDTH	1

/*
 * These macros compute the number of pixels to fit a given number of
 * characters on the screen.  They account for the scroll bar width
 * and height and the borders being used.
 */
#define	XSIZE(cols) \
	(((cols) * charwidth) + \
	 (FRAMEXWIDTH*2 + 	/* pixels for left&right frame border */ \
	    SCROLLBAR_WIDTH +	/* pixels for scroll bar */ \
	    LINXWIDTH +		/* pixels for line in the middle */ \
	    TFXWIDTH*2))	/* pixels for left&right text frame border */

#define	YSIZE(rows) \
	(((rows) * charheight) + \
	 (FRAMEYWIDTH*2) +	/* pixels for top&bottom frame border */ \
	 (TFYWIDTH*2))		/* pixels for top&bottom text frame border */

/*
 * What we really want to do here is to allocate 4 color indexes that we
 * can have our way with.
 */
#define	C_BACKGROUND	0
#define	C_FOREGROUND	7
#define	C_REVERSE	3

#define	C_CURSOR	(32)	/* first cursor to use */

/* number of retrace intervals per second */
#define	VRETRACE	66

extern	short	softqtest(), softqread();
