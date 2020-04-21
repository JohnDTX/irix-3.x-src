#ifndef	__USTAT__
#define	__USTAT__

#ifdef	KERNEL
/*
 * $Source: /d2/3.7/src/sys/h/RCS/ustat.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:30:19 $
 */

struct  ustat {
	daddr_t	f_tfree;	/* total free */
	ino_t	f_tinode;	/* total inodes free */
	char	f_fname[6];	/* filsys name */
	char	f_fpack[6];	/* filsys pack name */
};
#endif	KERNEL

#endif	__USTAT__
