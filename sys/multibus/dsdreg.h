/*
** Definitions for the Midas 5217 disk/tape/floppy controller
**
** $Source: /d2/3.7/src/sys/multibus/RCS/dsdreg.h,v $
** $Revision: 1.1 $
** $Date: 89/03/27 17:31:20 $
*/

#define	SPL() spl1()			/* Priority level of controller */

typedef struct	wub {
	u_char	w_xx;		/*1  *//* UNUSED */
	u_char	w_ext;		/*0  *//* extension */
	u_char	*w_ccb;		/*2-5*//* Ptr to ccb */
} wub_t;
#define W_EXT  7		/* Extended Addressing for 24 bits */

typedef struct	ccb {
	u_char	c_busy;		/*1  *//* Busy 1 flag FF=Busy; 00=Idle*/
	u_char	c_ccw1;		/*0  *//* Channel Control Word 1 (==1) */
	u_char	*c_cib;		/*2-5*//* Ptr to cib */
	u_short	c_xx;		/*6-7*//* UNUSED */
	u_char	c_busy2;	/*9  *//* UNUSED */ /* Busy 2 flag */
	u_char	c_ccw2;		/*8  *//* UNUSED */ /* Channel Control Word 2 */
	u_char	*c_cp;		/*a-d*//* UNUSED */ /* CP Pointer */
	u_short	c_ctrlptr;	/*e-f*//* Control Pointer (==4) */
} ccb_t;

typedef struct	cib {
	u_char	i_opstat;	/*1  *//* Operation Status */
	u_char	i_xx;		/*0  *//* UNUSED */ /* Reserved */
	u_char	i_stsem;	/*3  *//* Status Semaphore */
	u_char	i_cmdsem;	/*2  *//* UNUSED */ /* Command Semaphore */
	u_char	*i_zero;	/*4-7*//* UNUSED */ /* must be all zeros */
	u_char	*i_iopb;	/*8-b*//* IOPB Pointer */
	u_long	i_xx2;		/*c-f*//* UNUSED */ /* must be all zeros */
} cib_t;

/* i_opstat codes */
#define	O_OPCOMP	0x01		/* Operation complete */
#define	O_SKCOMP	0x02		/* Seek Complete */
#define	O_MCH		0x04		/* Media Change */
#define	O_FLQICDN	0x09		/* Flp/Qic done */
#define O_TPCHG		0x0e		/* Tape Media Change */
#define O_TPLONG	0x0f		/* Tape Long Term Command */
#define	O_UNIT		0x30		/* UNIT bits */
#define	O_HARD		0x40		/* Hard Error */
#define	O_SUMMARY	0x80		/* Summary Error */
#define O_MASK		0xf0		/* Status Mask */

typedef struct	iopb {
	u_long	p_xx;
	u_long	p_atc;		/* Actual Transfer Count */
	u_short	p_dev;		/* Device code */
	u_char	p_func;		/* Function */
	u_char	p_unit;		/* Unit */
	u_short	p_mod;		/* Modifier */
	u_short	p_cyl;		/* Cylinder */
	u_char	p_sec;		/* Sector */
	u_char	p_hd;		/* Head */
	u_char	*p_dba;		/* Data Buffer Address */
	u_long	p_rbc;		/* Requested Byte Count */
	u_char	*p_gap;		/* General Address Pointer */
} iopb_t;

/* p_dev device codes */
#define	D_WIN		0x0000	/* Winchester */
#define	D_FLP		0x0001	/* Floppy */
#define	D_QIC		0x0002	/* QIC Device */
#define	D_217		0x0004	/* Streaming Tape Drive */

