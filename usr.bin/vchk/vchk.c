/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

#ifdef PWB
char _Version_[] = "(C) Copyright 1982, UniSoft Systems, Version III.1.10";
char _Origin_[] = "UniSoft Systems";
#else
char _Version_[] = "(C) UniSoft Corp., Version 1.15";
#endif
/*  VCHK  --		Read version table and output
 *			commands or instructions to correct it.
 */
#include "vchk.h"
#ifdef UCB_NET
#include "sys/misc.h"
#include "sys/in.h"
char HOST[] = "unisoft";	/* ethernet source file host name */
#endif

char *IDENT = "/etc/ident";		/* name of system we are running on */
	/* Used in a comment of `built' control files only */

char *ERRLIST = "/etc/vchk_errs";	/* errors found last run */
char *OEMLIST = "/etc/takelist";	/* list of oem permissions */
char *REMKMAC = "REMAKE";		/* define to be take -s or install */
char *Takecmd;				/* for internal use (remkmac value) */
					/* set whenever REMKMAC macro reset */
char *DEFTREE = "/etc/vchk_tree";	/* where to look for description file*/

char LPRENS[] = "({[<";			/* see LPREN, LSQIG, LBRKT, and LANGL*/
char RPRENS[] = ")}]>";			/* ditto */

char *initbrk;				/* sbrk upon startup */
int tty;				/* for iflag */

char *sys_id;		/* set to /etc/sys_id file or null if not unisofts */
#define SYSID "/etc/sys_id"
extern int idlev;
extern char *sbrk(), *end;
char *acctname();

char *defownr = "bin";		/* the default owner */
int Defmode = 0644;		/* the default mode */
int rem;			/* ethernet remote file descriptor */
extern int rflag;
extern char ***Pline, ***getoem();

struct stat *isfile();

main (c, l)
	char *l[];
{
	FILE *tfp;
	struct stat *sbp;
	struct plist *rpe, *pwlookup();
	char *getspec();
	extern char *loadfyl();
	char *cp;
	char *sbrk();
	char tkcmd[100];		/* to build take -s[sysid] command */
#ifdef UCB_NET
	char *host = HOST;
#endif

	umask(0);
	initbrk = sbrk(0);
	chrbp = chrbuf;
	args(0, --c, ++l);		/* parse options (can be done again)*/

	if (bflag) {			/* build new tree file */
		build();
		goto fini;
	}
	if (Pflag) {
		if (*Pflag && (Pline = getoem(Pflag)) == 0)
			F(("No entry for `%s' in %s\n",Pflag,OEMLIST));
		goto readfile;
	}

	if (!(rpe = pwlookup("root")))
		F(("No root account, Check passwd file\n"));
	if (rpe->uid != 0)
		F(("Root account is not a superuser!\n"));
	if (!(rpe = pwlookup(defownr)))
		F(("No %s account, Check passwd file\n",defownr));

	Defownr.name = defownr;
	Defownr.uid = rpe->uid;
	Defownr.gid = rpe->gid;

	/* Load system name file.  If it exists then we will issue
	 * take commands if necessary otherwise just list needed files.
	 */
	if (!ismac(REMKMAC)) {
		if (sys_id || (sys_id = loadfyl(SYSID,""))) {
			if (nflag) {
#ifdef UCB_NET
				for (cp=sys_id; *cp; cp++) ;
				while (!isprint(*--cp)) ;
				*++cp = '\0';
				sprintf(tkcmd,"remtake -i%s",sys_id);
				/*
				for (cp=tkcmd; *cp; cp++) ;
				while (!isprint(*--cp)) ;
				*++cp = '\0';
				*/
				if ((rem = rcmd(&host, IPPORT_CMDSERVER,
				   "usr68", "usr68", tkcmd, 0)) >= 0) {
					D(5,("Ethernet link established: rem desc=%d\n", rem))
					setmacro(REMKMAC,tkcmd);
				} else {
					D(5,("Ethernet link failure: rem desc=%d\n", rem))
					nflag = 0;
				}
#else
				nflag = 0;
#endif
			}
			if (nflag==0) {
				sprintf(tkcmd,"take -i%s",sys_id);
				for (cp=tkcmd; *cp; cp++) ;
				while (!isprint(*--cp)) ;
				*++cp = '\0';
				setmacro(REMKMAC,tkcmd);
			}
		} else
			setmacro(REMKMAC,"install");
	}
	
#ifdef UniSoft
	define ("UniSoft");
#endif

readfile:
/****
	if (!Pflag && !fflag && access(ERRLIST,4)) {
		setfile(F_FILENAME, ERRLIST);
		while (cp = getspec())
			chkentry(cp);
	}
****/

	setfile(F_FILEPTR, stdin);
	if ((sbp = isfile(stdin)) == 0 && !fflag)
		fflag = DEFTREE;

	if (fflag && (fflag[0] != '-' || fflag[1] != '\0')) {
		if (sbp)
			F(("Cannot have both `-f %s' and stdin files\n",fflag));
		setfile(F_FILENAME,fflag);
	}

	while (cp = getspec())
		if (!Pflag) chkentry(cp);	/* vchk */

fini:
	if (idlev) X(("EOF: unmatched .ifdef\n"));
	prtrmdr(first_node);
	unresolved();
	D(0,("\nVchk Done -- used %d bytes\n",sbrk(0)-initbrk))
	exit(0);
}

