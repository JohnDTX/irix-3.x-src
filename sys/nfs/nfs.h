/*      @(#)nfs.h 1.1 85/05/30 SMI      */

/*
 * We include rpc/types.h here to decouple rpc dependencies from nfs
 * dependencies.  rpc/types.h has a #ifndef wrapper now.
 */
#ifdef SVR3
# include "sys/time.h"
# include "sys/fs/nfs_compat.h"
# include "rpc/types.h"
# ifndef INKERNEL
#  include <sys/errno.h>
# endif
#else
# ifdef KERNEL
#  include "../bsd/time.h"
#  include "../nfs/nfs_compat.h"
#  include "../rpc/types.h"
# else
#  include <sys/time.h>
#  include <nfs/nfs_compat.h>
#  include <rpc/types.h>
# endif
#endif

#ifdef NFSDEBUG
extern int	nfsdebug;	/* debugging level */
#endif

/* Maximum size of data portion of a remote request */
#define	NFS_MAXDATA	8192
#define	NFS_MAXNAMLEN	255
#define	NFS_MAXPATHLEN	1024

/*
 * Rpc retransmission parameters
 */
#define	NFS_TIMEO       7       /* initial timeout in tenths of a sec. */
#define	NFS_RETRIES     4       /* times to retry request */

/*
 * Error status
 * Should include all possible net errors.
 * For IRIS, as with Sun, we just cast errno into an enum nfsstat.
 * For Clover we map.
 */
enum nfsstat {
	NFS_OK = 0,			/* no error */
	NFSERR_PERM=1,			/* Not owner */
	NFSERR_NOENT=2,			/* No such file or directory */
	NFSERR_SRCH=ESRCH,
	NFSERR_INTR=EINTR,
	NFSERR_IO=5,			/* I/O error */
	NFSERR_NXIO=6,			/* No such device or address */
	NFSERR_2BIG=E2BIG,
	NFSERR_NOEXEC=ENOEXEC,
	NFSERR_BADF=EBADF,
	NFSERR_CHILD=ECHILD,
	NFSERR_AGAIN=EAGAIN,
	NFSERR_NOMEM=ENOMEM,
	NFSERR_ACCES=13,		/* Permission denied */
	NFSERR_FAULT=EFAULT,
	NFSERR_NOTBLK=ENOTBLK,
	NFSERR_BUSY=EBUSY,
	NFSERR_EXIST=17,		/* File exists */
	NFSERR_XDEV=EXDEV,
	NFSERR_NODEV=19,		/* No such device */
	NFSERR_NOTDIR=20,		/* Not a directory*/
	NFSERR_ISDIR=21,		/* Is a directory */
	NFSERR_INVAL=EINVAL,
	NFSERR_NFILE=ENFILE,
	NFSERR_MFILE=EMFILE,
	NFSERR_NOTTY=ENOTTY,
	NFSERR_TXTBSY=ETXTBSY,
	NFSERR_FBIG=27,			/* File too large */
	NFSERR_NOSPC=28,		/* No space left on device */
	NFSERR_SPIPE=ESPIPE,
	NFSERR_ROFS=30,			/* Read-only file system */
	NFSERR_MLINK=EMLINK,
	NFSERR_PIPE=EPIPE,
	NFSERR_DOM=EDOM,
	NFSERR_RANGE=ERANGE,
	NFSERR_WOULDBLOCK=35,
	NFSERR_INPROGRESS=36,
	NFSERR_ALREADY=37,
	NFSERR_NOTSOCK=38,
	NFSERR_DESTADDRREQ=39,
	NFSERR_MSGSIZE=40,
	NFSERR_PROTOTYPE=41,
	NFSERR_NOPROTOOPT=42,
	NFSERR_PROTONOSUPPORT=43,
	NFSERR_SOCKTNOSUPPORT=44,
	NFSERR_OPNOTSUPP=45,
	NFSERR_PFNOSUPPORT=46,
	NFSERR_AFNOSUPPORT=47,
	NFSERR_ADDRINUSE=48,
	NFSERR_ADDRNOTAVAIL=49,
	NFSERR_NETDOWN=50,
	NFSERR_NETUNREACH=51,
	NFSERR_NETRESET=52,
	NFSERR_CONNABORTED=53,
	NFSERR_CONNRESET=54,
	NFSERR_NOBUFS=55,
	NFSERR_ISCONN=56,
	NFSERR_NOTCONN=57,
	NFSERR_SHUTDOWN=58,
	NFSERR_TOOMANYREFS=59,
	NFSERR_TIMEDOUT=60,
	NFSERR_CONNREFUSED=61,
	NFSERR_LOOP=62,
	NFSERR_NAMETOOLONG=63,		/* File name too long */
	NFSERR_HOSTDOWN=64,
	NFSERR_HOSTUNREACH=65,
	NFSERR_NOTEMPTY=66,		/* Directory not empty */
	NFSERR_PROCLIM=67,
	NFSERR_USERS=68,
	NFSERR_DQUOT=69,		/* Disc quota exceeded */
	NFSERR_STALE=70,		/* Stale NFS file handle */
	NFSERR_REMOTE=71,
	NFSERR_WFLUSH			/* write cache flushed */
};

