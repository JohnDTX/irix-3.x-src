static	char	*Main_c	= "@(#)main.c	1.18";
/*
 * Rogue
 * Exploring the dungeons of doom
 * Copyright (C) 1980 by Michael Toy and Glenn Wichman
 * All rights reserved
 *
 * Color graphics and other changes:
 * Copyright (C) 1983, 1984 by Bob Toxen
 * All rights reserved
 *
 * @(#)main.c	3.27 (Berkeley) 6/15/81
 * @(#)main.c	1.18 4/3/85
 */

#ifndef	STAND
#include <signal.h>
#include <pwd.h>
#include <errno.h>
#include "mach_dep.h"
#else	STAND
#define	EINTR	4
#endif

#include "rogue.h"

char	*himem;

#ifdef	CHECKTIME
static	int	num_checks;
#endif

int	myerrno;

FILE	*ddt = NULL;

main(argc,argv,envp)
int	argc;
char	**argv;
char	**envp;
{
    reg1 rega1	char	*env;
    reg2 rega2	struct	linked_list *item;
    reg3 rega3	struct	object	*obj;
#ifndef	STAND
    reg4 rega4	struct	passwd *pw;
#endif
#ifdef MAXLOAD
#define	MAX
		double	avec[3];
#endif
#ifdef MAXUSERS
#define	MAX
    reg5 regd1	int	cnt;
#endif
#ifndef	STAND
		struct	passwd	*getpwuid();
		char	*getpass(), *crypt();
#endif
		int	quit(), lowtime;
		long	now;
#ifdef WIZDB
		int	wizquit();
#endif

    himem = &argv[argc-1][0];
    if (envp && envp > (char	**) himem) {
	char	**p;

	p = envp;
	while (*envp)
	    himem = *envp++;
    }
    /*
     * check for print-score option
     */
    if (argc == 2 && strcmp(argv[1], "-s") == 0) {
	initscr();			/* Start up cursor package	  */
	waswizard = TRUE;
#ifndef	SMALL
	score(0, -1, 'Z');
#endif	SMALL
	exit(0);
    }
    /*
     * Check to see if he is a wizard
     */
#ifndef	SMALL
#ifndef	STAND
    if ((env=getenv("UCBHASH")) != NULL && strcmp(env, "/etc/pwtable") == 0)
	printf("Cheaters never prosper...\n");
    if (argc >= 2 && (argv[1][0] == '\0' ||
      argv[1][0] == '"' && argv[1][1] == '"' && !argv[1][2])) {
	if (strcmp(PASSWD, crypt(getpass("Wizard's password: "), "mT")) == 0)
	    wizard = TRUE;
        argv++;
        argc--;
    }
    setbuf(stdout, outbuf);
#endif
#endif	SMALL

    /*
     * get options from environment
     */
#ifdef	STAND
    env = "ROGUEOPTS=fruit Orange, jump, askme, name Bobby";
    parse_opts(env);
#else
    home[0] = '\0';
    if ((env = getenv("ROGUEOPTS")) != NULL)
	parse_opts(env);
    if ((env = getenv("HOME")) != NULL)
	strcpy(home, env);
    if (env == NULL || whoami[0] == '\0')
	if ((pw = getpwuid(getuid())) == NULL) {
	    printf("Say, who the hell are you?\n");
	    exit(1);
	} else {
	    strucpy(whoami, pw->pw_name, strlen(pw->pw_name));
	    if (!home[0])
		strcpy(home,pw->pw_dir);
	}
#endif
    if (env == NULL || fruit[0] == '\0')
	strcpy(fruit, "slime-mold");

#if MAXLOAD|MAXUSERS
    if (too_much() && !wizard && !author()) {
	printf("Sorry, %s, but the system is too loaded now.\n", whoami);
	printf("Try again later.  Meanwhile, why not enjoy a%s %s?\n",
	    vowelstr(fruit), fruit);
	exit(1);
    }
#endif
#ifndef	STAND
    strcpy(file_name, home);
    if (home[0])
	strcat(file_name,"/");
    strcat(file_name, "rogue.save");
#ifndef	SMALL
    if (argc == 2)
	if (!restore(argv[1], envp))	/* Note: restore will never return */
	    exit(1);
#endif	SMALL
    initscr();				/* Start up cursor package	  */
    time(&now);
    lowtime = (int) now;
    dnum = (wizard && getenv("SEED") != NULL ?
	atoi(getenv("SEED")) :
	lowtime + getpid());
#else
    dnum = 13;
    printf("Using dungeon #13\n");
#endif

    if (wizard)
	printf("Hello %s, welcome to dungeon #%d\n", whoami, dnum);
    else
	printf("Hello %s, just a moment while I dig the dungeon...\n", whoami);
    fflush(stdout);
    seed = dnum;

    setup();
    init_player();			/* Roll up the rogue		  */
    init_things();			/* Set up probabilities of things */
    init_names();			/* Set up names of scrolls	  */
    init_colors();			/* Set up colors of potions	  */
    init_rings();			/* Set up names of rings	  */
    init_materials();			/* Set up materials of wands	  */
    init_vwalls();			/* Set up walls, handling graphics*/
    init_termcap();			/* Set up termcap attributes	  */
    init_graph();			/* Set up graphics: fonts, etc	  */
    cw = newwin(LINES, COLS, 0, 0);	/* Make new windows */
    mw = newwin(LINES, COLS, 0, 0);
    hw = newwin(LINES, COLS, 0, 0);
    msgw = cw;				/* so we can use msg() for other wins */
    oldmsgw = cw;			/* so we can use msg() for other wins */
    scrollok(cw, FALSE);		/* Set no scrolling */
    scrollok(mw, FALSE);
    scrollok(stdscr, FALSE);
    scrollok(hw, FALSE);
    leaveok(mw, TRUE);			/* don't actually move cursor */
    waswizard = wizard;
#ifdef	iris
    if (_iris) {
	usemouse();
    }
#endif
    lastscore = -1;
    /*
     * Start up daemons and fuses
     */
    daemon(doctor, 0, AFTER);
    fuse(swander, 0, WANDERTIME, AFTER);
    daemon(stomach, 0, AFTER);
    daemon(runners, 0, AFTER);
    /*
     * Give the rogue his weaponry.  First a mace.
     */
    item = new_item(sizeof *obj);
    obj = (struct object *) ldata(item);
    obj->o_type = WEAPON;
    obj->o_which = MACE;
    init_weapon(obj, MACE);
    obj->o_hplus = 1;
    obj->o_dplus = 1;
    obj->o_flags |= ISKNOW;
    add_pack(item, TRUE);
    cur_weapon = obj;
    /*
     * Now a +1 bow
     */
    item = new_item(sizeof *obj);
    obj = (struct object *) ldata(item);
    obj->o_type = WEAPON;
    obj->o_which = BOW;
    init_weapon(obj, BOW);
    obj->o_hplus = 1;
    obj->o_dplus = 0;
    obj->o_flags |= ISKNOW;
    add_pack(item, TRUE);
    /*
     * Now some arrows
     */
    item = new_item(sizeof *obj);
    obj = (struct object *) ldata(item);
    obj->o_type = WEAPON;
    obj->o_which = ARROW;
    init_weapon(obj, ARROW);
    obj->o_count = 25+rnd(15);
    obj->o_hplus = obj->o_dplus = 0;
    obj->o_flags |= ISKNOW;
    add_pack(item, TRUE);
    /*
     * And his suit of armor
     */
    item = new_item(sizeof *obj);
    obj = (struct object *) ldata(item);
    obj->o_type = ARMOR;
    obj->o_which = RING_MAIL;
    obj->o_ac = a_class[RING_MAIL] - 1;
    obj->o_flags |= ISKNOW;
    cur_armor = obj;
    add_pack(item, TRUE);
    /*
     * Give him some food too
     */
    item = new_item(sizeof *obj);
    obj = (struct object *) ldata(item);
    obj->o_type = FOOD;
    obj->o_count = 1;
    obj->o_which = 0;
    add_pack(item, TRUE);
    hunglev = H_NONE;	/* not currently hungry */
    new_level();			/* Draw current level */
    playit();
}

