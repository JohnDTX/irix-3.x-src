/*
 * $Source: /d2/3.7/src/usr.bin/edge/common/RCS/makeshell.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:45:57 $
 */
/*
 * Make a shell
 */
#include "errno.h"
#include "stdio.h"
#include "signal.h"
#include "gsh.h"
#include "window.h"
#include "gl.h"
#include "sys/pty_ioctl.h"
#include "sys/termio.h"
#include "tf.h"
#include "manage.h"
#include "dbxshm.h"

int	child_pid, child_dead;

int	master_fd;
char	master_name[100];
char	slave_name[100];
char	*shell, *shell_name;
char	minus_shell_name[100] = "-";
char	title[80];

/*
 * Setup child termio state so that it is at a given default.
 * Let users shell enable special things.
 */
void
set_child_termio(termiop)
	register struct termio *termiop;
{
	termiop->c_iflag &= ~(IGNBRK | PARMRK | INPCK | INLCR | IGNCR | IBLKMD);
	termiop->c_iflag |= (BRKINT | IGNPAR | ISTRIP | ICRNL | IXON);
	termiop->c_oflag &= ~(OCRNL | ONOCR);
	termiop->c_oflag |= (OPOST | ONLCR);
	termiop->c_lflag |= (ISIG | ICANON | ECHO);
	if (ioctl(1, TCSETA, termiop) < 0) {
		abort();
	}
}

/*
 * Actually start a shell going
 */
void
start_child(argv)
char	**argv;
{
		execvp(*argv, argv);
}

/*
 * Hook up the child to the pty, then exec a shell
 */
void
setup_child(ptfd, wtmp, termiop, argv)
int	ptfd;
WINTTYMAP	*wtmp;
struct termio *termiop;
char	**argv;
{
	register int i;
	int save_errno;
	int fd;
	char *failure;
	char buf[100];

	/* let user own the tty */
	if (chown(wtmp->wt_slavename, getuid(), getgid()) == -1) {
		fprintf(stderr, "cannot chown %s\n", wtmp->wt_slavename);
	}
	(void) chmod(wtmp->wt_slavename, 0666);
		

	/* drop extra permissions */
	setgid(getgid());
	setuid(getuid());

	/*
	 * First, close down any lingering file descriptions that we
	 * don't want.
	 */
	for (i = 0; i < 100; i++) {
#ifndef mips
		if (i == commpipe[1]) {
			continue;
		}
#endif
		if (dbflag && ((wtmp == dbx_win) && (i == 2))) {
			continue;
		}
		if ((i != ptfd)) {
			close(i);
		}
	}
	/*
	 * this is a hack to get around a kernel bug.
	 */
	shmdetach();
/*
	setpgrp();
*/
	failure = "opening slave tty";
	if (dup(ptfd) == -1) {
		goto gasp;
	}
	if (dup(ptfd) == -1) {
		goto gasp;
	}
	if (dup(ptfd) == -1) {
		goto gasp;
	}
/*
	if (wtmp == dbx_win) {
		if (open(syncpty.sp_slavename, 2) < 0)
			goto gasp;
	}
*/

	/*
	 * Next, open up the slave tty.  If this fails, open up /dev/tty
	 * and gasp out the failure.  When this succeeds, setup the
	 * default terminal modes of the tty to match the parent.  However,
	 * if the terminal was in some kind of raw mode, put it in to
	 * a cooked mode.
	 * For compatability with STUPID code which read's on 1, or writes
	 * on 0, we make 0, 1, and 2 read/write.
	failure = "opening slave tty";
	if (open(wtmp->wt_slavename, 2) < 0)
		goto gasp;
	if (open(wtmp->wt_slavename, 2) < 0)
		goto gasp;
	if (open(wtmp->wt_slavename, 2) < 0)
		goto gasp;
	 */
	set_child_termio(termiop);
	/*
	 * Restore child's signals to default state
	 */
	for (i = 0; i < NSIG; i++)
		signal(i, SIG_DFL);
	/*
	 * Lastly, exec the shell, just like login
	 */
	start_child(argv);
	failure = "execing shell";

gasp:
	/*
	 * Oh well, eat hot death...
	 */
	save_errno = errno;
	if ((fd = open("/dev/tty", 2)) < 0) {
		/*
		 * Oh well, we can't even open /dev/tty.
		 * Give up the ghost, silently.
		 */
		myexit (-1);
	}
	sprintf(buf, "gsh: errno %d during %s\n", save_errno, failure);
	write(fd, buf, strlen(buf));
	myexit(-1);
}

/*
 * Setup the parent process (the gsh) that is controlling the shell
 */
setup_parent(pty_num)
	int pty_num;
{
	extern int child_died();
	char *my_loginname();
	char tty[100];;

	/*
	 * Log the user in
	 */
/*
	sprintf(tty, "ttyq%d", pty_num);
	winlogin(my_loginname(), tty, getpid());
*/
	signal(SIGCLD, child_died);
}