#ifdef SVR3
extern enum nfsstat	nfs_statmap[];
extern short		nfs_errmap[];

# define puterrno(error) \
	((u_short)(error) > LASTERRNO ? NFS_OK : nfs_statmap[(short)error])
# define geterrno(status) \
	((u_short)(status) > (u_short)NFSERR_WFLUSH ? (short) 0 \
	    : nfs_errmap[(short)status])
#else
# define puterrno(error)		((enum nfsstat)(error))
# define geterrno(status)	((int)(status))
#endif

/*
 * File types
 */
enum nfsftype {
	NFNON,
	NFREG,		/* regular file */
	NFDIR,		/* directory */
	NFBLK,		/* block special */
	NFCHR,		/* character special */
	NFLNK		/* symbolic link */
};

/*
 * Size of an fhandle in bytes
 */
#define	NFS_FHSIZE	32

/*
 * File access handle
 * This structure is supposed to be opaque from the client side.
 * It is handed out by a server for the client to use in further
 * file transactions.
 */
typedef struct {
	dev_t	fh_fsid;		/* filesystem id */
	u_long	fh_fno;			/* file number */
	u_long	fh_fgen;		/* file generation */
#if defined mips || defined vax
	char	fh_pad[NFS_FHSIZE - 12];	/* Pad struct to NFS_FHSIZE */
#else
	char	fh_pad[NFS_FHSIZE - 10];	/* Pad struct to NFS_FHSIZE */
#endif
} fhandle_t;


/*
 * Arguments to remote write and writecache
 */
struct nfswriteargs {
	fhandle_t	wa_fhandle;	/* handle for file */
	u_long		wa_begoff;	/* beginning byte offset in file */
	u_long		wa_offset;      /* current byte offset in file */
	u_long		wa_totcount;    /* total write count (to this offset)*/
	u_long		wa_count;	/* size of this write */
	char		*wa_data;	/* data to write (up to NFS_MAXDATA) */
	struct buf	*wa_bp;		/* empty buffer for server decode */
};


/*
 * File attributes
 */
struct nfsfattr {
	enum nfsftype	na_type;	/* file type */
	u_long		na_mode;	/* protection mode bits */
	u_long		na_nlink;	/* # hard links */
	u_long		na_uid;		/* owner user id */
	u_long		na_gid;		/* owner group id */
	u_long		na_size;	/* file size in bytes */
	u_long		na_blocksize;	/* prefered block size */
	u_long		na_rdev;	/* special device # */
	u_long		na_blocks;	/* Kb of disk used by file */
	u_long		na_fsid;	/* device # */
	u_long		na_nodeid;	/* inode # */
	struct timeval	na_atime;	/* time of last access */
	struct timeval	na_mtime;	/* time of last modification */
	struct timeval	na_ctime;	/* time of last change */
};

/*
 * Arguments to remote read
 */
struct nfsreadargs {
	fhandle_t	ra_fhandle;	/* handle for file */
	u_long		ra_offset;	/* byte offset in file */
	u_long		ra_count;	/* immediate read count */
	u_long		ra_totcount;	/* total read count (from this offset)*/
};

/*
 * Status OK portion of remote read reply
 */
