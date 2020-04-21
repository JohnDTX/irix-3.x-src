#ifdef	lint
char _Origin_[] = "UC Berkeley";
#endif
	char	*sccsid = "@(#)ls.c	2.4";
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/bin/RCS/ls.c,v 1.1 89/03/27 14:50:40 root Exp $";
/*
 * $Log:	ls.c,v $
 * Revision 1.1  89/03/27  14:50:40  root
 * Initial check-in for 3.7
 * 
 * Revision 1.14  87/11/17  17:41:31  vjs
 * hack to make ls work with long i-numbers
 * 
 * Revision 1.13  86/08/27  21:02:45  paulm
 * Change name of 'select' routine (conflicts with BSD system call).
 * 
 * Revision 1.12  86/07/23  21:40:34  paulm
 * Use opendir/readdir instead of reading directories as files.
 * 
 * Revision 1.1  86/06/07  16:28:03  paulm
 * Initial revision
 * 
 * Revision 1.11  85/11/21  10:33:06  jym
 * Allow printing of inode nums greater than 32k.
 * 
 * Revision 1.10  85/10/07  15:10:36  root
 * Fixed so "%s not found\n" is printed on stderr, not stdout.
 * This change conforms with V.2, 4.2, and general principles.  It also
 * allows one to say, e.g.
 * 	tar `ls name1 name2* name[3-9] 2>/dev/null`
 * in the Bourne shell.  See SCR 1096.
 * 
 * Revision 1.9  85/06/04  16:58:35  bob
 * Installed Donovan's latest symlink stuff.
 * 
 */
/*	ls	COMPILE:	cc -O ls.c -s -i -o ls clstat.o	*/

/* Copyright (c) 1979 Regents of the University of California */
/*
 * ls - list file or directory
 *
 * Modified by Bill Joy UCB May/August 1977
 * Modified by Dave Presotto BTL Feb/80
 * Modified by Bob Toxen SGI March 1984: added named pipes & group listing, etc.
 * Modified by D. Fong SGI April 1985: for symbolic links and vm system.
 *
 * This version of ls is designed for graphic terminals and to
 * list directories with lots of files in them compactly.
 * It supports three variants for listings:
 *
 *	1) Columnar output.
 *	2) Stream output.
 *	3) Old one per line format.
 *
 * Columnar output is the default.
 * If, however, the standard output is not a teletype, the default
 * is one-per-line.
 *
 * With columnar output, the items are sorted down the columns.
 * We use columns only for a directory we are interpreting.
 * Thus, in particular, we do not use columns for
 *
 *	ls /usr/bin/p*
 *
 * This version of ls also prints non-printing characters as '?' if
 * the standard output is a teletype.
 *
 * Flags relating to these and other new features are:
 *
 *	-m	force stream output.
 *
 *	-1	force one entry per line, e.g. to a teletype
 *
 *	-q	force non-printings to be '?'s, e.g. to a file
 *
 *	-C	force columnar output, e.g. into a file
 *
 *	-n	like -l, but user/group id's in decimal rather than
 *	looking in /etc/passwd to save time
 *
 *	-F	turns on the "flagging" of executables and directories
 *
 *	-R	causes ls to recurse through the branches of the subtree
 *	ala find
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <dirent.h>
#include <termio.h>
#include <stdio.h>
#include <ctype.h>

# ifdef S_IFLNK
extern int clstat();
# else  S_IFLNK
#define	clstat	stat
# endif S_IFLNK

#define	MAXIDSTRING 8
#define	MAXFILEWIDTH 14	/* XXX */
#define	NFILES	1024
#define	DIRECT	10	/* Number of direct blocks */

struct lbuf {
	char	*lnamep;
	char	*linktext;
	char	ltype;
	long	lnum;
	short	lflags;
	short	lnl;
	short	luid;
	short	lgid;
	long	lsize;
	long	lmtime;
};

struct dchain {
	char *dc_name;		/* the path name */
	struct dchain *dc_next; /* the next directory on the chain */
};

struct dchain *dfirst;		/* the start of the directory chain */
struct dchain *dlast;		/* end of the directory chain */
struct dchain *dtemp;		/* temporary used when linking */
char *curdir;			/* the current directory */