void
makechild(wtmp, argv)
WINTTYMAP	*wtmp;
char	**argv;
{
	int pty_num;
	struct termio termio;
	int	ptfd;


	/*
	 * Now, setup the child process.  First, snapshot the terminal
	 * characteristics of the parent.
	 */
	(void) chown(wtmp->wt_slavename, getuid(), getgid());
	(void) chmod(wtmp->wt_slavename, 0666);
	setpgrp();
	if ((ptfd = open(wtmp->wt_slavename, 2)) < 0) {
		fprintf(stderr, "canot open slave %s\n", wtmp->wt_slavename);
		myexit(-1);
	}
	(void) ioctl(0, TCGETA, &termio);
	switch (wtmp->wt_pid = fork()) {
	  case -1:
		printf("gsh: no more processes\n");
		myexit(-1);
		/* NOTREACHED */
	  case 0:
		setup_child(ptfd, wtmp, &termio, argv);
		break;
	  default:
		setup_parent(wtmp->wt_ptynum);
		break;
	}
}

/*
 * Pick which shell to use.  Let users environment be the first
 * choice.
 */
void
pickshell()
{
	extern char *getenv(), *strrchr();

	shell = "/jym/bin/dbx";

	/*
	 * Find the name of the shell for its first argument
	 */
	shell_name = strrchr(shell, '/');
	if (shell_name == NULL) {
		shell_name = shell;
		strcpy(minus_shell_name, shell);
	} else {
		shell_name++;
		strcat(minus_shell_name, shell_name);
	}
}

/*
 * Find a new origin for a clone window.  Make sure that it fits on the screen,
 * and is the same size.  Punt if the window is larger than the screen, in
 * the first place.
 */
void
neworigin(xorg, yorg, xsize, ysize)
	int *xorg, *yorg, xsize, ysize;
{
	int llx, lly;

	if ((xsize >= XMAXSCREEN) || (ysize >= YMAXSCREEN))
		return;

	llx = *xorg + 25;
	lly = *yorg - 25;
	if (llx < 0)
		llx = 0;
	if (lly < 0)
		lly = 0;
	if (llx + xsize > XMAXSCREEN)
		llx = XMAXSCREEN - xsize;
	if (lly + ysize > YMAXSCREEN)
		lly = YMAXSCREEN - ysize;
	*xorg = llx;
	*yorg = lly;
}

/*
 * Clone the current gsh, making as near a duplicate that we can.
 * Add a "-p" argument for the size of this window.
 */
void
clone(iseditor)
	int iseditor;
{
	int pid;
	char *newargv[500];
	int av;
	char **argv;
	char nums[4][50];
	int xsize, ysize, xorg, yorg;

	/*
	 * We get this information before we fork, because the child
	 * process won't have graphics capability.
	 */
	getsize(&xsize, &ysize);
	xsize--;
	ysize--;
	getorigin(&xorg, &yorg);

	pid = fork();
	switch (pid) {
	  case 0:			/* child */
		/*
		 * Construct newargv vector from command line flags.
		 */
		av = 0;
		newargv[av++] = "gsh";
		if (flag_font) {
			newargv[av++] = "-f";
			newargv[av++] = flag_font;
		}
		if (flag_hold)
			newargv[av++] = "-h";
		if (iseditor || flag_signal)
			newargv[av++] = "-v";

#ifdef	notdef
		/*
		 * setup position arguments
		 */
		newargv[av++] = "-p";
		neworigin(&xorg, &yorg, xsize, ysize);
		sprintf(&nums[0][0], "%d", xorg);
		sprintf(&nums[1][0], "%d", yorg);
		sprintf(&nums[2][0], "%d", xorg + xsize - 1);
		sprintf(&nums[3][0], "%d", yorg + ysize - 1);
		newargv[av++] = nums[0];
		newargv[av++] = nums[1];
		newargv[av++] = nums[2];
		newargv[av++] = nums[3];
#endif
		sprintf(&nums[0][0], "%d", txport[0].tx_rows);
		sprintf(&nums[1][0], "%d", txport[0].tx_cols);
		newargv[av++] = "-s";
		newargv[av++] = nums[0];
		newargv[av++] = nums[1];
		if (iseditor) {
			newargv[av++] = "-c";
			newargv[av++] = "vi";
		} else
		if (flag_shell) {
			newargv[av++] = "-c";
			argv = shell_argv;
			while (*argv)
				newargv[av++] = *argv++;
		}
		newargv[av] = 0;
		execvp(newargv[0], newargv);
		/* FALLTHROUGH */
	  case -1:			/* no more processes */
		/*
		 * If we fail for any reason, beep the bell so user knows
		 * that SOMETHING went worng
		 */
		ringbell();
		break;
	  default:			/* parent */
		break;
	}
}
