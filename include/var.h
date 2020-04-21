/* @(#)var.h	1.2 */
struct var {
	int	v_buf;		/* NBUF */
	int	v_call;		/* NCALL */
	int	v_inode;	/* NINODE */
#ifdef LINT
	struct inode *ve_inode;	/* &inode[NINODE] */
	int	v_file;		/* NFILE */
	struct file *ve_file;	/* &file[NFILE] */
	int	v_mount;	/* NMOUNT */
	struct mount *ve_mount;	/* &mount[NMOUNT] */
	int	v_proc;		/* NPROC */
	struct proc *ve_proc;	/* &proc[1] */
	int	v_text;		/* NTEXT */
	struct text *ve_text;	/* &text[NTEXT] */
#else
	char *	ve_inode;	/* (char *)(&inode[NINODE]) */
	int	v_file;		/* NFILE */
	char *	ve_file;	/* (char *)(&file[NFILE]) */
	int	v_mount;	/* NMOUNT */
	char *	ve_mount;	/* (char *)(&mount[NMOUNT]) */
	int	v_proc;		/* NPROC */
	char *	ve_proc;	/* (char *)(&proc[1]) */
	int	v_text;		/* NTEXT */
	char *	ve_text;	/* (char *)(&text[NTEXT]) */
#endif
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
#ifdef LINT
	struct callo *ve_call;	/* &callout[NCALL] */
#else
	char *	ve_call;	/* (char *)(&callout[NCALL]) */
#endif
	int	v_stkgap;	/* STACKGAP */
	int	v_cputype;	/* CPU_MC68000 */
	int	v_cpuver;	/* VER_MC68000 */
	int	v_mmutype;	/* MMU_SINGLE */
	int	v_doffset;	/* DOFFSET */
	int	v_kvoffset;	/* KVOFFSET */
	int	v_svtext;	/* NSVTEXT */
#ifdef LINT
	struct svtext *ve_svtext;/* &svtext[NSVTEXT] */
#else
	char *	ve_svtext;	/* (char *)(&svtext[NSVTEXT]) */
#endif
	int	v_pbuf;		/* NPBUF */
	int	v_nscatload;	/* NSCATLOAD */
	int	v_fill[64-45];	/* sized to make var 256 bytes long */
};
extern struct var v;
