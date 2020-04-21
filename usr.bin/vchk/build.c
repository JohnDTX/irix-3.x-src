/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/* @(#)build.c	1.6 */
#include "vchk.h"

#ifdef	UniSoft
# include <b.out.h>
#else	UniSoft

#ifdef SYSTEMV
# include <a.out.h>
#else
# include <a.out.h>
#endif

#endif	UniSoft

char *index();
char *fname();

char pathname[PATHSIZ];
char rootname[PATHSIZ];
char TOPDIR = 'R';
extern char *IDENT;
extern char _Version_[];
char *realname();

int MAXPDCOL = 5;
int MINPDCOL = 3;

build ()
{
	int rl, ge;
	char *rp;
	long st;
	char *ident, *loadfyl();

	if (aflag) MAXPDCOL++;		/* allow more room for data chk field */
	ident = loadfyl(IDENT,"");
	st = time(0);

	if ((ge = gadn(rootname, sizeof(rootname))) < 0)
		F(("Problems determining current dir name[%s]\n",rootname));

	if (chdir(rootname) == -1)
		E(("cant chdir(%s)\n",rootname));

	printf("## UNEDITED VCHK SNAPSHOT OF [%s] %s\n",rootname,tvtods(st));
	printf("## DELETE THESE TWO LINES IF YOU EDIT THIS FILE\n");
	printf("## Created by %s %s\n",acct(getuid()),fmtym(st));
	if (ident) {
		if (rp = index(ident,'\n')) *rp = '\0';
		printf("## %s\n",ident);
		free(ident);
	}
	printf("#\tVchk %s\n",_Version_);

	rp = pathname;
	if ((rl = strlen(rootname)) > 1) {
		printf("\n%c = .\t\t# %s\n",TOPDIR,rootname);
		*rp++ = '.';
	}
	*rp++ = '/';
	*rp = '\0';

	if (dsrch(pathname) == -1)
		F(("Can't read top directory[%s]: %s\n",pathname,SE));
	
	descend(rp,first_node);
}

struct list {
	short	l_mode;
	short	l_uid;
	short	l_gid;
	short	l_ord;		/* used as temporary */
};

descend (fn, node)
	char *fn;
	struct tree_node *node;
{
	struct list *rt, *ft, *nt, *et, *getmem();
	char *op, *cp, **sp, **ep;
	int i, nf, dirprt=0, pmode(), puid(), pgid(), dsort();
	struct stat sb;
	short bestv();
	int maxlen = 0, len, initlen, fknt;

	Curdir = node;
	initlen = strlen(pathname);
	if (*pathname != '/') initlen += 1;	/* cvt . to $R on output */

	*fn = '\0';
	D(5,("in descend(%s: id=%d)\n",pathname,Curdir->t_idnum));
	if (node->t_ls == 0) {
		X(("Cannot read contents of directory `%s'\n",pathname));
		return 1;
	}

	if (iflag && !query("Descend `%s'",*pathname?pathname:".")) {
		free(node->t_ls);	/* directory release contents */
		node->t_ls = (char ***)0;
		return 0;
	}

	sp = node->t_ls[0];		/* ptr to first filename */
	ep = node->t_ls[1];		/* ptr just after last filename */
	ft = 0;
	if ((nf = ep - sp) <= 0)	/* number of files in this dir */
		goto fini;

	if ((ft = getmem(sizeof(struct list)*nf,0,"file table",pathname)) == 0)
		return 1;

/* Stat all files in each dir to get an idea of what the default setup is.
 */
	nt = ft;
	rt = 0;
	fknt = 0;
	for (; sp<ep; sp++, nt++) {
		if (*sp == 0) {
		elim:
			*fn = '\0';
			nt->l_mode = sp - node->t_ls[0]; /* ordinal number */
			nt->l_ord = -2;	/* flag as dead entry */
			continue;
		}
		len = initlen;
		cp = *sp;
		op = fn;
		while (*cp) {
			if (escchar(*cp)) *op++ = '\\';
			*op++ = *cp++;
		}
		*op = '\0';
		len += op - fn;
		if (len > maxlen) maxlen = len;
		if (stat(realname(pathname),&sb)) {
			X(("stat(%s): %s\n",pathname,SE));
			goto elim;
		}
		nt->l_uid = sb.st_uid;
		nt->l_gid = sb.st_gid;
		nt->l_mode = sb.st_mode;
		nt->l_ord = 0;
		if ((nt->l_mode & S_IFMT) == S_IFDIR)
			continue;
		if (sb.st_nlink > 1) {
			if (chklink(pathname,sb.st_ino,sb.st_dev,sb.st_nlink)) {
				*sp = (char *)0; /* done for all but last link*/
				goto elim;
			}
		}
		fknt++;
	}

