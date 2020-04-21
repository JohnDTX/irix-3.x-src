/*
 * tmtreg.h	- Copyright (C) Silicon Graphics - Mountain View, CA 94043
 */

#define SPL()	spl5()

#define LB(x)	((long)(x) & 0xff)		/* Low Byte */
#define MB(x)	(((long)(x) >> 8) & 0xff)	/* Mid Byte */
#define HB(x)	(((long)(x) >> 16) & 0x0f)	/* High Byte */

#define SWAPB(x) ((u_short) ((((x) << 8) & 0xFF00) | (((x) >> 8) & 0xFF)))
#define SWAP(x) ((long)(((SWAPB((long)(x)))<<16)|(SWAPB(((long)(x))>>16))))

/*
 * I/O space registers - byte accesses only
 * -- Byte Swapped on our 68K --
 */
#define TMT_R0		1		/* CSR (read), Start (write) */
#define TMT_R1		0		/* Error (read) and Reset (write) */
#define TMT_R2		3		/* Tp stat (read) and Clear (write) */
#define TMT_R3		2		/* Not used */
#define TMT_R4		5		/* LSB Parameter Block */
#define TMT_R5		4		/* MSB Parameter Block */
#define TMT_R6		7		/* HSB Parameter Block */
#define TMT_R7		6		/* Not used */

/* Writes to the the I/O Registers */
#define START()		 *(tmtsoftc.sc_ioaddr+TMT_R0) = 1
#define RESET()		 *(tmtsoftc.sc_ioaddr+TMT_R1) = 1
#define CLEAR()		 *(tmtsoftc.sc_ioaddr+TMT_R2) = 1
#define ADDR0(x)	 *(tmtsoftc.sc_ioaddr+TMT_R4) = LB(x)
#define ADDR1(x)	 *(tmtsoftc.sc_ioaddr+TMT_R5) = MB(x)
#define ADDR2(x)	 *(tmtsoftc.sc_ioaddr+TMT_R6) = HB(x)
#define ADDR3(x)	 *(tmtsoftc.sc_ioaddr+TMT_R7) = 0
/* Reads from the I/O Registers */
#define CSTATUS()	 *(tmtsoftc.sc_ioaddr+TMT_R0)
#define GETERROR()	 *(tmtsoftc.sc_ioaddr+TMT_R1)
#define TPSTATUS()	 *(tmtsoftc.sc_ioaddr+TMT_R2)

/*
 * Controller (adaptor) Status Register (TMT_R0)
 */
#define CSR_BUSY	0x01		/* TM II Busy */
#define CSR_ERROR	0x02		/* Error */
#define CSR_RBI		0x04		/* Ring Buffer Interrupt */
#define CSR_CCI		0x08		/* Command Complete Interrupt */

/*
 * Tape Status Register (TMT_R2)
 */
#define TSR_UMASK	0x07		/* Mask for unit selected */
#define TSR_NRDY	0x08		/* 0 = drive ready */
#define TSR_NBOT	0x10		/* 0 = beginning of tape */
#define TSR_NEOT	0x20		/* 0 = end of tape */
#define TSR_NRWD	0x40		/* 0 = drive is rewinding */
#define TSR_NONL	0x80		/* 0 = drive online */

/*
 * Interface Status Block (returned by CMD_STATUS command)
 * (Note that 1st byte is almost the complement of the Tape Status Reg.)
 */
#define DS_RDY		0x0008		/* Drive Ready */
#define DS_BOT		0x0010		/* Beginning Of Tape */
#define DS_EOT		0x0020		/* End Of Tape */
#define DS_RWD		0x0040		/* drive Rewinding */
#define DS_ONL		0x0080		/* drive Online */
#define DS_FBUSY	0x0100		/* Formatter Busy */
#define DS_DBUSY	0x0200		/* Drive Busy */
#define DS_DEN		0x0400		/* 1 = Hi Density */ 
#define DS_SPD		0x0800		/* 1 = Hi Speed */
#define DS_FPT		0x1000		/* Write Protected */
#define DS_ID		0x2000		/* GCR => 1, NRZI => 0, PE => ?
					 * (P2, pin 16 signal)
					 */
#define DS_SGL		0x4000		/* 1 = single gap head, 0 = dual */ 


