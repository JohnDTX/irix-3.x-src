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

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <gl.h>
#include <fperr.h>

#include "term.h"
#include "hostio.h"

#define CONFIGFILE	    ".wsirisrc"
#define READ		    0
#define WRITE		    1
#define isconsoleshell()    (getppid() == 1)

static char    usage[] = 
"wsiris [options] [hostname]\n\
Options:\n\
-d n: debugging option n (see manual page)\n\
-e c: use `c' for the escape character (default is `~')\n\
-f: don't read ~/.wsirisrc\n\
-h: half-duplex serial communications\n\
-i: try TCP/IP connection first if Ethernet communications (default is XNS)\n\
-l line: use `line' for serial communications (default is /dev/ttyd2)\n\
-p: print textport output even when textport is off\n\
-s speed: use `speed' baud for serial communications\n\
-x: enable local XON/XOFF\n\
-y: process XON/XOFF from host (serial communcations only)\n\
-z n: special instruction n (see manual page)\
";

static int parent = 1;
static char buf[BUFSIZ];
static char hostname[MAXHOSTLEN + 1];
static int  nargc;
static char *nargv[30];		/* surely no more than 30 args */
static int  usenetinput = 0;

int	    cleanup();
extern int  setpipeready();
extern int  exit();
extern char *concat();
extern char *getenv();
extern void perror();
extern char *getirisaddr();

