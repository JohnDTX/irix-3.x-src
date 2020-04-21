/*
 * $Source: /d2/3.7/src/include/RCS/stand.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:11:56 $
 */

/*
 * Header file for standalone package
 */

#ifdef vax
#define	FsTYPE	2
#else
#define	FsTYPE	1
#endif
#include "errno.h"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/inode.h"

/*
 * I/O block flags
 */

#define F_READ	01
#define F_WRITE	02
#define F_ALLOC	04
#define F_FILE	010

/*
 * Request codes -- must be
 * the same as an F_XXX above
 */

#define	READ	1
#define	WRITE	2

/*
 * Buffer sizes
 */

#if FsTYPE == 2
#define BLKSIZ	1024
#else
#define BLKSIZ	512
#endif
#define NAMSIZ	60

/*
 * devsw table --
 * initialized in conf.c
 */

struct devsw {
	int	(*dv_strategy)();
	int	(*dv_open)();
	int	(*dv_close)();
};
extern struct devsw _devsw[];

/*
 * dtab table -- entries
 * are created by MKNOD
 */

#define NDEV	8

struct dtab {
	char		*dt_name;
	struct devsw	*dt_devp;
	int		dt_unit;
	daddr_t		dt_boff;
};
extern struct dtab _dtab[];

/*
 * mtab table -- entries
 * are created by mount
 */

#define NMOUNT	4

struct mtab {
	char		*mt_name;
	struct dtab	*mt_dp;
};
extern struct mtab _mtab[];

/*
 * I/O block: includes an inode,
 * cells for the use of seek, etc,
 * and a buffer.
 */

#define NFILES	6

struct iob {
	char		i_flgs;
	struct inode	i_ino;
	time_t		i_atime;
	time_t		i_mtime;
	time_t		i_ctime;
	struct dtab	*i_dp;
	off_t		i_offset;
	daddr_t		i_bn;		/* disk block number (physical) */
	char		*i_ma;
	int		i_cc;		/* character count */
	char		i_buf[BLKSIZ];
};
extern struct iob _iobuf[];

#ifdef pdp11
/*
 * Set to which 32Kw segment the code is physically running in.
 * Must be set by the user's main (or thereabouts).
 */

int	segflag;
#endif

/*
 * Set to the error type of the routine that
 * last returned an error -- may be read by perror.
 */

extern int errno;
#define	RAW	040
#define	LCASE	04
#define	XTABS	02
#define	ECHO	010
#define	CRMOD	020
#define B300	7
struct sgttyb {
	char	sg_ispeed;		/* input speed */
	char	sg_ospeed;		/* output speed */
	char	sg_erase;		/* erase character */
	char	sg_kill;		/* kill character */
	int	sg_flags;		/* mode flags */
};
