/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/* @(#)chkit.c	1.6 */
#include "vchk.h"
#include <errno.h>

char *Takecmd;			/* rebuild command */
char *sys_id;			/* name of the system we are running on */

extern char *pindex();
char *acct(), *chkvers(), *mkdevcmd(), *fname();

extern struct stat sibuf;	/* buffer with stat of item from doit() */
extern char dnpath[];		/* used when linking to hold cur link name */
extern int lnsearch;		/* if set doit started link search */

/* This procedure is called when the main link is missing.  It uses islink
 * to find and save the first link that exists and then relinks the
 * rest of them.
 */
int cantfix;			/* cant be fixed without human intervention */
/* reflects situations too complex and rare to bother with programatically */

getlink (fn, lnl)
	char *fn, *lnl;
{
	int islink(), relink();
	char *explink();
	static int cfmsg = 0;

	dnpath[0] = 0;
	lnl = explink(lnl);
	D(3,("getlink(%s, %s)\n",fn,lnl));
	pexpand(lnl,islink);
	if (dnpath[0]) {
		D(3,("getlink foundlink (%s)\n",dnpath));
		if (cantfix) {		/* had two non-identical copies */
			D(3,("getlink cantfix (%s, %s, %s)\n",fn,lnl,dnpath));
			if (cfmsg++ == 0) {
				X(("%s is missing and more than one of the files\n",fn));
				M(("\tthat are supposed to be links to it exist and are different\n"));
				M(("\tThis situation must be fixed by hand\n"));
			} else
				X(("%s is missing and more than one of it's links exists\n",fn));
			return 0;
		} else {
			X(("Warning: %s is missing but can be relinked to %s\n",fn,dnpath));
			cexec("ln %s %s",dnpath,fn);
			eliminate(realname(dnpath),Curdir);
			pexpand(lnl,relink);
		}
		return 1;
	}
	return 0;
}

/* This proceedure is used when the main link is missing.  pexpand will
 * call it with each pathname.  If one exists then it will use it to
 * rebuild the rest.
 */
islink (p, k)
	char *p;
{
	int rv;
	char *lp;
	char *prtlinks();
	struct stat sbuf;
	struct stat *sb = &sbuf;

	if (dnpath[0] == 0) sb = &sibuf;
	if (stat(realname(p),sb) != 0) return -1;
	if (sb->st_nlink > 1)
		chklink(p,sb->st_ino,sb->st_dev,sb->st_nlink);
	if (dnpath[0] != 0) {		/* this is another existing link */
		D(3,("Have both `%s' and `%s' to check for equality\n",dnpath,p));
		if ((lp = prtlinks(p)) == 0 || !instr(lp,dnpath)) {
			D(3,("Have links[%s]\n",lp));
			if (rv = cmp(dnpath,p)) {
				cantfix = 1;
				return 0;	/* halt pexpand */
			}
		}
	} else
		strcpy(dnpath,p);
	return -1;				/* stop search */
}

/* Called when dnpath has been set to a link that exists.  It's job
 * is fix the link it is called with.  The only difficulty is what
 * to do when it does exist.
 */
relink (p, k)
	char *p;
{
	static int rm_mesg;
	struct stat sb;
	int v;

	if (strcmp(dnpath, p) != 0) {
		if (stat(realname(p),&sb) == 0) {
			scratch(p);
			if (sb.st_nlink > 1)	/* find others */
				chklink(p,sb.st_ino,sb.st_dev,sb.st_nlink);
			if (sb.st_ino!=sibuf.st_ino || sb.st_dev!=sibuf.st_dev){
				D(3,("Comparing `%s' with `%s'\n",dnpath,p))
				if ((v = cmp(dnpath,p)) != 0) {
					if (v < 0) {
						M(("\tGood grief! I can't decide which one to link to!\n"));
						return 0;
					}
					X(("%s is not a link or copy of %s\n",p,dnpath));
					if (rm_mesg++ == 0) {
						M(("This program will not remove files it cannot identify, therefore\n"));
						M(("%s and %s will have to be relinked by hand\n",dnpath,p));
					}
				} else {
					X(("%s is a duplicate and can be removed and relinked\n",p));
					cexec("rm %s",p);
					cexec("ln %s %s",dnpath,p);
				}
			}
		} else {
			X(("%s needs relinking to %s\n",p,dnpath));
			cexec("ln %s %s",dnpath,p);
		}
	}
	return -1;
}

