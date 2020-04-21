/*
 * One structure allocated per active process. It contains all data needed
 * about the process while the process may be swapped out.
 * Other per process data (user.h) is swapped with the process.
 *
 * $Source: /d2/3.7/src/sys/h/RCS/proc.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:29:51 $
 */

struct	proc {
	long	p_flag;			/* process flag */
	char	p_stat;
	char	p_pri;			/* priority, negative is high */
	char	p_time;			/* resident time for scheduling */
	char	p_cpu;			/* cpu usage for scheduling */
	char	p_nice;			/* nice for cpu usage */
	char	p_slptime;		/* time since last block */
	ushort	p_uid;			/* real user id */
	ushort	p_suid;			/* set (effective) user id */
	long	p_pctcpu;		/* %cpu for this proc during p_time */
	short	p_cpticks;		/* ticks of cpu time */
	short	p_pgrp;			/* name of process group leader */
	short	p_pid;			/* unique process id */
	short	p_ppid;			/* process id of parent */
	struct	pte *p_addr;		/* addr of udot */
	short	p_poip;			/* page outs in progress */
	short	p_szpt;			/* copy of page table size */
	size_t	p_tsize;		/* size of text (clicks) */
	size_t	p_dsize;		/* size of data space (clicks) */
	size_t	p_ssize;		/* copy of stack size (clicks) */
	size_t 	p_rssize; 		/* resident set size in clicks */
    /*
     * Historically, p_maxrss was used to hold the maximum resident set size a
     * process could ever have.  Now it is used to hold the average resident
     * set size of a process, but retains its name for utility compatability.
     */
	size_t	p_maxrss;
#define	p_avgrss	p_maxrss

	size_t	p_swrss;		/* resident set size before last swap */
	swblk_t	p_swaddr;		/* disk addr of u area when swapped */
	struct	pte *p_p0br;		/* page table base P0BR */
	struct	proc *p_xlink;		/* list of procs sharing same text */
	long	p_sig;			/* signals pending to this process */
	caddr_t	p_wchan;		/* event process is awaiting */
	struct	text *p_textp;		/* pointer to text structure */
	struct	proc *p_link;		/* linked list of running processes */
	time_t	p_clktim;		/* time to alarm clock signal */
	short	p_ndx;			/* index in proc table */
	short	p_loadc;		/* click load addr of proc in vmem */
	short	p_smend;		/* ending click for shared memory */

    /* graphics gunk */
	long	p_grhandle;		/* handle into graphics stuff */
	caddr_t	p_lockaddr;		/* feedback locked virtual addr */
	long	p_lockcount;		/* size of locked feedback area */

	union	{
		struct proccx {
#ifdef IP2
			short	cx_txnum;	/* text/data context */
			short	cx_bsize;	/* size of region (in clicks) */
			short	cx_snum;	/* stack context */
			short	cx_ssize;	/* size of region (in clicks) */

#define	p_cxtdnum	p_u.P_cx.cx_txnum
#define	p_cxbsize	p_u.P_cx.cx_bsize
#define	p_cxsnum	p_u.P_cx.cx_snum
#define	p_cxssize	p_u.P_cx.cx_ssize
#endif
#ifdef PM2
			short	cx_num;		/* cx register value */
			short	cx_bsize;	/* size of region (in clicks) */

#define	p_cxnum		p_u.P_cx.cx_num
#define	p_cxbsize	p_u.P_cx.cx_bsize
#endif
			struct	proc *cx_next;
			struct	proc *cx_prev;	/* links for lru list */
		} P_cx;
#define	p_cxnext	p_u.P_cx.cx_next
#define	p_cxprev	p_u.P_cx.cx_prev

		struct zombie {
			short	Xp_xstat;	/* Exit status for wait */
			time_t	Xp_utime;	/* user time, this proc */
			time_t	Xp_stime;	/* system time, this proc */
		} P_zombie;
#define	xp_xstat	p_u.P_zombie.Xp_xstat
#define	xp_utime	p_u.P_zombie.Xp_utime
#define	xp_stime	p_u.P_zombie.Xp_stime
	} p_u;
};

/* stat codes */
#define	SSLEEP	1		/* awaiting an event */
#define	SWAIT	2		/* (abandoned state) */
#define	SRUN	3		/* running */
#define	SIDL	4		/* intermediate state in process creation */
#define	SZOMB	5		/* intermediate state in process termination */
#define	SSTOP	6		/* process being traced */

/* flag codes */
#define	SLOAD	0x0000001	/* in core */
#define	SSYS	0x0000002	/* swapper or pager process */
#define	SLOCK	0x0000004	/* process being swapped out */
#define	SSWAP	0x0000008	/* save area flag */
#define	STRC	0x0000010	/* process is being traced */
#define	SWTED	0x0000020	/* another tracing flag */
#define	SULOCK	0x0000040	/* user settable lock in core */
#define	SPAGE	0x0000080	/* process in page wait state */
#define	SKEEP	0x0000100	/* another flag to prevent swap out */
#define	SWEXIT	0x0000200	/* working on exiting */
#define	SPHYSIO	0x0000400	/* doing physical i/o (bio.c) */
#define	SGR	0x0000800	/* process is using graphics */
#define	SPTECHG	0x0001000	/* pte's for process have changed */
#define	SFEED	0x0002000	/* proc doing feedback */
#define	SLOSTCX	0x0004000	/* lost hardware pagemap */
#define	HAVTDCX 0x0008000	/* have a text/data context */
#define	HAVSCX	0x0010000	/* have a stack context */
#define	SSHMEM	0x0020000	/* has shared memory */
#define	SSEQL	0x0040000	/* user warned of sequential vm behavior */
#define	SUANOM	0x0080000	/* user warned of random vm behavior */
#define	SDLOCK	0x0100000	/* data seg's are locked */
#define	STLOCK	0x0200000	/* text seg is locked */

/* notice that the following two bits are the same--the server similar
 *	purposes in 4.3 and 5.3 */
#define SPOLL   0x0100000	/* in stream polling code & no recent event */
#define SSEL	0x0100000	/* selecting; wakeup/waiting danger */


#ifdef	KERNEL
extern	struct proc *proc, *procNPROC;	/* the proc table itself */
extern	int nproc;		/* number of processes */

extern	struct proc *curproc;	/* current proc */
extern	struct proc *runq;	/* head of linked list of running processes */
extern	short mpid;		/* next process id */
#endif
