/*
 * Global data
 *
 * $Source: /d2/3.7/src/usr.bin/edge/gsh/RCS/gsh.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:46:32 $
 */

/*
 * Flags and values setup by argument processing
 */
extern	char	flag_shell;		/* use a specific shell, not default */
extern	int	flag_hold;		/* hold output after shell exits */
extern	char	*flag_font;		/* font to use */
extern	char	flag_position;		/* set position */
extern	int	flag_rows, flag_cols;	/* arguments selection rows && cols */
extern	char	flag_script;		/* script input/output to a file */
extern	char	flag_debug;		/* enable debugging */
extern	char	flag_blink;		/* blink cursor */
extern	int	force_title;		/* force title from cmd line */
extern	char	*title_value;		/* title forced */
extern	int	force_colors;		/* force colors from command line */
extern	int	c1, c2, c3;		/* colors forced */
extern	char	flag_startsmall;	/* we should start as an icon */
extern	char	flag_icon;		/* use alternative icon */
extern	char	flag_movie;		/* show icon movie */
extern	char	*icon_name;		/* alternative icon to use */

extern	int	pty_num;		/* pty number we are using */
extern	char	**shell_argv;		/* arguments for user shell */
extern	char	**global_argv;		/* global version of argv */

extern	short	font_width, font_height, font_descender;
extern	char	title[];
extern	int	child_pid;		/* child process id */
extern	char	*fontlib;		/* path to font library dir */
extern	char	opened;			/* window is fully visible */
extern	char	noicon;			/* non-zero if no icon file */

/* location of the default font library directory */
#define	FONTLIB		"/usr/lib/gl2/fonts"
#define	ICONLIB		"/usr/lib/gl2/icons"
#define	MYICON		"gsh.icon"

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
 * Borders.
 */
#define	TFXWIDTH	1
#define	TFYWIDTH	1
#define	FRAMEXWIDTH	0
#define	FRAMEYWIDTH	0
#define	LINXWIDTH	0

/*
 * These macros compute the number of pixels to fit a given number of
 * characters on the screen.  They account for the scroll bar width
 * and height and the borders being used.
 */
#define	XSIZE(cols) \
	(((cols) * font_width) + \
	 (FRAMEXWIDTH*2 + 	/* pixels for left&right frame border */ \
	    LINXWIDTH +		/* pixels for line in the middle */ \
	    TFXWIDTH*2))	/* pixels for left&right text frame border */

#define	YSIZE(rows) \
	(((rows) * font_height) + \
	 (FRAMEYWIDTH*2) +	/* pixels for top&bottom frame border */ \
	 (TFYWIDTH*2))		/* pixels for top&bottom text frame border */


#define	DEFAULT_ROWS	40
#define	DEFAULT_COLS	80

/*
 * What we really want to do here is to allocate 4 color indexes that we
 * can have our way with.
 */
#define	C_BACKGROUND	0
#define	C_FOREGROUND	7
#define	C_REVERSE	3

#define	C_CURSOR	(32+pty_num)	/* first cursor to use */

/* number of retrace intervals per second */
#define	VRETRACE	66

extern	short	softqtest(), softqread();

/* gsh procedures */
extern	void	xcharstr();
extern	void	shelltool();
extern	void	initmenus();
extern	void	kb_init();
extern	void	makeshell();
extern	void	winlogin();
extern	void	domenu();
extern	void	pickshell();
extern	void	clone();
extern	void	winlogout();
extern	void	setRegularSize();
extern	void	setIconConstraints();
extern	void	initIcon();

/* import's from libc */
extern	char			*malloc();
extern	char			*calloc();
extern	void			free();
extern	void			perror();
extern	long			time();
extern	void			qsort();
