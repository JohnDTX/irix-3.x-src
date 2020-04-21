#ifndef lint
/* @(#)quotacheck.c	2.1 86/04/17 NFSSRC */
static	char *sccsid = "@(#)quotacheck.c 1.1 86/02/05 SMI; from UCB 4.4 ";
#endif

/*
 * Fix up / report on disc quotas & usage
 */
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/vnode.h>
#include <ufs/inode.h>
#include <ufs/fs.h>
#include <ufs/quota.h>
#include <sys/stat.h>
#include <mntent.h>
#include <pwd.h>

union {
	struct	fs	sblk;
	char	dummy[MAXBSIZE];
} un;
#define	sblock	un.sblk

#define	ITABSZ	256
struct	dinode	itab[ITABSZ];
struct	dinode	*dp;

struct fileusage {
	struct fileusage *fu_next;
	u_long fu_curfiles;
	u_long fu_curblocks;
	u_short	fu_uid;
};
#define FUHASH 997
struct fileusage *fuhead[FUHASH];
struct fileusage *lookup();
struct fileusage *adduid();
int highuid;

int fi;
ino_t ino;
struct	passwd	*getpwent();
struct	dinode	*ginode();
char *malloc(), *makerawname();

int	vflag;		/* verbose */
int	aflag;		/* all file systems */
char *listbuf[50];

#define QFNAME "quotas"
struct dqblk zerodqbuf;

main(argc, argv)
	int argc;
	char **argv;
{
	register struct mntent *mntp;
	char **listp;
	int listcnt;
	char quotafile[MAXPATHLEN + 1];
	FILE *mtab, *fstab;
	int i;
	int errs = 0;

again:
	argc--, argv++;
	if (argc > 0 && strcmp(*argv, "-v") == 0) {
		vflag++;
		goto again;
	}
	if (argc > 0 && strcmp(*argv, "-a") == 0) {
		aflag++;
		goto again;
	}
	if (argc <= 0 && !aflag) {
		fprintf(stderr, "Usage:\n\t%s\n\t%s\n",
			"quotacheck [-v] -a",
			"quotacheck [-v] filesys ...");
		exit(1);
	}
	/*
	 * If aflag go through fstab and make a list of appropriate
	 * filesystems.
	 */
	if (aflag) {
		listp = listbuf;
		listcnt = 0;
		fstab = setmntent(MNTTAB, "r");
		while (mntp = getmntent(fstab)) {
			if (strcmp(mntp->mnt_type, MNTTYPE_42) != 0 ||
			    !hasmntopt(mntp, MNTOPT_QUOTA) ||
			    hasmntopt(mntp, MNTOPT_RO))
				continue;
			*listp = malloc(strlen(mntp->mnt_fsname) + 1);
			strcpy(*listp, mntp->mnt_fsname);
			listp++;
			listcnt++;
		}
		endmntent(fstab);
		*listp = (char *)0;
		listp = listbuf;
	} else {
		listp = argv;
		listcnt = argc;
	}
	mtab = setmntent(MOUNTED, "r");
	while (mntp = getmntent(mtab)) {
		if (strcmp(mntp->mnt_type, MNTTYPE_42) == 0 &&
		    !hasmntopt(mntp, MNTOPT_RO) &&
		    (oneof(mntp->mnt_fsname, listp, listcnt) ||
		     oneof(mntp->mnt_dir, listp, listcnt)) ) {
			sprintf(quotafile, "%s/%s", mntp->mnt_dir, QFNAME);
			errs += chkquota(mntp->mnt_fsname, quotafile);
		}
	}
	endmntent(mtab);
	while (listcnt--) {
		if (*listp) {
			fprintf(stderr, "%s not found in %s\n",
			    *listp, MOUNTED);
		}
	}
	exit(errs);
}

