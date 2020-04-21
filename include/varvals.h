	/*	varvals.h - constant values obtained from the XVAR
			system call
	*/
struct var {
	int	v_buf;		/* NBUF */
	int	v_call;		/* NCALL */
	int	v_inode;	/* NINODE */
	int	v_file;		/* NFILE */
	int	v_mount;	/* NMOUNT */
	int	v_proc;		/* NPROC */
	int	v_text;		/* NTEXT */
	int	v_clist;	/* NCLIST */
	int	v_sabuf;	/* NSABUF */
	int	v_maxup;	/* MAXUP */
	int	v_cmap;		/* CMAPSIZ */
	int	v_smap;		/* SMAPSIZ */
	int	v_hbuf;		/* NHBUF */
	int	v_hmask;	/* NHBUF-1 */
	int	v_flock;	/* NFLOCK */
	int	v_phys;		/* NPHYS */
	int	v_clsize;	/* CLSIZE */
	int	v_txtrnd;	/* ctos(1) */
	int	v_bsize;	/* BSIZE */
	int	v_cxmap;	/* CXMAPSIZ for sun MMU for 68451 */
	int	v_clktick;	/* CLKTICK */
	int	v_hz;		/* HZ */
	int	v_usize;	/* USIZE */
	int	v_pageshift;	/* PAGESHIFT */
	int	v_pagemask;	/* PAGEMASK */
	int	v_segshift;	/* SEGSHIFT */
	int	v_segmask;	/* SEGMASK */
	int	v_ustart;	/* USTART */
	int	v_uend;		/* UEND */
	int	v_stkgap;	/* STACKGAP */
	int	v_cputype;	/* CPU_MC68000 */
	int	v_cpuver;	/* VER_MC68000 */
	int	v_mmutype;	/* MMU_SINGLE */
	int	v_doffset;	/* DOFFSET */
	int	v_kvoffset;	/* KVOFFSET */
	int	v_svtext;	/* NSVTEXT */
	int	v_pbuf;		/* NPBUF */
	int	v_nscatload;	/* NSCATLOAD */
};

	struct var v = { 44, /* NBUF */
 56, /* NCALL */
 92, /* NINODE */
 128, /* NFILE */
 8, /* NMOUNT */
 40, /* NPROC */
 28, /* NTEXT */
 164, /* NCLIST */
 0, /* NSABUF */
 25, /* MAXUP */
 0, /* CMAPSIZ */
 82, /* SMAPSIZ */
 32, /* NHBUF */
 31, /* NHBUF-1 */
 0, /* NFLOCK */
 4, /* NPHYS */
 26, /* CLSIZE */
 4096, /* ctos(1) */
 512, /* BSIZE */
 50, /* CXMAPSIZ for sun MMU for 68451 */
 15625, /* CLKTICK */
 60, /* HZ */
 1, /* USIZE */
 12, /* PAGESHIFT */
 16773120, /* PAGEMASK */
 12, /* SEGSHIFT */
 0, /* SEGMASK */
 4096, /* USTART */
 16248832, /* UEND */
 8, /* STACKGAP */
 0, /* CPU_MC68000 */
 0, /* VER_MC68000 */
 0, /* MMU_SINGLE */
 0, /* DOFFSET */
 0, /* KVOFFSET */
 28, /* NSVTEXT */
 0, /* NPBUF */
 2048 /* NSCATLOAD */
 }; 
