/*	@(#)mntent.h 1.1 86/02/03 SMI	*/
/* @(#)mntent.h	2.1 86/04/14 NFSSRC */

/*
 * File system table, see mntent (5)
 *
 * Used by dump, mount, umount, swapon, fsck, df, ...
 *
 * Quota files are always named "quotas", so if type is "rq",
 * then use concatenation of mnt_file and "quotas" to locate
 * quota file.
 */

#define	MNTTAB		"/etc/fstab"
#define	MOUNTED		"/etc/mtab"

#define	MNTMAXSTR	128

#ifdef sgi
# include <sys/fsid.h>

# define MNTTYPE_EFS	FSID_EFS	/* extent filesystem */
# define MNTTYPE_NFS	FSID_NFS	/* network file system */
# define MNTTYPE_SOCKET	FSID_SOCKET	/* socket pseudo-filesystem */
# ifdef SVR3
#  define MNTTYPE_BELL	S51K		/* bell (att s5) filesystem */
#  define MNTTYPE_PROC	PROC		/* process pseudo-filesystem */
#  define MNTTYPE_PIPE	FSID_COM	/* pipe pseudo-filesystem */
# endif
# ifdef SVR0
#  define MNTTYPE_BELL	FSID_BELL	/* bell (att V.0) filesystem */
#  define MNTTYPE_PROC	FSID_PROC	/* process pseudo-filesystem */
#  define MNTTYPE_PIPE	FSID_PIPE	/* pipe pseudo-filesystem */
# endif
#else	/* not sgi */
#define	MNTTYPE_42	"4.2"	/* 4.2 file system */
#define	MNTTYPE_NFS	"nfs"	/* network file system */
#endif
#define	MNTTYPE_PC	"pc"	/* IBM PC (MSDOS) file system */
#define	MNTTYPE_SWAP	"swap"	/* swap file system */
#define	MNTTYPE_IGNORE	"ignore"/* No type specified, ignore this entry */

#define	MNTOPT_RO	"ro"	/* read only */
#define	MNTOPT_RW	"rw"	/* read/write */
#define	MNTOPT_QUOTA	"quota"	/* quotas */
#define	MNTOPT_NOQUOTA	"noquota"	/* no quotas */
#define	MNTOPT_SOFT	"soft"	/* soft mount */
#define	MNTOPT_HARD	"hard"	/* hard mount */
#define	MNTOPT_NOSUID	"nosuid"	/* no set uid allowed */
#ifdef sgi
# define MNTOPT_RAW	"raw"	/* raw device name */
# define MNTOPT_FSCK	"fsck"	/* fsck by default */
# define MNTOPT_NOFSCK	"nofsck"/* do not fsck */
#endif

struct	mntent{
	char	*mnt_fsname;		/* name of mounted file system */
	char	*mnt_dir;		/* file system path prefix */
	char	*mnt_type;		/* MNTTYPE_* */
	char	*mnt_opts;		/* MNTOPT* */
	int	mnt_freq;		/* dump frequency, in days */
	int	mnt_passno;		/* pass number on parallel fsck */
};

struct	mntent *getmntent();
char	*hasmntopt();
FILE	*setmntent();
int	endmntent();
