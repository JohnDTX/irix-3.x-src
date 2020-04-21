/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/* @(#)file.c	1.6 */
#include "vchk.h"
int LLno;
extern int mustdef;
extern int musteval;

FILE *popen(), *fopen();

/* The routines in this file are listed below.  They implement a generalized
 * file stack.  Input streams are pushed, poped, and set.  When an eof
 * is read that input stream is poped.  Only when the bottom of the stack is
 * reached will getchr() return eof.
 * Input can come from a file, pipe, or macro.
 * In all the calls below `type' is one of
 *	F_FILE,	      F_FILENAME,    F_COMMAND,		F_STRING
 * and tells the routine whether `thing' is a
 *	file pointer, file name,    command string, or input string.
 *
 *	setfile (type, thing)	pop current file (if any) and use thing.
 *	pushfile(type, thing)	push thing onto file stack.
 *	popfile()		pops the current input file.
 *	getchr()		returns the next character from the top file
 *	peek()			like getchr() but doesn't actually read it.
 */

struct fstack {
	unsigned int	f_type:2,		/* string, cmd, or file */
			f_gotsome:1,		/* f_nextc is valid or not */
			f_eof:1,		/* read eof */
			f_opened:1,		/* file needs opening */
			f_named:13;		/* macro or file name */
	char		f_nextc;		/* next char to be read */
	char		*f_name;		/* ptr to name if f_named */
#ifdef UniSoft
	char		*f_buf;			/* malloc'd buffer for setbuf*/
#endif
	int		f_lno;			/* current line number */
	union f_p {
		FILE		*f_fp;		/* ptr to open file */
		char		*f_sp;		/* string pointer */
	}		f_p;
};
	/* values of f_type above */
#define FT_FILE	1
#define FT_STR	2
#define FT_CMD	3

#define FSTKSIZ 16
struct fstack fstack[FSTKSIZ];
struct fstack *cis;			/* Current input stream */
char cmd_val;				/* returned value from last execl */

#ifdef STDIO_BUG
DUMPCIS (fsp,str)
	struct fstack *fsp;
	char *str;
{
	int i;
	FILE *fd;
	char *die = 0;
	char *filetype(), *cvtfflg();

	for (i=0; i<FSTKSIZ; i++)
		if (fsp == &fstack[i]) goto ok;
	fprintf(stderr,"%s: Invalid file stack pointer 0x%x\n",str,fsp);
	*die = 1;	/* die violently */
ok:
	fprintf(stderr,"%s: DUMP OF %s %s STACK POINTER at 0x%x\n",
		str, nth(fsp - fstack), filetype(fsp->f_type), fsp);
	fprintf(stderr,"gotsome=%d  named=%d  eof=%d  opened=%d  nextc=%s ",
		fsp->f_gotsome, fsp->f_named, fsp->f_eof, fsp->f_opened,
		pchr(fsp->f_nextc));
	fprintf(stderr,"name=`%s'  lno=%d\n",fsp->f_name,fsp->f_lno);
	if (fsp->f_type == FT_FILE || fsp->f_type == FT_CMD) {
		fd = fsp->f_p.f_fp;
		for (i=0; i<_NFILE; i++)
			if (fd == &_iob[i]) goto good_fp;
		fprintf(stderr,"Invalid file pointer 0x%x\n",fd);
		*die = 1;	/* die violently */
	good_fp:
		fprintf(stderr,
			"%s fp slot: ptr=0x%x cnt=%d base=0x%x flag=%s fd=%d\n",
			nth(i), fd->_ptr, fd->_cnt, fd->_base,
			cvtfflg(fd->_flag), fd->_file);
	} else
		fprintf(stderr,"string begins at 0x%x: `%20s...'\n",
			fsp->f_p.f_fp, fsp->f_p.f_fp);
}

char *
filetype (t)
{
	if (t == FT_FILE) return "file";
	if (t == FT_CMD) return "file (popen)";
	if (t == FT_STR) return "string";
	return "INVALID FILETYPE";
}

char *
cvtfflg(flg)
{
	static char buf[10];
	char *bp = buf;

	if ((flg & _IORW) != 0) *bp++ = 'm'; else *bp++ = '.';
	if ((flg & _IOSTRG) != 0) *bp++ = 's'; else *bp++ = '.';
	if ((flg & _IOERR) != 0) *bp++ = 'e'; else *bp++ = '.';
	if ((flg & _IOEOF) != 0) *bp++ = 'o'; else *bp++ = '.';
	if ((flg & _IOMYBUF) != 0) *bp++ = 'y'; else *bp++ = '.';
	if ((flg & _IONBF) != 0) *bp++ = 'u'; else *bp++ = '.';
	if ((flg & _IOWRT) != 0) *bp++ = 'w'; else *bp++ = '.';
	if ((flg & _IOREAD) != 0) *bp++ = 'r'; else *bp++ = '.';
	*bp++ = 0;
	return buf;
}
#else
#define DUMPCIS(_x_,_y_)
#endif