typedef struct parameter {
	u_char	cmd;		/* 1   Command  */
	u_char	control;	/* 0   Control */
	u_short	count;		/* 2-3 Byte/Record Count */
	u_long	addr;		/* 4-7 Source/Destination Pointer */
	u_long	pblink;		/* 8-b Linked Parameter Block Pointer */
	u_char	gate;		/* d   Gate */
	u_char	error;		/* c   Error Code */
	u_short	transfer;	/* e-f Return Count */
} para_t;

/*
 * cmd - Command Codes
 */
#define CMD_CONFIG	0x00	/* Configure */
#define CMD_GIVECONF	0x01	/* Return Configuration */
#define CMD_RESET	0x10	/* Reset */
#define CMD_SECURE	0x12	/* Security Erase */
#define CMD_ERASE	0x13	/* Fixed Erase */
#define CMD_ONLINE	0x14	/* Online */
#define CMD_OFFLINE	0x15	/* Offline */
#define CMD_VARERASE	0x16	/* Variable Erase */
#define CMD_STATUS	0x17	/* Get Drive Status */
#define CMD_REWIND	0x19	/* Non-overlapped Rewind */
#define CMD_WFM		0x20	/* Write File Mark */
#define CMD_FSF		0x21	/* Space File Mark */
#define CMD_FSR		0x22	/* Space Record(s) */
#define CMD_READ	0x30	/* Read */
#define CMD_RBREAD	0x31	/* Ring Buffer Read */
#define CMD_WRITE	0x32	/* Write */
#define CMD_RBWRITE	0x33	/* Ring Buffer Write */
#define CMD_WUR		0x35	/* Ring Buffer Write Unlimited Record Size */
#define CMD_RUR		0x36	/* Ring Buffer Read Unlimited Record Size */
#define CMD_DIAG1	0x40	/* Diagnostic Test 1 */
#define CMD_DIAG2	0x41	/* Diagnostic Test 2 */
#define CMD_NSC		0x50	/* Non-Standard Command */
#define CMD_NSCP	0x51	/* Non-Standard Command with Parameters */
#define CMD_NSRS	0x52	/* Non-Standard Read Status Command */

/* for tmtcmd() */
#define WAIT 1
#define NOWAIT 0

/*
 * control - Control
 */
#define C_FOR		0x00	/* Forward Direction */
#define C_UNITSEL	0x07	/* Bits for Unit Select */
#define C_REV		0x08	/* Tape Direction */
#define C_INTERRUPT	0x10	/* Interrupt upon command execution */
#define C_LINK		0x20	/* Linked parameter block follows */
#define C_EDIT		0x40	/* Write Edit */

/*
 * gate - Command Gate
 */
#define G_ENTERED	0x01	/* Command Entered */
#define G_COMPLETE	0x02	/* Command Done */
#define G_ERROR		0x04	/* Error in command execution */

/*
 * Adaptor Error Codes
 */
	/* Recoverable errors */
#define ERR_BUS_TO		0x01	/* Bus Timeout */
#define ERR_BOT_REV		0x02	/* Can't Reverse at BOT */
#define ERR_EOT			0x03	/* End Of Tape */
#define ERR_FILEMARK		0x04	/* Filemark detected */
#define ERR_FBUSY		0x05	/* Formatter Busy */
#define ERR_TAPE		0x06	/* Hard Tape Error (uncorrectable) */
#define ERR_SWPADDR		0x07	/* Invalid swap addr (not even) */
#define ERR_SWPWIDTH		0x08	/* Invalid swap bus width (not 16) */
#define ERR_SWPCOUNT		0x09	/* Invalid swap count (not even) */
#define ERR_ILLEGALCMD		0x0A	/* Invalid Command */
#define ERR_COUNT		0x0B	/* Invalid Byte/Record Count */
#define ERR_RETRY_CNT		0x0C	/* Invalid retry count */
#define ERR_THRTL_SIZ		0x0D	/* Invalid Throttle size */
#define ERR_READ_OVFL		0x0E	/* Actual count > requested */
#define ERR_PREV_PB		0x0F	/* Linked pblock error in prev pb */
#define ERR_ILL_ONL		0x10	/* ONLINE command unsupported */
#define ERR_PARITY		0x11	/* Parity error */
#define ERR_RDOVERRUN		0x12	/* Read overrun */
#define ERR_BOT			0x13	/* Reversed onto BOT */
#define ERR_READ_UNFL		0x14	/* Actual count < requested */
#define ERR_SOFTTERR		0x15	/* Soft Tape Error (corrected) */
#define ERR_WRITEPROTECTED	0x16
#define ERR_DRVBUSY		0x17	/* Drive busy (data busy) */
#define ERR_NOTONL		0x18	/* Drive not Online */
#define ERR_TAPENOTREADY	0x19	/* Drive not Ready */
#define ERR_NO_FM		0x1A	/* Filemark not detected */
#define ERR_WRT_UNDERRUN	0x1B	/* Write Underrun */
#define ERR_SIGNAL_PARAM	0x1C	/* Bad signals param in conf block */
#define ERR_BLANK_TAPE		0x1D	/* Blank tape found during read */
#define ERR_NOGO		0x1E	/* error during intitialization */
#define ERR_NSC_LENGTH		0x1F	/* Bad NSC command length */
#define ERR_NSC_BCNT		0x20	/* Bad NSC byte count */
#define ERR_STROBES		0x23	/* Invalid write strobes */

