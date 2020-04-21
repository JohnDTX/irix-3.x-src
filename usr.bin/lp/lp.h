/* "@(#)lp.h	3.1" */

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<dirent.h>
#include	<signal.h>
#include	<pwd.h>
#include	<ctype.h>
#include	<fcntl.h>
#include	<utmp.h>

#ifndef SPOOL
#define	SPOOL	"/usr/spool/lp"
#endif
#ifndef ADMIN
#define	ADMIN	"lp"
#endif
#ifndef ADMDIR
#define	ADMDIR	"/usr/lib"
#endif
#ifndef USRDIR
#define	USRDIR	"/usr/bin"
#endif

time_t time();
char *date(), *getname();
#define	SCCSID(id)	static char sccsid[] = id;
#define	TRUE	1
#define	FALSE	0

#define	CORMSG	"out of memory"
#define	ADMINMSG	"this command for use only by LP Administrators"
#define	ISADMIN	(getuid() == 0 || strcmp(getname(), ADMIN) == 0)

#define	TITLEMAX	80	/* maximum length of title */
#define	OPTMAX	512
#define	NAMEMAX	15	/* max length of simple filename + 1 */
#define	DESTMAX	NAMEMAX - 1	/* max length of destination name */
#define	FILEMAX	512	/* max pathname for file  */
#define	LOGMAX	15	/* maximum length of logname */

#define	MEMBER	"member"
#define	CLASS	"class"
#define	DEFAULT	"default"
#define	REQUEST	"request"
#define	INTERFACE	"interface"
#define	ERRLOG	"log"
#define	OLDLOG	"oldlog"
#define	FIFO	"FIFO"
#define	DISABLE	"disable"
#define	REJECT	"reject"
#define	LPDEST	"LPDEST"
#define	SCHEDLOCK	"SCHEDLOCK"

#define	QSTATUS	"qstatus"
#define	QSTATLOCK	"QSTATLOCK"

#define	SEQLOCK	"SEQLOCK"
#define	SEQFILE	"seqfile"
#define	SEQLEN	4	/* max length of sequence number */
#define	SEQMAX	10000	/* maximum sequence number + 1 */
#define	IDSIZE	DESTMAX+SEQLEN+1	/* maximum length of request id */
#define	RNAMEMAX	sizeof(REQUEST)+DESTMAX+DESTMAX+2

#define	OUTPUTQ	"outputq"
#define	TOUTPUTQ	"Toutputq"
#define	OUTQLOCK	"OUTQLOCK"

#define	PSTATUS	"pstatus"
#define	PSTATLOCK	"PSTATLOCK"

#define	LOCKTRIES	20
#define	LOCKSLEEP	5
#define	LOCKTIME	10L

#define	WAITTIME	"20"
#define	ALTIME	5

/* single-character commands for request file: */

#define	R_FILE	'F'	/* file name */
#define	R_MAIL	'M'	/* mail has been requested */
#define	R_WRITE	'W'	/* user wants message via write */
#define	R_TITLE	'T'	/* user-supplied title */
#define	R_COPIES	'C'	/* number of copies */
#define	R_OPTIONS	'O'	/* printer- and classs-dependent options */

#define	OSIZE	7
#define	PIDSIZE	5
struct outq {		/* output queue request */
	char o_dest[DESTMAX+1];	/* output destination (class or member) */
	char o_logname[LOGMAX+1];	/* logname of requester */
	int o_seqno;		/* sequence number of request */
	long o_size;		/* size of request -- # of bytes of data */
	char o_dev[DESTMAX+1];	/* if printing, the name of the printer.
				   if not printing, "-".  */
	time_t o_date;		/* date of entry into output queue */
	short o_flags;		/* See below for flag values */
};

/* Value interpretation for o_flags: */

#define	O_DEL	1		/* Request deleted */
#define	O_PRINT	2		/* Request now printing */

#define	Q_RSIZE	81

struct qstat {			/* queue status entry */
	char q_dest[DESTMAX+1];	/* destination */
	short q_accept;		/* TRUE iff lp accepting requests for dest,
				   otherwise FALSE.	*/
	time_t q_date;		/* date status last modified */
	char q_reason[Q_RSIZE];	/* if accepting then "accepting",
				   otherwise the reason requests for dest
				   are being rejected by lp.	*/
};

#define	P_RSIZE	81

struct pstat {		/* printer status entry */
	char p_dest[DESTMAX+1];	/* destination name of printer */
	int p_pid;		/* if busy, process id that is printing,
				   otherwise 0 */
	char p_rdest[DESTMAX+1];   /* if busy, the destination requested by
				   user at time of request, otherwise "-" */
	int p_seqno;		/* if busy, sequence # of printing request */
	time_t p_date;		/* date last enabled/disabled */
	char p_reason[P_RSIZE];	/* if enabled, then "enabled",
				   otherwise the reason the printer has
				   been disabled.	*/
	short p_flags;		/* See below for flag values */
};

/* Value interpretation for p_flags: */

#define	P_ENAB	1		/* printer enabled */
#define	P_AUTO	2		/* disable printer automatically */
#define	P_BUSY	4		/* printer now printing a request */

/* messages for the scheduler that can be written to the FIFO */

#define	F_ENABLE	'e'	/* arg1 = printer */
#define	F_MORE		'm'	/* arg1 = printer */
#define	F_DISABLE	'd'	/* arg1 = printer */
#define	F_ZAP		'z'	/* arg1 = printer */
#define	F_REQUEST	'r'	/* arg1 = destination, arg2 = sequence #,
				   arg3 = logname */
#define	F_CANCEL	'c'	/* arg1 = destination, arg2 = sequence # */
#define	F_DEV		'v'	/* arg1 = printer, arg2 = new device */
#define	F_NOOP		'n'	/* no args */
#define	F_NEWLOG	'l'	/* no args */
#define	F_QUIT		'q'	/* no args */
#define	F_STATUS	's'	/* no args */

/* Arguments to the access(2) system call */

#define	ACC_R	4		/* read access */
#define	ACC_W	2		/* write access */
#define	ACC_X	1		/* execute access */
#define	ACC_DIR	8		/* must be a directory */
