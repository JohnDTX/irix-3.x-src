/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/* @(#)specfnc.c	1.7 */
#include "vchk.h"

extern char *initbrk;
char *
getmem (b, mhf, mesg, thing)	/* size in bytes and must have flag */
	char *mesg, *thing;
{
	char *malloc(), *sbrk();
	extern char *end;
	char *rp, *lastbrk, *nbrk;

	lastbrk = sbrk(0);
	rp = malloc(b);
	nbrk = sbrk(0);
	D(1,("GETMEM(%d) %s[%s] ",b,mesg,thing))
	if (nbrk != lastbrk) D(1,("[adv brk %d bytes, using %d] ",nbrk-lastbrk,nbrk-initbrk))
	D(1,("%s\n",rp?"succeeded":"failed"))
	if (mhf && rp == (char *)0)
		E(("Memory allocate[%d bytes] failed %s\n",b,mesg));
	return rp;
}

char *cmdtype[] = {
	"echo", "chmod", "chown", "chgrp",
	"if",   "ln",    "mkdir", "mknod",
	"take", "rm",	 "install","remtake",
	0
};

#define ECHO	0
#define CHMOD	1
#define CHOWN	2
#define CHGRP	3

#define IF	4
#define LN	5
#define MKDIR	6
#define MKNOD	7

#define TAKE	8
#define RM	9
#define INSTALL	10
#define TAKEIT	11

/* Returns 1 if xflag (commands are executed) and the command failed.
 * Otherwise it returns 0.
 */
