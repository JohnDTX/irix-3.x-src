/*
 * Each block device has a iobuf, which contains private state stuff
 * and 2 list heads: the b_forw/b_back list, which is doubly linked
 * and has all the buffers currently associated with that major
 * device; and the d_actf/d_actl list, which is private to the
 * device but in fact is always used for the head and tail
 * of the I/O queue for the device.
 * Various routines in bio.c look at b_forw/b_back
 * (notice they are the same as in the buf structure)
 * but the rest is private to each device driver.
 *
 * $Source: /d2/3.7/src/sys/h/RCS/iobuf.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:29:37 $
 */
struct iobuf {
	long	b_flags;		/* see buf.h */
	struct	buf *b_forw;		/* reserved for drivers */
	struct	buf *b_back;		/* reserved for drivers */
	struct	buf *b_actf;		/* head of I/O queue */
	struct 	buf *b_actl;		/* tail of I/O queue */
	dev_t	b_dev;			/* major+minor device name */
	char	b_active;		/* busy flag */
	char	b_errcnt;		/* error count (for recovery) */
	short	io_s1;			/* space for drivers to leave things */
	short	io_s2;			/* ... */
	short	io_s3;			/* ... */
};

/* standard uses for drive fields */
#define	io_unit		io_s1		/* unit of command */
#define	io_cylin	io_s2		/* cylinder of command */
#define	io_type		io_s3		/* type of device */
