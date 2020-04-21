/*
** xyreg.h
**
** $Source: /d2/3.7/src/sys/multibus/RCS/xyreg.h,v $
** $Revision: 1.1 $
** $Date: 89/03/27 17:31:51 $
*/
/*
** Xylogics 450 multibus SMD disk controller
** registers and bits.
*/

#define u_char	unsigned char
#define u_short	unsigned short
#define u_long unsigned long

/*
** Multibus I/O Registers for the Xylogics 450 Controller
** Byte swapped
*/
#define XY_R1	0 	/* IOPB Relocation Buffer High Byte */
#define XY_R0	1	/* IOPB Relocation Buffer Low Byte */
#define XY_R3	2	/* IOPB Address Register High Byte */
#define XY_R2	3	/* IOPB Address Register Low Byte */
#define XY_R5	4	/* Controller Reset/Update Register */
#define XY_R4	5	/* Controller Status Register */
/*
 * Codes for Writing to XY_Registers
 */
/*
** bits of xycsr Xylogics Controller - Status Register
*/
#define	CSR_GOBUSY	0x80	/* execute an iopb (write), busy (read) */
#define	CSR_ERROR	0x40	/* general error */
#define	CSR_DOUBLEERROR	0x20	/* bus error (double error) */
#define	CSR_INTERRUPT	0x10	/* interrupt pending */
#define	CSR_24BITADDR	0x08	/* 24 bit address mode when set */
#define	CSR_ATTENREQ	0x04	/* attention request */
#define	CSR_ATTENACK	0x02	/* attention acknowledge */
#define	CSR_DRIVEREADY	0x01	/* selected drive is ready */
#define XY_GO (CSR_GOBUSY | CSR_24BITADDR)
#define XY_CLEAR CSR_INTERRUPT 

#define	CLEAR()	(*((char *)(xy_ioaddr+XY_R4)) = XY_CLEAR)
#define	START()	(*((char *)(xy_ioaddr+XY_R4)) = XY_GO)
#define RESET() (i = (*((char *)(xy_ioaddr+XY_R5))))
#define XYSTATUS() (i = (*((char *)(xy_ioaddr+XY_R4))))

#define HB(x)		(((long)x >> 16) & 0xff)
#define MB(x)		(((long)x >> 8) & 0xff)
#define LB(x)		((long)x & 0xff)

#define ADDR0(x)  	((long)x & 0xff)
#define ADDR1(x)  	(((long)x >> 8) & 0xff)
#define ADDR2(x)  	(((long)x >> 16) & 0xff)
#define ADDR3(x)  	(((long)x >> 24) & 0xff)

struct xy_iopb
{
	u_char	i_imode;	/*  1 *//* interrupt mode */
	u_char	i_cmd;		/*  0 *//* disk command */
	u_char	i_status2;	/*  3 *//* status */
	u_char	i_status1;	/*  2 *//* status */
	u_char	i_drive;	/*  5 *//* drive type, unit select */
	u_char	i_throttle;	/*  4 *//* throttle */
	u_char	i_sector;	/*  7 *//* sector address */
	u_char	i_head;		/*  6 *//* head address */
	u_char	i_cylh;		/*  9 *//* cylinder address */
	u_char	i_cyll;		/*  8 *//* cylinder address */
	u_char	i_scch;		/*  b *//* sector count */
	u_char	i_sccl;		/*  a *//* sector count */
	u_char	i_bufh;		/*  d *//* memory address */
	u_char	i_bufl;		/*  c *//* memory address */
	u_char	i_relh;		/*  f *//* relocation part of mem address */
	u_char	i_rell;		/*  e *//* relocation part of mem address */
	u_char	i_reserved;	/* 11 *//* not used */
	u_char	i_headoffset;	/* 10 *//* head offset */
	u_char	i_nextioph;	/* 13 *//* next iopb address */
	u_char	i_nextiopl;	/* 12 *//* next iopb address */
	u_char	i_eccmaskh;	/* 15 *//* ECC mask pattern */
	u_char	i_eccmaskl;	/* 14 *//* ECC mask pattern */
	u_char	i_eccaddrh;	/* 17 *//* ECC bit address */
	u_char	i_eccaddrl;	/* 16 *//* ECC bit address */
};

typedef struct xy_iopb xy_iopb_t;

