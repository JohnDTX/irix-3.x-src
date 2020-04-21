/*
** ipreg.h 	- Copyright (C) KCC Computer Services - Los Gatos CA 95030
**		- Chase - December 1983
**		- Any use, copy or alteration is strictly prohibited
**		- and is morally inexcusable
**		- unless authorized in writing by KCC Computer Services.
**
** $Source: /d2/3.7/src/stand/include/RCS/iphreg.h,v $
** $Revision: 1.1 $
** $Date: 89/03/27 17:13:45 $
*/
/*
** Header file for the Interphase 2190 controller from Interphase
*/

/*
** Multibus I/O Registers
*/
#define IP_R1	0
#define IP_R0	1	/* For the SUN CPU they must be byte swapped */
#define IP_R3	2
#define IP_R2	3

#define	IPR0	(ipsoftc.sc_ioaddr+IP_R0)
#define	IPR1	(ipsoftc.sc_ioaddr+IP_R1)
#define	IPR2	(ipsoftc.sc_ioaddr+IP_R2)
#define	IPR3	(ipsoftc.sc_ioaddr+IP_R3)
#define	CLEAR()	*IPR0 = IP_CLEAR
#define	START()	*IPR0 = IP_GO

/*
** Codes for Writing to IP_R0
*/
#define IP_16BITS	0x20			/* 16 Bit Bus */
#define IP_GO		(1 | IP_16BITS)	/* Start the Controller */
#define IP_CLEAR	(2 | IP_16BITS)	/* Clear the interrupt */

/*
** Codes for Reading from IP_R0
*/
#define IP_BUSY	1	/* Operation is in Progress */
#define IP_DONE	2	/* Operation is Complete */

/*
 * The IOPB -- I/O Parameter Block
 */
typedef struct iopb {
	u_char	i_option;		/* 1	-- command option */
	u_char 	i_cmd;			/* 0	-- command */
	u_char	i_error;		/* 3	-- error code */
	u_char 	i_status;		/* 2	-- command status */
	u_char	i_head;			/* 5	-- head select */
	u_char	i_unit;			/* 4	-- unit select */
	u_char	i_cyll;			/* 7	-- cylinder select lsb */
	u_char	i_cylh;			/* 6	-- cylinder select msb */
	u_char	i_secl;			/* 9	-- sector select lsb */
	u_char	i_sech;			/* 8	-- sector select msb */
	u_char	i_sccl;			/* b	-- sector count lsb */
	u_char	i_scch;			/* a	-- sector count msb */
	u_char	i_bufh;			/* d	-- buffer address high */
	u_char	i_dmacount;		/* c	-- dma count */
	u_char	i_bufl;			/* f	-- buffer address low */
	u_char	i_bufm;			/* e	-- buffer address mid */
	u_char	i_iol;			/* 11	-- I/O Address low */
	u_char	i_ioh;			/* 10	-- I/O Address high */
	u_char	i_rell;			/* 13	-- relative address low */
	u_char	i_relh;			/* 12	-- relative address high */
	u_char	i_linkh;		/* 15	-- linked iopb address high */
	u_char	i_reserved;		/* 14   -- reserved */
	u_char	i_linkl;		/* 17	-- linked iopb address low */
	u_char	i_linkm;		/* 17	-- linked iopb address mid */
} iopb_t;

/*
 * Some of the IOPB changes for formatting.
 */
#define I_THEAD	i_secl			/* 9	-- Target head select */
#define I_TCYLL i_sccl			/* b	-- Target cylinder select low */
#define I_TCYLH i_scch			/* a	-- Target cylinder select */

/*
 * commands used in i_cmd
 */
#define C_READ		0x81		/* read data */
#define C_WRITE		0x82		/* write data */
#define C_VERIFY	0x83		/* Verify data */
#define C_FORMAT	0x84		/* Format a track */
#define C_MAP		0x85		/* Map a track */
#define C_REPORT	0x86		/* Report The Controller info */
#define C_INIT		0x87		/* Initialize a drive */
#define C_RESTORE	0x89		/* Restore a drive */
#define C_SEEK		0x8a		/* Seek */
#define C_ZERO		0x8b		/* Zero a sector */
#define C_RESET		0x8f		/* Reset the Controller */
#define C_READDIR	0x91		/* Read Direct */
#define C_WRITEDIR	0x92		/* Write Direct */
#define C_READABSOLUTE	0x93		/* Read with no caching */
#define C_READNOCACHE	0x94		/* Read with no caching */

