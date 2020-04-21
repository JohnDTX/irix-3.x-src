/*
* $Source: /d2/3.7/src/stand/include/RCS/sysmacros.h,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:13:55 $
*/

#define	btop(v)		((unsigned)(v) >> PGSHIFT)
#define	btopr(x)	(((x)+NBPG-1)>>PGSHIFT)
#define	ptob(v)		((caddr_t)(v) << PGSHIFT)
#define	vtoptv(v)	((u_long *)PTMAP_BASE + btop((v)))

#define	major(x)	(int)((unsigned)(x)>>8)
#define	minor(x)	(int)((x)&0377)
#define	makedev(x,y)	(dev_t)(((x)<<8) | (y))
