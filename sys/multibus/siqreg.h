/*
 *	siqreg.h
 */

/*
 * Multibus I/O Registers
 * -- Byte Swapped --
 */
#define ST_R0	1
#define ST_R1	0
#define ST_R2	3
#define ST_R3	2

#define STR0(sc)	((sc)->sc_ioaddr + ST_R0)
#define STR1(sc)	((sc)->sc_ioaddr + ST_R1)
#define STR2(sc)	((sc)->sc_ioaddr + ST_R2)
#define STR3(sc)	((sc)->sc_ioaddr + ST_R3)

/*
 * Macros for Writing to ST_R0 -- Command/Status Register
 */
#define ST_RESET	0x80
#define ST_ABORT	0x40
#define ST_16BITS	0x20			/* 16 Bit Bus */
#define ST_NOINTERRUPT	0x10
#define ST_CLEAR	2			/* Clear Interrupt */
#define ST_GO		1
#define ST_START	(ST_GO | ST_16BITS)	/* Start Command */

#define CLEAR(sc)	*(STR0(sc)) = ST_CLEAR
#define START(sc)	*(STR0(sc)) = ST_START
#define ABORT(sc)	*(STR0(sc)) = ST_ABORT
#define RESET(sc)	*(STR0(sc)) = ST_RESET

/*
 * Macros for Reading from ST_R0 -- Command/Status Register
 */
#define STATUSREG(sc)	*(STR0(sc))
#define CSR_ERROR		8	/* Error (assuming CSR_DONE = 1) */
#define CSR_STATUSCHANGE	4	/* Change in drive ready status */
#define CSR_DONE		2	/* Operation is Complete */
#define CSR_BUSY		1	/* Controller is busy */

/*
 * Macros for Reading from ST_R1 & ST_R2
 */
#define RDINTR(sc)	*(STR1(sc))
#define TAPESTATUS(sc)	*(STR2(sc))		/* Tape units ready? */

#define TAPEUNITOFFSET	4
	/* for putting tape unit# into ST_R1 format */
#define R1FMT(unit)	((unit) + TAPEUNITOFFSET)
	/* to get tape unit# from ST_R1 */
#define TAPEUNITVAL(r1)	(((r1) & 0x7) - TAPEUNITOFFSET)
/*
 * The IOPB -- I/O Parameter Block
 */
struct iopb {
	u_char	i_option;		/* 1  -- command option */
	u_char 	i_cmd;			/* 0  -- command */
	u_char	i_error;		/* 3  -- error code */
	u_char 	i_status;		/* 2  -- command status */
	u_char	i_tpoption;		/* 5  -- special tape options */
	u_char	i_unit;			/* 4  -- unit select */
	u_char	i_filecountl;		/* 7  -- file count lo */
	u_char	i_filecounth;		/* 6  -- file count hi */
	u_char	i_sectorl;		/* 9  -- disk op only */
	u_char	i_sectorh;		/* 8  -- disk op only */
	u_char	i_blkcntl; 		/* b  -- block count lo */
	u_char	i_blkcnth;		/* a  -- block count hi */
	u_char	i_bufh;			/* d  -- buffer address high */
	u_char	i_dmaburst;		/* c  -- max #bytes per DMA burst */
	u_char	i_bufl;			/* f  -- buffer address low */
	u_char	i_bufm;			/* e  -- buffer address mid */
	u_char	i_iol;			/* 11  -- I/O Address low */
	u_char	i_ioh;			/* 10  -- I/O Address high */
	u_char	i_rell;			/* 13  -- relative address low */
	u_char	i_relh;			/* 12  -- relative address high */
	u_char	i_linkh;		/* 15  -- linked iopb address high */
	u_char	i_tpcopy;		/* 14  -- tape unit (copy only) */
	u_char	i_linkl;		/* 17  -- linked iopb address low */
	u_char	i_linkm;		/* 16  -- linked iopb address mid */
};
typedef struct iopb iopb_t;

/* variations in the iopb for the configure command */
#define i_pbyte1	i_filecountl
#define i_pbyte0	i_filecounth
#define i_pbyte3	i_sectorl
#define i_pbyte2	i_sectorh
#define i_pbyte5	i_blkcntl
#define i_pbyte4	i_blkcnth
#define i_timeout	i_dmaburst	/* timeout value for tape ops */

/* value for i_timeout */
#define O_TIMEOUT	60		/* 60 * (10-sec) = 10 minutes */

/*
 * The Tape commands
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
#define C_TPFSR		0xAB		/* Seek # Blocks */

/* for siqcmd() */
#define WAIT 1
#define NOWAIT 0

/*
 * i_option
 */
#define O_IOPB		0x10		/* get iopb with 16 bit bus */
#define O_BUF		0x01		/* do dma with 16 bits */
#define O_OPTIONS	(O_IOPB|O_BUF)

/*
 * i_tpoption
 */
	/* for C_TPCONFIG command */
#define OC_CHST		0x01		/* status change int enable */
#define OC_UP_IOPB	0x04		/* update IOPB enable */
#define OC_TPOPTIONS	(OC_CHST|OC_UP_IOPB)

	/* for C_TPWRITE command */
