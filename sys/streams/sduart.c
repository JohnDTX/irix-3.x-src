/* Copyright 1986, Silicon Graphics Inc., Mountain View, CA. */
/* streams driver for signetics SCN2681 (motorola SCN68681) dual
 *	asynchronous receiver/transmitters.
 *
 * $Source: /d2/3.7/src/sys/streams/RCS/sduart.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:34:45 $
 */

#ifdef SVR3
#include "sys/sbd.h"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/signal.h"
#include "sys/pcb.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/fs/s5dir.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/termio.h"
#include "sys/file.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/strids.h"
#include "sys/stty_ld.h"
#include "sys/debug.h"
#include "sys/sysinfo.h"
#include "sys/ksa.h"
#include "sys/pda.h"

#define UNTIMEOUT_ID untimeout
#define WBFLUSH() wbflush()
#ifndef KOPT_NOGL
#define MIPS_GL
#endif
#define OS_CDEBUG

#else not SVR3
#include "../h/param.h"
#include "../h/systm.h"
#include "machine/cpureg.h"
#include "../h/user.h"
#include "../h/termio.h"
#include "../h/file.h"
#include "../streams/stream.h"
#include "../streams/stropts.h"
#include "../streams/strids.h"
#include "../streams/stty_ld.h"
#include "../streams/strcomp.h"

#define UNTIMEOUT_ID untimeout_id
#define WBFLUSH()

#ifndef	KOPT_NOGL
#define	IP2_GL
#endif

#endif not SVR3

#ifdef OS_CDEBUG
extern short kdebug;
#endif

#ifdef IP2
extern char rev_A;			/* !=0 on revision-A IP2s */
#endif
#ifdef PM2
static char sduart_cdpolling = 0;	/* !=0 when CD timer running */
static char sduart_cdpolled = 0;	/* !=0 after CD polled */
#endif


#define	CARRPRI	STIPRI			/* sleep for carrier at this */


extern struct stty_ld def_stty_ld;

/* default cflags */
#define DEF_CFLAG ((ushort)(CREAD | def_stty_ld.st_cflag))


/* stream stuff */

static struct module_info dum_info = {
	STRID_DUART,			/* module ID */
	"DUART",			/* module name */
	0,				/* minimum packet size */
	INFPSZ,				/* maximum packet size--infinite */
	128,				/* hi-water mark */
	16,				/* lo-water mark */
};

#define MIN_RMSG_LEN 16			/* minimum buffer size */
#define MAX_RMSG_LEN 2048		/* largest msg allowed */
#define XOFF_RMSG_LEN 256		/* send XOFF here */
#define MAX_RBUF_LEN 1024

#define MAX_RSRV_CNT 3			/* continue input timer this long */
#define RSRV_DURATION (HZ/50)		/* send input this often */

static int du_open();
static du_rsrv(), du_close();
static struct qinit du_rinit = {
	NULL, du_rsrv, du_open, du_close, NULL, &dum_info, NULL
};

static du_wput();
static struct qinit du_winit = {
	du_wput, NULL, NULL, NULL, NULL, &dum_info, NULL
};

struct streamtab duinfo = {&du_rinit, &du_winit, NULL, NULL};


/* maximum # of duarts we have */
#if defined(PM2) || defined(IP2) || defined(R2300)
#define	NUMDUARTS	2
#endif
#ifdef IP4
#define	NUMDUARTS	3
#endif

#define LN_DUARTPORTS	1		/* ports/DUART */
#define DUARTPORTS	(1<<LN_DUARTPORTS)
/* This is assumed to be (2**n)-1 on 68000 systems */
#define	MAXPORT	   (NUMDUARTS*DUARTPORTS-1)	/* maximum port #. */



/* MIPS CPU boards are rather confused:
 * duart port 0= back panel mouse port = minor #31= SIO_1 connector
 *   "    "   1=  "    "    port 2     =   "   #2 = SIO_2 connector = pdbx port
 *   "    "   2=  "    "    keybd port =   "   #30
 *   "    "   3=  "    "    port 1     =   "   #1 = secondary console
 */
#ifdef R2300
#ifdef MULTIPROC
#define SCNDCPORT	0
#define PDBXPORT	1
#else
#define	SCNDCPORT	3		/* port for the secondary console */
#define PDBXPORT	1		/* XXX--kernel debugging port */
#endif
#define NUMMINOR	32		/* must be (2**n) */
#define PORT(dev)	port_map[(dev) & (NUMMINOR-1)]
#define MODEM(dev)	((dev) & (NUMMINOR*3))
#define FLOW_MODEM(dev)	((dev) & (NUMMINOR*2))
static u_char port_map[NUMMINOR] = {
#ifdef MULTIPROC
	SCNDCPORT,SCNDCPORT, PDBXPORT,255, 255,255,255,255,
#else
	255,SCNDCPORT, PDBXPORT,255, 255,255,255,255,
#endif
	255,255,255,255, 255,255,255,255,
	255,255,255,255, 255,255,255,255,
	255,255,255,255, 255,255,  2,  0,
};
#endif R2300

/* IP4 CPU boards are simple:
 * duart port 0= back panel keybd port  = minor #30
 *   "    "   1=   "    "   mouse port  =  "    #31
 *   "    "   2=   "    "   port 1      =  "    #1
 *   "    "   3=   "    "    "   2      =  "    #2
 *   "    "   4=   "    "    "   3      =  "    #3
 *   "    "   5=   "    "    "   4      =  "    #4
 */
#ifdef IP4
#define	SCNDCPORT	2		/* port for the secondary console */
#define PDBXPORT	3		/* XXX--kernel debugging port */
#define NUMMINOR	32		/* must be (2**n) */
#define PORT(dev)	port_map[(dev) & (NUMMINOR-1)]
#define MODEM(dev)	((dev) & (NUMMINOR*3))
#define FLOW_MODEM(dev)	((dev) & (NUMMINOR*2))
static u_char port_map[NUMMINOR] = {
	255,  2,  3,  4,   5,255,255,255,
	255,255,255,255, 255,255,255,255,
	255,255,255,255, 255,255,255,255,
	255,255,255,255, 255,255,  0,  1,
};
#endif

#if !defined(IP4) && !defined(R2300)	/* 68000s */
#define	SCNDCPORT	1		/* port for the secondary console */
#define	GFXKEYPORT	0		/* port for the graphics keyboard */
#define	DIALBPORT	3		/* port for dial box use	 */
#define PORT(dev)	((ushort)((dev) & 0x1f))
#define MODEM(dev)	((dev) & 0x80)
#define FLOW_MODEM(dev)	((dev) & 0x40)
#endif



/*
 * Each DUART has the following addresses:
 *	These two structures define the device addresses of a single DUART
 *	port.  Each port has the address of one of each.  However, each pair
 *	of ports on a single DUART have identical 'chip' address.
 *
 *	The machines differ in how they decode the chip addresses.
 */
#undef ADDRP
#undef CAT
#define CAT(str) str
#ifdef IP2
#define ADDRP(lab,n) u_char lab
#endif
#ifdef PM2
#define ADDRP(lab,n) u_char lab, CAT(pad)n
#endif
#ifdef R2300
#define ADDRP(lab,n) u_char lab, CAT(padA)n,CAT(padB)n,CAT(padC)n
#endif
#ifdef IP4
#define ADDRP(lab,n) u_char lab, CAT(pad)n[15]
#endif
#ifdef mips
#define VOL volatile
#else
#define VOL
#endif
typedef VOL struct {			/* *** Port Address ***		*/
	ADDRP(pa_mr,0);			/* Mode Register		*/
	ADDRP(pa_sr,1);			/* Status Register		*/
	ADDRP(pa_cr,2);			/* Command Register		*/
	ADDRP(pa_rhr,3);		/* RX Holding Reg		*/
} PAD;
#define	pa_csr	pa_sr			/* Clock Select Register	*/
#define	pa_thr	pa_rhr			/* TX Holding Register		*/


