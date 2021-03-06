/* $Header: /d2/3.7/src/stand/pm2mon/dev/RCS/mdrblk.h,v 1.1 89/03/27 17:16:57 root Exp $ */
/* $Log:	mdrblk.h,v $
 * Revision 1.1  89/03/27  17:16:57  root
 * Initial check-in for 3.7
 * 
 * Revision 1.1  87/05/02  12:44:08  andre
 * Initial revision
 * 
 * Revision 1.2  84/03/28  09:19:13  root
 * release script
 * 
 * Revision 1.1  84/02/27  16:14:31  root
 * release script
 * 
 * Revision 1.1  84/02/27  15:59:05  root
 * release script
 * 
 * Revision 1.1  84/01/12  21:32:42  gb
 * snapshot script
 * 
 * Revision 1.1  83/11/09  01:49:31  djb
 * Initial revision
 *  */

/* #define DEBUG 1				/*****/




typedef unsigned long ULONG;
typedef unsigned int  UINT;
typedef unsigned short USHORT;
typedef unsigned char UCHAR;

extern short drive;
extern long boffset;


#define SWAPW(x)	(((UINT)(x) <<16)|((UINT)(x) >>16))

/*#endif*/

struct	wub {
	UCHAR	w_xx;
	UCHAR	w_ext;		/* extension */
	UCHAR	*w_ccb;		/* Ptr to ccb */
};
typedef	struct wub wub_t;

#define W_EXT  7		/* Extended Addressing for 24 bits */
struct	ccb {
	UCHAR	c_busy1;	/* Busy 1 flag (==1) */
	UCHAR	c_ccw1;		/* Channel Control Word 1 */
	UCHAR	*c_cib;		/* Ptr to cib */
	USHORT	c_xx;
	UCHAR	c_busy2;	/* Busy 2 flag */
	UCHAR	c_ccw2;		/* Channel Control Word 2 (==1) */
	UCHAR	*c_cp;		/* CP Pointer */
	USHORT	c_ctrlptr;	/* Control Pointer (==4) */
};
typedef struct ccb ccb_t;

struct	cib {
	UCHAR	i_opstat;	/* Operation Status */
	UCHAR	i_xx;
	UCHAR	i_stsem;	/* Status Semaphore */
	UCHAR	i_cmdsem;	/* Command Semaphore */
	UCHAR	*i_csa;		/* Control Store Address */
	UCHAR	*i_iopb;	/* IOPB Pointer */
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
	UCHAR	*p_dba;		/* Data Buffer Address */
	ULONG	p_rbc;		/* Requested Byte Count */
	UCHAR	*p_gap;		/* General Address Pointer */
};
typedef struct iopb iopb_t;

/* p_dev device codes */
#define	D_WIN		0x0000	/* Winchester */
#define	D_FLP		0x0001	/* Floppy */
#define D_217		0x0004	/* 217 Tape */

/* p_func operations */
#define	F_INIT		0x00	/* Initialize */
#define	F_TSTAT		0x01	/* Transfer Status */
#define	F_READ		0x04	/* Read Data */
#define	F_WRITE		0x06	/* Write Data */

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




#define WUBADDR (IOAddr<<4)

#define CIBADDR 0x0010
#define CCBADDR 0x0020
#define IOPBADDR 0x0030
#define INIBADDR 0x0050
#define SBADDR 0x0070
#define SECSIZE 512

extern char *mdvarp;
#define MDKLUGESIZE	0x1000

#define IOAddr 0x7f00

#define MDIOADDR  ((struct mddevice *)mbiotov(IOAddr))
#define	WUB  ((struct wub *)ptov(WUBADDR))
#define	CIB  ((cib_t *)(mdvarp + CIBADDR))
#define	CCB  ((ccb_t *)(mdvarp + CCBADDR))
#define	IOPB  ((iopb_t *)(mdvarp + IOPBADDR))
#define	INIB  ((inib_t *)(mdvarp + INIBADDR))
#define STATB ((UCHAR *)(mdvarp + SBADDR))
struct mddevice {
	char md_ptr1;	/* Not Used */
	char md_ptr0;	/* Used for clear, start and reset */
};
#define CLEAR(rp)	(rp->md_ptr0 = mdZero)	/* Clear the Interrupt */
#define START(rp)	(rp->md_ptr0 = 1)	/* Start the Process */
#define RESET(rp)	(rp->md_ptr0 = 2)	/* Reset the Board */
int mdZero = 0;

