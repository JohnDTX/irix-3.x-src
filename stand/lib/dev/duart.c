/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/duart.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:26 $
 */

#include	"sys/types.h"
#include	"cpureg.h"
#include	"common.h"

#define	DUINCR	0x8		/* Offset to B port	*/

#define	CSTART	021	/* cntl q */
#define	CSTOP	023	/* cntl s */

/*
** Duart registers 
**   we overlap d_mrb thru d_stop ontop of d_mr thru d_ctl for many
**   cases of access.  Only a few accesses require we explicitly
**   address d_iport and the like.  This is done by having the
**   address of the duart port, then referencing the field
**   For example using the address for duart 0 port 0 if you
**   access d_rhra you get the channel A holding register.  If you
**   used the address of duart 0 port b and made the same reference
**   you would get holding register b.  Hence the overlap.
*/
typedef struct	 {
	u_char
		d_mr,		/* Mode Register A		*/
		d_sr,		/* Status Register A		*/
		d_cr,		/* Command Register A		*/
		d_rhra,		/* RX Holding Reg A		*/
		d_ipcr,		/* Input Port Change Reg	*/
		d_isr,		/* Interrupt Status Reg		*/
		d_ctu,		/* Counter/Timer Upper		*/
		d_ctl,		/* Counter/Timer Lower		*/

		d_mrb,		/* Mode Register B		*/
		d_srb,		/* Status Register B		*/
		d_crb,		/* Command Register B		*/
		d_rhrb,		/* RX Holding Reg B		*/
		d_x1,		/* Reserved			*/
		d_iport,	/* Input Port			*/
		d_start,	/* Start Counter Command	*/
		d_stop;		/* Stop Counter Command		*/
} duart;

/*
** Extra definitions for paired registers (mostly for writing)
*/
#define	d_csr	d_sr		/* Clock Select Register (A/B)	*/
#define	d_thr	d_rhra		/* TX Holding Register (A/B)	*/
#define	d_acr	d_ipcr		/* Aux Control Register		*/
#define	d_imr	d_isr		/* Interrupt Mask Register	*/
#define	d_ctur	d_ctu		/* C/T Upper Register		*/
#define	d_ctlr	d_ctl		/* C/T Lower Register		*/
#define	d_opcr	d_iport		/* Output Port Conf. Reg	*/
#define	d_sopbc	d_start		/* Set Output Port Bits Command */
#define	d_ropbc	d_stop		/* Reset Output Port Bits Command*/

/* some bit definitions for the duart */

/* mr1 */
#define	MR1_ODDPARITY	0x04		/* odd parity	*/
#define	MR1_NOPARITY	0x10		/* no parity	*/
#define	MR1_8BPC	0x03		/* 8 bits per character	*/

/* mr2 */
#define	MR2_1STOP	0x07		/* 1 stop bit	  */
#define	MR2_2STOP	0x0F		/* 2 stop bits	  */
#define	MR2_CTS		0x10		/* rts/cts enable */

/* sr */
#define	SR_RXREADY	0x01		/* receiver ready	*/
#define	SR_TXREADY	0x04		/* xmit ready		*/
#define	SR_PERROR	0x20		/* parity error		*/
#define	SR_FRERROR	0x40		/* frame error		*/
#define	SR_OVERRUN	0x10		/* overrun		*/
#define	SR_RBREAK	0x80		/* break received	*/

/* cr */
#define	CR_RESETMR	0x10		/* reset MR pointer to be MR1	*/
#define	CR_DISABLE	0x0A		/* disable rx and tx		*/
#define	CR_ENABLE	0x05		/* enable rx and tx		*/
#define	CR_RXRESET	0x20		/* reset receiver		*/
#define	CR_TXRESET	0x30		/* reset transmitter		*/
#define	CR_ERRCLR	0x40		/* clear error bits		*/
#define	CR_STARTBREAK	0x60		/* start a break		*/
#define	CR_STOPBREAK	0x70		/* stop a break			*/