typedef VOL struct {			/* *** Chip Address ***		*/
	ADDRP(ca_mra,0);		/* Mode Register A		*/
	ADDRP(ca_sra,1);		/* Status Register A		*/
	ADDRP(ca_cra,2);		/* Command Register A		*/
	ADDRP(ca_rhra,3);		/* RX Holding Reg A		*/
	ADDRP(ca_ipcr,4);		/* Input Port Change Reg	*/
	ADDRP(ca_isr,5);		/* Interrupt Status Reg		*/
	ADDRP(ca_ctu,6);		/* Counter/Timer Upper		*/
	ADDRP(ca_ctl,7);		/* Counter/Timer Lower		*/

	ADDRP(ca_mrb,8);		/* Mode Register B		*/
	ADDRP(ca_srb,9);		/* Status Register B		*/
	ADDRP(ca_crb,10);		/* Command Register B		*/
	ADDRP(ca_rhrb,11);		/* RX Holding Reg B		*/
	ADDRP(ca_x1,12);		/* Reserved (interrupt vector)	*/
	ADDRP(ca_iport,13);		/* Input Port			*/
	ADDRP(ca_start,14);		/* Start Counter Command	*/
	ADDRP(ca_stop,15);		/* Stop Counter Command		*/
} CAD;
#define	ca_acr	ca_ipcr			/* Aux Control Register		*/
#define	ca_imr	ca_isr			/* Interrupt Mask Register	*/
#define	ca_ctur	ca_ctu			/* C/T Upper Register		*/
#define	ca_ctlr	ca_ctl			/* C/T Lower Register		*/
#define	ca_opcr	ca_iport		/* Output Port Conf. Reg	*/
#define	ca_sopbc ca_start		/* Set Output Port Bits Command */
#define	ca_ropbc ca_stop		/* Reset Output Port Bits Command*/
#undef ADDRP
#undef CAT


#ifdef IP2
#define	DUINCR	0x8			/* Offset to B port		*/
#endif
#ifdef PM2
#define	DUINCR	0x10
#endif
#ifdef R2300
#define DUINCR	0x20
#endif
#ifdef IP4
#define DUINCR	0x80
#endif

/* some bit definitions for the duart */

/* mr1 */
#define	MR1_ODDPARITY	0x04		/* odd parity	*/
#define	MR1_NOPARITY	0x10		/* no parity	*/

/* mr2 */
#define	MR2_1STOP	0x07		/* 1 stop bit	  */
#define	MR2_2STOP	0x0F		/* 2 stop bits	  */
#define	MR2_CTS		0x10		/* CTS enable */

/* sr */
#define	SR_RXREADY	0x01		/* receiver ready	*/
#define	SR_TXREADY	0x04		/* xmit ready		*/
#define	SR_PERROR	0x20		/* parity error		*/
#define	SR_FRERROR	0x40		/* frame error		*/
#define	SR_OVERRUN	0x10		/* overrun		*/
#define	SR_RBREAK	0x80		/* break received	*/

/* cr */
#define	CR_RESETMR	0x10		/* reset MR pointer to be MR1	*/
#define CR_DISABLE_TX	0x08		/* disable tx			*/
#define CR_DISABLE_RX	0x02		/* disable rx			*/
#define CR_ENABLE_TX	0x04		/* enable tx			*/
#define CR_ENABLE_RX	0x01		/* enable tx			*/
#define	CR_RXRESET	0x20		/* reset receiver		*/
#define	CR_TXRESET	0x30		/* reset transmitter		*/
#define	CR_ERRCLR	0x40		/* clear error bits		*/
#define	CR_STARTBREAK	0x60		/* start a break		*/
#define	CR_STOPBREAK	0x70		/* stop a break			*/

/* opcr */
#ifdef PM2
#define	OPCR_INIT0	0x04		/* enable refresh timer output	*/
#define	OPCR_INIT1	0x00		/* no enables			*/
#endif
#ifdef IP2
#define	OPCR_INIT0	0x04		/* enable refresh timer output	*/
#define	OPCR_INIT1	0x00		/* no enables			*/
#define	OPCR_INIT1_REVA	0x04		/* bus errors on rev A IP2	*/
#endif
#ifdef R2300
#define OPCR_INIT0	0x00		/* no output enables		*/
#define OPCR_INIT1	0x00
#endif
#ifdef IP4
#define OPCR_INIT0	0x00		/* no output enables for mouse	*/
#define OPCR_INIT1	0x04		/* refresh timer		*/
#define OPCR_INIT2	0x04		/* 38.4K baud clock		*/
#endif

/* acr */
#ifdef PM2
#define	ACR_RATE1	30720		/* 60hz, that is		  */
#define	ACR_SETUP	0xE8		/* baud set 2, timer, extern clock*/
#endif
#ifdef IP2
#define	ACR_RATE1	3		/* 38.4K baud rate generator	  */
#define ACR_RATE1_REVA	30720		/* bus error generator on rev A.  */
#define	ACR_SETUP	0xEC		/* baud set 2, timer, extern clock*/
#endif
#ifdef R2300
#define ACR_SETUP	0xEC		/* baud set 2, timer, extern clock*/
#endif
#ifdef IP4
#define ACR_RATE2	3		/* 38.4K baud rate generator	  */
#define ACR_SETUP	0xEC		/* baud set 2, timer, extern clock*/
#endif

/* isr */
#define	ISR_TXA		0x01		/* port a transmitter is ready	*/
#define	ISR_TXB		0x10		/* port b transmitter is ready	*/
#define	ISR_RXA		0x02		/* port a reciever is ready	*/
#define	ISR_RXB		0x20		/* port b reciever is ready	*/
#define	ISR_DCD		0x80		/* enable carrier detect ints	*/

/* imr */
#define	IMR_DCD		0x80		/* enable carrier detect ints	  */
#define	IMR_COUNTER	0x08		/* enable counter ints		  */
#define	IMR_TXA		ISR_TXA		/* enable port a transmitter ints */
#define	IMR_TXB		ISR_TXB		/* enable port b transmitter ints */
#define	IMR_RXA		ISR_RXA		/* enable port a reciever ints	  */
#define	IMR_RXB		ISR_RXB		/* enable port b reciever ints	  */

/* iport */
#if defined(IP2) || defined(IP4)
#define	IPORT_DCDA	0x08		/* dcd input bit for A ports	*/
#define	IPORT_DCDB	0x04		/* dcd input bit for B ports	*/
#endif
#ifdef PM2
#define	IPORT_DCDA	0x08		/* dcd input bit for A ports	*/
#define	IPORT_DCDB	0x20		/* this is a bug in the PM2	*/
#endif
#ifdef R2300
#define	IPORT_DCDA	0x00		/* dcd input for A ports--not used */
#define	IPORT_DCDB	0x04		/* dcd input bit for B ports	*/
#endif

/* oport (used with ca_ropbc and ca_sopbc) */
#ifdef R2300
#define	OPORT_RTSA	0x00		/* rts for port a--absent for MIPS */
#define	OPORT_RTSB	0x02		/* rts for port b */
#define	OPORT_DTRA	0x00		/* dtr for port a--absent for MIPS */
#define	OPORT_DTRB	0x01		/* dtr for port b */
#else
#define	OPORT_RTSA	0x01		/* rts for port a */
#define	OPORT_RTSB	0x02		/* rts for port b */
#define	OPORT_DTRA	0x10		/* dtr for port a */
#define	OPORT_DTRB	0x20		/* dtr for port b */
#endif

/* addresses of the chips */
#ifdef IP2
#define DUART0 DUART0_BASE
#define DUART1 DUART1_BASE
#endif
#ifdef PM2
#define DUART0 DUART0_VBASE
#define DUART1 DUART1_VBASE
#endif
#ifdef R2300
#define DUART0 (0x1e008002+K1BASE)
#define DUART1 (0x1e800000+K1BASE)
#endif
#ifdef IP4
#define DUART0 (0x1fb80000+K1BASE)
#define DUART1 (0x1fb80004+K1BASE)
#define DUART2 (0x1fb80008+K1BASE)
#endif

/*
 * duart bit patterns for various speeds
 */
#define BAUDBAD		0x01
#define	BAUD75		0x00
#define	BAUD110		0x11
#define	BAUD134		0x22
#define	BAUD150		0x33
#define	BAUD300		0x44
#define	BAUD600		0x55
#define	BAUD1200	0x66
#define	BAUD1800	0xAA
#define	BAUD2400	0x88
#define	BAUD4800	0x99
#define	BAUD9600	0xBB
#define	BAUD19200	0xCC
#define	BAUD38400	0XDD