/*
** IOPB -- Commands for byte 0.
*/
#define	C_NOP		0x00	/* no operation */
#define	C_WRITE		0x01	/* write */
#define	C_READ		0x02	/* read */
#define C_WRITEHEADER	0x03	/* write track headers */
#define C_READHEADER	0x04	/* read track headers */
#define	C_SEEK		0x05	/* seek */
#define	C_RESET		0x06	/* drive reset */
#define	C_FORMAT	0x07	/* write format */
#define	C_XREADHEADER	0x08	/* read header, data, and ECC */
#define	C_READSTATUS	0x09	/* read drive status */
#define	C_XWRITEHEADER	0x0a	/* write header, data, anc ECC */
#define	C_SETSIZE	0x0b	/* set drive size */
#define	C_SELFTEST	0x0c	/* self test */
#define	C_RESERVED	0x0d	/* Reserved */
#define	C_MAINLOAD	0x0e	/* maintenance buffer load */
#define	C_MAINDUMP	0x0f	/* maintenance buffer dump */
/*
** Bits used in bits in the Command byte
*/
#define	C_INTERRUPT	0x10	/* interrupt enable */
#define	C_COMMANDCHAIN	0x20	/* command-chaining enable */
#define	C_RELOCATION	0x40	/* iopb/data relocation enable */
#define	C_IOPBUPDATE	0x80	/* auto iopb update enable */
/*
** IOPB -- IMODE bits (interrupt mode / function modification)
*/
#define	IM_ENABLEINT		0x40	/* interrupt on each iopb */
#define	IM_ERRORINT		0x20	/* interrupt on error */
#define	IM_DUALPORT		0x10	/* hold dual port */
#define	IM_RETRY		0x08	/* automatic seek retry */
#define	IM_ENABLEEXT		0x04	/* enable extended function */
#define IM_ENABLEECC		0x03	/* bits of the ecc mode */
/*
** Modes for ECC recovery
*/
#define	IM_ECCMODE0		0x00	/* enable automatic ECC correction */
#define	IM_ECCMODE1		0x01	/* enable automatic ECC correction */
#define	IM_ECCMODE2		0x02	/* enable automatic ECC correction */
#define	IM_ECCMODE3		0x03	/* enable automatic ECC correction */

/*
** IOPB -- Status codes for the iopb status byte Number 1.
*/
#define	S_ERROR		0x80	/* error */
#define	S_DONE		0x01	/* command done */
/*
** For the summary of completion error codes see "data.c" for the actual
** char array for error printouts.
*/

/*
** IOPB -- Throttle bits
*/
#define	T_BYTEENABLE	0x80	/* byte mode for DMA */
#define	T_T2		0x00	/*   2 word/byte throttle */
#define	T_T4		0x01	/*   4 word/byte throttle */
#define	T_T8		0x02	/*   8 word/byte throttle */
#define	T_T16		0x03	/*  16 word/byte throttle */
#define	T_T32		0x04	/*  32 word/byte throttle */
#define	T_T64		0x05	/*  64 word/byte throttle */
#define	T_T128		0x06	/* 128 word/byte throttle */

/*
** IOPB -- returned when drive status is executed into i_sccl
*/
#define	S_ONCYLINDER	0x80	/* on cylinder, when not set (low) */
#define	S_DRIVEREADY	0x40	/* drive ready, when not set (low) */
#define	S_WRITEPROTECT	0x20	/* drive write protected, when set (high) */
#define	S_DUALPORTBUSY	0x10	/* dual port busy, when set (high) */
#define	S_SEEKERROR	0x08	/* seek error, when set (high) */
#define	S_DRIVEFAULT	0x04	/* drive faulted, when set (high) */

#ifdef KERNEL

char *xyerrlist[] = {
/* 00-03  */ "Successful", "Int pending", "Reserved", "Busy Conflict",
/* 04-07  */ "Op Time out", "No Header", "ECC Hard Error", "Illegal Cylinder",
/* 08-0b  */ "Reserved", "Sector Slip", "Illegal Sector", "Reserved",
/* 0c-0f  */ "Reserved", "Last Sector too Small", "ACK Error", "Reserved",
/* 10-13  */ "Reserved", "Reserved", "Track Header Error", "Seek Retry",
/* 14-16  */ "Write Protected", "Illegal Command", "Drive Not Ready",
/* 17-19  */ "Sector Count Zero", "Drive Faulted", "Illegal Sector Size",
/* 1a-1d  */ "Self Test A", "Self Test B", "Self Test C", "Reserved",
/* 1e-1f  */ "Soft ECC Error", "Soft ECC error Sucessful",
/* 20-21  */ "Illegal Head Error", "Disk Sequencer Error",
/* 22-25  */ "Reserved", "Reserved", "Reserved", "Seek Error",
		0
};

char *xycmdlist[] = {
/* 0-4 */ "NOP", "Write", "Read", "Write Track Header", "Read Track Header",
/* 5-8 */ "Seek", "Drive Clear", "Write Format", "Read Header, Data and ECC",
/* 9-b */ "Read Drive Status", "Write Header, Data and ECC", "Set Drive Size",
/* c-e */ "Self Test", "Reserved", "Maintenance Buffer Load", 
/* f   */ "Maintenance Buffer Dump",
	   0
};

#define FS(d)	(minor(d) & 7)
#define DRIVE(d) ((minor(d) >> 3) & 3)
#define SPL()	spl4()				/* Interrupt Level */
#define MAX_DRIVES	4		/* Maximum Number of Disk drives */
#define MX_FS		8		/* Maximum Number of File Systems */
#define XY_LOG		9
#define BLKSIZE		512
#define XY_RETRY	3

struct iobuf xytab;
struct buf xyrtab;

#define INITED	0x01

struct	xyinfo {
	short	xy_bpc;	/* Calculated blocks (sectors) per cyl */
	struct 	disk_map xy_fs[NFS];	/* File system map */
	short	xy_cyl;		/* Configuration info */
	char	xy_sec;
	char	xy_hd;
	char	xy_flags;	/* misc flags */
	char	xy_ilv;
} xydb[MAX_DRIVES];

#endif 			/* KERNEL */
