/*
 * Graphical shell - quick & dirty prototype for a user code version
 * of the textport code
 *
 * Hacked by: Kipp Hickman
 */

#include "stdio.h"
#include "signal.h"
#include "sys/types.h"
#include "window.h"
#include "gl.h"
#include "device.h"
#include "gsh.h"
#include "string.h"

#undef	DEBUG

extern	char *getenv();

char	flag_shell;		/* shell to use instead of default */
int	flag_hold;		/* hold output after shell exits */
char	*flag_font;		/* use a particular font */
char	**shell_argv;		/* arguments for user shell */
char	**global_argv;		/* global version of argv */
int	flag_rows, flag_cols;	/* arguments selection rows && cols */
char	flag_script;		/* script input/output to a file */
char	flag_debug;		/* enable debugging */
char	flag_blink;		/* blink cursor N times per second */
int	force_title;
char	*title_value;
int	force_colors;		/* force colors to values on command line */
int	c1, c2, c3;
#ifdef	SHRINK
char	flag_startsmall;	/* non-zero if we should come up as icon */
char	flag_icon;		/* use alternative icon */
char	flag_movie;		/* do movie please */
#endif

short	font_width, font_height, font_descender;
char	*fontlib;

int	script_fd;
int	mywinid;

struct	textframe	*tframe;
struct	textview	*tview;
char	*termcap;

char	opened = 1;
char	*icon_name = MYICON;

/* return 1 if character "c" is an ascii digit */
#define	isadigit(c)	(((c) >= '0') && ((c) <= '9'))

/* ARGSUSED */
main(argc, argv)
	int argc;
	char *argv[];
{
	int o, errflg;
	extern	char *optarg;
	extern	int optind;

	/*
	 * Save this in case user does a clone
	 */
	global_argv = argv;

	/*
	 * Run at better than average priority to make the tool have
	 * better interactive response
	 */
#ifdef	LOGIN
	nice(-20);
	nice(-20);
	nice(10);
#endif

	/*
	 * Process arguments.
	 *
	 * ***NOTE***
	 * ***NOTE***
	 * ***NOTE***
	 *	If you add any more arguments, fix the clone() procedure to
	 *	know about them.
	 * ***NOTE***
	 * ***NOTE***
	 * ***NOTE***
	 */
	errflg = 0;
#ifdef	DEBUG
	printargs(argv);
#endif
#ifdef	SHRINK
#define	OPTS "i:mIb:c:C:f:hds:t:S"
#else
#define	OPTS "b:c:C:f:hds:t:S"
#endif
	while ((o = getopt(argc, argv, OPTS)) != EOF) {
		switch (o) {
		  case 'b':				/* blink cursor */
			flag_blink = atoi(optarg);
			if ((flag_blink < 0) || (flag_blink > VRETRACE))
				flag_blink = 2;
			break;
		  case 'c':				/* force command */
			flag_shell = 1;
			shell_argv = &argv[optind - 1];
			goto done;
			break;
		  case 'd':
			flag_debug = 1;
			break;
		  case 'f':				/* force font */
			flag_font = optarg;
			break;
		  case 'h':				/* hold output */
			flag_hold = 1;
			break;
#ifdef	SHRINK
		  case 'i':				/* set icon */
			flag_icon = 1;
			icon_name = optarg;
			break;
		  case 'm':				/* use icon movie */
			flag_movie = 1;
			break;
#endif
		  case 's':				/* size */
			flag_rows = atoi(optarg);
			if ((flag_rows <= 0) || (flag_rows > MAXROWS))
				errflg++;
			else
			if ((optind < argc) && isadigit(argv[optind][0])) {
				flag_cols = atoi(argv[optind]);
				optind++;
				if ((flag_cols <= 0) || (flag_cols > MAXCOLS))
					errflg++;
			} else
				errflg++;
			break;
		  case 't':
			force_title = 1;
			title_value = optarg;
			break;
#ifdef	SHRINK
		  case 'I':
			opened = 0;
			flag_startsmall = 1;
			break;
#endif
		  case 'C':
			force_colors = 1;
			if (optind > argc - 2)
				errflg++;
			else {
				c1 = atoi(optarg);
				c2 = atoi(argv[optind++]);
				c3 = atoi(argv[optind++]);
			}
			break;
		  case 'S':
			flag_script = 1;
			break;
		  case '?':
			errflg++;
		}
	}

done:
	if (errflg) {
printf("usage:\n");
printf("gsh [-b rate] [-t title] [-f font] [-h] [-s rows cols]\n");
#ifdef	SHRINK
printf("    [-i icon] [-I] [-m]\n");
#endif
printf("    [-C textcolor pagecolor reversecolor] [-c cmds ...]\n");
		exit(-1);
	}

	initialize(argv[0]);
	shelltool();
}