#if defined(IP2) || defined(IP4)
static u_char du_speeds_38[16] = {
	BAUDBAD,	BAUDBAD,	BAUD75,		BAUD110,
	BAUD134,	BAUD150,	BAUDBAD,	BAUD300,
	BAUD600,	BAUD1200,	BAUD1800,	BAUD2400,
	BAUD4800,	BAUD9600,	BAUD19200,	BAUD38400
};
#endif
#if defined(PM2) || defined(R2300)
#define du_speeds_38 du_speeds_no38	/* no 38kb for some */
#endif
static u_char du_speeds_no38[16] = {
	BAUDBAD,	BAUDBAD,	BAUD75,		BAUD110,
	BAUD134,	BAUD150,	BAUDBAD,	BAUD300,
	BAUD600,	BAUD1200,	BAUD1800,	BAUD2400,
	BAUD4800,	BAUD9600,	BAUD19200,	BAUDBAD
};



static u_char duart_imr[NUMDUARTS];	/* shadow of IMR register on chip */

/*
 * each port has a data structure that looks like this
 */
struct dport{
	CAD	*dp_ca;			/* chip device addresses	    */
	PAD	*dp_pa;			/* port device addresses	    */

	u_char	*dp_imrp;		/* pointer to shadow IMR register   */
	u_char  *dp_speed;		/* speed table			    */
	u_char	dp_tximr;		/* transmitter-ready interrupt bit  */
	u_char	dp_rximr;		/* receiver-ready interrupt bit	    */

	u_char	dp_rts;			/* RTS bit for the port		    */
	u_char	dp_dtr_rts;		/* RTS & DTR bits for the port	    */
	u_char	dp_dcd;			/* input mask for carrier (dcd)     */

	u_char	dp_litc;		/* escape next char--e.g. XOFF	    */
	u_char	dp_stopc;		/* output XOFF character	    */
	u_char	dp_startc;		/* output XON character		    */
	struct termio dp_termio;
#define dp_iflag dp_termio.c_iflag	/* use some of the bits (see below) */
#define dp_cflag dp_termio.c_cflag	/* use all of the standard bits	    */
#define dp_line dp_termio.c_line	/* 'line discipline'		    */

	u_char	dp_index;		/* port number (0 to MAXPORT)	    */

	u_char	dp_rsrv_cnt;		/* input timer count		    */

	ushort dp_state;		/* current state		    */

	queue_t	*dp_rq, *dp_wq;		/* our queues			    */
	mblk_t	*dp_rmsg, *dp_rmsge;	/* current input message	    */
	int	dp_rmsg_len;
	int	dp_rbsize;		/* resent msg length		    */
	mblk_t	*dp_rbp;		/* current buffer		    */
	mblk_t	*dp_wbp;		/* current output buffer	    */

	int	dp_tid;			/* (recent) output delay timer ID   */

	int	dp_fe, dp_over;		/* framing & overrun error counts   */
	int	dp_allocb_fail;		/* losses due to allocb() failures  */
};


/* bits in dp_state */
#define DP_ISOPEN	0x0001		/* device is open		*/
#define DP_WOPEN	0x0002		/* waiting for carrier		*/
#define DP_DCD		0x0004		/* we have carrier		*/
#define DP_TIMEOUT	0x0008		/* delaying			*/
#define DP_BREAK	0x0010		/* breaking			*/
#define DP_BREAK_QUIET	0x0020		/* finishing break		*/
#define DP_TXSTOP	0x0040		/* output stopped by received XOFF */
#define DP_LIT		0x0080		/* have seen literal character	*/
#define DP_BLOCK	0x0100		/* XOFF sent because input full */
#define DP_TX_TXON	0x0200		/* need to send XON		*/
#define DP_TX_TXOFF	0x0400		/* need to send XOFF		*/

#define DP_FLOW		0x4000		/* do hardware flow control	*/


/* define a single port */
#define DEFPORT(ca,ab,pn,rts,dtr,dcd,sp) {(CAD*)(ca), (PAD*)(ca+DUINCR*(ab)), \
	&duart_imr[(pn)/DUARTPORTS], &sp[0], \
	IMR_TXA<<((ab)*4), IMR_RXA<<((ab)*4),  \
	rts, rts|dtr, dcd, \
	CLNEXT, CSTOP, CSTART, {0}, pn}

/* define the pair of ports on a single chip */
#define DEFCHIP(ca,pn,sp) \
	DEFPORT(ca,0,pn*DUARTPORTS,  OPORT_RTSA,OPORT_DTRA,IPORT_DCDA,sp), \
	DEFPORT(ca,1,pn*DUARTPORTS+1,OPORT_RTSB,OPORT_DTRB,IPORT_DCDB,sp)

struct dport dports[] = {
	DEFCHIP(DUART0, 0, du_speeds_no38),
	DEFCHIP(DUART1, 1, du_speeds_38),
#ifdef IP4
	DEFCHIP(DUART2, 2, du_speeds_38),
#endif
};


/* given one port, find the other one on the same chip */
#define OTHER(dp) (((dp)->dp_index & 1) ? ((dp)-1) : ((dp)+1))


/* turn off TX interrupts
 *	Interrupts should be made safe before coming here.
 */
static
du_stop(dp)
register struct dport *dp;
{
	register u_char imr;

	imr = (*dp->dp_imrp & ~dp->dp_tximr);
	*dp->dp_imrp = imr;
	dp->dp_ca->ca_imr = imr;
	WBFLUSH();
}



/* start the output
 *	Call here only with interrupts safe.
 */
static
du_start(dp)
register struct dport *dp;
{
	register u_char imr;

	imr = *dp->dp_imrp;
	if (!(dp->dp_state & (DP_TIMEOUT|DP_BREAK|DP_BREAK_QUIET))
	    && !(imr & dp->dp_tximr)) {
		imr |= dp->dp_tximr;
		dp->dp_ca->ca_imr = imr;
		*dp->dp_imrp = imr;
		WBFLUSH();

		du_tx(dp);		/* send something */
	}
}



/* gobble any waiting input
 *	this should be called only when safe from interrupts
 */
static
du_rclr(dp)
register struct dport *dp;
{
	register PAD *pa = dp->dp_pa;
	register u_char c;

	while (pa->pa_sr & SR_RXREADY) {
		c = pa->pa_rhr;
#ifdef OS_CDEBUG
		/* if we get a control-A, debug on this port */
		if (kdebug && dp->dp_index == SCNDCPORT && '\001' == c)
			debug("ring");
#endif
#ifdef lint
		c = c;
#endif
	}

	pa->pa_cr = CR_ERRCLR;		/* clear any errors */
	WBFLUSH();
}



/* activate a port
 *	this should be called only when safe from interrupts
 */
static int				/* return !=0 if carrier present */
du_act(dp)
register struct dport *dp;
{
	register u_char imr, o_imr;

	o_imr = *dp->dp_imrp;
	imr = o_imr | dp->dp_rximr;
	if (!(dp->dp_cflag & CLOCAL))	/* watch carrier-detect only if */
		imr |= IMR_DCD;		/* this is a modem port */
	*dp->dp_imrp = imr;
	dp->dp_ca->ca_imr = imr;

	dp->dp_ca->ca_sopbc = dp->dp_dtr_rts;

	if (!(o_imr & dp->dp_rximr))	/* if now enabling input, */
		du_rclr(dp);		/* gobble old stuff */

	if (dp->dp_ca->ca_iport & dp->dp_dcd) {	/* the input is low-true */
		dp->dp_state &= ~DP_DCD;
		return 0;
	} else {
		dp->dp_state |= DP_DCD;
		return 1;
	}
}



/* shut down a port
 *	this should be called only when safe from interrupts
 */
static
du_zap(dp,nohup)
register struct dport *dp;
int nohup;				/* 0=drop DTR; !=0 to not drop */
{
	register u_char imr;
	register struct dport *o_dp;

	du_flushw(dp);			/* forget pending output */

	imr = *dp->dp_imrp;		/* turn off input interrupt */
#ifdef OS_CDEBUG
	if (!kdebug || dp->dp_index != SCNDCPORT)
#endif OS_CDEBUG
		imr &= ~dp->dp_rximr;

	if (!nohup) {
		o_dp = OTHER(dp);	/* watch carrier-detect only if */
		if ((o_dp->dp_cflag & CLOCAL)	/* other port is open & cares */
		    || !(o_dp->dp_cflag & (DP_ISOPEN|DP_WOPEN)))
			imr &= ~IMR_DCD;
		dp->dp_ca->ca_ropbc = dp->dp_dtr_rts;
	}

	*dp->dp_imrp = imr;
	du_stop(dp);
	du_rclr(dp);
}