/* opcr */
#define	OPCR_INIT0	0x04		/* enable refresh timer output	*/
#define	OPCR_INIT1	0x04		/* no enables			*/

/* acr */
#define	ACR_RATE	30720		/* 60hz, that is		  */
#define	ACR_SETUP	0xEC		/* baud set 2, timer, extern clock*/

/* isr */
#define	ISR_TXA		0x01		/* port a transmitter is ready	*/
#define	ISR_TXB		0x10		/* port b transmitter is ready	*/
#define	ISR_RXA		0x02		/* port a reciever is ready	*/
#define	ISR_RXB		0x20		/* port b reciever is ready	*/
#define	ISR_DCD		0x80		/* enable carrier detect ints	*/

/* imr */
#define	IMR_DCD		0x80		/* enable carrier detect ints	  */
#define	IMR_COUNTER	0x08		/* enable counter ints		  */
#define	IMR_TXA		0x01		/* enable port a transmitter ints */
#define	IMR_TXB		0x10		/* enable port b transmitter ints */
#define	IMR_RXA		0x02		/* enable port a reciever ints	  */
#define	IMR_RXB		0x20		/* enable port b reciever ints	  */

/* iport */
#define	IPORT_DCD0	0x08		/* dcd bit for even #'d ports	*/
#define	IPORT_DCD1	0x20		/* dcd bit for odd #'d ports	*/

/* oport (used with d_ropbc and d_sopbc) */
#define	OPORT_RTSA	0x01		/* rts for port a */
#define	OPORT_RTSB	0x02		/* rts for port b */
#define	OPORT_DTRA	0x10		/* dtr for port a */
#define	OPORT_DTRB	0x20		/* dtr for port b */

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

/*
** duart addresses.  Indexes into this array represent the duart
** port that _duputc needs
*/
duart	*du_addr[ 4 ] = {
	(duart *)(DUART0_BASE + 0*DUINCR),
	(duart *)(DUART0_BASE + 1*DUINCR),
	(duart *)(DUART1_BASE + 0*DUINCR),
	(duart *)(DUART1_BASE + 1*DUINCR)
};

/*
** OPCR bits which are different between the 2 duarts. Duart 0 clocks
** the RAM refreshes
*/
static char	opcr[ 4 ] = {
	OPCR_INIT0, OPCR_INIT0, OPCR_INIT1, OPCR_INIT1
};

/*
** duart bit patterns for various speeds
*/
u_char	du_speeds[ 16 ] = {
	BAUDBAD,	BAUDBAD,	BAUD75,		BAUD110,
	BAUD134,	BAUD150,	BAUDBAD,	BAUD300,
	BAUD600,	BAUD1200,	BAUD1800,	BAUD2400,
	BAUD4800,	BAUD9600,	BAUD19200,	BAUDBAD
};

