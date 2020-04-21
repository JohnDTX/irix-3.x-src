/*
**	siireg.h	- Copyright (C), SGI INC.
**			- Author: chase
**			- Date: July 1985
**			- Any use, copying or alteration is prohibited
**			- and morally inexcusable,
**			- unless authorized in writing by SGI.
**	$Source: /d2/3.7/src/sys/multibus/RCS/siireg.h,v $
**	$Author: root $
**	$Revision: 1.1 $
**	$Date: 89/03/27 17:31:34 $
*/

#define ICHECK		2
#define IFORCE		4
#define SWAPB(x)	(u_short)(((x>>8)&0xFF)|((x<<8)&0xFF00))

#define NUMBERIOPBS	14

#define B_TOBEDONE	0x7F
/*
** IOPB ADDRESS
*/
#define SC_IOPBBASE	(siisoftc.sc_cioaddr)

/* set register which tells board # of iopbs */
#define	SETNUMIOPBS(x) \
	*(short *)(siisoftc.sc_cioaddr+0x1E)=(((x)<<8)|(NUMBERIOPBS&0xFF));

/*
** Bits in the control word
*/
#define SC_ENINTERRUPT	0x01
#define SC_INTBON	0x02
#define SC_ENIOPB	0x80
/* The tape will only use interrupt B and the disk will use interrupt A */
#define SC_ENABLE	(SC_ENINTERRUPT | SC_ENIOPB | SC_INTBON)
/*
** Control Word Address Enable
*/
#define SC_ENABLECW(x)	*(short *)(siisoftc.sc_cioaddr+(x<<1))=(SC_ENABLE)
#define SC_CLEARCW(x)	*(short *)(siisoftc.sc_cioaddr+(x<<1))=(0x0001)

/*
** Multibus I/O Registers
** -- Byte Swapped --
*/
#define ST_R0	0x1F9
#define ST_R1	0x1F8
#define ST_R2	0x1FB
#define ST_R3	0x1FA
#define ST_R4	0x1FD
#define ST_R5	0x1FC
#define ST_R6	0x1FF
#define ST_R7	0x1FE

#define	STR0	(siisoftc.sc_cioaddr+ST_R0)
#define	STR1	(siisoftc.sc_cioaddr+ST_R1)
#define	STR2	(siisoftc.sc_cioaddr+ST_R2)
#define	STR3	(siisoftc.sc_cioaddr+ST_R3)
#define	STR4	(siisoftc.sc_cioaddr+ST_R4)
#define	STR5	(siisoftc.sc_cioaddr+ST_R5)
#define	STR6	(siisoftc.sc_cioaddr+ST_R6)
#define	STR7	(siisoftc.sc_cioaddr+ST_R7)

/*
** Macros for Writing to ST_R0 -- Command/Status Register
*/
#define ST_START	0x01
#define ST_CLEAR	0x02
#define ST_NOINTERRUPT	0x10
#define ST_16BITS	0x20
#define ST_ABORT	0x40
#define	ST_RESET	0x80

#define CLEAR()		*STR0 = ST_CLEAR
#define ZEROREG0()	*STR0 = 0
#define CLEARREG4()	*STR4 = ST_CLEAR
#define STARTWO()	*STR0 = (ST_START | ST_NOINTERRUPT)
#define START()		*STR0 = ST_START
#define SIIABORT()	*STR0 = ST_ABORT
#define RESET()		*STR0 = ST_RESET

/*
** Macros for Reading from ST_R0 -- Command/Status Register
*/
#define ST_BUSY			0x01
#define ST_DONE			0x02
#define ST_IOPBSHIFT		0x03
#define ST_STCHANGE		0x04
#define ST_INTERRUPTMASK	0x07
#define ST_ERROR		0x08
#define ST_IOPBMASK		0x78
#define ST_QUEUEMODE		0x80

#define STATUSREG()	*STR0
#define RDINTR()	*STR1
#define	TAPESTATUS()	*STR2

/*
** Floppy stuff
*/
#define FLP_OFFSET	2

/*
** The IOPB -- I/O Parameter Block
** Have to byte swap the bytes in the SHORT--SHIT!!!!!!!
** When in queued mode the iopb is in I/O space and byte swapped BACK.
** Clean up by having an iopb with shorts and with chars.
*/
struct iopb {
	u_short	option_cmd;		/* 0-1	 */
	u_short	err_stat;		/* 2-3	 */
	u_short head_unit;		/* 4-5	 */
	u_short	cyl;			/* 6-7	 */
	u_short	sec;			/* 8-9	 */
	u_short	scc;			/* 10-11 */
	u_short	bufh_dma;		/* 12-13 */
	u_short	bufl;			/* 14-15 */
	union {
		struct {
			u_short	ioaddr;			/* 16-17 */
			u_short	relative;		/* 18-19 */
		} iopb_s;
		struct	buf *bp;
	} iopb_u;
};
typedef struct iopb iopb_t;

/* definitions for easy use of union */
#define	iopb_bp		iopb_u.bp
#define	iopb_ioaddr	iopb_u.iopb_s.ioaddr
#define	iopb_relative	iopb_u.iopb_s.relative

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
** Miscellaneous Commands
*/
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
/* updateiopb makes the controller a little slower */
#define	U_OPTIONS  (CACHEENABLE | ZEROLATENCY | CBREQENABLE)

/*
** Macros for the high -- mid -- low bytes of longs
*/
#define LB(x)	((long) (x) & 0xFF)		/* Low Byte */
#define MB(x)	(((long) (x) >> 8) & 0xFF)	/* Mid Byte */
#define HB(x)	(((long) (x) >> 16) & 0x0F)	/* High Byte */

#ifdef KERNEL

#define TYPE(d) ((minor(d) >> 5) & 7)
#define SPL()	spl5()				/* Interrupt Level */

/*
** Miscellaneous constants
*/
#define	MAX_WINNYS	2		/* # of winchesters per controller */
#define	MAX_FLOPPYS	1		/* one floppy */
#define	BLKSIZE		512
#define BLKSHIFT	9		/* log shift for the sectors */
#define ST_DMACOUNT	0		/* Burst length for DAM xfers */
#define	STRETRIES	2		/* # of retries before ecc */

#define D_WIN		0
#define D_FLP		1

/* sc_flags bits */
#define SC_ALIVE	0x01
#define	SC_HARDERROR	0x02
#define	SC_OPEN		0x04
#define SC_WANTED	0x08

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
};

/* controller data base */
struct	softc {
	short	sc_flags;			/* controller state bits */
	short 	*sc_sioaddr;			/* Base I/O Address */
	caddr_t	sc_cioaddr;			/* Base I/O Address */
	iopb_t	sc_iopb;			/* Default IOPB */
	struct	iobuf sc_tab;			/* work/active queue */
	struct	softc_disk sc_disk[MAX_WINNYS];
	struct	softc_disk sc_floppy[MAX_FLOPPYS];
} siisoftc;

#endif 			/* KERNEL */