	et = nt;			/* ptr just past last list entry */
if (et-ft != nf)
D(0,("FILE TABLE(%s) DOES NOT AGREE WITH LISTING\n",((fn[1]='\0'),pathname)));

	i = (maxlen + TAB_WIDTH - 1) / TAB_WIDTH;
	while (i > Pdfcol && Pdfcol < MAXPDCOL) {
		Pdfcol++;
		Psfcol++;
	}
	while (i < Pdfcol && Pdfcol > MINPDCOL) {
		Pdfcol--;
		Psfcol--;
	}
	if (fknt > 2) {
		node->t_dmode = bestv(ft,et,pmode);
		node->t_duid = bestv(ft,et,puid);
		node->t_dgid = bestv(ft,et,pgid);
	}
	i = 0;

	for (nt=ft; nt<et; nt++) {	/* save ordinal to allow sorting */
		if (nt->l_ord != -2)
			nt->l_ord = i;
		i++;
	}

	qsort (ft,et-ft,sizeof(struct list),dsort);	/* dirs first */
	for (nt=ft; nt<et; nt++)
		if ((nt->l_mode & S_IFMT) != S_IFDIR) {
			rt = nt;
			while (nt < et) {
				if (nt->l_ord >= 0) goto doents;
				nt++;
			}
			if (dirdiff()) {
				*fn = '\0';
				dirprt = 1;
				prtdirent();
			}
			goto dodirs;
		}

	if (!rt) {			/* no files in this dir (so far) */
		rt = et;
		*fn = '\0';
		dirprt = 1;
		prtdirent();
		goto dodirs;
	}

doents:
	while (nt<et) {			/* print file descriptions */
		if (nt->l_ord < 0) {
			nt++;
			continue;
		}
		if ((op = node->t_ls[0][nt->l_ord]) == 0) {
			nt++;
			continue;
		}
		if (dirprt == 0) {	/* print entry for dir if files exist */
			*fn = '\0';
			dirprt = 1;
			prtdirent();
		}
		cp = fn;
		while (*op) {
			if (escchar(*op)) *cp++ = '\\';
			*cp++ = *op++;
		}
		*cp = '\0';
		prtdesc(nt);
		nt++;
	}

dodirs:
D(3,("About to do subdirectories of `%s'\n",((*fn=0),pathname)));
	for (nt=ft; nt<rt; nt++) {	/* now do each subdirectory */
		if (cps(fn,node->t_ls[0][nt->l_ord]) < 1) {/* build pathname */
			continue;
		}
			
		op = pathname + strlen(pathname);
		*op++ = '/';
		*op = '\0';
		if (dsrch(pathname) == -1) {	/* sets Curdir */
			M(("Cannot read `%s': %s\n",pathname,SE));
			continue;
		}
		descend(op,Curdir);
		dirprt = 1;
	}

fini:
	if (dirprt == 0) {
		*fn = '\0';
		prtdirent();
	}
	if (ft) free(ft);		/* release core for file-table */
	prtrmdr(node);			/* print un-identified files */
	rmtree(node);			/* remove sub-tree */
}

prtdirent ()
{
	struct tree_node *p;
	char buf[PATHSIZ], *cp;
	char *setpname();
	int pflg = 0;
	int sflg = 0;

	cp = setpname(buf);
	p = Curdir->t_parent;
	if (!p || Curdir->t_dmode != p->t_dmode) {
		pflg = 1;
		sprintf(cp," %o",Curdir->t_dmode);
		cp += strlen(cp);
	}
	if (!p || Curdir->t_duid != p->t_duid) {
		pflg = 1;
		sprintf(cp,"\t%s",acct(Curdir->t_duid));
		cp += strlen(cp);
	}
	if (!p || Curdir->t_uid != p->t_uid) {
		sflg = 1;
		sprintf(cp," (chown %s",acct(Curdir->t_uid));
		cp += strlen(cp);
	}
	if (!p || Curdir->t_mode != p->t_mode) {
		sprintf(cp,"%schmod %o",sflg?"; ":" (",Curdir->t_mode);
		cp += strlen(cp);
		sflg = 1;
	}
	if (sflg)
		*cp++ = RPREN;
	*cp = '\0';
	if (sflg || pflg)
		printf("\n");
	spectype(buf);
}

