/*
 * Constants and structure definitions for the Silicon Graphics graphics
 * system call.
 *
 * $Source: /d2/3.7/src/sys/h/RCS/sgigsc.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:29:57 $
 */

/*
 * Machine independent operations
 */
#define	SGWM_LOCK	1		/* lock graphics hardware */
#define	SGWM_UNLOCK	2		/* unlock graphics hardware */
#define	SGWM_SIO	3		/* steal sio channel */
#define	SGWM_UNSIO	4		/* unsteal sio channel */
#define	SGWM_WMRUNNING	5		/* query window manager existance */
#define	SGWM_MEWM	6		/* declare process as window manager */
#define	SGWM_QMEM	7		/* map shared q */
#define	SGWM_UNQMEM	8		/* unmap shared q */
#define	SGWM_LOSTGR	9		/* test for graphics capability */
#define	SGWM_TIEMOUSE	10		/* tie mouse to cursor */
#define	SGWM_UNTIEMOUSE	11		/* untie mouse from cursor */

/*
 * Machine dependent operations
 */

/* IRIS 3000 series */
#define	SG3K_CONSOLE	0x77707770	/* stream ioctl code - see hack.c */

/* IRIS 4D/XX */
#define	SG4DXX_TAKECX	100		/* take a context away */
#define	SG4DXX_GIVECX	101		/* give a context to somebody */
#define	SG4DXX_GMWAIT	102		/* wait to complete state save */
#define	SG4DXX_UNLOADAREA 103		/* get acess to GM shared memory */

/*
 * Data structures for SGWM_QMEM operation.
 * All the data structures described below are assumed to be aligned on
 * a longword boundary.
 */

/*
 * This structure defines the argument for the SGWM_QMEM system call.
 */
struct	sgigsc_qmem {
	char	*base;			/* base address of q memory */
	long	entries;		/* number of q entries */
};

/*
 * This structure defines an entry in the q.  Each entry is defined to
 * be 8 longwords in length, aligned on a longword boundary.
 */
struct	sgigsc_qentry {
	struct {
		long	seconds;
		long	microseconds;
	} timeStamp;			/* time of event; relative to boot */

	long	event;			/* unique event identifier */

	union {
		long	value[5];		/* maximum data area */
		unsigned char ascii;		/* ascii code */
		int	pid;			/* process id */

		struct {
			long	x;		/* mouse x coordinate */
			long	y;		/* mouse y coordinate */
			long	buttons;	/* mouse button data */
		} mouse;

		struct {
			dev_t	dev;		/* unix device */
			unsigned char c;	/* character from sio device */
		} sio;
	} ev;				/* event value */
};

/* definitions for bits in of ev.mouse.buttons */
#define	SGE_MOUSE_RIGHTMOUSE	0x01
#define	SGE_MOUSE_MIDDLEMOUSE	0x02
#define	SGE_MOUSE_LEFTMOUSE	0x04
#define	SGE_LPEN_BUTTON		0x01

/*
 * This structure defines an actual q.  The first two fields are the index's
 * for the q entry and remove points.  The final field is a variable length
 * array containing the actual q entries.  The number of entries is defined
 * when the SGWM_QMEM call is given, via the sgigsc_mem.entries field.
 */
struct	sgigsc_q {
	long	qin;			/* index for next q enter */
	long	qout;			/* index for next q remove */
	struct sgigsc_qentry q[1];	/* actual q */
};

/*
 * Event identifiers (sgigsc_qentry.event).
 */
#define	SGE_PDEATH	1		/* graphics process death */
#define	SGE_PBLOCK	2		/* graphics process blocked */
#define	SGE_KB		3		/* keyboard data */
#define	SGE_MOUSE	4		/* mouse data */
#define	SGE_SIO		5		/* sio data */
#define	SGE_LPEN	6		/* light pen data */

#ifdef	KERNEL
struct	{
	short	flags;		/* random flags */
	struct	sgigsc_q *q;	/* kernel virtual addr of q */
	caddr_t	uvaddr;		/* users virtual addr of q */
	int	qlen;		/* length of total q memory, in bytes */
	int	pages;		/* # of pages used to map q */
	int	entries;	/* max # of entries in the q */
	struct	proc *wman;	/* proc pointer of window manager process */
	struct	pte *pte;	/* pte pointer to users base addr */
} sgstate;

/* flags */
#define	WMRUNNING	0x01	/* window manager is running */
#define	LOCKED		0x02	/* graphics are locked */
#define	QMEM		0x04	/* q memory is shared */

#endif	/* KERNEL */
