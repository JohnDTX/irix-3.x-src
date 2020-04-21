/* Copyright 1986, Silicon Graphics Inc., Mountain View, CA. */
/*
 * Streams driver for the Central Data serial board
 *
 * To Do:
 *	o write a standalone diagnostic to go with the driver (MIPS)
 *	o fix ^S/^Q to use send-immediate
 *	o make sure driver works on pm2
 *	o test multiple boards on clover
 *	o test multiple boards on pm2/ip2
 *
 * $Header: /d2/3.7/src/sys/multibus/RCS/cdsio.c,v 1.1 89/03/27 17:31:15 root Exp $
 */

#if defined(PM2) || defined(IP2)
#define	wbflush()
#include "cd.h"
#include "../debug/debug.h"
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/sysmacros.h"
#include "../h/user.h"
#include "../h/termio.h"
#include "../streams/stream.h"
#include "../streams/stropts.h"
#include "../streams/strids.h"
#include "../streams/stty_ld.h"
#include "../h/file.h"
#include "machine/cpureg.h"
#define	INDRIVER
#include "../multibus/cdsioreg.h"
#include "../multibus/mbvar.h"
#else
#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/fs/s5dir.h"
#include "sys/pcb.h"
#include "sys/immu.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/termio.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/strids.h"
#include "sys/stty_ld.h"
#include "sys/edt.h"
#include "sys/file.h"
#define	INDRIVER
#include "sys/cdsioreg.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"
#include "sys/kopt.h"
#include "sys/sysinfo.h"
#include "sys/ksa.h"
#include "sys/pda.h"
#endif

#if !defined(SVR0) || NCD > 0
#if defined(mips) && defined(SVR0)
error					/* we run SVR3 on MIPS chips */
#endif

#if !defined(CD3608) && !defined(CD3100)
error					/* it must be one of these boards */
#endif

#if defined(mips) && !defined(IP4) && !defined(R2300)
error					/* code assumes 1 or other MIPS CPU */
#endif


#ifdef SVR3
/*
 * lboot configuration info
 */
extern	struct cdport cdport[];		/* ordered by external port # */
extern	short int cdnumports;		/* total ports configured */
extern	short int cdnumboards;		/* total boards configured */

extern	char	cdprobed[];

#else SVR3
/*
 * manual configuration info
 */
/* software struct for each port */
static struct cdport cdport[NCD*CDUPC];	/* ordered by external port # */
static short int cdnumports = NCD * CDUPC;
static short int cdnumboards = NCD;

static	char	cdprobed[NCD];

int	cdintr();
static	int	cdProbe();
static	struct mb_device *cddinfo[NCD];
struct	mb_driver cddriver = {
	cdProbe, (int (*)())0, (int (*)())0, (int (*)())0, cdintr,
	(char *(*)())0, "cdsio", cddinfo,
};

#ifdef IP2
#define	CDMEMBASE	0x400000	/* base multibus address to start */
#else
#define	CDMEMBASE	error		/* base multibus address to start */
#endif
#define	CDMEMSIZE	0x010000	/* size of each board */
#define	CDIPL		(5|INT_ENABLE)	/* interrupt info */
#endif SVR3


#define	TRUE	(1)
#define	FALSE	(0)

#define INT_HZ	50			/* max. input interrupts/sec */
#define INT_LIM_MAX 1800
#define INT_LIM (INT_LIM_MAX/INT_HZ)

#define HWM	(INBUF_LEN-10)		/* set high water mark to this */


#define DELAY_TIME 2000			/* delay this long for commands */
#define PROBE_TIME 50000000		/* this long to probe the board */

/* decide when to turn on RTS or DTR */
#define	RTSBIT(dp,b)	(((((dp)->dp_state & (DP_FLOW|DP_RTS)) == DP_FLOW) \
			  || !((dp)->dp_state & (DP_ISOPEN|DP_WOPEN))) \
			 ? 0 : (b))
#define	DTRBIT(cf,dp,b)	((((cf) & CBAUD) \
			  && ((dp)->dp_state & (DP_ISOPEN|DP_WOPEN))) \
			 ? (b) : 0)

#ifdef	CD3608
/*
 * lookup tables to convert CFLAG's to write register bits
 */
static char wr3BitsPerChar[] = { WR3_BPC_5, WR3_BPC_6, WR3_BPC_7, WR3_BPC_8 };
static char wr5BitsPerChar[] = { WR5_BPC_5, WR5_BPC_6, WR5_BPC_7, WR5_BPC_8 };

/* macros to use lookup tables and whatnot */
#define	WR3BPC(cf)	wr3BitsPerChar[((cf) & CSIZE) >> 4]
#define	WR5BPC(cf)	wr5BitsPerChar[((cf) & CSIZE) >> 4]
#define	STOPBITS(cf)	(((cf) & CSTOPB) ? WR4_STOP_2 : WR4_STOP_1)
#define	PARITY(cf) \
	(((cf) & PARENB) \
	 ? (((cf) & PARODD) ? WR4_PARITY : WR4_EVEN|WR4_PARITY) \
	 : 0)

#define WR3FLOW(dp)	(((dp)->dp_state&DP_FLOW) ? WR3_DCD : 0)
#define	WR3OFF(cf,dp)	(WR3BPC(cf) | WR3FLOW(dp) |   0)
#define	WR3ON(cf,dp)	(WR3BPC(cf) | WR3FLOW(dp) | WR3_RCV)
#define	WR4BITS(cf)	(WR4_CLK_16 | STOPBITS(cf) | PARITY(cf))
#define	WR5BITS(cf,dp)	(DTRBIT(cf,dp,WR5_DTR) | WR5BPC(cf) | WR5_BIT3 \
			 | RTSBIT(dp,WR5_RTS))
#define	WR14BITS	WR14_CONST
#define BAUDOK(b)	TRUE

/* CBAUD to WR13 and WR12 values */
static	struct {
	char	wr13;			/* wr13 value */
	char	wr12;			/* wr12 value */
} baud_tbl[CBAUD+1] = {
	{ 0x00, 0x06 },			/* B0 --> hangup port */
	{ 0x05, 0xFE },			/* B50 */
	{ 0x03, 0xFE },			/* B75 */
	{ 0x02, 0xB8 },			/* B110 */
	{ 0x02, 0x39 },			/* B134.5 */
	{ 0x01, 0xFE },			/* B150 */
	{ 0x01, 0x7E },			/* B200 */
	{ 0x00, 0xFE },			/* B300 */
	{ 0x00, 0x7E },			/* B600 */
	{ 0x00, 0x3E },			/* B1200 */
	{ 0x00, 0x29 },			/* B1800, approximately */
	{ 0x00, 0x1E },			/* B2400 */
	{ 0x00, 0x0E },			/* B4800 */
	{ 0x00, 0x06 },			/* B9600 */
	{ 0x00, 0x02 },			/* B19200 */
	{ 0x00, 0x00 },			/* B38400 */
};
#endif	/* CD3100 */

