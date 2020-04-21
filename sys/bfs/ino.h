#ifndef	__INO__
#define	__INO__
/*
 * Inode structure as it appears on a disk block (bell filesystem).
 *
 * $Source: /d2/3.7/src/sys/bfs/RCS/ino.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:26:22 $
 */

#define	NADDR	13
#define	NDADDR	(NADDR * 3)
#define	NSADDR	(NADDR*sizeof(daddr_t)/sizeof(short))

/*
 * The 39 address bytes in di_addr make 13 addresses of 3 bytes each.
 */
struct dinode
{
	ushort	di_mode;	/* mode and type of file */
	short	di_nlink;    	/* number of links to file */
	ushort	di_uid;      	/* owner's user id */
	ushort	di_gid;      	/* owner's group id */
	off_t	di_size;     	/* number of bytes in file */
	char  	di_addr[NDADDR];/* disk block addresses */
	char	di_gen;		/* generation number */
	time_t	di_atime;   	/* time last accessed */
	time_t	di_mtime;   	/* time last modified */
	time_t	di_ctime;   	/* time created */
};

#endif	__INO__
