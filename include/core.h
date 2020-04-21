/*
 * $Source: /d2/3.7/src/include/RCS/core.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:11:14 $
 */

/* machine dependent stuff for core files */

#if vax
#define TXTRNDSIZ 512L
#define stacktop(siz) (0x80000000L)
#define stackbas(siz) (0x80000000L-siz)
#endif

#if pdp11
#define TXTRNDSIZ 8192L
#define stacktop(siz) (0x10000L)
#define stackbas(siz) (0x10000L-siz)
#endif

#if u3b
#define TXTRNDSIZ 0x20000
#define stacktop(siz) 0xC0000
#define stackbas(siz) (0xC0000 + siz)
#endif

#if sgi
#include <machine/vmparam.h>
#define	TXTRNDSIZ	0x1000
#define	stacktop(siz)	USRSTACK
#define	stackbas(siz)	(USRSTACK - siz)
#endif
