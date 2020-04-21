/*
**	stdreg.h	- Copyright (C), JCS Computer Services 1983
**			- Author: chase
**			- Date: December 1984
**			- Any use, copying or alteration is prohibited
**			- and morally inexcusable,
**			- unless authorized in writing by JCS.
**	$Source: /d2/3.7/src/sys/multibus/RCS/stdreg.h,v $
**	$Revision: 1.1 $
**	$Date: 89/03/27 17:31:41 $
*/

#define ICHECK		2
#define IFORCE		4

/*
** Multibus I/O Registers
** -- Byte Swapped --
*/
#define ST_R0	1
#define ST_R1	0
#define ST_R2	3
#define ST_R3	2
#define ST_R4	5
#define ST_R5	4
#define ST_R6	7
#define ST_R7	6

#define	STR0	(stdsoftc.sc_ioaddr+ST_R0)
#define	STR1	(stdsoftc.sc_ioaddr+ST_R1)
#define	STR2	(stdsoftc.sc_ioaddr+ST_R2)
#define	STR3	(stdsoftc.sc_ioaddr+ST_R3)
#define	STR4	(stdsoftc.sc_ioaddr+ST_R4)
#define	STR5	(stdsoftc.sc_ioaddr+ST_R5)
#define	STR6	(stdsoftc.sc_ioaddr+ST_R6)
#define	STR7	(stdsoftc.sc_ioaddr+ST_R7)

/*
** Macros for Writing to ST_R0 -- Command/Status Register
*/
#define	ST_RESET	0x80
#define ST_ABORT	0x40
#define ST_16BITS	0x20			/* 16 Bit Bus */
#define ST_NOINTERRUPT	0x10
#define ST_START	(1 | ST_16BITS)		/* Start the Controller */
/* #define ST_START	(1)			/* Start the Controller */
#define ST_CLEAR	(2 | ST_16BITS)		/* Clear the interrupt */

#define CLEAR()		*STR0 = ST_CLEAR
#define START()		*STR0 = (ST_START | ST_CLEAR)
#define OSTART()	*STR0 = (ST_START)
#define STARTWO()	*STR0 = (ST_START | ST_NOINTERRUPT)
#define ABORT()		*STR0 = (ST_ABORT | ST_NOINTERRUPT)
#define RESET()		*STR0 = (ST_RESET | ST_NOINTERRUPT)

/*
** Macros for Reading from ST_R0 -- Command/Status Register
*/
#define ST_BUSY		1		/* Operation is in Progress */
#define ST_DONE		2		/* Operation is Complete */
#define	ST_CHANGE	4		/* status change interrupt */
#define ST_0DRIVE	0x10
#define ST_1DRIVE	0x20
#define STATUSREG()	*STR0
#define RDINTR()	*STR1

/*
** The IOPB -- I/O Parameter Block
*/
struct iopb {
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
	u_char	i_tpcopy;		/* 14   -- tape unit (copy only) */
	u_char	i_linkl;		/* 17	-- linked iopb address low */
	u_char	i_linkm;		/* 16	-- linked iopb address mid */

	/* rest is used by unix */
	struct	iopb *i_next;		/* next iopb in unix chain */
	long	i_mbva;			/* multibus virtual addr of this iopb */
	struct	buf *i_bp;		/* buffer this iopb is for */
};
typedef struct iopb iopb_t;

/*
** Some of the IOPB changes for formatting.
*/
#define I_THEAD	i_secl			/* 9	-- Target head select */
#define I_TCYLL i_sccl			/* b	-- Target cylinder select low */
#define I_TCYLH i_scch			/* a	-- Target cylinder select */

/*
** commands used in i_cmd
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
#define C_REFORMAT	0x8b		/* Reformat the Disk */
#define C_RESET		0x8f		/* Reset the Controller */
#define C_RDABSOLUTE	0x93		/* Read Absolute */
#define C_READNOCACHE	0x94		/* Read No Cache */
/*
** The Tape commands
*/
#define C_TPRETEN	0xa0		/* Tape retention */
#define C_TPREAD	0xa1		/* Read the tape */
#define C_TPWRITE	0xa2		/* Write the tape */
#define C_TPVERIFY	0xa3		/* Verify the Tape */
#define C_TPERASE	0xa4		/* Erase the Tape */
#define C_TPFILEMK	0xa5		/* Write File Mark */
#define C_TPSTATUS	0xa6		/* Report Tape Status */
#define C_TPCONFIG	0xa7		/* Configure Tape drive */
#define C_TPREWIND	0xa8		/* Rewind the Tape */
#define C_TPRDFLMK	0xa9		/* Read File Marks */
#define	C_TPSKBLKS	0xab		/* Seek # Blocks */
		/* Miscellaneous Commands */
#define	C_CMDPASS	0xac		/* Pass a blank command through */
#define C_RUNDIAG	0x70		/* Run Diagnostics */
#define C_RDLONG	0x71		/* Read Long */
#define C_WRLONG	0x72		/* Write Long */
#define C_SECTIME	0x73		/* Read Sector Time */
#define C_RDHDRS	0x74		/* Read Headers */
#define C_RDUIB		0x77		/* Read UIB */

/*
** options for i_options
*/
#define O_IOPB		0x10		/* get iopb with 16 bit bus */
#define O_BUF		0x01		/* do dma with 16 bits */
#define O_OPTIONS	(O_IOPB|O_BUF)

/*
** bits returned in the statuspower down the modem 

*/
#define S_OK		0x80		/* Ok */
#define S_BUSY		0x81		/* Operation in progress, busy */
#define S_ERROR		0x82		/* Error on the last command */