/* finish a delay
 */
static
du_delay(dp)
register struct dport *dp;
{
	register int s;

	s = spltty();
	if ((dp->dp_state & (DP_BREAK|DP_BREAK_QUIET)) != DP_BREAK) {
		dp->dp_state &= ~(DP_TIMEOUT|DP_BREAK|DP_BREAK_QUIET);
		du_start(dp);		/* resume output */

	} else {			/* unless need to quiet break */
		dp->dp_pa->pa_cr = CR_STOPBREAK;
		dp->dp_state |= DP_BREAK_QUIET;
		dp->dp_tid = timeout(du_delay, (caddr_t)dp, HZ/20);
	}
	splx(s);
}



/* heartbeat to send input up stream
 *	This is used to reduced the large cost of trying to send each,
 *	individual input byte upstream by itself.
 */
static
du_rsrv_timer(dp)
register struct dport *dp;
{
	register int s;

	s = spltty();
	if (--dp->dp_rsrv_cnt != 0) {
		(void)timeout(du_rsrv_timer, (caddr_t)dp, RSRV_DURATION);
	}
	splx(s);

	if ((dp->dp_state & DP_ISOPEN)	/* quit if we are dead */
	    && 0 != dp->dp_rq
	    && canenable(dp->dp_rq))
		qenable(dp->dp_rq);
}



/* flush output
 *	Interrupts must have been made safe here.
 */
static
du_flushw(dp)
register struct dport *dp;
{
	if ((dp->dp_state & DP_TIMEOUT) == DP_TIMEOUT) {
		UNTIMEOUT_ID(dp->dp_tid);	/* forget stray timeout */
		dp->dp_state &= ~DP_TIMEOUT;
	}

	freemsg(dp->dp_wbp);
	dp->dp_wbp = NULL;
}



/* flush input
 *	interrupts must be safe here
 */
static
du_flushr(dp)
register struct dport *dp;
{
	freemsg(dp->dp_rmsg);
	dp->dp_rmsg = NULL;
	dp->dp_rmsg_len = 0;
	freemsg(dp->dp_rbp);
	dp->dp_rbp = NULL;

	qenable(dp->dp_rq);		/* turn input back on */
}



/* save a message on our write queue,
 *	and start the output interrupt, if necessary
 *
 *	We must be safe from interrupts here.
 */
static
du_save(dp, wq, bp)
register struct dport *dp;
queue_t *wq;
mblk_t *bp;
{
	putq(wq,bp);			/* save the message */

	if (!(*dp->dp_imrp & dp->dp_tximr))
		du_start(dp);		/* start TX only if we must */
}


/* get current tty parameters
 */
tcgeta(wq,bp, p)
queue_t *wq;
register mblk_t *bp;
struct termio *p;
{

	ASSERT(((struct iocblk*)bp->b_rptr)->ioc_count==sizeof(struct termio));

	*STERMIO(bp) = *p;

	bp->b_datap->db_type = M_IOCACK;
	qreply(wq,bp);
}


/* set parameters
 */
static int				/* 0=bad IOCTL */
du_tcset(dp,bp)
register struct dport *dp;
register mblk_t *bp;
{
	register struct iocblk *iocp;
	register struct termio *tp;
	register uint cflag;
	register int baud;

	iocp = (struct iocblk*)bp->b_rptr;
	tp = STERMIO(bp);

	cflag = tp->c_cflag;
	cflag &= ~CLOCAL;
	if (dp->dp_cflag & CLOCAL)
		cflag |= CLOCAL;
	baud = (cflag & CBAUD);

	if (0 != baud
	    && BAUDBAD == dp->dp_speed[baud]) {
		bp->b_datap->db_type = M_IOCNAK;
		return 0;
	}

	du_cont(dp, cflag, tp);
	tp->c_cflag = dp->dp_cflag;	/* tell line discipline the results */

	iocp->ioc_count = 0;
	bp->b_datap->db_type = M_IOCACK;
	return 1;
}


/* interrupt-process an IOCTL
 *	This function processes those IOCTLs that must be done by the output
 *	interrupt.
 */
static
du_i_ioctl(dp,bp)
register struct dport *dp;
register mblk_t *bp;
{
	register struct iocblk *iocp;

	iocp = (struct iocblk*)bp->b_rptr;

	switch (iocp->ioc_cmd) {
	case TCSBRK:
		if (0 == *(int*)bp->b_cont->b_rptr) {
			dp->dp_state |= (DP_TIMEOUT|DP_BREAK);
			dp->dp_pa->pa_cr = CR_STARTBREAK;
			dp->dp_tid = timeout(du_delay, (caddr_t)dp, HZ/4);
		}
		iocp->ioc_count = 0;
		bp->b_datap->db_type = M_IOCACK;
		break;

	case TCXONC:
		switch (*(int*)(bp->b_cont->b_rptr)) {
		case 0:			/* stop output */
			dp->dp_state |= DP_TXSTOP;
			du_stop(dp);
			break;
		case 1:			/* resume output */
			dp->dp_state &= ~DP_TXSTOP;
			du_start(dp);
			break;
		case 2:
			if (DP_FLOW & dp->dp_state)
				dp->dp_ca->ca_ropbc = dp->dp_rts;
			if (!(dp->dp_state & DP_BLOCK)) {
				dp->dp_state |= DP_TX_TXOFF;
				dp->dp_state &= ~DP_TX_TXON;
				du_start(dp);
			}
			break;
		case 3:
			dp->dp_ca->ca_sopbc = dp->dp_rts;
			if (dp->dp_state & DP_BLOCK) {
				dp->dp_state |= DP_TX_TXON;
				du_start(dp);
			}
			break;
		default:
			iocp->ioc_error = EINVAL;
			break;
		}
		bp->b_datap->db_type = M_IOCACK;
		iocp->ioc_count = 0;
		break;

	case TCSETAF:
		if (du_tcset(dp,bp))
			(void)putctl1(dp->dp_rq->q_next, M_FLUSH, FLUSHR);
		break;

	case TCSETA:
	case TCSETAW:
		(void)du_tcset(dp,bp);
		break;

	default:
		ASSERT(0);
	}

	putnext(dp->dp_rq,bp);
}



/* set the duart parameters.
 */
static
du_cont(dp,cflag,tp)
register struct dport *dp;
register uint cflag;			/* new control flags */
struct termio *tp;
{
	register u_char mr1, mr2;
	register uint diff;
	register int s;

	s = spltty();
	diff = cflag ^ dp->dp_cflag;

	if (diff &= (CBAUD|CSIZE|CSTOPB|PARENB|PARODD)) {
		if (B0 == (cflag & CBAUD)) {	/* hang up line if asked */
			du_zap(dp,0);

		} else {
			register PAD *pa = dp->dp_pa;

			mr2 = ((cflag & CSTOPB) ? MR2_2STOP : MR2_1STOP);
			if (dp->dp_state & DP_FLOW)
				mr2 |= MR2_CTS;

			mr1 = (cflag & CSIZE) >> 4;	/* & # of data bits */

			if (!(cflag & PARENB))
				mr1 |= MR1_NOPARITY;	/* no parity */
			else if (cflag & PARODD)	/* or even parity */
				mr1 |= MR1_ODDPARITY;	/* or odd parity */

			pa->pa_cr = CR_RXRESET;
			pa->pa_cr = CR_TXRESET;
			pa->pa_cr = CR_RESETMR;	/* point to mr1 */
			pa->pa_mr = mr1;
			pa->pa_mr = mr2;
			pa->pa_csr = dp->dp_speed[cflag & CBAUD];

			pa->pa_cr = CR_ERRCLR;
			pa->pa_cr = CR_ENABLE_TX|CR_ENABLE_RX;

			(void)du_act(dp);	/* turn line (back) on */
		}
	}

	if (tp)
		dp->dp_termio = *tp;
	dp->dp_cflag = cflag;
	splx(s);
}



/* initialize the duart ports
 *	This is done to allow debugging soon after booting.
 */
