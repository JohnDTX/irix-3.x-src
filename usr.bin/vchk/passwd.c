/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/* @(#)passwd.c	1.6 */
#include "vchk.h"
/* #define CHKSHELL 	/* -- to enable checking of password line shell */

extern char *getvers(), *index();

char PASSWD[] = "/etc/passwd";
int rflag, gotpwf, pwsiz;
struct plist *pwf;		/* incore copy of passwd file */
char *PWSAVE = "/etc/vchk_pw";	/* default saved password summary file */

struct plist *
pwlookup (n)
	char *n;
{
	register struct plist *p;
	int i;

	if (!n) return 0;

	if (!gotpwf++) pwsiz = setpl(&pwf);
	for (i=0, p=pwf; i<pwsiz; i++, p++)
		if (!strcmp(n,p->name)) return p;
	return 0;
}

char *
acctname (u)
	short u;
{
	register struct plist *p;
	int i;

	if (!gotpwf++) pwsiz = setpl(&pwf);
	for (i=0, p=pwf; i<pwsiz; i++, p++)
		if (u == p->uid) return p->name;
	return 0;
}

char *
acct (u)
{
	static char buf[10];
	char *p;

	if (p = acctname(u)) return p;
	sprintf(buf,"%d",u);
	return buf;
}

acctuid (n)
	char *n;
{
	struct plist *p;

	if (!(p = pwlookup(n)))
		return -1;
	return p->uid;
}
char DIGITS[] = "0123456789";
#define MAXFLEN 50
struct pwid {
	char *pw_field;			/* name of password field */
	char *pw_cset;			/* permissable character set */
	char *pw_len;			/* permissable length */
} pwid[] = {
	{ "account name", 0, "1-8" },
	{ "password", 0, "0|13" },
	{ "user id", DIGITS, "1-5" },
	{ "group id", DIGITS, "1-5" },
	{ "comment", 0, "*" },
	{ "home directory", 0, "*" },
	{ "shell", 0, "*" },
};

#define FN pwid[fn].pw_field
pwchk (p, fn, ln)
	char *p;
{
	register char *cp = p;
	char *sp, *l;
	int flag, maxlen, minlen, len;

	flag = len = 0;
	sp = pwid[fn].pw_cset;
	for (l=pwid[fn].pw_len; *l; l++)
		if (isdigit(*l)) len = (len * 10) + (*l - '0');
		else break;
	minlen = len;
	switch (*l) {
		when '*': maxlen = MAXFLEN;
		when '|': maxlen = atoi(l+1); flag = 1;
		when '-': maxlen = atoi(l+1);
		when '+': minlen = 1;  maxlen = MAXFLEN;
	}
	if (maxlen > MAXFLEN) maxlen = MAXFLEN;
	while (*cp) {
		if (sp && !index(sp,*cp)) {
			N(("Error: Line %d: %s field has illegal char [%s]\n",ln,FN,pchr(*cp)));
			return 0;
		}
		cp++;
	}
	len = cp - p;

	if (len == 0 && (minlen > 0 || flag)) {
		if (!flag) N(("Error: "));
		N(("Line %d: %s field is null\n",ln,FN));
		return flag;
	}
	if (len < minlen) {
		N(("Error: Line %d: %s field[%s] is too short (ie. < %d chars)\n",ln,FN,p,minlen));
		return 0;
	}
	if (flag && len != maxlen)
		N(("Line %d: %s field[%s] is wrong length (ie. != %d chars)\n",ln,FN,p,maxlen))
	else if (len > maxlen) {
		if (ln < 4) N(("Error: "));
		N(("Line %d: %s[%s] field is too long (ie. > %d chars)\n",ln,FN,p,maxlen));
		if (ln < 4) return 0;
	}
	return 1;
}

char *shell[] = { "csh", "sh", 0 };

setpl (plist)
	struct plist **plist;
{	char ***p, ***pw, *cp;
	register char *dp, *np;
	register char **lp, ***pp;
	int rmpw, dirflag, i, lno, less = 0, tlines, totchrs;
	struct plist **opl, *pl;
	FILE *pfp;
	short *sp;
	struct stat sb, psb;
	int funnyshell, freeacct, nonacct;
	extern char ***loadfyl();
	char *getline();
	int pwsort();

	if (stat(PASSWD,&psb))
		E(("Cannot stat passwd file[%s]: %s\n",PASSWD,SE));

		/* check to see if we must re-evaluate the real passwd file */

	rmpw = 0;			/* flag that sez we must remake */
	if (!pflag) pflag = PWSAVE;
sb.st_mtime = 0;
sb.st_mode = 0;
	if (stat(pflag,&sb) ||
	    (sb.st_mtime<=psb.st_mtime) ||
	    (sb.st_mode & ~S_IFMT) !=0644) {
		rmpw = 1;
		if (!*pflag) pflag = PWSAVE;
		D(3,("SETTING PFLAG psb=%ld, sb=%ld, mode=%o",psb.st_mtime,sb.st_mtime,sb.st_mode));
	}
/* Restore saved passwd information */