int	aflg, bflg, dflg, lflg, sflg, tflg, uflg, iflg, fflg, gflg=1, cflg;
int	Aflg, nflg, qflg, Fflg, Rflg, across, Cflg, Hflg, Lflg;
int	oflg=1;			/* SGI: print owner in long listing */
int	pflg;			/* SGI: System V: half-a..ed -F */
int	Tflg;			/* SGI: include seconds in time */
int	nopad;
int	tabflg;
int	rflg	= 1;
long	year;
long	now;
int	flags;
int	lastuid = -1;
long	tblocks;
int	statreq;
int	xtraent;		/* for those switches which print out a total */
struct	lbuf	*flist[NFILES];
struct	lbuf	**lastp = flist;
struct	lbuf	**firstp = flist;
char	*dotp	= ".";

char	*makename();
char	*lsmalloc();
struct	lbuf *gstat();
char	*ctime();
long	nblock();

#define	ISARG	0100000
int	colwidth;
int	filewidth;
int	fixedwidth;
int	outcol;
char	obuf[BUFSIZ];


main(argc, argv)
int argc;
char *argv[];
{
	int i, width;
	register struct lbuf *ep;
	register struct lbuf **slastp;
	struct lbuf **epp;
	struct lbuf lb;
	char *t;
	char *cp;
	struct termio ttymode;
	int compar();

	setbuf(stdout, obuf);
	tabflg = 0;
	Aflg = getuid() == 0;
	lb.lmtime = time((long *) 0);
	lb.linktext = 0;
	year = lb.lmtime - 6L*30L*24L*60L*60L; /* 6 months ago */
	now = lb.lmtime + 59L*60L; /* current time (+59 minutes fudge) */
	qflg = (ioctl(1, TCGETA, &ttymode) != -1);

	/* guarantee at least on column width */
	fixedwidth = 1;

	/*
	 * If the standard output is not a teletype,
	 * then we default to one-per-line format
	 * otherwise decide between stream and
	 * columnar based on our name.
	 */
	if (qflg) {
		Cflg = 1;
		if ((ttymode.c_oflag & TABDLY) != TAB3)
			tabflg++;
		for (cp = argv[0]; cp[0] && cp[1]; cp++)
			continue;
		/*
		 * Name ends in l => stream
		 */
		if (cp[0] == 'l')
			nopad = 1, Cflg = 0;
		/*
		 * ... if ends in x ==> columns sorted across
		 */
		else if (cp[0] == 'x')
			across = 1;
		/*
		 * ... if ends in f ==> show directories and executable files
		 */
		else if (cp[0] == 'f') {
			Fflg = 1;
		}
	}
	while (--argc > 0 && *argv[1] == '-') {
		argv++;
		while (*++*argv) switch (**argv) {
		/*
		 * C - force columnar output
		 */
		case 'C':
			Cflg = 1;
			nopad = 0;
			continue;
		/*
		 * m - force stream output
		 */
		case 'm':
			Cflg = 0;
			nopad = 1;
			continue;
		/*
		 * x - force sort across
		 */
		case 'x':
			across = 1;
			nopad = 0;
			Cflg = 1;
			continue;
		/*
		 * q - force ?'s in output
		 */
		case 'q':
			qflg = 1;
			bflg = 0;
			continue;
		/*
		 * b - force octal value in output
		 */
		case 'b':
			bflg = 1;
			qflg = 0;
			continue;
		/*
		 * 1 - force 1/line in output
		 */
		case '1':
			Cflg = 0;
			nopad = 0;
			continue;
		/* STANDARD FLAGS */
		case 'a':
			aflg++;
			continue;

		case 'A':
			Aflg = !Aflg;
			continue;

		case 'c':
			cflg++;
			continue;

		case 's':
			fixedwidth += 5;
			sflg++;
			statreq++;
			xtraent++;
			continue;

		case 'd':
			dflg++;
			continue;

		/*
		 * n - don't look in password file
		 */
		case 'n':
			nflg++;
		case 'l':
			lflg++;
			statreq++;
			xtraent++;
			continue;

		case 'r':
			rflg = -1;
			continue;

		case 't':
			tflg++;
			statreq++;
			continue;

		case 'u':
			uflg++;
			continue;

		case 'i':
			fixedwidth += 5;
			iflg++;
			continue;

		case 'f':
			fflg++;
			continue;

		case 'o':
			gflg=0;		/* System V: don't list group */
			continue;

		case 'g':
			oflg=0;		/* System V: don't list owner */
			continue;

		case 'F':
			Fflg++;
			continue;

		case 'R':
			Rflg++;
			continue;

		case 'p':
			pflg++;		/* SGI: System V: half-a..ed -F */
			continue;

		case 'T':
			Tflg++;		/* SGI: include seconds in date */
			continue;

#ifdef	S_IFLNK
		case 'H':
			Hflg++;
			Lflg = 0;
			continue;

		case 'L':
			Lflg++;
			Hflg = 0;
			continue;
#endif	S_IFLNK

		default:
#ifdef	S_IFLNK
			fprintf(stderr,
			  "usage: ls [-1ACFHLRTabcdfgilmnopqrstux] [files]\n");
#else	S_IFLNK
			fprintf(stderr,
			  "usage: ls [-1ACFRTabcdfgilmnopqrstux] [files]\n");
#endif	S_IFLNK
			exit(1);
		}
	}
	if (Fflg || pflg)
		fixedwidth++;
	if (fflg) {
		aflg++;
		lflg = 0;
		sflg = 0;
		tflg = 0;
		statreq = 0;
		xtraent = 0;
	}
	if(lflg) {
		Cflg = 0;
		nopad = 0;
		fixedwidth = 78-8*(oflg+gflg);
	}
	if (Tflg)
		fixedwidth += 3;
	if (argc==0) {
		argc++;
		argv = &dotp - 1;
	}
	for (i=0; i < argc; i++) {
		argv++;
		if (Cflg) {
			width = strlen (*argv);
			if (width > filewidth)
				filewidth = width;
		}
		if ((ep = gstat(*argv, 1))==NULL)
			continue;
		ep->lnamep = *argv;
		ep->lflags |= ISARG;
	}
	if (!Cflg)
		filewidth = MAXFILEWIDTH;
	else
		colwidth = fixedwidth + filewidth;
	qsort(firstp, lastp - firstp, sizeof *lastp, compar);
	slastp = lastp;
	for (epp=firstp; epp<slastp; epp++) {
		ep = *epp;
		if (ep->ltype=='d' && dflg==0 || fflg)
			pdirectory(ep->lnamep, (argc>1), slastp);
		else 
			pentry(ep);
	}
	while (dfirst != 0) {
		pdirectory (dfirst->dc_name, 1, firstp);
		dtemp = dfirst;
		dfirst = dfirst->dc_next;
		cfree (dtemp->dc_name);
		cfree (dtemp);
	}
	if (outcol)
		putc('\n', stdout);
	fflush(stdout);
	if (ferror(stdout))
		perror("ls: write error"), exit(2);
	exit(0);
}