#ifdef SVR3
du_init()
#else
duinit()
#endif
{
	register struct dport *dp;
	register CAD *ca;
	u_char dummy;
#ifdef PDBXPORT
	extern char enable_pdbx_duart;
#endif PDBXPORT

#ifdef SVR0
	calibuzz();			/* initialize msdelay() parameters */
#endif SVR0

	ca = dports[0*DUARTPORTS].dp_ca;
	ca->ca_imr = 0;
	ca->ca_opcr = OPCR_INIT0;	/* clear the DUARTS */
	ca->ca_acr = ACR_SETUP;

	ca = dports[1*DUARTPORTS].dp_ca;
	ca->ca_imr = 0;
#ifdef IP2
	if (rev_A) {
		du_speeds_38[B38400] = BAUDBAD;
		ca->ca_opcr = OPCR_INIT1_REVA;
		ca->ca_acr = ACR_SETUP;
		ca->ca_ctu = (u_char)(ACR_RATE1_REVA >> 8);
		ca->ca_ctl = (u_char)(ACR_RATE1_REVA & 0xFF);
	} else {
#endif IP2
		ca->ca_opcr = OPCR_INIT1;
		ca->ca_acr = ACR_SETUP;	/* baud rate for IP2, clock on PM2 */
#ifdef ACR_RATE1
		ca->ca_ctu = (u_char)(ACR_RATE1 >> 8);
		ca->ca_ctl = (u_char)(ACR_RATE1 & 0xFF);
#endif ACR_RATE1
#ifdef IP2
	}
#endif IP2
	dummy = ca->ca_start;		/* start clock on 2nd DUART */

#ifdef OPCR_INIT2
	ca = dports[2*DUARTPORTS].dp_ca;
	ca->ca_imr = 0;
	ca->ca_opcr = OPCR_INIT2;
	ca->ca_acr = ACR_SETUP;
#ifdef ACR_RATE2
	ca->ca_ctu = (u_char)(ACR_RATE2 >> 8);
	ca->ca_ctl = (u_char)(ACR_RATE2 & 0xFF);
	dummy = ca->ca_start;		/* start clock on 3rd DUART */
#endif ACR_RATE2
#endif OPCR_INIT2

#ifdef lint
	dummy = dummy;
#endif lint


	for (dp = &dports[0]; dp <= &dports[MAXPORT]; dp++) {
		register PAD *pa = dp->dp_pa;
#ifdef PDBXPORT
		if (enable_pdbx_duart && dp == &dports[PDBXPORT])
			continue;
#endif PDBXPORT
		pa->pa_cr = CR_RXRESET;
		pa->pa_cr = CR_TXRESET;
		dp->dp_ca->ca_ropbc = dp->dp_dtr_rts;
	}

#ifdef SCNDCPORT
	/* initialize the secondary console port */
	dp = &dports[SCNDCPORT];
	du_cont(dp, DEF_CFLAG, &def_stty_ld.st_termio);
	(void)du_act(dp);
#endif

#ifdef GFXKEYPORT
	/* initialize the graphics keyboard port. */
	dp = &dports[GFXKEYPORT];
	dp->dp_iflag = IGNBRK;
	du_cont(dp, B600|CS8|PARODD|PARENB|CREAD|CLOCAL, (struct termio*)0);
	(void)du_act(dp);
#endif
}



#ifdef PM2
/* start the system clock on the PM2
 */
clkstart()
{
	register int s;

	s = spltty();
	dports[1*DUARTPORTS].dp_ca->ca_imr = (duart_imr[1] |= IMR_COUNTER);
	splx(s);
}
#endif



/* open a stream DUART port
 */
static int
du_open(rq, dev, flag, sflag)
queue_t *rq;				/* our new read queue */
dev_t dev;
int flag;
int sflag;
{
	extern dev_t console_dev, duart_dev;
	register struct dport *dp;
	register queue_t *wq = WR(rq);
	register ushort port;
	int s;

	if (sflag)			/* only a simple stream driver */
		return OPENFAIL;

#ifndef SVR3
	if (major(dev) == console_dev)
		dev = makedev(duart_dev,SCNDCPORT);
#endif
	port = PORT(dev);
	if (port > MAXPORT		/* fail if bad device # */
#ifdef IP2_GL
	    || gl_portinuse(port)	/* or if in use by graphics */
#endif
	    ) {
		u.u_error = ENXIO;
		return OPENFAIL;
	}

	dp = &dports[port];

	s = spltty();
	if (!(dp->dp_state & (DP_ISOPEN|DP_WOPEN))) {	/* on the 1st open */
		register ushort cflag;

		cflag = def_stty_ld.st_cflag;
		dp->dp_state &= ~(DP_TXSTOP|DP_LIT|DP_BLOCK
				  |DP_TX_TXON|DP_TX_TXOFF
				  |DP_FLOW);
		if (MODEM(dev)) {
#ifdef PM2
/* poll for CD if this is an old PM2 */
			if (port != 2
			    && !sduart_cdpolling)
				dupoll();
#endif
			cflag &= ~CLOCAL;
			if (FLOW_MODEM(dev))
				dp->dp_state |= DP_FLOW;
		}
		dp->dp_litc = CLNEXT;	/* clear everything */
		dp->dp_stopc = CSTOP;
		dp->dp_startc = CSTART;
		du_cont(dp, cflag, &def_stty_ld.st_termio);

		if (!du_act(dp)		/* wait for carrier */
		    && !(dp->dp_cflag & CLOCAL)
		    && !(flag & FNDELAY)) {
			dp->dp_state |= DP_WOPEN;
			do {
				if (sleep((caddr_t)dp, CARRPRI|PCATCH)) {
					u.u_error = EINTR;
					du_zap(dp,0);
					dp->dp_state &= ~(DP_WOPEN|DP_ISOPEN);
					splx(s);
					return OPENFAIL;
				}
			} while (!(dp->dp_state & DP_DCD));
			dp->dp_state &= ~DP_WOPEN;
		}

		rq->q_ptr = (caddr_t)dp;	/* connect device to stream */
		wq->q_ptr = (caddr_t)dp;
		dp->dp_wq = wq;
		dp->dp_rq = rq;
		dp->dp_state |= DP_ISOPEN;
		dp->dp_cflag |= CREAD;

		du_rclr(dp);		/* discard input */

		if (!strdrv_push(rq,"stty_ld",dev)) {
			du_zap(dp,0);
			dp->dp_state &= ~(DP_ISOPEN|DP_WOPEN);
			splx(s);
			return OPENFAIL;
		}


/* You cannot open two streams to the same device.  The dp structure can only
 * point to one of them.  Therefore, you cannot open two different minor
 * devices that are synonyms for the same device.  That is, you cannot open
 * both ttym1 and ttyd1.
 */
	} else {
		if (dp->dp_rq == rq) {
			ASSERT(dp->dp_wq == wq);
			ASSERT(dp->dp_rq->q_ptr == (caddr_t)dp);
			ASSERT(dp->dp_wq->q_ptr == (caddr_t)dp);
		} else {
			u.u_error = ENOSR;	/* fail if already open */
			splx(s);
			return OPENFAIL;
		}
	}

	splx(s);
	return minor(dev);		/* return successfully */
}



/* close a port
 */
static
du_close(rq)
queue_t *rq;				/* our read queue */
{
	register struct dport *dp = (struct dport*)rq->q_ptr;
	register int s;

	if (!dp)			/* quit now if not open */
		return;

	s = spltty();
	ASSERT(dp >= &dports[0] && dp->dp_rq == rq);

	du_flushr(dp);
	du_flushw(dp);
	dp->dp_rq = NULL;
	dp->dp_wq = NULL;
	dp->dp_state &= ~(DP_ISOPEN|DP_WOPEN);

#ifdef IP2_GL
	if (!gl_portinuse(dp->dp_index))	/* if graphics owns it */
#endif
		du_zap(dp, (dp->dp_cflag&HUPCL));	/* do not zap it */
	splx(s);
}



/* send a bunch of 1 or more characters up the stream
 *	This should be invoked only because a message could not be sent
 *	upwards by the interrupt, and things have now drained.
 */