#ifdef	CD3100
static unchar mr1BitsPerChar[] = {MR1_BPC_5, MR1_BPC_6, MR1_BPC_7, MR1_BPC_8};
static unchar mr2Baud[CBAUD+1] = {
	MR2_BAUD(0x0E),			/* B0 --> hangup port */
	MR2_BAUD(0x00),			/* B50 */
	MR2_BAUD(0x01),			/* B75 */
	MR2_BAUD(0x02),			/* B110 */
	MR2_BAUD(0x03),			/* B134 */
	MR2_BAUD(0x04),			/* B150 */
	MR2_BAUD(0xFF),			/* B200 -- not supported */
	MR2_BAUD(0x05),			/* B300 */
	MR2_BAUD(0x06),			/* B600 */
	MR2_BAUD(0x07),			/* B1200 */
	MR2_BAUD(0x08),			/* B1800 */
	MR2_BAUD(0x0A),			/* B2400 */
	MR2_BAUD(0x0C),			/* B4800 */
	MR2_BAUD(0x0E),			/* B9600 */
	MR2_BAUD(0x0F),			/* B19200 */
	MR2_BAUD(0xFF),			/* B38400 -- not supported */
};

#define	STOPBITS(cf)	(((cf) & CSTOPB) ? MR1_STOP_2 : MR1_STOP_1)
#define	MR1BPC(cf)	mr1BitsPerChar[((cf) & CSIZE) >> 4]
#define	PARITY(cf) \
	(((cf) & PARENB) \
	 ? (((cf) & PARODD) ? MR1_PARITY : (MR1_EVEN|MR1_PARITY)) \
	 : 0)

#define	MR1BITS(cf)	(STOPBITS(cf) | PARITY(cf) | MR1BPC(cf) | MR1_BIT1)
#define	MR2BITS(baud)	mr2Baud[baud]
#define	CRBITS(cf,dp)	(CR_CONST | RTSBIT(dp,CR_RTS) | DTRBIT(cf,dp,CR_DTR))
#define	BAUDOK(b)	(mr2Baud[b] != (unchar)0xFF)
#endif

/* if any of these change in cflag, then the chip must be reprogrammed */
#define RE_PROG	(CBAUD|CSIZE|CSTOPB|PARENB|PARODD)


/* stream stuff */
extern	struct stty_ld def_stty_ld;
static	struct module_info dum_info = {
	STRID_CD3608,			/* module ID */
	"CD3608",			/* module name */
	0,				/* minimum packet size */
	INFPSZ,				/* maximum packet size--infinite */
	128,				/* hi-water mark */
	16,				/* lo-water mark */
};

#define MIN_RMSG_LEN 16			/* minimum buffer size */
#define MAX_RMSG_LEN 2048		/* largest msg allowed */
#define XOFF_RMSG_LEN 256		/* send XOFF here */
#define MAX_RBUF_LEN 1024

#define	CARRPRI	STIPRI			/* sleep for carrier at this */

static int cdOpen();
static int cdRsrv(), cdClose();
static struct qinit cdrinit = {
	NULL, cdRsrv, cdOpen, cdClose, NULL, &dum_info, NULL
};

static int cdWput();
static struct qinit cdwinit = {
	cdWput, NULL, NULL, NULL, NULL, &dum_info, NULL
};

struct streamtab cdsioinfo = {&cdrinit, &cdwinit, NULL, NULL};


/* macros to get at bits in dev
 *	They know that CDUPC and NUMMINOR are powers of 2,
 *	and that CDUPC<NUMMINOR.
 */
#ifdef SVR0
#define PORT_KLUDGE	0
#define NUMMINOR	64		/* must be (2**n) */
#define	DEV_OFFSET	4
#else
#define PORT_KLUDGE	2		/* Clover ports are misnumbered */
#define NUMMINOR	32		/* must be (2**n) */
#define	DEV_OFFSET	5
#endif
#if PORT_KLUDGE == 0
#define EXPORT_TO_BDPORT(p) ((p) & (CDUPC-1))
#else
#define EXPORT_TO_BDPORT(p) (((p) + PORT_KLUDGE) & (CDUPC-1))
#endif
#define DEV_TO_EXPORT(dev)  (((dev) - DEV_OFFSET) & (NUMMINOR-1))
#define DEV_TO_BDPORT(dev)  EXPORT_TO_BDPORT(DEV_TO_EXPORT(dev))
#define DEV_TO_BOARD(dev)   ((((dev)-DEV_OFFSET) & (NUMMINOR-1)) >> CDUPC_LN)
#define BOARD_TO_EXPORT(b)  ((b) << CDUPC_LN)
#define BOARD_TO_MINOR(b,n) (BOARD_TO_EXPORT(b) + DEV_OFFSET + (n))

#define	MODEM(dev)	((dev) & (NUMMINOR*3))
#define FLOW_MODEM(dev)	((dev) & (NUMMINOR*2))


static	char *cderrors[] = {
	"no error",
#ifdef	CD3608
	"bad eprom checksum",
	"bad scratchpad ram",
	"bad dual-port ram",
	"bad scc addressing",
	"bad usart test",
	"receiver interrupt failed",
#else
	"unknown error",
	"bad eprom checksum",
	"bad scratchpad ram",
	"bad dual-port ram",
	"bad bus-lock",
	"bad usart test",
	"receiver interrupt failed",
	"partial usart damage",
#endif
};
#define	CDERRS		(sizeof(cderrors) / sizeof(char *))
#define cdErrorMsg(c)	((c) >= CDERRS ? "Unknown error" : cderrors[c])


static	void	cdStartOutput();
static	void	cdDelay();


#ifdef OS_DEBUG
int cdsio_noise=0;
#ifdef SVR0
int cdsio_debug=0;
#endif
#endif

#ifdef OS_METER
/* slowest command observed to date */
#define NUM_TIMOS 0x12
unchar cdsio_max_timo[NUM_TIMOS];
#endif

/*
 * Poll the board, waiting for it to be ready for a new command
 */
