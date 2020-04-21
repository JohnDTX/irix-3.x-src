/*
 * $Source: /d2/3.7/src/bsd/include/sys/RCS/dir.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 15:00:13 $
 */

/*
 * This sets the "page size" for directories.
 * Requirements are DEV_BSIZE <= DIRBLKSIZ <= MINBSIZE with
 * DIRBLKSIZ a power of two.
 * Dennis Ritchie feels that directory pages should be atomic
 * operations to the disk, so we use DEV_BSIZE.
 */
#ifndef DEV_BSIZE
#define	DEV_BSIZE	512
#endif
#define DIRBLKSIZ DEV_BSIZE

/*
 * This limits the directory name length. Its main constraint
 * is that it appears twice in the user structure. (u. area)
 */
#define MAXNAMLEN 255

struct	direct {
	u_long	d_ino;
	short	d_reclen;
	short	d_namlen;
	char	d_name[MAXNAMLEN + 1];
	/* typically shorter */
};

struct _dirdesc {
	int	dd_fd;
	long	dd_loc;
	long	dd_size;
	char	dd_buf[DIRBLKSIZ];
	struct	direct dd_direct;
};

/*
 * useful macros.
 */
#undef DIRSIZ
#define DIRSIZ(dp) \
    ((sizeof(struct direct) - MAXNAMLEN + (dp)->d_namlen + sizeof(ino_t) - 1) &\
    ~(sizeof(ino_t) - 1))
typedef	struct _dirdesc DIR;
#ifndef	NULL
#define	NULL	0
#endif

/*
 * functions defined on directories
 */

#ifdef SVR3

#define rewinddir(dirp)	BSDseekdir((dirp), 0)
#define opendir BSDopendir
#define readdir BSDreaddir
#define telldir BSDtelldir
#define seekdir BSDseekdir
#define closedir BSDclosedir

#endif /* SVR3 */

extern DIR *opendir();
extern struct direct *readdir();
extern long telldir();
extern void seekdir();
extern void closedir();