/*
 * options for i_options
 */
#define O_IOPB		0x10		/* get iopb with 16 bit bus */
#define O_BUF		0x01		/* do dma with 16 bits */

/*
 * bits returned in the status
 */
#define S_OK		0x80		/* Ok */
#define S_BUSY		0x81		/* Operation in progress, busy */
#define S_ERROR		0x82		/* Error on the last command */

/*
 * Error Codes returned in the IOPB
 */
#define E_NOTRDY	0x10		/* Disk not ready */
#define E_INVADDR	0x11		/* Invalid Disk Address */

/*
 * The Unit Initialization block -- UIB
 */

struct uib {
	u_char	u_spt;			/*  1 *//* Sectors per track */
	u_char	u_hds;			/*  0 *//* Heads */
	u_char	u_bpsh;			/*  3 *//* Bytes per sector high */
	u_char	u_bpsl;			/*  2 *//* Bytes per sector high */
	u_char	u_gap2;			/*  5 *//* bytes in gap 2 */
	u_char	u_gap1;			/*  4 *//* bytes in gap 1 */
	u_char	u_retry;		/*  7 *//* retry count */
	u_char	u_ilv;			/*  6 *//* interleave factor */
	u_char	u_reseek;		/*  9 *//* reseek enable */
	u_char	u_eccon;		/*  8 *//* ecc enable */
	u_char	u_inchd;		/*  b *//* increment head enable */
	u_char	u_mvbad;		/*  a *//* move bad data enable */
	u_char	u_intron;		/*  d *//* interrupt on status change */
	u_char	u_dualp;		/*  c *//* dual port drive */
	u_char	u_group;		/*  f *//* Group size for cache */
	u_char	u_skew;			/*  e *//* spiral skew factor */
	u_char	u_resv2;		/* 11 *//* reserved */
	u_char  u_resv1;		/* 10 *//* reserved */
	u_char	u_resv3;		/* 12 *//* reserved */
};

typedef struct uib uib_t;

/* Miscellaneous variables for the disk */

#define BLK_SIZE	512		/* Size for the physical sectors */
#define BLKSIZE		512		/* Size for the physical sectors */
#define BLK_SHIFT	9		/* log2(BLK_SIZE) for shifting */

#define IP_ILV		2		/* Interleave factor */
#define IP_RETRY	3		/* Retry count */

/*
** Might have a major problem with time on the bus
*/
#define IP_DMACOUNT	8		/* Burst length for DAM-DMA xfers */
#define IP_GROUPSIZE	16		/* Almost Half a track */
#define IP_WAIT		1		/* Wait for command to complete */
#define IP_NOWAIT	0		/* Don't Wait for command to complete */
#define IP_GROUPENABLE  0x80		/* Enable the Grouping with Caching */
#define IP_CACHEENABLE  0x40		/* Enable the Caching */

/* Macros for the high -- mid -- low bytes of longs */

#define LB(x)	((long)(x) & 0xff)		/* Low Byte */
#define MB(x)	(((long)(x) >> 8) & 0xff)	/* Mid Byte */
#define HB(x)	(((long)(x) >> 16) & 0x0f)	/* High Byte (HACK) */

#ifdef KERNEL

#define SPL()	spl5()				/* Interrupt Level */
#define MAX_DRIVES	4		/* Maximum Number of Disk drives */

/* offets for error and command codes */
#define ERROFFSET	0x10
#define CMDOFFSET	0x80

/* sc_flags bits */
#define SC_ALIVE	0x01

/* per drive data base */
struct	softc_disk {
	short	sc_flags;		/* drive state bits */
	short	sc_spc;			/* sectors per cylinder */
	struct 	disk_map sc_fs[NFS];	/* file system map */
	short	sc_cyl;			/* cylinders on drive */
	char	sc_sec;			/* sectors per track */
	char	sc_hd;			/* heads on drive */
};

/* controller data base */
struct	softc {
	short	sc_flags;		/* controller state bits */
	caddr_t	sc_ioaddr;		/* addr of Multibus i/o registers */
	iopb_t	*sc_iopb;		/* iopb */
	uib_t	*sc_uib;		/* uib */
	long	sc_iopb_mbva;		/* multibus address of iopb */
	struct	iobuf sc_tab;		/* work/active queue */
	struct	buf sc_diskbuf;		/* raw buffer header */
	struct	softc_disk sc_disk[MAX_DRIVES];
} ipsoftc;

#endif