main(argc, argv)
char  **argv;
{
    register int    i;
    register char  *cp;
    char *home;
    char *configfile;
    int  rcfd;
    int diffq = 0;
    int again;
    char *bufp = buf;

    init();
    
    /*
     * Process flags -  first check if -f is there
     */
    for (i = 1; i < argc; i++) {
	if (strcmp(argv[i], "-f") == 0) {
	    fflag++;
	    break;
	}
    }

    /*
     * Check if .wsirisrc should be read
     */
    if (!fflag && (home = concat(getenv("HOME"), "/"))
	    && (configfile = concat(home, CONFIGFILE))
	    && (rcfd = open(configfile, O_RDONLY)) >= 0) {
	/*
	 * Use .wsirisrc.  Place before command line arguments in argv
	 */

	for (i = 0; i < sizeof buf; i++)
	    buf[i] = '\0';
	i = read(rcfd, buf, sizeof buf);
	if (i < 0)
	    errorm('F',"error reading .wsirisrc");
	if (i >= sizeof buf)
	    errorm('f',".wsirisrc file must have less than %d chars",
		    sizeof buf);
	close(rcfd);
	free(home);
	free(configfile);
	nargc = 0;
	nargv[nargc++] = argv[0];
	cp = strtok(buf, " \t\n");
	while (cp) {
	    nargv[nargc++] = cp;
	    cp = strtok(NULLP(char), " \t\n");
	}
	while (--argc > 0)
	    nargv[nargc++] = *++argv;
	nargv[nargc] = NULL;
	argv = nargv;
	argc = nargc;
    }
    i = processflags(argc, argv);
    argv += (argc - i);
    argc = i;

    /*
     * Get hostname, if necessary, and make connection.  If TCP/IP only
     * and no /etc/hosts and running on 2300 or 3010, prompt for IRIS 
     * and host IP addresses and initialize TCP/IP.
     */

#ifdef TCP
    if (zflag[3] && iflag && access(HOSTS,04) < 0) {
	char cmd[100];
	extern long inet_addr();
	
	if (!isconsoleshell())	/* give up if not console of 2300 or 3010 */
	    errorm('f',"no %s",HOSTS);
	diffq = 1;
	fprintf(stdout, "\n\r");
	again = 1;
	while (again) {
	    fprintf(stdout,"Enter IRIS IP address: ");
	    fgets(buf, sizeof buf, stdin);
	    if (!noesc && buf[0] == escchar && buf[1] == '!') {
		shell();
		buf[0] = '\n';
		buf[1] = '\0';
	    }
	    else {
		buf[ strlen(buf)-1 ] = '\0';	/* get rid of \n */
		again = !goodipaddr(buf) || inet_addr(&bufp) == -1;
		if (again) {
		    errorm('w',"TCP/IP: invalid IP address\n\r");
		}
	    }
	}
	fprintf(stdout,"Initializing TCP/IP\n\r");
	sprintf(cmd,"%s %s %s","/bin/sh",TCPINIT,buf);
	if (system(cmd))
	    errorm('f', "TCP/IP initialization failed");
	else
	    sleep(6);
    }
#endif

    if (argc == 0) {
askhost:
	do {
	    fprintf(stdout, "\n\r");
	    do {
		if (diffq) 
		    fprintf(stdout, "Enter host IP address: ");
		else
		    fprintf(stdout, "Connect to what host? ");
		fgets(buf, sizeof buf, stdin);
		if (!noesc && buf[0] == escchar && buf[1] == '!') {
		    shell();
		    buf[0] = '\n';
		    buf[1] = '\0';
		}
		else {
		    buf[ strlen(buf)-1 ] = '\0';	/* get rid of \n */
		    if (diffq && (!goodipaddr(buf) || inet_addr(&bufp) == -1)) {
		        errorm('w',"TCP/IP: invalid IP address\n\r");
		        buf[0] = '\n';
		        buf[1] = '\0';
		    }
		}
	    }
	    while (!(cp = strtok(buf, " \n")));
	    strncpy(hostname, cp, MAXHOSTLEN);
	    hostname[MAXHOSTLEN] = '\0';
	}
	while ((host = hostconnect(hostname)) < 0);
    }
    else if (argc == 1) {
	strncpy(hostname, argv[0], MAXHOSTLEN);
	hostname[MAXHOSTLEN] = '\0';
	if ((host = hostconnect(hostname)) < 0) {
	    if (isconsoleshell())
		goto askhost;
	    else
		errorm('f',NULLP(char));
	}
    }
    else
	errorm('u');

    kblamp(LAMP_LOCAL, 0);
    initcom();
    rawmode(ttyfd);
 
    switch (hosttype) {
    case SERIAL_TYPE:
	condline();
        fprintf(stdout, "Serial connection to host (%d baud)", speed);
	if (have488)
	    fprintf(stdout," - using IEEE-488 for graphics");
	break;
    case I488_TYPE:
	hflag = 0;	/* never half-duplex 488 */
        fprintf(stdout, "IEEE-488 connection to host");
	break;
    default:
	hflag = 0;	/* never half-duplex ethernet */
        fprintf(stdout, "%s connection to %s",
			(hosttype == XNS_TYPE) ? "XNS" : "TCP/IP", hostname);
	break;
    }
    fprintf(stdout, "\n\r\n\r");

    strcat(cmdbuf, hostname);

    pipesetup();

    signal(SIGHUP, cleanup);
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);

    /* only want readhost() to optionally dump core for these signals */
    signal(SIGSEGV, exit);
    signal(SIGBUS, exit);
    
    /* both readhost() and writehost() use same routine */
    signal(SIGUSR1, setpipeready);

    /*
     * readhost() is the process which does graphics.  It is made the
     * parent so that the debugger can be used on it.
     */
    switch (pipereaderpid = writehostpid = fork()) {
	case -1: 
	    errorm('F',"fork failed");
	    break;

	case 0:
	    parent = 0;
	    pipereaderpid = getppid();
	    close(fromwriter);
	    close(towriter);
	    if (usenetinput)
		close(infile);
	    errno = 0;
	    writehost();	/* keyboard to host */
	    cleanup(0);
	    break;

    }
    /*
     * if half duplex or IEEE-488 with serial, we need another process to read
     * the host line.
     */
    if (usenetinput) {
	switch (netinputpid = fork()) {
	    case -1: 
		errorm('F',"fork failed");
		break;

	    case 0:
		signal(SIGUSR1, SIG_DFL);
		parent = 0;
		pipereaderpid = 0;
		writehostpid = 0;
		close(fromreader);
		close(towriter);
		close(fromwriter);
		close(toreader);
		close(infile);
		errno = 0;
		netinput();
		cleanup(0);
		break;
	}
    }
    /*
     * if IEEE-488 with serial, we need yet another process to read the
     * IEEE-488 line
     */
    if (hosttype == SERIAL_TYPE && have488) {
	switch (i488inputpid = fork()) {
	    case -1: 
		errorm('F',"fork failed");
		break;

	    case 0:
		signal(SIGUSR1, SIG_DFL);
		parent = 0;
		pipereaderpid = 0;
		writehostpid = 0;
		netinputpid = 0;
		close(fromreader);
		close(towriter);
		close(fromwriter);
		close(toreader);
		close(infile);
		errno = 0;
		i488input();
		cleanup(0);
		break;
	}
    }
    close(fromreader);
    close(toreader);
    if (usenetinput)
	close(outfile);
    if (dflag[2]) {
	signal(SIGSEGV, SIG_DFL);
	signal(SIGBUS, SIG_DFL);
    }
    else {
	signal(SIGSEGV, cleanup);
	signal(SIGBUS, cleanup);
    }
    errno = 0;
    readhost();		/* host to screen and gl */
    cleanup(0);
}

