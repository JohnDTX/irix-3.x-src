/*
 * Every error record has a header as follows.
 */

struct errhdr {
        short   e_type;         /* record type */
        short   e_len;          /* bytes in record (with header) */
        time_t  e_time;         /* time of day */
};

/*
 * Error record types
 */

#define E_GOTS  010             /* Start for UNIX/TS */
#define E_GORT  011             /* Start for UNIX/RT */
#define E_STOP  012             /* Stop */
#define E_TCHG  013             /* Time change */
#define E_CCHG  014             /* Configuration change */
#define E_BLK   020             /* Block device error */
#define E_STRAY 030             /* Stray interrupt */
#define E_PRTY  031             /* Memory parity */

/*
 * Error logging startup record. One of these is
 * sent to the logging daemon when logging is
 * first activated.
 */


struct estart {
	struct utsname e_name;
        unsigned e_bconf;       /* block device configuration */
};

/*
 * Error logging termination record that is sent to the daemon
 * when it stops error logging.
 */

#define eend errhdr

/*
 * A time change record is sent to the daemon whenever
 * the system's time of day is changed.
 */

struct etimchg {
        time_t  e_ntime;        /* new time */
};

/*
 * A configuration change message is sent to
 * the error logging daemon whenever a block device driver
 * is attached or detached (MERT only).
 */

struct econfchg {
        char    e_trudev;       /* "true" major device number */
        char    e_cflag;        /* driver attached or detached */
};

#define E_ATCH  1
#define E_DTCH  0


/*
 * Template for the error record that is logged by block devices.
 * This record followed by an array of "e_nreg" structures which
 * contain 2 longs and then two character arrays which are for:
/*		long	draddr;		/* address of the register */
/*		long	drvalue;	/* value of the register */
/*		char	drname[];	/* name of the register */
/*		char	drbits[];	/* bit names or error interp of value */
/**/

struct eblock {
        dev_t   e_dev;          /* "true" major + minor dev number */
        unsigned e_bacty;       /* other block I/O activity */
        struct iostat e_stats;  /* unit I/O statistics */
        short   e_bflags;       /* read/write, error, etc */
        short   e_nreg;         /* number of device registers */
        daddr_t e_bnum;         /* logical block number */
        unsigned e_bytes;       /* number of bytes to transfer */
        paddr_t e_memadd;       /* buffer memory address */
        ushort e_rtry;          /* number of retries */
	struct pos {		/* where the block device the error occurred */
				/* set invalid fields to -1 */
		unsigned unit;
		unsigned cyl;
		unsigned trk;
		unsigned sector;
	} e_pos;
};

/*
 * Flags (selected subset of flags in buffer header)
 */

#define E_WRITE 0
#define E_READ  1
#define E_NOIO  02
#define E_PHYS  04
#define E_MAP   010
#define E_ERROR 020

/*
 * Template for the stray interrupt record that is logged
 * every time an unexpected interrupt occurs.
 */

struct estray {
        physadr e_saddr;        /* stray loc or device addr */
        unsigned e_sbacty;      /* active block devices */
};

/*
 * Memory parity error record that is logged whenever one
 * of those things occurs.  What can be done?  For now, just
 * save away the pc of the instr that caused the par. err.  Cant
 * really do much at all without hardware support like 11/70.
 */

struct eparity {
        int e_parreg;      /* memory subsystem registers */
};

/*
 * structure of data sent by block device to error logging routine
 */
struct deverreg {
	long	draddr;		/* register address */
	char	*drname;	/* register name */
	long	drvalue;	/* register value */
	char	*drbits;	/* meaning of value or bit names */
};
