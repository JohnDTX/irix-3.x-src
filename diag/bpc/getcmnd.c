
/*
 *	Kurt Akeley
 *	4 August 1984
 *
 *	Interactive routine to accept commands from a keyboard.
 *
 *	Procedures:
 *		getcmnd (cmnd, cmndlist, helplist)
 *		copycmnd (cmnd1, cmnd2)
 *		printcmnd (cmnd, cmndlist)
 *		newline ()
 *
 *	Functions:
 *		int negetch ()
 *		int kgtline (line, maxline)
 *
 *	Static functions:
 *		Cmnddef *getcmnddef (cmndstr, cmndlist)
 *		Cmnddef *getcdef (number, cmndlist)
 *		boolean isokprefix (cmndstr, cmndlist)
 *		boolean append (s, c)
 *		boolean delete (s)
 *		boolean ishexdigit (c)
 *		boolean erasearg ()
 *		int     hexvalue (s)
 *		boolean extend (cmndlist)
 *
 *	Static procedures:
 *		ringbell ()
 *		clearextra ()
 *		redraw (cdef, cmnd)
 *		help (cdef, helplist)
 */

#include "getcmnd.h"

#define BELL		007
#define BACKSPACE	010
#define TAB		011
#define LINEFEED	012
#define FORMFEED	014
#define RETURN		015
#define NAK		025
#define CONTROL_W	027
#define ESCAPE		033
#define SPACE		040
#define DELETE		0177

#define MAXSTRLEN	20

#define FALSE		0
#define TRUE		1

static Cmnddef *getcmnddef ();
static Cmnddef *getcdef ();

static Cmnddef *cdef;			/* valid after cmndstr accepted     */
static char cmndstr[MAXSTRLEN];		/* command string		    */
static char argstr[MAXARG][MAXSTRLEN];	/* argument strings		    */
static int argno;			/* number of argument being read    */
static boolean white;			/* true while white space is read   */
static char message[100];		/* text of printed message	    */
static int mesarg;			/* argument to print message about  */
static int extra;			/* length of printed message        */

getcmnd (cmnd, cmndlist, helplist)
Command *cmnd;
Cmnddef *cmndlist;
Help *helplist;
{
    /*
     *	Accepts a command from the keyboard and returns its parameters in
     *	  the passed structure.
     */
    char c;
    int i, j;

    extra = 0;
    cmndstr[0] = '\0';
    for (i=0; i<MAXARG; i++)
	argstr[i][0] = '\0';
    white = FALSE;
    argno = -1;		/* the command is the -1st argument */
    cdef = 0;
    while (1) {
	c = negetch ();
	if (c != SPACE && c != '\n') {
	    clearextra ();
	    }
	switch (c) {
	    case SPACE:
	    case '\n':
		if (cmndstr[0] == '\0') {
		    /* auto repeat */
		    cdef = getcdef (cmnd->number, cmndlist);
		    redraw (cdef, cmnd);
		    newline ();
		    return;
		    }
		if (!white && argno == -1) {
		    /* extend command string if possible, else break */
		    if (!extend (cmndlist))
			break;
		    }
		if (c == '\n' && (argno+1) >= cdef->minargs) {
		    /* if done, copy arguments to cmnd and return */
		    clearextra ();
		    if (strcmp (cdef->name, "$") == 0) {
			/* replace a single argument */
			i = hexvalue (argstr[0]) - 1;
			j = hexvalue (argstr[1]);
			cdef = getcdef (cmnd->number, cmndlist);
			while (erasearg ());
			if (i >= 0 && i < cdef->maxargs) {
			    cmnd->arg[i] = j;
			    redraw (cdef, cmnd);
			    }
			else {
			    ringbell ();
			    printf ("arg index out of range [1,%d]",
				cdef->maxargs);
			    }
			}
		    else {
			cmnd->number = cdef->number;
			cmnd->args = argno+1;
			if (!white)
			    putchar (SPACE);
			for (i=0; i<=argno; i++)
			    cmnd->arg[i] = hexvalue (argstr[i]);
			for (   ; i<cdef->maxprint; i++) {
			    cmnd->arg[i] = cdef->arg[i].value;
			    printf ("%x ", cmnd->arg[i]);
			    }
			for (   ; i<cdef->maxargs; i++) {
			    cmnd->arg[i] = cdef->arg[i].value;
			    }
		 	for (   ; i<MAXARG; i++)
			    cmnd->arg[i] = 0;
			}
		    newline ();
		    return;
		    }
		if (!white)
		    putchar (SPACE);
		if (white || c == '\n') {
		    if (mesarg < cdef->maxargs) {
			sprintf (message, " <%s>", cdef->arg[mesarg].name);
			extra += strlen (message);
			printf ("%s", message);
			mesarg += 1;
			}
		    }
		white = TRUE;
		break;
	    case DELETE:	/* delete 1 character */
	    case BACKSPACE:
		if (white) {
		    printf ("\b \b");
		    white = FALSE;
		    }
		else if (argno == -1) {
		    if (delete (cmndstr))
			printf ("\b \b");
		    }
		else { /* argno >= 0 */
		    delete (argstr[argno]);
		    printf ("\b \b");
		    if (argstr[argno][0] == '\0') {
			argno -= 1;
			white = TRUE;
			}
		    }
		break;
	    case CONTROL_W:	/* delete the current argument */
		erasearg ();
		break;
	    case NAK:		/* erase line */
		while (erasearg ());
		break;
	    case '?':		/* print help message about current command */
		if (argno == -1 && !white) {
		    if (append (cmndstr, c)) {
			if (isokprefix (cmndstr, cmndlist)) {
			    putchar (c);
			    break;
			    }
			else
			    delete (cmndstr);
			}
		    if (extend (cmndlist)) {
			putchar (SPACE);
			white = 1;
			}
		    else
			break;
		    }
		if (!help (cdef, helplist))
		    break;
		partialdraw ();
		break;
	    case FORMFEED:	/* redraw line */
		newline ();
		partialdraw ();
		break;
	    case TAB:		/* default argument */
		if (argno == -1 && !white && !extend (cmndlist))
		    break;
		if ((argno+1) < cdef->maxargs) {
		    char s[20], *sp;
		    argno += 1;
		    if (!white)
			putchar (SPACE);
		    white = TRUE;
		    sprintf (s, "%x", cdef->arg[argno].value);
		    for (sp=s; *sp; sp++) {
			append (argstr[argno], *sp);
			putchar (*sp);
			}
		    putchar (SPACE);
		    }
		break;
	    default:
		if (white) {
		    argno += 1;
		    if (argno >= cdef->maxargs) {
			argno -=1;
			ringbell ();
			break;
			}
		    else
			white = FALSE;
		    }
		if (argno == -1) {
		    if (!append (cmndstr, c))
			break;
		    if (isokprefix (cmndstr, cmndlist)) {
			putchar (c);
			}
		    else {
			delete (cmndstr);
			ringbell ();
			}
		    }
		else if ((ishexdigit (c)) ||
			 (c == '-' && argstr[argno][0] == '\0')) {
		    if (!append (argstr[argno], c))
			break;
		    putchar (c);
		    }
		else {
		    ringbell ();
		    }
		break;
	    }
	if (c != SPACE && c != '\n')
	    mesarg = argno+1;
	}
    }
		    
