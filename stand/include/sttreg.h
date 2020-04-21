/*
**	sttreg.h	- Copyright (C), JCS Computer Services 1983
**			- Author: chase
**			- Date: December 1984
**			- Any use, copying or alteration is prohibited
**			- and morally inexcusable,
**			- unless authorized in writing by JCS.
**	$Source: /d2/3.7/src/stand/include/RCS/sttreg.h,v $
**	$Revision: 1.1 $
**	$Date: 89/03/27 17:13:54 $
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

#define	STR0	(sttsoftc.sc_ioaddr+ST_R0)
#define	STR1	(sttsoftc.sc_ioaddr+ST_R1)
#define	STR2	(sttsoftc.sc_ioaddr+ST_R2)
#define	STR3	(sttsoftc.sc_ioaddr+ST_R3)
#define	STR4	(sttsoftc.sc_ioaddr+ST_R4)
#define	STR5	(sttsoftc.sc_ioaddr+ST_R5)
#define	STR6	(sttsoftc.sc_ioaddr+ST_R6)
#define	STR7	(sttsoftc.sc_ioaddr+ST_R7)

/*
** Macros for Writing to ST_R0 -- Command/Status Register
*/
#define	ST_RESET	0x80
#define ST_ABORT	0x40
#define ST_16BITS	0x20			/* 16 Bit Bus */
#define ST_NOINTERRUPT	0x10
#define ST_START	(1)			/* Start the Controller */
#define ST_CLEAR	(2)			/* Clear the Controller */

#define CLEAR()		*STR0 = ST_CLEAR
#define START()		*STR0 = (ST_START | ST_16BITS)
#define STARTWO()	*STR0 = (ST_START | ST_NOINTERRUPT)
#define ABORT()		*STR0 = ST_ABORT
#define RESET()		*STR0 = ST_RESET

/*
** Macros for Reading from ST_R0 -- Command/Status Register
*/
#define ST_BUSY		1		/* Operation is in Progress */
#define ST_DONE		2		/* Operation is Complete */
#define ST_STATUSCHANGE 4		/* Status Change interrupt */

#define STATUSREG()	*STR0
#define RDINTR()	*STR1
#define TPREADY()	*STR2		/* Tape units ready? */
#define TAPEUNITOFFSET	4		/* Offset for the Storager */
#define TP_UNIT0	4		/* Tape drive unit 0 for iopb */
#define TP_UNIT1	5		/* Tape drive unit 1 for iopb */
#define TP_UNIT2	6		/* Tape drive unit 2 for iopb */
#define TP_UNIT3	7		/* Tape drive unit 3 for iopb */

/*
** The IOPB -- I/O Parameter Block
*/
struct iopb {
	u_char	i_option;		/* 1	-- command option */
	u_char 	i_cmd;			/* 0	-- command */
	u_char	i_error;		/* 3	-- error code */
	u_char 	i_status;		/* 2	-- command status */
	u_char	i_tpoption;		/* 5	-- special tape options */
	u_char	i_unit;			/* 4	-- unit select */
	/* 6 and 7 are also file mark counts */
	u_char	i_pbyte1;		/* 7	-- parameter block 1 */
	u_char	i_pbyte0;		/* 6	-- parameter block 0 */
	u_char	i_pbyte3;		/* 9	-- parameter block 3 */
	u_char	i_pbyte2;		/* 8	-- parameter block 2 */
	u_char	i_pbyte5;		/* b	-- parameter block 5 */
	u_char	i_pbyte4;		/* a	-- parameter block 4 */
	u_char	i_bufh;			/* d	-- buffer address high */
	u_char	i_timeout;		/* c	-- tape timeout */
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
};
typedef struct iopb iopb_t;

#define i_filecounth	i_pbyte0
#define i_filecountl	i_pbyte1
#define i_scch 		i_pbyte4
#define i_sccl 		i_pbyte5
#define i_dmacount 	i_timeout
#define DMACOUNT	8
/*
** The Tape commands
*/
#define C_TPRETENSION	0xA0		/* Tape retention */
#define C_TPREAD	0xA1		/* Read the tape */
#define C_TPWRITE	0xA2		/* Write the tape */
#define C_TPVERIFY	0xA3		/* Verify the Tape */
#define C_TPERASE	0xA4		/* Erase the Tape */
#define C_TPWEOF	0xA5		/* Write File Mark */
#define C_TPSTATUS	0xA6		/* Report Tape Status */
#define C_TPCONFIG	0xA7		/* Configure Tape drive */
#define C_TPREWIND	0xA9		/* Rewind the Tape */
#define C_TPFSF		0xAA		/* Read File Marks */
#define	C_TPFSR		0xAB		/* Seek # Blocks */

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
** Macros for the high -- mid -- low bytes of longs
*/
#define LB(x)	((long) (x) & 0xFF)		/* Low Byte */
#define MB(x)	(((long) (x) >> 8) & 0xFF)	/* Mid Byte */
#define HB(x)	(((long) (x) >> 16) & 0x0F)	/* High Byte */

#define UNIT(d) 	((minor(d) >> 4) & 3)
#define DEVICE(d) 	(minor(d) & 0x0f)
#define SPL()		spl5()				/* Interrupt Level */
/* device types */
#define NORMAL		0x00
#define NOREWIND	0x01

/* BYTE 0 defines */
#define NOCARTRIDGE	0x40
#define NOTONLINE	0x20
#define WRITEPROTECTED	0x10
#define ENDOFTAPE	0x08
#define DATAERROR	0x04
#define BOTNOTFOUND	0x02
#define FILEFOUND	0x01
/* BYTE 1 defines */
#define	ILLEGALCMD	0x40
#define	NODATAFOUND	0x20
#define	MAXRETRIES	0x10
#define	BOT		0x08
#define	RESVERED0	0x04
#define	RESVERED1	0x02
#define	RESETOCCURRED	0x01
/*
** Miscellaneous constants
*/
#define	MAX_TAPES	4		/* # of winchesters per controller */
#define	BLKSIZE		512
#define BLKSHIFT	9		/* log shift for the sectors */
#define ST_DMACOUNT	8		/* Burst length for DAM xfers */
#define	STRETRIES	2		/* # of retries before ecc */

#define D_WIN		0
#define D_FLP		1
#define D_TAPE		2

/* software controller state for the tape */
struct	softc_tape {
	short	sc_flags;		/* state of tape (see below) */
	u_char	sc_status0;		/* 6 status bytes */
	u_char	sc_status1;
	u_char	sc_status2;
	u_char	sc_status3;
	u_char	sc_status4;
	u_char	sc_status5;
	u_char	sc_unit;		/* unit # */
	daddr_t	sc_blkno;		/* current tape block # */
	daddr_t	sc_fileno;		/* current tape file # */
	daddr_t	sc_nxrec;		/* next tape block # */
};

/* sc_flags bits */
#define SC_ALIVE	0x01
#define	SC_HARDERROR	0x02
#define	SC_ENDOFFILE	0x04
#define SC_INITED	0x08
#define SC_WANTED	0x10
#define SC_TAPEINUSE	0x20
#define SC_WRITTEN	0x40

/* controller data base */
struct	softc {
	u_short	sc_flags;		/* controller state bits */
	caddr_t	sc_ioaddr;		/* addr of Multibus i/o registers */
	iopb_t	*sc_iopb;		/* one iopb */
	struct	softc_tape sc_tape[4];
	struct	iobuf sc_tab;		/* work/active queue */
	struct	buf sc_tapebuf;		/* raw buffer header */
} sttsoftc;
