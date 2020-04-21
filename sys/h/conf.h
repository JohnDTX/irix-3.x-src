#ifndef	__conf__
#define	__conf__
/*
 * Declaration of block device switch. Each entry (row) is
 * the only link between the main unix code and the driver.
 * The initialization of the device switches is in the file conf.c.
 *
 * $Source: /d2/3.7/src/sys/h/RCS/conf.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:29:14 $
 */
struct bdevsw {
	int	(*d_open)();
	int	(*d_close)();
	int	(*d_strategy)();
	int	(*d_print)();
	int	(*d_dump)();
};

/*
 * Character device switch.
 */
struct cdevsw {
	int	(*d_open)();
	int	(*d_close)();
	int	(*d_read)();
	int	(*d_write)();
	int	(*d_ioctl)();
	struct tty *d_ttys;
	struct streamtab *d_str;
};

/*
 * Stream module switch.
 */
#define	FMNAMESZ	8

struct fmodsw {
	char	f_name[FMNAMESZ+1];
	struct  streamtab *f_str;
};

/*
 * Line discipline switch.
 */
struct linesw {
	int	(*l_open)();
	int	(*l_close)();
	int	(*l_read)();
	int	(*l_write)();
	int	(*l_ioctl)();
	int	(*l_input)();
	int	(*l_output)();
	int	(*l_mdmint)();
};

/*
 * File system switch.
 */
struct fstypsw {
/* 0*/	int		(*fs_init)();
/* 1*/	int		(*fs_iput)();
/* 2*/	struct inode *	(*fs_iread)();
/* 3*/	int		(*fs_filler)();
/* 4*/	int		(*fs_iupdat)();
/* 5*/	int		(*fs_readi)();
/* 6*/	int		(*fs_writei)();
/* 7*/	int		(*fs_itrunc)();
/* 8*/	int		(*fs_statf)();
/* 9*/	int		(*fs_namei)();
/*10*/	int		(*fs_mount)();
/*11*/	int		(*fs_umount)();
/*12*/	struct inode *	(*fs_getinode)();
/*13*/	int		(*fs_openi)();		/* open inode */
/*14*/	int		(*fs_closei)();		/* close inode */
/*15*/	int		(*fs_update)();		/* update */
/*16*/	int		(*fs_statfs)();		/* statfs and ustat */
/*17*/	int		(*fs_access)();
/*18*/	int		(*fs_getdents)();
/*19*/	int		(*fs_allocmap)();	/* Let the fs decide if */
						/* if can build a map so */
						/* this fs can be used for */
						/* paging */
/*20*/	int *		(*fs_freemap)();	/* free block list */
/*21*/	int		(*fs_readmap)();	/* read a page from the fs */
						/* using the block list */
/*22*/	int		(*fs_setattr)();	/* set attributes */
/*23*/	long		(*fs_notify)();		/* notify fs of action */
/*24*/	int		(*fs_fcntl)();		/* fcntl */
/*25*/	int		(*fs_fsinfo)();		/* additional info */
/*26*/	int		(*fs_ioctl)();		/* ioctl */

/*
 * Begin SGI extensions.  These extensions are used only if the filesystem
 * shares code with the "com" filesystem.
 */
/*27*/	int		(*fs_dirlookup)();	/* lookup name in directory */
/*28*/	int		(*fs_direnter)();	/* enter name,inum in dir */
/*29*/	int		(*fs_dirremove)();	/* remove a dir entry */
/*30*/	int		(*fs_dirinit)();	/* initialize a new dir */
/*31*/	int		(*fs_dirisempty)();	/* dir empty predicate */
/*32*/	int		(*fs_bmap)();		/* bmap */
/*33*/	struct inode	*(*fs_ialloc)();	/* allocate an inode */
/*34*/	int		(*fs_idestroy)();	/* free inode representation */
/*35*/	int		(*fs_ifree)();		/* free inode */
/*36*/	int		(*fs_setsize)();	/* set file size */
/*37*/	time_t		(*fs_gettime)();	/* get time from superblock */
/*38*/	int		(*fs_updatetime)();	/* update time in superblock */
/*39*/	int		(*fs_fill[1])();
};

/* FS specific data */
struct fsinfo {
	long		fs_flags;	/* flags - see fstyp.h */
	struct mount	*fs_pipe;	/* The mount point to be used as the */
					/* pipe device for this fstyp */
	char		*fs_name; 	/* pointer to fstyp name, see above */
	long		fs_notify;	/* flags for fs_notify */
					/* e.g., NO_CHDIR, NO_CHROOT */
					/* see nami.h */
};

#ifdef	KERNEL
extern struct bdevsw	bdevsw[];
extern struct cdevsw	cdevsw[];
extern struct fmodsw	fmodsw[];
extern struct linesw	linesw[];
extern struct fstypsw	fstypsw[];
extern struct fsinfo	fsinfo[];

extern short		bdevcnt;
extern short		cdevcnt;
extern int		fmodcnt;
extern short		linecnt;
extern short		nfstyp;
#endif	/* KERNEL */
#endif	/* __conf__ */