#define OW_FMARK	0x01		/* write filemark if bit 0 is on */

	/*for C_TPFSR command */
#define OR_FOR_REV	0x02		/* reverse if bit 1 is on */

/*
 * i_dmaburst
 */
#define O_DMABURST	32

/*
 * values returned in i_status
 */
#define S_OK		0x80		/* Ok */
#define S_BUSY		0x81		/* Operation in progress, busy */
#define S_ERROR		0x82		/* Error on the last command */

/*
 * values returned in i_error
 */
#define ERR_FILEMARK		0x02
#define ERR_NOTSELECTED		0x80
#define ERR_TAPENOTREADY	0x81
#define ERR_NOTONLINE		0x82
#define ERR_NOCARTRIDGE		0x83
#define ERR_PREMATURE_BOT	0x84
#define ERR_PREMATURE_EOT	0x85
#define ERR_PREMATURE_EOF	0x86
#define ERR_DATAERROR		0x87
#define ERR_NODATA		0x89
#define ERR_WRITEPROTECTED	0x8A
#define ERR_ILLEGALCMD		0x8B
#define ERR_STATUSTIMEOUT	0x8D
#define ERR_EXCEPTION		0x90
#define ERR_READYTIMEOUT	0x92
#define ERR_INVALID_BLKCNT	0x94

/*
 * Status bytes returned by the C_TPSTATUS command
 */
	/* BYTE 0 defines */
#define STATUS_NOCARTRIDGE	0x40
#define STATUS_NOTONLINE	0x20
#define STATUS_WRITEPROTECTED	0x10
#define STATUS_ENDOFTAPE	0x08
#define STATUS_DATAERROR	0x04
#define STATUS_BOTNOTFOUND	0x02
#define STATUS_FILEFOUND	0x01
	/* BYTE 1 defines */
#define STATUS_ILLEGALCMD	0x40
#define STATUS_NODATAFOUND	0x20
#define STATUS_MAXRETRIES	0x10
#define STATUS_BOT		0x08
#define STATUS_RESVERED0	0x04
#define STATUS_RESVERED1	0x02
#define STATUS_RESETOCCURRED	0x01

/*
 * Macros for the high -- mid -- low bytes of longs
 */
#define LB(x)	((long) (x) & 0xFF)		/* Low Byte */
#define MB(x)	(((long) (x) >> 8) & 0xFF)	/* Mid Byte */
#define HB(x)	(((long) (x) >> 16) & 0x0F)	/* High Byte */

/* minor device extraction macros */
#define FLAGS(d) 	(minor(d) & 0x0f)	/* bits 0-3 */
#define UNIT(d) 	((minor(d) >> 4) & 3)	/* bits 4-5 */
#define CTLR(d)		((minor(d) >> 6) & 3)	/* bits 6-7 */

/* flag's in minor bits */
#define FLAGS_REWIND	0x00			/* auto-rewind device */
#define FLAGS_NOREWIND	0x01			/* no rewind device */

/*
** Miscellaneous constants
*/
#define NULL_BP		((struct buf *) 0)

/*
 * Time constants  -  units are seconds
 */
#define TIME_FSF	(60 * 17)		/* no more than 17 minutes */
#define TIME_REWIND	(60 * 2)		/* no more than 2 minutes */
#define TIME_RDWR	(90)			/* 1.5 minutes */
#define TIME_RESET	(TIME_REWIND * 3)
#define TIME_WAIT	(60)			/* default */

/* software controller state for the tape */
struct	softc_tape {
	u_int	st_flags;		/* software state per tape unit */
	u_char	st_unit;		/* unit # */
	resource st_tape_res;		/* tape unit is a lockable resource */
	daddr_t	st_fileno;		/* current tape file # */
	int	st_resid;		/* byte cnt not done in prev I/O */
	char	st_name[8];		/* user friendly name for errors */
	struct	softc *st_ctlr;		/* back pointer to controller */
	u_char	st_status[6];		/* status bytes */
};

/* st_flags bits */
#define ST_ALIVE	0x001		/* drive is alive */
#define ST_OPEN		0x002		/* unit has been opened */
#define ST_ENDOFFILE	0x004		/* drive is at EOF */
#define ST_WRITTEN	0x008		/* tape has been written */
#define ST_EOT		0x010		/* EOT encountered */

/* controller data base */
struct	softc {
	u_char	sc_ctlr;		/* controller # */
	u_int	sc_flags;		/* state bits per controller */
	caddr_t	sc_ioaddr;		/* addr of Multibus i/o registers */
	iopb_t	sc_iopb;		/* one iopb */
	resource sc_ctlr_res;		/* ctlr iopb is a lockable resource */
	u_long	sc_iopb_mbva; 		/* multibus address of iopb */
	struct	softc_tape sc_tape[NSQ];
	struct	buf *sc_buf;		/* for I/O and C_TPSTATUS */
} siqsoftc[NSIQ];

/* sc_flags bits */
#define SC_ALIVE	0x01		/* controller probed */