static void
cdPoll(dp)
	register struct cdport *dp;
{
	register volatile struct iodevice *iod = dp->dp_iodevice;
	register volatile struct device *d = dp->dp_device;
	register unchar timo;

#define CHECK_FREQ 50
#define SCALED_DELAY_TIME (DELAY_TIME/CHECK_FREQ)
#if SCALED_DELAY_TIME > 127
	? ?/* error timo is only a char, so that it is cheap to meter */
#endif
	timo = 0;
	while (!(iod->hostStatusPort & HSP_READY)
	       && ++timo <= SCALED_DELAY_TIME) {
		DELAY(CHECK_FREQ);
	}

	if (timo > SCALED_DELAY_TIME || d->cmdStatus != 0) {
#define FAIL_PAT "ttyd%d: cmd 0x%x failed: data=(%x,%x) status=0x%x timo=%d\n"
#ifdef	SVR0
		printf(FAIL_PAT, dp->dp_minor, d->cmd, d->cmdHigh,
		       d->cmdLow, d->cmdStatus, timo);
#else
		cmn_err(CE_CONT, FAIL_PAT, dp->dp_minor, d->cmd, d->cmdHigh,
			d->cmdLow, d->cmdStatus, timo);
#endif
#ifdef OS_METER
	} else if (timo > 2) {
		register unchar *pc = &cdsio_max_timo[d->cmd];
		if (pc < &cdsio_max_timo[NUM_TIMOS]
		    && timo > *pc)
			*pc = timo;
#endif
	}
}

/* Start board on a new command
 *	Interrupts must be off.
 */
static void
cdCmd(dp, cmd, cmdhigh, cmdlow)
	register struct cdport *dp;
	unchar cmd, cmdhigh, cmdlow;
{
	register volatile struct device *d = dp->dp_device;
	register int s = spltty();

	cdPoll(dp);			/* wait for previous command */

	d->cmd = cmd;
	d->cmdHigh = cmdhigh;
	d->cmdLow = cmdlow;
	d->cmdStatus = 0xFF;
	wbflush();			/* get arguments on board */

	dp->dp_iodevice->cmdPort = dp->dp_bit;
	wbflush();			/* ZOT the board */

#ifdef OS_DEBUG
	if (cdsio_noise)
		printf("\tttyd%d:cmd=%x,data=(%x,%x),uart=%x ",
		       dp->dp_minor, cmd, cmdhigh, cmdlow,
		       dp->dp_device->uartStatus);
#ifdef SVR0
	if (cdsio_debug)
		debug();
#endif
#endif
	splx(s);
}

/*
 * Return TRUE if the given port is has carrier or does not care.
 */
static int
cdIsOn(dp)
	register struct cdport *dp;
{
	if (dp->dp_device->uartStatus & SCC_DCD) {
		dp->dp_state |= DP_DCD;
		return (TRUE);
	} else {
		dp->dp_state &= ~DP_DCD;
		return (0 != (dp->dp_cflag & CLOCAL));
	}
}

/* Turn DCD interrupt on or off
 *	Interrupts must be off here.
 */
static void
cdSetDCDint(dp)
	register struct cdport *dp;
{
	register uint mask0, mask1;

	mask0 = 0;
	mask1 = 0;
	if (!(dp->dp_cflag & CLOCAL)
	    && (dp->dp_cflag & CBAUD) != B0
	    && 0 != (dp->dp_state & (DP_ISOPEN|DP_WOPEN))) {
		if (!(dp->dp_state & DP_DCD)) {
			mask0 = DCDON_MASK0;
			mask1 = DCDON_MASK1;
		} else if (dp->dp_state & DP_ISOPEN) {
			mask0 = DCDOFF_MASK0;
			mask1 = DCDOFF_MASK1;
		}
	}

	cdCmd(dp, CMD_SET_ISB1, 0, mask1);
	cdCmd(dp, CMD_SET_ISB0, 0, mask0);
}

/* Clear receiver input buffer
 *	Interrupts must be off here.
 */
static void
cdReceiveClear(dp)
	struct cdport *dp;
{
	dp->dp_device->inputFillPtr = 0;
	wbflush();
	dp->dp_device->inputEmptyPtr = 0;
	wbflush();
}

/* tell board what to do about RTS and turn on DTR
 *	Interrupts must be off.
 */
static void
cdRTS_DTR(dp)
	register struct cdport *dp;
{
#ifdef	CD3608
	cdCmd(dp, CMD_SET_SCC, WR5BITS(dp->dp_cflag,dp), 5);
#else
	cdCmd(dp, CMD_SET_CR, 0, CRBITS(dp->dp_cflag,dp));
#endif
}

/*
 * flush output
 *	Interrupts must have been made safe here.
 */
static void
cdFlushOutput(dp)
	register struct cdport *dp;
{
	if ((dp->dp_state & (DP_TIMEOUT|DP_BREAK)) == DP_TIMEOUT) {
#ifdef	SVR0
		untimeout_id(dp->dp_tid);	/* forget stray timeout */
#else
		untimeout(dp->dp_tid);		/* forget stray timeout */
#endif
		dp->dp_state &= ~DP_TIMEOUT;
	}

	dp->dp_xmit = 0;
}

/*
 * flush input
 *	interrupts must be safe here
 */
static void
cdFlushInput(dp)
	register struct cdport *dp;
{
	freemsg(dp->dp_rmsg);
	dp->dp_rmsg = NULL;
	dp->dp_rmsg_len = 0;
	freemsg(dp->dp_rbp);
	dp->dp_rbp = NULL;
	cdReceiveClear(dp);

	qenable(dp->dp_rq);		/* turn input back on */
}

/*
 * Hang up a line,
 *	but only if all output is finished.  This is so that we do not
 *	drop DTR or RTS before we have done all output.  A common cabling
 *	convention is to connect CTS to either RTS or DTR.  If CTS drops
 *	before we are finished, we will not be able to reopen the port,
 *	because the open function will be unable to reprogram the UART.
 *	That is because it must avoid touching the UART while it is
 *	transmitting.
 */
static void
cdHangup(dp)
	struct cdport *dp;
{
	register int s = spltty();

	cdFlushOutput(dp);		/* forget pending output */

	dp->dp_cflag &= ~CBAUD;
	if (cdPoll(dp), !(dp->dp_device->outputBusy)) {
		cdRTS_DTR(dp);		/* kill the modem */
		dp->dp_oldcflag &= ~CBAUD;
	}
	splx(s);
}

/*
 * Set the port parameters
 */
static int
cdSetParams(dp, cf, tp)
	register struct cdport *dp;
	register ushort cf;		/* new control flags */
	struct termio *tp;
{
	register int baud;
	register int max;
	register int s;

	baud = (cf & CBAUD);
	if (!BAUDOK(baud))
		return (FALSE);

	s = spltty();

	/* get overflow count before the board resets it */
	dp->dp_overflows += dp->dp_device->inputOverflows;
	dp->dp_device->inputOverflows = 0;

	if (0 != tp)
		dp->dp_termio = *tp;
	dp->dp_cflag = cf;

