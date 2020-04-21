/*
 * {reboot|halt} [-n] [-q] [-h]:
 *	- reboot the system, optionally [-n] not syncing the disks and
 *	  optionally [-q] being quick about it, or not rebooting at all
 *	  and just halting [-h]
 *	- if the programs name is "halt" then force the "-h" flag
 */

static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/etc/RCS/reboot.c,v 1.1 89/03/27 15:38:37 root Exp $";

#include <stdio.h>
#include <sys/reboot.h>
#include <errno.h>

extern	int	errno;

main(argc, argv)
	int argc;
	char *argv[];
{
	register short i;
	short nosync, quick, dohalt;
	short flags;
	char *cp, *rindex();

    /* see if program name contains "halt" as the last component */
	nosync = dohalt = 0;
	quick = 0;
	flags = RB_AUTOBOOT;
	if ((cp = rindex(argv[0], "/")) == NULL)
		cp = argv[0];
	if (strcmp(cp + 1, "halt") == 0) {
		dohalt = 1;
		flags |= RB_HALT;
	}

    /* process arguments */
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-n") == 0) {
			nosync = 1;
			flags |= RB_NOSYNC;
		}
		else if (strcmp(argv[i], "-q") == 0)
			quick = 1;
		else if (strcmp(argv[i], "-h") == 0) {
			dohalt = 1;
			flags |= RB_HALT;
		}
		else {
			fprintf(stderr, "usage: %s [-n] [-q] [-h]\n",
					argv[0]);
			exit(-1);
		}
	}

	/*
	 * Enhanced (SCR 870) to check for reboot misdirected through a
	 * window onto another machine.
	 */
	if (isanetty(0)) {
		char buf[BUFSIZ];

		(void) gethostname(buf, BUFSIZ);
		printf("Reboot%s%s? (y or n): ",
		    (buf[0] == '\0') ? "" : " ", buf);
		gets(buf);
		if (buf[0] != 'y') {
			printf("Reboot cancelled.\n");
			exit(0);
		}
	}

	if (!quick)
		system("/etc/rebootrc");
	if (dohalt) {
		if (reboot(flags) < 0) 
			goto bad;
	}
	else {
		if (kill(1, (nosync) ? SIGRBNOSYNC : SIGREBOOT) < 0)
			goto bad;
	}
	exit(0);

bad:
	if (errno == EPERM)
		fprintf(stderr,"%s: Not superuser\n",argv[0]);
	else
		perror(argv[0]);
	exit(1);
}

isanetty(fd)
	int fd;
{
	char *tty, *ttyname();
	static char XNSTTY[] = "/dev/ttyn";	/* xns: /dev/ttyn* */
	static char TCPTTY[] = "/dev/ttyT";	/* tcp: /dev/ttyT* */

	return (isatty(fd) && (tty = ttyname(fd)) != NULL
		&& (strncmp(tty, XNSTTY, sizeof XNSTTY - 1) == 0
		    || strncmp(tty, TCPTTY, sizeof TCPTTY - 1) == 0));
}