	pfp = 0;
loadonly:
	if (!rmpw && (pfp = fopen(pflag,"r"))) {
#ifdef UniSoft
		setbuf(pfp,blk_buf);
#endif UniSoft
		pushfile(F_FILEPTR, pfp);
		if (!(np = getline()) || !isdigit(*np)) {
		garbled:
			popfile();
			X(("%s garbled!, rebuilding it\n",pflag));
			goto rebuild;
		}
		if (!(dp = index(np,',')) || !isdigit(*++dp)) goto garbled;
		tlines = atoi(np);
		totchrs = atoi(dp);
		i = (sizeof(struct plist) * tlines) + totchrs;
		pl = (struct plist *) getmem(i,1,"reload uid map",pflag);
		*plist = pl;
		np = (char *)(&pl[tlines]);
		i = 0;
		while (dp = getline()) {
			if (i >= tlines) {
			syntax:
				X(("Syntax err at line %d\n",i+1));
				free(pl);
				goto garbled;
			}
			if (!(cp = index(dp,':')) || !isdigit(*++cp))
				goto syntax;
			pl[i].uid = (short)(atoi(cp));
			cp[-1] = '\0';
			pl[i].name = np;
			np += cps(np,dp);
			*np++ = '\0';
			if (!(dp = index(cp,',')) || !isdigit(*++dp))
				goto syntax;
			pl[i].gid = (short)(atoi(dp));
			i++;
		}
		if (i != tlines) goto syntax;
		popfile();
		D(3,("Sucessfully loaded saved account list\n"));
		return tlines;
	}
rebuild:

	pw = loadfyl(PASSWD,"\n:");	/* read in entire password file */
	if (!pw)
		F(("Cannot load `%s' file\n",PASSWD));

/* Now check each account to make sure it has a directory which is owned by
 * it and that there are no other accounts with the same name or directory.
 */
	N(("\nChecking Password file ...\n"));
	for (p=pw; *p; p++) {			/* for each account */
		funnyshell = 0;
		lno = (p - pw) + 1;		/* line # in passwd file */
		lp = p[0];			/* optimization */
		for(i=0; lp[i]; i++) ;		/* count fields on line */
		if (i != 7) {			/* should be seven fields */
			N(("Error: "));
			if (i == 0) {
				N(("Line %d of passwd file is blank\n",lno));
				goto pwerr;
			}
			N(("Line %d: `",lno));
			for (i=0; lp[i] && i<5; i++)
				N(("%s%c",lp[i],lp[i+1]?':':'\''));
			if (lp[i]) N(("... "));
			N(("\n\thas %d instead of 7 fields\n",i));
			goto pwerr;
		}

		for (i=0; i<7; i++)		/* check each field */
			if (!pwchk(lp[i],i,(p-pw)+1)) goto pwerr;

		freeacct = 0;
		nonacct = 0;

		if (!*lp[1]) freeacct = 1;
		else if (!strcmp("xxxxxxxxxxxxx",lp[1])) nonacct= 1;
		lp[2] = (char *)atoi(lp[2]);	/* make uid useable */
		lp[3] = (char *)atoi(lp[3]);	/* ditto for gid */
		dirflag = 0;			/* homedir or something else */
		np = lp[0];
		dp = lp[5];
		if (strcmp(fname(dp),np)) {	/* does home dir look like one*/
			dirflag = 1;
			if (*dp && strcmp(dp,"/"))
				N(("Line %d: account `%s' has home dir `%s'\n",lno,np,dp));
		}

		if (!*lp[6]) {			/* No shell specified */
			if (nonacct == 0) {	/* and possible to login */
				N(("Error: Line %d: No shell for `%s'\n",lno,np));
				goto pwerr;
			}
			goto pw_ok;		/* dont care if its a dup */
		} else for (i=0; shell[i]; i++)	/* standard shell ? */
			if (!strcmp(shell[i],fname(lp[6]))) goto stdshell;
		funnyshell = 1;
	stdshell:

/* Now check for duplicates
 */
		if (!*dp || !strcmp(dp,"/")) dp = 0;
		for (pp=pw; pp<p; pp++) {
			if (!pp[0]) continue;
			if (!strcmp(pp[0][0],np)) {
				N(("Error: Line %d: Duplicate account `%s'\n",lno,np));
				goto pwerr;
			}
			if (!dp) continue;	/* No directory */

/* Accounts with non-standard shells can share home directories without
 * begin able to use them.
 */
			if (!funnyshell && !strcmp(pp[0][5],dp)) {
				if (pp[0][2] != lp[2]) {
					N(("Error: Line %d: account `%s' shares home with `%s' but has a different user id\n",lno,np,pp[0][0]));
					goto pwerr;
				}
				N(("Line %d: account `%s' uses home directory of account `%s'\n",lno,np,pp[0][0]));
				goto pw_ok;
			}
		}
		if (!dp || nonacct)	/* needn't check dir login impossible */
			goto pw_ok;

		if (stat(dp,&sb)) {		/* stat home directory */
			N(("Home directory `%s' for account `%s' is missing\n",dp,np));
			if (dirflag || funnyshell) {
				N(("\tand cannot be created by this program.\n"));
				goto pw_ok;
			}
			cexec("echo Making home dir %s for acct %s ...",dp,np);
			cexec("mkdir %s",dp);
			cexec("chown %d %s",lp[2],dp);
			cexec("chgrp %d %s",lp[3],dp);
			cexec("chmod 700 %s",dp);
		} else {
			if ((sb.st_mode & S_IFMT) != S_IFDIR) {
				N(("Line %d: `%s' is listed as the home directory\n\tfor account `%s' and it is not a directory\n",lno,dp,np));
				goto pwerr;
			}
			if (funnyshell || dirflag) goto chkshell;
			if ((char *)(sb.st_uid) != lp[2]) {
				N(("Home[%s] of account `%s' has owner %d instead of %d\n",dp,np,sb.st_uid,lp[2]));
				cexec("echo setting owner of %s ...",dp);
				cexec("chown %d %s",lp[2],dp);
			}
			if ((char *)(sb.st_gid) != lp[3]) {
				N(("Home[%s] of account `%s' has group %d instead of %d\n",dp,np,sb.st_gid,lp[3]));
				cexec("echo setting group of %s ...",dp);
				cexec("chgrp %d %s",lp[3],dp);
			}
		}

	chkshell:
#ifdef CHKSHELL
		if (!(cp = getvers(lp[6])) || *cp != '(') { /* valid shell? */
			if (!cp) {
				N(("Error: Line %d: No shell[%s] for account `%s'\n",lno,lp[6],np));
				goto pwerr;
			}
			N(("Shell[%s] for account `%s' is %s\n",lp[6],np,cp));
		}
#endif CHKSHELL
	pw_ok:
		continue;
	pwerr:
		p[0] = (char **)0;	/* flag line as in error */
	}