	/* Bang on the chip. If the board is busy, wait to set it up. It
	 * goes crazy if you change the UART modes while it is
	 * transmitting. */
	if ((cf ^ dp->dp_oldcflag) & RE_PROG) {
		if (!baud) {
			cdHangup(dp);
			splx(s);
			return TRUE;
		}

		if (cdPoll(dp), dp->dp_device->outputBusy) {
			splx(s);
			return TRUE;
		}
#ifdef	CD3608
		/*
		 * Disable receiver.  Setup transmitter state, then
		 * enable the receiver.
		 */
		cdCmd(dp, CMD_SET_SCC, WR3OFF(cf,dp), 3);
		cdCmd(dp, CMD_SET_SCC, WR4BITS(cf), 4);
		cdCmd(dp, CMD_SET_BAUD, baud_tbl[baud].wr13,
		      baud_tbl[baud].wr12);
		cdCmd(dp, CMD_SET_SCC, WR14BITS, 14);
		cdCmd(dp, CMD_SET_SCC, WR3ON(cf,dp), 3);
#else
		cdCmd(dp, CMD_SET_MR12, MR1BITS(cf), MR2BITS(baud));
#endif
		cdRTS_DTR(dp);
		cdCmd(dp, CMD_SET_IDT, INT_LIM>>8, INT_LIM);
		cdCmd(dp, CMD_SET_HWM, HWM>>8, HWM);
		if (!(cf & CLOCAL))	/* violates modularity, but */
			cdSetDCDint(dp);	/* stupid board is slow */
	}
	dp->dp_oldcflag = cf;

	/* Compute a good output burst size.  If we do not need to
	 *	stop output quickly, we can use most of the buffer.
	 *
	 *	These values should be about .03 seconds when we might
	 *	have to stop, and about .5 otherwise.
	 */
	max = 4;
	if (baud > B1200)
		max += (2 << (baud-B1200));
	if (!(dp->dp_iflag & IXON) && !(dp->dp_lflag & ISIG)) {
		max <<= 4;
#if		OUTBUF_LEN <= (2<<(CBAUD-B1200+4))+4
		if (max > OUTBUF_LEN)
			max = OUTBUF_LEN;
#endif
	}
	dp->dp_xmitlimit = max;

	splx(s);
	return (TRUE);
}

/*
 * open a serial port
 */
static int
cdOpen(rq, dev, flag, sflag)
	queue_t *rq;
	dev_t dev;
	int flag, sflag;
{
	register struct cdport *dp;
	register queue_t *wq = WR(rq);
	register ushort export;
	int s;

	if (sflag)			/* only a simple stream driver */
		return (OPENFAIL);

	/* validate device */
	export = DEV_TO_EXPORT(dev);
	if ((export > cdnumports) || !cdprobed[DEV_TO_BOARD(dev)]) {
		u.u_error = ENXIO;
		return (OPENFAIL);
	}

	dp = &cdport[export];

	s = spltty();
	if (!(dp->dp_state & (DP_ISOPEN|DP_WOPEN))) {	/* on the 1st open */
		register uint cflag;

		cflag = def_stty_ld.st_cflag;
		dp->dp_state &= ~(DP_TXSTOP|DP_LIT|DP_BLOCK
				  |DP_TX_TXON|DP_TX_TXOFF
				  |DP_FLOW);
		if (MODEM(dev)) {
			cflag &= ~CLOCAL;
			if (FLOW_MODEM(dev))
				dp->dp_state |= DP_FLOW;
		}
		dp->dp_state |= (DP_WOPEN|DP_RTS);
		dp->dp_litc = CLNEXT;	/* clear everything */
		dp->dp_stopc = CSTOP;
		dp->dp_startc = CSTART;
		dp->dp_cflag = 0;

		for (;;) {
			(void)cdSetParams(dp, cflag, &def_stty_ld.st_termio);
			if (dp->dp_cflag != dp->dp_oldcflag) {
				/* Turn on DTR even if the UART is too busy
				 * to be reprogrammed.  That is to help the
				 * UART drain its output buffer */
				cdRTS_DTR(dp);
			}
			cdReceiveClear(dp);

			/* wait for carrier, if we are supposed to */
			if (cdIsOn(dp) || (flag & FNDELAY))
				break;
			if (sleep((caddr_t)dp, CARRPRI|PCATCH)) {
				u.u_error = EINTR;
				dp->dp_state &= ~(DP_ISOPEN|DP_WOPEN);
				cdHangup(dp);
				splx(s);
				return (OPENFAIL);
			}
			/* loop on DCD glitch */
		}

		rq->q_ptr = (caddr_t)dp;	/* connect device to stream */
		wq->q_ptr = (caddr_t)dp;
		dp->dp_wq = wq;
		dp->dp_rq = rq;
		dp->dp_state &= ~DP_WOPEN;
		dp->dp_state |= (DP_ISOPEN|DP_RTS);
		dp->dp_cflag |= CREAD;

		if (!(cflag & CLOCAL))	/* violates modularity, but board */
			cdSetDCDint(dp);	/* is slow to program */

		if (!strdrv_push(rq, "stty_ld", dev)) {
			dp->dp_state &= ~(DP_ISOPEN|DP_WOPEN);
			cdHangup(dp);
			splx(s);
			return (OPENFAIL);
		}

	} else {
		/*
		 * You cannot open two streams to the same device.  The dp
		 * structure can only point to one of them.  Therefore, you
		 * cannot open two different minor devices that are synonyms
		 * for the same device.
		 * XXX eliminate this restriction?
		 */
		if (dp->dp_rq == rq) {
			ASSERT(dp->dp_wq == wq);
			ASSERT(dp->dp_rq->q_ptr == (caddr_t)dp);
			ASSERT(dp->dp_wq->q_ptr == (caddr_t)dp);
		} else {
			u.u_error = ENOSR;	/* fail if already open */
			splx(s);
			return (OPENFAIL);
		}
	}
	splx(s);
	return (minor(dev));		/* return successfully */
}

/*
 * close a port
 */
static int
cdClose(rq)
	queue_t *rq;
{
	register struct cdport *dp;
	register int s;

	dp = (struct cdport *)rq->q_ptr;
	if (!dp) {
		/* port wasn't really open */
		return;
	}

	s = spltty();
	ASSERT(dp >= &cdport[0] && dp->dp_rq == rq);
	cdFlushInput(dp);
	cdFlushOutput(dp);
	dp->dp_rq = NULL;
	dp->dp_wq = NULL;
	dp->dp_state &= ~(DP_ISOPEN|DP_WOPEN);

	/* hang up line if we are supposed to */
	if ((dp->dp_cflag & HUPCL))
		cdHangup(dp);
	splx(s);
}

/*
 * get a new buffer
 *	Interrupts ought to be off here.
 */