prtdesc (n)
	struct list *n;
{
	short md = -1, gn = -1;
	char *ln, *op;
	char *acctname();
	struct plist *pw, *pwlookup();
	char *sep="	(", *dp, *getdesc(), *mklnames();
	char *SEPSTR = "; ";
	char buf[LINESIZ];

	pw = (struct plist *)0;
	if ((n->l_mode & S_IFMT) != S_IFDIR) {
		if (!eliminate(fname(pathname),Curdir))
			E(("LOGIC(prtdesc): cannot eliminate `%s' from %s\n",pathname,pwd()));
	/* We keep four shorts per entry (n->): mode, uid, gid, and index into
	 * the t_ls structure.  Thus there is no clean way to remember that the
	 * file is not linked and avoid this test.
	 */
		ln = mklnames(pathname,Curdir->t_idnum);

		if (Curdir->t_dmode != (n->l_mode & (short)~S_IFMT))
			md = n->l_mode & ~S_IFMT;

		if (Curdir->t_duid != n->l_uid) {
			if ((pw = pwlookup(acctname(n->l_uid))) == 0) {
				M(("%s has uid %d[not in password file]\n",pathname,n->l_uid));
				return;
			}
			if (n->l_gid != pw->gid)
				gn = n->l_gid;
		} else if (Curdir->t_dgid != n->l_gid)
			gn = n->l_gid;
	}
	dp = getdesc(pathname);
	op = setpname(buf);
	if (dp) {
		*op++ = ' ';
		op += cps(op,dp);
	}

	if (pw || ln || md != -1 || gn != -1) {	/* have specifications */
		if (pw) {
 			sprintf(op,"%schown %s",sep,pw->name);
			sep = SEPSTR;
			op += strlen(op);
		}
		if (gn != -1) {
			sprintf(op,"%schgrp %d",sep,gn);
			sep = SEPSTR;
			op += strlen(op);
		}
		if (md != -1) {
			sprintf(op,"%schmod %o",sep,md);
			sep = SEPSTR;
			op += strlen(op);
		}
		if (ln) {
			sprintf(op,"%slink to %s",sep,ln);
			op += strlen(op);
		}
		*op++ = RPREN;
	}
	*op = '\0';
	spectype(buf);
}

char *
setpname(buf)
	char buf[];
{
	char *op;

	buf[0] = 0;
	if (*pathname != '/') sprintf(buf,"$%c/",TOPDIR);
	op = buf + strlen(buf);
	if (*pathname == '.') op += cps(op,pathname+2);
	else op += cps(op,pathname);
	return op;
}

short
bestv (ft, lt, f)
	struct list *ft, *lt;
	short (*f)();
{
	struct list *pt, *nt;
	struct list *nextent;
	struct list *best;
	short curval;

	for (nt=ft; nt<lt; nt++)
		if (nt->l_ord != -2) nt->l_ord = 0;	/* zero counters */
	nt = ft;

	while (nt && nt < lt) {
		if (nt->l_ord < 0 /*|| (nt->l_mode & S_IFMT) == S_IFDIR */) {
			nt++;
			continue;
		}
		nt->l_ord = 1;
		curval = f(nt);
		nextent = (struct list *)0;
		for (pt=nt+1; pt<lt; pt++) {
			if (pt->l_ord < 0 /* || (pt->l_mode & S_IFMT) == S_IFDIR*/ )
				continue;
			if (f(pt) == curval) {
				pt->l_ord = -1;
				nt->l_ord++;
				continue;
			}
			if (nextent == (struct list *)0)
				nextent = pt;
		}
		nt = nextent;
	}

	best = ft;
	for (nt=ft+1; nt < lt; nt++)
		if (nt->l_ord > best->l_ord)
			best = nt;
	return f(best);
}

pmode(l)
	struct list *l;
{
	return (l->l_mode & ~S_IFMT);
}

puid(l)
	struct list *l;
{
	return l->l_uid;
}

pgid(l)
	struct list *l;
{
	return l->l_gid;
}

dsort (a, b)
	struct list *a, *b;
{
	short am, bm;

	if (a->l_ord < 0) {		/* one or both not allocated */
		if (b->l_ord < 0) return 0;
		return 1;
	}
	if (b->l_ord < 0) return -1;
	am = a->l_mode & S_IFMT;
	bm = b->l_mode & S_IFMT;

