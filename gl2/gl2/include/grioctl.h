#ifndef GRIOCTLDEF
#define GRIOCTLDEF
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
 * Graphics ioctl's:
 *	- user issues a "grioctl(cmd, argvector)" system call
 *
 * $Source: /d2/3.7/src/gl2/gl2/include/RCS/grioctl.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 15:57:04 $
 */

/* alloc and free */
#define	GR_GRALLOC		1	/* allocate window */
#define	GR_GRINIT		2	/* alloc & init shared memory */
#define	GR_GRFREE		3	/* free graphics */
#define GR_INITINPUT		4 	/* init the input devices */
#define	GR_LOSEGR		5	/* lose gr handle... */
#define	GR_NULL			6	/* null grioctl */
#define GR_INITBLINK		7	/* turn off all blinking */

/* generic graphics commands */
#define GR_ERRORHANDLER		10	/* enter the error handler */
#define GR_QDEVICE		11	/* queue a device */
#define GR_UNQDEVICE		12	/* unqueue a device */
#define GR_SIGNALERROR		13	/* signal an error */
#define GR_REDIRECTERRORS	14	/* redirect errors */
#define GR_SETVALUATOR		15	/* set a valuator */
#define GR_GETVALUATOR		17	/* get a valuator positon */
#define GR_GETBUTTON		18	/* get a button's state */
#define	GR_QRESET		19	/* reset the queues */
#define	GR_QREAD		20	/* read element from queue */
#define	GR_QENTER	 	22	/* enter something in the queue */
#define GR_NOISE		23	/* set the noise on a device */
#define GR_TIE			24	/* tie valuators to a button */
#define	GR_ATTACHCURSOR		25	/* attach cursor to some valuators */
#define GR_CURSON		27	/* turn the cursor on */
#define GR_CURSOFF		28	/* turn the cursor off */
#define	GR_MAPCOLOR		29	/* map a color register */
#define GR_GETMCOLOR		30	/* get a color map entry */
#define GR_BLINK		31	/* blink a color */
#define GR_GSYNC		32	/* wait for retrace */
#define GR_SWAPINTERVAL		33	/* set the swap interval */
#define	GR_SINGLEBUFFER		34	/* go into double buffer mode */
#define	GR_DOUBLEBUFFER		35	/* go into single buffer mode */
#define	GR_RGBMODE		36	/* go into rgb mode */
#define	GR_NUMDBERS		37	/* number of db'ed windows */
#define	GR_NUMRGBERS		38	/* number of rgb'ed windows */
#define	GR_BLANKSCREEN		39	/* turn display on or off */
#define	GR_ONEMAP		40	/* go into one-map mode */
#define	GR_MULTIMAP		41	/* go into multi-map mode */
#define GR_SETMONITOR		42	/* change display format */
#define GR_GETMONITOR		43	/* read display format */
#define	GR_SETMAP		44	/* setmap gl routine	*/
#define	GR_GETMAP		45	/* getmap gl routine	*/
#define GR_GETCMM		46	/* get color map mode */
#define GR_SETDBLIGHTS		47	/* set the lights on the dial box */
#define GR_DBTEXT		48	/* set the text on the dial box */
#define GR_ISQUEUED		49	/* is a device queued or not? */

/* utilities used internally */
#define GR_WAITFORSWAP		50	/* wait for the swap interval */
#define GR_LOCK			52	/* grab the pipe for our use */
#define GR_UNLOCK		53	/* release the pipe */
#define	GR_SETCUROFFSET		55	/* set cursor offset */
#define	GR_SETCURSOR		56	/* set cursor */
#define	GR_RGBSETCURSOR		57	/* set rgb cursor offset */
#define	GR_GETCURSTATE		58	/* cursor on or off ? */
#define	GR_RGBGETCURSTATE	59	/* rgb cursor on or off ? */
#define	GR_FREEPAGES		60	/* number of free memory pages */
#define GR_SYSTYPE		61	/* UNIX or Vkernel? */
#define GR_WRITEMICRO		62	/* write microcode to FBC */
#define GR_SCRTIMEOUT		63	/* set the scr blank timeout */
#define	GR_WINSOFTINTR		64	/* add a char to the current textport */
#define	GR_KBDSOFTINTR		65	/* fake a char from raw keyboard */
#define	GR_CHANGEBUTTON		66	/* new button value */
#define	GR_CHANGEVALUATOR	67	/* new valuator value */
#define GR_MEKBMAN		69	/* register as the keyboard manager */

