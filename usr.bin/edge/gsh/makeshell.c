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
#include "sys/types.h"
#include "sys/stat.h"
#include "sys/sysmacros.h"
#include "fcntl.h"
#include "string.h"

int	child_pid, child_dead;
int	pty_num;

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
	termiop->c_oflag &= ~(OCRNL | ONOCR |
				    CRDLY|NLDLY|TABDLY|BSDLY|VTDLY|FFDLY);
	termiop->c_oflag |= (OPOST | ONLCR);
	termiop->c_lflag |= (ISIG | ICANON | ECHO);
	(void) ioctl(1, TCSETA, termiop);
}

/*
 * Actually start a shell going
 */
void
start_shell()
{
	if (flag_shell)
		execvp(*shell_argv, shell_argv);
	else
		execl(shell, minus_shell_name, 0);
}

/*
 * Hook up the child to the pty, then exec a shell
 */
void
setup_child(termiop)
	struct termio *termiop;
{
	register int i;
	int save_errno;
	int fd;
	char *failure;
	char buf[100];

	/*
	 * Restore priority to user normal
	 */
#ifdef	LOGIN
	nice(-20);
	nice(-20);
	nice(20);
#endif

	/* let user own the tty */
	(void) chown(slave_name, getuid(), getgid());
	(void) chmod(slave_name, 0666);

	/* drop extra permissions */
	setgid(getgid());
	setuid(getuid());

	/*
	 * First, close down any lingering file descriptions that we
	 * don't want.
	 */
	for (i = 0; i < 100; i++)
		close(i);
	setpgrp();

	/*
	 * Next, open up the slave tty.  If this fails, open up /dev/tty
	 * and gasp out the failure.  When this succeeds, setup the
	 * default terminal modes of the tty to match the parent.  However,
	 * if the terminal was in some kind of raw mode, put it in to
	 * a cooked mode.
	 * For compatability with STUPID code which read's on 1, or writes
	 * on 0, we make 0, 1, and 2 read/write.
	 */
	failure = "opening slave tty";
	if (open(slave_name, 2) < 0)		/* stdin */
		goto gasp;
	if (open(slave_name, 2) < 0)		/* stdout */
		goto gasp;
	if (open(slave_name, 2) < 0)		/* stderr */
		goto gasp;
	set_child_termio(termiop);
	/*
	 * Restore child's signals to default state
	 */
	for (i = 0; i < NSIG; i++)
		signal(i, SIG_DFL);
	/*
	 * Lastly, exec the shell, just like login
	 */
	start_shell();
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
		exit (-1);
	}
	sprintf(buf, "gsh: errno %d during %s\n", save_errno, failure);
	write(fd, buf, (unsigned) strlen(buf));
	exit(-1);
}

/*
 * Setup the parent process (the gsh) that is controlling the shell
 */
setup_parent(pty_num)
	int pty_num;
{
	extern int child_died();
#ifdef	LOGIN
	extern char *my_loginname();
#endif
	char tty[100];;

	/*
	 * Log the user in
	 */
	(void) sprintf(tty, "ttyq%d", pty_num);
#ifdef	LOGIN
	winlogin(my_loginname(), tty, getpid());
#endif
	(void) signal(SIGCLD, child_died);
}

void
makeshell()
{
	struct termio termio;
	struct stat sb;

	/*
	 * Open clone driver for pty and get a controller
	 */
	if ((master_fd = open("/dev/ptc", O_RDWR|O_NDELAY)) >= 0) {
		if (fstat(master_fd, &sb) < 0) {
			perror("gsh: can't stat");
			exit(-1);
		}
		pty_num = minor(sb.st_rdev);
		sprintf(slave_name, "/dev/ttyq%d", pty_num);
		sprintf(master_name, "/dev/ptc%d", pty_num);
		(void) ioctl(master_fd, PTIOC_QUEUE, 0);
		goto skip;
	}
	/*
	 * Clone device didn't work... Search the old way
	 */
	for (pty_num = 0; ; pty_num++) {
		sprintf(slave_name, "/dev/ttyq%d", pty_num);
		sprintf(master_name, "/dev/ptc%d", pty_num);
		if ((master_fd = open(master_name, O_RDWR|O_NDELAY)) < 0) {
			if (errno == EIO) {
				/*
				 * We get an EIO from the pty code if
				 * the slot is already in use.
				 */
				continue;
			}
			if (errno == ENXIO) {
				/*
				 * We get an ENXIO from the pty code,
				 * if we ran out of pty's
				 */
				printf("gsh: no more pty's\n");
				exit(-1);
				/* NOTREACHED */
			}
			perror("gsh: can't open any pty's");
			exit(-1);
			/* NOTREACHED */
		}
		/*
		 * We found a master tty!  Setup it in queued mode
		 */
		(void) ioctl(master_fd, PTIOC_QUEUE, 0);
		break;
	}

skip:

	/* save tty state */
	(void) ioctl(0, TCGETA, &termio);

	/*
	 * Now, setup the child process
	 */
	switch (child_pid = fork()) {
	  case -1:
		printf("gsh: no more processes\n");
		exit(-1);
		/* NOTREACHED */
	  case 0:
		setup_child(&termio);
		break;
	  default:
		setup_parent(pty_num);
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

	/*
	 * Choose shell to use: pick up shell from environment,
	 * defaulting to /bin/sh if environment is not set.
	 */
	shell = getenv("SHELL");
	if (shell == NULL)
		shell = "/bin/sh";

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

#ifdef	notdef
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
#endif
