/*
** disk.h
**
**	$Source: /d2/3.7/src/stand/cmd/ipfex/RCS/disk.h,v $
**	$Revision: 1.1 $
**	$Date: 89/03/27 17:12:20 $
*/

struct dtypes {
	char	*tname;
	char	ttype;
#ifdef MDFEX
	unsigned char 	tdev;
#endif
	struct disk_label *tlabel;
};

char  *MULTIBASE;

#ifdef PM1
#define	PHYS(x)		((char *)(x & ~0100000))
#endif

#ifdef IPFEX
#define	BUFSIZE	(128*1024)
#define	BUF0	(((int)MULTIBASE))
#define	BUF1	((int)MULTIBASE+BUFSIZE)
#ifdef pmII
#define BIGBUFSIZE	((mdioport<<4)-BUF0)
#endif
#ifdef juniper
#define BIGBUFSIZE	(BUFSIZE*2)
#endif
#else
#define DEFDISK 1		/* Vertex */
#define	FLOPPY	2
#define	BUFSIZE	(256*1024)
#define	BUF0	(((int)MULTIBASE+0x200))
#define	BUF1	(((int)MULTIBASE+0x200+BUFSIZE))	/* BUF0+128K */
#define	BIGBUFSIZE	((ioport<<4)-BUF0)
#endif

#define	RETRY	0	/* Return vals from rsq() */
#define	SKIP	1	/*  (retry/skip/quit) */
#define	QUIT	2

long	quiet;
#define	QP0(s) if(!quiet) printf(s)
#define	QP1(s,a) if(!quiet) printf(s,a)
#define	QP2(s,a,b) if(!quiet) printf(s,a,b)
#define	QP3(s,a,b,c) if(!quiet) printf(s,a,b,c)

#ifdef IPFEX
#define NUNIT	4		/* Maximum Number of Units */
#else
#define	NUNIT	2		/* Maximum Number of Units */
#endif
