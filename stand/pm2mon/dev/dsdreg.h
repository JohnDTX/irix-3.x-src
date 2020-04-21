/*
** $Source: /d2/3.7/src/stand/pm2mon/dev/RCS/dsdreg.h,v $
** $Date: 89/03/27 17:16:40 $
** $Revision: 1.1 $
*/

/* #define DEBUG 1				/*****/


typedef unsigned long ULONG;
typedef unsigned short USHORT;
typedef unsigned char UCHAR;

extern short drive;
extern long boffset;
extern short disktype;

#define SWAPW(x)	(((ULONG)(x) <<16)|(((ULONG)(x) >>16)))

struct	wub {
	UCHAR	w_xx;
	UCHAR	w_ext;		/* extension */
	char	*w_ccb;		/* Ptr to ccb */
};
typedef	struct wub wub_t;

#define W_EXT  7		/* Extended Addressing for 24 bits */
struct	ccb {
	UCHAR	c_busy;		/* Busy 1 flag (==1) */
	UCHAR	c_ccw1;		/* Channel Control Word 1 */
	char	*c_cib;		/* Ptr to cib */
	USHORT	c_xx;
	UCHAR	c_busy2;	/* Busy 2 flag */
	UCHAR	c_ccw2;		/* Channel Control Word 2 (==1) */
	char	*c_cp;		/* CP Pointer */
	USHORT	c_ctrlptr;	/* Control Pointer (==4) */
};
typedef struct ccb ccb_t;

struct	cib {
	UCHAR	i_opstat;	/* Operation Status */
	UCHAR	i_xx;
	UCHAR	i_stsem;	/* Status Semaphore */
	UCHAR	i_cmdsem;	/* Command Semaphore */
	char	*i_csa;		/* Control Store Address */
	char	*i_iopb;	/* IOPB Pointer */
	ULONG	i_xx2;		/* Reserved */
};
typedef struct cib cib_t;

/* i_opstat codes */
#define	OPCOMP	0x01		/* Operation complete */
#define	SKCOMP	0x02		/* Seek Complete */
#define	MCH	0x04		/* Media Change */
#define	DTYPE	0x08		/* Drive Type (0=wini, 1=flp) */
#define	UNIT	0x30		/* UNIT bits */
#define	HARD	0x40		/* Hard Error */
#define	SUMMARY	0x80		/* Summary Error */

struct	iopb {
	ULONG	p_xx;
	ULONG	p_atc;		/* Actual Transfer Count */
	USHORT	p_dev;		/* Device code */
	UCHAR	p_func;		/* Function */
	UCHAR	p_unit;		/* Unit */
	USHORT	p_mod;		/* Modifier */
	USHORT	p_cyl;		/* Cylinder */
	UCHAR	p_sec;		/* Sector */
	UCHAR	p_hd;		/* Head */
	char	*p_dba;		/* Data Buffer Address */
	ULONG	p_rbc;		/* Requested Byte Count */
	char	*p_gap;		/* General Address Pointer */
};
typedef struct iopb iopb_t;

/* p_dev device codes */
#define	D_WIN		0x0000	/* Winchester */
#define	D_FLP		0x0001	/* Floppy */
#define D_217		0x0004
#define D_TAPE		0x0010	/* Regular tape is ??? */

/* p_func operations */
#define	F_INIT		0x00	/* Initialize */
#define	F_TSTAT		0x01	/* Transfer Status */
#define	F_READ		0x04	/* Read Data */
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
#define	F_TRST		0x80	/* Tape Reset */
#define	F_BACKUP	0x81	/* Disk Image Backup */
#define	F_RESTORE	0x82	/* Disk Image Restore */
#define	F_TPSTAT	0x83	/* Tape Status */
#define	F_RETEN		0x84	/* Retension Tape */


/* p_mod codes */
#define	M_NOINT		0x0001	/* No Interrupt on completion */