static mblk_t *
cdGetNewbp(dp,pri)
	register struct cdport *dp;
	uint pri;
{
	register int size;
	register mblk_t *bp;
	register mblk_t *rbp;

	rbp = dp->dp_rbp;
	if (dp->dp_rmsg_len >= MAX_RMSG_LEN	/* if overflowing */
	    || (rbp != 0		/* or current buffer empty */
		&& rbp->b_rptr >= rbp->b_wptr)) {
		bp = 0;
	} else {
		size = dp->dp_rbsize;
		if (size > MAX_RBUF_LEN)	/* larger buffer */
			size = MAX_RBUF_LEN;	/* as we get behind */
		for (;;) {
			if (size < MIN_RMSG_LEN)
				size = MIN_RMSG_LEN;

			bp = allocb(size, pri);
			if (bp != 0)
				break;

			if (BPRI_HI == pri
			    && size > MIN_RMSG_LEN) {
				size >>= 2;
				continue;
			}
			break;
		}
	}

	if (rbp == 0) {			/* if we have an old buffer */
		dp->dp_rbp = bp;
	} else if ((bp != 0) ||
		   (rbp->b_wptr >= rbp->b_datap->db_lim)) {
		   /* & a new buffer */
		    /* or old buffer is full */
		str_conmsg(&dp->dp_rmsg, &dp->dp_rmsge, rbp);
		dp->dp_rmsg_len += (rbp->b_wptr - rbp->b_rptr);
		dp->dp_rbp = bp;
	}

	if (dp->dp_rmsg_len >= XOFF_RMSG_LEN
	    || !dp->dp_rbp) {
		if ((dp->dp_state & DP_RTS) && (dp->dp_state & DP_FLOW)) {
			dp->dp_state &= ~DP_RTS;
			cdRTS_DTR(dp);
		}
		if ((dp->dp_iflag & IXOFF)	/*  do XOFF */
		    && !(dp->dp_state & DP_BLOCK)) {
			dp->dp_state |= DP_TX_TXOFF;
			dp->dp_state &= ~DP_TX_TXON;
			cdStartOutput(dp);
		}
	}

	return bp;
}

/*
 * send a bunch of 1 or more characters up the stream
 *	This should be invoked only because a message could not be sent
 *	upwards by the interrupt, and things have now drained.
 */
static int
cdRsrv(rq)
	register queue_t *rq;
{
	register mblk_t *bp;
	register struct cdport *dp;
	register int s = spltty();

	dp = (struct cdport *)rq->q_ptr;
	ASSERT(dp->dp_rq == rq);
	ASSERT(dp >= &cdport[0] && dp <= &cdport[cdnumports]);
	ASSERT(dp->dp_state & DP_ISOPEN);

	if (!canput(rq->q_next)) {	/* quit if upstream congested */
		noenable(rq);
		splx(s);
		return;
	}
	enableok(rq);

	if ((bp = dp->dp_rbp) != NULL) {
		register int sz;
		sz = (bp->b_wptr - bp->b_rptr);
		if (sz > 0) {
			str_conmsg(&dp->dp_rmsg, &dp->dp_rmsge, bp);
			dp->dp_rmsg_len += sz;
			dp->dp_rbp = 0;
		}
	}

	if ((bp = dp->dp_rmsg) != NULL) {
		dp->dp_rmsg = 0;
		dp->dp_rbsize = (dp->dp_rmsg_len + dp->dp_rbsize)/2;
		dp->dp_rmsg_len = 0;
		splx(s);		/* without too much blocking, */
		putnext(rq, bp);	/* send the message */
		(void)spltty();
	}

	if (!dp->dp_rmsg) {
		if (!(dp->dp_state & DP_RTS)) {
			dp->dp_state |= DP_RTS;
			cdRTS_DTR(dp);
		}
		if (dp->dp_state & DP_BLOCK) {	/* do XON */
			dp->dp_state |= DP_TX_TXON;
			cdStartOutput(dp);
		}
	}

	if (!dp->dp_rbp)
		(void) cdGetNewbp(dp, BPRI_LO);

	splx(s);
}

/*
 * 'put' function
 *	Just start the output if we like the message.
 */
static int
cdWput(wq, bp)
	queue_t *wq;
	register mblk_t *bp;
{
	register struct cdport *dp;
	register struct iocblk *iocp;
	register int s;

	dp = (struct cdport *)wq->q_ptr;
	if (!dp) {
		sdrv_error(wq,bp);	/* quit now if not open */
		return;
	}

	s = spltty();
	ASSERT(dp->dp_wq == wq);
	ASSERT(dp >= &cdport[0] && dp <= &cdport[cdnumports]);
	ASSERT(dp->dp_state & DP_ISOPEN);

	switch (bp->b_datap->db_type) {

	case M_FLUSH:
		if (*bp->b_rptr & FLUSHW) {
			cdFlushOutput(dp);
			dp->dp_state &= ~DP_TXSTOP;
			cdStartOutput(dp);	/* restart output */
		}
		if (*bp->b_rptr & FLUSHR)
			cdFlushInput(dp);
		sdrv_flush(wq,bp);
		break;

	case M_DATA:
	case M_DELAY:
		putq(wq, bp);
		cdStartOutput(dp);
		break;

	case M_IOCTL:
		iocp = (struct iocblk*)bp->b_rptr;
		switch (iocp->ioc_cmd) {
		case TCXONC:
			ASSERT(iocp->ioc_count == sizeof(int));
			switch (*(int*)(bp->b_cont->b_rptr)) {
			case 0:		/* stop output */
				dp->dp_state |= DP_TXSTOP;
				break;
			case 1:		/* resume output */
				dp->dp_state &= ~DP_TXSTOP;
				cdStartOutput(dp);
				break;
			case 2:
				dp->dp_state &= ~DP_RTS;
				cdRTS_DTR(dp);
				if (!(dp->dp_state & DP_BLOCK)) {
					dp->dp_state |= DP_TX_TXOFF;
					dp->dp_state &= ~DP_TX_TXON;
					cdStartOutput(dp);
				}
				break;
			case 3:
				dp->dp_state |= DP_RTS;
				cdRTS_DTR(dp);
				if (dp->dp_state & DP_BLOCK) {
					dp->dp_state |= DP_TX_TXON;
					cdStartOutput(dp);
				}
				break;
			default:
				iocp->ioc_error = EINVAL;
				break;
			}
			bp->b_datap->db_type = M_IOCACK;
			iocp->ioc_count = 0;
			qreply(wq, bp);
			break;

		case TCSETA:
		case TCSETAW:
		case TCSETAF:
			ASSERT(iocp->ioc_count == sizeof(struct termio));
			putq(wq, bp);
			cdStartOutput(dp);
			break;

		case TCGETA:
			tcgeta(wq, bp, &dp->dp_termio);
			break;

		case TCSBRK:
			putq(wq, bp);
			cdStartOutput(dp);
			break;

		default:
			bp->b_datap->db_type = M_IOCNAK;
			qreply(wq, bp);
			break;
		}
		break;


	default:
		sdrv_error(wq, bp);
	}

	splx(s);
}

