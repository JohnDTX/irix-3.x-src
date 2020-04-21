/* @(#)elog.h	1.1 */
/*
 * "True" major device numbers. These correspond
 * to standard positions in the configuration
 * table, but are used for error logging
 * purposes only.
 */

#define RK0	0
#define RP0	1
#define RF0	2
#define TM0	3
#define TC0	4
#define HP0	5
#define HT0	6
#define HS0	7
#define RL0	8
#define	HP1	9
#define	HP2	10
#define	HP3	11

/*
 * IO statistics are kept for each physical unit of each
 * block device (within the driver). Primary purpose is
 * to establish a guesstimate of error rates during
 * error logging.
 */

struct iostat {
	long	io_ops;		/* number of read/writes */
	long	io_misc;	/* number of "other" operations */
	ushort io_unlog;	/* number of unlogged errors */
};

/*
 * structure for system accounting
 */
struct iotime {
	struct iostat ios;
	long	io_bcnt;	/* total blocks transferred */
	time_t	io_act;		/* total controller active time */
	time_t	io_resp;	/* total block response time */
};
#define	io_cnt	ios.io_ops