/* Like cmp the program.
 * Returns 0 if a and b are the same, 1 if they are different,
 * -1 if a is unreadable, -2 if b is unreadable and -3 if both.
 */
cmp (a, b)
	char *a, *b;
{
	FILE *fa, *fb;
	char ca, cb;
	int rv = 0;

	fa = fopen(a,"r");
	fb = fopen(b,"r");

	if (fa == NULL) {			/* cannot open a */
		X(("cannot open `%s' for reading: %s\n",a,SE));
		rv = -1;
	}
	if (fb == NULL) {			/* cannot open b */
		X(("cannot open `%s' for reading: %s\n",b,SE));
		if (rv == 0) {
			fclose(fa);
			rv = -1;
		}
		rv = -3;
	}
	if (rv != 0) return rv;

	while (1) {			/* compare character by character */
		ca = getc(fa);
		cb = getc(fb);
		if (ca != cb) break;
		if (feof(fa)) {
			if (feof(fb)) {	/* including the EOF's */
				fclose(fb);
				fclose(fa);
				return 0;
			}
			break;
		}
		if (feof(fb)) break;
	}
	fclose(fb);
	fclose(fa);
	return 1;
}

/* Check one item.
 * `f' is the pointer to the line to be checked.  It starts with a
 * simple anchored pathname (ie no parenthesis and starting at the root).
 * It is optionally followed by a space and some specifications.
 * This function determines what must be done to bring this file up to par.
 * It calls cexec with printf like shell commands to implement the changes.
 * The reason for this interface is more historic than suitable.
 */
