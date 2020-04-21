/*
 * Structure of the result of stat
 *
 * $Source: /d2/3.7/src/sys/h/RCS/stat.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:30:02 $
 */

struct	oldstat
{
	dev_t	st_dev;
	ino_t	st_ino;
	ushort 	st_mode;
	short  	st_nlink;
	ushort 	st_uid;
	ushort 	st_gid;
	dev_t	st_rdev;
	off_t	st_size;
	time_t	st_atime;
	time_t	st_mtime;
	time_t	st_ctime;
};

struct	stat
{
	dev_t	st_dev;
	long	st_ino;
	ushort 	st_mode;
	short  	st_nlink;
	ushort 	st_uid;
	ushort 	st_gid;
	dev_t	st_rdev;
	off_t	st_size;
	time_t	st_atime;
	time_t	st_mtime;
	time_t	st_ctime;
};

#define	S_IFMT	0170000		/* type of file */
#define		S_IFDIR	0040000	/* directory */
#define		S_IFCHR	0020000	/* character special */
#define		S_IFBLK	0060000	/* block special */
#define		S_IFREG	0100000	/* regular */
#define		S_IFIFO	0010000	/* fifo */
#define		S_IFLNK	0120000	/* symbolic link */
#define	S_ISUID	04000		/* set user id on execution */
#define	S_ISGID	02000		/* set group id on execution */
#define	S_ISVTX	01000		/* save swapped text even after use */
#define	S_IREAD	00400		/* read permission, owner */
#define	S_IWRITE	00200		/* write permission, owner */
#define	S_IEXEC	00100		/* execute/search permission, owner */

/* modes which apply only to symbolic links */
#define S_INLNK		02000	/* network symbolic link */
#define S_ILNIX		00777	/* mask for logical network number */
#define S_ISQUASH	04000	/* flag for "squashed" symbolic link */