/*
 * endit:
 *	Exit the program abnormally.
 */

endit()
{
    fatal("Ok, if you want to exit that badly, I'll have to allow it\n");
}

/*
 * fatal:
 *	Exit the program, printing a message.
 */

fatal(s)
char	*s;
{
    Clear();
    Move(LINES-1, 0);
    printw("%s", s);
    Draw(stdscr);
    endwin();
    resetty();			/* savetty() done in initscr()	*/
    exit(0);
}

/*
 * rnd:
 *	Pick a very random number.
 */

rnd(range)
reg1 regd1	int	range;
{
/*  return range == 0 ? 0 : abs(RN + RN + RN + RN + RN) % range; 2.6 */
/*  return range == 0 ? 0 : abs(RN) / ((1<<14)/range); Divides by zero */
    return range == 0 ? 0 : abs(RN + RN + RN + RN + RN) % range;
}

/*
 * roll:
 *	roll a number of dice
 */

roll(number, sides)
reg1 regd1	int	number;
reg2 regd2	int	sides;
{
    reg3 regd3	int	dtotal = 0;

    while (number--)
	dtotal += rnd(sides)+1;
    return dtotal;
}

# ifdef SIGTSTP
/*
 * handle stop and start signals
 */
tstp()
{
    mvcur(0, COLS - 1, LINES - 1, 0);
    endwin();
    fflush(stdout);
    kill(0, SIGTSTP);
    signal(SIGTSTP, tstp);
    savetty();		/* shouldn't be needed */
    crmode();
    noecho();
    clearok(curscr, TRUE);
    touchwin(cw);
    Draw(cw);
    flushi();		/* flush input */
}
# endif

