/*
 * Code to manage a shell frame.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/shell.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:36 $
 */

#include "tf.h"
#include "te.h"
#include "kb.h"
#include "shell.h"
#include "message.h"
#include "device.h"

#include "sys/types.h"
#include "sys/stat.h"
#include "sys/pty_ioctl.h"
#include "sys/termio.h"
#include "sys/sysmacros.h"
#include "utmp.h"
#include "signal.h"
#include "fcntl.h"
#include <stdio.h>

/*
 * Figure the users login name out
 * XXX this needs to be written for real
 */
char *
my_loginname()
{
	char *name;
	extern char *getenv();

	/*
	 * Be lazy - get environment variable for now
	 */
	name = getenv("LOGNAME");
	if (name == (char *)0)
		return ("nobody");
	else
		return (name);
}

/*
 * Fill in the utmp entry
 */
void
fill_utmp_entry(sh, entry)
	register shellframe *sh;
	register struct utmp *entry;
{
	char tty[10];
	char *name;

	/* clear out the utmp entry */
	bzero(entry, sizeof(struct utmp));

	/* build tty name */
	tty[1] = 0;
	tty[2] = 0;
	sprintf(tty, "%d", sh->sh_ptynum);

	/* get login name */
	name = my_loginname();

	/* fill in entry */
	strncpy(entry->ut_user, name, sizeof(entry->ut_user));
	entry->ut_id[0] = 'q';
	entry->ut_id[1] = tty[0];
	entry->ut_id[2] = tty[1];
	entry->ut_id[3] = tty[2];
	strcpy(entry->ut_line, "ttyq");
	strcat(entry->ut_line, tty);
	entry->ut_pid = sh->sh_childpid;
	entry->ut_type = USER_PROCESS;
	entry->ut_time = time((long *) 0);
}

/*
 * Login in the shell so that it shows up in utmp.  This allows the
 * user to then run "login" if they want to change uid/gid's
 */
void
shlogin(sh)
	register shellframe *sh;
{
	register struct utmp *u;
	struct utmp entry;

	fill_utmp_entry(sh, &entry);
	/*
	 * For each entry in the utmp file, get an entry and see if
	 * its for this tty.  If so, log it in.  Note that the pid
	 * check isn't valid for the shell frame because there will
	 * be no direct "exec" path to this code, thus the pid will
	 * have no meaning.  Instead, look for an entry with the same
	 * "id" tag as what we will write.
	 */
	while (u = getutent()) {
		if (strncmp(u->ut_id, entry.ut_id, sizeof(entry.ut_id)) == 0) {
			/*
			 * NOTE: By doing a structure copy here, we
			 * assure that the less than useful code in
			 * libc will work correctly.
			 */
			*u = entry;
			pututline(u);
			break;
		}
	}
	if (!u) {
		/*
		 * Couldn't find an entry for this line.  Append a new
		 * entry to the end of the utmp file.
		 */
		pututline(&entry);
	}
}

/*
 * Logout the shell from utmp
 */
void
shlogout(sh)
	register shellframe *sh;
{
	struct utmp *u;
	struct utmp entry;

	if (sh->sh_flags & SH_LOGIN) {
		fill_utmp_entry(sh, &entry);
		setutent();
		if (u = getutline(&entry)) {
			u->ut_type = DEAD_PROCESS;
			pututline(u);
		}
		endutent();
	}
}

static void
shdoread(sh)
	shellframe *sh;
{
	char buf[1024];
	int nb;
	int count;

	teliftcursor(sh->sh_te);
	count = 0;
	for (;;) {
		nb = read(sh->sh_masterfd, buf, sizeof(buf));
		(void) ioctl(sh->sh_masterfd, PTIOC_QUEUE, 0);
		if (nb <= 0)
			break;
		count += nb;
		teputchars(sh->sh_te, buf, nb);
		if (count >= 300) {
			tedraw(sh->sh_te);
			count = 0;
		}
	}

	if (count) {
		if (count < 300) {
			/*
			 * If count is small, wait until the timer has gone
			 * off to do the update.  This gives the shell
			 * process a chance to send more data.
			 */
			if (!(sh->sh_flags & SH_TIMER)) {
				qdevice(TIMER0);
				sh->sh_flags |= SH_TIMER;
			}
			sh->sh_flags |= SH_NEEDUPDATE;
		} else {
			tedraw(sh->sh_te);
			tedropcursor(sh->sh_te);
		}
	} else
		tedropcursor(sh->sh_te);
}

