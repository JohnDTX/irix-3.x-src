/* @(#)uucp.h	1.8 */
#include "stdio.h"
/* define DATAKIT if datakit is available */

/* define DIALOUT if new dialout routine is available */

/* define XNS if XNS should be available */
#define	XNS

/* define the last characters for ACU */
#define ACULAST "<"

/* define UNAME if uname() should be used to get uucpname */
#define UNAME

/* define the value of WFMASK - for umask call - used for all uucp work files */
#define WFMASK 0177

/* define UUSTAT if you need "uustat" command */
#define UUSTAT
 
/*	define UUSUB if you need "uusub" command */
#define UUSUB

#ifdef	XNS
/* use this in "baud rate" column of L-devices */
#define	XNSCLASS "xns"
/* any old unique number */
#define	XNSSPEED 30000
/* must do ALL reads of xns fd with counts of 512 && buffer! Ugh! */
/* #define	read	Read */
/* #define	write	Write */
#endif


/*
 * some system names
 * put in local uucp name of this machine
 */
#ifndef	UNAME
#define MYNAME		"mhtsa"
#endif

#define XQTDIR		"/usr/lib/uucp/.XQTDIR"
#define SQFILE		"/usr/lib/uucp/SQFILE"
#define SQTMP		"/usr/lib/uucp/SQTMP"
#define SLCKTIME	5400	/* system/device timeout (LCK.. files) */
#define SEQFILE		"/usr/lib/uucp/SEQF"
#define SYSFILE		"/usr/lib/uucp/L.sys"
#define DEVFILE		"/usr/lib/uucp/L-devices"
#define DIALFILE	"/usr/lib/uucp/L-dialcodes"
#define USERFILE	"/usr/lib/uucp/USERFILE"
#define FWDFILE		"/usr/lib/uucp/FWDFILE"
#define ORIGFILE	"/usr/lib/uucp/ORIGFILE"
#define CMDFILE		"/usr/lib/uucp/L.cmds"
#define ADMIN		"/usr/lib/uucp/ADMIN"

#define SPOOL		"/usr/spool/uucp"
#define LOGDIR		"/usr/spool/uucp"
#define SQLOCK		"/usr/spool/uucp/LCK.SQ"
#define SYSLOG		"/usr/spool/uucp/SYSLOG"
#define LOGDEL		"/usr/spool/uucp/LOGDEL"
#define LOGWRN		"/usr/spool/uucp/LOGWRN"
#define PUBDIR		"/usr/spool/uucppublic"

#define SEQLOCK		"LCK.SEQL"
#define CMDPRE		'C'
#define DATAPRE		'D'
#define XQTPRE		'X'

#define LOGPREFIX	"LOG."
#define LOGLOCK	"/usr/spool/uucp/LCK.LOG"
#define LOGFILE	"/usr/spool/uucp/LOGFILE"
#define ERRLOG	"/usr/spool/uucp/ERRLOG"

#define RMTDEBUG	"AUDIT"
#define SQTIME		60
#define TRYCALLS	2	/* number of tries to dial call */

/*define PROTODEBUG = 1 if testing protocol - introduce errors */
#define DEBUG(l, f, s) if (Debug >= l) fprintf(stderr, f, s)

#define ASSERT(e, s1, s2, i1) if (!(e)) {\
assert(s1, s2, i1);\
cleanup(FAIL);};


#define SAME 0
#define ANYREAD 04
#define ANYWRITE 02
#define FAIL -1
#define SUCCESS 0
#define CNULL (char *) 0
#define STBNULL (struct sgttyb *) 0
#define MASTER 1
#define SLAVE 0
#define MAXFULLNAME 100
/* #define MAXMSGTIME 45 */
#define MAXMSGTIME 25
#define MAXCHARTIME 15
#define MAXBASENAME 14			/* can't use DIRSIZ any more */
#define NAMESIZE (MAXBASENAME+1)
#define EOTMSG "\004\n\004\n"
#define CALLBACK 1


/*
 * commands
 */
#define SHELL		"/bin/sh"
#define MAIL		"mail"
#define UUCICO		"/usr/lib/uucp/uucico"
#define UUXQT		"/usr/lib/uucp/uuxqt"
#define UUCP		"uucp"



/*
 * call connect fail stuff
 */
#define CF_SYSTEM	-1
#define CF_TIME		-2
#define CF_LOCK		-3
#define CF_DIAL		-5
#define CF_LOGIN	-6


/*
 * system status stuff
 */
#define SS_OK		0
#define SS_FAIL		4
#define SS_DFAIL	6
#define SS_NODEVICE	1
#define SS_CALLBACK	2
#define SS_INPROGRESS	3
#define SS_BADSEQ	5


/*
 * fail/retry parameters
 */
#define RETRYTIME 3300
#define MAXRECALLS 10


/*
 * stuff for command execution
 */
#define X_RQDFILE	'F'
#define X_STDIN		'I'
#define X_STDOUT	'O'
#define X_CMD		'C'
#define X_USER		'U'
#define X_NONOTI	'N'
#define X_NONZERO	'Z'	/* don't notify if zero return */
#define X_SENDFILE	'S'
#define X_MAILF	'M'
#define X_LOCK		"LCK.XQT"
#define X_LOCKTIME	3600

#define WKDSIZE 100	/*  size of work dir name  */
extern int Ifn, Ofn;
extern char **Env;
extern char Rmtname[];
extern char User[];
extern char Uucp[];
extern char Loginuser[];
extern char *Spool;
extern char Myname[];
extern int Debug;
extern int Pkdrvon;
extern char Wrkdir[];
extern long Retrytime;
extern short Usrf;
extern char Progname[];
extern char Pchar;
extern char Pprot[];

/*
 * Statistics
 */
/*
 * Structure returned by times()
 */
struct tms {
	long	tms_utime;		/* user time */
	long	tms_stime;		/* system time */
	long	tms_cutime;		/* user time, children */
	long	tms_cstime;		/* system time, children */
};
struct nstat{
	int	t_pid;		/* process id				*/
	long	t_start;	/* process id				*/
	long	t_beg;		/* start  time			*/
	long	t_scall;	/* start call to system			*/
	long	t_ecall;	/* end call to system			*/
	long	t_tacu;		/* acu time				*/
	long	t_tlog;		/* login time				*/
	long	t_sftp;		/* start file transfer protocol		*/
	long	t_sxf;		/* start xfer 				*/
	long	t_exf;		/* end xfer 				*/
	long	t_eftp;		/* end file transfer protocol		*/
	long	t_qtime;	/* time file queued			*/
	int	t_ndial;	/* # of dials				*/
	int	t_nlogs;	/* # of login trys			*/
	struct tms t_tbb;	/* start execution times		*/
	struct tms t_txfs;	/* xfer start times			*/
	struct tms t_txfe;	/* xfer end times 			*/
	struct tms t_tga;	/* garbage execution times		*/
};
extern struct nstat nstat;
extern char	dn[];			/* auto dialer name			*/
extern char	dc[];			/* line name				*/
extern char	Fwdname[];		/* foward name				*/
extern int	seqn;			/* sequence #				*/
extern long	tconv;			/* conversation time			*/
extern int	jobid;
extern char	subjob[2];
#define updjb()	if(++subjob[0] == 0) ++subjob[0];
extern int Role;
long	times();
long	time();
#define	TRUE	1
#define	FALSE	0
#define FOWARD	1
#define NAMEBUF	32
#define DFILMODE	0600
#define dlogent(a, b)	if(Debug)logent(a, b)
#define NDLINES		30
#define ACULTNCY	(2*60)
#define SYSNSIZE	8
