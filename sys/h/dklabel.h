/*
** dklabel.h -- Representing the label on each SGI disk.
**
** $Source: /d2/3.7/src/sys/h/RCS/dklabel.h,v $
** $Date: 89/03/27 17:29:20 $
** $Revision: 1.1 $
*/

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
	u_char	d_bootfs;		/* boot filesystem # */
	u_char	d_swapfs;		/* swap filesystem # */
	struct	disk_map d_map[NFS];	/* filesystem map */
	char	d_interleave;		/* sector interleave */
	char	d_trackskew;		/* track skew (on same cylinder) */
	char	d_cylskew;		/* cylinder and spiral skew */
	short	d_badspots;		/* # of bad spots */
	char	d_name[50];		/* name of drive */
	char	d_serial[50];		/* serial # of drive */
	/*
	** Using d_misc for the gap 1 and gap 2 sizes and groupsizes.
	*/
	long	d_misc[20];		/* miscellaneous drive dependent junk */
	u_char	d_rootnotboot;		/* root and boot filesystems separate */
	u_char	d_rootfs;		/* root filesystem # */
};

/* d_magic value */
#define	D_MAGIC	0x072959

/* d_type's */
#define	DT_3046		0		/* atasi 46MB drive (3046) */
#define	DT_V170		1		/* vertex 72MB drive (V170) */
#define	DT_2312		2		/* fujitsu 84MB drive (2312K) */
#define	DT_2351A	3		/* fujitsu eagle drive (2351) */
#define	DT_1085		4		/* Maxtor 1085 Disk Drive (1085) */
#define	DT_WRENII	5		/* CDC Wren II (94155-86MB) */
#define	DT_V185		6		/* Vertex V185 85MB Disk Drive */
#define	DT_5118		7		/* Hitachi 85MB Disk Drive */
#define	DT_1140		8		/* Maxtor 1140 Disk Drive (1140) */
#define	DT_1325		9		/* Micropolis 1325 */
#define DT_V130		10		/* Vertex V130 */
#define DT_2243		11		/* Fujitsu 2243 */
#define DT_MEM514	12		/* Memorex 514 */
#define DT_1055		12		/* NEC Floppy Model 1055 */
#define DT_T101		13		/* Tandon Floppy (Also Qume) */
#define DT_TM252	14		/* Tandon TM-252 10MB hard disk */
#define DT_QUME		15		/* Qume 592 Floppy */
#define DT_96202	16		/* AST 96202 60 MBytes ESDI Disk */
#define DT_96203	17		/* AST 96203 100 MBytes ESDI Disk */
#define DT_D570		18		/* Cynthia Peripherals 72 MB Disk */
#define DT_3212		19		/* Miniscribe 3212 10 MB Drive */
#define DT_WRENESDI	20		/* CDC Wren II ESDI */
#define DT_TM362	21		/* Tandon TM-362 20MB hard disk */
#define DT_4175		22		/* Maxtor 4175 ESDI */
#define DT_5128		23		/* Hitachi 512-8 ESDI */
#define DT_51212	24		/* Hitachi 512-2 ESDI */
#define DT_51217	25		/* Hitachi 512-7 ESDI */
#define DT_AIMSMD	26		/* Aim Dart 130 SMD Disk Drive */
#define DT_3426		27		/* CMI 20 MB disk Model 3426 */
#define DT_D5126	28		/* NEC 20 MB disk Model D5126 */
#define DT_2246		29		/* Fujitsu 170 MB Disk ESDI */
#define DT_4380		30		/* Micropolis 1550-15 */
#define DT_1100		31		/* Siemens 1100 */
#define DT_1200		32		/* Siemens 1200 */
#define DT_1300		33		/* Siemens 1300 */
#define DT_2085		34		/* Maxtor 2085 */
#define DT_156FA        35              /* Toshiba 156FA 170 MB ESDI */
#define DT_MK56         36              /* Toshiba MK56FB 85 MB ST506 */
#define DT_WREN3	37		/* CDC Wren III 170 MB ESDI */
#define DT_9766		38		/* CDC SMD 9766-300MB Removable */
#define DT_2249		39		/* Fujitsu 390 MB ESDI */
#define DT_51438	40		/* Hitachi 380 MB ESDI */
#define DT_513		41		/* AMS 513 Century Data SMD */

/* d_controller's */
#define	DC_DSD5217		0	/* dsd 5217 disk/tape controller */
#define	DC_XYLOGICS450		1	/* xylogics 450 disk controller */
#define	DC_INTERPHASE2190	2	/* interphase 2190 disk controller */
#define	DC_STORAGER		3	/* Interphase Storager */
#define	DC_NEW1			4
#define	DC_NEW2			5
#define	DC_NEW3			6
#define	DC_NEW4			7

#ifdef	KERNEL

/*
 * Macros for converting dev_t's to filesystem/drive/controller
 * numbers, for standard controllers. These macros know what NFS is.
 */
#define	D_FS(d)		((short) ((d) & 7))		/* 8 filesystems */
#define	D_DRIVE(d)	((short) (((d) >> 3) & 3))	/* 4 drives */
#define	D_CONTROLLER(d)	((short) (((d) >> 5) & 7))	/* 8 controllers */

struct	iobuf *dk_seekck();

/*
 * b_active values
 */
#define	AC_IDLE		0
#define	AC_BUSY		(!AC_IDLE)
#define	AC_DISK		1		/* disk i/o in progress */
#define	AC_TAPE		2		/* tape i/o in progress */
#define	AC_OTHER1	3		/* reserved for future use */
#define	AC_OTHER2	4
#define	AC_OTHER3	5
#define	AC_SEEK		50		/* seek command in progress */

#endif	KERNEL