#define MAX_RECOV_ERR		0x24

		/* Unrecoverable Errors */
#define ERR_LOCALMEM		0x80	/* Local memory error */
#define ERR_LATCHEMPTY		0x81	/* Parameter latch empty */
#define ERR_LATCHFULL		0x82	/* Parameter latch full */
#define ERR_PROMCHK		0x83	/* PROM checksum error */
#define ERR_PFIFO		0x84	/* Parameter/FIFO test error */
#define ERR_FIFORAM		0x85	/* FIFO Ram test error */
#define ERR_TAPESIDE		0x86	/* Tape Side test error */
#define ERR_UTEA		0x87	/* Underrun Test Error A */
#define ERR_UTEB		0x88	/* Underrun Test Error B */
#define ERR_OTEA		0x89	/* Overrun Test Error A */
#define ERR_OTEB		0x8A	/* Overrun Test Error B */
#define ERR_TPTEA		0x8B	/* Tape Prescale Test Error A */
#define ERR_TPTEB		0x8C	/* Tape Prescale Test Error B */
#define ERR_TPTEC		0x8D	/* Tape Prescale Test Error C */
#define ERR_BPTEA		0x8E	/* Bus Prescale Test Error A */
#define ERR_BPTEB		0x8F	/* Bus Prescale Test Error B */
#define ERR_BTHE		0x90	/* Bus timeout hdw error */
#define ERR_PLHE		0x91	/* Parameter latch hdw error */
#define ERR_PBPRE		0x92	/* Parameter block ptr reg error */
#define ERR_ERE			0x93	/* Board Error Register failure */
#define ERR_BSRE		0x94	/* Board Status Register failure */
#define ERR_TSRE		0x95	/* Tape Status Register failure */

/*
 * Configuration Block
 */
typedef struct cblock {
	u_char	bus;		/* 1 - Bus Control */
	u_char	tape1;		/* 0 - Tape Control 1 (units 0 & 1)*/
	u_char	signals;	/* 3 - Signals */
	u_char	throttle;	/* 2 - Throttle */
	u_char	retries;	/* 5 - Retry Value */
	u_char	tape2;		/* 4 - Tape Control 2 (units 2 & 3)*/
} cblk_t;
/*
 * Bus Control (for cblock)
 */
#define B_WIDTH8	0x01	/* to select 8-Bit Bus Width */
#define B_TIMETYPE	0x02	/* to select Time Type Throttle */
#define B_BYTESWAP	0x04	/* to select Byte Swap during data transfer */
#define B_INHIBIT	0x08	/* Inhibit advancing of address count */
/*
 * Tape Control (for cblock)
 */
#define T_HIGHSPD	0x01	/* Tape Speed */
#define T_HIGHDEN	0x02	/* Tape Density */
#define T_EXTGAP	0x04	/* Inter-Record Gap Lenth */
#define T_INCLUDECHK	0x08	/* Check Character Enable */
/*
 * Redefinition of some Pertec Interface Signals
 */
#define S_1J16	0x10		/* Use J1, Pin 16 for load & online */
#define S_2J50	0x20		/* Use J1, Pin 50 to select speed */
#define S_DEFAULT (S_1J16 + S_2J50)
/*
 * Other configure block defines
 */
#define CB_THROTTLE	32;	/* Number of bytes on the bus */
#define CB_RETRIES	10;	/* Number of retries */


#ifdef NONSTD_COMMAND
/*
 * NSC, NSCP, NSRS - Non-Standard Command Block
 */
typedef struct nscblock {
	u_char	bcount;		/* 1   Byte Count */
	u_char	clength;	/* 0   Command Length */
	u_long	sdptr;		/*2-5  Source/Destination Block Pointer */
	u_char	cmd2;		/* 7   Command 2 */
	u_char	cmd1;		/* 6   Command 1 */
} nsc_t;