static
du_rsrv(rq)
register queue_t *rq;			/* our read queue */
{
	register mblk_t *bp;
	register struct dport *dp = (struct dport*)rq->q_ptr;
	register int s = spltty();

	ASSERT(dp->dp_rq == rq);
	ASSERT(dp >= &dports[0] && dp <= &dports[MAXPORT]);
	ASSERT((dp->dp_state & (DP_ISOPEN|DP_WOPEN)) == DP_ISOPEN);

	if (!canput(rq->q_next)) {	/* quit if upstream congested */
		noenable(rq);
		splx(s);
		return;
	}
	enableok(rq);

	if (0 != (bp = dp->dp_rbp)) {
		register int sz;
		sz = (bp->b_wptr - bp->b_rptr);
		if (sz > 0
		    && (!dp->dp_rsrv_cnt || !dp->dp_rmsg)) {
			str_conmsg(&dp->dp_rmsg, &dp->dp_rmsge, bp);
			dp->dp_rmsg_len += sz;
			dp->dp_rbp = 0;
		}
	}

	if (0 != (bp = dp->dp_rmsg)) {
		dp->dp_rmsg = 0;
		dp->dp_rbsize = (dp->dp_rmsg_len + dp->dp_rbsize)/2;
		dp->dp_rmsg_len = 0;
		splx(s);		/* without too much blocking, */
		putnext(rq, bp);	/* send the message */
		(void)spltty();
	}

	if (!dp->dp_rmsg) {
		dp->dp_ca->ca_sopbc = dp->dp_rts;	/* restore RTS */
		if (dp->dp_state & DP_BLOCK) {	/* do XON */
			dp->dp_state |= DP_TX_TXON;
			du_start(dp);
		}
	}

	if (!dp->dp_rbp) {
		mblk_t *du_getbp();
		(void)du_getbp(dp,BPRI_LO);
	}

	splx(s);
}



/* 'put' function
 *	Just start the output if we like the message.
 */
static
du_wput(wq, bp)
queue_t *wq;				/* out write queue */
register mblk_t *bp;
{
	register struct dport *dp = (struct dport*)wq->q_ptr;
	register struct iocblk *iocp;
	register int s;

	if (!dp) {
		sdrv_error(wq,bp);	/* quit now if not open */
		return;
	}

	s = spltty();
	ASSERT(dp->dp_wq == wq);
	ASSERT(dp >= &dports[0] && dp <= &dports[MAXPORT]);
	ASSERT((dp->dp_state & (DP_ISOPEN|DP_WOPEN)) == DP_ISOPEN);

	switch (bp->b_datap->db_type) {

	case M_FLUSH:
		if (*bp->b_rptr & FLUSHW) {
			du_flushw(dp);
			dp->dp_state &= ~DP_TXSTOP;
			du_start(dp);	/* restart output */
		}
		if (*bp->b_rptr & FLUSHR)
			du_flushr(dp);
		sdrv_flush(wq,bp);
		break;

	case M_DATA:
	case M_DELAY:
		du_save(dp, wq, bp);
		break;

	case M_IOCTL:
		iocp = (struct iocblk*)bp->b_rptr;
		switch (iocp->ioc_cmd) {
		case TCXONC:
			ASSERT(iocp->ioc_count == sizeof(int));
			bp->b_datap->db_type = M_IOCACK;
			du_save(dp, wq, bp);	/* save it at head of queue */
			break;

		case TCSETA:
			ASSERT(iocp->ioc_count == sizeof(struct termio));
			bp->b_datap->db_type = M_IOCACK;
			du_save(dp, wq, bp);	/* save it at head of queue */
			break;

		case TCSETAW:
		case TCSETAF:
			ASSERT(iocp->ioc_count == sizeof(struct termio));
			du_save(dp, wq, bp);
			break;

		case TCGETA:
			tcgeta(wq,bp, &dp->dp_termio);
			break;

		case TCSBRK:
			du_save(dp, wq, bp);
			break;

		case TCBLKMD:
			dp->dp_iflag |= IBLKMD;
			iocp->ioc_count = 0;
			bp->b_datap->db_type = M_IOCACK;
			qreply(wq,bp);
			break;

		default:
			bp->b_datap->db_type = M_IOCNAK;
			qreply(wq,bp);
			break;
		}
		break;


	default:
		sdrv_error(wq,bp);
	}

	splx(s);
}



#undef POLL_DUARTS

#ifdef PM2
/* timer driven poll for Carrier-Detect
 *	Just fake an interrupt every so often, if we have not had one.
 */
static
dupoll()
{
	register int s;

	sduart_cdpolling = 1;
	(void)timeout(dupoll, 0, HZ);	/* once a second */

	s = spltty();
	if (!sduart_cdpolled)
		duintr_both();
	sduart_cdpolled = 0;
	splx(s);
}

/* poll both duarts
 */
duintr_both()
{
#define POLL_DUARTS
	while (duintr(0) || duintr(2)) continue;
	sduart_cdpolled = 1;
}
#endif

#ifdef R2300
#define POLL_DUARTS
du_poll()
{
	while (duintr(0) || duintr(2)) continue;
}
#endif

#ifdef IP4
#define POLL_DUARTS
du_poll( ports )
int	ports;
{
	while ( duintr(0) || duintr(2) || duintr(4) )
		continue;
}
#endif

/* process interrupt for a single DUART
 */
#ifdef POLL_DUARTS
static int
#endif
duintr(cn)
register int cn;			/* 1st port of chip that woke up */
{
	register struct dport *dp, *dp1;
	register u_char isr;
	register u_char worked = 0;

	ASSERT(cn < MAXPORT);
	dp = &dports[(ushort)cn];
	dp1 = dp+1;


#ifndef POLL_DUARTS
	do {
#endif
		worked = 0;
		isr = dp->dp_ca->ca_isr & *dp->dp_imrp;

		/* first process input interrupts */
		if (isr & ISR_RXA)
			worked++, du_rx(dp);
		if (isr & ISR_RXB)
			worked++, du_rx(dp1);

		/* then carrier-detect interrupts */
		if (isr & ISR_DCD) {
			register u_char iport;	/* get state */
			iport = dp->dp_ca->ca_ipcr;	/* & reset change */
#ifdef PM2
			iport = dp->dp_ca->ca_iport;	/* get state */
#endif
			if (iport & IPORT_DCDA) {
				if (dp->dp_state & DP_DCD)
					worked++, du_coff(dp);
			} else {
				if (!(dp->dp_state & DP_DCD))
					worked++, du_con(dp);
			}

			if (iport & IPORT_DCDB) {
				if (dp1->dp_state & DP_DCD)
					worked++, du_coff(dp1);
			} else {
				if (!(dp1->dp_state & DP_DCD))
					worked++, du_con(dp1);
			}
		}

		/* finally xmit data */
		if (isr & ISR_TXA)
			worked++, du_tx(dp);
		if (isr & ISR_TXB)
			worked++, du_tx(dp1);
#ifdef POLL_DUARTS
		return worked;
#else
	} while (0 != worked);
#endif
}


/* get a new buffer
 *	Interrupts ought to be off here.
 */
static mblk_t*				/* return NULL or the new buffer */
du_getbp(dp,pri)
register struct dport *dp;
uint pri;				/* BPRI_HI=try hard to get buffer */
{
	register int size;
	register mblk_t *bp;
	register mblk_t *rbp;

	rbp = dp->dp_rbp;
	if (dp->dp_rmsg_len >= MAX_RMSG_LEN	/* if overflowing */
	    || (0 != rbp		/* or current buffer empty */
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
			if (0 != bp)
				break;

			if (BPRI_HI == pri
			    && size > MIN_RMSG_LEN) {
				size >>= 2;
				continue;
			}
			break;
		}
	}

	if (0 == rbp) {			/* if we have an old buffer */
		dp->dp_rbp = bp;
	} else if (0 != bp		/* & a new buffer */
		   || (rbp->b_wptr	/* or old buffer is full */
		       >= rbp->b_datap->db_lim)) {
		str_conmsg(&dp->dp_rmsg, &dp->dp_rmsge, rbp);
		dp->dp_rmsg_len += (rbp->b_wptr - rbp->b_rptr);
		dp->dp_rbp = bp;
	}

	if (dp->dp_rmsg_len >= XOFF_RMSG_LEN
	    || !dp->dp_rbp) {
		if (DP_FLOW & dp->dp_state)	/* drop RTS */
			dp->dp_ca->ca_ropbc = dp->dp_rts;
		if ((dp->dp_iflag & IXOFF)	/*  do XOFF */
		    && !(dp->dp_state & DP_BLOCK)) {
			dp->dp_state |= DP_TX_TXOFF;
			dp->dp_state &= ~DP_TX_TXON;
			du_start(dp);
		}
	}

	return bp;
}



