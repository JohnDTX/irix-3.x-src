/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*#ident	"@(#)kern-port:sys/fs/s5param.h	10.4"*/
#ident	"$Header: /d2/3.7/src/bin/newsh/RCS/s5param.h,v 1.1 89/03/27 14:55:01 root Exp $"

/*
/*
 * filesystem parameters
 */

#define	SUPERB	((daddr_t)1)	/* block number of the super block */
#define	DIRSIZ	14		/* max characters per directory */
#define	NICINOD	100		/* number of superblock inodes */
#define	NICFREE	50		/* number of superblock free blocks */
#define	S5ROOTINO	2	/* i number of all roots */

#ifndef FsTYPE
#define FsTYPE	3
#endif


#if FsTYPE==1
/* Original 512 byte file system */
#define	BSIZE		512		/* size of secondary block (bytes) */
#define SBUFSIZE	512		/* system buffer size */
#define	BSHIFT		9		/* log2(BSIZE) */
#define	NINDIR		(BSIZE/sizeof(daddr_t))	/* BSIZE/sizeof(daddr_t) */
#define	BMASK		0777		/* BSIZE-1 */
#define INOPB		8		/* BSIZE/sizeof(struct dinode) */
#define INOSHIFT	3		/* log2(INOPB) */
#define	NMASK		0177		/* NINDIR-1 */
#define	NSHIFT		7		/* log2(NINDIR) */
#define NDPC		4		/* number of blocks/click */
#define Fs2BLK		0x8000		/* large block flag in bsize */
#endif

#if FsTYPE==2
/* New 1024 byte file system */
#define	BSIZE		1024		/* size of secondary block (bytes) */
#define SBUFSIZE	1024		/* system buffer size */
#define	BSHIFT		10		/* log2(BSIZE) */
#define	NINDIR		(BSIZE/sizeof(daddr_t))	/* BSIZE/sizeof(daddr_t) */
#define	BMASK		01777		/* BSIZE-1 */
#define INOPB		16		/* BSIZE/sizeof(struct dinode) */
#define INOSHIFT	4		/* log2(INOPB) */
#define	NMASK		0377		/* NINDIR-1 */
#define	NSHIFT		8		/* log2(NINDIR) */
#define NDPC		4		/* number of blocks per click */
#define Fs2BLK		0x8000		/* large block flag in bsize */
#endif

#if FsTYPE==3
/* Dual file system */
#define	BSIZE		512		/* size of secondary block (bytes) */
#define SBUFSIZE	1024		/* system buffer size */
#define	BSHIFT		9		/* log2(BSIZE) */
#define	NINDIR		128		/* BSIZE/sizeof(daddr_t) */
#define	BMASK		0777		/* BSIZE-1 */
#define INOPB		8		/* BSIZE/sizeof(struct dinode) */
#define INOSHIFT	3		/* log2(INOPB) */
#define	NMASK		0177		/* NINDIR-1 */
#define	NSHIFT		7		/* log2(NINDIR) */
#define NDPC		4
#define Fs2BLK		0x8000		/* large block flag in bsize */
#endif

#define SUPERBOFF	512	/* superblock offset */