chkit (f)
	char *f;			/* always points at the filename */
{
	char *p, *np, *vp;		/* various pointers */
	unsigned cs, fcs, csum();	/* stuff for checksums */
	struct plist *pwp, *pwlookup();	/* stuff for fast name/uid/gid mapping*/
	char *takeflag=0, *devflag=0;	/* remake flags (dirs made in dsrch) */
	char *ownrflag=0;		/* must change onwer flag */
	int modeflag=0, grpflag= -1;	/* ditto for mode and group */
	int mode, uid, gid;		/* setup to defaults for this file */
	int hadlink=0, chkdlen=0;	/* when explicit inst*/
	int cmdflag=0, nomake=dflag,	/* ;, [], <>, ~ flags */
		nomatter=0, ownflag=0;	/* passed as !,-,^ and ~ respectively */
	int nofix=0;			/* more of above, {} flag */
	char *cptr=0, *explink();	/* comment ptr, expand $. function */
	int rv;				/* various `returned values' (oh well)*/
	char **lp, **sp, **divs();
	int makefail=0;			/* set if cexec fails */
	int saverrno;

	cantfix = 0;			/* assume we can fix it */

#ifdef BUGCHECK
	if (Curdir == 0) {		
		fprintf(stderr,"LOGIC: no [internal] current directory!\n");
		exit(1);
	}
#endif BUGCHECK
	mode = Curdir->t_dmode;		/* initialize modes from current dir */
	uid = Curdir->t_duid;
	gid = Curdir->t_dgid;
	if (uid == ANYOWNER) ownflag = 1;

	D(1,("chkit <%s> (m=%o u=%d g=%d)\n",f,mode,uid,gid));

	if ((p = pindex(f,' ')) == 0)		/* find end of pathname */
		p = f + strlen(f);
	np = p--;				/* save ptr to end */

	if (*p == '/' || *p == '*') {		/* lop off directory flag */
		X(("directory contents checking unimplemented\n"));
		*p-- = '\0';
		if (*p == '/') *p = '\0';
	}
	if (*np == ' ') {			/* terminate pathname */
		p = np;
		*p++ = '\0';
		if (np = pindex(p,'"')) {	/* lop off comment */
			if (*--np != ' ') np++;
			*np++ = '\0';
			cptr = np;
			if (*cptr == '"') cptr++;
		}
	} else p = (char *)0;

	if (p) {			/* Determine if non standard action */
		if (*p == '^') {	/* don't report non-existance */
			if (!aflag)
				nomatter = 1;
			nomake = 1;
			*p++;
		}
		if (*p == '-') {	/* Don't attempt to remake */
			nomake = 1;
			*p++;
		}
		if (*p == '+') {	/* Don't attempt to fix if wrong */
			nofix = 1;
			*p++;
		}
		if (*p == '!') {	/* Commands follow -- dont issue any */
			cmdflag = 1;
			*p++;
		}
		if (*p == '~') {	/* Owner doesn't matter */
			ownflag = 1;
			*p++;
		}
		if (*p == ' ')
			p++;
	}

	vp = p;
	while (p) {		/* setup default mode and ownership */
		if (np = pindex(p,' ')) *np++ = '\0';
		if (isdigit(*p)) {		/* non-standard mode */
			saverrno = errno;
			sscanf(p,"%o",&mode);
			errno = saverrno;
		} else if (*p == '~' && p[1] == '\0')
			ownflag = 1;
		else if (pwp = pwlookup(p)) {	/* non-standard owner */
			uid = pwp->uid;
			gid = pwp->gid;
		}
		if (p = np) np[-1] = ' ';
	}
	p = vp;

	if (sibuf.st_ino == 0) {	/* file is missing (global stat) */
		if (p && devchk(p))	/* dev spec comes before link spec */
			devflag = mkdevcmd(p);	/* not done if link found */

		while (p && (*p != '>'))	/* look for link */
			if (p = pindex(p,' ')) p++;
		if (p) {		/* supposed to be links to this file */
			p++;		/* skip over '>' (link flag) */
			if (np = pindex(p,' ')) *np++ = 0;
			if (getlink(f,p)) {
				p = vp;
				hadlink = 1;
				goto is_ok;	/* so check it */
			} else goto link_fail;
		} else {		/* file not here and we cant find it */
		link_fail:
			if (!nomatter && !cantfix) {
				if (errno == ENOENT) {
					if (cptr) X(("%s[%s] ",f,cptr))
					else X(("%s ",f))
					M(("is missing%s\n",nomake&&!cmdflag?" and cannot be remade automatically":""))
					takeflag = Takecmd;
				}
				else X(("%s: %s\n",f,SE))
			}
		}
	} else {		/* File exists, perform all checks */

	is_ok:
		if (nofix) nomake = 1;
		if ((sibuf.st_mode & S_IFMT) == S_IFDIR) {
			if (p && cindex(p,"CVL"))
				X(("%s is a directory[mode %o, owner %s] not a file\n",f,sibuf.st_mode&~S_IFMT,acct(sibuf.st_uid)))
			else
				M(("ERROR: directory[line %d: %20s ...] pathnames must end in `/'\n",Lno,f));
			rv = -1;
			goto fini;
		}
		while (p) {		/* check various data check fields */
			if (np = pindex(p,' ')) *np++ = '\0';

			if (*p == 'C') {	/* checksum */
				if (kflag && (cs=csum(f)) != (fcs=atoi(p+1))) {
					X(("Chksum(%s)=%d instead of %d\n",f,cs,fcs));
					takeflag = Takecmd;
				}
			} else if (*p == 'L') {		/* length of file */
				chkdlen = 1;
				if (sibuf.st_size != atoi(p+1)) {
					X(("Sizeof(%s)=%d instead of %d\n",f,sibuf.st_size,atoi(p+1)));
					takeflag = Takecmd;
				}
			} else if (*p == 'V') {		/* version number */
				if (vp = chkvers(f,p+1)) {
					if (outofdate(vp,p+1)) oodm(f);
					else {
						X(("%s: Version %s instead of %s\n",f,vp,p+1));
						takeflag = Takecmd;
					}
				}
			} else if (*p == '>') {			/* link */
				if (!hadlink) {		/* got here via is_ok */
					p++;
					hadlink = 1;
					p = explink(p);
					strcpy(dnpath,f);
					pexpand(p,relink);
				}
			} else if (devchk(p)) {	/* device specification */
				if (*p++ == 'c') {
					if ((sibuf.st_mode & S_IFMT) != S_IFCHR)
						X(("%s: not character special\n",f));
				} else {
					if ((sibuf.st_mode & S_IFMT) != S_IFBLK)
						X(("%s: not block special\n",f));
				}
				p++;	/* skip over : */
				if (*p!='x' && ((sibuf.st_rdev >> 8) & 0xFF) != atoi(p)) {	
					X(("%s: major dev %d instead of %d\n",f,(sibuf.st_dev>>8)&0xFF,atoi(p)));
				}
				p = pindex(p,':')+1;
				if (*p!='x' && (sibuf.st_rdev & 0xFF) != atoi(p)) {	
					X(("%s: minor dev %d instead of %d\n",f,sibuf.st_dev&0xFF, atoi(p)));
				}
			} else if (*p == '*') {		/* NOT FOR DIST */
				if (!sys_id && !modeflag) {
					if (((sibuf.st_mode& ~S_IFMT)&044)||sibuf.st_uid)
						N(("`%s' is not for distribution yet is readable\n",f));
					if ((sibuf.st_mode&~S_IFMT)&044) modeflag = 0700;
					if (sibuf.st_uid != 0) ownrflag = "root";
				}
			}
			p = np;
		}

		if (!hadlink) {
			if (sibuf.st_nlink > 1) {
				if (lnsearch == 1)
					X(("unspecified links for %s\n",f))
				else if (lnsearch == -1) {
					np = prtlinks(f); /* get link names */
					X(("%s linked to %s\n",f,np=prtlinks(f)));
					if (iflag && (lp = divs(np," "))) {
						for (sp=lp; *sp; sp++) {
							cexec("rm %s",*sp);
							cexec("ln %s %s",f,*sp);
						}
						free(lp);
					}
				}
			}
		}
		if ((sibuf.st_mode & ~(short)(S_IFMT)) != mode) {
			N(("%s has mode %o instead of %o\n",f,(sibuf.st_mode&~(short)(S_IFMT)),mode));
			modeflag = mode;
		}

		if (!ownflag) {
			if (uid != sibuf.st_uid) {
				N(("%s: uid %d instead of %d\n",f,sibuf.st_uid,uid));
				ownrflag = acct(uid);
			}
			if (gid != sibuf.st_gid) {
				N(("%s: gid %d instead of %d\n",f,sibuf.st_gid,gid));
				grpflag = gid;
			}
		}
		if (!chkdlen && ((sibuf.st_mode & S_IFMT) == S_IFREG) &&
		    sibuf.st_size == 0)
			X(("Warning: %s is empty\n",f)); 
	}

	if (cmdflag) {
		docmds();
		goto fini;
	}

	if (!nomake && ((takeflag  && *takeflag) || devflag)) {
#ifdef TESTOMIT
		if (nomatter && takeflag) {
			X(("%s needs remaking but has been flagged\n\t[by using <>'s] as not automatically remakeable\n",f));
		} else {
#endif TESTOMIT
			if (!ownrflag)
				ownrflag = acct(uid);
			modeflag = mode;
			if (takeflag) makefail = cexec("%s %s",takeflag,f);
			else makefail = cexec("mknod %s %s",f,devflag);
#ifdef TESTOMIT
		}
#endif TESTOMIT
	}

	if (!nomake && xflag) {
		if (stat(realname(f), &sibuf))
			X(("WARNING: could not make `%s'\n",f));
		if ((sibuf.st_mode & ~(short)(S_IFMT)) != mode)
			modeflag = mode;
		if (uid != sibuf.st_uid)
			ownrflag = acct(uid);
		if (gid != sibuf.st_gid)
			grpflag = gid;
	}

	if (ownrflag && !ownflag && !makefail) {
		makefail = cexec("chown %s %s",ownrflag,f);
		if (!modeflag && (mode & 07000)) modeflag = mode;
	}
	if (grpflag != -1 && !ownflag && !makefail) {
		makefail = cexec("chgrp %d %s",grpflag,f);
		if (!modeflag && (mode & 07000)) modeflag = mode;
	}
	if (modeflag && !makefail) cexec("chmod %o %s",modeflag,f);
	rv = makefail ? 0 : 1;
fini:
	return rv;
}

devchk (p)
	register char *p;
{
	return ((*p == 'c' || *p == 'b') && *++p == ':') ?1 :0;
}

char *
mkdevcmd (p)
	char *p;
{
	static char buf[20];

	if (pindex(p,'x')) return 0;
	sprintf(buf,"%c %d %d",*p,p+2,pindex(p+2,':'));
	return buf;
}

docmds ()
{
	char *cp, *gettext();

	while (cp = gettext()) {
		while (eval(cp, 1))
			;
		cp = explink(cp);
		cexec(0,cp);
	}
}
