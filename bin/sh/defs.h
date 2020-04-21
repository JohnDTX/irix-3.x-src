/*
 *	UNIX shell
 */
/*	@(#)defs.h	1.2	*/
/*	3.0 SID #	1.2	*/
/*
 * @(#)$Header: /d2/3.7/src/bin/sh/RCS/defs.h,v 1.1 89/03/27 14:55:35 root Exp $
 */
/*
 * $Log:	defs.h,v $
 * Revision 1.1  89/03/27  14:55:35  root
 * Initial check-in for 3.7
 * 
 * Revision 1.3  85/10/31  19:06:23  root
 * Changed MAXTRAP from 20 to 18 per SCR 718.
 * 
 * Revision 1.2  85/03/12  01:25:31  bob
 * Fixed SCR 628 by taking out false conditional code that prevented
 * login from being recognized and exec'ed without fork.
 * 
 */


/* error exits from various parts of shell */
#define ERROR	1
#define SYNBAD	2
#define SIGFAIL 2000
#define SIGFLG	1000

/* command tree */
#define FPRS	020
#define FINT	040
#define FAMP	0100
#define FPIN	0400
#define FPOU	01000
#define FPCL	02000
#define FCMD	04000
#define COMMSK	017

#define TCOM	0
#define TPAR	1
#define TFIL	2
#define TLST	3
#define TIF	4
#define TWH	5
#define TUN	6
#define TSW	7
#define TAND	8
#define TORF	9
#define TFORK	10
#define TFOR	11

/* execute table */
#define SYSSET	1
#define SYSCD	2
#define SYSEXEC	3
#define SYSNEWGRP 4
#define SYSTRAP	5
#define SYSEXIT	6
#define SYSSHFT 7
#define SYSWAIT	8
#define SYSCONT 9
#define SYSBREAK 10
#define SYSEVAL 11
#define SYSDOT	12
#define SYSRDONLY 13
#define SYSTIMES 14
#define SYSXPORT 15
#define SYSNULL 16
#define SYSREAD 17
#define	SYSTST	18
#ifdef RES	/*	exclude umask code	*/
#define SYSLOGIN	20	/*	include login code	*/
#else RES
#define SYSUMASK 20
#define SYSULIMIT 21
#define SYSLOGIN	22	/*	include login code	*/
#endif

/*	builtin table	*/

#define TEST	127
/* used for input and output of shell */
#define INIO 18
#define OTIO 19

/*io nodes*/
#define USERIO	10
#define IOUFD	15
#define IODOC	16
#define IOPUT	32
#define IOAPP	64
#define IOMOV	128
#define IORDW	256
#define INPIPE	0
#define OTPIPE	1

/* arg list terminator */
#define ENDARGS	0

#include	"mac.h"
#include	"mode.h"
#include	"name.h"


/*	error catching	*/
extern	INT	errno;
/* result type declarations */
#define alloc malloc
extern ADDRESS		alloc();
extern VOID		addblok();
extern STRING		make();
extern STRING		movstr();
extern TREPTR		cmd();
extern TREPTR		makefork();
extern NAMPTR		lookup();
extern VOID		setname();
extern VOID		setargs();
extern DOLPTR		useargs();
extern REAL		expr();
extern STRING		catpath();
extern STRING		getpath();
extern STRING		*scan();
extern STRING		mactrim();
extern STRING		macro();
extern STRING		execs();
extern VOID		await();
extern VOID		post();
extern STRING		copyto();
extern VOID		exname();
extern STRING		staknam();
extern VOID		printnam();
extern VOID		printflg();
extern VOID		prs();
extern VOID		prc();
extern VOID		getenv();
extern STRING		*setenv();

#define 	setflg	010
#define attrib(n,f)	(n->namflg |= f)
#define round(a,b)	(((int)((ADR(a)+b)-1))&~((b)-1))
#define closepipe(x)	(close(x[INPIPE]), close(x[OTPIPE]))
#define eq(a,b)		(cf(a,b)==0)
#define max(a,b)	((a)>(b)?(a):(b))
#define assert(x)	;

/* temp files and io */
extern UFD		output;
extern INT		ioset;
extern IOPTR		iotemp;		/* files to be deleted sometime */
extern IOPTR		iopend;		/* documents waiting to be read at NL */
extern INT	stripflg;
/* substitution */
extern INT		dolc;
extern STRING		*dolv;
extern STRING		cmdadr;
extern DOLPTR		argfor;
extern ARGPTR		gchain;