copycmnd (cmnd1, cmnd2)
Command *cmnd1, *cmnd2;
{
    int i;
    cmnd2->number = cmnd1->number;
    cmnd2->args = cmnd1->args;
    for (i=0; i<MAXARG; i++)
	cmnd2->arg[i] = cmnd1->arg[i];
    }

printcmnd (cmnd, cmndlist)
Command *cmnd;
Cmnddef *cmndlist;
{
    redraw (getcdef (cmnd->number, cmndlist), cmnd);
    }

negetch () {
#ifdef PM1
    int c;
    return ((c = getchar ()) == RETURN) ? '\n' : c;
#endif PM1
#ifdef PM2
    return negetchar ();
#endif PM2
#ifdef PM3
    int c;
    return ((c = negetchar ()) == RETURN) ? '\n' : c;
#endif PM3
    }

newline () {
#ifdef PM1
    putchar (RETURN);
#endif PM1
#ifdef PM2
    putchar ('\n');
#endif PM2
#ifdef PM3
    putchar ('\r');
    putchar ('\n');
#endif PM3
    }

static boolean isokprefix (cmndstr, cmndlist)
char *cmndstr;
Cmnddef *cmndlist;
{
    char *s, *cs;
    int i;
    for (i=0; *(s=cmndlist[i].name); i++) {
	cs = cmndstr;
	while (1) {
	    if (*cs == '\0')
		return TRUE;
	    else if (*s++ != *cs++)
		break;
	    }
	}
    return FALSE;
    }

static Cmnddef *getcmnddef (cmndstr, cmndlist)
char *cmndstr;
Cmnddef *cmndlist;
{
    char *s, *cs;
    int i;
    for (i=0; *(cmndlist[i].name); i++) {
	s = cmndlist[i].prefix;
	cs = cmndstr;
	while (1) {
	    if (*s == '\0')
		return &cmndlist[i];
	    else if (*s++ != *cs++)
		break;
	    }
	}
    return 0;
    }

static Cmnddef *getcdef (number, cmndlist)
int number;
Cmnddef *cmndlist;
{
    int i;
    for (i=0; *(cmndlist[i].name); i++) {
	if (number == cmndlist[i].number)
	    return &cmndlist[i];
	}
    return 0;
    }

