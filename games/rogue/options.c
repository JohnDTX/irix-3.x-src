static	char	*Options_c	= "@(#)options.c	1.9";
/*
 * This file has all the code for the option command.
 * I would rather this command were not necessary, but
 * it is the only way to keep the wolves off of my back.
 *
 * @(#)options.c	3.3 (Berkeley) 5/25/81
 */

#include <ctype.h>
#include "rogue.h"

#define	NUM_OPTS	(sizeof optlist / sizeof (OPTION))

/*
 * description of an option and what to do with it
 */
struct	optstruct {
    char	*o_name;	/* option name */
    char	*o_prompt;	/* prompt for interactive entry */
    int			*o_opt;		/* pointer to thing to set */
    int			(*o_putfunc)();	/* function to print value */
    int			(*o_getfunc)();	/* function to get value interactively */
};

typedef struct	optstruct	OPTION;

int	put_bool(), get_bool(), put_str(), get_str();

OPTION	optlist[] = {
    {"terse",	 "Terse output: ",
		 (int	*) &terse,	put_bool,	get_bool	},
    {"flush",	 "Flush typeahead during battle: ",
		 (int	*) &fight_flush,	put_bool,	get_bool	},
    {"jump",	 "Show position only at end of run: ",
		 (int	*) &jump,		put_bool,	get_bool	},
    {"step",	"Do inventories one line at a time: ",
		(int	*) &slow_invent,	put_bool,	get_bool	},
    {"askme",	"Ask me about unidentified things: ",
		(int	*) &askme,		put_bool,	get_bool	},
    {"name",	 "Name: ",
		 (int	*) whoami,	put_str,	get_str		},
    {"fruit",	 "Fruit: ",
		 (int	*) fruit,		put_str,	get_str		},
    {"file",	"Save file: ",
		(int	*) file_name,	put_str,	get_str		}
};

/*
 * print and then set options from the terminal
 */
option()
{
    reg1 rega1	OPTION	*op;
    reg2 regd1	int	retval;

    wclear(hw);
    touchwin(hw);
    /*
     * Display current values of options
     */
#ifdef	iris
    wmove(hw, 1, 0);
#endif
    for (op = optlist; op < &optlist[NUM_OPTS]; op++) {
	waddstr(hw, op->o_prompt);
	(*op->o_putfunc)(op->o_opt);
	waddch(hw, '\n');
    }
    /*
     * Set values
     */
#ifdef	iris
    wmove(hw, 1, 0);
#else
    wmove(hw, 0, 0);
#endif
    for (op = optlist; op < &optlist[NUM_OPTS]; op++) {
	waddstr(hw, op->o_prompt);
	if ((retval = (*op->o_getfunc)(op->o_opt, hw)))
	    if (retval == QUIT)
		break;
	    else if (op > optlist) {	/* MINUS */
#ifdef	iris
		wmove(hw, (op - optlist) , 0);
#else
		wmove(hw, (op - optlist) - 1, 0);
#endif
		op -= 2;
	    } else {	/* trying to back up beyond the top */
		putchar('\007');
#ifdef	iris
		wmove(hw, 1, 0);
#else
		wmove(hw, 0, 0);
#endif
		op--;
	    }
    }
    /*
     * Switch back to original screen
     */
#ifdef	iris
    mvwaddstr(hw, 0, 0, "Press space to continue");
#else
    mvwaddstr(hw, LINES-1, 0, "--Press space to continue--");
    Draw(hw);
#endif
    wait_prompt(hw, ' ');
    clearok(cw, TRUE);
    touchwin(cw);
    after = FALSE;
}

/*
 * put out a boolean
 */
put_bool(b)
bool	*b;
{
    waddstr(hw, *b ? "True" : "False");
}

/*
 * put out a string
 */
put_str(str)
char	*str;
{
    waddstr(hw, str);
}

/*
 * allow changing a boolean option and print it out
 */

get_bool(bp, win)
bool	*bp;
WINDOW	*win;
{
    reg1 regd1	int	oy;
    reg2 regd2	int	ox;
    reg3 regd3	bool	op_bad;

    op_bad = TRUE;
    getyx(win, oy, ox);
    waddstr(win, *bp ? "True" : "False");
    while (op_bad) {
	wmove(win, oy, ox);
#ifdef	iris
	prompt(win);
#else
	Draw(win);
#endif
	switch (readchar()) {
	    case 't':
	    case 'T':
		*bp = TRUE;
		op_bad = FALSE;
		break;
	    case 'f':
	    case 'F':
		*bp = FALSE;
		op_bad = FALSE;
		break;
	    case '\n':
	    case '\r':
		op_bad = FALSE;
		break;
	    case '\033':
	    case '\007':
		return QUIT;
	    case '-':
		return MINUS;
	    default:
		mvwaddstr(win, oy, ox + 10, "(T or F)");
	}
    }
    wmove(win, oy, ox);
    waddstr(win, *bp ? "True" : "False");
    waddch(win, '\n');
    return NORM;
}