/* Parse arguments: Convert `name [ -flags ... args ... ] ...'
 * to `args ...' for later processing.
 */
args(flg, c, l)
	char *l[];
{
	char **ap, *cp, *mp;
	int i;
	char mbuf[10];

	ap = l;
	for (i=0; i<c; i++) {
		if (*(cp = l[i]) == '-') {
			while (*++cp) switch (*cp) {
			when 'a': aflag = 1;		/* check everything */
			when 'b': if (!flg) bflag = 1;	/* build */
				  else usage(flg,'b');
			when 'c': cflag = 1;		/* print commands */
			when 'd': dflag = 1;		/* fix mode&owner only*/
			when 'e': eflag = 1;		/* chk exists only */
			when 'f': if (!(fflag=l[++i]))	/* alt tree */
					usage(flg,'f');
			when 'i': iflag = 1;		/* go interactive */
			when 'k': kflag = 1;		/* do checksums */
			when 'l': lflag = 1;		/* don't read dirs */
			when 'm': mflag = 1;		/* multi system */
			when 'n': nflag = 1;		/* ethernet */
			when 'p': if (!flg) 		/* force passwd chk */
					pflag = ++cp;
				  else usage(flg,'p');
				  goto esc;
			when 'r': if (!flg) rflag = 1;	/* leave dup accts */
				  else usage(flg,'r');
			when 's': sflag = 1;		/* silent */
			when 'v': vflag = 1;		/* quiet about trivia */
			when 'x': xflag = 1;		/* execute */
			when 'y': yflag = 1;		/* sync after remake */
			when 'A': if (!(sys_id=l[++i]))	/* alt sys_id */
					usage(flg,'A');
			when 'B': if (!flg) Bflag = 1;	/* rebuild tree */
				  else usage(flg,'B');
#ifdef DEBUG
			when 'D': Dflag++;		/* debugging */
#endif
			when 'I': Iflag = 1;		/* Ignore */
			when 'P': Pflag = ++cp;		/* Preprocess only */
				  goto esc;
			when 'S': Sflag = 1;		/* Silent */
			otherwise:usage(flg,*cp);
			}
		} else if (*cp == '+') {
			while (*++cp) switch (*cp) {
			when 'a': aflag = 0;		/* all (everything) */
			when 'b': if (!flg) usage(flg,'b');
			when 'c': cflag = 0;		/* print commands */
			when 'd': dflag = 0;		/* fix mode&owner only*/
			when 'e': eflag = 0;		/* chk exists only */
			when 'i': iflag = 0;		/* go interactive */
			when 'k': kflag = 0;		/* do checksums */
			when 'l': lflag = 0;		/* don't read dirs */
			when 'm': mflag = 0;		/* multi system */
			when 'p': if (!flg) usage(flg,'p');
			when 'r': if (!flg) usage(flg,'r');
			when 's': sflag = 0;		/* silent */
			when 'v': vflag = 0;		/* quiet about trivia */
			when 'x': xflag = 0;		/* execute */
			when 'y': yflag = 0;		/* no syncing */
#ifdef DEBUG
			when 'D': Dflag--;		/* debugging */
#endif
			when 'I': Iflag = 0;		/* ignore */
			when 'S': Sflag = 0;		/* Silent */
			otherwise:usage(flg,*cp);
			}
		} else *ap++ = cp;
	esc: ;
	}
	*ap = '\0';

	if (iflag && !tty) {
		if ((tty = open("/dev/tty",2)) == -1)
			E(("can't setup terminal for input\n"));
	}
	if (bflag) {
		Pflag = "";
		if (cflag || xflag || rflag)
			usage(flg,-3);
	}
	c = ap - l;		/* reset arg count */
	i = 1;
	while (c > 0) {
		if (cp = pindex(*l,'=')) {
			mp = cp;
			while (*--mp == ' ') *mp = '\0';
			mp = *l;
			*cp++ = '\0';
			while (*cp == ' ') *cp++ = '\0';
		} else {
			define(*l);
			sprintf(mbuf,"%d",i++);
			cp = mbuf;
			mp = *l;
		}
		setmacro(mp,cp);
		l++;
		c--;
	}
}