#ifndef	SMALL
ddtcolor()
{
	int	i;

	ddt = fopen("/dev/ttyd1","w");
	if (ddt == NULL)
		printf("ddtcolor():can't open /dev/ttyd1\n"), exit(1);
	fprintf(ddt,"ddtcolor(): starting up\n");
	Draw(cw);

#ifdef	COLOR
	fprintf(ddt,"attrib is 0%o\n",(unsigned int)cw->_attrib);
#endif	COLOR
	fprintf(ddt,"begy=%d cury=%d maxy=%d\n",
	  (int)cw->_begy, (int)cw->_cury, (int)cw->_maxy);
	fprintf(ddt,"begx=%d curx=%d maxx=%d\n",
	  (int)cw->_begx, (int)cw->_curx, (int)cw->_maxx);

	fprintf(ddt,"First line is:        '%40s'\n", &cw->_y[0][0]);
	fprintf(ddt,"First line colors are:'");
#ifdef	COLOR
	for (i=0; i<40; i++)
		fprintf(ddt,"%c",
		  "BrgybmcW"[dfcolor(cw->_Y[0][i])]);
#endif	COLOR
	fprintf(ddt,"'\n");

	fprintf(ddt,"ddtcolor(): exiting\n");
	fflush(ddt);
	exit(7);
}
#endif	SMALL

setup()
{
#ifndef	STAND
#ifdef CHECKTIME
    int		 checkout();
#endif
    int		save_game();
    int		async();

    signal(SIGALRM, async);
#ifdef WIZDB
    if (wizard)
	signal(SIGINT, wizquit);
    else
#endif
	signal(SIGINT, quit);
#ifdef SIGTSTP
    signal(SIGTSTP, tstp);
#endif
    signal(SIGHUP,  save_game);
#ifndef	CORE
    signal(SIGQUIT, save_game);
    signal(SIGTERM, save_game);
#endif
#ifndef V7
    signal(SIGUSR1, save_game);
    signal(SIGUSR1, ddtcolor);
    signal(SIGUSR2, save_game);
    signal(SIGPWR,  save_game);
#endif
    crmode();				/* Cbreak mode */
    noecho();				/* Echo off */
#endif

/*
 * #ifndef DUMP
 *     signal(SIGHUP, auto_save);
 *     signal(SIGILL, auto_save);
 *     signal(SIGTRAP, auto_save);
 *     signal(SIGIOT, auto_save);
 *     signal(SIGEMT, auto_save);
 *     signal(SIGFPE, auto_save);
 *     signal(SIGBUS, auto_save);
 *     signal(SIGSEGV, auto_save);
 *     signal(SIGSYS, auto_save);
 *     signal(SIGPIPE, auto_save);
 *     signal(SIGTERM, auto_save);
 * #endif
 */

#ifdef CHECKTIME
    if (!author()) {
	signal(SIGALRM, checkout);
	alarm(CHECKTIME * 60);
	num_checks = 0;
    }
#endif
}

/*
 * async:
 *	catch misc asynchronous signals.
 * Catch Alarm Clock signals each time the minute changes to call status() to
 * update the clock on status line.
 */
async(sig)
{
	signal(sig, async);	/* to catch it again */
	myerrno = EINTR;	/* so that readchar() will catch */
}

/*
 * playit:
 *	The main loop of the program.  Loop until the game is over,
 * refreshing things and looking at the proper times.
 */

