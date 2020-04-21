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

#define PXDMASIZ	0x4000	/* 8 2k/8k bytewides on PX */
#define PXBUFSIZ	0x1000	/* PCOX screen buffer size */
#define DEFAULTRU	1536	/* SNA default */
#define SMALL_DMA	15216	/* divisible by 12 */

/*  command bits to the PCOX card
*/
#define RUN_BIT		0x1	/* signal port run bit */
#define SIG1		0x2	/* stop rx from px if set, write config */
#define SIG2		0x4	/* px diag enabled */
#define SIG3		0x8	/* write to nano buffer enabled */

#ifdef DEBUG
#define DT	if (trace) trace_msg
#else
#define DT
#endif

/*  delay constants for output to the PCOX card
*/
#define MS_15		15
#define MS_20		20
#define MS_100		100
#define ZAP_COUNT	1500000	/* 1 sec wait for data from dma */

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

#define NORMAL		0
#define SHIFT		1
#define ALT		2
#define DONTK		4

/*  aid indexes into send_x_key table
*/
#define	X_CENT		05
#define	X_CLEAR		06
#define	X_ENTER		15
#define	X_ER_EOF	16
#define X_PA1		23
#define	X_RESET		49
#define X_ALT_MAKE      53

#define ROWS		25	/* number of rows including status line */
#define MAX_ROWS	40	/* max no. of rows including status line */
#define COLS		80
#define S_CHARS		(ROWS * COLS)	/* number of screen characters */
#define PXINDSIZ	(25*80)

#define ERROR		-1
#define CNTU		1


#define hibyte(x) (((unsigned char *)&(x))[0])
#define lobyte(x) (((unsigned char *)&(x))[1])
#define	min(a,b) ( (a) <= (b) ? (a) : (b) )

typedef struct {		/* pxdio's buffer */
	long	d_cnt;		/* number of chars in buffer */
	u_char *d_rdp;		/* read pointer */
	u_char	d_buf[PXDMASIZ];	/* buffer */
} dma_buffer; 

typedef struct {		/* pxdio's buffer */
	long	d_cnt;		/* number of chars in buffer */
	u_char *d_rdp;		/* read pointer */
} dma_opr; 

typedef struct {
	long	mbaddr;
	u_char	mbchar;
} getput;

typedef struct {
	u_char k_mode;		/* mode: NORMAL, SHIFT, ALT, DONTK */
	u_char k_code;
} key_table;

typedef struct {
	u_char	k_stroke[4];	/* up to 3 character sequence from the keyboard
				   plus a null sequence terminator */
/*	u_char	k_mode;		/* mode: NORMAL, SHIFT, ALT, DONTK */
/*	u_char	k_code;		/* key code to the 3270 */
} x_key_table;

typedef struct {
	u_char *hdraddr;	/* outbound file header at */
	u_char *bodyaddr;	/* outbound file body at */
	long    bodylen;	/* max body length (multiple of 3) */
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
	long	max_len;	/* max length allowable (multiple of 3) */
} px_status;

#define kb_nano(c)		ioctl(fd, 0, (c))
#define set_signal(c)		ioctl(fd, 1, (c))
#define reset_signal(c) 	ioctl(fd, 2, (c))
#define force_signal(c)		ioctl(fd, 3, (c))
#define get_current(addr)	ioctl(fd, 4, (addr))
#define init_signal()		ioctl(fd, 5, (char *)0)
#define read_avail(addr)	ioctl(fd, 6, (addr))
#define fetch_byte(addr)	ioctl(fd, 7, (addr))
#define store_byte(addr)	ioctl(fd, 8, (addr))
#define get_outb_status(addr)	pxdioctl(10, (addr))
#define raise1()		ioctl(fd,11, (char *)0)
#define kill_outb()		pxdioctl(12, (char *)0)
#define force_open_init()	ioctl(fd,13, (char *)0)
#define read_open_init(addr)	ioctl(fd,14, (addr))
#define test_dma_memory(addr)	ioctl(fd,15, (addr))
#define set_rglout_ptr(addr)	pxdioctl(16, (addr))
#define set_rglcntu(addr)	pxdioctl(17, (addr))
#define get_read_count(addr)	ioctl(fd,16, (addr))
#define EACK		90	/* part of outb received */
#define EDONE		91	/* all of an outb file received */
#define ELENGTHBAD	92	/* host length negative or in error */
#define ECOPYOUT	93	/* copyout returned an error */
#define ERBUF		94	/* overrun in kernal rx copyout buf */
#define EZAP		95	/* overrun on PXD hardware, kernel discovered */
#define E74LEN		96	/* length ended by 3274 (not at home) */
#define ERZAP		97	/* reread failure or outb length not found */
#define ESTART		98	/* pxdread detected start of message */
#define ECNTU		99	/* outbread has wrong msg type */
#define EACKNAK		101	/* cent r or cent s found */

#define DATA2RXFER	0x46	/* dollar sign was 0x1a now 0x46         wpc */
#define DATA3RXFER	(u_char)0x82	/* outb continuation sign */
#define TEXT2RXFER	(u_char)0x93	/* outb text start sign */
#define TEXT3RXFER	(u_char)0x8c	/* outb text continuation sign */
#define CENT		0x45    /* was 0x1b change to 0x45 lessen impact wpc */
#define DOLLAR		0x46    /* was 0x1a change to 0x46 lessen impact wpc */
#define F3274		(u_char)0x85
#define KILLIT		(u_char)0x8a
#define ADOLLAR		0x80	/* ascii was 0x24 now 0x80 for Asc_xlat wpc */
#define EDOLLAR		0x65	/* ebcdic was 0x5b now 0x65 new ctrl char wpc */
#define MXFER		1
#define RGLXFER		2
#define FXFER		4

#define iscommand(c) ((c & CMD_MASK) == CMD_PATTERN)

typedef long (*fptr_t)();

static u_char trace = 0;	/* each module has it's own trace variable */