/* p_func operations */
#define	F_INIT		0x00	/* Initialize */
#define	F_TSTAT		0x01	/* Transfer Status */
#define	F_FORMAT	0x02	/* Format */
#define	F_RDID		0x03	/* Read ID */
#define	F_READ		0x04	/* Read Data */
#define	F_RBV		0x05	/* Read to Buffer and Verify */
#define	F_WRITE		0x06	/* Write Data */
#define	F_WBD		0x07	/* Write Buffer Data */
#define	F_SEEK		0x08	/* Seek */
/* Reserved from 0x09 to 0x0d */
#define	F_BIO		0x0E	/* Buffer I/O */
#define	F_DIAG		0x0F	/* Diagnostic */
#define F_TINIT		0x10	/* Tape initialize */
#define F_TREW		0x11	/* Tape rewind */
#define F_TSPFILE	0x12	/* Tape space forward a file */
#define F_TFLMK		0x14	/* Tape write file mark */
#define F_TERASE	0x17	/* Tape erase tape */
#define F_TSPREC	0x1A	/* Tape space forward a record */
#define F_TRESET	0x1C	/* Tape reset */
#define F_TRETEN	0x1D	/* Tape retension */
#define F_TDSTAT	0x1E	/* Tape drive status */
#define F_TTERM		0x1F	/* Tape read/write terminate */

/* p_mod codes */
#define	M_NOINT		0x0001	/* No Interrupt on completion */
#define	M_NOERR		0x0002	/* Inhibit retries for error recovery */
#define	M_ALLOWDD	0x0004	/* Allow Deleted Data to be read/written */
#define M_TAPELG	0x0040  /* Read error status after long tape cmd */

struct	inib {
	u_short	i_ncyl;		/* Number of cylinders */
	u_char	i_rhd;		/* Removable heads */
	u_char	i_fhd;		/* Fixed heads */
	u_char	i_bpsl;		/* Bytes Per Sector Low */
	u_char	i_spt;		/* Sectors Per Track */
	u_char	i_nacyl;	/* Number of Alternate Cylinders */
	u_char	i_bpsh;		/* Bytes Per Sector High */
};				/*  NOTE: Should be a union */

typedef struct	fmtb {
	u_char	f_pat1;		/* Pattern Byte 1 */
	u_char	f_func;		/* Which format function */
	u_char	f_pat3;		/* Pattern Byte 3 */
	u_char	f_pat2;		/* Pattern Byte 2 */
	u_char	f_ilv;		/* Interleave */
	u_char	f_pat4;		/* Pattern Byte 4 */
} fmtb_t;

typedef union	inist {
	struct	inib	is_inib;
	struct 	fmtb	is_fmtb;
	u_char		is_sb[14];
} inist_t;

/* Indices into is_sb[] (xor with 1 to char swap) */
#define SB_EXS		(12^1)		/* Extended status for the disk */
#define SB_RETRY	(11^1)		/* Retries for the disk */
#define	HARD_BYTE0	(0^1)
#define	HARD_BYTE1	(1^1)
#define SOFT_BYTE0	(2^1)
#define BOT		(3^1)
#define FILE_MARK	(5^1)
#define NO_DATA		(8^1)

#define	SECTOR_NOT_FOUND 0x25	/* Bad header most likely */

/*
 * Bits of the Soft Error Byte
 */
#define BUFOVER 	0x0040		/* Buffer Over/Under Run */
#define DRFAULT		0x0020		/* Drive Fault */
#define DATAERR		0x0008		/* Unrecoverable Data error */
#define RECDATA 	0x0002		/* Recoverable Data Err with Retries */
/*
 * Bits of the Hard Error Byte #0
 */
#define SBEOM		0x0080		/* End of Media */
#define ILLFMT		0x0040		/* Illegal Format */
#define LNGTERM 	0x0020		/* Long Term Command in Progress */
#define FAILINIT 	0x0010		/* Failed Tape initialize Command */
#define BADCMD		0x0004		/* Bad Command for the 5217 */
#define BAD217		0x0002		/* 5217 Command rejected */
#define BAD215		0x0001		/* 5215 Command Rejected */
/*
 * Bits of the Hard Error Byte #1
 */
#define WRPROT		0x0080		/* Write Protected */
#define NOTRDY		0x0040		/* Unit Not ready */
#define INVADDR 	0x0020		/* Invalid Address ???? */
#define NOTAPE		0x0010		/* No Tape Cartridge */
#define INVCMD  	0x0008		/* Invalid Command see byte 0 */
#define TIMEOUT 	0x0004		/* Timeout on Tape Operation */
#define LGTHERR 	0x0001		/* Length Error /512 bytes */

#define NRETRY	14			/* Number of disk retries */

#define	CLEAR()		*mdsoftc.sc_ioaddr = 0
#define	START()		*mdsoftc.sc_ioaddr = 1
#define	RESET()		*mdsoftc.sc_ioaddr = 2
#define	WUB()		((struct wub *)mdsoftc.sc_wub)