/*
 * Setup window constraints
 */
void
reset_constraints()
{
	stepunit(font_width, font_height);
	fudge(XSIZE(0), YSIZE(0));
	minsize(XSIZE(3), YSIZE(3));
	maxsize(XSIZE(MAXCOLS), YSIZE(MAXROWS));
}

/*
 * Initialize the software structures
 */
initialize(what)
	char *what;
{
	extern char *shell_name;
	int gid;
	register char **argv;
	extern int byebye();

	/*
	 * Note font library
	 */
	if ((fontlib = getenv("FONTLIB")) == NULL)
		fontlib = FONTLIB;

	/*
	 * Setup shell stuff
	 */
	pickshell();

	/*
	 * Setup title
	 */
	if (flag_shell) {
		title[0] = 0;
		argv = shell_argv;
		while (*argv) {
			strcat(title, *argv++);
			/*
			 * If there is another argument following this one,
			 * concatenate a space onto the end of the title so
			 * that a space will seperate each argument
			 */
			if (*argv)
				strcat(title, " ");
		}
	} else
		strcpy(title, shell_name);
	if (force_title)
		strcpy(title, title_value);

	/*
	 * Open script file
	 */
	if (flag_script) {
		script_fd = creat("typescript", 0666);
		if (script_fd < 0) {
			flag_script = 0;
			fprintf(stderr, "gsh: can't open typescript\n");
		}
	}
	if (flag_debug)
		foreground();


	/*
	 * Setup font info.  First, get the graphics going without actually
	 * using up any screen space.
	 */
	noport();
	gid = winopen("bogus-gsh");
	if (flag_font || (flag_font = getenv("FONT"))) {
		if (setup_font() == 0) {
			fprintf(stderr, "gsh: can't load font\n");
			exit(-1);
		}
	} else {
		/*
		 * Get info from os about the default font
		 */
		font(0);
#ifndef	R2300
		gl_getcharinfo(&font_width, &font_height, &font_descender);
#else
		font_width = 8;
		font_height = 13;
		font_descender = 2;
#endif
	}

	/* do misc non-graphical setup */
	makeshell();				/* start shell */
#ifdef	SHRINK
	iconInit();				/* read in icon */
	if (noicon)
		flag_startsmall = 0;		/* oh well */
#endif
	tx_open(0, flag_blink);			/* open textport */
	kb_init();				/* setup keyboard */
	initmenus();				/* setup menus */

	/*
	 * Now that we know how big the font is, setup the window
	 * size for a max-ed out textport
	 */
	reset_constraints();
#ifdef	SHRINK
	if (flag_startsmall) {
	    if (flag_rows)
		setRegularSize(XSIZE(flag_cols), YSIZE(flag_rows));
	    else
		setRegularSize(XSIZE(DEFAULT_COLS), YSIZE(DEFAULT_ROWS));
	    setIconConstraints();
	} else
#endif
	{
	    if (flag_rows)
		prefsize(XSIZE(flag_cols), YSIZE(flag_rows));
	    else
		prefsize(XSIZE(DEFAULT_COLS), YSIZE(DEFAULT_ROWS));
	}
	mywinid = winopen(what);

	winconstraints();
	reset_constraints();
	winconstraints();

	winclose(gid);
#ifdef	SHRINK
	if (!flag_startsmall)
#endif
	{
		wintitle(title);
		winattach();
	}
#ifdef	SHRINK
	else {
		shrink();
		flag_startsmall = 0;
	}
#endif

	(void) signal(SIGTERM, byebye);
}

#ifdef	DEBUG
printargs(argv)
	register char **argv;
{
	while (*argv)
		printf("%s ", *argv++);
	printf("\n");
}
#endif