pdirectory(name, title, lp)
char *name;
int title;
struct lbuf **lp;
{
	filewidth = 0;
	curdir = name;
	if (title)
		printf("\n%s:\n", name);
	lastp = lp;
	rddir(name);
	if (!Cflg)
		filewidth = MAXFILEWIDTH;
	colwidth = fixedwidth + filewidth;
	if (tabflg) {
		if (colwidth <= 8)
			colwidth = 8;
		else
			if (colwidth <= 16)
				colwidth = 16;
	}
	if (fflg==0)
		qsort(lp,lastp - lp,sizeof *lastp,compar);
	if (lflg || sflg)
		printf("total %ld", tblocks);
	pem(lp, lastp);
	newline();
}

pem(slp, lp)
register struct lbuf **slp, **lp;
{
	int ncols, nrows, row, col;
	register struct lbuf **ep;

	if (tabflg) {
		if (colwidth <= 8)
			colwidth = 8;
		else
			if (colwidth <= 16)
				colwidth = 16;
	}
	ncols = 80 / colwidth;
	if (ncols == 1 || Cflg == 0) {
		for (ep = slp; ep < lp; ep++)
			pentry(*ep);
		return;
	}
	if (across) {
		for (ep = slp; ep < lp; ep++)
			pentry(*ep);
		return;
	}
	if (xtraent)
		slp--;
	nrows = (lp - slp - 1) / ncols + 1;
	for (row = 0; row < nrows; row++) {
		col = row == 0 && xtraent;
		for (; col < ncols; col++) {
			ep = slp + (nrows * col) + row;
			if (ep < lp)
				pentry(*ep);
		}
		if (outcol)
			printf("\n");
	}
}

pputchar(c)
char c;
{
	char cc;

	switch (c) {
		case '\t':
			outcol = (outcol + 8) &~ 7;
			break;
		case '\n':
			outcol = 0;
			break;
		default:
			if (c < ' ' || c >= 0177) {
				if (qflg)
					c = '?';
				else if (bflg) {
					outcol += 3;
					putc ('\\', stdout);
					cc = '0' + (c>>6 & 07);
					putc (cc, stdout);
					cc = '0' + (c>>3 & 07);
					putc (cc, stdout);
					c = '0' + (c & 07);
				}
			}
			outcol++;
			break;
	}
	putc(c, stdout);
	if (ferror(stdout))
		perror("ls: write error"), exit(2);
}

