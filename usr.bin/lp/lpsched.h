/* "@(#)lpsched.h	3.1" */

#define	EXECMSG	"can't execute printer interface program"
#define	MSGMAX	512
#define	ARGMAX	512	/* maximum # of args to interface program */
#define	BEL	'\07'	/* bell character */
#define	UTMP	"/etc/utmp"
#define	OPENTIME	10

/* Exit codes returned by 2nd level child of scheduler */

#define	EX_SYS	0200	/* this is a system (vs. interface pgm) exit code */
#define	EX_READY	0100	/* printer ready for more */
#define	EX_RESET	040	/* reset file status */
#define	EX_TERM	020		/* child was SIGTERM'd */

char *malloc();
char *date();
char *strcpy();
FILE *popen();
FILE *fopen();

struct dest {		/* structure of a destination */
	char *d_dname;	 	/* name of destination */
	int d_status;		/* status of destination -- see flags below
				   for interpretation */
	char *d_device;		/* full pathname of device associated
				   with printer */
	int d_pid;		/* process id of busy printer */
	struct outlist *d_print;	/* output request currently printing */
	struct dest *d_dnext;	/* next destination */
	struct dest *d_dprev;	/* previous destination */
	struct dest *d_tnext;	/* next destination of same type */
	struct dest *d_tprev;	/* previous destination of same type */
	struct destlist *d_class;  /* class list for printers,
					member list for classes */
	struct outlist *d_output;  /* list of output requests for dest */
};

/* The following flags are used to interpret dest.d_status */

#define	D_PRINTER	1		/* destination is a printer */
#define	D_CLASS	2			/* destination is a class */
#define	D_ENABLED	8		/* printer is enabled */
#define	D_BUSY	16			/* printer is busy */

struct destlist {		/* structure of a destination list */
	struct dest *dl_dest;	/* pointer to destination */
	struct destlist *dl_next;  /* pointer to next destination in list */
	struct destlist *dl_prev;  /* pointer to prev destination in list */
};

struct outlist {		/* structure of an output request list */
	int ol_seqno;		/* sequence number assigned by lp */
	char *ol_name;		/* logname of requestor */
	int ol_time;		/* time request was received by scheduler */
	struct dest *ol_dest;	/* pointer to request destination */
	struct dest *ol_print;	/* if printing, a pointer to the printer */
	struct outlist *ol_next;  /* next output request in list */
	struct outlist *ol_prev;  /* previous output request in list */
};

/* For all destinations */
#define	FORALLD(d)	for(d = dest.d_dnext; d != &dest; d = d->d_dnext)
/* For all printers */
#define	FORALLP(p)	for(p = printer.d_tnext; p != &printer; p = p->d_tnext)
/* For all classes */
#define	FORALLC(c)	for(c = class.d_tnext; c != &class; c = c->d_tnext)
