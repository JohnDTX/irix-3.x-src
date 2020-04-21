static	char	*Save_c	= "@(#)save.c	1.13";
/*
 * save and restore routines
 *
 * @(#)save.c	3.9 (Berkeley) 6/16/81
 * @(#)save.c	1.13 4/3/85
 */

#ifndef	STAND
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "rogue.h"

#ifndef	SMALL
typedef struct	stat STAT;

extern char	*sys_errlist[], version[], encstr[];
extern bool	_endwin;
extern int	errno;

char	*sbrk();

bool	fromfile = FALSE;

STAT sbuf;
#endif	SMALL

save_game(how)
{
#ifdef	SMALL
	msg("Sorry, can't save (no memory) - keep on sluggin'");
	return FALSE;
#else	SMALL
    reg1 rega1	char	*sp;
    reg2 rega2	FILE	*savef;
    reg3 regd1	int	c;
    reg4 rega3	char	*col;
    reg5 regd3	int	line;
		int	euid;
		int	egid;
		char	buf[80];
		char	trans[128];

#ifdef	iris
    if (CT == 2) {
	msg(
	"Sorry, can't save on IRIS due to ginit nonrestartability (yet)\n");
	return FALSE;
    }
#endif
/*
 * 'till debugged
 *
 *  if (how)
 *	signal(how,SIG_IGN);
 */
    msg("");
    mpos = 0;
    msg("");
#ifdef	iris
    _linfont[0] = 0;	/* normal font for top line */
#endif	iris
    for (;;) {
	msg("Saving game in %s",file_name);
/*
 *	if (!how)
 *	    *sp = '\0';
 */
	if (!fork()) {	/* cope with directory being mode 755 or 700	*/
	    if (how)
		unlink(file_name);
	    euid = geteuid();
	    egid = getegid();
	    setuid(getuid());
	    setgid(getgid());
	    if (how)
		unlink(file_name);
#ifndef	V7
	    umask(0);
	    close(creat(file_name,0644));
	    chown(file_name,euid,egid);
#endif
#ifdef	V7
	    close(creat(file_name,0666));
#endif
	    exit(0);
	}
	wait((int *)0);
	if ((savef = fopen(file_name, "w")) != NULL)
	    break;
	msg("%s: %s",file_name,sys_errlist[errno]);	/* fake perror() */
	if (how) {
	    exit(2);
	}
	msg("File name: ");
	mpos = 0;
	buf[0] = '\0';
	if (get_str(buf, cw) == QUIT) {
	    msg("");
#ifdef	iris
	    _linfont[0] = 1;	/* Old English font for top line */
#endif	iris
	    return FALSE;
	}
	strcpy(file_name, buf);
    }
#ifdef	iris
    _linfont[0] = 1;	/* Old English font for top line */
    if (_iris) {
	doublebuffer();
	color(BLACK);
	glclear();
	swapbuffers();
	glclear();
	gexit();
	unlink(MUTEX);
	_iris = 0;
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\
\n\n\n\n\n\n\n\n\n\n");
    }
#endif

    /*
     * Convert any line drawing chars back to standard rogue chars.
     */
			/* create a translation array */
    for (line=0; line<128; line++)
	trans[line] = line;
    trans[wallv] = '|';
    trans[wallh] = '-';
    trans[wallur] = Wallur;
    trans[walllr] = Walllr;
    trans[wallul] = Wallul;
    trans[wallll] = Wallll;
    trans[door] = Door;
    trans[floor] = Floor;
    trans[passage] = Passage;
    trans[cursor] = Cursor;
			/* translate */
    for (line=0; line<LINES; line++)
	for (col=cw->_y[line]; col<cw->_y[line]+COLS; col++)
	    *col = trans[*col];

    /*
     * write out encrpyted file (after a stat)
     * The fwrite is to force allocation of the buffer before the write
     */
    fstat(fileno(savef), &sbuf);
    fwrite("junk", 1, 5, savef);
    fseek(savef, 0L, 0);
    _endwin = TRUE;
    encwrite(version, sbrk(0) - version, savef);
    fclose(savef);
    wmove(cw, LINES-1, 0); 
    wclrtoeol(cw);
    Draw(cw);
    endwin();
    sync();
    sync();
    exit(0);
    /* NOT REACHED */
#endif	SMALL
}

