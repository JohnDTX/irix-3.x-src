/*
 * Structure of a directory entry.
 *
 * $Source: /d2/3.7/src/sys/h/RCS/dir.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:29:16 $
 */

#ifndef	DIRSIZ
#define	DIRSIZ	14
#endif

#ifndef	__DIR__

#define	__DIR__

struct	direct {
	ino_t	d_ino;
	char	d_name[DIRSIZ];
};
#endif	__DIR__
