/*
 * Mount structure. One allocated on every mount.
 *
 * $Source: /d2/3.7/src/sys/h/RCS/mount.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:29:45 $
 */

struct	mount {
	dev_t	m_dev;			/* device mounted */
	short	m_fstyp;		/* type of filesystem fs is on */
	long    m_bsize;		/* block size of this fs */
	char	m_flags;		/* read-only and exported */
	char	m_exflags;		/* options flags if exported */
	ushort	m_exroot;		/* exported root uid */
	char	*m_fs;			/* pointer on superblock */
	struct	inode *m_inodp;		/* pointer to mounted-on inode */
	struct	inode *m_mount;		/* pointer to root inode */
	struct	mount *m_next;		/* next mount structure */
};

#define	MRDONLY		0x4	/* file system is mounted read only */
#define	MEXPORTED	0x8	/* file system is exported (NFS) */
#define	rdonlyfs(mp)	((mp)->m_flags & MRDONLY)

/*
 * exported fs flags.
 */
#define EX_RDONLY       0x01    /* exported read only */
#define	EX_SUBMOUNT	0x02	/* export fs mounted under nfs mount */

#ifdef	KERNEL
extern	struct mount *mount;	/* head of mount list */

/*
 * Mount structure operations.  Find looks up a mount by its device.
 * Insert adds mp to the mount list and sets the mounted-on inode
 * pointer to ip, adding an appropriate flag and use-count.  Remove
 * is the inverse of insert.
 */
struct mount	**mount_find(/* dev_t dev */);
void		mount_insert(/* struct mount *mp, struct inode *ip */);
void		mount_remove(/* struct mount **mpp */);
#endif