#ifndef	SMALL
restore(file, envp)
reg1 rega1	char	*file;
		char	**envp;
{
    reg2 regd1	int	inf;
    reg3 rega2	char	*col;
    reg4 regd2	int	line;
		char	trans[128];
    extern	char	**environ;
		char	buf[80];
		STAT	sbuf2;
		int	oldCOLS;	/* restore on next level */
		int	oldLINES;

    if (!strcmp(file,"-r"))
	file = file_name;
    if ((inf = open(file, 0)) < 0) {
	perror(file);
	return FALSE;
    }

    fflush(stdout);
    encread(buf, strlen(version) + 1, inf);
    if (strcmp(buf, version) != 0) {
	printf("Sorry, saved game is out of date.\n");
	return FALSE;
    }

    fstat(inf, &sbuf2);
    fflush(stdout);
    brk(version + sbuf2.st_size);
    lseek(inf, 0L, 0);
    encread(version, (unsigned) sbuf2.st_size, inf);

    if (!wizard)
	if (resttst(sbuf2.st_ino != sbuf.st_ino || sbuf2.st_dev != sbuf.st_dev,
	  "Sorry, saved game is not in the same file.\n") ||
	  resttst(sbuf2.st_ctime - sbuf.st_ctime > 10,
	  "Sorry, file has been touched.\n"))
	    return FALSE;
    mpos = 0;
    mvwprintw(cw, 0, 0, "%s: %s", file, ctime(&sbuf2.st_mtime));

    /*
     * defeat multiple restarting from the same place
     */
    if (!wizard)
	if (sbuf2.st_nlink != 1) {
	    printf("Cannot restore from a linked file\n");
	    return FALSE;
	} else
	    if (creat(file,0) < 0) {
		printf("Cannot creat %s\n",file);
		return FALSE;
	    }
	    if (!fork()) {
		unlink(file);
		setuid(getuid());
		setgid(getgid());
		unlink(file);
		exit(0);
	    }
	    wait((int *)0);
    close(inf);		/* finally close it */

    environ = envp;
    fromfile = TRUE;
    oldCOLS = COLS;
    oldLINES = LINES;
    if (!My_term && isatty(2))
    {
	char	*sp;

	_tty_ch = 2;
	gettmode();
	if ((sp = getenv("TERM")) == NULL)
	    sp = Def_term;
	setterm(sp);
    } else
	setterm(Def_term);
    init_vwalls();
    init_termcap();
    init_graph();

    /*
     * Convert standard rogue chars back to any line drawing chars.
     */
			/* create a translation array */
    for (line=0; line<128; line++)
	trans[line] = line;
    trans['|'] = wallv;
    trans['-'] = wallh;
    trans[Wallur] = wallur;
    trans[Walllr] = walllr;
    trans[Wallul] = wallul;
    trans[Wallll] = wallll;
    trans[Door] = door;
    trans[Floor] = floor;
    trans[Passage] = passage;
    trans[Cursor] = cursor;
			/* translate */
    for (line=0; line<LINES; line++)
	for (col=cw->_y[line]; col<cw->_y[line]+COLS; col++)
	    *col = trans[*col];

    if (COLS < oldCOLS || LINES < oldLINES) {
	printf("Sorry, can't restart because your screen is too small\n");
	COLS = oldCOLS;
	LINES = oldLINES;
	save(0);
	exit(3);
    }
#ifdef	COLOR
#ifdef	iris
    if (CT == 2) {
	printf("Sorry, can't restart on IRIS due to ginit nonrestartability (yet)\n");
	COLS = oldCOLS;
	LINES = oldLINES;
	save(0);
	exit(3);
    }
#endif	iris
#endif	COLOR
    strcpy(file_name, file);
    setup();
    clearok(curscr, TRUE);
    touchwin(cw);
    srand(getpid());
    playit();
    /*NOTREACHED*/
}

resttst(ok,msg)
char	*msg;
{
    if (ok == 0)	/* if ok then return 0 */
	return 0;
			/*
			 * else print error message and allow
			 * override password
			 */
    printf("%s",msg);
    printf("If you can tell me the wizard's password you can play anyway\n");
    fflush(stdout);
    if (strcmp(PASSWD, crypt(getpass("Wizard's password: "), "mT")) == 0) {
	printf("OK.\n");
        fflush(stdout);
	return 0;
    } else {
	printf("Sorry!\n");
        fflush(stdout);
	return 1;
    }
}

/*
 * perform an encrypted write
 */
encwrite(start, size, outf)
reg1 rega1	char	*start;
		UNSIGN	size;
reg2 rega2	FILE	*outf;
{
    reg3 rega3	char	*ep;

    ep = encstr;

    while (size--) {
	putc(*start++ ^ *ep++, outf);
	if (*ep == '\0')
	    ep = encstr;
    }
}

/*
 * perform an encrypted read
 */
encread(start, size, inf)
reg1 rega1	char	*start;
		UNSIGN	size;
reg2 regd1	int	inf;
{
    reg3 rega2	char	*ep;
			    /*
			     * %% may need to be unsigned or (char	*)
			     * on 16-bit machines such as pdp-11 & z8000
			     */
    reg4 regd2	int	read_size;

    if ((read_size = read(inf, start, size)) == -1 || read_size == 0)
	return read_size;

    ep = encstr;

    while (size--) {
	*start++ ^= *ep++;
	if (*ep == '\0')
	    ep = encstr;
    }
    return read_size;
}
#endif	SMALL
#endif	STAND
