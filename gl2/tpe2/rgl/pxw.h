/**************************************************************************
 *									  *
 * 		 Copyright (C) 1984, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/*
 * these are pmII values replacing the pmI config.h values
 */
#ifdef PM1
#define MBIOBASE	0x1f0000/* MBIO */
#define PXDMASIZ	0x3800 	/* 1ff800 reserved */
#else
#define MBIOBASE	0xf6b000
#define PXDMASIZ	0x4000 	/* 8 2k bytewides on PX */
#endif
#define PXBUFSIZ	0x1000	/* PCOX screen buffer size */

struct pxdma {			/* 8237 DMA controller on PX */
	u_char	BADDR0;
	u_char	BWRDCT0;	/* PX card has correct addresses on mb */
	u_char	BADDR1;
	u_char	BWRDCT1;
	u_char	BADDR2;
	u_char	BWRDCT2;
	u_char	BADDR3;
	u_char	BWRDCT3;

	u_char	COMMAND; /* read for status */
	u_char	REQUEST;
	u_char	SINGLMASK;
	u_char	MODE;
	u_char	CLEARBYTE;
	u_char	DMACLEAR;	/* read for temporary register */
	u_char	CLEARMASK;
	u_char	MASKWRITE;
};

#define PXDMAADDR ((struct pxdma *)(MBIOBASE + 0x7e00))
/*   the PX uses DMA1 for input from the PCOX */
#define SETMASK1	0x05
#define CLRMASK1	0x01
#define MODE0		0x48	/* single,incr,read,0 */
#define MODE1		0x55	/* single,incr,autoinit,write,1 */
#define MODE2		0x4a	/* single,incr,read,2 */
#define MODE3		0x4b	/* single,incr,read,3 */

struct pxddevice {		/* PCOX controller on PX */
	u_char	PXD_SIGNAL;
	u_char	PXD_KREG;
};
#define PXIOADDR ((struct pxddevice *)(MBIOBASE + 0x7eee))

struct pxdm {
	u_char	Dma;
};
#define DMAADDR ((struct pxdm *)(MBIOBASE + 0xc000))

/*  command bits to the PCOX card
*/
#define RUN_BIT		0x1	/* signal port run bit */
#define SIG1		0x2	/* stop rx from px if set, write config */
#define SIG2		0x4	/* px diag enabled */
#define SIG3		0x8	/* write to nano buffer enabled */

#define TERMINAL_ID	(u_char)0xe4	/* model 2 = 24 x 80 */

#ifdef DEBUG
#define DP0(s) { if(Pxd_debug) (void)messagef(s); }
#define DP1(s,a) { if(Pxd_debug) (void)messagef(s,a); }
#else
#define DP0(s)
#define DP1(s,a)
#endif

#define true		1
#define false		0
#define YES		1
#define NO		0

/*  direct write mode settings (indicates last command sent to nano)
*/
#define DIR_MODE_SET	1	/* set cursor address */
#define DIR_MODE_WD	2	/* writing data to buffer */
#define DIR_MODE_WA	3	/* writing attribute to buffer */

/*  delay constants for output to the PCOX card
*/
#define MS_15		15
#define MS_100		100
#define ONESEC		1200

/*  Commands for writing directly to nano buffer
**   DIR_CMD_WA is used for c0 type writes where the desired data
**     is c0 or higher (e.g. d1 = c1,11; f2 = c1,32; c0 = c1,00).
**   DIR_CMD_SETLX is used with c2,c3 type writes where the desired
**     low cursor is 80 or higher (e.g. 7cf = c2,07,c4,4f).
**   Normal example is 77f = c2,07,c3,7f.
*/
#define DIR_CMD_WD	(u_char)0xc0	/* write data to buffer */
#define DIR_CMD_WA	(u_char)0xc1	/* write data with PCOX adding c0 */
#define DIR_CMD_SETH	(u_char)0xc2	/* set direct cursor high */
#define DIR_CMD_SETL	(u_char)0xc3	/* set direct cursor low */
#define DIR_CMD_SETLX	(u_char)0xc4	/* set cursor low with PCOX add 80 */

/*  special characters and indicators from the PCOX
*/
#define CMD_MASK	(u_char)0xd2	/* mask for nano commands */
#define CMD_PATTERN	(u_char)0xc2	/* pattern for nano commands 11x0 xx1x */
#define ATTR_IND	(u_char)0xc0	/* bit pattern for attrib/indicators */
#define END_WRITE	(u_char)0xe6	/* end of write data from PCOX */
#define DMA_ZAP		(u_char)0xef	/* no data from 3270 indicator */

/*  status bits in 'Status_flags' set by the PCOX reader
*/
#define W_FLAG		0x10	/* are writing screen to host */
#define A_FLAG		0x20	/* alarm has been sounded */
#define R_FLAG		0x40	/* pos ACK1 from host after write */
#define S_FLAG		(u_char)0x80	/* neg ACK0 from host after write */

/*  codes to the 3270
*/
#define SHIFT_MAKE	0x4d	/* shift make for px */
#define ALT_MAKE	0x4f	/* alt make for px */
#define SHIFT_BREAK	(u_char)0xcd	/* shift make + break bit */
#define ALT_BREAK	(u_char)0xcf	/* alt make + break bit */
#define BSPACE		0x31
#define CLEAR		0x51
#define ENTER		0x18
#define PA1		0x5f
#define PA2		0x5e
#define RESET		0x34
#define SPACE		0x10

