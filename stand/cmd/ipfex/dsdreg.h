/*
**	$Source: /d2/3.7/src/stand/cmd/ipfex/RCS/dsdreg.h,v $
**	$Revision: 1.1 $
**	$Date: 89/03/27 17:12:21 $
*/

#define	mdCLEAR() (*((char *)(md_ioaddr+1)) = zero)
#define	mdSTART() (*((char *)(md_ioaddr+1)) = 1)
#define	mdRESET() (*((char *)(md_ioaddr+1)) = 2)

#ifdef pmII
#define	mdSWAPW(x) ((char *)((((ULONG)x)<<16)|(((ULONG)x)>>16)))
#endif

#ifdef juniper
#include "mbenv.h"
#define	mdSWAPW(x) ((char *)((((ULONG)mbvtop(x))<<16)|(((ULONG)mbvtop(x))>>16)))
#endif

struct	wub {
	UCHAR	w_xx;		/*1  *//* UNUSED */
	UCHAR	w_ext;		/*0  *//* extension */
	char	*w_ccb;		/*2-5*//* Ptr to ccb */
};
typedef	struct wub wub_t;

#define W_EXT  7		/* Extended Addressing for 24 bits */

struct	ccb {
	UCHAR	c_busy;		/*1  *//* Busy 1 flag FF=Busy; 00=Idle*/
	UCHAR	c_ccw1;		/*0  *//* Channel Control Word 1 (==1) */
	char	*c_cib;		/*2-5*//* Ptr to cib */
	USHORT	c_xx;		/*6-7*//* UNUSED */
	UCHAR	c_busy2;	/*9  *//* UNUSED */ /* Busy 2 flag */
	UCHAR	c_ccw2;		/*8  *//* UNUSED */ /* Channel Control Word 2 */
	char	*c_cp;		/*a-d*//* UNUSED */ /* CP Pointer */
	USHORT	c_ctrlptr;	/*e-f*//* Control Pointer (==4) */
};
typedef struct ccb ccb_t;

struct	cib {
	UCHAR	i_opstat;	/*1  *//* Operation Status */
	UCHAR	i_xx;		/*0  *//* UNUSED */ /* Reserved */
	UCHAR	i_stsem;	/*3  *//* Status Semaphore */
	UCHAR	i_cmdsem;	/*2  *//* UNUSED */ /* Command Semaphore */
	char	*i_csa;		/*4-7*//* UNUSED */ /* must be all zeros */
	char	*i_iopb;	/*8-b*//* IOPB Pointer */
	long	i_xx2;		/*c-f*//* UNUSED */ /* must be all zeros */
};
typedef struct cib cib_t;

/* i_opstat codes */
#define	OPCOMP		0x01		/* Operation complete */
#define	SKCOMP		0x02		/* Seek Complete */
#define	MCH		0x04		/* Media Change */
#define TPCHG		0x0e		/* Tape Media Change */
#define TPLONG		0x0f		/* Tape Long Term Command */
#define	UNIT		0x30		/* UNIT bits */
#define	HARD		0x40		/* Hard Error */
#define	SUMMARY		0x80		/* Summary Error */
#define MASK		0xf0		/* Status Mask */

struct	mdiopb {
	long	p_xx;
	long	p_atc;		/* Actual Transfer Count */
	USHORT	p_dev;		/* Device code */
	UCHAR	p_func;		/* Function */
	UCHAR	p_unit;		/* Unit */
	USHORT	p_mod;		/* Modifier */
	USHORT	p_cyl;		/* Cylinder */
	UCHAR	p_sec;		/* Sector */
	UCHAR	p_hd;		/* Head */
	char	*p_dba;		/* Data Buffer Address */
	long	p_rbc;		/* Requested Byte Count */
	char	*p_gap;		/* General Address Pointer */
};
typedef struct mdiopb mdiopb_t;

/* p_dev device codes */
#define	D_WIN		0x0000	/* Winchester */
#define	D_FLP		0x0001	/* Floppy */
#define	D_TAPE		0x0010	/* Streaming Tape Drive */
#define D_217		0x0004	/* 217 Streaming Tape Drive */

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
#define F_TSPREC	0x1a	/* Tape space forward a record */
#define F_TRESET	0x1c	/* Tape reset */
#define F_TRETEN	0x1d	/* Tape retension */
#define F_TDSTAT	0x1e	/* Tape drive status */
#define F_TTERM		0x1f	/* Tape read/write terminate */
/* OLD FASHIONED Tape commands */
#define	F_TRST		0x80	/* Tape Reset */
#define	F_BACKUP	0x81	/* Disk Image Backup */
#define	F_RESTORE	0x82	/* Disk Image Restore */
#define	F_TPSTAT	0x83	/* Tape Status */
#define	F_RETEN		0x84	/* Retension Tape */

/* p_mod codes */
#define	M_NOINT		0x0001	/* No Interrupt on completion */
#define	M_NORETRY	0x0002	/* Inhibit retries for error recovery */
#define	M_ALLOWDD	0x0004	/* Allow Deleted Data to be read/written */
#define M_TAPELG	0x0040  /* Read error status after long tape cmd */

struct	inib {
	USHORT	i_ncyl;		/* Number of cylinders */
	UCHAR	i_rhd;		/* Removable heads */
	UCHAR	i_fhd;		/* Fixed heads */
	UCHAR	i_bpsl;		/* Bytes Per Sector Low */
	UCHAR	i_spt;		/* Sectors Per Track */
	UCHAR	i_nacyl;	/* Number of Alternate Cylinders */
	UCHAR	i_bpsh;		/* Bytes Per Sector High */
};

struct	fmtb {
	UCHAR	f_pat1;		/* Pattern Byte 1 */
	UCHAR	f_func;		/* Which format function */
	UCHAR	f_pat3;		/* Pattern Byte 3 */
	UCHAR	f_pat2;		/* Pattern Byte 2 */
	UCHAR	f_ilv;		/* Interleave */
	UCHAR	f_pat4;		/* Pattern Byte 4 */
};

struct	statb {
	UCHAR	sb[14];
};

struct tapest {
	UCHAR tb[12];
};

struct alttrk {
	USHORT cylinder;		/* 0-1 cylinder */
	UCHAR sector;		/* 3   sector */
	UCHAR head;		/* 2   head */
	UCHAR xx;		/* 5   xx */
	UCHAR flags;		/* 4   flags */
};

typedef struct alttrk alttrk_t;

/* Tape stuff for the 215 code */
#define HRDBYT0		0x01
#define HRDBYT1		0x00

/* Byte 0 */
#define T_MASK0		0x7f
#define T_NOTAPE	0x40
#define T_NODRIVE	0x20
#define T_WRPROT	0x10
#define T_ENDTRK	0x08
#define T_DATERR	0x04
#define T_UNRERR	0x02
#define T_FLMARK	0x01
/* Byte 1 */
#define T_MASK1		0x7e
#define T_ILLCMD	0x40
#define T_NODATA	0x20
#define T_RETRIES	0x10
#define T_BOT		0x08
#define T_RESV2		0x04
#define T_RESV1		0x02
#define T_PWRESET	0x01

/*
 * Flags for reading sector ids
 */
#define GOOD		0x00
#define BAD		0x01
#define NORMAL		0x00
#define ALTERNATE	0x01
#define DEFECTIVE	0x02
#define INVALID		0x03
#define ALTTRK(x)	((x >> 6) & 0x03)
