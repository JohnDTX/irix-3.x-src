/*
 * Disk label data structures:
 *	- Blocks are 512 bytes each!
 *	- Block #0 (cylinder 0, head 0, sector 0) contains the boot
 *	  label, which is used by the boot proms and the operating system
 *	  to read drive dependent information
 *	- Block's #1 through 4 are used to hold the bad block map
 *	- Logical block addresses are decomposed by:
 *		spc  = d_heads * d_sectors;	sectors per cylinder
 *		cyl  = lba / spc;		cylinder number
 *		head = (lba % spc) / d_sectors;	head number
 *		sec  = (lba % spc) % d_sectors;	sector number
 *	  and composed by
 *		lba = cyl * spc + head * d_sectors + sec;
 */
/*
**	$Source: /d2/3.7/src/stand/pm2mon/dev/RCS/dklabel.h,v $
**	$Revision: 1.1 $
**	$Date: 89/03/27 17:16:39 $
*/


/* Bad block map entry
 *	Each bad block entry is 8 bytes, so 64 fit in each block
 *	4 blocks are reserved for the map, so no more than 256
 *	bad spots are allowed on one physical disk.
 */
#define	MAXBBM	256			/* Max bad blocks */
#define	NBBMBLK	4			/* # of Bad Block blocks */
struct	disk_bbm {
	long	d_bad;
	long	d_good;
};

/* disk partition map structure */
struct	disk_map {
	long	d_base;			/* base logical block */
	long	d_size;			/* # of blocks */
};

/* Block #0 boot label */
#define	NFS	8
struct	disk_label {
	long	d_magic;		/* magic cookie */
	short	d_type;			/* drive type */
	short	d_controller;		/* controller type */
	short	d_cylinders;		/* # of cylinders per head */
	short	d_heads;		/* # of heads per drive */
	short	d_sectors;		/* # of sectors per track */
	long	d_altstart;		/* Start lba of alternates */
	short	d_nalternates;		/* Number of alternates */
	char	d_rootfs;		/* root filesystem # */
	char	d_swapfs;		/* swap filesystem # */
	struct	disk_map d_map[NFS];	/* filesystem map */
	char	d_interleave;		/* sector interleave */
	char	d_trackskew;		/* track skew (on same cylinder) */
	char	d_cylskew;		/* cylinder skew */
	short	d_badspots;		/* # of bad spots */
	char	d_name[50];		/* name of drive */
	char	d_serial[50];		/* serial # of drive */
	long	d_misc[20];		/* miscellaneous drive dependent junk */
};

/* d_magic value */
#define	D_MAGIC	0x072959

/* d_type's */
#define	DT_ATASI46	0		/* atasi 46mb drive (3046) */
#define	DT_VERTEX72	1		/* vertex 72Mb drive (V170) */
#define	DT_FUJI84	2		/* fujitsu 84Mb drive (2312K) */
#define	DT_EAGLE	3		/* fujitsu eagle drive (2315A) */
#define	DT_MAXTOR	4		/* Maxtor Model 1140 */
#define	DT_FLOPPY	5		/* Standard 80 cyl dbl den 2 hd */
#define	DT_MK56		6		/* MK56 Toshiba 85 MB ST506 */
#define	DT_NEW4		7

/* d_controller's */
#define	DC_DSD5217		0	/* dsd 5217 disk/tape controller */
#define	DC_XYLOGICS450		1	/* xylogics 450 disk controller */
#define	DC_INTERPHASE2190	2	/* interphase 2190 disk controller */
#define	DC_CPC50		3	/* rimfire 50 disk controller */
#define	DC_NEW1			4
#define	DC_NEW2			5
#define	DC_NEW3			6
#define	DC_NEW4			7

#ifdef	KERNEL

/*
 * Macros for converting dev_t's to filesystem/drive/controller
 * numbers, for standard controllers. These macros know what NFS is.
 */
#define	D_FS(d)		((d) & 7)		/* 8 filesystems */
#define	D_DRIVE(d)	(((d) >> 3) & 3)	/* 4 drives */
#define	D_CONTROLLER(d)	(((d) >> 5) & 7)	/* 8 controllers (???) */

#endif	KERNEL
