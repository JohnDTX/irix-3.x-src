/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef	__fstyp__
#define	__fstyp__
/*
 * $Source: /d2/3.7/src/sys/h/RCS/fstyp.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:29:33 $
 */
#define NULL_FS	0		/* Null FS type - Invalid */

#define FSTYPSZ		16	/* max size of fs identifier */

/*
 * Opcodes for the sysfs() system call.
 */
#define GETFSIND	1	/* translate fs identifier to fstype index */
#define GETFSTYP	2	/* translate fstype index to fs identifier */
#define GETNFSTYP	3	/* return the number of fstypes */

#ifdef KERNEL

#include "../h/conf.h"		/* our users will need struct declarations */

/* Bit settings for fs_flags */
#define FS_NOICACHE	0x1	/* Retain old inodes in in-core cache. */
				/* Should an iput be done on last */
				/* reference?  If this flag is set, an */
				/* iput() is done.  System V fs should have */
				/* this clear */

#define	FSBSIZE(ip)	((ip)->i_mntdev->m_bsize)

/* Passed to fs_getinode to indicate intended use of inode */
#define FSG_PIPE	1	/* Pipe */
#define FSG_CLONE	2	/* Use for clone device */

extern	short	pipefstyp;

/*
 * Virtual filesystem operation call macros.
 */
#define FS_IPUT(ip) \
	(*(ip)->i_fstypp->fs_iput)(ip)
#define FS_IREAD(ip) \
	(*(ip)->i_fstypp->fs_iread)(ip)
#define FS_IUPDAT(ip, tm1, tm2) \
	(*(ip)->i_fstypp->fs_iupdat)(ip, tm1, tm2)
#define FS_READI(ip) \
	(*(ip)->i_fstypp->fs_readi)(ip)
#define FS_WRITEI(ip) \
	(*(ip)->i_fstypp->fs_writei)(ip)
#define FS_ITRUNC(ip) \
	(*(ip)->i_fstypp->fs_itrunc)(ip)
#define FS_STATF(ip, arg) \
	(*(ip)->i_fstypp->fs_statf)(ip, arg)
#define FS_NAMEI(ip, p, arg) \
	(*(ip)->i_fstypp->fs_namei)(p, arg)
#define FS_OPENI(ip, mode) \
	(*(ip)->i_fstypp->fs_openi)(ip, mode)
#define FS_CLOSEI(ip, f, c, o) \
	(*(ip)->i_fstypp->fs_closei)(ip, f, c, o)
#define FS_ACCESS(ip, mode) \
	(*(ip)->i_fstypp->fs_access)(ip, mode)
#define FS_GETDENTS(ip, bp, bsz) \
	(*(ip)->i_fstypp->fs_getdents)(ip, bp, bsz)
#define FS_ALLOCMAP(ip) \
	(*(ip)->i_fstypp->fs_allocmap)(ip)
#define FS_FREEMAP(ip) \
	(*(ip)->i_fstypp->fs_freemap)(ip)
#define FS_READMAP(ip, off, sz, va, sf) \
	(*(ip)->i_fstypp->fs_readmap)(ip, off, sz, va, sf)
#define FS_SETATTR(ip, mode) \
	(*(ip)->i_fstypp->fs_setattr)(ip, mode)
#define FS_NOTIFY(ip, arg) \
	(*(ip)->i_fstypp->fs_notify)(ip, arg)
#define FS_FCNTL(ip, cmd, arg, flag, offset) \
	(*(ip)->i_fstypp->fs_fcntl)(ip, cmd, arg, flag, offset)
#define FS_IOCTL(ip, cmd, arg, flag) \
	(*(ip)->i_fstypp->fs_ioctl)(ip, cmd, arg, flag)

/*
 * AT&T fails to define virtual call macros for the following and
 * for fs_getinode.
 */
#define	FS_INIT(fstyp) \
	(*fstypsw[fstyp].fs_init)()
#define	FS_MOUNT(fstyp, devip, mp, flags) \
	(*fstypsw[fstyp].fs_mount)(devip, mp, flags)
#define	FS_UMOUNT(mp) \
	(*fstypsw[(mp)->m_fstyp].fs_umount)(mp)
#define	FS_STATFS(ip, sfsp, fstyp) \
	(*fstypsw[(fstyp) ? (fstyp) : (ip)->i_fstyp].fs_statfs) \
	    (ip, sfsp, fstyp)
#define	FS_UPDATE(mp) \
	(*fstypsw[(mp)->m_fstyp].fs_update)(mp)

/*
 * SGI extensions for inodes and superblocks.
 */
struct bmapval {		/* result parameter type for FS_BMAP */
	daddr_t	bn;		/* physical block # */
	long	length;		/* length of this extent in bytes */
};

#define	FS_BMAP(ip, how, direct, readahead) \
	(*(ip)->i_fstypp->fs_bmap)(ip, how, direct, readahead)
#define	FS_IALLOC(ip, mode, nlinks, cbdev) \
	(*(ip)->i_fstypp->fs_ialloc)(ip, mode, nlinks, cbdev)
#define	FS_IDESTROY(ip) \
	(*(ip)->i_fstypp->fs_idestroy)(ip)
#define	FS_IFREE(ip) \
	(*(ip)->i_fstypp->fs_ifree)(ip)
#define	FS_SETSIZE(ip, size) \
	(*(ip)->i_fstypp->fs_setsize)(ip, size)
#define	FS_GETTIME(mp) \
	(*fstypsw[(mp)->m_fstyp].fs_gettime)(mp)
#define	FS_UPDATETIME(mp) \
	(*fstypsw[(mp)->m_fstyp].fs_updatetime)(mp)

/*
 * Directory operation extensions.
 * NB: DIR_LOOKUP should account blocks read via sysinfo.dirblk++
 */
struct dirlookupres {		/* result/value parameter type for DIR_ ops */
	ino_t	dlr_inum;	/* entry inumber or zero if not-found */
	off_t	dlr_offset;	/* offset or free space cookie if not-found */
};

typedef	unsigned char dflag_t;	/* flag for fs_dirisempty */
#define	DIR_HASDOT	0x1
#define	DIR_HASDOTDOT	0x2

#define	DIR_LOOKUP(dp, name, len, startoff, dlresp) \
	(*(dp)->i_fstypp->fs_dirlookup)(dp, name, len, startoff, dlresp)
#define	DIR_ENTER(dp, name, len, dlresp) \
	(*(dp)->i_fstypp->fs_direnter)(dp, name, len, dlresp)
#define	DIR_REMOVE(dp, name, len, dlresp) \
	(*(dp)->i_fstypp->fs_dirremove)(dp, name, len, dlresp)
#define	DIR_INIT(dp, pinum) \
	(*(dp)->i_fstypp->fs_dirinit)(dp, pinum)
#define	DIR_ISEMPTY(dp, dflagp) \
	(*(dp)->i_fstypp->fs_dirisempty)(dp, dflagp)

#endif	/* KERNEL */
#endif	/* __fstyp__ */
