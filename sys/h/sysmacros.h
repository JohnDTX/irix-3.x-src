#ifndef	__SYSMACROS__
#define	__SYSMACROS__
/*
 * Some macros for units conversion
 *
 * $Source: /d2/3.7/src/sys/h/RCS/sysmacros.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:30:06 $
 */

#include "machine/sysmacros.h"

/* inumber to disk address */
#ifdef INOSHIFT
#define	itod(x)	(daddr_t)(((unsigned)x+(2*INOPB-1))>>INOSHIFT)
#else
#define	itod(x)	(daddr_t)(((unsigned)x+(2*INOPB-1))/INOPB)
#endif

/* inumber to disk offset */
#ifdef INOSHIFT
#define	itoo(x)	(int)(((unsigned)x+(2*INOPB-1))&(INOPB-1))
#else
#define	itoo(x)	(int)(((unsigned)x+(2*INOPB-1))%INOPB)
#endif

/* major part of a device */
#define	major(x)	((unsigned short)((unsigned)(x)>>8))
#define	bmajor(x)	((unsigned short)(((unsigned)(x)>>8)&037))
#define	brdev(x)	((x)&0x1fff)

/* minor part of a device */
#define	minor(x)	((unsigned short)(x&0377))

/* make a device number */
#define	makedev(x,y)	((dev_t)(((x)<<8) | ((y)&0377)))

#endif	__SYSMACROS__