newline()
{
	if (outcol)
		putc('\n', stdout);
	outcol = 0;
}

column()
{

	if (outcol == 0)
		return;
	if (nopad) {
		putc(',', stdout);
		outcol++;
		if (outcol + colwidth + 2 > 80) {
			putc('\n', stdout);
			outcol = 0;
			return;
		}
		putc(' ', stdout);
		outcol++;
		return;
	}
	if (Cflg == 0) {
		putc('\n', stdout);
		return;
	}
	if ((outcol / colwidth + 2) * colwidth > 80) {
		putc('\n', stdout);
		outcol = 0;
		return;
	}
	if (tabflg && (colwidth <= 16)) {
		if (colwidth > 8)
			if ((outcol % 16) < 8) {
				outcol += 8 - (outcol % 8);
				putc ('\011', stdout);
			}
		outcol += 8 - (outcol % 8);
		putc ('\011', stdout);
		return;
	}
	do {
		outcol++;
		putc(' ', stdout);
	} while (outcol % colwidth);
}


long nblock(size)
long size;
{
	long blocks, tot;

	blocks = tot = (size + (BSIZE - 1)) / BSIZE;
#if FsTYPE==2
	blocks /= 2;
#endif
	if(blocks > DIRECT)
		tot += ((blocks - DIRECT - 1) >> NSHIFT) + 1;
	if(blocks > DIRECT + NINDIR)
		tot += ((blocks - DIRECT - NINDIR - 1) >> (NSHIFT * 2)) + 1;
	if(blocks > DIRECT + NINDIR + NINDIR*NINDIR)
		tot++;
	return(tot);
}

int	m1[] = { 1, S_IREAD>>0, 'r', '-' };
int	m2[] = { 1, S_IWRITE>>0, 'w', '-' };
int	m3[] = { 2, S_ISUID, 's', S_IEXEC>>0, 'x', '-' };
int	m4[] = { 1, S_IREAD>>3, 'r', '-' };
int	m5[] = { 1, S_IWRITE>>3, 'w', '-' };
int	m6[] = { 2, S_ISGID, 's', S_IEXEC>>3, 'x', '-' };
int	m7[] = { 1, S_IREAD>>6, 'r', '-' };
int	m8[] = { 1, S_IWRITE>>6, 'w', '-' };
int	m9[] = { 2, S_ISVTX, 't', S_IEXEC>>6, 'x', '-' };

int	*m[] = { m1, m2, m3, m4, m5, m6, m7, m8, m9};

pmode(aflag)
{
	register int **mp;

	flags = aflag;
	for (mp = &m[0]; mp < &m[sizeof(m)/sizeof(m[0])];)
		pselect(*mp++);
}

pselect(pairp)
register int *pairp;
{
	register int n;

	n = *pairp++;
	while (--n>=0 && (flags&*pairp++)==0)
		pairp++;
	pputchar(*pairp);
}

char *
makename(dir, file)
char *dir, *file;
{
	static char dfile[MAXNAMLEN];
	register char *dp, *fp;
	register int i;

	dp = dfile;
	fp = dir;
	if (fp)
	while (*fp)
		*dp++ = *fp++;
	if (*(dp-1) != '/')
		*dp++ = '/';
	fp = file;
	for (i=0; i<sizeof dfile; i++) {
		if ((*dp++ = *fp++) == '\0')
			break;
	}
	*dp = 0;
	return(dfile);
}

rddir(dir)
char *dir;
{
	DIR	*dirf;
	register struct dirent *dentp;
	register int j, width;
	register struct lbuf *ep;

	if ((dirf = opendir(dir)) == NULL) {
		printf("%s unreadable\n", dir);
		return;
	}
	tblocks = 0;
	for(;;) {
		if ((dentp = readdir(dirf)) == NULL)
			break;
		if (dentp->d_ino==0 ||
			aflg==0 && dentp->d_name[0]=='.' && (
			!Aflg ||
			dentp->d_name[1]=='\0'
			|| dentp->d_name[1]=='.' && dentp->d_name[2]=='\0'))
			continue;
		/*
		|| Names returned by readdir are guaranteed null terminated
		*/
		width = strlen(dentp->d_name);
		if (Cflg) {
			if (width > filewidth)
				filewidth = width;
		}
		ep = gstat(makename(dir, dentp->d_name), Fflg || pflg || Rflg);
		if (ep==NULL)
			continue;
		if (ep->lnum != -1)
			ep->lnum = dentp->d_ino;
		/*
		|| Allocate a buffer for the name and store it
		*/
		ep->lnamep = lsmalloc(width + 1);
		if (ep->lnamep == NULL)
			continue;
		strcpy(ep->lnamep, dentp->d_name);
	}
	closedir(dirf);
}