static boolean append (s, c)
char *s, c;
{
    int i;
    for (i=0; i<(MAXSTRLEN-2); i++) {
	if (*s++ == '\0') {
	    *s-- = '\0';
	    *s = c;
	    return TRUE;
	    }
	}
    return FALSE;
    }

static boolean delete (s)
char *s;
{
    if (*s == '\0')
	return FALSE;
    while (*s)
	s++;
    *(--s) = '\0';
    return TRUE;
    }
    
static boolean ishexdigit (c)
char c;
{
    if ((('0' <= c) && (c <= '9')) ||
        (('a' <= c) && (c <= 'f')) ||
        (('A' <= c) && (c <= 'F')))
	return TRUE;
    return FALSE;
    }

static int hexvalue (s)
char *s;
{
    char s2[MAXSTRLEN+2];
    int i;
    sprintf (s2, "%s ", s);
    sscanf (s2, "%x", &i);
    return i;
    }

static ringbell () {
    putchar (BELL);
    }

static clearextra () {
    int i;
    for (i=0; i<extra; i++)
	printf ("\b \b");
    extra = 0;
    }

static redraw (cdef, cmnd)
Cmnddef *cdef;
Command *cmnd;
{
    int i, max;
    if (cdef != 0) {
	max = cdef->maxprint;
	if (max < cmnd->args)
	    max = cmnd->args;
	printf ("%s", cdef->name);
	for (i=0; i<max; i++) {
	    printf (" %x", cmnd->arg[i]);
	    }
	}
    }

static partialdraw () {
    short i;
    printf ("-> %s", cmndstr);
    if (-1 < argno || white)
	putchar (SPACE);
    for (i=0; i<=argno; i++) {
	printf ("%s", argstr[i]);
	if (i < argno || white)
	    putchar (SPACE);
	}
    }

static boolean erasearg () {
    if (cmndstr[0] == '\0')
	return FALSE;
    if (white)
	printf ("\b \b");
    if (argno == -1) {
	while (delete (cmndstr))
	    printf ("\b \b");
        white = FALSE;
	}
    else {
	while (delete (argstr[argno]))
	    printf ("\b \b");
	argno -= 1;
	white = TRUE;
	}
    return TRUE;
    }

static boolean help (cdef, helplist)
Cmnddef *cdef;
Help *helplist;
{
    /* Searches the helplist for an entry that corresponds to number.
     *   If one is found, it is printed paragraph by paragraph, with a
     *	 prompt at every blank line.
     */
    short i;
    newline ();
    printf ("  %s ", cdef->name);
    for (i=0; i<cdef->maxargs; i++)
	printf ("<%s> ", cdef->arg[i].name);
    newline ();
    for (;helplist->number >= 0; helplist++) {
	if (helplist->number == cdef->number) {
	    char *s;
	    boolean first;
	    for (s=helplist->string,first=TRUE; *s; s++) {
		if (*s == '\n') {
		    newline ();
		    first = TRUE;
		    if (*(s+1) == '\n') {
			short c, i;
			while (*(s+1) == '\n')
			    s += 1;
			printf ("<q >");
			c = negetch ();
			for (i=0; i<4; i++)
			    printf ("\b \b");
			if (c == 'q')
			    break;
			else
			    newline ();
			}
		    }
		else {
		    if (first) {
			first = FALSE;
			printf ("    ");
			}
		    putchar (*s);
		    }
		}
	    return TRUE;
	    }
	}
    return TRUE;
    }

static boolean extend (cmndlist)
Cmnddef *cmndlist;
{
    cdef = getcmnddef (cmndstr, cmndlist);
    if (cdef == 0) {
	ringbell ();
	return FALSE;
	}
    while (delete (cmndstr))
	printf ("\b \b");
    strcpy (cmndstr, cdef->name);
    printf ("%s", cmndstr);
    return TRUE;
    }



kgetline (line, maxline)
char *line;
short maxline;
{
    char c, *s;
    short count;
    short cont;
    /*
     *	Gets a line from stdin.  Maxline is max number of characters to be
     *	  put in line.  Null termination is extra, allow for it in string
     *	  declaration.  Newline character is included and must be allowed
     *	  for in maxline.
     */

    for (s=line, count=0, cont=1; cont;) {
	c = negetch ();
	switch (c) {
	    case 0012:	/* linefeed */
	    case 0015:	/* carriage return */
		c = '\n';
		if (count < maxline) {
		    count += 1;
		    *s++ = '\n';
		    }
#ifdef PM3
		putchar ('\r');
#endif PM3
		cont = 0;
		break;
	    case 0010:	/* backspace */
	    case 0177:	/* delete */
		if (count > 0) {
		    count -= 1;
		    s -= 1;
		    if (c == 0177)
			putchar ('\010');
		    putchar (SPACE);
		    putchar ('\010');
		    }
		break;
	    default:
		if (count < maxline) {
		    count += 1;
		    *s++ = c;
		    }
		break;
	    }
	putchar (c);
	}
    *s = '\0';
    }

