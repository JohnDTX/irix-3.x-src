/*
 * $Source: /d2/3.7/src/sys/h/RCS/param.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:29:49 $
 */

#ifdef	KERNEL
#include "../h/sysmacros.h"
#include "../h/types.h"
#include "../h/signal.h"
#include "../debug/debug.h"
#else
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/signal.h>
#endif
#include "machine/param.h"

#define	NPTEPG		(NBPG/(sizeof (struct pte)))

/*
 * fundamental variables
 * don't change too often
 */
#define	NOFILE	40		/* max open files per process */
#define	MAXPID	30000		/* max process id */
#define	MAXUID	60000		/* max user id */
#define	MAXLINK	1000		/* max links */

#define	USIZE		(NBPG * UPAGES)

#define	CANBSIZ	256		/* max size of typewriter line	*/
/* HZ is defined in <machine/param.h> */
#define	NCARGS	10240		/* # characters in exec arglist */

/*
 * priorities
 * should not be altered too much
 */
#define	PMASK	0177
#define	PCATCH	0400
#define	PSWP	0
#define	PINOD	10
#define	PRIBIO	20
#define	PZERO	25
#define	NZERO	20
#define	PPIPE	26
#define	PWAIT	30
#define	PSLEP	39
#define	PUSER	60
#define	PIDLE	127

/*
 * fundamental constants of the implementation--
 * cannot be changed easily
 */

#define	NBPW	sizeof(int)	/* number of bytes in an integer */

#ifndef	FsTYPE
#define	FsTYPE	2
#endif

#if FsTYPE==1
	/* Original 512 byte file system */
#define	BSIZE	512		/* size of file system block (bytes) */
#define	SBUFSIZE	BSIZE	/* system buffer size */
#define	BSHIFT	9		/* LOG2(BSIZE) */
#define	NINDIR	(BSIZE/sizeof(daddr_t))
#define	BMASK	0777		/* BSIZE-1 */
#define	INOPB	8		/* inodes per block */
#define	INOSHIFT	3	/* LOG2(INOPB) if exact */
#define	NMASK	0177		/* NINDIR-1 */
#define	NSHIFT	7		/* LOG2(NINDIR) */
#define	FsBSIZE(dev)	BSIZE
#define	FsBSHIFT(dev)	BSHIFT
#define	FsNINDIR(dev)	NINDIR
#define	FsBMASK(dev)	BMASK
#define	FsINOPB(dev)	INOPB
#define	FsLTOP(dev, b)	b
#define	FsPTOL(dev, b)	b
#define	FsNMASK(dev)	NMASK
#define	FsNSHIFT(dev)	NSHIFT
#define	FsITOD(dev, x)	itod(x)
#define	FsITOO(dev, x)	itoo(x)
#define	FsINOS(dev, x)	(((x)&~(INOPB-1))+1)
#define	FsBOFF(dev, x)	((x)&BMASK)
#define	FsBNO(dev, x)	((x)>>BSHIFT)
#endif

#if FsTYPE==2
/* compatability stuff */
#define	BBSPERBLOCK	2
#define	DEV_BSIZE	512
	/* New 1024 byte file system */
#define	BSIZE	1024		/* size of file system block (bytes) */
#define	SBUFSIZE	BSIZE	/* system buffer size */
#define	BSHIFT	10		/* LOG2(BSIZE) */
#define	NINDIR	(BSIZE/sizeof(daddr_t))
#define	BMASK	01777		/* BSIZE-1 */
#define	INOPB	16		/* inodes per block */
#define	INOSHIFT	4	/* LOG2(INOPB) if exact */
#define	NMASK	0377		/* NINDIR-1 */
#define	NSHIFT	8		/* LOG2(NINDIR) */
#define	FsBSIZE(dev)	BSIZE
#define	FsBSHIFT(dev)	BSHIFT
#define	FsNINDIR(dev)	NINDIR
#define	FsBMASK(dev)	BMASK
#define	FsINOPB(dev)	INOPB
#define	FsLTOP(dev, b)	(b<<1)
#define	FsPTOL(dev, b)	(b>>1)
#define	FsNMASK(dev)	NMASK
#define	FsNSHIFT(dev)	NSHIFT
#define	FsITOD(dev, x)	itod(x)
#define	FsITOO(dev, x)	itoo(x)
#define	FsINOS(dev, x)	(((x)&~(INOPB-1))+1)
#define	FsBOFF(dev, x)	((x)&BMASK)
#define	FsBNO(dev, x)	((x)>>BSHIFT)
#endif