char *
lsmalloc(size)
int size;
{
	static int nomocore;
	extern char *malloc();
	char *ret;

	if (nomocore)
		return(NULL);
	if ((ret = malloc(size)) == NULL) {
		fprintf(stderr, "ls: out of memory\n");
		nomocore = 1;
		return(NULL);
	}
	return(ret);
}

struct lbuf *
gstat(file, argfl)
char *file;
{
	struct stat statb;
	register struct lbuf *rep;

	rep = (struct lbuf *)lsmalloc(sizeof(struct lbuf));
	if (rep==NULL)
		return(NULL);
	if (lastp >= &flist[NFILES]) {
		static int msg;
		lastp--;
		if (msg==0) {
			fprintf(stderr, "ls: too many files\n");
			msg++;
		}
	}
	*lastp++ = rep;
	rep->lnamep = NULL;
	rep->lflags = 0;
	rep->lnum = 0;
	rep->ltype = '-';
	rep->linktext = 0;
	if (argfl || statreq) {
		if (clstat(file, &statb)<0) {
			fprintf(stderr, "%s not found\n", file);
			statb.st_ino = -1;
			statb.st_size = 0;
			statb.st_mode = 0;
			if (argfl) {
				lastp--;
				return(0);
			}
		}
#ifdef	S_IFLNK
		if (!Hflg && (statb.st_mode&S_IFMT) == S_IFLNK) {
			struct stat nstatb;

			if (stat(file,&nstatb) == 0)
				if (Lflg || (nstatb.st_mode&S_IFMT) == S_IFDIR
				  && !lflg)
					statb = nstatb;
		}
#endif	S_IFLNK
		rep->lnum = statb.st_ino;
		rep->lsize = statb.st_size;
		switch(statb.st_mode&S_IFMT) {

		case S_IFDIR:
			rep->ltype = 'd';
			break;

		case S_IFBLK:
			rep->ltype = 'b';
			rep->lsize = statb.st_rdev;
			break;

		case S_IFCHR:
			rep->ltype = 'c';
			rep->lsize = statb.st_rdev;
			break;

		case S_IFIFO:
			rep->ltype = 'p';
			break;

#ifdef	S_IFLNK
		case S_IFLNK:
			rep->ltype = 'l';
			if (statb.st_mode&S_INLNK)
				rep->ltype = 'n';
			if (lflg) {
				extern char *sl_getlink();
				
				rep->linktext = sl_getlink(file);
			}
			break;
#endif	S_IFLNK
		}
		rep->lflags = statb.st_mode & ~S_IFMT;
		rep->luid = statb.st_uid;
		rep->lgid = statb.st_gid;
		rep->lnl = statb.st_nlink;
		if(uflg)
			rep->lmtime = statb.st_atime;
		else if (cflg)
			rep->lmtime = statb.st_ctime;
		else
			rep->lmtime = statb.st_mtime;
		tblocks += nblock(statb.st_size);
	}
	return(rep);
}

compar(pp1, pp2)
struct lbuf **pp1, **pp2;
{
	register struct lbuf *p1, *p2;

	p1 = *pp1;
	p2 = *pp2;
	if (dflg==0) {
		if (p1->lflags&ISARG && p1->ltype=='d') {
			if (!(p2->lflags&ISARG && p2->ltype=='d'))
				return(1);
		} else {
			if (p2->lflags&ISARG && p2->ltype=='d')
				return(-1);
		}
	}
	if (tflg) {
		if(p2->lmtime > p1->lmtime)
			return(rflg);
		if(p2->lmtime < p1->lmtime)
			return(-rflg);
		return(0);
	}
	return(rflg * strcmp(p1->lnamep, p2->lnamep));
}

pentry(ap)
struct lbuf *ap;
{
	struct { char dminor, dmajor;};
	register struct lbuf *p;
	register struct dchain *dp;
	register char *cp;
	char fname[100];
	char *pname;
	extern char *agetpw(),*agetgr();

