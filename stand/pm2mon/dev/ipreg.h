/*
** $Source: /d2/3.7/src/stand/pm2mon/dev/RCS/ipreg.h,v $
** $Date: 89/03/27 17:16:53 $
** $Revision: 1.1 $
*/

/*
** ipreg.h
**		- Chase - December 1983
** 		- Header file for the Interphase 2190 controller
*/

typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef	unsigned long ULONG;

#define	SWAPW(x) (((ULONG)x<<16)|((ULONG)x>>16))

/*
 * Multibus I/O Registers
 */
#define IP_R1	0
#define IP_R0	1	/* For the SUN CPU they must be byte swapped */
#define IP_R3	2
#define IP_R2	3
/*
 * Codes for Writing to IP_R0
 */
#define IP_16BITS	0x20			/* 16 Bit Bus */
#define IP_GO		(1 | IP_16BITS)		/* Start the Controller */
#define IP_CLEAR	(2 | IP_16BITS)		/* Clear the interrupt */
#define IP_RESET	(0x80)
#define IP_IOADDR	((char *)mbiotov(0x7010))

#define	RESET(rp)	(rp[IP_R0] = IP_RESET)

/*
 * Codes for Reading from IP_R0
 */
#define IP_BUSY	1	/* Operation is in Progress */
#define IP_DONE	2	/* Operation is Complete */

/*
 * The IOPB -- I/O Parameter Block
 */
struct iopb {
	UCHAR	i_option;		/* 1	-- command option */
	UCHAR 	i_cmd;			/* 0	-- command */
	UCHAR	i_error;		/* 3	-- error code */
	UCHAR 	i_status;		/* 2	-- command status */
	UCHAR	i_head;			/* 5	-- head select */
	UCHAR	i_unit;			/* 4	-- unit select */
	UCHAR	i_cyll;			/* 7	-- cylinder select lsb */
	UCHAR	i_cylh;			/* 6	-- cylinder select msb */
	UCHAR	i_secl;			/* 9	-- sector select lsb */
	UCHAR	i_sech;			/* 8	-- sector select msb */
	UCHAR	i_sccl;			/* b	-- sector count lsb */
	UCHAR	i_scch;			/* a	-- sector count msb */
	UCHAR	i_bufh;			/* d	-- buffer address high */
	UCHAR	i_dmacount;		/* c	-- dma count */
	UCHAR	i_bufl;			/* f	-- buffer address low */
	UCHAR	i_bufm;			/* e	-- buffer address mid */
	UCHAR	i_iol;			/* 11	-- I/O Address low */
	UCHAR	i_ioh;			/* 10	-- I/O Address high */
	UCHAR	i_rell;			/* 13	-- relative address low */
	UCHAR	i_relh;			/* 12	-- relative address high */
	UCHAR	i_linkh;		/* 15	-- linked iopb address high */
	UCHAR	i_reserved;		/* 14   -- reserved */
	UCHAR	i_linkl;		/* 17	-- linked iopb address low */
	UCHAR	i_linkm;		/* 16	-- linked iopb address mid */
};
typedef struct iopb iopb_t;

/*
 * commands used in i_cmd
 */
#define C_READ		0x81		/* read data */
/* #define C_WRITE		0x82		/* write data */
/* #define C_VERIFY	0x83		/* Verify data */
/* #define C_FORMAT	0x84		/* Format a track */
/* #define C_MAP		0x85		/* Map a track */
/* #define C_REPORT	0x86		/* Report The Controller info */
#define C_INIT		0x87		/* Initialize a drive */
/* #define C_RESTORE	0x89		/* Restore a drive */
/* #define C_SEEK		0x8a		/* Seek */
/* #define C_ZERO		0x8b		/* Zero a sector */
/* #define C_RESET		0x8f		/* Reset the Controller */
/* #define C_READDIR	0x91		/* Read Direct */
/* #define C_WRITEDIR	0x92		/* Write Direct */
/* #define C_READABSOLUTE  0x93		/* Read Absolute */
/* #define C_READNOCACHE	0x94		/* Read with no caching */

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
 * The Unit Initialization block -- UIB
 */

struct uib {
	UCHAR	u_spt;			/*  1 *//* Sectors per track */
	UCHAR	u_hds;			/*  0 *//* Heads */
	UCHAR	u_bpsh;			/*  3 *//* Bytes per sector high */
	UCHAR	u_bpsl;			/*  2 *//* Bytes per sector high */
	UCHAR	u_gap2;			/*  5 *//* bytes in gap 2 */
	UCHAR	u_gap1;			/*  4 *//* bytes in gap 1 */
	UCHAR	u_retry;		/*  7 *//* retry count */
	UCHAR	u_ilv;			/*  6 *//* interleave factor */
	UCHAR	u_reseek;		/*  9 *//* reseek enable */
	UCHAR	u_eccon;		/*  8 *//* ecc enable */
	UCHAR	u_inchd;		/*  b *//* increment head enable */
	UCHAR	u_mvbad;		/*  a *//* move bad data enable */
	UCHAR	u_intron;		/*  d *//* interrupt on status change */
	UCHAR	u_dualp;		/*  c *//* dual port drive */
	UCHAR	u_group;		/*  f *//* Group size for cache */
	UCHAR	u_skew;			/*  e *//* spiral skew factor */
	UCHAR	u_resv2;		/* 11 *//* reserved */
	UCHAR   u_resv1;		/* 10 *//* reserved */
	UCHAR	u_resv3;		/* 12 *//* reserved */
};
typedef struct uib uib_t;

/* Miscellaneous variables for the disk */

#define IP_SIZE	512		/* Size for the sectors */
#define IP_LOG		9		/* log shift for the sectors */
#define IP_ILV		1		/* Interleave factor */
#define IP_RETRY	3		/* Retry count */
#define IP_DMACOUNT	0		/* Burst length for DAM xfers */

/* Macros for the high -- mid -- low bytes of longs */

#define LB(x)	((long) (x) & 0xFF)		/* Low Byte */
#define MB(x)	(((long) (x) >> 8) & 0xFF)	/* Mid Byte */
#define HB(x)	(((long) (x) >> 16) & 0x0F)	/* High Byte */

#define F2351A
#undef F2312