struct	inib {
	USHORT	i_ncyl;		/* Number of cylinders */
	UCHAR	i_rhd;		/* Removable heads */
	UCHAR	i_fhd;		/* Fixed heads */
	UCHAR	i_bpsl;		/* Bytes Per Sector Low */
	UCHAR	i_spt;		/* Sectors Per Track */
	UCHAR	i_nacyl;	/* Number of Alternate Cylinders */
	UCHAR	i_bpsh;		/* Bytes Per Sector High */
};
typedef struct inib inib_t;

struct	statb {
	UCHAR	sb[14];
};
typedef struct statb statb_t;

struct tapestat {
	UCHAR sb[12];
};
typedef struct tapestat tapestat_t;

#define SB_EXS 13		/* Extended status for the disk */
#define SB_RTRY 10		/* Retries for the disk */

/*
 * Bits of the Soft Error Byte
 */
#define BUFOVER 0x0040		/* Buffer Over/Under Run */
#define DRFAULT	0x0020		/* Drive Fault */
#define DATAERR	0x0008		/* Unrecoverable Data error */
#define RECDATA 0x0002		/* Recoverable Data Error with Retries */
/*
 * Bits of the Hard Error Byte #0
 */
#define SBEOM	0x0080		/* End of Media */
#define ILLFMT	0x0040		/* Illegal Format */
#define LNGTERM 0x0020		/* Long Term Command in Progress */
#define FAILINIT 0x0010		/* Failed Tape initialize Command */
#define BADCMD	0x0004		/* Bad Command for the 5217 */
#define BAD217	0x0002		/* 5217 Command rejected */
#define BAD215	0x0001		/* 5215 Command Rejected */
/*
 * Bits of the Hard Error Byte #1
 */
#define WRPROT	0x0080		/* Write Protected */
#define NOTRDY	0x0040		/* Unit Not ready */
#define INVADDR 0x0020		/* Invalid Address ???? */
#define NOTAPE	0x0010		/* No Tape Cartridge */
#define INVCMD  0x0008		/* Invalid Command see byte 0 */
#define TOTIMO  0x0004		/* Timeout on Tape Operation */
#define LGTHERR 0x0001		/* Length Error /512 bytes */

extern char *mdvarp;

#ifdef JCS
#define WUBADDR 0x0000
#define IOAddr 0x0000
#else
#ifdef PM2
#define WUBADDR (IOAddr<<4)
#define IOAddr 0x7f00
#else
#define WUBADDR 0x40
#define IOAddr 0x0004
#endif
#endif

#define CIBADDR 0x0010
#define CCBADDR 0x0020
#define IOPBADDR 0x0030
#define INIBADDR 0x0050
#define SBADDR 0x0070
#define STADDR 0x0090
#define SECSIZE 512
#define MDIOADDR  ((struct mddevice *)mbiotov(IOAddr))
#define	WUB  ((struct wub *)ptov(WUBADDR))
#define	CIB  ((cib_t *)(mdvarp + CIBADDR))
#define	CCB  ((ccb_t *)(mdvarp + CCBADDR))
#define	IOPB  ((iopb_t *)(mdvarp + IOPBADDR))
#define	INIB  ((inib_t *)(mdvarp + INIBADDR))
#define STATB ((char *)(mdvarp + SBADDR))
#define TAPEST ((tapestat_t *)(mdvarp + STADDR))
/*((unsigned)(((long)x) - MBMemVA)) */
struct mddevice {
	char md_ptr1;	/* Not Used */
	char md_ptr0;	/* Used for clear, start and reset */
};
#define CLEAR(rp,x)	((rp)->md_ptr0 = (x))	/* Clear the Interrupt */
#define START(rp)	((rp)->md_ptr0 = 1)	/* Start the Process */
#define RESET(rp)	((rp)->md_ptr0 = 2)	/* Reset the Board */

#define TYPE(x)		(x)

struct dsdtype {
	char *tname;
	short tcyl;
	short tacyl;
	short thd;
	short tsec;
	short tbpc;
};
