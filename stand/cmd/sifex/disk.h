/*
** disk.h
**
**	$Source: /d2/3.7/src/stand/cmd/sifex/RCS/disk.h,v $
**	$Revision: 1.1 $
**	$Date: 89/03/27 17:13:11 $
*/

struct dtypes {
	char	*tname;
	char	ttype;
	struct disk_label *tlabel;
};

char  *MULTIBASE;

#define	PHYS(x)		((char *)(x & ~0100000))
#define DEFDISK 1		/* Vertex */
#define	BUFSIZE	(256*1024)
#define	BUF0	(((int)MULTIBASE+0x200))
#define	BUF1	(((int)MULTIBASE+0x200+BUFSIZE))	/* BUF0+64K */

#define	RETRY	0	/* Return vals from rsq() */
#define	SKIP	1	/*  (retry/skip/quit) */
#define	QUIT	2

long	quiet;
#define	QP0(s) if(!quiet) printf(s)
#define	QP1(s,a) if(!quiet) printf(s,a)
#define	QP2(s,a,b) if(!quiet) printf(s,a,b)
#define	QP3(s,a,b,c) if(!quiet) printf(s,a,b,c)

#define	NUNIT		4		/* Maximum Number of Units */
#define NCONTROLLERS	2		/* Maximum Number of Controllers */