usage (flg, c)
{
	switch (c) {
	when 'f': case 't':
		X(("`t' and `f' options require a filename\n"));
	when 'b': case 'p': case 'r': case 'B':
		X(("cannot reset the `%c' option while running\n",c));
	when 'A':
		X(("`A' option requires a sys_id\n"));
	when '\0':
		X(("Usage: vchk [ -abcdefiklmprsvxDBIPS ] ... [ args ] ...\n"));
	when -3:
		X(("-b option does not make sense with any of [crx]\n"));
	otherwise:
		X(("`%c' is not a recognized option\n",c));
	}
	if (!flg) {
		if (c) usage(0,0);
		exit(1);
	}
	return;
}

#ifdef JUNK
setsys(sn)
	char *sn;
{
	char ****lf, ****vp, ***kp, **ap;

	if ((lf = (char ****)loadfyl(OEMLIST,"\n:|")) == 0)
		F(("Cannot load user list[%s]\n",OEMLIST));
	for (vp=lf; *vp; vp++) {
		for (kp= *vp; *kp; kp++) ;
		if (kp - *vp != 4)
			F(("Line %d of `%s' is invalid\n",vp-lf+1,OEMLIST));
		D(1,("comparing %s and %s\n",sn,***vp));
		if (strcmp(***vp,sn) == 0) {
			mkmacro(vp[0][1],SYSNAME);
			mkmacro(vp[0][2],RELOC);
			for (ap=vp[0][3]; *ap; ap++)
				define(*ap);
			break;
		}
	}
	if (!*vp)
		F(("`%s' is not in the distribution list[%s]\n",sn,OEMLIST));
	free(lf);
}

mkmacro (wl, mn)
	char **wl, *mn;
{
	char buf[PATHSIZ];
	char *bp = buf;
	int i;

	for (i=0; wl[i]; i++)
		;
	if (i > 1) {
		*bp++ = LSQIG;
		for (i=0; wl[i]; i++) {
			bp += cps(bp,wl[i]);
			if (wl[i+1]) *bp++ = ',';
		}
		*bp++ = RSQIG;
		*bp = '\0';
	} else
		cps(bp,wl[0]);
	setmacro(mn,buf);
}
#endif