/*  extended character scan table (program interface)
**  extended keyboard character translate table
*/
typedef struct {
	u_char	k_stroke[4];	/* up to 3 character sequence from the keyboard
				   plus a null sequence terminator */
	u_char	k_mode;		/* mode: NORMAL, SHIFT, ALT, DONTK */
	u_char	k_code;		/* key code to the 3270 */
} x_key_table ;

#define NORMAL		0
#define SHIFT		1
#define ALT		2
#define DONTK		4

/*  aid indexes into send_x_key table
*/
#define	X_CLEAR		04
#define	X_ENTER		13
#define	X_ER_EOF	14
#define	X_ER_INPUT	15
#define X_FM		16
#define X_FTAB          17
#define	X_HOME		18
#define X_PA1		21
#define X_PA2		22
#define	X_RESET		47
#define X_TEST		49
#define X_ALT_BREAK     50
#define X_ALT_MAKE      51
#define X_SHIFT		52

#define ROWS		25	/* number of rows including status line */
#define MAX_ROWS	40	/* max no. of rows including status line */
#define COLS		80
#define S_CHARS		(ROWS * COLS)	/* number of screen characters */
#define PXINDSIZ	(25*80)

#define ERROR		-1

#define DT

#define hibyte(x) (((unsigned char *)&(x))[0])
#define lobyte(x) (((unsigned char *)&(x))[1])
#define	min(a,b) ( (a) <= (b) ? (a) : (b) )

typedef struct {		/* pxdio's buffer */
	long    d_cnt;		/* number of chars in buffer */
	u_char *d_rdp;		/* read pointer */
	u_char  d_buf[PXDMASIZ];	/* buffer */
} dma_buffer;

typedef struct {
	long	mbaddr;
	char	mbchar;
} getput;

typedef struct {
	u_char *hdraddr;	/* outbound file header at */
	u_char *bodyaddr;	/* outbound file body at */
	long    bodylen;	/* outbound file max body length */
} outft;

typedef struct {
	u_char *bodyaddr;	/* outbound file body at */
	long    bodylen;	/* outbound file max body length */
} rglft;

typedef struct {		/* buffer */
	u_char *rdp;		/* unload pointer */
	u_char *wrp;		/* load pointer */
	u_char *bufp;		/* pointer to buffer */
} px_buf_t;

typedef struct {		/* px driver status (all pointers) */
	px_buf_t pxt_buf;	/* local terminal buffer */
	u_char *dma_buf;	/* user's dma buffer */
} px_bufs;

typedef struct {		/* px driver status (all pointers) */
	u_char *wrp;		/* load pointer */
	u_char *bufp;		/* pointer to buffer */
	short	ft;		/* flag for in ft */
	short	ftdone;		/* flag for done rxft */
	long	host_len;	/* working length */
	long	length;		/* host length request */
	u_char *headrp;		/* file header store addr */
	long	max_len;	/* max received length allowable */
	long	r1save;		/* r1 when try4 stops outb */
	long	r2save;		/* r2 when try4 stops outb */
} px_status;


#define kb_nano(c)		ioctl(fid, 0, (c))
#define set_signal(c)		ioctl(fid, 1, (c))
#define reset_signal(c) 	ioctl(fid, 2, (c))
#define force_signal(c)		ioctl(fid, 3, (c))
#define init_signal()		ioctl(fid, 5, 0)
#define read_avail(addr)	ioctl(fid, 6, (addr))
#define fetch_byte(addr)	ioctl(fid, 7, (addr))
#define store_byte(addr)	ioctl(fid, 8, (addr))
#define set_outb_pointer(addr)	pxdioctl(fid, 9, (addr))
#define get_outb_status(addr)	pxdioctl(fid,10, (addr))
#define raise1()		ioctl(fid,11, 0)
#define kill_outb()		pxdioctl(fid,12, 0)
#define force_open_init()	ioctl(fid,13, 0)
#define read_open_init(addr)	ioctl(fid,14, (addr))
#define test_dma_memory(addr)	ioctl(fid,15, (addr))
#define set_rglout_ptr(addr)	pxdioctl(fid,16, (addr))
#define EACK		90	/* part of outb received */
#define EDONE		91	/* all of an outb file received */
#define ELENGTHBAD	92	/* host length negative or in error */
#define ECOPYOUT	93	/* copyout returned an error */
#define ERBUF		94	/* overrun in kernal rx copyout buf */
#define ERZAP		95	/* reread failure or outb length not found */
#define E74LEN		96	/* length ended by 3274 (not at home) */
#define EZAP		97	/* overrun on PXD hardware, kernel discovered */
#define ESTART		98	/* pxdread detected start of message */

#define DATA1RXFER	0x1b	/* cent sign */
#define DATA2RXFER	0x1a	/* dollar sign */
#define DATA3RXFER	(u_char)0x82	/* c is outb continuation sign */
#define CENT		0x1b
#define DOLLAR		0x1a
#define F3274		0x85	/* buck,f starts inbound binary */
#define ADOLLAR		0x24	/* ascii dollar */
#define EDOLLAR		0x5b	/* ebcdic dollar */
#define FT_ATP		1
#define FT_RGL		2

#define iscommand(c) ((c & CMD_MASK) == CMD_PATTERN)

typedef int (*fptr_t)();

static u_char trace = 0;	/* each module has it's own trace variable */