struct nfsrrok {
	struct nfsfattr	rrok_attr;	/* attributes, need for pagin*/
	u_long		rrok_count;	/* bytes of data */
	char		*rrok_data;	/* data (up to NFS_MAXDATA bytes) */
	struct buf	*rrok_bp;	/* buffer pointer for bread */
	u_long		rrok_bsize;	/* mbuf memory size */
};

/*
 * Reply from remote read
 */
struct nfsrdresult {
	enum nfsstat	rr_status;		/* status of read */
	union {
		struct nfsrrok	rr_ok_u;	/* attributes, need for pagin*/
	} rr_u;
};
#define	rr_ok		rr_u.rr_ok_u
#define	rr_attr		rr_u.rr_ok_u.rrok_attr
#define	rr_count	rr_u.rr_ok_u.rrok_count
#define	rr_data		rr_u.rr_ok_u.rrok_data
#define rr_bp		rr_u.rr_ok_u.rrok_bp
#define	rr_bsize	rr_u.rr_ok_u.rrok_bsize


/*
 * File attributes which can be set
 */
struct nfssattr {
	u_long		sa_mode;	/* protection mode bits */
	u_long		sa_uid;		/* owner user id */
	u_long		sa_gid;		/* owner group id */
	u_long		sa_size;	/* file size in bytes */
	struct timeval	sa_atime;	/* time of last access */
	struct timeval	sa_mtime;	/* time of last modification */
};

#define	SATTR_NULL	((u_long) -1)


/*
 * Reply status with file attributes
 */
struct nfsattrstat {
	enum nfsstat	ns_status;		/* reply status */
	union {
		struct nfsfattr ns_attr_u;	/* NFS_OK: file attributes */
	} ns_u;
};
#define	ns_attr	ns_u.ns_attr_u


/*
 * NFS_OK part of read sym link reply union
 */
struct nfssrok {
	u_long	srok_count;	/* size of string */
	char	*srok_data;	/* string (up to NFS_MAXPATHLEN bytes) */
};

/*
 * Result of reading symbolic link
 */
struct nfsrdlnres {
	enum nfsstat	rl_status;		/* status of symlink read */
	union {
		struct nfssrok	rl_srok_u;	/* name of linked to */
	} rl_u;
};
#define	rl_srok		rl_u.rl_srok_u
#define	rl_count	rl_u.rl_srok_u.srok_count
#define	rl_data		rl_u.rl_srok_u.srok_data


/*
 * Arguments to readdir
 */
struct nfsrddirargs {
	fhandle_t rda_fh;	/* directory handle */
	u_long rda_offset;	/* offset in directory (opaque) */
	u_long rda_count;	/* number of directory bytes to read */
};

/*
 * NFS_OK part of readdir result
 */
struct nfsrdok {
	u_long	rdok_offset;		/* next offset (opaque) */
	u_long	rdok_size;		/* size in bytes of entries */
	bool_t	rdok_eof;		/* true if last entry is in result*/
	struct dirent *rdok_entries;	/* variable number of entries*/
	struct buf *rdok_bp;		/* buffer containing entries */
};

/*
 * Readdir result
 */
struct nfsrddirres {
	u_long		rd_bufsize;	/* size of client request (not xdr'ed)*/
	enum nfsstat	rd_status;
	union {
		struct nfsrdok rd_rdok_u;
	} rd_u;
};
#define	rd_rdok		rd_u.rd_rdok_u
#define	rd_offset	rd_u.rd_rdok_u.rdok_offset
#define	rd_size		rd_u.rd_rdok_u.rdok_size
#define	rd_eof		rd_u.rd_rdok_u.rdok_eof
#define	rd_entries	rd_u.rd_rdok_u.rdok_entries
#define	rd_bp		rd_u.rd_rdok_u.rdok_bp


/*
 * Arguments for directory operations
 */
struct nfsdiropargs {
	fhandle_t	da_fhandle;	/* directory file handle */
	char		*da_name;	/* name (up to NFS_MAXNAMLEN bytes) */
};

/*
 * NFS_OK part of directory operation result
 */
struct  nfsdrok {
	fhandle_t	drok_fhandle;	/* result file handle */
	struct nfsfattr	drok_attr;	/* result file attributes */
};

/*
 * Results from directory operation 
 */
