/*	@(#)defs.c	1.2	*/
#include	"defs.h"

/* result type declarations */
ADDRESS		alloc();
VOID		addblok();
STRING		make();
STRING		movstr();
TREPTR		cmd();
TREPTR		makefork();
NAMPTR		lookup();
VOID		setname();
VOID		setargs();
DOLPTR		useargs();
REAL		expr();
STRING		catpath();
STRING		getpath();
STRING		*scan();
STRING		mactrim();
STRING		macro();
STRING		execs();
VOID		await();
VOID		post();
STRING		copyto();
VOID		exname();
STRING		staknam();
VOID		printnam();
VOID		printflg();
VOID		prs();
VOID		prc();
VOID		getenv();
STRING		*setenv();


/* temp files and io */
UFD		output = 2;
INT		ioset;
IOPTR		iotemp;		/* files to be deleted sometime */
IOPTR		iopend;		/* documents waiting to be read at NL */
INT	stripflg;
/* substitution */
INT		dolc;
STRING		*dolv;
STRING		cmdadr;
DOLPTR		argfor;
ARGPTR		gchain;

/* stack */


/* string constants */
extern	MSG		atline;
extern	MSG		readmsg;
extern	MSG		colon;
extern	MSG		minus;
extern	MSG		nullstr;
extern	MSG		sptbnl;
extern	MSG		unexpected;
extern	MSG		endoffile;
extern	MSG		synmsg;

/* name tree and words */
extern	SYSTAB		reserved;
INT		wdval;
INT		wdnum;
ARGPTR		wdarg;
INT		wdset;
BOOL		reserv;

/* prompting */
extern	MSG		stdprompt;
extern	MSG		supprompt;
extern	MSG		profile;
extern	MSG		sysprofile;
/* built in names */
extern	NAMNOD		fngnod;
extern	NAMNOD		ifsnod;
extern	NAMNOD		homenod;
extern	NAMNOD		mailnod;
extern	NAMNOD		pathnod;
extern	NAMNOD		ps1nod;
extern	NAMNOD		ps2nod;

/* special names */
extern	MSG		flagadr;
STRING		exitadr;
STRING		dolladr;
STRING		pcsadr;
STRING		pidadr;

extern	MSG		defpath;

/* names always present */
extern	MSG		mailname;
extern	MSG		homename;
extern	MSG		pathname;
extern	MSG		fngname;
extern	MSG		ifsname;
extern	MSG		ps1name;
extern	MSG		ps2name;

/* transput */
extern	CHAR		tmpout[];
STRING		tmpnam;
INT		serial;
extern	FILE		standin;
INT		peekc;
STRING		comdiv;
extern	MSG		devnull;

/* flags */
INT		flags;
INT		rwait;	/* flags read waiting */

/* error exits from various parts of shell */
jmp_buf		subshell;
jmp_buf		errshell;

/* fault handling */
extern	POS	brkincr;


VOID		fault();
BOOL		trapnote;
extern	STRING		trapcom[];
extern	BOOL		trapflg[];

/* name tree and words */
extern	STRING		*environ;
extern	CHAR		numbuf[];
extern	MSG		export;
extern	MSG		duperr;
extern	MSG		readonly;

/* execflgs */
INT		exitval;
BOOL		execbrk;
INT		loopcnt;
INT		breakcnt;

/* messages */
extern	MSG		mailmsg;
extern	MSG		coredump;
extern	MSG		badopt;
extern	MSG		badparam;
extern	MSG		unset;
extern	MSG		badsub;
extern	MSG		nospace;
extern	MSG		notfound;
extern	MSG		badtrap;
extern	MSG		baddir;
extern	MSG		badshift;
extern	MSG		illegal;
extern	MSG		restricted;
extern	MSG		execpmsg;
extern	MSG		notid;
extern	MSG		badulimit;
extern	MSG		wtfailed;
extern	MSG		badcreate;
extern	MSG		nofork;
extern	MSG		noswap;
extern	MSG		piperr;
extern	MSG		badopen;
extern	MSG		badnum;
extern	MSG		arglist;
extern	MSG		txtbsy;
extern	MSG		toobig;
extern	MSG		badexec;
extern	MSG		badfile;

/*	'builtin' error messages	*/

extern	MSG		btest;
extern	MSG		badop;

/*	fork constant	*/
/*	comment delimeter 	*/

extern	address	end[] ;


INT		wasintr;	/* used to tell if break or delete is hit
					   while executing a wait		   */

INT		eflag;

/*	The following stuff is from stak.h	*/

STKPTR	stakbas ;
STKPTR	staktop ;
BLKPTR	stakbsy ;
STKPTR	brkend ;
