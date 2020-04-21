#ifdef	GL2
error error; don't use this file for GL2
#endif

/*
 * Variables and constants used by unix to manipulate the graphics hardware
 */

#define	XMAXSCREEN	1023
#define	YMAXSCREEN	767
#define	NQUEUES		100		/* # of items possible in queue */
#define	WNTIMEOUT	(10*60)		/* # of seconds before screen blanks */

/* random variables */
short	gefound;			/* # of ge chips found in gefind */
short	gemask;				/* mask of ge chips found << 1 */
long	gezero;				/* a long word of zero's */
short	fbcalive;			/* existance of fbc */
short	fbcstatus;			/* status of fbc */
ushort	fbccursor;			/* cursor state machine */
time_t	grlastupdate;			/* time last wn_redisplay was done */
short	gl_dialboxinuse;		/* dial box is in use */
struct	proc *grproc;			/* active graphics process */
short	shmem_pa;			/* base phys address of shared mem */
short	shmem_pa1;			/* base phys address of feedback mem */
short	gr_safe;			/* true if graphics proc is trusted */
short	gr_tpblank;			/* textport (console txport) is off */
short	keypadmode;			/* mode of key pad on keyboard */
short	kbsetup;			/* kb has been setup */
char	gl_version[];			/* vers of gl this kernel supports */

/* configuration parameters for FBCconfig */
short	gr_blanked;			/* screen is blanked due to no usage */
#define	CONFIGOFF	0xD0		/* blanked display (A & B off) */
#define	CONFIGBOTH	0xDF		/* A & B on */
#define	CONFIGMASK	0x03		/* display A & B bits */

/* kernel graphics state */
short	kgr;
#define	KGR_BUSY	0x01		/* kernel is doing graphics */
#define	KGR_DIDRESET	0x02		/* kgreset attempted */
#define	KGR_REPAINTING	0x04		/* in tx_redisplay */
#define	KGR_WANTED	0x08		/* kernel wants graphics */

/* display mode being used */
short	gr_mode;			/* form of display to generate */
#define	MD_SINGLE	0		/* single buffer */
#define	MD_DOUBLE	1		/* double buffer */
#define	MD_RGB		2		/* rgb */

/* i/o queue managed by the kernel */
struct	queueentry {
	short	type;			/* type of queue entry */
	short	value;			/* value of queue entry */
};
struct	queueentry queue[NQUEUES];
short	nqueued;			/* # of items in queue */
struct	queueentry *queuein;		/* input pointer */
struct	queueentry *queueout;		/* output pointer */
short	queuewaiting;			/* someone is waiting for queue data */

/* mouse stuff */
short	_mousex, _mousey;		/* mouse location */
short	mousebusy;			/* busy flag */
short	cursorx, cursory;		/* cursor location */

/* bpcxlate */
long (*bpcxfunc)();
# define bpcxlate(x)	(*bpcxfunc)(x)