#ifdef	KERNEL

/* minor device assignments */
#define	TAPEMINOR(d)	((d)&0x7f)	/* Tape minors */
#define FLP_TYPE(d)	(((d)>>5)&7)	/* Type of Floppy Disk Drive */
#define TAPEDRIVE(d)	(((d)>>7)&0x01)

#define	WIN_DRIVES	2		/* Number of Winchester Units */
#define QIC_DRIVES 	2		/* Number of Tape Drives */
#define FLP_DRIVES	2		/* Number of Floppy Drives */

#define	BLK_SIZE	512		/* Physical block (sector) size */
#define	BLK_SHIFT	9		/* log2(BLK_SIZE) */

#define FLP_DBLDENS	0x000
#define FLP_SGLDENS	0x001
#define FLP_SGLSIDE	0x010
#define FLP_DBLSIDE	0x000
#define FLP_SEC256	0x100
#define	FLP_SEC512	0x000

/* Tape minors are from 0 - 0x7f */
#define TAPE_NORMAL	0x00		/* NEW PROMS Rev. E-L (Wangtek) */
#define TAPE_NOREWIND	0x01
#define TAPE_OLDPROMS	0x04
#define TAPE_TEST	0x20
/* #define TAPE_IOCTL	0x40 */
#define	INFINITY 2000000000	/* Large positive number */

#define	b_type	b_error			/* Device type temporary */

/* software controller state for the winchester and floppy */
struct	softc_disk {
	short	sc_flags;		/* state of disk (see below) */
	short	sc_spc;			/* sectors per cylinder */
	short	sc_cyl, sc_sec, sc_hd;	/* # of cylinders, sectors, heads */
	short	sc_blksize;		/* floppy block size */
	u_char	sc_blkshift;		/* log2 of sc_blksize */
	u_char	sc_unit;		/* unit # */
	struct	disk_map sc_fs[NFS];	/* file system map */
};

/* software controller state for the tape */
struct	softc_tape {
	short	sc_flags;		/* state of tape (see below) */
	u_char	sc_status;		/* drive status after an error */
	u_char	sc_unit;		/* unit # */
	daddr_t	sc_blkno;		/* current tape block # */
	daddr_t	sc_fileno;		/* current tape file # */
	daddr_t	sc_nxrec;		/* next tape block # */
	short	sc_lastaccess;		/* last access condition */
#if 0
	int	sc_resid;		/* byte cnt not done in prev I/O */
#endif
};

/*
 * sc_flags bits:
 *	- most of these bits are for tape drives
 *	- the wanted, busy, and alive bits apply to controllers
 *	  as well as drives
 */
#define	SC_WANTED	0x0001		/* device is wanted */
#define	SC_BUSY		0x0002		/* device is busy */
#define	SC_ALIVE	0x0004		/* device is alive */
#define	SC_HARDERROR	0x0008		/* tape has a permanent hard error */
#define	SC_LONGCMD	0x0010		/* tape long command in progress */
#define	SC_SHORTCMD	0x0020		/* tape short command in progress */
#define	SC_WRITTEN	0x0040		/* tape has been written on */
#define	SC_PROTECT	0x0080		/* tape is write protected */
#define	SC_TAPEINUSE	0x0100		/* tape is in use */
#define SC_READ		0x0200		/* tape was read from */
#define SC_CONFIGURED	0x0400		/* disks have been configured */

/* controller state */
struct	softc {
	short	sc_flags;		/* current controller state */
	daddr_t	sc_blkno;		/* holds current physical block # */
	ccb_t	sc_ccb;			/* channel control block */
	cib_t	sc_cib;			/* controller invocation block */
	iopb_t	sc_iopb;		/* i/o parameter block */
	inist_t	sc_inist;		/* init block/status block */
	caddr_t	sc_ioaddr;		/* virtual address of i/o port */
	struct	wub *sc_wub;		/* virtual address of wub */
	struct	iobuf sc_tab;		/* work/active queue */
	struct	softc_disk sc_disk[WIN_DRIVES];
	struct	softc_disk sc_floppy[FLP_DRIVES];
	struct	softc_tape sc_tape[QIC_DRIVES];
} mdsoftc;

#endif
