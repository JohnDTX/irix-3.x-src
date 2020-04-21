/*
 * Process management subroutines
 *
 */
#include "win.h"
#include "gltypes.h"
#include "stdio.h"
#include "signal.h"
#include "sys/types.h"
#include "termio.h"
#include "window.h"
#include "misc.h"
#include "string.h"

static	short procs;
static	short beensignaled;
static	struct termio termio;
static	char minusshell[20] = "-";
static	char *shell;
char	*getenv();

child_cleanup()
{
    wait((int *)0);
    signal(SIGCLD, child_cleanup);
}

/*
 * procinit:
 *	- initialize the process management code
 */
procinit()
{
	int fd;
	char *cp;

	if ((fd = open("/dev/tty", 0)) < 0) {
		perror(" mex (opening /dev/tty)");
		exit(-1);
	}
	ioctl(fd, TCGETA, &termio);
	close(fd);

	/* figure out what shell to use */
	shell = getenv("SHELL");
	if (!shell || !*shell)
		shell = "/bin/sh";
	if (cp = strrchr(shell, '/'))
		cp++;
	else
		cp = shell;
	strcat(minusshell, cp);
	signal(SIGCLD, child_cleanup);
	/*signal(SIGCLD, SIG_IGN);	/* manual says this prevents */
					/* zombie processes */
}

/*
 * procwait:
 *	- wait for all children to die
 */
procwait()
{
	while (procs)
		pause();
}

/*
 * waitforchild:
 *	- when the child completes its fork and has opened up the window
 *	  tty device, it sends us a signal indicating that the window
 *	  is really truly active
 */
waitforchild()
{
	beensignaled = 1;
	signal(SIGUSR1, SIG_IGN);
}

/*
 * newproc:
 *	- create a new shell for a window
 * TODO	- fix up SIGUSR1 code for 4.2 (needed?)
 */
newproc(w)
struct wm_window *w;
{
	register short i;
	int pid, ppid;
	char dev[20];
	extern int (*saveint)(), (*savequit)(), (*savehup)(), (*saveterm)();

	if(wm_attachtx(w) == -1) {
		fprintf(stderr,"mex: no more textports\n");
		return 0;
	}
	ppid = getpid();
	procs++;				/* assume child makes it */
	signal(SIGUSR1, waitforchild);
	beensignaled = 0;
	if (pid = fork()) {
		if (pid == -1) {
			procs--;		/* oops; no more procs */
fprintf(stderr,"mex: can't fork\n");
			return 0;
		}
		w->w_ownerpid = pid;
		if (!beensignaled)
			pause();
		return 1;			/* child is going */
	} else {
		(void) grioctl(GR_LOSEGR, 0);
		for (i = 0; i < 20; i++)	
			close(i);
		sprintf(dev, "/dev/ttyw%d", w->w_txport);
		setpgrp();
		open(dev, 2);			/* stdin */
		open(dev, 2);			/* stdout */
		open(dev, 2);			/* stderr */
		kill(ppid, SIGUSR1);		/* wakeup parent */
		ioctl(0, TCSETA, &termio);	/* make same as console */
		wn_setpieces(w, 1);
		signal (SIGINT, saveint);	/* restore user signals */
		signal (SIGQUIT, savequit);
		signal (SIGHUP, savehup);
		signal (SIGTERM, saveterm);
		execl(shell, minusshell, 0);
		exit(-1);
	}
}

/*
 * killproc:
 *	- kill the process tree attached to a given window
 * TODO	- what if user is running a suid/sgid program?????
 */

#define	killpg(a, b)	kill(-(a), b)

killproc(w)
struct wm_window *w;
{
	if(w->w_ownerpid > 1) {     /* Who knows about this stuff: ??? */
		if(!fork()) {
			if(!(w->w_state&WS_TEXTPORT)) {
				killpg(w->w_ownerpid,SIGTERM);
				kill(w->w_ownerpid,SIGTERM);
				sleep(5);
			}
			killpg(w->w_ownerpid,SIGKILL);
			kill(w->w_ownerpid,SIGKILL);
			exit(0);
		}
	}
}
