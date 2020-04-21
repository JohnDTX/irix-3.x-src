/*
 *	Kurt Akeley			9/18/82
 *
 *	To be included in "console.c" and its routines.
 *
 *	Updates:
 *		9/18/82  KBA	Copied from console.c
 *		2/3/83	 KBA	Globals changed for more consistent colormap
 *				handling.
 *		4/19/83  KBA	Added list of external functions
 *		12/8/83  KBA	Removed variable declarations, now in main.c
 */

#ifndef FALSE
#define FALSE		0
#define TRUE		1
#endif

#define boolean		short
#define XMAX		1024
#define YMAX		768

#define MAXLINE 	100	/* size of the line buffer		*/
#define MACROSIZE	50	/* max number of commands in a macro	*/
#define MAXMACRO	10	/* max number of macros			*/
#define MAXALPHA	20	/* size of strings in alpha lists	*/

#define ONESTIPADDR	0x20	/* permanent all one stipple (init)	*/
#define CHECKSTIPADDR	0x30	/* permanent checkered stipple (init)	*/
#define STIPADDR	0x40	/* font memory address for stipples	*/

#define SINGLEBUFMODE(i) ((i&(DISPLAYA|DISPLAYB))==(DISPLAYA|DISPLAYB))
#define DISPLAYAMODE(i)	((i&(DISPLAYA|DISPLAYB))==(DISPLAYA))
#define DISPLAYBMODE(i)	((i&(DISPLAYA|DISPLAYB))==(DISPLAYB))

#define DCR(x)		{dc_dcr = (x); DCflags = dc_dcr;}
#ifdef UC4
#define UCR(x)		{uc_ucr = (x); *UCRAddr = uc_ucr;}
#endif UC4

typedef struct {short cfb;
		short edb;
		short ecb;
		short xsb;
		short xeb;
		short ysb;
		short yeb;
		short fmab;
		short rpb;
		short mdb;
		short ucr;
		short dcr;
		} Save;

/* global variables - declared in main.c */
extern short	dc_dcr;	/* interface.c */
extern short	colorindex;
extern short	weindex;	
extern long	colorcode;
extern long	wecode;
extern long	sigplanes;
extern long	installedplanes;
extern short	onestipple[];
extern short	checkstipple[];

/* functions */
extern long	bpctest ();
extern long	testplanes ();
extern long	comparemem ();
extern long	stripetest ();
extern long	fmtest ();
extern long	comparefm ();
extern long	maptest ();
extern long	comparemap ();
extern long	colorwetest ();
extern long	dcpaltest ();
extern long	dcrtest ();
extern long	linetest ();
extern long	viewporttest ();
extern long	recttest ();
extern long	chartest ();
extern long	planeword ();
extern long	planecode ();
extern long	newcode ();
extern long	rgbcode ();

extern short	bitcount ();
extern short	readfont ();
extern short	getindex ();
extern short	mapcode ();
extern short	maxindex ();
extern short	newindex ();

extern boolean	checkline ();
extern boolean	writefont ();
extern boolean	readmap ();
extern long	env[];
extern long	geterrors ();
extern long	bits ();
extern long	testall ();