static void
cdDelay(dp)
	register struct cdport *dp;
{
	int s;

	s = spltty();
	dp->dp_state &= ~(DP_TIMEOUT|DP_BREAK);
	cdStartOutput(dp);		/* resume output */
	splx(s);
}

/*
 * interrupt-process an IOCTL
 *	This function processes those IOCTLs that must be done by the output
 *	interrupt.
 */
static int				/* 1=board is now busy */
cdInterruptIoctl(dp, bp)
	register struct cdport *dp;
	register mblk_t *bp;
{
	register struct iocblk *iocp;
	register int needsAttention;
	register struct termio *tp;
	register uint cflag;

	needsAttention = 0;
	iocp = (struct iocblk *)bp->b_rptr;
	if (iocp->ioc_cmd == TCSBRK) {
		if (*(int *)bp->b_cont->b_rptr == 0) {
			register volatile struct device *d;

			/* tell board to send a break */
			cdCmd(dp,CMD_SEND_BREAK,0,0);

			dp->dp_state |= (DP_TIMEOUT|DP_BREAK);
			/* The manual says BREAK may last 350 msec, so
			 * to avoid talking to board until it is finished,
			 * we delay for 350 msec.  Then, we delay about a
			 * character time to let the other host recognize the
			 * end of the break.
			 */
			dp->dp_tid = timeout(cdDelay, (caddr_t)dp,
					     (HZ*350)/1000+1 + HZ/20);
			needsAttention = 1;
		}
		bp->b_datap->db_type = M_IOCACK;

	} else {
		tp = STERMIO(bp);
		cflag = tp->c_cflag;
		cflag &= ~CLOCAL;
		if (dp->dp_cflag & CLOCAL)
			cflag |= CLOCAL;
		tp->c_cflag = cflag;

		if (!cdSetParams(dp, cflag, tp)) {	/* oops, error */
			bp->b_datap->db_type = M_IOCNAK;
		} else {
			if (iocp->ioc_cmd == TCSETAF)
				(void) putctl1(dp->dp_rq->q_next,
					       M_FLUSH, FLUSHR);
			bp->b_datap->db_type = M_IOCACK;
		}
	}
	iocp->ioc_count = 0;
	putnext(dp->dp_rq, bp);

	return (needsAttention);
}

/*
 * Output Interrupt Service Routine
 */
static void
cdOutput(dp)
	register struct cdport *dp;
{
	register mblk_t *wbp;
	register volatile unchar *obuf;
	register unchar *p;
	register int i;
	register int xmitlimit, xmit;

	if (dp->dp_cflag != dp->dp_oldcflag)
		cdSetParams(dp,dp->dp_cflag,0);

	if (!(dp->dp_state & (DP_ISOPEN|DP_WOPEN)
	      || !(dp->dp_cflag & CBAUD)))
		return;

	if (!(dp->dp_state & DP_ISOPEN))
		return;

	xmitlimit = (dp->dp_state & DP_TXSTOP) ? 0 : dp->dp_xmitlimit;
	xmit = dp->dp_xmit;
	obuf = &dp->dp_device->outputBuf0[xmit];
	if (dp->dp_state & DP_TX_TXON) {	/* send XON or XOFF */
		xmit++;
		*obuf++ = dp->dp_startc;
		dp->dp_state &= ~(DP_TX_TXON|DP_TX_TXOFF|DP_BLOCK);
	} else if (dp->dp_state & DP_TX_TXOFF) {
		xmit++;
		*obuf++ = dp->dp_stopc;
		dp->dp_state &= ~DP_TX_TXOFF;
		dp->dp_state |= DP_BLOCK;
	}

	wbp = 0;
	while (xmit < xmitlimit) {
		wbp = getq(dp->dp_wq);	/* get another msg */
		if (!wbp) break;

		switch (wbp->b_datap->db_type) {
		case M_DATA:
			i = (wbp->b_wptr - (p=wbp->b_rptr));
			xmit += i;
			if (xmit > xmitlimit) {	/* (fast if obsure code) */
				i -= (xmit - xmitlimit);
				xmit = xmitlimit;
			}
#ifdef SVR3
			hwcpout(p,obuf,i);
			obuf += i;
			p += i;
#else not SVR3
			while (--i >= 0)
				*obuf++ = *p++;
#endif SVR3
			wbp->b_rptr = p;
			if (p >= wbp->b_wptr) {
				register mblk_t *next_wbp;
				ASSERT(wbp->b_datap->db_type == M_DATA);
				next_wbp = rmvb(wbp,wbp);
				freeb(wbp);
				wbp = next_wbp;
			}
			break;

		case M_DELAY:
			if (xmit == 0) {
				dp->dp_state |= DP_TIMEOUT;
				dp->dp_tid = timeout(cdDelay,
						     (caddr_t)dp,
						     *(int*)wbp->b_rptr);
				freemsg(wbp);
				wbp = 0;
			}
			goto exit;

		case M_IOCTL:
			if (xmit != 0)
				goto exit;
			if (cdInterruptIoctl(dp, wbp)) {
				wbp = 0;
				goto exit;
			}
			break;

		default:
#ifdef	SVR0
			printf("ttyd%d: bad msg %d\n",
			       dp->dp_minor, wbp->b_datap->db_type);
			panic("bad streams msg");
#else
			cmn_err(CE_PANIC, "ttyd%d: bad msg %d",
				dp->dp_minor, wbp->b_datap->db_type);
#endif SVR0
		}
	}
exit:;
	dp->dp_xmit = xmit;
#ifdef SVR3
	SYSINFO.outch += xmit;
#endif SVR3
	if (0 != wbp)
		putbq(dp->dp_wq, wbp);
}

/* start output command for several ports
 */
static void
cdCmdOutput(dp)
	register struct cdport *dp;	/* any port on the board */
{
	register int i;
	register unchar mask;
	dp = &cdport[BOARD_TO_EXPORT(DEV_TO_BOARD(dp->dp_minor))];

	for (i = 0, mask = 0; i < CDUPC; i++, dp++) {
		if (dp->dp_xmit != 0
		    && !(dp->dp_state & DP_BUSY)
		    && (cdPoll(dp), !dp->dp_device->outputBusy)) {
			register volatile struct device *d = dp->dp_device;

#ifdef OS_DEBUG
			if (cdsio_noise)
				printf("\tttyd%d:out=%d ",
				       dp->dp_minor, dp->dp_xmit);
#endif
			d->cmd = CMD_SEND_0;
			d->cmdHigh = dp->dp_xmit>>8;
			d->cmdLow = dp->dp_xmit;
			d->cmdStatus = 0xFF;
			mask |= dp->dp_bit;
			dp->dp_xmit = 0;
		}
	}
	if (0 != mask) {
		dp -= CDUPC;
		wbflush();
		dp->dp_iodevice->cmdPort = mask;
		wbflush();
	}
}

