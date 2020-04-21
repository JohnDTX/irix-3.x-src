/*
 * structure of the tape header used by
 * volcopy, labelit, finc, frec.
 * a tape dump of the EFS file system
 * also contains a redundant copy of
 * the superblock and inode blocks
 * following the tape header on the first reel.
 */
struct Tphdr {
	char	t_magic[8];
	char	t_volume[6];
	char	t_reels,
		t_reel;
	long	t_time;
	long	t_length;
	long	t_dens;
	long	t_reelblks;	/* u370 added field */
	long	t_blksize;	/* u370 added field */
	long	t_nblocks;	/* u370 added field */
	char	t_fill[468];
	int	t_type;		/* does tape have nblocks field? (u3b) */
};
