/* terminal I/O definitions
 *	some System V, some BSD, but all SGI.  (sigh)
 *
 *	$Source: /d2/3.7/src/sys/h/RCS/termio.h,v $
 *	$Revision: 1.1 $
 *	$Date: 89/03/27 17:30:09 $
 */


#ifndef __TERMIO_H__
#define __TERMIO_H__

#define	NCC	8

/* control characters */
#define	VINTR	0
#define	VQUIT	1
#define	VERASE	2
#define	VKILL	3
#define	VEOF	4
#define	VEOL	5
#define	VEOL2	6
#define	VMIN	4
#define	VTIME	5
#ifdef SVR3
#define VSWTCH	7
#endif


#define	CNUL	0
#define	CDEL	0377

/* default control chars */
#define	CTRL(c)	('c'&037)
#define	CESC	'\\'
#define	CINTR	0177	/* DEL */
#define	CQUIT	034	/* FS, cntl | */
#define	CERASE	CTRL(H)
#define	CKILL	CTRL(U)
#define	CEOF	CTRL(d)
#define	CSTART	CTRL(q)
#define	CSTOP	CTRL(s)
#ifdef SVR3
#define	CSWTCH	CTRL(z)
#define CNSWTCH	0
#endif

#define	CSUSP	CTRL(z)			/* someday, */
#define	CDSUSP	CTRL(y)			/* we may have 4.2 signals */

#define	CRPRNT	CTRL(r)			/* we now have these control chars */
#define	CFLUSH	CTRL(o)
#define	CWERASE	CTRL(w)
#define	CLNEXT	CTRL(v)


/* input modes */
#define	IGNBRK	0000001
#define	BRKINT	0000002
#define	IGNPAR	0000004
#define	PARMRK	0000010
#define	INPCK	0000020
#define	ISTRIP	0000040
#define	INLCR	0000100
#define	IGNCR	0000200
#define	ICRNL	0000400
#define	IUCLC	0001000
#define	IXON	0002000
#define	IXANY	0004000
#define	IXOFF	0010000
#define	IBLKMD	0020000

/* output modes */
#define	OPOST	0000001
#define	OLCUC	0000002
#define	ONLCR	0000004
#define	OCRNL	0000010
#define	ONOCR	0000020
#define	ONLRET	0000040
#define	OFILL	0000100
#define	OFDEL	0000200
#define	NLDLY	0000400
#define	NL0	0
#define	NL1	0000400
#define	CRDLY	0003000
#define	CR0	0
#define	CR1	0001000
#define	CR2	0002000
#define	CR3	0003000
#define	TABDLY	0014000
#define	TAB0	0
#define	TAB1	0004000
#define	TAB2	0010000
#define	TAB3	0014000
#define	BSDLY	0020000
#define	BS0	0
#define	BS1	0020000
#define	VTDLY	0040000
#define	VT0	0
#define	VT1	0040000
#define	FFDLY	0100000
#define	FF0	0
#define	FF1	0100000

/* control modes */
#define	CBAUD	0000017
#define	B0	0
#define	B50	0000001		/* not supported */
#define	B75	0000002
#define	B110	0000003
#define	B134	0000004
#define	B150	0000005
#define	B200	0000006		/* not supported */
#define	B300	0000007
#define	B600	0000010
#define	B1200	0000011
#define	B1800	0000012		/* not supported */
#define	B2400	0000013
#define	B4800	0000014
#define	B9600	0000015
#define	B19200	0000016
#define	EXTA	0000016
#define	B38400	0000017
#define	EXTB	0000017
#define	CSIZE	0000060
#define	CS5	0
#define	CS6	0000020
#define	CS7	0000040
#define	CS8	0000060
#define	CSTOPB	0000100
#define	CREAD	0000200
#define	PARENB	0000400
#define	PARODD	0001000
#define	HUPCL	0002000
#define	CLOCAL	0004000
#ifdef SVR3
/* #define RCV1EN	0010000 */
/* #define XMT1EN	0020000 */
#define LOBLK	0040000
#endif

/* line discipline 0 modes in lflag */
#define	ISIG	0000001
#define	ICANON	0000002
#define	XCASE	0000004
#define	ECHO	0000010
#define	ECHOE	0000020
#define	ECHOK	0000040
#define	ECHONL	0000100
#define	NOFLSH	0000200

#define	SSPEED	B9600


#ifndef _IO
/*
 * 4.3BSD Ioctl's have the command encoded in the lower word,
 * and the size of any in or out parameters in the upper
 * word.  The high 2 bits of the upper word are used
 * to encode the in/out status of the parameter; for now
 * we restrict parameters to at most 128 bytes.
 */