/* textport stuff */
#define	GR_TEXTINIT		70	/* init the textport */
#define	GR_TEXTREFRESH		71	/* refresh the textport */
#define	GR_TPON			72	/* textport on */
#define	GR_TPOFF		73	/* textport off */
#define GR_TEXTPORT		74	/* set the textport position */
#define GR_GETTEXTPORT		75	/* get the textport position */
#define	GR_TEXTWRITEMASK	76	/* textport writemask */
#define	GR_TEXTCOLOR		77	/* text color */
#define	GR_PAGECOLOR		78	/* page color */
#define	GR_PAGEWRITEMASK	79	/* page writemask */
#define GR_GETCHARINFO		80	/* get def height, width, descender */
#define GR_SETCHARINFO		81	/* set def height, width, descender */
#define GR_GETCHAROFFSETS	82	/* get offset table for def font */
#define GR_SETCHAROFFSETS	83	/* set offset table for def font */
#define GR_SETCHARMASKS		84	/* set the bits in the font def */
#define GR_GETNUMCHARS		85	/* get num characters in font 0 */

/* keyboard stuff */
#define	GR_CLKON		90	/* kbd click on */
#define	GR_CLKOFF		91	/* kbd click off */
#define	GR_LAMPON		92	/* kbd lamps on */
#define	GR_LAMPOFF		93	/* kbd lamps off */
#define	GR_SETBELL		94	/* duration of kbd bell */
#define	GR_RINGBELL		95	/* ring kbd bell */

/* window stuff */
#define GR_MEWMAN		100	/* register as the window manager */
#define GR_SEND			101	/* send to some other process */
#define	GR_REPLY		102	/* reply to a send */
#define GR_SHREAD 		103	/* read from someone's shmem */
#define	GR_SHWRITE		104	/* write to somebody else's shmem */
#define	GR_GFINPUTCHANNEL	105	/* attach input to a window */
#define	GR_GETGFPORT		106	/* get next free gfport */
#define	GR_PUTGFPORT		108	/* release gfport */
#define	GR_GETTXPORT		109	/* get next free txport # */
#define	GR_PUTTXPORT		110	/* release txport # */
#define	GR_SETPIECE		111	/* define window pieces */
#define	GR_SAFE			112	/* graphics hardware is unused */
#define	GR_RESERVEBITS		113	/* reserve a few upper bit planes */
#define	GR_TXINPUTCHANNEL	114	/* attach input to a textport */
#define	GR_RESERVEBUTTON	115	/* reserve a but for exclusive use */
#define GR_SETGFPORT		116	/* make a new gfport be current */
#define	GR_GETINCHAN		117	/* get next free inchan # */
#define	GR_PUTINCHAN		118	/* release inchan # */
#define GR_ISMEX		119	/* is mex running? */

/* feedback grioctls */
#define	GR_STARTFEED		120	/* start feedback */
#define	GR_ENDFEED		121	/* end feedback */
#define	GR_SETFBC		122	/* set fbc and ge flags */
#define	GR_GETFBC		123	/* get fbc and ge flags */

#define GR_CYCLEMAP		131	/* cyclemap user call */
#define GR_FONTMEM		132	/* set font ram size for this user */
#define GR_GETSHMEM		133	/* get some inputchanels shmem */
#define GR_GETCONFSW		134	/* read pmII config switches */
#define GR_SWAPANYTIME		135	/* don't wait for me to swap */
#define GR_GETADDRS		136	/* get some interesting addresses */
#define GR_GETOTHERMONITOR	137	/* get alternative monitor config */
#define	GR_SIGCON		138	/* signal a connection */
#define	GR_MODCON		139	/* modify a connection */
#define	GR_REPLYCON		140	/* unblock sender */

/* misc grioctls */
#define GR_GETDEV		160	/* sample multiple devices */
#define GR_DEVPORT		161	/* select serial port for a device */
#define GR_MOUSEWARP		162	/* set turbo mouse warp power */
#define GR_LPENSET		163	/* set light pen offset */
#define GR_ANYQENTER		164	/* put something in another's queue */

#ifdef CLOVER
/* clover emulation grioctls */
#define GR_SENDGE		200	/* send a smallbuf's worth of data */
#define GR_MESIMGUY		201	/* register the simulator process */
#endif

struct	grpiece {
	short	gr_xmin, gr_xmax;	/* x location of piece */
	short	gr_ymin, gr_ymax;	/* y location of piece */
};

/* header for GR_SETPIECE info (fillowed by gr_pieces # of grpiece structs) */
struct	grpiecehdr {
	unsigned char	gr_no;			/* port # */
	unsigned char	gr_type;		/* txport/gfport */
	unsigned short	gr_pieces;		/* # of pieces */
	unsigned short	gr_llx, gr_lly;		/* lower left x,y */
	unsigned short	gr_urx, gr_ury;		/* upper right x,y */
	unsigned short	gr_ncols, gr_nrows;	/* extents, in chars */
	short		gr_doredraw;		/* if set do redraw */
};
/* gr_type's */
#define	TXPORT	0
#define	GFPORT	1

struct errorrec {
	long errno;		/* error number */
	long severity;		/* error serverity */
	char *str;		/* printf format string */
	long slen;		/* format str length */
	long arg0,		/* 4 args */
	     arg1,
	     arg2,
	     arg3;
};

#endif GRIOCTLDEF