/*
 * WARNING: The dual system is compatible with FsTYPE==1, but not FsTYPE==2:
 * This incompatability is present in the defines for the basic constant,
 * not with the macros (which don't use the constants!).  Unfortunately,
 * utility code uses the constants, not the macros.
 */
#if FsTYPE==3
	/* Dual system */
#define	BSIZE	512		/* size of file system block (bytes) */
#define	SBUFSIZE	(BSIZE*2)	/* system buffer size */
#define	BSHIFT	9		/* LOG2(BSIZE) */
#define	NINDIR	(BSIZE/sizeof(daddr_t))
#define	BMASK	0777		/* BSIZE-1 */
#define	INOPB	8		/* inodes per block */
#define	INOSHIFT	3	/* LOG2(INOPB) if exact */
#define	NMASK	0177		/* NINDIR-1 */
#define	NSHIFT	7		/* LOG2(NINDIR) */
#define	Fs2BLK	0x2000
#define	FsLRG(dev)	(dev&Fs2BLK)
#define	FsBSIZE(dev)	(FsLRG(dev) ? (BSIZE*2) : BSIZE)
#define	FsBSHIFT(dev)	(FsLRG(dev) ? 10 : 9)
#define	FsNINDIR(dev)	(FsLRG(dev) ? 256 : 128)
#define	FsBMASK(dev)	(FsLRG(dev) ? 01777 : 0777)
#define	FsBOFF(dev, x)	(FsLRG(dev) ? ((x)&01777) : ((x)&0777))
#define	FsBNO(dev, x)	(FsLRG(dev) ? ((x)>>10) : ((x)>>9))
#define	FsINOPB(dev)	(FsLRG(dev) ? 16 : 8)
#define	FsLTOP(dev, b)	(FsLRG(dev) ? b<<1 : b)
#define	FsPTOL(dev, b)	(FsLRG(dev) ? b>>1 : b)
#define	FsNMASK(dev)	(FsLRG(dev) ? 0377 : 0177)
#define	FsNSHIFT(dev)	(FsLRG(dev) ? 8 : 7)
#define	FsITOD(dev, x)	(daddr_t)(FsLRG(dev) ? \
	((unsigned)x+(2*16-1))>>4 : ((unsigned)x+(2*8-1))>>3)
#define	FsITOO(dev, x)	(daddr_t)(FsLRG(dev) ? \
	((unsigned)x+(2*16-1))&017 : ((unsigned)x+(2*8-1))&07)
#define	FsINOS(dev, x)	(FsLRG(dev) ? \
	((x&~017)+1) : ((x&~07)+1))
#endif

#define	NICFREE	50		/* number of superblock free blocks */
#define	NULL	0
#define	CMASK	0		/* default mask for file creation */
#define	CDLIMIT	(1L<<(30-BBSHIFT))	/* default max write block address */
#define	NODEV	(dev_t)(-1)
#define	ROOTINO	((ino_t)2)	/* i number of all roots */
#define	SUPERB	((daddr_t)1)	/* physical block number of the super block */
#define	SUPERBOFF	512	/* byte offset of the super block */
#define	DIRSIZ	14		/* max characters per directory */
#define	NICINOD	100		/* number of superblock inodes */

#define	lobyte(X)	(((unsigned char *)&(X))[1])
#define	hibyte(X)	(((unsigned char *)&(X))[0])
#define	loword(X)	(((ushort *)&(X))[1])
#define	hiword(X)	(((ushort *)&(X))[0])

#define MIN(a, b)	(((a) < (b))?(a):(b))
#define MAX(a, b)	(((a) > (b))?(a):(b))

/*
 * Macros for counting and rounding.
 */
#define	howmany(x, y)	(((x)+((y)-1))/(y))
#define	roundup(x, y)	((((x)+((y)-1))/(y))*(y))

/*
 * Maximum size of hostname and domainname recognized and stored in
 * the kernel.
 */
#define MAXHOSTNAMELEN	64