struct  nfsdiropres {
	enum nfsstat	dr_status;	/* result status */
	union {
		struct  nfsdrok	dr_drok_u;	/* NFS_OK result */
	} dr_u;
};
#define	dr_drok		dr_u.dr_drok_u
#define	dr_fhandle	dr_u.dr_drok_u.drok_fhandle
#define	dr_attr		dr_u.dr_drok_u.drok_attr

/*
 * arguments to setattr
 */
struct nfssaargs {
	fhandle_t	saa_fh;		/* fhandle of file to be set */
	struct nfssattr	saa_sa;		/* new attributes */
};

/*
 * arguments to create and mkdir
 */
struct nfscreatargs {
	struct nfsdiropargs	ca_da;	/* file name to create and parent dir */
	struct nfssattr		ca_sa;	/* initial attributes */
};

/*
 * arguments to link
 */
struct nfslinkargs {
	fhandle_t		la_from;	/* old file */
	struct nfsdiropargs	la_to;		/* new file and parent dir */
};

/*
 * arguments to rename
 */
struct nfsrnmargs {
	struct nfsdiropargs rna_from;	/* old file and parent dir */
	struct nfsdiropargs rna_to;	/* new file and parent dir */
};

/*
 * arguments to symlink
 */
struct nfsslargs {
	struct nfsdiropargs	sla_from;	/* old file and parent dir */
	char			*sla_tnm;	/* new name */
	struct nfssattr		sla_sa;		/* attributes */
};

/*
 * NFS_OK part of statfs operation
 */
struct nfsstatfsok {
	u_long fsok_tsize;	/* preferred transfer size in bytes */
	u_long fsok_bsize;	/* fundamental file system block size */
	u_long fsok_blocks;	/* total blocks in file system */
	u_long fsok_bfree;	/* free blocks in fs */
	u_long fsok_bavail;	/* free blocks avail to non-superuser */
};

/*
 * Results of statfs operation
 */
struct nfsstatfs {
	enum nfsstat	fs_status;	/* result status */
	union {
		struct	nfsstatfsok fs_fsok_u;	/* NFS_OK result */
	} fs_u;
};
#define	fs_fsok		fs_u.fs_fsok_u
#define	fs_tsize	fs_u.fs_fsok_u.fsok_tsize
#define	fs_bsize	fs_u.fs_fsok_u.fsok_bsize
#define	fs_blocks	fs_u.fs_fsok_u.fsok_blocks
#define	fs_bfree	fs_u.fs_fsok_u.fsok_bfree
#define	fs_bavail	fs_u.fs_fsok_u.fsok_bavail

/*
 * XDR routines for handling structures defined above
 */
bool_t xdr_attrstat();
bool_t xdr_creatargs();
bool_t xdr_diropargs();
bool_t xdr_diropres();
bool_t xdr_drok();
bool_t xdr_fattr();
bool_t xdr_fhandle();
bool_t xdr_linkargs();
bool_t xdr_rddirargs();
bool_t xdr_putrddirres();
bool_t xdr_getrddirres();
bool_t xdr_rdlnres();
bool_t xdr_rdresult();
bool_t xdr_readargs();
bool_t xdr_rnmargs();
bool_t xdr_rrok();
bool_t xdr_saargs();
bool_t xdr_sattr();
bool_t xdr_slargs();
bool_t xdr_srok();
bool_t xdr_timeval();
bool_t xdr_writeargs();
bool_t xdr_statfs();

/*
 * Remote file service routines
 */
#define	RFS_NULL	0
#define	RFS_GETATTR	1
#define	RFS_SETATTR	2
#define	RFS_ROOT	3
#define	RFS_LOOKUP	4
#define	RFS_READLINK	5
#define	RFS_READ	6
#define	RFS_WRITECACHE	7
#define	RFS_WRITE	8
#define	RFS_CREATE	9
#define	RFS_REMOVE	10
#define	RFS_RENAME	11
#define	RFS_LINK	12
#define	RFS_SYMLINK	13
#define	RFS_MKDIR	14
#define	RFS_RMDIR	15
#define	RFS_READDIR	16
#define	RFS_STATFS	17
#define	RFS_NPROC	18

/*
 * remote file service numbers
 */
#define	NFS_PROGRAM	((u_long)100003)
#define	NFS_VERSION	((u_long)2)
#define	NFS_PORT	2049
