#define	D0A	0xfc4000	/* Base of duart0 regs */
#define	D1A	0xfc6000	/* Base of duart1 regs */
#define	DINCR	0x10		/* Offset to B port */

typedef struct	 {
	unsigned char
		d_mr,	xya,		/* (0x00) Mode Register 1/2 (A/B) */
		d_sr,	xyb,		/* (0x02) Status Register (A/B) */
		d_cr,	xyc,		/* (0x04) Command Register (A/B) */
		d_rhr,	xyd,		/* (0x06) RX Holding Reg (A/B) */
		d_ipcr,	xye,		/* (0x08) Input Port Change Reg */
		d_isr,	xyf,		/* (0x0A) Interrupt Status Reg */
		d_ctu,	xyg,		/* (0x0C) Counter/Timer Upper */
		d_ctl,	xyh,		/* (0x0E) Counter/Timer Lower */
		d_xx1,	xyi,
		d_xx2,	xyj,		/* (0x12) B port mr1, sra */
		d_xx3,	xyk,
		d_xx4,	xyl,		/* (0x14) B port cra, rhr */
		d_ivr,	xym,		/* (0x16) Interrupt Vector Register */
		d_iport, xyn,		/* (0x18) Input Port */
		d_ccgo,	xyo,		/* (0x1A) Start Counter Command */
		d_ccstp, xyp;		/* (0x1C) Stop Counter Command */
} duart;

duart *dad[];

/*
 * Duart registers are low (odd) bytes of 16 bit registers, therefore
 *  each high byte is defined as a dummy.
 */

/* Extra definitions for paired registers (mostly for writing) */
#define	d_csr	d_sr		/* Clock Select Register (A/B) */
#define	d_thr	d_rhr		/* TX Holding Register (A/B) */
#define	d_acr	d_ipcr		/* Aux Control Register */
#define	d_imr	d_isr		/* Interrupt Mask Register */
#define	d_ctur	d_ctu		/* C/T Upper Register */
#define	d_ctlr	d_ctl		/* C/T Lower Register */
#define	d_opcr	d_iport		/* Output Port Conf. Reg */
#define	d_sopbc	d_ccgo		/* Set Output Port Bits Command */
#define	d_ropbc	d_ccstp		/* Reset Output Port Bits Command */

/* MR1 bits */
#define	M1RRTSC	0x80		/* Receiver Request To Send Control */
#define	M1RIS	0x40		/* Receiver Interrupt Select (1=FifoFull) */
#define	M1EMS	0x20		/* Error Mode Select (0=st/char mode) */
#define	M1PMS	0x18		/* Parity Mode Select (10=no parity) */
#define	M1NoPar	0x10
#define	M1PTS	0x04		/* Parity Type Select (0=even) */
#define	M1BPCS	0x03		/* Bits per Char Sel (11=8) */
#define	M1B8	0x03
#define	M1Dflt	(M1RRTSC|M1NoPar|M1B8)

/* MR2 bits */
#define	M2MS	0xc0		/* Mode Select (00=Normal) */
#define	M2Normal 0x00
#define	M2TRSC	0x20		/* Tx Req to Send Ctl (0=no) */
#define	M2CSC	0x10		/* Clr to Send Ctl (0=no) (IP0/1) */
#define	M2SBLS	0x0f		/* Stop Bit Length Select */
#define	M2St1	0x07		/* 1 stop */
#define	M2St1p5	0x08		/* 1.5 stop bits */
#define M2St2	0x0f		/* 2 stop bits */
#define	M2Dflt	(M2Normal|M2St1)

