#ifndef  GLTYPESDEF
#define  GLTYPESDEF
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

#ifndef NULL
#define NULL	0
#endif

#ifndef KERNEL		/* userland stuff */

typedef struct cons {
    struct cons	*link;
    short	*item;
} cons;

#define INITOBJCHUNKSIZE 1020
#define HASHTABLESIZE	256		/* must be a power of 2 */

typedef struct hashrec {
    struct hashrec	*link;
    short		*item;
    Tag			tag;
    Object		obj;
} hashrec;

typedef struct objhdr {
    short	*head;	/* ptr to first valid chunk of disp list */
    short	valid;	/* == 0 ==> object deleted. */
    cons	*chunks;	/* linked list of chunks of data */
    long	datasize;	/* # shorts of real data */
    long	physicalsize;	/* total physical shorts */
    cons	*tags;		/* list of tags */
    cons	*strings;	/* for reclaiming strings */
    short	*tailptr;	/* ptr to end of object */
    short	*tailend;	/* end of last chunk */
} objhdr;

#endif KERNEL	/* end of userland stuff */


#ifdef KERNEL	/* kernelland stuff */

#define BLINK_EVENT		1
#define TIMER_EVENT		2

#define MAX_RETRACE_EVENTS 20

typedef union {
    struct {
	short 	colorindex;
	short	currentcolor; 
	short	r1, g1, b1;
	short   r0, g0, b0;
    } colors;
    struct {
	short  dnum;
    } timer;
} retracedata;

typedef struct retrevent {
    struct retrevent 	*next;  /* link to next one */
    short 		count;	/* clicks left 'til this event is called */
    short		repeat;	/* clicks between events of this type */
    struct inputchan    *ic;	/* input channel owning this event */
    short		type;
    retracedata		data;
} retrevent;

typedef struct {
    short 	type;
    short 	value;
} queueentry;

#define BUFFER_SIZE 50	/* one more than number of type-value pairs */

typedef struct {
    struct inputchan	*reserved;
    char 		state;		/* 1 = down; 0 = up */
} buttondata;

typedef struct {
    short 		value;
    short 		minval;
    short		maxval;
    long 		offset;
    long		raw;
    long		halfmaxraw;
    short		absdevice;
} valuatordata;

typedef struct {
    char 		doqueue;	/* do queue changes? */
    unsigned short 	tiedevice1;	/* valuator device number (or zero) */
    unsigned short 	tiedevice2;	/* valuator device number (or zero) */
} procbuttondata;

typedef struct {
    char 		doqueue;	/* do queue changes? */
    short 		oldvalue;
    short 		noise;
} procvaluatordata;

typedef struct {
    char 		doqueue;	/* do queue changes? */
    short 		noise;
} proctimerdata;

typedef struct connection {
	struct connection *next;
	struct inputchan *ic;
	short dev;
} connection;

#endif KERNEL

typedef struct fontrec {
	struct fontrec *next;
	long index;		/* fontrec index */
	short offset;		/* where thing is in fontram */
	short size;		/* how many words it takes up */
	Fontchar *chars;	/* where character descriptions are */
	long maxheight;		/* maximum character height	*/
	long maxdescender;	/* maximum descender */
	long maxwidth;		/* maximum width */
	short maxchars;		/* maximum number of characters */
} fontrec;

struct matdata {
    short		dummy;		/* GEstoremm command	*/
    Matrix		mat		/* matrix data		*/
};

struct vpdata {
    long 		vcx, vsx;		/* viewport data	*/
    long		vcy, vsy;
    long		vcz, vsz;
    long		vcs, vss;
    short 		llx,lly,urx,ury;	/* screen mask coords	*/
};

struct atrdata {
    long 		myconfig;		/* UC4 cfr		*/
    short		mylstyle,mytexture;	/* linestyle, texture	*/
    unsigned short 	mycolor, mywenable;	/* color and writemask	*/
    unsigned short	mylwidth, mylsrepeat;	/* line width and repeat*/
    fontrec		*currentfont;		/* pointer to fontrec	*/
    short		myr,myg,myb;		/* RGB colors		*/
    short		myrm,mygm,mybm;		/* RGB writemasks	*/
};

/* this struct is used by both user and kernel */

#define MAXWSPIECES 	70	/* carefull! */

typedef struct windstate {
    long		gpos[4];		/* holds graphics pos	*/
    short		cpos[2];		/* holds char pos	*/
    short		validcpos;		/* and valid/invalid	*/
    short		mybackface, myzbuffer;	/* mode settings	*/
    Colorindex		imin, imax;		/* shade range parameters */
    short		z1, z2;			/* shade range params */
    short		a, b;			/* FBC shade parameters	*/
    short		zmin, zmax;		/* setdepth params */
    unsigned short	mylscode;		/* ls bits	*/
    short		mytexcode;		/* fontram adr	*/

    short		xmin, ymin, xmax, ymax;	/* screen coords of window */
    short		numrects;		/* number of pieces	*/
    short		rectlist[4*MAXWSPIECES];/* coords of each piece	*/

    struct vpdata 	*vpstatep;		/* viewport/screenmask stack*/
    struct vpdata	vpstack[VPSTACKDEPTH];
    struct vpdata	curvpdata;		/* current data		*/

    struct matdata	*matrixstatep;		/* matrix stack overflow */
    long 		matrixlevel;
    long		hdwrstackbottom, softstacktop;
    struct matdata	matrixstack[MATRIXSTACKDEPTH];

    struct atrdata	*attribstatep;		/* attribute stack	*/
    struct atrdata	attribstack[ATTRIBSTACKDEPTH];
    struct atrdata	curatrdata;		/* current data		*/

    long	bitplanemask;		/* used to mask color indices	*/
    short	fontrambase;		/* base address of font stuff	*/
    short	fontramlength;		/* size of font stuff		*/
    /* the next 3 are only till the micro code takes absolute offset */
    short	patternbase;
    short	cursorbase;
    short	fontbase;
} windowstate;

#endif GLTYPESDEF