/* slow and hopefully infrequently used function to put characters
 *	somewhere where they will go up stream.
 */
static
du_slowr(dp,c)
register struct dport *dp;
u_char c;				/* send this byte */
{
	register mblk_t *bp;

	if (dp->dp_iflag & IBLKMD)	/* this kludge apes the old */
		return;			/* block mode hack */

	if (!(bp = dp->dp_rbp)		/* get buffer if have none */
	    && !(bp = du_getbp(dp,BPRI_HI))) {
		dp->dp_allocb_fail++;;
		return;
	}
	*bp->b_wptr = c;
	if (++bp->b_wptr >= bp->b_datap->db_lim) {
		(void)du_getbp(dp,BPRI_LO);	/* send buffer when full */
	}
}



/* input interrupt
 */
static
du_rx(dp)
register struct dport *dp;
{
	register u_char c;
	register u_char sr;
	register PAD *pa = dp->dp_pa;
	register mblk_t *bp;

#ifdef SVR3
	SYSINFO.rcvint++;
#endif SVR3

	/* must be open to input
	 */
	if (DP_ISOPEN != ((DP_ISOPEN|DP_WOPEN) & dp->dp_state)
#ifdef IP2_GL
	    && !gl_portinuse(dp->dp_index)
#endif
	    ) {
		if (!((DP_ISOPEN|DP_WOPEN) & dp->dp_state))
			du_zap(dp, (dp->dp_cflag&HUPCL));
		else
			du_rclr(dp);	/* just forget it if partly open */
		return;
	}


	/* must be reading, to read */
	if (!(dp->dp_cflag & CREAD)
#ifdef IP2_GL
	    && !gl_portinuse(dp->dp_index)
#endif
	    ) {
		du_rclr(dp);
		return;
	}


	/* process all available characters
	 */
	while ((sr = pa->pa_sr) & SR_RXREADY) {
#ifdef SVR3
		SYSINFO.rawch++;
#endif SVR3
		c = pa->pa_rhr;
#ifdef OS_CDEBUG
		/* if we get a control-A, debug on this port */
		if (kdebug && dp->dp_index == SCNDCPORT	&& '\001' == c) {
			debug("ring");
			continue;
		}
#endif
#if defined(IP2_GL) || defined(MIPS_GL)
		if (gl_softintr(dp->dp_index, c))
			continue;	/* give graphic input to graphics */
#endif
		/* start or stop output (if permitted) when we get XOFF or XON
		 */
		if (dp->dp_iflag & IXON) {
			register u_char cs = c & 0x7f;

			if ((DP_TXSTOP & dp->dp_state)
			    && (cs == dp->dp_startc
				|| ((IXANY & dp->dp_iflag)
				    && (cs != dp->dp_stopc
					|| dp->dp_line == LDISC0)))) {
				dp->dp_state &= ~DP_TXSTOP;
				du_start(dp);	/* restart output */
				if (cs == dp->dp_startc)
					continue;
			} else if (DP_LIT & dp->dp_state) {
				dp->dp_state &= ~DP_LIT;
			} else if (cs == dp->dp_stopc) {
				dp->dp_state |= DP_TXSTOP;
				du_stop(dp);	/* stop output */
				continue;
			} else if (cs == dp->dp_startc) {
				continue;	/* ignore extra control-Qs */
			} else if (cs == dp->dp_litc	/* just note escape */
				   && LDISC0 != dp->dp_line) {
				dp->dp_state |= DP_LIT;
			}
		}

		if (sr & (SR_FRERROR|SR_PERROR|SR_OVERRUN|SR_RBREAK)) {
			pa->pa_cr = CR_ERRCLR;	/* clear an error */

			if (SR_OVERRUN & sr)
				dp->dp_over++;	/* count overrun */

			if (SR_RBREAK & sr) {	/* if there was a BREAK	*/
				if (dp->dp_iflag & IGNBRK)
					continue;	/* ignore it if ok */
				if (dp->dp_iflag & BRKINT) {
					(void)putctl1(dp->dp_rq->q_next,
						      M_FLUSH,FLUSHRW);
					(void)putctl1(dp->dp_rq->q_next,
						      M_PCSIG, SIGINT);
					continue;
				}
				c = '\0';

			} else if (IGNPAR & dp->dp_iflag) {
				continue;

			} else if (!(INPCK & dp->dp_iflag)) {
				/* ignore input parity errors if asked */

			} else if ((SR_PERROR|SR_FRERROR) & sr) {
				if (SR_FRERROR & sr)
					dp->dp_fe++;
				if (dp->dp_iflag & PARMRK) {
					du_slowr(dp,0377);
					du_slowr(dp,0);
				} else {
					c = '\0';
				}
			}


		} else if (dp->dp_iflag & ISTRIP) {
			c &= 0x7f;
		} else if (c == 0377
			   && (dp->dp_iflag & PARMRK)) {
			du_slowr(dp,0377);
		}


		if (!(bp = dp->dp_rbp)	/* get a buffer if we have none */
		    && !(bp = du_getbp(dp,BPRI_HI))) {
			dp->dp_allocb_fail++;;
			continue;	/* forget it no buffer available */
		}
		*bp->b_wptr = c;
		if (++bp->b_wptr >= bp->b_datap->db_lim) {
			(void)du_getbp(dp,BPRI_LO);	/* send when full */
		}
	}


	if (!dp->dp_rsrv_cnt		/* start service counter */
	    && 0 != dp->dp_rbp) {
		timeout(du_rsrv_timer, (caddr_t)dp, RSRV_DURATION);
		dp->dp_rsrv_cnt = MAX_RSRV_CNT;
	}
}



/* process carrier-on interrupt
 */
static
du_con(dp)
register struct dport *dp;
{
#ifdef SVR3
	SYSINFO.mdmint++;
#endif SVR3

	if (!(dp->dp_state & DP_DCD)) {
		dp->dp_state |= DP_DCD;

		if (dp->dp_state & DP_WOPEN)
			wakeup((caddr_t)dp);	/* awaken open() requests */
	}
}



/* process carrier-off interrupt
 */
static
du_coff(dp)
register struct dport *dp;
{
	dp->dp_state &= ~DP_DCD;	/* note the change */

#ifdef SVR3
	SYSINFO.mdmint++;
#endif SVR3

	if (!(dp->dp_cflag & CLOCAL)	/* worry only for an open modem */
	    && (dp->dp_state & DP_ISOPEN)) {
		du_zap(dp,0);		/* kill the modem */
		flushq(dp->dp_wq, FLUSHDATA);
		(void)putctl1(dp->dp_rq->q_next,
			      M_FLUSH,FLUSHW);
		(void)putctl(dp->dp_rq->q_next, M_HANGUP);
	}
}



/* output something
 */
static
du_tx(dp)
register struct dport *dp;
{
	register u_char c;
	register mblk_t *wbp;

#ifdef SVR3
	SYSINFO.xmtint++;
#endif SVR3

	/* send all we can
	 */
	while (dp->dp_pa->pa_sr & SR_TXREADY) {
		if ((dp->dp_state & (DP_TXSTOP|DP_TIMEOUT|DP_ISOPEN))
		    != DP_ISOPEN) {
			du_stop(dp);
			return;
		}

#ifdef SVR3
		SYSINFO.outch++;
#endif SVR3
		if (dp->dp_state & DP_TX_TXON) {	/* send XON or XOFF */
			c = dp->dp_startc;
			dp->dp_state &= ~(DP_TX_TXON|DP_TX_TXOFF|DP_BLOCK);
		} else if (dp->dp_state & DP_TX_TXOFF) {
			c = dp->dp_stopc;
			dp->dp_state &= ~DP_TX_TXOFF;
			dp->dp_state |= DP_BLOCK;

		} else {
			if (!(wbp = dp->dp_wbp)) {	/* get another msg */
				wbp = getq(dp->dp_wq);
				if (!wbp) {
					du_stop(dp);
					return;
				}

				switch (wbp->b_datap->db_type) {
				case M_DATA:
					dp->dp_wbp = wbp;
					break;

				case M_DELAY:	/* start output delay */
					dp->dp_state |= DP_TIMEOUT;
					dp->dp_tid = timeout(du_delay,
							     (caddr_t)dp,
							  *(int*)wbp->b_rptr);
					freemsg(wbp);
					continue;

				case M_IOCACK:
				case M_IOCTL:
					du_i_ioctl(dp,wbp);
					continue;

				default:
					panic("bad duart msg");
				}
			}

			if (wbp->b_rptr >= wbp->b_wptr) {
				ASSERT(wbp->b_datap->db_type == M_DATA);
				dp->dp_wbp = rmvb(wbp,wbp);
				freeb(wbp);
				continue;
			}

			c = *wbp->b_rptr++;
		}

		dp->dp_pa->pa_thr = c;
		WBFLUSH();
	}
}