/* Start transmitting
 *	should be called with interrupts off
 */
static void
cdStartOutput(dp)
	register struct cdport *dp;
{
	if (!(dp->dp_state & DP_BUSY)
	    && 0 == dp->dp_xmit
	    && (cdPoll(dp), !dp->dp_device->outputBusy)) {
		cdOutput(dp);
		if (0 != dp->dp_xmit)
			cdCmdOutput(dp);

	}
}

/*
 * slow and hopefully infrequently used function to put characters
 *	somewhere where they will go up stream.
 */
static mblk_t *
cdStuff(dp, c)
	register struct cdport *dp;
	unchar c;
{
	register mblk_t *bp;

	if (!(bp = dp->dp_rbp) && !(bp = cdGetNewbp(dp, BPRI_HI))) {
		dp->dp_allocb_fail++;;
		return bp;
	}
	*bp->b_wptr = c;
	if (++bp->b_wptr >= bp->b_datap->db_lim) {
		/* send buffer when full */
		bp = cdGetNewbp(dp, BPRI_HI);
	}
	return bp;
}

/*
 * Handle receive interrupt
 */
static void
cdReceiveInterrupt(dp)
	register struct cdport *dp;
{
	register volatile struct device *d;
	register uint emptyPtr;
	register mblk_t *bp;
	unchar needSend;

#ifdef SVR3
	SYSINFO.rcvint++;		/* count it for sar(1) */
#endif SVR3

	/* must be open and readable, to read */
	if (!(dp->dp_state & DP_ISOPEN) || !(dp->dp_cflag & CREAD)) {
		cdReceiveClear(dp);
		return;
	}

	if (!(bp = dp->dp_rbp))		/* try for a buffer, outside loop */
		bp = cdGetNewbp(dp, BPRI_HI);

	/* handle each input character */
	d = dp->dp_device;
	needSend = 0;
	while (d->inputFillPtr != (emptyPtr = d->inputEmptyPtr)) {
		register unchar c = d->inputBuf[emptyPtr];
		register unchar err = d->errorBuf[emptyPtr];
		emptyPtr++;
		emptyPtr &= (INBUF_LEN-1);
		d->inputEmptyPtr = emptyPtr;
#ifdef SVR3
		SYSINFO.rawch++;
#endif SVR3

		/*
		 * start or stop output (if permitted) when we
		 * get XOFF or XON
		 */
		if (dp->dp_iflag & IXON) {
			register unchar cs = c & 0x7f;

			if ((dp->dp_state & DP_TXSTOP) &&
			    ((cs == dp->dp_startc) ||
			     ((dp->dp_iflag & IXANY) &&
			      ((cs != dp->dp_stopc) ||
			       (dp->dp_line == LDISC0))))) {

				/* restart output, if none pending */
				dp->dp_state &= ~DP_TXSTOP;
				cdStartOutput(dp);
				if (cs == dp->dp_startc)
					continue;
			} else if (dp->dp_state & DP_LIT) {
				dp->dp_state &= ~DP_LIT;
			} else if (cs == dp->dp_stopc) {
				dp->dp_state |= DP_TXSTOP;
				continue;
			} else if (cs == dp->dp_startc) {
				continue;	/* ignore extra control-Qs */
			} else if ((cs == dp->dp_litc)
				   && (LDISC0 != dp->dp_line)) {
				dp->dp_state |= DP_LIT;
			}
		}

		if (err & (ERRB_FRAMING | ERRB_OVERRUN | ERRB_PARITY)) {
			if (err & ERRB_OVERRUN)
				dp->dp_overruns++;

			/* if there was a BREAK	*/
			if ((err & ERRB_FRAMING) && (c == 0)) {
				if (dp->dp_iflag & IGNBRK)
					continue;	/* ignore it if ok */
				if (dp->dp_iflag & BRKINT) {
					(void)putctl1(dp->dp_rq->q_next,
						      M_FLUSH,FLUSHRW);
					(void)putctl1(dp->dp_rq->q_next,
						      M_PCSIG, SIGINT);
					continue;
				}
			} else if (IGNPAR & dp->dp_iflag)
				continue;
			else if (!(INPCK & dp->dp_iflag)) {
				/* ignore input parity errors if asked */
			} else if (err & (ERRB_PARITY|ERRB_FRAMING)) {
				if (err & ERRB_FRAMING)
					dp->dp_framingErrors++;
				if (dp->dp_iflag & PARMRK) {
					(void)cdStuff(dp, 0377);
					bp = cdStuff(dp, 0);
				} else
					c = '\0';
			}
		} else if (dp->dp_iflag & ISTRIP) {
			c &= 0x7f;
		} else if (c == 0377 && (dp->dp_iflag & PARMRK)) {
			bp = cdStuff(dp, 0377);
		}

		/* get a buffer if we have none */
		if (!bp) {
			/* drop character if no buffer available */
			dp->dp_allocb_fail++;
			continue;
		}
		*bp->b_wptr = c;
		if (++bp->b_wptr >= bp->b_datap->db_lim) {
			/* send buffer upstream when full */
			bp = cdGetNewbp(dp, BPRI_HI);
		}
		needSend = 1;
	}

	if (needSend) {
		/* enable read service routine now that some data is ready */
		if ((dp->dp_rq != NULL) && canenable(dp->dp_rq))
			qenable(dp->dp_rq);
	}
}

/*
 * Carrier on or off interrupt
 */
cdStatusInterrupt(dp)
	register struct cdport *dp;
{
#ifdef SVR3
	SYSINFO.mdmint++;		/* count it for sar(1) */
#endif SVR3

	if (cdIsOn(dp)) {
		if (dp->dp_state & DP_WOPEN)	/* DCD-on interrupt */
			wakeup((caddr_t)dp);
	} else if (dp->dp_state & DP_ISOPEN) {
		cdHangup(dp);		/* off interrupt--kill the modem */
		flushq(dp->dp_wq, FLUSHDATA);
		(void)putctl1(dp->dp_rq->q_next,
			      M_FLUSH,FLUSHW);
		(void)putctl(dp->dp_rq->q_next, M_HANGUP);
	}

	cdSetDCDint(dp);
}

#if defined(PM2) || defined(IP2)
int
cdintr()
{
	register int i;
	register int rv;

	rv = 0;
	for (i = 0; i < NCD; i++) {
		if (cdprobed[i])
			rv |= cdsiointr(i);
	}
	return (rv);
}
#endif