/*
 * set a string option
 */
get_str(opt, win)
reg1 rega1	char	*opt;
WINDOW	*win;
{
    reg2 rega2	char	*sp;
    reg3 regd1	char	c;
    reg4 regd2	int	oy;
    reg5 regd3	int	ox;
		char	buf[80];

    getyx(win, oy, ox);
#ifdef	iris
    prompt(win);
#else
    Draw(win);
#endif

    /*
     * loop reading in the string, and put it in a temporary buffer
     */
    sp = buf;
    while ((c = readchar()) != '\n' && c != '\r' && c != '\033'
      && c != '\007') {
	if (c == -1)
	    continue;
#ifdef V7
	if (c == _tty.sg_erase)		/* process erase character */
#else
	if (c == _tty.c_cc[VERASE])	/* process erase character */
#endif
	{
	    if (sp > buf) {
		sp--;
		waddch(win, '\b');
		if (!isprint(*sp))
		    waddch(win, '\b');
	    }
	}
#ifdef V7
	else if (c == _tty.sg_kill)	/* process kill character */
#else
	else if (c == _tty.c_cc[VKILL])	/* process kill character */
#endif
	{
	    sp = buf;
	    wmove(win, oy, ox);
	} else if (sp == buf) {
	    if (c == '-')
		break;
	    else if (c == '~') {
		strcpy(buf, home);
		waddstr(win, home);
		sp += strlen(home);
	    } else {
		*sp++ = c;
		waddstr(win, unctrl(c));
	    }
	} else {
	    *sp++ = c;
	    waddstr(win, unctrl(c));
	}
	wclrtoeol(win);
#ifdef	iris
	prompt(win);
#else
	Draw(win);
#endif
    }
    *sp = '\0';
    if (sp > buf)	/* only change option if something has been typed */
	strucpy(opt, buf, strlen(buf));
    wmove(win, oy, ox);
    waddstr(win, opt);
    waddch(win, '\n');
    Draw(win);
    if (win == cw)
	mpos += sp - buf;
    if (c == '-')
	return MINUS;
    else if (c == '\033' || c == '\007')
	return QUIT;
    else
	return NORM;
}

/*
 * parse options from string, usually taken from the environment.
 * the string is a series of comma seperated values, with booleans
 * being stated as "name" (true) or "noname" (false), and strings
 * being "name=....", with the string being defined up to a comma
 * or the end of the entire option string.
 */

parse_opts(str)
reg1 rega1	char	*str;
{
    reg2 rega2	char	*sp;
    reg3 rega3	OPTION *op;
    reg4 regd1	int	len;

    while (*str) {
	/*
	 * Get option name
	 */
	for (sp = str; isalpha(*sp); sp++)
	    continue;
	len = sp - str;
	/*
	 * Look it up and deal with it
	 */
	for (op = optlist; op < &optlist[NUM_OPTS]; op++)
	    if (EQSTR(str, op->o_name, len)) {
		if (op->o_putfunc == put_bool)	/* if option is a boolean */
		    *(bool	*)op->o_opt = TRUE;
		else {				/* string option */
		    /*
		     * Skip to start of string value
		     */
		    for (str = sp + 1; !isalpha(*str); str++)
			continue;
		    /*
		     * Skip to end of string value
		     */
		    for (sp = str + 1; *sp && *sp != ','; sp++)
			continue;
		    strucpy(op->o_opt, str, sp - str);
		}
		break;
	    }
	    /*
	     * check for "noname" for booleans
	     */
	    else if (op->o_putfunc == put_bool
	      && EQSTR(str, "no", 2) && EQSTR(str + 2, op->o_name, len - 2)) {
		*(bool	*)op->o_opt = FALSE;
		break;
	    }

	/*
	 * skip to start of next option name
	 */
	while (*sp && !isalpha(*sp))
	    sp++;
	str = sp;
    }
}

/*
 * copy string using unctrl for things
 */
strucpy(s1, s2, len)
reg1 rega1	char	*s1;
reg2 rega2	char	*s2;
reg3 regd1	int	len;
{
    reg4 rega3	char	*sp;

    while (len--) {
	strcpy(s1, (sp = unctrl(*s2++)));
	s1 += strlen(sp);
    }
    *s1 = '\0';
}

prompt(win)
WINDOW	*win;
{
#ifdef	iris
    int		iy, ix;
    PIXEL oldcolor;
    char	c2;

    if (_iris) {
        getyx(win, iy, ix);
	oldcolor = win->_attrib;
	c2 = winch(win);
	if (c2 == ' ') {
	    acolor(win, C_red, Dbackground);
	    waddch(win, cursor);
	} else {
	    acolor(win, Dbackground, Dforeground);
	    waddch(win, c2);
	}
	win->_attrib = oldcolor;
	wmove(win, iy, ix);
	oldcolor = win->_attrib;
    }
#endif
    Draw(win);
}
