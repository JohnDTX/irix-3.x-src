char _Origin_[] = "UC Berkeley";

#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <dirent.h>
/*
 * #include <misc/whoami.h>
 */

static char *bindirs[] = {
	"/bin",
	"/usr/bin",
	"/etc",
	"/usr/etc",
	"/usr/etc/yp",
	"/usr/ucb",
	"/usr/games",
	"/usr/demos",
	"/usr/spool/lp/etc/util",
	"/lib",
	"/usr/lib",
	"/usr/lib/spell",
	"/usr/local/bin",
	"/usr/local/etc",
	"/usr/local/lib",
	0
};
static char *mandirs[] = {
	"/usr/man/u_man/man1",
	"/usr/man/a_man/man1",
	"/usr/man/u_man/man2",
	"/usr/man/u_man/man3",
	"/usr/man/u_man/man4",
	"/usr/man/u_man/man5",
	"/usr/man/u_man/man6",
	"/usr/man/a_man/man7",
	"/usr/man/a_man/man8",
	0
};
static char *srcdirs[]  = {
	"/usr/src/bin",
	"/usr/src/usr.bin",
	"/usr/src/etc",
	"/usr/src/ucb",
	"/usr/src/games",
	"/usr/src/demos",
	"/usr/src/libc/gen",
	"/usr/src/libc/print",
	"/usr/src/libc/stdio",
	"/usr/src/libc/sys",
	"/usr/src/lib",
	"/usr/src/usr.lib",
	"/usr/src/include",
	"/usr/src/include/sys",
	"/usr/src/local",
	"/usr/src/local/bin",
	"/usr/src/local/etc",
	"/usr/src/local/lib",
	0
};

char	sflag = 1;
char	bflag = 1;
char	mflag = 1;
char	**Sflag;
int	Scnt;
char	**Bflag;
int	Bcnt;
char	**Mflag;
int	Mcnt;
char	uflag;
int	count;
int	print;
/*
 * whereis name
 * look for source, documentation and binaries
 */
main(argc, argv)
	int argc;
	char *argv[];
{
	argc--, argv++;
	if (argc == 0) {
usage:
		fprintf(stderr, "whereis [ -sbmu ] [ -SBM dir ... -f ] name...\n");
		exit(1);
	}
	do
		if (argv[0][0] == '-') {
			register char *cp = argv[0] + 1;
			while (*cp) switch (*cp++) {

			case 'f':
				break;

			case 'S':
				getlist(&argc, &argv, &Sflag, &Scnt);
				break;

			case 'B':
				getlist(&argc, &argv, &Bflag, &Bcnt);
				break;

			case 'M':
				getlist(&argc, &argv, &Mflag, &Mcnt);
				break;

			case 's':
				zerof();
				sflag++;
				continue;

			case 'u':
				uflag++;
				continue;

			case 'b':
				zerof();
				bflag++;
				continue;

			case 'm':
				zerof();
				mflag++;
				continue;

			default:
				goto usage;
			}
			argv++;
		} else
			lookup(*argv++);
	while (--argc > 0);
	exit(0);
}

getlist(argcp, argvp, flagp, cntp)
	char ***argvp;
	int *argcp;
	char ***flagp;
	int *cntp;
{

	(*argvp)++;
	*flagp = *argvp;
	*cntp = 0;
	for ((*argcp)--; *argcp > 0 && (*argvp)[0][0] != '-'; (*argcp)--)
		(*cntp)++, (*argvp)++;
	(*argcp)++;
	(*argvp)--;
}


zerof()
{

	if (sflag && bflag && mflag)
		sflag = bflag = mflag = 0;
}

lookup(cp)
	register char *cp;
{
	register char *dp;

	for (dp = cp; *dp; dp++)
		continue;
	for (; dp > cp; dp--) {
		if (*dp == '.') {
			*dp = 0;
			break;
		}
	}
	for (dp = cp; *dp; dp++)
		if (*dp == '/')
			cp = dp + 1;
	if (uflag) {
		print = 0;
		count = 0;
	} else
		print = 1;
again:
	if (print)
		printf("%s:", cp);
	if (sflag) {
		looksrc(cp);
		if (uflag && print == 0 && count != 1) {
			print = 1;
			goto again;
		}
	}
	count = 0;
	if (bflag) {
		lookbin(cp);
		if (uflag && print == 0 && count != 1) {
			print = 1;
			goto again;
		}
	}
	count = 0;
	if (mflag) {
		lookman(cp);
		if (uflag && print == 0 && count != 1) {
			print = 1;
			goto again;
		}
	}
	if (print)
		printf("\n");
}

looksrc(cp)
	char *cp;
{
	if (Sflag == 0) {
		find(srcdirs, cp);
	} else
		findv(Sflag, Scnt, cp);
}

lookbin(cp)
	char *cp;
{
	if (Bflag == 0)
		find(bindirs, cp);
	else
		findv(Bflag, Bcnt, cp);
}

lookman(cp)
	char cp[];
{
	char truncatedname[10];
	int  i;

	for (i=0; (cp[i] != '\0') && (i < 9); i++) {
		if (isupper(cp[i]))
			truncatedname[i] = tolower(cp[i]);
		else
			truncatedname[i] = cp[i];
	}
	truncatedname[i] = '\0';

	if (Mflag == 0) {
		find(mandirs, truncatedname);
	} else
		findv(Mflag, Mcnt, truncatedname);
}

findv(dirv, dirc, cp)
	char **dirv;
	int dirc;
	char *cp;
{

	while (dirc > 0)
		findin(*dirv++, cp), dirc--;
}

find(dirs, cp)
	char **dirs;
	char *cp;
{

	while (*dirs)
		findin(*dirs++, cp);
}

findin(dir, cp)
	char *dir, *cp;
{
	register DIR *d;
	register struct dirent *ep;

	d = opendir(dir);
	if (d == NULL)
		return;
	while ((ep = readdir(d)) != NULL) {
		if (ep->d_ino == 0)
			continue;
		if (itsit(cp, ep->d_name)) {
			count++;
			if (print)
				printf(" %s/%s", dir, ep->d_name);
		}
	}
	closedir(d);
}

itsit(cp, dp)
	register char *cp, *dp;
{
	register int i = 14;

	if (dp[0] == 's' && dp[1] == '.' && itsit(cp, dp+2))
		return (1);
	while (*cp && *dp && *cp == *dp)
		cp++, dp++, i--;
	if (*cp == 0 && *dp == 0)
		return (1);
	while (isdigit(*dp))
		dp++;
	if (*cp == 0 && *dp++ == '.') {
		--i;
		while (i > 0 && *dp)
			if (--i, *dp++ == '.')
				return (*dp++ == 'C' && *dp++ == 0);
		return (1);
	}
	return (0);
}