/* CR bits */
#define	CRCMD	0x70		/* Command bits: */
# define CRNOP	0x00		/*  Nop */
# define CRRMR	0x10		/*  Reset MR to MR1 */
# define CRRR	0x20		/*  Reset Receiver */
# define CRRT	0x30		/*  Reset Transmitter */
# define CRRES	0x40		/*  Reset Error Status */
# define CRRBCI	0x50		/*  Reset Break Change Interrupt */
#define RESET_BRK_CHANGE_INT CRRBCI /* lets have a mneumonic here (grumble) */
# define CRSB	0x60		/*  Start Break */
#define START_BREAK CRSB	
# define CRSPB	0x70		/*  Stop Break */
#define STOP_BREAK CRSPB	/* ditto */
#define	CRDT	0x08		/* Disable Transmitter */
#define	CRET	0x04		/* Enable Transmitter */
#define	CRDR	0x02		/* Disable Receiver */
#define	CRER	0x01		/* Enable Receiver */

/* SR bits */
#define	SRRB	0x80		/* Receive Break */
#define	SRFE	0x40		/* Framing Error */
#define	SRPE	0x20		/* Parity Error */
#define	SROE	0x10		/* Overrun Error */
#define	SRTE	0x08		/* Transmitter Empty */
#define	SRTR	0x04		/* Transmitter Ready */
#define	SRFF	0x02		/* Receive Fifo Full */
#define	SRRR	0x01		/* Receiver Ready */

/* OPCR bits */
#define	OPCR	0xF4		/* TxRDYB/A, RxRDYB/A */
#define OP5BIT	0x20		/* this bit must be RESET if op5 is
				   to be passed through */

/* ACR bits */
#define	ACR	0xEB		/* Baud rate set 2, IP3,2 delta int enabl */

/* IPCR bits */
#define	IPDTR	0x04		/* DTR A, <<1 DTR B */
#define	IPdDTR	0x40		/* delta DTRA, <<1 delta DTRB */

/* ISR bits -- various interrupt states */
#define	ISIPCS	0x80		/* Input Port Change Status */
#define	ISCR	0x08		/* Counter Ready */
  /* These bits are tested in low nibble (A) and high nibble (B) */
#define	ISCB	0x04		/* Change in Break */
#define	ISRR	0x02		/* Receiver Ready */
#define	ISTR	0x01		/* Transmitter Ready */
#define	ISCBB	0x40		/* Change in Break - B port */
#define	ISRRB	0x20		/* Receiver Ready - B port */
#define	ISTRB	0x10		/* Transmitter Ready - B port */

/* IMR bits -- Turn a bit on to enable corresponding ISR interrupt */
/* #define	IMR	0xF7		/* ALL ENABLED - Counter */
#define	IMR	0x00		/* None for now */
#define	IMRCI	0x08		/* Counter interrupt enable */
#define	IMRII	0x80		/* Input port change interrupt enable */
#define IMRCBI	0x04		/* input port A change in break int. en */
#define IMRCBIB 0x40		/* input port B change in break int. en */
  /* These bits are tested in low nibble (A) and high nibble (B) */
#define	IMRTIB	0x10
#define	IMRRIB	0x20
#define	IMRTI	0x01
#define	IMRRI	0x02

/* SOPBC/ROPBC bits -- Set/Reset output port bits */
#define	SOPDS	0x10		/* Data Set Ready For Port A, <<1 for Port B */
#define	ROPDS	0x10		/*  Ditto, but Reset */

# ifdef NOTDEF
/*#define	DEBUG			/* Comment out to remove debug messages */
#ifdef	DEBUG
# define D0(s) printf(s)
# define D1(s,a) printf(s,a)
# define D2(s,a,b) printf(s,a,b)
#else
# define D0(s)
# define D1(s,a)
# define D2(s,a,b)
#endif
# endif NOTDEF

#define SCREEN 0
#define LOCALCHIP 0
#define LOCAL 1
#define HOSTCHIP 2
#define HOST  2
#define TIMER 2
#define TIMERCHIP 2

/* bit patterns to write to the csr for different baud rates */
#define BAUD300  0x44
#define BAUD600	0x55
#define BAUD1200 0x66
#define BAUD2400 0x88
#define BAUD4800 0x99
#define BAUD9600 0xBB
#define BAUD19200 0xCC

#define SETBAUD(port,rate) dad[port]->d_csr = rate
#define BREAKCHAR (-2)
#define NOCHAR (-1)