cexec (cmd)
	char *cmd;
{
	char **a = &cmd;
	static char eb[BUFSIZ];
	extern int tty;
	char *fn, *al;
	int spcf, nonl, i, gid, uid, mode;
	struct stat sb;
	char *index(), *mkdir();
	int saverrno;

	if (cmd) {
		sprintf(eb,a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8],a[9]);
		cmd = eb;
	} else {
		if (cflag) printf("%s\n",a[1]);
		if (xflag) return system(a[1]) ? 1 : 0;
		return 0;
	}

	if (iflag && !query(cmd))
		return cflag ?0 :1;

	if (cflag) printf("%s\n",cmd);
	if (!xflag) return 0;

	cmd = (char *)realname(cmd);
	if (*cmd == '/') {
		if (exec(cmd)) {
			X(("exec (%s) failed: %s\n",cmd,SE));
			return 1;
		}
		return 0;
	}

	al = index(cmd,' ');
	if (!al) {
		X(("Illegal command `%s' not executed\n",cmd));
		return 1;
	}
	*al++ = '\0';

	for (i=0; cmdtype[i]; i++)
		if (strcmp(cmdtype[i],cmd) == 0) break;

	if (!cmdtype[i]) {
		X(("Unknown command name `%s' not executed\n",cmd));
		return 1;
	}

	/*
	 * ease the net through
	 */
	if (i == TAKEIT)
		i = TAKE;

	switch (i) {
	when ECHO:
		nonl = 0;
		if (!strncmp("-n ",al,3)) {
			nonl = 1;
			al += 3;
		}
		X(("%s",al));
		if (!nonl) putc('\n',stderr);
	when CHMOD:
		saverrno = errno;
		sscanf(al,"%o",&mode);
		errno = saverrno;
		fn = index(al,' ');
		if (!fn) {
			X(("invalid chmod cmd not executed\n"));
			return 1;
		}
		*fn++ = '\0';
		if (chmod(fn,mode)) {
			X(("chmod (%s,%o): %s\n",fn,mode,SE));
			return 1;
		}
	when CHOWN:
		fn = index(al,' ');
		if (!fn) {
			X(("invalid chown cmd not executed\n"));
			return 1;
		}
		*fn++ = '\0';
		if (isdigit(*al)) uid = atoi(al); else uid = acctuid(al);
		if (uid < 0) {
			X(("`%s' is not a valid account: chown(%s) not executed\n",al,fn));
			return 1;
		}
		if (stat(fn,&sb)) {
			X(("chown(%s): not performed: %s\n",fn,SE));
			return 1;
		}
		if (chown(fn,uid,sb.st_gid)) {
			X(("chown (%s,%d,%d): %s\n", fn, uid, sb.st_gid, SE));
			return 1;
		}
	when CHGRP:
		fn = index(al,' ');
		if (!fn) {
			X(("invalid chgrp cmd not executed\n"));
			return 1;
		}
		*fn++ = '\0';
		if (!isdigit(*al)) {
			X(("Don't use group names in chgrp cmds\n"));
			return 1;
		}
		gid = atoi(al);
		if (stat(fn,&sb)) {
			X(("chgrp(%s): not performed: %s\n",fn,SE));
			return 1;
		}
		if (chown(fn,sb.st_uid,gid)) {
			X(("chown (%s,%d,%d): %s\n", al, sb.st_uid, gid, SE));
			return 1;
		}
	when IF:
		al[-1] = ' ';
		system(cmd);
	when LN:
		fn = index(al,' ');
		if (!fn) {
			X(("invalid ln cmd not executed\n"));
			return 1;
		}
		*fn++ = '\0';
		if (link(al,fn)) {
			X(("link (%s,%s): %s\n",al,fn,SE));
			return 1;
		}
	when MKDIR:
		if (fn = mkdir(al)) {
			X(("mkdir(%s): %s\n",al,fn));
			return 1;
		}
	when MKNOD:
		X(("Cannot exec \"%s\" myself\n",cmd));
		return 1;
	when TAKE:
		al[-1] = ' ';
		while (fn = index(al,' '))	/* find file name */
			al = fn+1;
		tsavf(al);			/* move the existing file */
#ifdef UCB_NET
		if (nflag) {
			if (getnet(al)) {
				X(("Warning: network take failed\n"));
				while (fn = index(al,' '))
					al = fn+1;
				cexec("rm %s",al);
				trstf();	/* restore the saved file */
				return 1;
			}
		} else
#endif
		if (i = exec(cmd)) {
			X(("Warning: `%s' returned status %d\n",cmd,i));
			while (fn = index(al,' '))
				al = fn+1;
			cexec("rm %s",al);
			trstf();		/* restore the saved file */
			return 1;
		}
		trmf();				/* remove saved file */
		if (yflag) sync();
	when RM:
		if (unlink(al)) {
			X(("unlink(%s): %s\n",al,SE));
			return 1;
		}
	when INSTALL:
		al[-1] = ' ';
		tsavf(al);			/* move the existing file */
		if (system(cmd))
			trstf();		/* restore the saved file */
		else
			trmf();			/* remove saved file */
		if (yflag) sync();
	otherwise:
		E(("Built in command table does not agree with switch loop in \"cexec(specfnc.c)\"\n"));
		return 1;
	}
	return 0;
}

/* Save the file about to be remade so that it can be restored if the remake
 * fails.
 */
char snbuf[PATHSIZ];
char tnbuf[PATHSIZ];
char Xnofile;				/* no file to save flag */
tsavf(fn)
	char *fn;
{
	int i, len, rlen;

	if (access(fn,0)) {
		Xnofile = 1;
		return;
	}
	Xnofile = 0;
	len = cps(snbuf,fn);		/* save name of file */
	rlen = cps(tnbuf,fn);		/* save name of temp link */
	while (--len > 0)
		if (snbuf[len] == '/')
			break;
	len++;
	i = 0;
	while (i < 14) {
		tnbuf[len+i] = 'a';
		while (!access(tnbuf,0))
			if (tnbuf[len+i]++ >= 'z')
				goto next;
		D(4,("Saving %s in %s till REMAKE finishes\n",snbuf,tnbuf))
		if (link(snbuf,tnbuf)) {
			E(("Cannot save %s to remake a new copy: %s\n",fn,SE));
		} else unlink(snbuf);
		return;
	next:
		i++;
	}
	E(("CANNOT HAPPEN -- Cannot make unique temp name!!\n"));
	return;
}

trstf()
{
	if (Xnofile)
		return;
	D(4,("Restoring %s from saved copy in %s\n",snbuf,tnbuf))
	unlink(snbuf);
	link(tnbuf,snbuf);
	unlink(tnbuf);
}