setfile (t, n)
	union f_p n;
{
	if (cis) popfile();
	pushfile(t, n);
}

#define FNBS 200
char fn_buf[FNBS];
char *nbufp = fn_buf;

pushfile (t, n)
	union f_p n;
{
	int len;
	char *getmem();
	static char blk_buf[BUFSIZ];

	if (!cis) {			/* initial open */
		cis = &fstack[-1];
	} else {
		cis->f_lno = Lno;	/* save current line number */
	}

	if (++cis >= &fstack[FSTKSIZ])
		E(("input file stack[%d levels] overflow\n",FSTKSIZ));

	cis->f_gotsome = 0;
	cis->f_eof = 0;
	cis->f_nextc = 0;
	cis->f_lno = 0;
#ifdef UniSoft
	cis->f_buf = (char *)0;
#endif

	switch (t) {
	when F_FILEPTR:
		cis->f_type = FT_FILE;
		cis->f_named = 0;
		cis->f_opened = 0;
		cis->f_name = 0;
		cis->f_p.f_fp = n.f_fp;
	when F_FILENAME:
		cis->f_type = FT_FILE;
		cis->f_opened = 1;
		len = strlen(n.f_sp) + 1;
		if (nbufp + len > fn_buf + sizeof(fn_buf))
			E(("Out of space[%d chars] to save file names\n",FNBS));
		cis->f_name = nbufp;
		nbufp += (cis->f_named = cps(nbufp,n.f_sp));
		*nbufp++ = '\0';
		if ((cis->f_p.f_fp = fopen(cis->f_name,"r")) == 0)
			E(("Cannot fopen(%s): %s\n",cis->f_name,SE));
#ifdef UniSoft
		cis->f_buf = getmem(BUFSIZ,0,"io buffer",cis->f_name);
		setbuf(cis->f_p.f_fp,cis->f_buf);
#endif UniSoft
	when F_COMMAND:
		cis->f_type = FT_CMD;
		cis->f_named = 0;
		cis->f_opened = 1;
		cis->f_name = 0;
		if ((cis->f_p.f_fp = popen(n.f_sp,"r")) == 0)
			E(("Cannot popen(%s): %s\n",n.f_sp,SE));
	when F_STRING:
		if (n.f_sp == 0 || !(cis->f_nextc = *n.f_sp++)) {
			n.f_sp = "";
			cis->f_nextc = 0;
		}
		cis->f_gotsome = 1;
		cis->f_type = FT_STR;
		cis->f_named = 0;
		cis->f_opened = 0;
		cis->f_name = 0;
		cis->f_p.f_sp = n.f_sp;
	}
	Lno = 0;
	/*
	DUMPCIS(cis,"pushfile");
	*/
}

popfile()
{
/*	DUMPCIS(cis,"popfile");*/
	if (cis == 0 || cis < fstack)
		return 0;
	if (cis->f_opened) {
		if (cis->f_type == FT_CMD) cmd_val = pclose(cis->f_p.f_fp);
		else fclose(cis->f_p.f_fp);
	}
	if (cis->f_named)
		nbufp -= (cis->f_named + 1);
#ifdef UniSoft
	if (cis->f_buf) {	/* free malloc'd setbuf buffer */
		free(cis->f_buf);
		cis->f_buf = 0;
	}
#endif UniSoft
	LLno = Lno;
	Lno = 0;
	if (--cis >= fstack)
		Lno = cis->f_lno;
	else cis = 0;
	return 1;
}