init()
{
    register unsigned i;
    long err = 0;
    FILE *fp;

    signal(SIGINT, cleanup);
    signal(SIGQUIT, cleanup);
    signal(SIGTERM, cleanup);
#ifdef NOFPE_ABORT
    fpsigset(NULL, NOFPE_ABORT);
#endif

#ifdef GL1
    fprintf(stdout, "IRIS GL1 Terminal Emulator\n\r");
#endif GL1
#ifdef GL2
    fprintf(stdout, "IRIS GL2 Terminal Emulator\n\r");
#endif GL2

    errorm('i', usage, perror, NULLP(FUNPTR), cleanup);
    kblamp(LAMP_LOCAL, 1);

    /*
     * initialize maxcom and check that none of the routines in the
     * dispatch table have too many arguments
     */
    maxcom = dispatchLen;
#ifndef GL1
    maxcom--;	/* don't count the the null entry on the end */
#endif	
    for (i = 0; i < maxcom; i++) {
	if (strlen(dispatch[i].format) > MAXARGS) {
	    fprintf(stderr, "wsiris: remote routine ");
	    errorm('w',"remote routine %s has more than %d arguments",
		       getcmdname(i), MAXARGS);
	    err++;
	}
    }
    if (err)
	errorm('f',"fix these routines");

    /*
     * get default serial speed from config switches
     */
    speedcode = convspeed(speed = getdefaultspeed());

    /*
     * is there a IEEE-488 board?
     */
    if (fp = fopen(I488_RDEV,"r+")) {
	have488 = 1;
fclose(fp);
    }
    else
	have488 = 0;

}

processflags(argc, argv)
register int    argc;
register char  *argv[];
{
    register char  *ptr;
    register int    i;

    while (--argc > 0 && *(++argv)[0] == '-') {
	ptr = argv[0];
	while (*++ptr) {
	    switch (*ptr) {
		case 'd': 
		    if (--argc < 1)
			errorm('u');
		    ++argv;
		    if ((i = atoi(*argv)) && i > 0 && i < n_dflag)
			dflag[i]++;
		    else
			errorm('u');
		    if (i == 3) {
			if (--argc < 1)
			    errorm('u');
			++argv;
			strcpy(logfile,*argv);
		    }
		    break;

		case 'e': 
		    if (--argc < 1)
			errorm('u');
		    ++argv;
		    if (strcmp(*argv, "none") == 0)
			noesc = 1;
		    else if (strlen(*argv) != 1)
			errorm('u');
		    else {
			escchar = *argv[0];
			noesc = 0;
		    }
		    break;
		case 'f': 
		    fflag++;
		    break;
		case 'h': 
		    hflag++;
		    break;
		case 'i': 
#ifdef TCP
		    iflag++;
#else
		    errorm('f', "this version doesn't support TCP/IP");
#endif
		    break;
		case 'l': 
		    if (--argc < 1)
			errorm('u');
		    ++argv;
		    if (strncmp(*argv, "/dev/", 5) == 0)
			strcpy(serialline, *argv);
		    else {
			strcpy(serialline, "/dev/");
			strcat(serialline, *argv);
		    }
		    break;
		case 'p': 
		    pflag++;
		    break;
		case 's': 
		    if (--argc < 1)
			errorm('u');
		    speedcode = convspeed(speed = atoi(*++argv));
		    if (speedcode == 0)
			errorm('f',"invalid speed\n");
		    break;
		case 'x': 
		    xflag++;
		    break;
		case 'y': 
		    yflag++;
		    break;
		case 'z': 
		    if (--argc < 1)
			errorm('u');
		    ++argv;
		    if ((i = atoi(*argv)) && i > 0 && i < n_zflag)
			zflag[i]++;
		    else
			errorm('u');
#ifdef GL1
		    if (zflag[4])
			errorm('f',
	"This option is not available on Series 1000 hardware");
#endif GL1
		    break;

		default: 
		    errorm('u');
		    break;

	    }
	}
    }
    return(argc);
}