	if (am == S_IFDIR) {
		if (bm == S_IFDIR) goto cmp;
		return -1;
	}
	if (bm == S_IFDIR) return 1;
cmp:
	return strcmp(Curdir->t_ls[0][a->l_ord], Curdir->t_ls[0][b->l_ord]);
}

char *xstab[] = { "o", "out", 0 };

char *
getdesc (file)
	char *file;
{
	int j, extflag, gotvers = 0;
	unsigned sum;
	char *rn;
	char dchr, *bp, *fn, *cp, *xp, *vp, **xsp, *findvers();
	struct bhdr bhdr;
	struct stat sb;
	FILE *fp;
	static char buf[LINESIZ];
	static char iobuf[BUFSIZ];
	char *instr();

	dchr = 'c';
	if (stat((rn = realname(file)),&sb)) {
		X(("Cannot stat `%s' after having done so already\n",rn));
		return "";
	}
	switch (sb.st_mode & S_IFMT) {
	case S_IFBLK:	dchr = 'b';
	case S_IFCHR:	sprintf(buf,"%c %d %d",dchr,major(sb.st_rdev),minor(sb.st_rdev));
			return buf;
	case S_IFDIR:	return "LOGIC ERROR";
	}

	extflag = 0;
	fn = fname(file);
	if (xp = index(fn,'.')) {		/* has extension */
		xp++;
		for (xsp = xstab; *xsp; xsp++) {
			if ((j = strcmp(*xsp,xp)) >= 0) {
				if (j == 0) extflag = 1;
				break;
			}
		}
	}

	if ((sb.st_mode & S_IFMT) != S_IFREG) {
		X(("Nonstandard special files[%s] unimplemented\n",rn));
		return "";
	}

	fp = fopen(rn, "r");
	if (fp == NULL) {
		X(("Cannot open `%s': %s\n",rn,SE));
		return "";		/* cannot open */
	}
#ifdef UniSoft
	setbuf(fp,iobuf);
#endif UniSoft
	bp = buf;
	if ((j = fread(&bhdr, 1, sizeof(bhdr), fp)) != sizeof(bhdr)) {
		if (j <= 0) {
			if (ferror(fp)) {
				X(("Read error on `%s': %s\n",rn,SE));
				buf[0] = '\0';
				goto fini;	/* read error */
			}
			X(("Warning: `%s' is empty\n",file));
			cps(buf,"Len 0");
			goto fini;
		}
		goto notexec;
	}

	if (vp = findvers(&bhdr,fp)) {		/* return version */
		if (cp = instr(vp,"Version ")) {
			vp = cp + 8;
			if (cp = index(vp,' ')) *cp = '\0';
		} else if (cp = index(vp,' ')) {
			*cp = '\0';
		} else vp = "";
		if (*vp) {
			gotvers = 1;
/* The middle %s is due to version numbers (!?!) that begin with alphabetic
 * characters.  Without the space they confuse the parser which is look for
 * words before checking what they mean.
 */
			sprintf(buf,"%s%s%s", aflag?"V":"Version ",
				aflag&&isalpha(*vp)?" ":"", vp);
			if (!aflag)
				goto fini;
			else {
				bp = buf + strlen(buf);
				*bp++ = ' ';
				goto notexec;
			}
		}
#ifndef UniSoft
		if (!extflag && !gotvers)
			N(("`%s' is executable yet has no version number\n",file));
#endif UniSoft
		if (aflag) goto notexec;
		sprintf(bp,"Length %d",sb.st_size);
		goto fini;
	}

notexec:
	if (aflag != kflag) {
		if (chksum(fp,&sum)) {
			X(("could not checksum %s\n",file));
			*bp = 0;
			goto fini;
		}
		sprintf(bp,"C%d L%d",sum,sb.st_size);
	} else
		sprintf(bp,"Len %d",sb.st_size);
fini:
	fclose(fp);
	return buf;
}

dirdiff()
{
	struct tree_node *p;

	p = Curdir->t_parent;
	if (!p) return 1;
	if (p->t_mode != Curdir->t_mode ||
	    p->t_gid != Curdir->t_gid ||
	    p->t_uid != Curdir->t_uid ||
	    p->t_dmode != Curdir->t_dmode ||
	    p->t_duid != Curdir->t_duid ||
	    p->t_dgid != Curdir->t_dgid)
		return 1;
	return 0;
}