/*
 * cmd1 - Command 1
 */
#define NSC_ERA		0x01	/* Erase */
#define NSC_EDI		0x02	/* Edit */
#define NSC_WFM		0x04	/* Write File Mark */
#define NSC_WRT		0x08	/* Write */
#define NSC_REV		0x10	/* Reverse */
#endif


#ifdef RING_BUFFER
/*
 * RBREAD, RBWRITE, RUR, WUR - Ring Buffer Record
 */
typedef struct rbblock {
	u_char	gate;		/* 0   Gate */
	u_char	reserved;	/* 1   Reserved */
	u_short	bcount;		/*2-3  Byte Count */
	u_long	sdptr;		/*4-7  Source/Destination Pointer */
	u_long	next;		/*8-b  Next Ring Buffer Record Pointer */
} rbblk_t;

/*
 * gate - (Gate of Ring Buffer)
 */
#define RG_READY	0x01	/* Buffer Ready */
#define RG_BUSY		0x02	/* Buffer Busy */
#define RG_COMPLETE	0x04	/* Buffer Complete */
#define RG_ERROR	0x08	/* Error */
#define RG_ABORT	0x10	/* Abort Ring Buffer Operation */
#define RG_LAST		0x20	/* Last Ring Buffer */
#define RG_INTERRUPT	0x40	/* Rind Buffer Interrupt */

#endif

/*
 * Miscellaneous constants
 */
#define NULL_BP		((struct buf *) 0)

/*
 * Time constants  -  units are seconds
 */
#define TIME_FSF	(60 * 17)		/* no more than 17 minutes */
#define TIME_REWIND	(60 * 2)		/* no more than 2 minutes */
#define TIME_RDWR	(90)			/* 1.5 minutes */
#define TIME_WAIT	(60)			/* default */


struct softc_tape {
	resource st_tape_res;		/* tape unit is a lockable resource */
	u_int	st_flags;
	u_short	st_status;		/* 2 status bytes */
	u_char	st_unit;
	daddr_t st_fileno;
	int	st_resid;		/* byte cnt not done in prev I/O */
	struct	softc *st_ctlr;		/* back pointer to controller */
};

/*
 * st_flags bits:
 */
#define ST_ALIVE	0x001		/* drive is alive */
#define ST_OPEN		0x002		/* unit has been opened */
#define ST_ENDOFFILE	0x004		/* drive is at EOF */
#define ST_WRITTEN	0x008		/* tape has been written */
#define ST_EOT		0x010		/* EOT encountered */
#define ST_IGNEOT	0x020		/* ignore EOT condition */

/*
 * sc_flags bits:
 */
#define SC_ALIVE	0x01		/* controller probed */


/* controller state */
struct	softc {
	resource sc_ctlr_res;		/* ctlr iopb is a lockable resource */
	u_int	sc_flags;
	para_t	sc_pb;			/* parameter block */
	cblk_t	sc_cb;			/* configuration block */
	caddr_t	sc_ioaddr;		/* virtual address of i/o port */
	u_long	sc_pblk_mbva; 		/* multibus address of pblk */
	u_long	sc_cblk_mbva; 		/* multibus address of cblock */
	struct	buf *sc_buf;		/* buffer ptr for dma */
	int	sc_unit;
	struct softc_tape sc_tape[NTM];
#ifdef NONSTD_COMMAND
	nsc_t 	sc_nsb;			/* Non-standard command block */
#endif
#ifdef RING_BUFFER
	rbblk_t sc_rb;			/* Ring buffer */
#endif
} tmtsoftc;				/* allow only 1 ctlr for now */

/* Bits of the minor device number:
 *		87654321
 *		^^^^^^^^
 *		|\ /\__/
 *		| |  |
 *		| |  |
 *		| |  TAPEMINOR (4-1)
 *		| |
 *		| UNIT (7-5)
 *		|
 *		No rewind at close (8) (no longer used)
 */
#define TAPEMINOR(dev)	(minor(dev) & 0xf)
#define UNIT(dev)  	((minor(dev)>>4) &0x07)

/*
 * Tape Minors
 */
#define NOREWIND	0x01
#define DENS_LO		0x02	/* Low Density */
#define SPEED_HI	0x04	/* Hi Speed (bad if streaming tape
				 *   won't stream)
				 */
#define BYTESWAP	0x08