int
cdsiointr(board)
	int board;
{
	register struct cdport *dp;
	register int p;
	unchar pisf[CDUPC];
	int retval = 0;
	int needout = 0;

	ASSERT(board >= 0 && board < cdnumboards);
#if defined(PM2) || defined(IP2)
	ASSERT(cdprobed[board] != 0);
#else
	if (cdprobed[board] == 0)	/* interrupt is too early */
		return retval;
#endif

	dp = &cdport[BOARD_TO_EXPORT(board)];
	for (;;) {
		if (dp->dp_device->portIntStatusFlag == 0)
			break;		/* quit if not this board */
		retval = 1;

		for (p = 0; p < CDUPC; dp++, p++)
			pisf[p] = dp->dp_device->intSourceFlag;
		dp -= CDUPC;

		cdPoll(dp);
		dp->dp_device->cmd = 0;
		dp->dp_iodevice->cmdPort = 0;	/* clear interrupt */

		/*
		 * process interrupts for each board
		 */
		for (p = 0; p < CDUPC; dp++, p++) {
			if (dp->dp_device->inputEmptyPtr
			    != dp->dp_device->inputFillPtr)
				cdReceiveInterrupt(dp);
			if (pisf[p] & ISF_STATUS)
				cdStatusInterrupt(dp);
			if (pisf[p] & ISF_OUTPUT) {
#ifdef SVR3
				SYSINFO.xmtint++;	/* count for sar(1) */
#endif SVR3
				if (!(dp->dp_state & DP_BUSY)
				    && (cdPoll(dp),
					!dp->dp_device->outputBusy)) {
					cdOutput(dp);
					needout |= dp->dp_xmit;
				}
			}
		}
		dp -= CDUPC;
	}

	if (needout)			/* do delayed output */
		cdCmdOutput(dp);

	return (retval);
}

#if defined(PM2) || defined(IP2)
/*
 * Probe for the board at "reg"
 */
static int
cdProbe(reg)
	long reg;
{
	register int i;
	register struct cdport *dp;
	register volatile struct device *d;
	register volatile struct iodevice *iod;
	register int board;
	register long timo;
	static char nextBoard = 0;

	board = nextBoard++;
	iod = (struct iodevice *) (MBIO_VBASE + reg ^ 1);
#ifdef	PM2
	d = (struct device *)(SEG_MBMEM + CDMEMBASE + board*CDMEMSIZE);
#endif
#ifdef	IP2
	d = (struct device *)(SEG_MBMEM + CDMEMBASE + board*CDMEMSIZE);
#endif

	iod->cmdPort = 0;			/* poke */

	/*
	 * If we get here, the board probed.  Wait for board to come up
	 * to a clean state
	 */
	timo = PROBE_TIME/100;
	while (((iod->hostStatusPort & HSP_READY) == 0) && --timo) {
		DELAY(100);
	}
	if (timo == 0) {
		printf("cdsio%d: board reset timed out, err=%s (0x%x)\n",
		       board, cdErrorMsg(d->outputBuf0[0]),
		       d->outputBuf0[0]);
		return (CONF_DEAD);
	}

	dp = &cdport[BOARD_TO_EXPORT(board)];
	for (i = 0; i < CDUPC; i++, dp++) {
		dp->dp_minor = BOARD_TO_MINOR(board,i);
		dp->dp_bit = 1 << EXPORT_TO_BDPORT(i);
		dp->dp_device = d + EXPORT_TO_BDPORT(i);
		dp->dp_iodevice = iod;
		cdHangup(dp);		/* reset the port */
	}
	dp -= CDUPC;

	/* program interrupt vectors and levels */
	cdCmd(dp, CMD_SET_INPUT, CDIPL, 0);
	cdCmd(dp, CMD_SET_OUTPUT, CDIPL, 0);
	cdCmd(dp, CMD_SET_STATUS, CDIPL, 0);
	cdCmd(dp, CMD_SET_PARITY, CDIPL, 0);

	cdPoll(dp);			/* clear early interrupt */
	iod->cmdPort = 0;
	wbflush();

	cdprobed[board] = 1;

	printf("(firmware revision %d) ", d->firmwareRev);
	return (CONF_ALIVE);
}
#endif

#ifdef	SVR3
/*
 * Probe the given central data board
 */
cdsioedtinit(e)
	struct edt *e;
{
	register struct cdport *dp;
	register volatile struct device *d;
	register volatile struct iodevice *iod;
	register int i;
	register int ipl, vec;
	register int board;
	register long timo;

	board = e->e_intr_info->v_unit;
	d = (volatile struct device *) e->e_base;
	iod = (volatile struct iodevice *) d;
	/*
	 * Probe board by writing a zero.  This also has the side effect
	 * of stopping the board diagnostics.
	 */
	if (wbadaddr(&iod->cmdPort, sizeof(iod->cmdPort))) {
		printf("cdsio%d: missing\n", board);
		return;
	}

	/* wait for board to come up to a clean state */
	timo = PROBE_TIME/100;
	while (((iod->hostStatusPort & HSP_READY) == 0) && --timo) {
		DELAY(100);
	}
	if (timo == 0) {
		printf("cdsio%d: board reset timed out, err=%s (0x%x)\n",
		       board, cdErrorMsg(d->outputBuf0[0]),
		       d->outputBuf0[0]);
		return;
	}

	/* initialize data structures */
	dp = &cdport[BOARD_TO_EXPORT(board)];
	for (i = 0; i < CDUPC; i++, dp++) {
		dp->dp_minor = BOARD_TO_MINOR(board,i);
		dp->dp_bit = 1 << EXPORT_TO_BDPORT(i);
		dp->dp_device = d + EXPORT_TO_BDPORT(i);
		dp->dp_iodevice = iod;
		cdHangup(dp);		/* reset the port */
	}
	dp -= CDUPC;

	/* program interrupt vectors and levels */
	vec = e->e_intr_info->v_vec;
	ipl = e->e_intr_info->v_brl | INT_ENABLE | INT_ROAK;
	cdCmd(dp, CMD_SET_INPUT, ipl, vec);
	cdCmd(dp, CMD_SET_OUTPUT, ipl, vec);
	cdCmd(dp, CMD_SET_STATUS, ipl, vec);
	cdCmd(dp, CMD_SET_PARITY, ipl, vec);

	cdPoll(dp);			/* clear early interrupt */
	iod->cmdPort = 0;
	wbflush();

	cdprobed[board] = 1;

	if (showconfig)
		printf("cdsio%d: firmware revision %d, ipl %d, vec 0x%x\n",
		       board, d->firmwareRev, e->e_intr_info->v_brl, vec);
}
#endif SVR3
#endif	/* NCD==0 */