void
shdowrite(sh, keybuf, n)
	shellframe *sh;
	short *keybuf;
	int n;
{
	register int i;
	char c;

	for (i = n; i--; keybuf++) {
		if (*keybuf & 0xFF00) {
			if (*keybuf == CODE_BREAK)
				ioctl(sh->sh_masterfd, TCSBRK, 0);
		} else {
			c = *keybuf;
			if (write(sh->sh_masterfd, &c, 1) < 0) {
				perror("wsh");
				shlogout(sh);
			}
		}
	}
}

/*
 * Run the given shell frame.
 */
void
shevent(sh, event, value)
	register shellframe *sh;
	long event, value;
{
	switch (event) {
	  case QPTY_CANREAD:
		if (value == sh->sh_ptynum)
			shdoread(sh);
		break;
	  case QPTY_STOP:
		if (value == sh->sh_ptynum)
			sh->sh_flags |= SH_STOPPED;
		break;
	  case QPTY_START:
		if (value == sh->sh_ptynum)
			sh->sh_flags &= ~SH_STOPPED;
		break;
	  case TIMER0:
		if (sh->sh_flags & SH_TIMER) {
			unqdevice(TIMER0);
			sh->sh_flags &= ~SH_TIMER;
			if (sh->sh_flags & SH_NEEDUPDATE) {
				sh->sh_flags &= ~SH_NEEDUPDATE;
				tedraw(sh->sh_te);
				tedropcursor(sh->sh_te);
			}
		}
		break;
	  case KBMSG:
		shdowrite(sh, ((struct kbmsgvalue *)value)->data,
			      ((struct kbmsgvalue *)value)->len);
		free(((struct kbmsgvalue *)value)->data);	/* XXX */
		free((char *) value);				/* XXX */
		break;
	  case UNIXSIG+SIGCLD:
		if (value == sh->sh_childpid) {
			printf("shproc: exit (pid=%d)\n", value);
			shlogout(sh);
			exit(0);				/* XXX */
		}
		break;
	  case REDRAW:
		teredraw(sh->sh_te);
		break;
	  case SCROLL:
		tescroll(sh->sh_te, value);
		break;
	  default:
		fprintf(stderr, "shproc: unknown event type %d\n", event);
		ASSERT(1 == 0);
		break;
	}
}

/*
 * Inform dispatcher of the events we want to see
 */
void
shCatchEvents(sh)
	register shellframe *sh;
{
	catchEvent(QPTY_CANREAD, shevent, sh);
	catchEvent(QPTY_STOP, shevent, sh);
	catchEvent(QPTY_START, shevent, sh);
	catchEvent(TIMER0, shevent, sh);
	catchEvent(UNIXSIG+SIGCLD, shevent, sh);
	catchEvent(KBMSG, shevent, sh);
	catchEvent(REDRAW, shevent, sh);
	catchEvent(SCROLL, shevent, sh);
}

/*
 * Inform dispatcher of the events we no longer wish to see
 */
void
shUnCatchEvents(sh)
	register shellframe *sh;
{
	catchEvent(QPTY_CANREAD, shevent, sh);
	catchEvent(QPTY_STOP, shevent, sh);
	catchEvent(QPTY_START, shevent, sh);
	catchEvent(TIMER0, shevent, sh);
	catchEvent(UNIXSIG+SIGCLD, shevent, sh);
	catchEvent(KBMSG, shevent, sh);
	catchEvent(REDRAW, shevent, sh);
	catchEvent(SCROLL, shevent, sh);
}