/* stack */
#define		BLK(x)	((BLKPTR)(x))
#define		BYT(x)	((BYTPTR)(x))
#define		STK(x)	((STKPTR)(x))
#define		ADR(x)	((char*)(x))

/* stak stuff */
#include	"stak.h"

/* string constants */
extern MSG		atline;
extern MSG		readmsg;
extern MSG		colon;
extern MSG		minus;
extern MSG		nullstr;
extern MSG		sptbnl;
extern MSG		unexpected;
extern MSG		endoffile;
extern MSG		synmsg;

/* name tree and words */
extern SYSTAB		reserved;
extern INT		wdval;
extern INT		wdnum;
extern ARGPTR		wdarg;
extern INT		wdset;
extern BOOL		reserv;

/* prompting */
extern MSG		stdprompt;
extern MSG		supprompt;
extern MSG		profile;
extern MSG		sysprofile;

/* built in names */
extern NAMNOD		fngnod;
extern NAMNOD		cdpnod;
extern NAMNOD		ifsnod;
extern NAMNOD		homenod;
extern NAMNOD		mailnod;
extern NAMNOD		pathnod;
extern NAMNOD		ps1nod;
extern NAMNOD		ps2nod;

/* special names */
extern MSG		flagadr;
extern STRING		exitadr;
extern STRING		dolladr;
extern STRING		pcsadr;
extern STRING		pidadr;

extern MSG		defpath;

/* names always present */
extern MSG		mailname;
extern MSG		homename;
extern MSG		pathname;
extern MSG		fngname;
extern MSG		cdpname;
extern MSG		ifsname;
extern MSG		ps1name;
extern MSG		ps2name;

/* transput */
extern CHAR		tmpout[];
extern STRING		tmpnam;
extern INT		serial;
#define		TMPNAM 7
extern FILE		standin;
#define input	(standin->fdes)
#define eof	(standin->feof)
extern INT		peekc;
extern STRING		comdiv;
extern MSG		devnull;

/* flags */
#define		noexec	01
#define		sysflg	01
#define		intflg	02
#define		prompt	04
#define		errflg	020
#define		ttyflg	040
#define		forked	0100
#define		oneflg	0200
#define		rshflg	0400
#define		waiting	01000
#define		stdflg	02000
#define		STDFLG	's'
#define		STDFLGLOC	4
#define		execpr	04000
#define		readpr	010000
#define		keyflg	020000
extern INT		flags;
extern INT		rwait;	/* flags read waiting */

/* error exits from various parts of shell */
#include	<setjmp.h>
extern jmp_buf		subshell;
extern jmp_buf		errshell;

/* fault handling */
#include	"brkincr.h"
extern POS	brkincr;
#define MINTRAP	0
#define MAXTRAP	18	/* 1 more than allowed -> SIGCLD */

#define INTR	2
#define QUIT	3
#define MEMF	11
#define ALARM	14
#define KILL	15
#define TRAPSET	2
#define SIGSET	4
#define SIGMOD	8
#define SIGCAUGHT	16

extern VOID		fault();
extern BOOL		trapnote;
extern STRING		trapcom[];
extern BOOL		trapflg[];

/* name tree and words */
extern STRING		*environ;
extern CHAR		numbuf[];
extern MSG		export;
extern MSG		duperr;
extern MSG		readonly;

/* execflgs */
extern INT		exitval;
extern BOOL		execbrk;
extern INT		loopcnt;
extern INT		breakcnt;

/* messages */
extern MSG		mailmsg;
extern MSG		coredump;
extern MSG		badopt;
extern MSG		badparam;
extern MSG		unset;
extern MSG		badsub;
extern MSG		nospace;
extern MSG		notfound;
extern MSG		badtrap;
extern MSG		baddir;
extern MSG		badshift;
extern MSG		illegal;
extern MSG		restricted;
extern MSG		execpmsg;
extern MSG		notid;
extern MSG		badulimit;
extern MSG		wtfailed;
extern MSG		badcreate;
extern	MSG		nofork;
extern	MSG		noswap;
extern MSG		piperr;
extern MSG		badopen;
extern MSG		badnum;
extern MSG		arglist;
extern MSG		txtbsy;
extern MSG		toobig;
extern MSG		badexec;
extern MSG		badfile;

/*	'builtin' error messages	*/

extern MSG		btest;
extern MSG		badop;

/*	fork constant	*/
#define	FORKLIM	32
/*	comment delimeter 	*/

#define	COMCHAR	'#'
extern address	end[] ;

#include	"ctype.h"

extern	INT		wasintr;	/* used to tell if break or delete is hit
					   while executing a wait		   */
extern	INT		eflag;