playit()
{
    reg1 rega1	char	*opts;
    reg2 regd1	bool	wasrun;

    /*
     * set up defaults for slow terminals
     */

#ifndef	STAND
# ifdef V7
    if (_tty.sg_ospeed < B1200)
# else
    if (_tty.c_cflag < B1200)
# endif
    {
	terse = TRUE;
	jump = TRUE;
    }
#endif

    /*
     * parse environment declaration of options
     */
#ifdef	STAND
    opts = "ROGUEOPTS=fruit Orange, jump, askme, name Bobby";
    parse_opts(opts);
#else
    if ((opts = getenv("ROGUEOPTS")) != NULL)
	parse_opts(opts);
#endif

    oldpos = hero;
    oldrp = roomin(&hero);
    while (playing) {
/* 2.6
 *	wasrun = running;
 *	look();
 *	if (!running && wasrun) {
 *	    door_stop = FALSE;
 *	    wasrun = FALSE;
 *	}
 *	status();
 *	lastscore = purse;
 *					/ * Maybe set Hero's color here? * /
 *	wmove(cw, hero.y, hero.x);
 *	if (!((running || count) && jump))
 *	    Draw(cw);			/ * Draw screen * /
 */
	command();			/* Command execution */
/* 2.6
 *	if (!running && wasrun)
 *	    door_stop = FALSE;
 */
    }
    endit();
}

#ifdef	MAX
/*
 * see if a user is an author of the program
 */
author()
{
    switch (getuid()) {
	case 27876:
	case 222:
	case 11:
	case 16:
	case 0:
	    return TRUE;
	default:
	    return FALSE;
    }
}
#endif

#ifndef	SMALL
char	*
crypt(fromcrt,key)	/* My Crpyt, ha ha */
char	*fromcrt;
char	*key;
{
	if (!strcmp(fromcrt,"wldf1re") || !strcmp(fromcrt,"wyldfir"))
		return PASSWD;
	return "oops";
}
#endif	SMALL

#ifdef CHECKTIME
checkout()
{
    static	char	*msgs[] = {
	"The load is too high to be playing.  Please leave in %d minutes",
	"Please save your game.  You have %d minutes",
	"Last warning.  You have %d minutes to leave",
    };
    int		checktime;

    signal(SIGALRM, checkout);
    if (too_much()) {
	if (num_checks == 3)
	    fatal("Sorry.  You took to long.  You are dead\n");
	checktime = CHECKTIME / (num_checks + 1);
	chmsg(msgs[num_checks++], checktime);
	alarm(checktime * 60);
    } else {
	if (num_checks) {
	    chmsg("The load has dropped back down.  You have a reprieve.");
	    num_checks = 0;
	}
	alarm(CHECKTIME * 60);
    }
}

/*
 * checkout()'s version of msg.  If we are in the middle of a shell, do a
 * printf instead of a msg to avoid the refresh.
 */
chmsg(fmt, arg)
char	*fmt;
int	arg;
{
    if (in_shell) {
	printf(fmt, arg);
	putchar('\n');
	fflush(stdout);
    } else
	msg(fmt, arg);
}
#endif

#ifdef LOADAV

#include <nlist.h>

struct	nlist avenrun =
{
    "_avenrun"
};

loadav(avg)
reg1 rega1	double *avg;
{
    reg2 regd1	int	kmem;

    if ((kmem = open("/dev/kmem", 0)) < 0)
	goto bad;
    nlist(NAMELIST, &avenrun);
    if (avenrun.n_type == 0) {
bad:
	avg[0] = avg[1] = avg[2] = 0.0;
	return;
    }

    lseek(kmem, (long) avenrun.n_value, 0);
    read(kmem, avg, 3 * sizeof (double));
}
#endif

#ifdef UCOUNT

#ifndef	V7
#include <sys/types.h>
#endif
#include <utmp.h>

ucount()
{
    reg1 rega1 struct utmp *ut;
    reg2 regd1 int count = 0;

#ifdef UTMP
    utmpname(UTMP);	/* not needed */
#endif
    setutent();
    while ((ut = getutent()) != NULL) {
	if (ut->ut_type == USER_PROCESS)
	    count++;
    }
    endutent();
    return count;
}
#endif


#if MAXLOAD|MAXUSERS

/*
 * see if the system is being used too much for this game
 */

too_much()
{
#ifdef MAXLOAD
    double avec[3];
#else
    reg1 regd1	int	cnt;
#endif

#ifdef MAXLOAD
    loadav(avec);
    return (avec[2] > (MAXLOAD / 10.0));
#else
    return (ucount() > MAXUSERS);
#endif
}

#endif