void
shfree(sh)
	register shellframe *sh;
{
	signal(SIGCLD, SIG_IGN);			/* no races, please */
	if (sh->sh_childpid)
		kill(SIGKILL, - sh->sh_childpid);
	close(sh->sh_masterfd);
	close(sh->sh_slavefd);
	if (sh->sh_master)
		FREE(sh->sh_master);
	if (sh->sh_slave)
		FREE(sh->sh_slave);
	shUnCatchEvents(sh);
	FREE(sh);
}

/*
 * Catch SIGCLD and qenter a message
 * XXX no way to pass status
 */
shsigcld_handler()
{
	int pid;

printf("SIGCLD: ");
	pid = wait((int *)0);
	if (pid >= 0)
		qenter(UNIXSIG+SIGCLD, pid);
printf("pid=%d\n", pid);
	signal(SIGCLD, shsigcld_handler);
}

/*VARARGS2*/
int
shnew(te, logitin, prog, av)
	termulator *te;
	int logitin;
	char *prog;
	int av;
{
	register shellframe *sh;
	struct termio termio;
	struct stat sb;
	char buf[20];
	int i;
	long shid;

	sh = MALLOC(shellframe *, sizeof(shellframe));
	if (sh) {
		/*
		 * Try to find a tty to use
		 */
		if ((sh->sh_masterfd = open("/dev/ptc", O_RDWR|O_NDELAY)) >= 0) {
			if (fstat(sh->sh_masterfd, &sb) < 0)
				goto error;

			sh->sh_ptynum = minor(sb.st_rdev);
			sprintf(buf, "/dev/ttyq%d", sh->sh_ptynum);
			sh->sh_slave = MALLOC(char *, strlen(buf)+1);
			strcpy(sh->sh_slave, buf);

			sprintf(buf, "/dev/ptc%d", sh->sh_ptynum);
			sh->sh_master = MALLOC(char *, strlen(buf)+1);
			strcpy(sh->sh_master, buf);

			(void) ioctl(sh->sh_masterfd, PTIOC_QUEUE, 0);
		} else
			goto error;
		sh->sh_te = te;

		sh->sh_slavefd = open(sh->sh_slave, O_RDWR|O_NDELAY);
		if (sh->sh_slavefd < 0)
			goto error;

		/* start up process */
		(void) ioctl(0, TCGETA, &termio);
		sh->sh_childpid = fork();
		if (sh->sh_childpid < 0)
			goto error;
		if (sh->sh_childpid == 0) {
			/* child */
			nice(-20);			/* restore priority */
			nice(-20);			/* restore priority */
			nice(20);

			/* let user own the tty */
			(void) chown(sh->sh_slave, getuid(), getgid());
			(void) chmod(sh->sh_slave, 0666);

			setgid(getgid());		/* drop permissions */
			setuid(getuid());
			for (i = 0; i < 100; i++)
				close(i);
			setpgrp();
			(void) open(sh->sh_slave, O_RDWR);	/* stdin */
			dup(0);					/* stdout */
			dup(0);					/* stderr */

			termio.c_iflag &= ~(IGNBRK | PARMRK | INPCK |
						     INLCR | IGNCR | IBLKMD);
			termio.c_iflag |= (BRKINT | IGNPAR | ISTRIP |
						    ICRNL | IXON);
			termio.c_oflag &= ~(OCRNL | ONOCR);
			termio.c_oflag |= (OPOST | ONLCR);
			termio.c_lflag |= (ISIG | ICANON | ECHO);
			(void) ioctl(1, TCSETA, &termio);

			for (i = 0; i < NSIG; i++)
				signal(i, SIG_DFL);
			execvp(prog, &av);
		} else {
			char *cp;

			/* parent */
			nice(-20);
			nice(-20);
			nice(10);
			signal(SIGCLD, shsigcld_handler);
			noise(TIMER0, 2);
			if (logitin) {
				sh->sh_flags |= SH_LOGIN;
				shlogin(sh);
			}
			shCatchEvents(sh);
		}
	}
	kbinit();
	return (1);

error:
	shfree(sh);
	return (0);
}