/*
** _duinit
**   initalize the duart (BOTH CHANNELS)
**   layout is:
**	duart 0
**	  channel A	Keyboard (if graphics monitor present)
**	  channel B	tty 1 (Secondary diagnostic port)
**	  Timer is used to control refresh
**
**	duart 1
**	  channel A	tty 2
**	  channel B	tty 3
*/
_duinit( port, aspeed, bspeed )
register int port;
{
	register duart *dp;

	port &= ~1;			/* Force to low channel	*/
	dp = du_addr[ port ];		/* get addr of the port	*/

	/* Set common chip bits */

	dp->d_opcr = opcr[ port ];	/* Set Output Port bits */
	dp->d_acr  = ACR_SETUP;		/* Set Aux Control bits */
	dp->d_imr  = 0;			/* Set Interrupt mask	*/

	/* set up port A */

	if ( port == 0 )
		dp->d_mr = MR1_ODDPARITY | MR1_8BPC;
	else
		dp->d_mr  = MR1_NOPARITY | MR1_8BPC;
	dp->d_mr  = MR2_1STOP;
	dp->d_cr  = CR_RESETMR;		/* Reset MR to MR1 */
	dp->d_cr  = CR_RXRESET;		/* Reset Rcvr */
	dp->d_cr  = CR_TXRESET;		/* Reset Txmt */
	dp->d_cr  = CR_ERRCLR;		/* Reset Error Status */

	switch ( aspeed ) {

	case CON_9600:
		dp->d_csr = BAUD9600;		
		break;

	case CON_300:
		dp->d_csr = BAUD300;		
		break;

	case CON_1200:
		dp->d_csr = BAUD1200;		
		break;

	case CON_192:
		dp->d_csr = BAUD19200;		
		break;

	case CON_600:
		dp->d_csr = BAUD600;		
		break;
	}

	dp->d_cr  = CR_ENABLE;		/* Enable Receiver & Xmitter */

	/* Now set up port B */

	port++;
	dp = du_addr[ port ];
	dp->d_mr  = MR1_NOPARITY | MR1_8BPC;
	dp->d_mr  = MR2_1STOP;
	dp->d_cr  = CR_RESETMR;		/* Reset MR to MR1 */
	dp->d_cr  = CR_RXRESET;		/* Reset Rcvr */
	dp->d_cr  = CR_TXRESET;		/* Reset Txmt */
	dp->d_cr  = CR_ERRCLR;		/* Reset Error Status */

	switch ( bspeed ) {

	case CON_9600:
		dp->d_csr = BAUD9600;		
		break;

	case CON_300:
		dp->d_csr = BAUD300;		
		break;

	case CON_1200:
		dp->d_csr = BAUD1200;		
		break;

	case CON_192:
		dp->d_csr = BAUD19200;		
		break;

	case CON_600:
		dp->d_csr = BAUD600;		
		break;
	}
	dp->d_cr  = CR_ENABLE;		/* Enable Receiver & Xmitter */
}

/*
** _duputchar
**   print a character on the secondary console (duart 0 port B)
**   If this routine is called we assume this must be the console, so
**   we even support flow control of a sorts
*/
_duputchar( c )
register	c;
{
	register duart *dp = du_addr[ 1 ];


	_duputc( c, 1 );

	/*
	** flow control of a sorts!!!!
	if ( ( dp->d_sr & SR_RXREADY ) &&
	    ( ( dp->d_rhra & 0x7F ) == CSTOP ) )
		while ( _dugetchar(0) != CSTART )
			;
	*/
}

/*
** _duputc
**   output a character to the given duart port
*/
_duputc( c, port )
register	c;
register	port;
{
	register		s;
	register unsigned	timo;
	register duart		*dp = du_addr[ port ];

	timo = (unsigned) 60000;
	/*
	** Try waiting for the console tty to come ready,
	** otherwise give up after a reasonable time.
	*/
	while( ( dp->d_sr & SR_TXREADY ) == 0 )
		if (--timo == 0)
			break;

	if ( c == 0 )
		return;

	dp->d_thr = c;		/* Output char */

	if ( port != 0 && c == '\n' )
		_duputc( '\r', port );
}

/*
** _dugetchar
**   get a character on the secondary console (duart 0 port B)
*/   
_dugetchar(nowaitflag)
{
	register	c;

	/* read from duart port B*/
	if ( nowaitflag )
		return _dugetc( 1, 2 );		/* don't delay		*/
	else
		return _dugetc( 1, 0 );		/* delay forever	*/
}

/*
** _dugetc
**   read a character from duart port 'port'.  'timo' represents how
**   long to delay waiting for a character.  If timo == 0 then we will
**   delay forever.
*/
_dugetc( port, timo )
short		port;	/* duart port				*/
register long	timo;	/* how long to delay waiting for a char	*/
{
	register duart	*dp = du_addr[ port ];
	char c;

	if ( timo )
	{
		while ( ( ( dp->d_sr & SR_RXREADY ) == 0 ) && --timo )
			;
		if ( timo == 0 ) {
			return -1;
		}
	} else
	{
		while ( ( dp->d_sr & SR_RXREADY ) == 0 )
			;
	}
	c = dp->d_rhra;
	if ( port != 0 )
		if ( c == '\r' )
			c = '\n';
	return (unsigned)c;
}