trmf()
{
	if (Xnofile)
		return;
	D(4,("Removing %s, a saved copy of %s\n",tnbuf,snbuf))
	unlink(tnbuf);
}

query(msg)
	char *msg;
{
	char **a = &msg;
	char *cp, buf[BUFSIZ];
	static int inisok = 0;

	sprintf(buf,a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8],a[9]);
	cp = buf + strlen(buf);
	*cp++ = ' ';
	*cp++ = '?';
	*cp = '\0';
	write(tty,buf,cp-buf);

	if (read(tty,buf,BUFSIZ) <= 0) {
		if (inisok) F(("Vchk -- Terminating"));
		inisok = 1;
		if (xflag) {
			if (query("Should I continue to issue commands")) {
				if (cflag) {
					if (!query("And list them also"))
						cflag = 0;
				} else if (query("Want them printed just before they are executed"))
					cflag = 1;
			} else {
				xflag = 0;
				if (cflag) {
					if (!query("continue printing them"))
						cflag = 0;
				} else {
					if (query("start printing them instead"))
						cflag = 1;
				}
			}
		} else {
			if (cflag) {
				if (!query("Should I continue to print the commands"))
					cflag = 0;
			}
			if (bflag) {
				if (!query("Should I continue"))
					exit(0);
			}
		}
		M(("\nVchk -- terminating interactive operation\n"));
		iflag = 0;
		inisok = 0;
		return 1;
	}
	return ((buf[0] == 'y' || buf[0] == 'Y') ?1 :0);
}

#ifdef DEBUG
char *
typename (t)
{
	switch (t) {

	when DIRECTORY:
		return "directory";
	when ITEM:
		return "item";
	when CONTENTS:
		return "contents of a directory";
	when IGNORE:
		return "ignored";
	when ERROR:
		return "error";
	otherwise:
		return "???";
	}
}
#endif

outofdate (bin, desc)
	char *bin, *desc;
{
	char *bp, *dp;
	int b, d;

	/* if (cflag) return 1; */
	
/* Break off leading ascii (if any) from version numbers.
 */
	bp = bin;
	while (*bp && !isdigit(*bp))
		bp++;

	dp = desc;
	while (*dp && !isdigit(*dp))
		dp++;

	if (dp != desc && dp[-1] != '.') { /* bad tree (no verson number) */
		X(("Inappropriate version number in tree file\n"));
		return 1;
	}

	d = dp - desc;
	if (d && strncmp(desc,bin, d))	/* string mismatch means bad file */
		return 0;

loop:
	if (*dp == 0)
		return 1;	/* binary version number is longer, help! */
	if (*bp == 0)
		return 0;	/* tree file version number is longer */
	if (!isdigit(*dp)) {
		X(("Inappropriate version number in tree file\n"));
		return 1;
	}
	if (!isdigit(*bp)) return 0;	/* bad version number in binary */
	b = atoi(bp);
	d = atoi(dp);
	if (b < d) return 0;	/* binary out of date */
	if (b > d) return 1;	/* tree file out of date? */
	while (isdigit(*dp))
		dp++;
	if (*dp == '.')
		dp++;
	while (isdigit(*bp))
		bp++;
	if (*bp == '.')
		bp++;
	goto loop;
}

errout (x)
	char *x;
{
	register char **a = &x;
	fprintf(stderr,a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7]);
}

lerrout (x)
	char *x;
{
	register char **a = &x;
	if (Lno) fprintf(stderr,"Line %d: ",Lno);
	fprintf(stderr,a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7]);
}

oodm(f)
	char *f;
{
	static int saw = 0;
	static int lno = 0;

	if (Lno == lno || saw++ != 0) {
		lno = Lno;
		if (f != 0 || saw == 1) return;
		M(("%d lines of the treefile are out of date\n",saw-1));
	}
	lno = Lno;
		
	M(("The version number actually in `%s'\n",f));
	M(("is greater than what is specified in the tree file.\n"));
	M(("The description file may be out of date.\n"));
}