#define	IOCPARM_MASK	0x7f		/* parameters must be < 128 bytes */
#define	IOC_VOID	0x20000000	/* no parameters */
#define	IOC_OUT		0x40000000	/* copy out parameters */
#define	IOC_IN		0x80000000	/* copy in parameters */
#define	IOC_INOUT	(IOC_IN|IOC_OUT)
/* the 0x20000000 is so we can distinguish new ioctl's from old */
#define	_IO(x,y)	(IOC_VOID|('x'<<8)|y)
#define	_IOR(x,y,t)	(IOC_OUT|((sizeof(t)&IOCPARM_MASK)<<16)|('x'<<8)|y)
#define	_IOW(x,y,t)	(IOC_IN|((sizeof(t)&IOCPARM_MASK)<<16)|('x'<<8)|y)
/* this should be _IORW, but stdio got there first */
#define	_IOWR(x,y,t)	(IOC_INOUT|((sizeof(t)&IOCPARM_MASK)<<16)|('x'<<8)|y)
#endif


/*
 * Ioctl control packet
 */
struct termio {
	unsigned short	c_iflag;	/* input modes */
	unsigned short	c_oflag;	/* output modes */
	unsigned short	c_cflag;	/* control modes */
	unsigned short	c_lflag;	/* line discipline modes */
	char	c_line;			/* line discipline */
	unsigned char	c_cc[NCC];	/* control chars */
};
#define	IOCTYPE	0xff00

#define	TIOC	('T'<<8)
#define	TCGETA	(TIOC|1)
#define	TCSETA	(TIOC|2)
#define	TCSETAW	(TIOC|3)
#define	TCSETAF	(TIOC|4)
#define	TCSBRK	(TIOC|5)
#define	TCXONC	(TIOC|6)
#define	TCFLSH	(TIOC|7)
#define	TCDSET	(TIOC|32)
#define	TCBLKMD	(TIOC|33)
#define	TIOCPKT		(TIOC|112)	/* pty: set/clear packet mode */
#define		TIOCPKT_DATA		0x00	/* data packet */
#define		TIOCPKT_FLUSHREAD	0x01	/* flush packet */
#define		TIOCPKT_FLUSHWRITE	0x02	/* flush packet */
/* not supported #define TIOCPKT_STOP		0x04	/* stop output */
/* not supported #define TIOCPKT_START		0x08	/* start output */
#define		TIOCPKT_NOSTOP		0x10	/* no more ^S, ^Q */
#define		TIOCPKT_DOSTOP		0x20	/* now do ^S ^Q */
#define TIOCNOTTY (TIOC|113)		/* disconnect from tty & pgrp */
#define TIOCSTI	(TIOC|114)		/* simulate terminal input */

/*
 * Window size structure
 */
struct winsize {
	unsigned short	ws_row, ws_col;		/* character size of window */
	unsigned short	ws_xpixel, ws_ypixel;	/* pixel size of window */
};

#define TIOCGWINSZ	_IOR(t, 104, struct winsize)	/* get window size */
#define TIOCSWINSZ	_IOW(t, 103, struct winsize)	/* set window size */

#define	TFIOC	('F'<<8)
#define oFIONREAD (TFIOC|127)		/* pre-3.5 value of FIONREAD */
/* these are also defined in soioctl.h--XXX should be cleaned up */
#ifndef FIONREAD
#define	FIONREAD _IOR(f, 127, int)	/* get # bytes to read */
#define	FIONBIO	 _IOW(f, 126, int)	/* set/clear non-blocking i/o */
#define	FIOASYNC _IOW(f, 125, int)	/* set/clear async i/o */
#define	FIOSETOWN _IOW(f, 124, int)	/* set owner */
#define	FIOGETOWN _IOR(f, 123, int)	/* get owner */
#endif


#define	LDIOC	('D'<<8)
#define	LDOPEN	(LDIOC|0)
#define	LDCLOSE	(LDIOC|1)
#define	LDCHG	(LDIOC|2)
#define	LDGETT	(LDIOC|8)
#define	LDSETT	(LDIOC|9)

/*
 * Terminal types
 */
#define	TERM_NONE	0	/* tty */
#define	TERM_TEC	1	/* TEC Scope */
#define	TERM_V61	2	/* DEC VT61 */
#define	TERM_V10	3	/* DEC VT100 */
#define	TERM_TEX	4	/* Tektronix 4023 */
#define	TERM_D40	5	/* TTY Mod 40/1 */
#define	TERM_H45	6	/* Hewlitt-Packard 45 */
#define	TERM_D42	7	/* TTY Mod 40/2B */

/*
 * Terminal flags
 */
#define TM_NONE		0000	/* use default flags */
#define TM_SNL		0001	/* special newline flag */
#define TM_ANL		0002	/* auto newline on column 80 */
#define TM_LCF		0004	/* last col of last row special */
#define TM_CECHO	0010	/* echo terminal cursor control */
#define TM_CINVIS	0020	/* do not send esc seq to user */
#define TM_SET		0200	/* must be on to set/res flags */

/*
 * 'Line Disciplines
 */
#define LDISC0	0			/* ancient, standard */
#define LDISC1	1			/* new, 4.2BSD-like in streams */


/*
 * structure of ioctl arg for LDGETT and LDSETT
 *	This structure is no longer used.
 */
struct	termcb	{
	char	st_flgs;	/* term flags */
	char	st_termt;	/* term type */
	char	st_crow;	/* gtty only - current row */
	char	st_ccol;	/* gtty only - current col */
	char	st_vrow;	/* variable row */
	char	st_lrow;	/* last row */
};
#endif
