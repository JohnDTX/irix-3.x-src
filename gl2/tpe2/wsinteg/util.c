#include <sys/param.h>
#include <termio.h>
#include <stdio.h>
#include <signal.h>
#include "term.h"

#define DEFAULT_SHELL	    "/bin/sh"
#define BELL		    7


shell(ttyfd)
{
    register char **cp;
    char   *shell;
    extern char **environ;
    extern char *getenv();
    extern char cmdbuf[];
    extern int  subshellpid;

    for (cp = environ; *cp; cp++)
	if (strncmp(*cp, "CMDNAME=", 8) == 0)
	    *cp = cmdbuf;

    pushsig(SIGINT, SIG_IGN);
    pushsig(SIGQUIT, SIG_IGN);
    pushsig(SIGCLD, SIG_DFL);
    restoremode(ttyfd);

    if ((subshellpid = fork()) == -1)
	perror("wsiris: can't fork");
    else if (subshellpid == 0) {
	register int    i;

	if ((shell = getenv("SHELL")) == 0)
	    shell = DEFAULT_SHELL;

	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	for (i = 3; i < NOFILE; i++)
	    close(i);
	printf("!\n\r");
	fflush(stdout);
	execlp(shell, shell, 0);
	oops("wsiris: couldn't exec %s\n\r", shell);
    }
    else {
	while (wait((int *)0) != subshellpid)
	    continue;
    }
    subshellpid = 0;
    popsig(SIGINT);
    popsig(SIGQUIT);
    popsig(SIGCLD);
    printf("!\n\r");
    fflush(stdout);
    rawmode(ttyfd);
}

lshift(p, i)
register char  *p;
register int    i;
{
    while (i-- > 0) {
	p[0] = p[1];
	p++;
    }
}

setcatch(signo, action)
int     signo;
int     action;
{
    register int    osig;

    if ((osig = (int) signal(signo, SIG_IGN)) == (int) SIG_DFL)
	signal(signo, action);
    else
	signal(signo, osig);
}

int     osigs[NSIG + 1];
pushsig(signo, action)
int     signo;
int     action;
{
    osigs[signo] = (int) signal(signo, action);
}

popsig(signo)
int     signo;
{
    signal(signo, osigs[signo]);
}

putexpc(c, lf)
FILE * lf;
register    c;
{
    if (c < ' ' && c != '\n') {
	putc('^', lf);
	putc(c + '@', lf);
    }
    else if (c == 0177)
	fprintf(lf, "^?");
    else
	putc(c, lf);
}

/* 
 * concat - concats two strings, mallocing space for them.  Returns
 * NULL if malloc returns NULL.
 */
char   *concat(s1, s2)
register char  *s1, *s2;
{
    register char  *rv;

    if (s1 == NULL)
	return NULL;
    if (s2 == NULL)
	s2 = "";
    if (rv = (char *) malloc(strlen(s1) + strlen(s2) + 1)) {
	strcpy(rv, s1);
	strcat(rv, s2);
    }
    return rv;
}

bellring()
{
    putscreenchar(BELL);
    flushscreen();
}

/*
 * kblamp - manipulate the keyboard lamps.  A previous ginit() is not
 * required to call this routine.
 */
kblamp(which, state)
int     which, state;
{

    if (state)
	lampon(which);
    else
	lampoff(which);

}