/* read a character from a DUART port
 *	This function is not part of the driver.  It steals characters
 *	for the kgl or for debugging.
 */
int
duxgetchar(port, timo)
short port;
register long timo;			/* cycles around check loop */
{
	register PAD *pa;
	register u_char c;
	register int s;

	ASSERT(port <= MAXPORT);

	pa = dports[port].dp_pa;
	s = spltty();
#ifndef SVR3
	if (timo) {			/* old grot */
		while (!(pa->pa_sr & SR_RXREADY)) {
			if (!--timo) return -1;
		}
	} else
#endif
		while (!(pa->pa_sr & SR_RXREADY)) continue;
	c = pa->pa_rhr;
	splx(s);

	return c;
}


/* read a character from the secondary console port.
 *	Delay forever waiting for the character.
 */
dugetchar()
{
	return duxgetchar(SCNDCPORT, 0L);
}


/* output to a duart port
 */
duxputchar(c, port)
register short c;
register short port;
{
	register PAD *pa = dports[port].dp_pa;
	register int s;

	s = spl7();	/* called from places that don't want interrupts */
	while (!(pa->pa_sr & SR_TXREADY))	/* wait for xmitter ready */
		continue;

	pa->pa_thr = c;			/* send the character */
	WBFLUSH();
	splx(s);
}


/* output a character to the secondary console port.
 */
duputchar(c)
register short	c;
{
	register PAD *pa = dports[SCNDCPORT].dp_pa;
	register int s;

	duxputchar(c, SCNDCPORT);
#ifdef mips
	if (c == '\n')
		duxputchar('\r', SCNDCPORT);
#endif

	/* implement XON/XOFF */
	s = spl7();
	if ((pa->pa_sr & SR_RXREADY)
	    && (pa->pa_rhr & 0x7F) == CSTOP) {
		(void)dugetchar();
	}
	splx(s);
}


#ifndef mips				/* used only on 68000 systems */
/* output a character to the graphics keyboard port
 *	This is used to ring the bell
 */
kb_putc(c)
{
	duxputchar(c, GFXKEYPORT);
}



/* output a character to the 'dial box' port
 */
dial_putc(c)
short c;
{
	static char dial_intson = 0;

	if (!dial_intson) {
		dial_ints(1);
		dial_intson = 1;
	}
	duxputchar(c, DIALBPORT);
}



/* enable/disable dial box port interrupts.
 */
dial_ints(intson)
short	intson;
{
#if defined(IP2_GL) && defined(GL1)
	extern short	gl_dialboxinuse;

	gl_dialboxinuse = intson;
#endif

	serial_ints(DIALBPORT, intson);
}



/* enable/disable interrupts on a port
 */
serial_ints(port, intson)
int port;				/* on this port */
int intson;				/* !=0 to turn them on */
{
	register struct dport *dp;
	register int s;

	dp = &dports[port & MAXPORT];

	s = spltty();
	if (intson) {
		dp->dp_state &= ~(DP_WOPEN|DP_TXSTOP|DP_LIT|DP_BLOCK
				  |DP_TX_TXON|DP_TX_TXOFF);
		du_cont(dp, DEF_CFLAG, &def_stty_ld.st_termio);
		dp->dp_iflag = 0;
		dp->dp_state |= DP_ISOPEN;
		(void)du_act(dp);

	} else {
		dp->dp_state &= ~(DP_ISOPEN|DP_WOPEN|DP_TXSTOP|DP_LIT|DP_BLOCK
				  |DP_TX_TXON|DP_TX_TXOFF);
		du_zap(dp,0);
	}
	splx(s);
}


/* The following implements a simple delay facility.  It is reported to
 *	generate 'better' delays in some circumstances than the normal
 *	clock facilities.
 *
 *	It is used by the GPIP driver.
 *
 *	It should NOT be used unless absolutely necessary, because it just
 *	sits here.
 */
#define CLOCKFREQ	3686		/* clock freq in khz */
#define CLOCKDIVIDER	16		/* cycles per tick */
static long millibuzz, decimillibuzz, millifudge;

/*
 * calibrate 1ms and .1ms buzz count.
 * this routine sets global variables
 *	millibuzz	number of buzzloops which can be done in 1ms
 *	decimillibuzz	number of buzzloops which can be done in .1ms
 *	millifudge	number of buzzloops equivalent to calling overhead
 * used by msdelay() and decimsdelay().
 * calibuzz() MUST BE CALLED BEFORE ANY OF {msdelay(), decimsdelay(), or
 *	clockinit()}!
 */
static
calibuzz()
{
	register long timercount;
	register long buzzcount;

	/*
	 * keep trying longer and longer buzztests
	 * until the timercount reaches "several"
	 * duart ticks.
	 */
	buzzcount = 512;
	while (buzzcount > 0 && (timercount = buzztest(buzzcount)) < 8)
		buzzcount <<= 1;

	/*
	 * convert to nloops per millisecond,
	 * ie, nloops / nmilliseconds.
	 * 1ms ~ CLOCKFREQ/CLOCKDIVIDER duart ticks.
	 */
	millibuzz = ((long)(buzzcount*CLOCKFREQ)
		     / ((int)timercount*CLOCKDIVIDER));
	decimillibuzz = millibuzz/10;
	millifudge = 7;		/* approximate */
}


static int
buzztest(benchbuzz)
long benchbuzz;
{
	register CAD *ca = dports[2].dp_ca;
	u_char dummy;
	register unsigned short timercount, duartcount;

	/* start timer on some big count */
	timercount = 0xFFFF;

	millibuzz = benchbuzz;
	ca->ca_sopbc = 0;
	ca->ca_acr = BAUD9600;
	ca->ca_ctu = (unsigned char)(timercount>>BITSPERBYTE);
	ca->ca_ctl = (unsigned char)(timercount>>0);
	dummy = ca->ca_start;

	/* buzz the given number of times */
	msdelay(1);

	dummy = ca->ca_stop;

	/* find out how many duart ticks elapsed */
	duartcount = (((short)ca->ca_ctu<<BITSPERBYTE)
		      |((short)ca->ca_ctl<<0));

#ifdef	lint
	dummy = dummy;
#endif
	return timercount - duartcount;
}


/*
 * delay approx n milliseconds.
 */
msdelay(n)
register int n;
{
	register long buzzer;

	buzzer = millibuzz - millifudge;

	while (--n >= 0) {
		while (--buzzer >= 0) {}
		buzzer = millibuzz;
	}
}


/*
 * delay approx n tenths of a millisecond.
 */
decimsdelay(n)
register int n;
{
	register long buzzer;

	buzzer = decimillibuzz - millifudge;

	while (--n >= 0) {
		while (--buzzer >= 0) continue;
		buzzer = decimillibuzz;
	}
}
#endif

#if defined(OS_CDEBUG) && defined(SVR3)
/*
 * du_conpoll - see if a receive char - used for debugging ONLY
 * NOTE: since we toss anything but ^A, this can lose chars on the
 * console (small price to pay for being able to get into debugger
 */
du_conpoll()
{
	register struct dport *dp;
	register u_char c;
	register PAD *pa;

	if (!kdebug) return;

	dp = &dports[(ushort)SCNDCPORT];
	pa = dp->dp_pa;

	if (pa->pa_sr & SR_RXREADY) {
		c = pa->pa_rhr;
		if ('\001' == c)
			debug("ring");
	}
}
#endif
