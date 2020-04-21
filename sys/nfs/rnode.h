/*      @(#)rnode.h 1.1 85/05/30 SMI      */
#ifdef SVR0
# include "../com/com_pncc.h"
#endif

/*
 * Remote file information structure.
 * The rnode is the "inode" for remote files.  It contains
 * all the information necessary to handle remote file on the
 * client side.
 */
struct rnode {
	struct rnode	*r_forw;	/* active rnode list */
	struct rnode	*r_back;	/* active rnode list */
	struct inode	*r_ip;		/* pointer to inode */
	fhandle_t	r_fh;		/* file handle */
	unsigned char	r_flags;	/* flags, see below */
	unsigned char	r_open;		/* count of this rnode's open files */
	short		r_error;	/* async write error */
	daddr_t		r_lastr;	/* last block read (read-ahead) */
	struct ucred	r_cred;		/* current credentials */
	struct ucred	r_unlcred;	/* unlinked credentials */
	char		*r_unlname;	/* unlinked file name */
	struct inode	*r_unldip;	/* parent dir of unlinked file */
	struct nfsfattr	r_nfsattr;	/* cached nfs attributes */
	time_t		r_nfsattrtime;	/* time attributes cached */
	unsigned char	r_number;	/* rnode number */
	signed char	r_iocount;	/* number of outstanding i/o's */
	ncap_t		r_ncap;		/* and name cache capability */
};

/*
 * Rnode state flags
 */
#define	RLOCKED		0x01		/* rnode is in use */
#define	RWANT		0x02		/* someone wants a wakeup */
#define	RIOWAIT		0x04		/* waiting for i/o to finish */
#define	REOF		0x08		/* EOF encountered on read */
#define	RDIRTY		0x10		/* dirty buffers may be in buf cache */
#ifdef SVR3
# define RMAPPED	0x20		/* file has blocks mapped to pages */
#endif

/*
 * Convert between inode, rnode, and attributes
 */
#define	rtov(rp)	((rp)->r_ip)
#define	vtor(vp)	((struct rnode *)((vp)->i_fsptr))
#define	vtofh(vp)	(&(vtor(vp)->r_fh))
#define	rtofh(rp)	(&(rp)->r_fh)

/*
 * Name cache capability management.  Rather than searching the whole cache
 * often as Sun does, we remove a stale cache entry upon hitting it.  Stale
 * hits are detected by comparing rnode and cache entry versions identifiers
 * for both the entry rnode and its parent.
 */
#define rnewcap(vp)	(vtor(vp)->r_ncap = pncc_newcap())

/* fetch the name cache capability for an inode */
#define	rnamecap(ip)	(vtor(ip)->r_ncap)

/*
 * Rnodes are accessed via a table indexed by rnode number, which is the
 * same for each inode/rnode as the i-number.  Every NFS inode on a client
 * has a different minor device name (i_dev) which is its rnode number.
 * The major number names an NFS pseudo-device called via the bio.
 */
#define	rntor(rn)	(nfs_rtable[rn])
#define	rtorn(rp)	((rp)->r_number)
#define	devtorn(dev)	minor(dev)
#define	bptorn(bp)	devtorn((bp)->b_dev)
#define	bptor(bp)	rntor(bptorn(bp))

extern struct rnode	**nfs_rtable;
extern short		nfs_rtablesize;

/*
 * Since minor device encodes rnode number, the rnode table size is
 * bounded by the bitsize of a device minor.  This bound informs the
 * representation used for rnode number (r_number).
 */
#define	NFS_MAXRTABLESIZE	(1<<8)

/*
 * Limit on write and read offset, used to detect wild i/o parameters.
 */
#define	NFS_WILDOFF(off)	((off_t)(off) < 0)

/*
 * Flush and invalidate operations for an nfs inode's buffers.
 * Invalidate an inode's cached attributes.
 */
#define	rflush(ip)	bflush(ip->i_dev)
#define	rinval(ip)	binval(ip->i_dev)
#define	rinvalfree(ip)	binvalfree(ip->i_dev)
#define	nfsattr_inval(ip) (vtor(ip)->r_nfsattrtime = 0)

#ifdef NOTDEF
/*
 * Lock and unlock rnodes
 */
void	rlock(/* rp */);
void	runlock(/* rp */);
#endif

/*
 * Functions to lookup an NFS inode by file handle and by cached name.
 *	struct mount	*mp;
 *	fhandle_t	*fhp;
 *	struct nfsfattr	*attrp;
 *	struct inode	*dvp;
 *	struct ncentry	*nce;
 *	struct ucred	*cred;
 *	struct inode	**vpp;
 */
int	nfs_igetbyfh(/* mp, fhp, attrp, vpp */);
int	nfs_igetbyname(/* dvp, nce, cred, vpp */);