	fname[0] = 0;
	p = ap;
	if (p->lnum == -1)
		return;
	column();
	if (Rflg)
		if (ap->ltype == 'd' &&
		  strcmp(ap->lnamep, ".") && strcmp(ap->lnamep, "..")) {
			dp = (struct dchain *)calloc(1, sizeof(struct dchain));
			pname = makename (curdir, ap->lnamep);
			dp->dc_name = (char *)calloc(1, strlen(pname)+1);
			strcpy(dp->dc_name, pname);
			if (dfirst == 0)
				dfirst = dp;
			else
				dlast->dc_next = dp;
			dlast = dp;
		}
	if (iflg)
		if (nopad && !lflg)
			printf("%d ",  p->lnum);
		else
			printf("%5d ",  p->lnum);
	if (sflg)
		if (nopad && !lflg)
			printf("%ld ", nblock(p->lsize));
		else
			printf("%4ld ", nblock(p->lsize));
	if (lflg) {
		pputchar(p->ltype);
		pmode(p->lflags);
		printf("%2d ", p->lnl);
		if(oflg) {
			if (nflg)
				printf("%8d", p->luid);
			else
				printf("%-8.8s", agetpw(p->luid));
		}
		if(gflg) {
			if (nflg)
				printf("%8d",p->lgid);
			else
				printf("%-8.8s", agetgr(p->lgid));
		}
		if (p->ltype=='b' || p->ltype=='c')
			printf("%3d,%3d", major((int)p->lsize), minor((int)p->lsize));
		else
			printf("%7ld", p->lsize);
		cp = ctime(&p->lmtime);
				/* also include year if in future */
		if(p->lmtime < year || p->lmtime > now)
			if (Tflg)
				printf(" %-7.7s %-4.4s    ", cp+4, cp+20);
			else
				printf(" %-7.7s %-4.4s ", cp+4, cp+20);
		else
			if (Tflg)
				printf(" %-15.15s ", cp+4);
			else
				printf(" %-12.12s ", cp+4);
	}
	strncat (fname, p->lnamep, (sizeof fname) - 2);
	if (Fflg) {
		if (p->ltype == 'd')
			strcat (fname, "/");
		else if (p->ltype == 'l')
			strcat(fname, "@");
		else if (p->ltype == 'n')
			strcat(fname, "!");
		else if (p->lflags & 0111)
			strcat (fname, "*");
		else if (!nopad)
			strcat (fname, " ");
	}
	if (pflg && p->ltype == 'd' && !Fflg)
		strcat (fname, "/");
	printf ("%s", fname);
	if (lflg && ap->linktext != 0)
		printf(" -> %s", ap->linktext);
	freelbuf(ap);
}

freelbuf(lp)
struct lbuf *lp;
{
	if ((lp->lflags & ISARG) == 0 && lp->lnamep != 0)
		free(lp->lnamep);
	if (lp->linktext != 0)
		free(lp->linktext);
	free(lp);
}

/* char printf_id[] = "@(#) printf.c:2.2 6/5/79";*/
#include "varargs.h"
/* This version of printf is compatible with the Version 7 C
 * printf. The differences are only minor except that this
 * printf assumes it is to print through pputchar. Version 7
 * printf is more general (and is much larger) and includes
 * provisions for floating point.
 */
 

#define	MAXOCT	11	/* Maximum octal digits in a long */
#define	MAXINT	32767	/* largest normal length positive integer */
#define	BIG	1000000000	/* largest power of 10 less than an unsigned long */
#define	MAXDIGS 10	/* number of digits in BIG */

static int width, sign, fill;

char *b_dconv();

