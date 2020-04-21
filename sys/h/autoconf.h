/*
 * Root/swap configuration stuff
 *
 * $Source: /d2/3.7/src/sys/h/RCS/autoconf.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:29:10 $
 */

struct	autoconf {
	/*
	 * This section is filled in while booting
	 */
	struct	{
		short	r_major;		/* major device */
		char	r_drive;		/* drive to use */
		char	r_fs;			/* filesystem to use */
		struct	mb_driver *r_md;	/* driver */
		struct	mb_device *r_mi;	/* device */
	} a_root;
	struct	{
		short	s_major;		/* major device */
		char	s_drive;		/* drive to use */
		char	s_fs;			/* filesystem to use */
		struct	mb_driver *s_md;	/* driver */
		struct	mb_device *s_mi;	/* device */
		daddr_t	s_size;			/* biggest size to date */
	} a_swap;
	/*
	 * The rest are for maintenance while configuring
	 */
	short	a_probing;		/* we are currently probing devices */
	short	a_dkn;			/* next disk monitor slot to use */
};

#ifdef	KERNEL
extern	struct autoconf ac;
#endif	KERNEL