char
getchr()
{
	register FILE *fp;
	register struct fstack *lcis = cis;
	register char nc;
	register char c;

/*	DUMPCIS(lcis,"in getchr");*/

	if (!lcis || lcis < fstack) {	/* file unopened so far */
		return 0;
	}

	if (lcis->f_gotsome)
		c = lcis->f_nextc;
	else
		c = 0;

	if (lcis->f_type == FT_FILE || lcis->f_type == FT_CMD) {
		fp = lcis->f_p.f_fp;
#ifdef BUGCHECK
		if (!fp) E(("NULL FILE POINTER\n"));
#endif BUGCHECK
		if ((nc = getc(fp)) == EOF /*|| feof(fp) || ferror(fp)*/) {
			nc = 0;
			lcis->f_eof = 1;
		}
	} else if (lcis->f_type == FT_STR) {
		if ((nc = *lcis->f_p.f_sp++) == '\0') {
			lcis->f_p.f_sp--;
			lcis->f_eof = 1;
		}
	}
#ifdef BUGCHECK
	else E(("LOGIC: file stack entry has invalid type\n"));
#endif BUGCHECK
	if (nc == '\n') Lno++;

	if (lcis->f_eof) {
		popfile();
		if (!c) c = getchr();
		return c;
	}

	lcis->f_nextc = nc;
	if (lcis->f_gotsome == 0) {		/* one char readahead */
		lcis->f_gotsome = 1;
		c = getchr();
	}
/*	DUMPCIS(lcis,"returning from getchr");*/
	return c;
}

char
peek()
{
	if (!cis || cis < fstack)	/* file unopened so far */
		return 0;

	if (cis->f_gotsome == 0) {
		cis->f_gotsome = 1;
		getchr();
	}
	return cis->f_nextc;
}

/* This procedure is used to get vchk control lines from the input file.
 * Gettext is used to get the commands since there strings are significant
 * and they might be altered here.
 */

char *
getline()
{	char buf[LINESIZ];
	static char rbuf[LINESIZ];
	register char *lp;
	char last_c, nc, *p;
	extern errflag;
	int eol, cment, mdef, meval;

	mdef = mustdef = 0;		/* saw unescaped = */
	meval = musteval = 0;		/* saw unescaped $ */
	cment = 0;			/* saw start of comment indicator */
	lp = rbuf;
	last_c = 0;
	eol = 0;
skipcmd:
	while (lp < rbuf + sizeof(rbuf)-2) {	/* -2 is for esc chars */
		if ((nc = getchr()) == 0) {
			*lp = '\0';
			if (lp != rbuf)
				X(("partial line[%s] ignored\n",rbuf));
			return 0;
		}
		if ((lp == rbuf) && (nc == ' ' || nc == '\t')) {
			if (Pflag && !skipping) putc(nc,stdout);
			while (nc = getchr()) {
				if (Pflag && !skipping) putc(nc,stdout);
				if (nc == '\\') {
					nc = getchr();
					if (Pflag && !skipping) putc(nc,stdout);
				} else if (nc == '\n') goto skipcmd;
			}
			return 0;
		}
		if (nc == '\n') {
			if ((nc = peek()) == ' ' || nc == '\t')
				*lp++ = ';';
			eol = 1;
			break;
		}
		if (nc == '\t') nc = ' ';
		if (nc == ' ' && last_c == ' ') continue;
		if (nc == '\\') {
			if ((nc = getchr()) == '\n') {
				while ((nc = getchr()) == ' ' || nc == '\t')
					;
			}
			else *lp++ = '\\';
		} else if (nc == '$') meval = 1;
		else if (nc == '=') mdef = 1;
		else if (nc == '#' && !cment) { 
			mustdef = mdef;
			musteval = meval;
			cment = 1;
		}
		last_c = (*lp++ = nc);
	}
	*lp = '\0';
	if (eol == 0) {
		sprintf(Err,"line too long");
		errflag = 1;
	}
	if (!cment) {
		mustdef = mdef;
		musteval = meval;
	}
	D(4,("INPUT LINE %d \"%s\"\n",Lno,rbuf));
	return rbuf;
}

char *
gettext()
{	char buf[LINESIZ];
	static char rbuf[LINESIZ];
	register char *lp;
	char nc;
	extern errflag;
	int sol, eol;

	lp = rbuf;
	eol = 0;
	sol = 1;
	while (lp < rbuf + sizeof(rbuf)-2) {	/* -2 is for esc chars */
		if ((nc = peek()) == 0) return 0;
		if (nc == '\n') {
			getchr();
			eol = 1;
			break;
		}
		if (sol && (nc != ' ' && nc != '\t'))
			return 0;
		sol = 0;
		nc = getchr();		/* nc doesn't change here */
		if (nc == '\\') {
			if ((nc = getchr()) == '\n') nc = ' ';
			else *lp++ = '\\';
		}
		*lp++ = nc;
	}
	*lp = '\0';
	if (eol == 0) {
		sprintf(Err,"command line too long");
		errflag = 1;
		if (sol) return 0;
	}
	D(1,("getline returning `%s'\n",rbuf));
	return rbuf;
}