/*
**	pipesetup - setup up the pipes used to allow the processes to
**		    communicate with each other.
**
*/
pipesetup()
{
    int  pipefd[2];		/* array for pipe() call */
    /*
     * Set up pipe for half-duplex or IEEE-488 with serial (pipe 1).
     * Can''t just turn on echo because of textport.
     */
    if (hflag || zflag[4] || hosttype == SERIAL_TYPE && have488) {
	if (pipe(pipefd) != 0)
	    errorm('F',"can't create pipe 1");
	infile = pipefd[READ];
	outfile = pipefd[WRITE];
	usenetinput = 1;
    }
    else
	infile = host;

    /*
     * Pipe for readhost() to give commands to writehost()
     */
    if (pipe(pipefd))
	errorm('F',"can't create pipe 2");
    fromreader = pipefd[READ];
    towriter = pipefd[WRITE];
    /* non-blocking reads */
    fcntl(fromreader, F_SETFL, fcntl(fromreader, F_GETFL, 0) | O_NDELAY);

    /*
     * Pipe for writehost() to give commands to readhost()
     */
    if (pipe(pipefd))
	errorm('F',"can't create pipe 3");
    fromwriter = pipefd[READ];
    toreader = pipefd[WRITE];
    /* non-blocking reads */
    fcntl(fromwriter, F_SETFL, fcntl(fromwriter, F_GETFL, 0) | O_NDELAY);
}

cleanup(sig)
int sig;
{
    fflush(stdout);
    fflush(stderr);
    restoremode(ttyfd);
    kblamp(LAMP_LOCAL,0);
    kblamp(LAMP_KBDLOCKED, 0);
    if (graphinited) {
	xtpon();
	wsgexit(0);
    }
    if (lf)
	fflush(lf);

    /* netinput() and i488input() processes have pipereaderpid = 0 */
    if (!parent && pipereaderpid) {
	sendpipecmd(toreader, TERMINATE, 0);
    }

    /* 
     * readhost process will kill these three
     */
    killwritehost();
    if (netinputpid > 0 && kill(netinputpid, SIGKILL) == 0) {
	while (wait((int *)0) != netinputpid)
	    continue;
    }
    if (i488inputpid > 0 && kill(i488inputpid, SIGKILL) == 0) {
	while (wait((int *)0) != i488inputpid)
	    continue;
    }
    if (sig == SIGBUS) {
	errorm('w',"bus error");
	xstat = 1;
    }
    else if (sig == SIGSEGV) {
	errorm('w',"segmentation violation");
	xstat = 1;
    }
    sync();

#ifndef GL1
    /*
     * wait so user can see error message before textport disappears
     */
    if (parent && ismex() && !isconsoleshell())
	sleep(3);	
#endif

    /*
     *
     */
    if (parent && xstat && isconsoleshell()) {
	fprintf(stderr,"Error exit: invoking a shell\n");
	shell();
	restoremode(ttyfd);
    }
    exit(xstat);
}

killwritehost()
{
    if (writehostpid > 0 && kill(writehostpid, SIGKILL) == 0) {
	while (wait((int *)0) != writehostpid)
	    continue;
    }
}