printf(va_alist)
va_dcl
{
	va_list ap;
	register char *fmt;
	char fcode;
	int prec;
	int length,mask1,nbits,n;
	long int mask2, num;
	register char *bptr;
	char *ptr;
	char buf[134];

	va_start(ap);
	fmt = va_arg(ap,char *);
	for (;;) {
		/* process format string first */
		while ((fcode = *fmt++)!='%') {
			/* ordinary (non-%) character */
			if (fcode=='\0')
				return;
			pputchar(fcode);
		}
		/* length modifier: -1 for h, 1 for l, 0 for none */
		length = 0;
		/* check for a leading - sign */
		sign = 0;
		if (*fmt == '-') {
			sign++;
			fmt++;
		}
		/* a '0' may follow the - sign */
		/* this is the requested fill character */
		fill = 1;
		if (*fmt == '0') {
			fill--;
			fmt++;
		}
		
		/* Now comes a digit string which may be a '*' */
		if (*fmt == '*') {
			width = va_arg(ap, int);
			if (width < 0) {
				width = -width;
				sign = !sign;
			}
			fmt++;
		}
		else {
			width = 0;
			while (*fmt>='0' && *fmt<='9')
				width = width * 10 + (*fmt++ - '0');
		}
		
		/* maybe a decimal point followed by more digits (or '*') */
		if (*fmt=='.') {
			if (*++fmt == '*') {
				prec = va_arg(ap, int);
				fmt++;
			}
			else {
				prec = 0;
				while (*fmt>='0' && *fmt<='9')
					prec = prec * 10 + (*fmt++ - '0');
			}
		}
		else
			prec = -1;
		
		/*
		 * At this point, "sign" is nonzero if there was
		 * a sign, "fill" is 0 if there was a leading
		 * zero and 1 otherwise, "width" and "prec"
		 * contain numbers corresponding to the digit
		 * strings before and after the decimal point,
		 * respectively, and "fmt" addresses the next
		 * character after the whole mess. If there was
		 * no decimal point, "prec" will be -1.
		 */
		switch (*fmt) {
			case 'L':
			case 'l':
				length = 2;
				/* no break!! */
			case 'h':
			case 'H':
				length--;
				fmt++;
				break;
		}
		
		/*
		 * At exit from the following switch, we will
		 * emit the characters starting at "bptr" and
		 * ending at "ptr"-1, unless fcode is '\0'.
		 */
		switch (fcode = *fmt++) {
			/* process characters and strings first */
			case 'c':
				buf[0] = va_arg(ap, int);
				ptr = bptr = &buf[0];
				if (buf[0] != '\0')
					ptr++;
				break;
			case 's':
				bptr = va_arg(ap,char *);
				if (bptr==0)
					bptr = "(null pointer)";
				if (prec < 0)
					prec = MAXINT;
				for (n=0; *bptr++ && n < prec; n++) ;
				ptr = --bptr;
				bptr -= n;
				break;
			case 'O':
				length = 1;
				fcode = 'o';
				/* no break */
			case 'o':
			case 'X':
			case 'x':
				if (length > 0)
					num = va_arg(ap,long);
				else
					num = (unsigned)va_arg(ap,int);
				if (fcode=='o') {
					mask1 = 0x7;
					mask2 = 0x1fffffffL;
					nbits = 3;
				}
				else {
					mask1 = 0xf;
					mask2 = 0x0fffffffL;
					nbits = 4;
				}
				n = (num!=0);
				bptr = buf + MAXOCT + 3;
				/* shift and mask for speed */
				do
				    if (((int) num & mask1) < 10)
					*--bptr = ((int) num & mask1) + 060;
				    else
					*--bptr = ((int) num & mask1) + 0127;
				while (num = (num >> nbits) & mask2);
				
				if (fcode=='o') {
					if (n)
						*--bptr = '0';
				}
				else
					if (!sign && fill <= 0) {
						pputchar('0');
						pputchar(fcode);
						width -= 2;
					}
					else {
						*--bptr = fcode;
						*--bptr = '0';
					}
				ptr = buf + MAXOCT + 3;
				break;
			case 'D':
			case 'U':
			case 'I':
				length = 1;

				fcode = fcode + 'a' - 'A';
				/* no break */
			case 'd':
			case 'i':
			case 'u':
				if (length > 0)
					num = va_arg(ap,long);
				else {
					n = va_arg(ap,int);
					if (fcode=='u')
						num = (unsigned) n;
					else
						num = (long) n;
				}
				if (n = (fcode != 'u' && num < 0))
					num = -num;
				/* now convert to digits */
				bptr = b_dconv(num, buf);
				if (n)
					*--bptr = '-';
				if (fill == 0)
					fill = -1;
				ptr = buf + MAXDIGS + 1;
				break;
			default:
				/* not a control character, 
				 * print it.
				 */
				ptr = bptr = &fcode;
				ptr++;
				break;
			}
			if (fcode != '\0')
				b_emit(bptr,ptr);
	}
	va_end(ap);
}

/* b_dconv converts the unsigned long integer "value" to
 * printable decimal and places it in "buffer", right-justified.
 * The value returned is the address of the first non-zero character,
 * or the address of the last character if all are zero.
 * The result is NOT null terminated, and is MAXDIGS characters long,
 * starting at buffer[1] (to allow for insertion of a sign).
 *
 * This program assumes it is running on 2's complement machine
 * with reasonable overflow treatment.
 */
