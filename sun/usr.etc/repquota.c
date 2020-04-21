#ifndef lint
/* @(#)repquota.c	2.1 86/04/17 NFSSRC */
static	char *sccsid = "@(#)repquota.c 1.1 86/02/05 SMI"; /* from UCB 4.2 */
#endif

/*
 * Quota report
 */
#include <stdio.h>
#include <sys/param.h>
#include <ufs/quota.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <mntent.h>
#include <pwd.h>

struct passwd	*getpwent();
char		*malloc();

int	vflag;		/* verbose */
int	aflag;		/* all file systems */
char *listbuf[50];

#define QFNAME "quotas"

main(argc, argv)
	int argc;
	char **argv;
{
	register struct mntent *mntp;
	char **listp;
	int listcnt;
	char quotafile[MAXPATHLEN + 1];
	FILE *mtab, *fstab;
	int i, errs = 0;

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
			"repquota [-v] -a",
			"repquota [-v] filesys ...");
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
		endmntent(mtab);
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
			errs += repquota(mntp->mnt_fsname, quotafile);
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

repquota(fsdev, qffile)
	char *fsdev;
	char *qffile;
{
	FILE *qf;
	u_short uid;
	struct dqblk dqbuf;
	struct stat statb;

	fprintf(stdout, "*** Quota report for %s\n", fsdev);
	qf = fopen(qffile, "r");
	if (qf == NULL) {
		perror(qffile);
		return (1);
	}
	if (fstat(fileno(qf), &statb) < 0) {
		perror(qffile);
		return (1);
	}
	quotactl(Q_SYNC, fsdev, 0, NULL);
	header();
	uid = getuid();
	if (uid) {
		fseek(qf, (long)dqoff(uid), 0);
		if (fread(&dqbuf, sizeof(struct dqblk), 1, qf) != 1)
			return (0);
		if (!vflag &&
		    dqbuf.dqb_curfiles == 0 && dqbuf.dqb_curblocks == 0)
			return (0);
		prquota(uid, &dqbuf);
		return (0);
	}
	for (uid = 0; ; uid++) {
		fread(&dqbuf, sizeof(struct dqblk), 1, qf);
		if (feof(qf))
			break;
		if (!vflag &&
		    dqbuf.dqb_curfiles == 0 && dqbuf.dqb_curblocks == 0)
			continue;
		prquota(uid, &dqbuf);
	}
	return (0);
}

header()
{

	printf(
"                      Block limits                      File limits\n"
	);
	printf(
"User           used   soft   hard    timeleft    used   soft   hard    timeleft\n"
	);
}

prquota(uid, dqp)
	u_short uid;
	struct dqblk *dqp;
{
	struct passwd *pwp;
	struct timeval tv;
	char ftimeleft[80], btimeleft[80];

	if (dqp->dqb_bsoftlimit == 0 && dqp->dqb_bhardlimit == 0 &&
	    dqp->dqb_fsoftlimit == 0 && dqp->dqb_fhardlimit == 0)
		return;
	gettimeofday(&tv, NULL);
	pwp = getpwuid(uid);
	if (pwp != NULL)
		printf("%-10s", pwp->pw_name);
	else
		printf("#%-9d", uid);
	if (dqp->dqb_bsoftlimit && dqp->dqb_curblocks>=dqp->dqb_bsoftlimit) {
		if (dqp->dqb_btimelimit == 0) {
			strcpy(btimeleft, "NOT STARTED");
		} else if (dqp->dqb_btimelimit > tv.tv_sec) {
			fmttime(btimeleft,
			    (long)dqp->dqb_btimelimit - tv.tv_sec);
		} else {
			strcpy(btimeleft, "EXPIRED");
		}
	} else {
		btimeleft[0] = '\0';
	}
	if (dqp->dqb_fsoftlimit && dqp->dqb_curfiles>=dqp->dqb_fsoftlimit) {
		if (dqp->dqb_ftimelimit == 0) {
			strcpy(ftimeleft, "NOT STARTED");
		} else if (dqp->dqb_ftimelimit > tv.tv_sec) {
			fmttime(ftimeleft,
			    (long)dqp->dqb_ftimelimit - tv.tv_sec);
		} else {
			strcpy(ftimeleft, "EXPIRED");
		}
	} else {
		ftimeleft[0] = '\0';
	}
	printf("%c%c%7d%7d%7d%12s %7d%7d%7d%12s\n",
		dqp->dqb_bsoftlimit && 
		    dqp->dqb_curblocks >= 
		    dqp->dqb_bsoftlimit ? '+' : '-',
		dqp->dqb_fsoftlimit &&
		    dqp->dqb_curfiles >=
		    dqp->dqb_fsoftlimit ? '+' : '-',
		dqp->dqb_curblocks / btodb(1024),
		dqp->dqb_bsoftlimit / btodb(1024),
		dqp->dqb_bhardlimit / btodb(1024),
		btimeleft,
		dqp->dqb_curfiles,
		dqp->dqb_fsoftlimit,
		dqp->dqb_fhardlimit,
		ftimeleft
	);
}

fmttime(buf, time)
	char *buf;
	register long time;
{
	int i;
	static struct {
		int c_secs;		/* conversion units in secs */
		char * c_str;		/* unit string */
	} cunits [] = {
		{60*60*24*28, "months"},
		{60*60*24*7, "weeks"},
		{60*60*24, "days"},
		{60*60, "hours"},
		{60, "mins"},
		{1, "secs"}
	};

	if (time <= 0) {
		strcpy(buf, "EXPIRED");
		return;
	}
	for (i = 0; i < sizeof(cunits)/sizeof(cunits[0]); i++) {
		if (time >= cunits[i].c_secs)
			break;
	}
	sprintf(buf, "%.1f %s", (double)time/cunits[i].c_secs, cunits[i].c_str);
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
