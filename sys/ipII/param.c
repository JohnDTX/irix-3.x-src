/*
 * $Source: /d2/3.7/src/sys/ipII/RCS/param.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:30:53 $
 */
#include "../h/param.h"
#include "../h/text.h"
#include "../h/acct.h"
#include "../h/tty.h"
#include "../h/buf.h"
#include "../h/file.h"
#include "../h/inode.h"
#include "../h/proc.h"
#include "../h/map.h"
#include "../h/callout.h"
#include "../h/elog.h"
#include "../h/err.h"
#include "../h/ipc.h"
#include "../h/utsname.h"
#include "../ipII/vmparam.h"
#include "../ipII/cx.h"
#include "../ipII/cpureg.h"

#include "xns.h"

#ifdef	lint
#define	MAXUSERS	1
#endif

/* system naming */
struct	utsname utsname;

#define	HZ	64
int	hz = HZ;

#define	NPROC		(100 + 16 * MAXUSERS)	/* typically 164 */
#define	NINODE		(NPROC + 64)		/* typically 228 */
#define	PNCC_SIZE	NINODE
#define	NFILE		(16 * NPROC / 10 + 32)	/* typically 294 */
#define	NCLIST		(50 + 16 * MAXUSERS)
#define	NTEXT		(NPROC / 3 + MAXUSERS)
#define	NCALLOUT	(32 + 8 * MAXUSERS)	/* typically 64 */

int	nproc = NPROC;
short	nbuf;
short	ninode = NINODE;
short	pncc_size = PNCC_SIZE;		/* pathname component cache size */
short	nfile = NFILE;
#if NXNS > 0
short	nclist = NCLIST;
#endif
int	ntext = NTEXT;
short	ncallout = NCALLOUT;

struct	inode *acctp;

/*****************************************************************************/
/*
 * System V msg's
 */
#include "../h/msg.h"

#define	MSGMAP	100
#define	MSGMAX	2048
#define	MSGMNB	4096
#define	MSGMNI	50
#define	MSGSSZ	8
#define	MSGTQL	40
#define	MSGSEG	1024
struct	map		msgmap[MSGMAP];
struct	msqid_ds	msgque[MSGMNI];
char			msglock[MSGMNI];
struct	msg		msgh[MSGTQL];
struct msginfo	msginfo = {
	MSGMAP,
	MSGMAX,
	MSGMNB,
	MSGMNI,
	MSGSSZ,
	MSGTQL,
	MSGSEG
};

/*****************************************************************************/
/*
 * System V semaphores
 */
#include "../h/sem.h"

#define	SEMMAP	10
#define	SEMMNI	10
#define	SEMMNS	60
#define	SEMMNU	30
#define	SEMMSL	25
#define	SEMOPM	10
#define	SEMUME	10
#define	SEMVMX	32767
#define	SEMAEM	16384

struct	semid_ds	sema[SEMMNI];
struct	sem		sem[SEMMNS];
struct	map		semmap[SEMMAP];
struct	sem_undo	*sem_undo[NPROC];
#ifndef	lint
long			semu[((16+8*SEMUME)*SEMMNU+NBPW-1)/NBPW];
long			semtmp[(MAX(2*SEMMSL,MAX(0x20,8*SEMOPM))+NBPW-1)/NBPW];
#else
struct	sem_undo	semu[1];
#endif
struct	seminfo seminfo = {
	SEMMAP,
	SEMMNI,
	SEMMNS,
	SEMMNU,
	SEMMSL,
	SEMOPM,
	SEMUME,
	16+8*SEMUME,
	SEMVMX,
	SEMAEM
};

/*****************************************************************************/
/*
 * System V shared memory
 */
#include "../h/shm.h"

#define	SHMMAX	0x10000			/* 64kb maximum segment */
#define	SHMMIN	1			/* 1 byte minimum */
#define	SHMMNI	40			/* max # of segments */
#define	SHMSEG	10			/* max # of segments per proc */
#define	SHMBRK	btoc(32768)		/* shm seg separation from text&data */
#define	SHMALL	btoc(0x40000)		/* Max of 256kb of shared mem */

struct	shmid_ds shmem[SHMMNI];
struct	shmid_ds *shm_shmem[NPROC * SHMSEG];
struct	shmpt_ds shm_ptbl[NPROC * SHMSEG];
struct	shminfo shminfo = {
	SHMMAX,
	SHMMIN,
	SHMMNI,
	SHMSEG,
	SHMBRK,
	SHMALL,
};

/*****************************************************************************/
/*
 * Sysinfo stuff
 */
#include "../h/sysinfo.h"

struct	syswait syswait;
struct	sysinfo sysinfo;
struct	syserr syserr;

/*****************************************************************************/
/*
 * File and record locking
 */
#include "../h/flock.h"

#define	FLCKREC	100
struct flckinfo flckinfo = {
	FLCKREC,
};
struct filock	flox[FLCKREC];
