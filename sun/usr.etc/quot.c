#ifndef lint
/* @(#)quot.c	2.1 86/04/17 NFSSRC */ 
static	char *sccsid = "@(#)quot.c 1.1 86/02/05 SMI"; /* from UCB 4.9 83/09/22 */
#endif

/*
 * quot
 */

#include <stdio.h>
#include <ctype.h>
#include <mntent.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/vnode.h>
#include <ufs/inode.h>
#include <ufs/fs.h>
#include <sys/file.h>
#include <sys/stat.h>

#define	ISIZ	(MAXBSIZE/sizeof(struct dinode))
union {
	struct fs u_sblock;
	char dummy[SBSIZE];
} sb_un;
#define sblock sb_un.u_sblock
struct dinode itab[MAXBSIZE/sizeof(struct dinode)];

struct du {
	struct	du *next;
	long	blocks;
	long	blocks30;
	long	blocks60;
	long	blocks90;
	long	nfiles;
	int	uid;
#define	NDU	2048
} du[NDU];
int	ndu;
#define	DUHASH	8209	/* smallest prime >= 4 * NDU */
#define	HASH(u)	((u) % DUHASH)
struct	du *duhash[DUHASH];

#define	TSIZE	500
int	sizes[TSIZE];
long	overflow;

int	nflg;
int	fflg;
int	cflg;
int	vflg;
int	hflg;
int	aflg;
long	now;

unsigned	ino;

char	*malloc();
char	*getname();

main(argc, argv)
	int argc;
	char *argv[];
{
	register int n;

	now = time(0);
	argc--, argv++;
	while (argc > 0 && argv[0][0] == '-') {
		register char *cp;

		for (cp = &argv[0][1]; *cp; cp++)
			switch (*cp) {
			case 'n':
				nflg++; break;
			case 'f':
				fflg++; break;
			case 'c':
				cflg++; break;
			case 'v':
				vflg++; break;
			case 'h':
				hflg++; break;
			case 'a':
				aflg++; break;
			default:
				fprintf(stderr,
				    "usage: quot [-nfcvha] [filesystem ...]\n");
				exit(1);
			}
		argc--, argv++;
	}
	if (aflg) {
		quotall();
	}
	while (argc-- > 0) {
		if (getdev(argv) == 0 && check(*argv) == 0)
			report();
		argv++;
	}
	exit (0);
}

quotall()
{
	FILE *fstab;
	register struct mntent *mntp;
	register char *cp;
	char dev[80], *rindex();

	fstab = setmntent(MNTTAB, "r");
	if (fstab == NULL) {
		fprintf(stderr, "quot: no %s file\n", MNTTAB);
		exit(1);
	}
	while (mntp = getmntent(fstab)) {
		if  (strcmp(mntp->mnt_type, MNTTYPE_42) != 0)
			continue;
		cp = rindex(mntp->mnt_fsname, '/');
		if (cp == 0)
			continue;
		sprintf(dev, "/dev/r%s", cp + 1);
		if (check(dev) == 0)
			report();
	}
	endmntent(fstab);
}

check(file)
	char *file;
{
	register int i, j, nfiles;
	register struct du **dp;
	daddr_t iblk;
	int c, fd;

	/*
	 * Initialize tables between checks;
	 * because of the qsort done in report()
	 * the hash tables must be rebuilt each time.
	 */
	for (i = 0; i < TSIZE; i++)
		sizes[i] = 0;
	overflow = 0;
	for (dp = duhash; dp < &duhash[DUHASH]; dp++)
		*dp = 0;
	ndu = 0;
	fd = open(file, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "quot: ");
		perror(file);
		return (-1);
	}
	printf("%s:\n", file);
	sync();
	bread(fd, SBLOCK, (char *)&sblock, SBSIZE);
	if (nflg) {
		if (isdigit(c = getchar()))
			ungetc(c, stdin);
		else while (c != '\n' && c != EOF)
			c = getchar();
	}
	nfiles = sblock.fs_ipg * sblock.fs_ncg;
	for (ino = 0; ino < nfiles; ) {
		iblk = fsbtodb(&sblock, itod(&sblock, ino));
		bread(fd, iblk, (char *)itab, sblock.fs_bsize);
		for (j = 0; j < INOPB(&sblock) && ino < nfiles; j++, ino++) {
			if (ino < ROOTINO)
				continue;
			acct(&itab[j]);
		}
	}
	close(fd);
}

acct(ip)
	register struct dinode *ip;
{
	register struct du *dp;
	struct du **hp;
	long blks, frags, size;
	char n;
	static fino;

	if ((ip->di_mode & IFMT) == 0)
		return;
	/*
	 * By default, take block count in inode.  Otherwise (-h),
	 * take the size field and estimate the blocks allocated.
	 * The latter does not account for holes in files.
	 */
	if (!hflg)
		size = ip->di_blocks / 2;
	else {
		blks = lblkno(&sblock, ip->di_size);
		frags = blks * sblock.fs_frag +
			numfrags(&sblock, dblksize(&sblock, ip, blks));
		size = frags * sblock.fs_fsize / 1024;
	}
	if (cflg) {
		if ((ip->di_mode&IFMT) != IFDIR && (ip->di_mode&IFMT) != IFREG)
			return;
		if (size >= TSIZE) {
			overflow += size;
			size = TSIZE-1;
		}
		sizes[size]++;
		return;
	}
	hp = &duhash[HASH(ip->di_uid)];
	for (dp = *hp; dp; dp = dp->next)
		if (dp->uid == ip->di_uid)
			break;
	if (dp == 0) {
		if (ndu >= NDU)
			return;
		dp = &du[ndu++];
		dp->next = *hp;
		*hp = dp;
		dp->uid = ip->di_uid;
		dp->nfiles = 0;
		dp->blocks = 0;
		dp->blocks30 = 0;
		dp->blocks60 = 0;
		dp->blocks90 = 0;
	}
	dp->blocks += size;
#define	DAY (60 * 60 * 24)	/* seconds per day */
	if (now - ip->di_atime > 30 * DAY)
		dp->blocks30 += size;
	if (now - ip->di_atime > 60 * DAY)
		dp->blocks60 += size;
	if (now - ip->di_atime > 90 * DAY)
		dp->blocks90 += size;
	dp->nfiles++;
	while (nflg) {
		register char *np;

		if (fino == 0)
			if (scanf("%d", &fino) <= 0)
				return;
		if (fino > ino)
			return;
		if (fino < ino) {
			while ((n = getchar()) != '\n' && n != EOF)
				;
			fino = 0;
			continue;
		}
		if (np = getname(dp->uid))
			printf("%.7s	", np);
		else
			printf("%d	", ip->di_uid);
		while ((n = getchar()) == ' ' || n == '\t')
			;
		putchar(n);
		while (n != EOF && n != '\n') {
			n = getchar();
			putchar(n);
		}
		fino = 0;
		break;
	}
}