chkquota(fsdev, qffile)
	char *fsdev;
	char *qffile;
{
	register struct fileusage *fup;
	dev_t quotadev;
	FILE *qf;
	u_short uid;
	int cg, i;
	char *rawdisk;
	struct passwd *pwp;
	struct stat statb;
	struct dqblk dqbuf;

	rawdisk = makerawname(fsdev);
	if (vflag)
		printf("*** Check quotas for %s\n", rawdisk);
	fi = open(rawdisk, 0);
	if (fi < 0) {
		perror(rawdisk);
		return (1);
	}
	qf = fopen(qffile, "r+");
	if (qf == NULL) {
		perror(qffile);
		return (1);
	}
	if (fstat(fileno(qf), &statb) < 0) {
		perror(qffile);
		return (1);
	}
	quotadev = statb.st_dev;
	if (stat(fsdev, &statb) < 0) {
		perror(fsdev);
		return (1);
	}
	if (quotadev != statb.st_rdev) {
		fprintf(stderr, "%s dev (0x%x) mismatch %s dev (0x%x)\n",
			qffile, quotadev, fsdev, statb.st_rdev);
		return (1);
	}
	quotactl(Q_SYNC, fsdev, 0, NULL);
	sync();
	bread(SBLOCK, (char *)&sblock, SBSIZE);
	ino = 0;
	for (cg = 0; cg < sblock.fs_ncg; cg++) {
		dp = NULL;
		for (i = 0; i < sblock.fs_ipg; i++)
			acct(ginode());
	}
	for (uid = 0; uid <= highuid; uid++) {
		fup = lookup(uid);
		if (fup == 0)
			continue;
		fseek(qf, (long)dqoff(uid), 0);
		i = fread(&dqbuf, sizeof(struct dqblk), 1, qf);
		if (i == 0)
			dqbuf = zerodqbuf;
		if (dqbuf.dqb_curfiles == fup->fu_curfiles &&
		    dqbuf.dqb_curblocks == fup->fu_curblocks) {
			if (vflag) {
				pwp = getpwuid(uid);
				if (pwp != NULL)
					printf("%-16s:", pwp->pw_name);
				else
					printf("#%-15d:", uid);
				printf(" inodes (%d) blocks (%d)\n",
				    fup->fu_curfiles, fup->fu_curblocks);
			}
			fup->fu_curfiles = 0;
			fup->fu_curblocks = 0;
			continue;
		}
		pwp = getpwuid(uid);
		if (pwp != NULL)
			printf("%-10s fixed:", pwp->pw_name);
		else
			printf("#%-9d fixed:", uid);
		printf(" inodes (old %d, new %d)",
		    dqbuf.dqb_curfiles, fup->fu_curfiles);
		printf(" blocks (old %d, new %d)\n",
		    dqbuf.dqb_curblocks, fup->fu_curblocks);
		dqbuf.dqb_curfiles = fup->fu_curfiles;
		dqbuf.dqb_curblocks = fup->fu_curblocks;
		fseek(qf, (long)dqoff(uid), 0);
		fwrite(&dqbuf, sizeof(struct dqblk), 1, qf);
		quotactl(Q_SETQUOTA, fsdev, uid, &dqbuf);
		fup->fu_curfiles = 0;
		fup->fu_curblocks = 0;
	}
	return (0);
}

acct(ip)
	register struct dinode *ip;
{
	register struct fileusage *fup;

	if (ip == NULL)
		return;
	if (ip->di_mode == 0)
		return;
	fup = lookup(ip->di_uid);
	if (fup == 0)
		fup = adduid(ip->di_uid);
	fup->fu_curfiles++;
	if ((ip->di_mode & IFMT) == IFCHR || (ip->di_mode & IFMT) == IFBLK)
		return;
	fup->fu_curblocks += ip->di_blocks;
}

oneof(target, listp, n)
	char *target;
	register char **listp;
	register int n;
{

	while (n--) {
		if (*listp && strcmp(target, *listp) == 0) {
			*listp = (char *)0;
			return (1);
		}
		listp++;
	}
	return (0);
}

struct dinode *
ginode()
{
	register unsigned long iblk;

	if (dp == NULL || ++dp >= &itab[ITABSZ]) {
		iblk = itod(&sblock, ino);
		bread(fsbtodb(&sblock, iblk), (char *)itab, sizeof itab);
		dp = &itab[ino % INOPB(&sblock)];
	}
	if (ino++ < ROOTINO)
		return(NULL);
	return(dp);
}

bread(bno, buf, cnt)
	long unsigned bno;
	char *buf;
{

	lseek(fi, (long)dbtob(bno), 0);
	if (read(fi, buf, cnt) != cnt) {
		printf("read error %u\n", bno);
		exit(1);
	}
}

struct fileusage *
lookup(uid)
	u_short uid;
{
	register struct fileusage *fup;

	for (fup = fuhead[uid % FUHASH]; fup != 0; fup = fup->fu_next)
		if (fup->fu_uid == uid)
			return (fup);
	return ((struct fileusage *)0);
}

struct fileusage *
adduid(uid)
	u_short uid;
{
	struct fileusage *fup, **fhp;

	fup = lookup(uid);
	if (fup != 0)
		return (fup);
	fup = (struct fileusage *)calloc(1, sizeof(struct fileusage));
	if (fup == 0) {
		fprintf(stderr, "out of memory for fileusage structures\n");
		exit(1);
	}
	fhp = &fuhead[uid % FUHASH];
	fup->fu_next = *fhp;
	*fhp = fup;
	fup->fu_uid = uid;
	if (uid > highuid)
		highuid = uid;
	return (fup);
}

char *
makerawname(name)
	char *name;
{
	register char *cp;
	char tmp, ch, *rindex();
	static char rawname[MAXPATHLEN];

	strcpy(rawname, name);
	cp = rindex(rawname, '/') + 1;
	if (cp == (char *)1 || *cp == 'r')
		return (name);
	for (ch = 'r'; *cp != '\0'; ) {
		tmp = *cp;
		*cp++ = ch;
		ch = tmp;
	}
	*cp++ = ch;
	*cp = '\0';
	return (rawname);
}