char *
b_dconv(value, buffer)
long value;
char *buffer;
{
	register char *bp;
	register int svalue;
	int n;
	long lval;
	
	bp = buffer;
	
	/* zero is a special case */
	if (value == 0) {
		bp += MAXDIGS;
		*bp = '0';
		return(bp);
	}
	
	/* develop the leading digit of the value in "n" */
	n = 0;
	while (value < 0) {
		value -= BIG;	/* will eventually underflow */
		n++;
	}
	while ((lval = value - BIG) >= 0) {
		value = lval;
		n++;
	}
	
	/* stash it in buffer[1] to allow for a sign */
	bp[1] = n + '0';
	/*
	 * Now develop the rest of the digits. Since speed counts here,
	 * we do it in two loops. The first gets "value" down until it
	 * is no larger than MAXINT. The second one uses integer divides
	 * rather than long divides to speed it up.
	 */
	bp += MAXDIGS + 1;
	while (value > MAXINT) {
		*--bp = (int)(value % 10) + '0';
		value /= 10;
	}
	
	/* cannot lose precision */
	svalue = value;
	while (svalue > 0) {
		*--bp = (svalue % 10) + '0';
		svalue /= 10;
	}
	
	/* fill in intermediate zeroes if needed */
	if (buffer[1] != '0') {
		while (bp > buffer + 2)
			*--bp = '0';
		--bp;
	}
	return(bp);
}

/*
 * This program sends string "s" to pputchar. The character after
 * the end of "s" is given by "send". This allows the size of the
 * field to be computed; it is stored in "alen". "width" contains the
 * user specified length. If width<alen, the width will be taken to
 * be alen. "sign" is zero if the string is to be right-justified
 * in the field, nonzero if it is to be left-justified. "fill" is
 * 0 if the string is to be padded with '0', positive if it is to be
 * padded with ' ', and negative if an initial '-' should appear before
 * any padding in right-justification (to avoid printing "-3" as
 * "000-3" where "-0003" was intended).
 */
b_emit(s, send)
register char *s;
char *send;
{
	char cfill;
	register int alen;
	int npad;
	
	alen = send - s;
	if (alen > width)
		width = alen;
	cfill = fill>0? ' ': '0';
	
	/* we may want to print a leading '-' before anything */
	if (*s == '-' && fill < 0) {
		pputchar(*s++);
		alen--;
		width--;
	}
	npad = width - alen;
	
	/* emit any leading pad characters */
	if (!sign)
		while (--npad >= 0)
			pputchar(cfill);
			
	/* emit the string itself */
	while (--alen >= 0)
		pputchar(*s++);
		
	/* emit trailing pad characters */
	if (sign)
		while (--npad >= 0)
			pputchar(cfill);
}

#include <pwd.h>
#include <grp.h>

struct idname *uidnames = 0;
struct idname *gidnames = 0;

struct idname {
	char id_string[MAXIDSTRING];
	int id_id;
	struct idname *id_next;
};

char *
agetpw(uid)
	int uid;
{
	extern char *idsearch();
	extern char *getpp();

	return idsearch(&uidnames,getpp,uid);
}

char *
getpp(uid)
	int uid;
{
	extern char *idsearch();
	extern struct passwd *getpwuid();

	register struct passwd *pp;
	static char dumbuf[20];

	if ((pp = getpwuid(uid)) == 0) {
		sprintf(dumbuf,"%d",uid);
		return dumbuf;
	}
	return pp->pw_name;
}

char *
agetgr(gid)
	int gid;
{
	extern char *getgg();

	return idsearch(&gidnames,getgg,gid);
}

char *
getgg(gid)
	int gid;
{
	extern struct group *getgrgid();

	static char dumbuf[20];
	register struct group *gg;

	if ((gg = getgrgid(gid)) == 0) {
		sprintf(dumbuf,"%d",gid);
		return dumbuf;
	}
	return gg->gr_name;
}

char *
idsearch(_hip,getfunc,id)
	struct idname **_hip;
	char *(*getfunc)();
	int id;
{
	register struct idname *ip;
	register char *name;

	for(ip = *_hip; ip != 0; ip = ip->id_next)
		if (ip->id_id == id)
			return ip->id_string;

	name = (*getfunc)(id);
	if ((ip = (struct idname*)malloc(sizeof*ip)) == 0)
		return name;

	strncpy(ip->id_string,name,MAXIDSTRING);
	ip->id_id = id;
	ip->id_next = *_hip;
	*_hip = ip;

	return name;
}