/*
** The Unit Initialization block -- UIB
*/
struct uib {
	u_char	u_spt;			/*  1 *//* Sectors per track */
	u_char	u_hds;			/*  0 *//* Heads */
	u_char	u_bpsh;			/*  3 *//* Bytes per sector high */
	u_char	u_bpsl;			/*  2 *//* Bytes per sector low */
	u_char	u_gap2;			/*  5 *//* bytes in gap 2 */
	u_char	u_gap1;			/*  4 *//* bytes in gap 1 */
	u_char	u_retry;		/*  7 *//* retry count */
	u_char	u_ilv;			/*  6 *//* interleave factor */
	u_char	u_reseek;		/*  9 *//* reseek enable */
	u_char	u_eccon;		/*  8 *//* ecc enable */
	u_char	u_inchd;		/*  b *//* increment head enable */
	u_char	u_mvbad;		/*  a *//* move bad data enable */
	u_char	u_intron;		/*  d *//* interrupt on status change */
	u_char	u_resv0;		/*  c *//* reserved */
	u_char	u_resv1;		/*  f *//* reserved */
	u_char	u_skew;			/*  e *//* spiral skew factor */
	u_char	u_options;		/* 11 *//* operations options byte */
	u_char	u_mohu;			/* 10 *//* motor off and head unload */
	u_char	u_gap3;			/* 13 *//* bytes in gap 3 */
	u_char	u_ddb;			/* 12 *//* drive descriptor byte */
	u_char	u_spw;			/* 15 *//* step pulse width */
	u_char 	u_smc;			/* 14 *//* step and motor control */
	u_char	u_spih;			/* 17 *//* step pulse interval (high) */
	u_char	u_spil;			/* 16 *//* step pulse interval (low( */
	u_char 	u_ttst;			/* 19 *//* track to track seek time */
	u_char	u_hlst;			/* 18 *//* head load or settling time */
	u_char 	u_nch;			/* 1b *//* number of cylinders high */
	u_char 	u_ncl;			/* 1a *//* number of cylinders low */
	u_char	u_wpsch;		/* 1d *//* write precomp start cylh */
	u_char	u_wpscl;		/* 1c *//* write precomp start cyll */
	u_char	u_rwcsch;		/* 1f *//* reduced wr cur start cylh */
	u_char	u_rwcscl;		/* 1e *//* reduced wr cur start cyll */
};
typedef struct uib uib_t;

/*
** uib motor and step control
*/
#define BUFFSTEP	0x20		/* Buffered Step Pulses */
#define STEPCONTROL	0x10		/* Floppy */
#define U_SMC		BUFFSTEP

/*
** uib options
*/
#define CACHEENABLE	0x01
#define ZEROLATENCY	0x02
#define UPDATEIOPB	0x04
#define CBREQENABLE	0x40
#define OVRD		0x80

#define	U_OPTIONS	(CACHEENABLE | ZEROLATENCY | CBREQENABLE)

/*
** Macros for the high -- mid -- low bytes of longs
*/
#define LB(x)	((long) (x) & 0xFF)		/* Low Byte */
#define MB(x)	(((long) (x) >> 8) & 0xFF)	/* Mid Byte */
#define HB(x)	(((long) (x) >> 16) & 0x0F)	/* High Byte */

#ifdef KERNEL

#define FS(d)	(minor(d) & 7)
#define UNIT(d) ((minor(d) >> 3) & 3)
#define TYPE(d) ((minor(d) >> 5) & 7)
#define SPL()	spl5()				/* Interrupt Level */

/*
** Miscellaneous constants
*/
#define	MAX_WINNYS	2		/* # of winchesters per controller */
#define	MAX_FLOPPYS	1		/* one floppy */
#define	MAX_IOPBS	2		/* per drive */
#define	BLKSIZE		512
#define BLKSHIFT	9		/* log shift for the sectors */
/* XXX can we make this larger than 8? */
#define ST_DMACOUNT	8		/* Burst length for DAM xfers */
#define	TOTAL_IOPBS	(MAX_WINNYS * MAX_IOPBS)

#define D_WIN		0
#define D_FLP		1

/* sc_flags bits */
#define SC_ALIVE	0x01		/* device is alive and well */
#define	SC_HARDERROR	0x02		/* oops, hard error happened */
#define	SC_OPEN		0x04		/* device is open */
#define	SC_NEEDWAIT	0x08		/* need to wait before start */

/* per drive data base */
struct	softc_disk {
	short	sc_flags;		/* drive state bits */
	short	sc_spc;			/* sectors per cylinder */
	struct 	disk_map sc_fs[NFS];	/* file system map */
	short	sc_cyl;			/* cylinders on drive */
	char	sc_sec;			/* sectors per track */
	char	sc_hd;			/* heads on drive */
	uib_t	sc_uib;			/* per drive uib */
	daddr_t	sc_bn;			/* block # drive is at */
	short	sc_niopbs;		/* # of iopbs being used */
	struct	iobuf sc_tab;		/* work/active queue */
};

/* controller data base */
struct	softc {
	caddr_t	sc_ioaddr;		/* addr of Multibus i/o registers */
	short	sc_flags;		/* controller state bits */
	iopb_t	*sc_nextiopb;		/* next free iopb (or NULL) */
	short	sc_niopbs;		/* total # of iopb's filled in */
	short	sc_cmds;		/* total pending commands */
	iopb_t	sc_iopb[MAX_IOPBS*MAX_WINNYS];	/* iopb's */
	iopb_t	*sc_first;		/* first iopb to do */
	iopb_t	*sc_last;		/* last iopb to do */
	struct	softc_disk sc_disk[MAX_WINNYS];
/*	struct	softc_disk sc_floppy[MAX_FLOPPYS]; NOT YET */
} stdsoftc;

#endif	KERNEL