bread(fd, bno, buf, cnt)
	unsigned bno;
	char *buf;
{

	lseek(fd, (long)bno * DEV_BSIZE, L_SET);
	if (read(fd, buf, cnt) != cnt) {
		fprintf(stderr, "quot: read error at block %u\n", bno);
		exit(1);
	}
}

qcmp(p1, p2)
	register struct du *p1, *p2;
{
	char *s1, *s2;

	if (p1->blocks > p2->blocks)
		return (-1);
	if (p1->blocks < p2->blocks)
		return (1);
	s1 = getname(p1->uid);
	if (s1 == 0)
		return (0);
	s2 = getname(p2->uid);
	if (s2 == 0)
		return (0);
	return (strcmp(s1, s2));
}

report()
{
	register i;
	register struct du *dp;

	if (nflg)
		return;
	if (cflg) {
		register long t = 0;

		for (i = 0; i < TSIZE - 1; i++)
			if (sizes[i]) {
				t += i*sizes[i];
				printf("%d	%d	%D\n", i, sizes[i], t);
			}
		printf("%d	%d	%D\n",
		    TSIZE - 1, sizes[TSIZE - 1], overflow + t);
		return;
	}
	qsort(du, ndu, sizeof (du[0]), qcmp);
	for (dp = du; dp < &du[ndu]; dp++) {
		register char *cp;

		if (dp->blocks == 0)
			return;
		printf("%5D\t", dp->blocks);
		if (fflg)
			printf("%5D\t", dp->nfiles);
		if (cp = getname(dp->uid))
			printf("%-8.8s", cp);
		else
			printf("#%-8d", dp->uid);
		if (vflg)
			printf("\t%5D\t%5D\t%5D",
			    dp->blocks30, dp->blocks60, dp->blocks90);
		printf("\n");
	}
}

#include <pwd.h>
#include <utmp.h>

struct	utmp utmp;

#define NUID	2048
#define	NMAX	(sizeof (utmp.ut_name))

char	names[NUID][NMAX+1];
char	outrangename[NMAX+1];
int	outrangeuid = -1;

char *
getname(uid)
	int uid;
{
	register struct passwd *pw;
	static init;
	struct passwd *getpwent();

	if (uid >= 0 && uid < NUID && names[uid][0])
		return (&names[uid][0]);
	if (uid >= 0 && uid == outrangeuid)
		return (outrangename);
rescan:
	if (init == 2) {
		if (uid < NUID)
			return (0);
		setpwent();
		while (pw = getpwent()) {
			if (pw->pw_uid != uid)
				continue;
			outrangeuid = pw->pw_uid;
			strncpy(outrangename, pw->pw_name, NMAX);
			endpwent();
			return (outrangename);
		}
		endpwent();
		return (0);
	}
	if (init == 0)
		setpwent(), init = 1;
	while (pw = getpwent()) {
		if (pw->pw_uid < 0 || pw->pw_uid >= NUID) {
			if (pw->pw_uid == uid) {
				outrangeuid = pw->pw_uid;
				strncpy(outrangename, pw->pw_name, NMAX);
				return (outrangename);
			}
			continue;
		}
		if (names[pw->pw_uid][0])
			continue;
		strncpy(names[pw->pw_uid], pw->pw_name, NMAX);
		if (pw->pw_uid == uid)
			return (&names[uid][0]);
	}
	init = 2;
	goto rescan;
}

getdev(devpp)
	char **devpp;
{
	struct stat statb;
	FILE *fstab;
	struct mntent *mntp;

	if (stat(*devpp, &statb) < 0) {
		perror(*devpp);
		return (-1);
	}
	if ((statb.st_mode & S_IFMT) == S_IFBLK ||
	    (statb.st_mode & S_IFMT) == S_IFCHR) 
		return (0);
	fstab = setmntent(MNTTAB, "r");
	if (fstab == NULL) {
		fprintf(stderr, "quot: no %s file\n", MNTTAB);
		exit(1);
	}
	while (mntp = getmntent(fstab)) {
		if (strcmp(mntp->mnt_dir, *devpp) == 0) {
			if (strcmp(mntp->mnt_type, MNTTYPE_42) != 0) {
				fprintf(stderr,
				    "quot: %s not 4.2 filesystem\n",
				    *devpp);
				return (-1);
			}
			*devpp = malloc(strlen(mntp->mnt_fsname) + 1);
			strcpy(*devpp, mntp->mnt_fsname);
			endmntent(fstab);
			return (0);
		}
	}
	endmntent(fstab);
	return (-1);
}
