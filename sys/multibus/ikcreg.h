/*
 * ikcreg.h
 *
 * $Source: /d2/3.7/src/sys/multibus/RCS/ikcreg.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:31:24 $
 */

# define UCHAR		unsigned char


# define DEVICE(x)	(minor(x))


/* minor devices */
# define TEK		0x00		/* DMA tektronix */
# define VERS		0x01		/* versatec */
# define CENTRONICS	0x02		/* centronics */
# define RAW		0x05		/* raw interface */
# define TEKPOLL	0x06		/* tektronix */


# define IKNREGS	0x20

/* register offsets */
# define IK_MR		0x00		/* mode register */
# define IK_SR		0x01		/* status register */
# define IK_CR		0x02		/* command register */
# define IK_DCR		0x03		/* diagnostic command register */
# define IK_DOR		0x04		/* data out register */
# define IK_DDR		0x05		/* diagnostic data register */
# define IK_MAR2	0x06		/* mem address register (hi) */
# define IK_IMR		0x08		/* 8259A interrupt mode register */
# define IK_ISR		0x08		/* 8259A interrupt status register */
# define IK_ICR		0x0A		/* 8259A interrupt cond register */
# define IK_MAR1	0x0C		/* mem address register (mid) */
# define IK_MAR0	IK_MAR1		/* mem address register (lo) */
# define IK_BCR1	0x0E		/* byte count register (hi) */
# define IK_BCR0	IK_BCR1		/* byte count register (lo) */
# define IK_DMASR	0x1C		/* dma start register */
# define IK_FLCLR	0x1A		/* clear first-last register */


/* mode register bits */
# define MR_ONL_	(1<<7)		/* vers:  test online */
# define MR_NOPAPER_	(1<<6)		/* vers:  test no paper */
# define MR_IENABLE	(1<<6)		/* interrupt enable */
# define MR_OPTION	(1<<5)		/* select option port */
# define MR_STREAMING	(1<<4)		/* opt:  data streaming mode */
# define MR_IPRIME	(1<<3)		/* opt:  latched reset (iprime) */
# define MR_OPTIMIZE	(1<<2)		/* optimized addressing mode */
# define MR_VPLOT	(1<<1)		/* vers:  plot mode */
# define MR_VPPLOT	(1<<0)		/* vers:  print/plot */

/* status register bits */
# define SR_OPTION_	(1<<7)		/* option port not selected */
# define SR_DRDY	(1<<6)		/* diagnostic ready bit */
# define SR_READY	(1<<5)		/* interface ready */
# define SR_EMPTY	(1<<4)		/* dma not in progress */
# define SR_BUSY	(1<<3)		/* opt:  busy */
# define SR_FAULT	(1<<2)		/* opt:  fault */
# define SR_SEL		(1<<1)		/* opt:  selected */
# define SR_PAPEROUT	(1<<0)		/* opt:  paper out */
# define SR_PLOT	(1<<3)		/* vers:  plot mode */
# define SR_PPLOT	(1<<2)		/* vers:  print/plot mode */
# define SR_ONL		(1<<1)		/* vers:  online */
# define SR_NOPAPER	(1<<0)		/* vers:  no paper */

/* command register bits */
# define CR_SOFTACK	(1<<7)		/* software ACK */
# define CR_RESET	(1<<4)		/* master reset */
# define CR_CLEAR	(1<<3)		/* vers:  clear */
# define CR_FF		(1<<2)		/* vers:  form feed */
# define CR_EOT		(1<<1)		/* vers:  remote EOT */
# define CR_EOL		(1<<0)		/* vers:  remote line terminate */

/* interrupt status register bits */
# define ISR_INTR	(1<<7)		/* interrupt asserted */
# define ISR_COND	(03<<0)		/* interrupt cond */
# define COND_RDY	(1<<0)		/* interface ready */

/* interrupt mode register */
# define IMR_IENABLE	0x12		/* enable interrupts */
# define IMR_POLL	0x0C		/* poll for interrupts */
# define IMR_ACK	0x20		/* acknowledge interrupt */


/* reads and writes to the registers */
# define IK_INREG(a)	(((UCHAR *)iksoftc.sc_ioaddr)[a])
# define IK_OUTREG(a,d)	(((UCHAR *)iksoftc.sc_ioaddr)[a] = (d))


/* controller state */
struct	iksoftc {
	short	sc_flags;		/* current controller state */
	dev_t	sc_dev;			/* device subtype */
	caddr_t	sc_ioaddr;		/* virtual address of i/o port */
	short	sc_status;		/* status flag */
	short	sc_timer;		/* ticks left until wakeup */
	short	sc_nintr;		/* # interrupts */
};

/* .sc_flags bits */
# define SC_ALIVE	(1<<0)		/* probed successfully */
# define SC_IENABLE	(1<<1)		/* interrupts enabled */
# define SC_OPEN	(1<<2)		/* is open */
# define SC_PIOMODE	(1<<3)		/* is in pio (non-dma) mode */
# define SC_HUNG	(1<<4)		/* is hung */
# define SC_WANTED	(1<<5)		/* is wanted */
# define SC_BUSY	(1<<6)		/* is busy */
# define SC_TICKING	(1<<7)		/* has timer on */


/* interrupt level */
# define IKINTLEV	5
# define USEPRI		register int s
# define RAISE		s = spl5()
# define LOWER		splx(s)


/* sleeping priority */
# define IKPRI		PRIBIO		/* unkillable */


/* dma size */
# define IKDMASIZE	4096

# ifdef KERNEL
extern struct buf *getdmablk();
# define GETEBLK(bp)	(bp = getdmablk(BTOBB(IKDMASIZE)))
# define BRELSE(bp)	brelse(bp)
# define KVADDR(bp)	(bp->b_un.b_addr)
# define MBVADDR(bp)	(bp->b_iobase)
extern struct iksoftc iksoftc;
# endif KERNEL


/* ----- tektronix specific constants */
/* tektronix image states */
# define TS_COM		0
# define TS_RAST	1
# define TS_ERR		(-1)

/* tektronix constants */
# define TEK_PIXPERLINE	1536		/* max pixels per line */
# define TEK_HEADSIZE	9

/* tektronix command bytes */
# define T_NULL		0x00		/* Null */
# define T_EOT		0x01		/* End Of Transmission (page) */
# define T_EOL		0x02		/* End Of Line */
# define T_ABORT	0x03		/* Abort */
# define T_COPY		0x04		/* Copy (print) request */
# define T_RESERVE	0x05		/* Reserve printer */
# define T_BIT		0x07		/* Bit prompt (read next status bit) */
# define T_STATUS	0x08		/* Send status from printer */
/* ----- */
