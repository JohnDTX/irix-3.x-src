/*	dk.h	6.1	83/07/29	*/

/*
 * Instrumentation
 */
#define	CPUSTATES	4

#define	CP_USER		0
#define	CP_NICE		1
#define	CP_SYS		2
#define	CP_IDLE		3

#define	DK_NDRIVE	4

#ifdef	KERNEL
long	cp_time[CPUSTATES];
long	dk_busy;
long	dk_time[DK_NDRIVE];
long	dk_seek[DK_NDRIVE];
long	dk_xfer[DK_NDRIVE];
long	dk_wds[DK_NDRIVE];

#ifndef	sgi
float	dk_mspw[DK_NDRIVE];
#else
long	dk_mspw[DK_NDRIVE];

#define	dkunit(bp)	(minor((bp)->b_dev) >> 3)
extern	char *dk_types[];

#endif	sgi

#endif	KERNEL
