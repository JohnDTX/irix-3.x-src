/*
** streg.h
**
** $Source: /d2/3.7/src/stand/cmd/sifex/RCS/streg.h,v $
** $Date: 89/03/27 17:13:25 $
** $Revision: 1.1 $
**
** Header file for the Interphase storager controller from Interphase
*/

#define VTOP(x)		(x)
#define	SWAPW(x) 	(((u_long)x<<16)|((u_long)x>>16))

/*
** Multibus I/O Registers
*/
#define ST_R0	0x1F9
#define ST_R1	0x1F8
#define ST_R2	0x1FB
#define ST_R3	0x1FA
#define ST_R4	0x1FD
#define ST_R5	0x1FC
#define ST_R6	0x1FF
#define ST_R7	0x1FE

/*
** Codes for Writing to ST_R0
*/
#define ST_START	0x01
#define ST_CLEAR	0x02
#define ST_NOINT	0x10			/* No Interrupts */
#define ST_16BITS	0x20			/* 16 Bit Bus */
#define ST_ABORT	0x40
#define ST_RESET	0x80

#define	CLEAR()		(*((char *)(st_ioaddr+ST_R0)) = ST_CLEAR)
#define	ZERO()		(*((char *)(st_ioaddr+ST_R0)) = zero)
#define	START()		(*((char *)(st_ioaddr+ST_R0)) = (ST_START|ST_NOINT))
#define	RESET()		(*((char *)(st_ioaddr+ST_R0)) = (ST_RESET|ST_NOINT))
#define	ABORT()		(*((char *)(st_ioaddr+ST_R0)) = (ST_ABORT|ST_NOINT))
#define STATUS()	(i = (*((char *)(st_ioaddr+ST_R0))))
#define RDINT()		(i = (*((char *)(st_ioaddr+ST_R1))))

#define	TCLEAR()	(*((char *)(st_ioaddr+ST_R4)) = ST_CLEAR)
#define TZERO()		(*((char *)(st_ioaddr+ST_R4)) = zero)
#define	TSTART()	(*((char *)(st_ioaddr+ST_R4)) = ST_START|ST_NOINT)
#define	TRESET()	(*((char *)(st_ioaddr+ST_R4)) = ST_RESET)
#define	TABORT()	(*((char *)(st_ioaddr+ST_R4)) = ST_ABORT)
#define TSTATUS()	(i = (*((char *)(st_ioaddr+ST_R4))))
#define TRDINT()	(i = (*((char *)(st_ioaddr+ST_R5))))
#define TAPERDY()	(i = (*((char *)(st_ioaddr+ST_R6))))

/*
** Codes for Reading from ST_R0
*/
#define ST_BUSY		1	/* Operation is in Progress */
#define ST_DONE		2	/* Operation is Complete */
#define ST_ERROR 	4	/* Operation ended in error */

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
	u_char	i_tapeunit;		/* 14   -- tape unit (copy only) */
	u_char	i_linkl;		/* 17	-- linked iopb address low */
	u_char	i_linkm;		/* 16	-- linked iopb address mid */
};
typedef struct iopb iopb_t;
/* 6 and 7 are also file mark counts */
#define	i_pbyte1	i_cyll		/* 7	-- parameter block 1 */
#define	i_pbyte0	i_cylh		/* 6	-- parameter block 0 */
#define	i_pbyte3	i_secl		/* 9	-- parameter block 3 */
#define	i_pbyte2	i_sech		/* 8	-- parameter block 2 */
#define	i_pbyte5	i_sccl		/* b	-- parameter block 5 */
#define	i_pbyte4	i_scch		/* a	-- parameter block 4 */
					/* d    -- bufh */
#define	i_timeout	i_dmacount	/* c	-- tape timeout */
#define i_filel		i_cyll		/* 7 */
#define i_fileh		i_cylh		/* 6 */
#define i_tpoption	i_head		/* 5 */

/*
** Defines of the Option in the iopb
*/
#define OP_BUS16	0x01
#define	OP_RELBUF	0x02
#define OP_IOPB16	0x10
#define OP_OPTIONS	(OP_BUS16|OP_IOPB16)

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
#define C_DSKTOTAPE 	0x88		/* disk to tape */
#define C_RESTORE	0x89		/* Restore a drive */
#define C_SEEK		0x8a		/* Seek */
#define C_REFORMAT	0x8b		/* REFormat */
#define C_SPINUP	0x8E		/* Spin up */
#define C_RESET		0x8f		/* Reset the Controller */
#define C_READABSOLUTE  0x93		/* Read Absolute */
#define C_READNOCACHE	0x94		/* Read with no caching */
		/* Tape Commands */
#define C_TPRETEN	0xa0		/* Tape retention */
#define C_TPREAD	0xa1		/* Read the tape */
#define C_TPWRITE	0xa2		/* Write the tape */
#define C_TPVERIFY	0xa3		/* Verify the Tape */
#define C_TPERASE	0xa4		/* Erase the Tape */
#define C_TPFILEMK	0xa5		/* Write File Mark */
#define C_TPSTATUS	0xa6		/* Report Tape Status */
#define C_TPCONFIG	0xa7		/* Configure Tape drive */
#define C_TPREWIND	0xa9		/* Rewind the Tape */
#define C_TPRDFLMK	0xaa		/* Read File Marks */
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

/*
** bits returned in the status
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

/* Bit definitions for the uib */
#define DOUBLE_DENSITY	4		/* for u_ddb */
#define ST506		2		/* for u_ddb */
#define CACHE_ENABLE	1		/* for u_options */

/*
** Miscellaneous variables for the disk
*/
#define ST_RETRY	3		/* Retry count */
#define ST_DMACOUNT	0		/* Burst length for DMA xfers */
#define ST_BUFSTEP	0x20		/* use buffered step pulses */

/* Macros for the high -- mid -- low bytes of longs */
#define LB(x)	((long) (x) & 0xFF)		/* Low Byte */
#define MB(x)	(((long) (x) >> 8) & 0xFF)	/* Mid Byte */
#define HB(x)	(((long) (x) >> 16) & 0x0F)	/* High Byte */
