/*
* $Source: /d2/3.7/src/stand/include/RCS/stand.h,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:13:51 $
*/

#include "errno.h"
#include "types.h"
#include "inode.h"
#include "buf.h"

#define	NULL	0

/*
** Io block: includes pointers to
** an inode and a buf structure.
** A buffer area is also included.
*/
#define	NFILES	4

/*
** dummy for now till SGI FS
#define	SBSIZE		512
#define	DEV_BSIZE	512
*/
#define DBLOCK		512
#define DBSHIFT		9
#define	MAXBSIZE 	1024

struct	iob {
	int	i_flgs;		/* see F_ below			*/
				/* udot like stuff:		*/
	caddr_t	i_base;		/* base for io			*/
	unsigned i_count;	/* count for io			*/
	off_t	i_offset;	/* seek offset into file	*/
	int	i_error;	/* error code, if any		*/
				/* pointers:			*/
	struct	inode *i_ip;	/* inode pointer		*/
	struct	buf *i_bp;	/* a buf structure pointer	*/
};


#define F_READ		0x1	/* file opened for reading	*/
#define F_WRITE		0x2	/* file opened for writing	*/
#define F_ALLOC		0x4	/* buffer allocated		*/
#define	F_NET		0x10
#define	F_TAPE		0x20
#define	F_DISK		0x40
#define	F_PROM		0x80
#define	F_CPIO		0x100	/* file is a cpio archive	*/
#define	F_EOF		0x80000000 /* eof indicator, for networks	*/

/*
** Device switch table
*/
struct devsw {
	char	*dv_name;
	int	dv_flags;
	int	(*dv_strategy)();
	int	(*dv_open)();
	int	(*dv_close)();
	int	(*dv_ioctl)();
};

extern struct devsw	devsw[];
extern struct iob	iobuf[];
extern struct buf	bufs[];
extern struct inode	inodes[];
extern char		motormouth;

/*
** Request codes. Must be the same a F_XXX above
*/
#define	READ	1
#define	WRITE	2

/* Delay loop for times when you can't use real time clock	*/
#define	DELAY(n)	{ register ulong N = (n); while (--N); }
#define min(a,b)	( (a)<(b)?(a):(b) )
#define max(a,b)	( (a)>(b)?(a):(b) )
