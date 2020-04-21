/*
 * Data structure containing the information needed to manipulate a
 * valuator
 */
struct	valuatordata {
	short	noise;
	short	value;
	short	minvalue;
	short	maxvalue;
	short	queue;
};

/*
 * Structure describing each key in each distinct state
 */
struct	button {
	char	state;			/* state of button */
	ushort	tiedevice1;		/* valuator device number (or zero) */
	ushort	tiedevice2;		/* valuator device number (or zero) */
	short	queue;			/* queue button changes */
};
#define	NVALUATORS	14		/* # of kernel supported valuators */
#define	NBUTTONS	143		/* # of kernel supported buttons */

/* version stuff */
#define	MAXVERSION	20
#define	VERSIONSTRING	"WS 2.3"

/*
 * "Shared" memory data structure for cooperative manipulation of the
 * graphics hardware, between the kernel and the user
 */
#define	TOKENBUFSIZE	2000
struct	shmem {
	ushort	ge_status;		/* current ge status */

	short	IdleMode;		/* whether or not interp is running */
	short	autocursor;		/* cursor should be displayed */
	short	GE_mask;		/* mask of ge chips found << 1 */
	short	GE_found;		/* # of ge chips found */

	short	RetraceExpected;	/* user code is expecting a retrace */
	short	RetraceReceived;	/* count of interrupts recieved */
	short	EOFpending;		/* flag for waiting until interrupt */
	short	EOFcode;		/* type of eof returned */
	short	EOFbits;		/* ??? */

	short	clippnt;		/* flag for feedback of polylines */
	short	firstfeed;		/* flag for feeding polygons */
	short	*intbuf;		/* pointer to current feedback buf */
	char	*inttoken;		/* drawing info from feedback */
	long	intbuflen;		/* size of feedback buffer */
	short	hitbits;		/* hit bits from fbc */

    /* stuff for valuators and queues */
	struct	valuatordata valuators[NVALUATORS];
	struct	button Buttons[NBUTTONS];
	short	cursorxvaluator;	/* valuators for cursor x */
	short	cursoryvaluator;	/*   and cursor y */
	short	queuedkeyboard;		/* if entire keyboard is queued */

    /* prototype feedback buffer area sizes and locations */
	long	p_intbufsize;
	long	p_inttokensize;
	short	*p_intbuf;
	char	*p_inttoken;

    /* inttoken buffer */
	char	inttokenbuf[TOKENBUFSIZE];
    
    /* additional junk to be reorganized latter */
	long	gr_retraces;		/* total retraces since reboot */
	short	gr_cfr;			/* soft register of update controler */
	short	gr_dcr;			/* soft register display controller */
	short	gr_displaymode;		/* current display mode */
	short	gr_colormode;		/* current colormap mode */

    /* version id */
	char	version[MAXVERSION];	/* symbol version id */
};