	for (pp=pw; !*pp && pp<p; pp++) ;	/* find first good entry */
	totchrs = strlen(pp[0][0]);
	qsort(pw,p-pw,sizeof(char ***),pwsort);
	for (pp=pw; pp[1]; pp++)
		if (pp[0][2] == pp[1][2]) {
			less++;
			if (rflag)
				totchrs += strlen(pp[1][0]);
		} else
			totchrs += strlen(pp[1][0]);
	pp++;
	M(("%d accounts in the password file\n",p-pw));
	if (pp < p)
		M(("Errors found in %d of them\n",p-pp));
	if (less)
		M(("%d of %s have duplicate user id numbers%s\n",less,(pp<p)?"the remaining ones":"them",rflag?" and have been ignored":""));
	if (rflag) less = 0;
	tlines = (pp-pw) - less;
	i = (sizeof(struct plist) + 1) * tlines + totchrs;

	pl = (struct plist *) getmem(i,1,"build uid map",pflag);
	np = (char *)(&pl[tlines]);

	unlink(pflag);
	if (!(pfp = fopen(pflag,"w"))) {
		X(("CANT CREAT `%s' file (PASSWD SAVE FILE): %s\n",pflag,SE));
	} else {
		fflush(pfp);		/* this and next line are necc why? */
		fseek(pfp,0,0);
		fprintf(pfp,"%d,%d\n",tlines,tlines+totchrs);
	}

	for (i=0, p=pw; *p; p++) {
		pl[i].name = np;
	 	pl[i].uid = (short)((int)(p[0][2]));
	 	pl[i].gid = (short)((int)(p[0][3]));
		np += cps(np,p[0][0]);
		*np++ = '\0';
		if (pfp)
			fprintf(pfp,"%s:%d,%d\n",pl[i].name,pl[i].uid,pl[i].gid);
		if (!rflag) {
			while (p[1] && (p[0][2] == p[1][2])) p++;
		}
		i++;
	}
	if (pfp) {
		fclose(pfp);
		chmod (pflag,0644);
	}
	free(pw);
	*plist = pl;
	return i;
}

pwsort (a, b) char ***a, ***b;
{	
	if (!a[0]) {
		if (!b[0]) return 0;
		return 1;
	}
	if (!b[0]) return -1;
	if (!strcmp(a[0][0],"root")) return -1;
 	if (!strcmp(b[0][0],"root")) return 1;
	if (a[0][2] > b[0][2]) return 1;
	if (a[0][2] < b[0][2]) return -1;
	if (**a < **b) return -1;
	return 1;
}
