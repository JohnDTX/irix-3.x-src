/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* #ident "@(#)kern-3b2:sys/sysinfo.h	1.3" */

/*
 *	System Information.
 */

struct sysinfo {
	time_t	cpu[5];
#define	CPU_IDLE	0
#define	CPU_USER	1
#define	CPU_KERNEL	2
#define	CPU_WAIT	3
#define CPU_SXBRK	4
	time_t	wait[3];
#define	W_IO	0
#define	W_SWAP	1
#define	W_PIO	2
	long	bread;
	long	bwrite;
	long	lread;
	long	lwrite;
	long	phread;
	long	phwrite;
	long	swapin;
	long	swapout;
	long	bswapin;
	long	bswapout;
	long	pswitch;
	long	syscall;
	long	sysread;
	long	syswrite;
	long	sysfork;
	long	sysexec;
	long	runque;
	long	runocc;
	long	swpque;
	long	swpocc;
	long	iget;
	long	namei;
	long	dirblk;
	long	readch;
	long	writech;
	long	rcvint;
	long	xmtint;
	long	mdmint;
	long	rawch;
	long	canch;
	long	outch;
	long	msg;
	long	sema;
	long	pnpfault;
	long	wrtfault;
};

extern struct sysinfo sysinfo;

struct syswait {
	short	iowait;
	short	swap;
	short	physio;
};


struct minfo {
	unsigned long 	freemem[2]; 	/* freemem in pages */
					/* "double" long format	*/
					/* freemem[0] least significant */
	long	freeswap;	/* free swap space */
	long    vfault;  	/* translation fault */
	long    demand;		/*  demand zero and demand fill pages */
	long    swap;		/*  pages on swap */
	long    cache;		/*  pages in cache */
	long    file;		/*  pages on file */
	long    pfault;		/* protection fault */
	long    cw;		/*  copy on write */
	long    steal;		/*  steal the page */
	long    freedpgs;	/* pages are freed */
	long    unmodsw;	/* getpages finds unmodified pages on swap */
	long	unmodfl;	/* getpages finds unmodified pages in file */ 
};

extern struct minfo minfo;
extern struct syswait syswait;

struct syserr {
	long	inodeovf;
	long	fileovf;
	long	textovf;
	long	procovf;
};

extern struct syserr syserr;
