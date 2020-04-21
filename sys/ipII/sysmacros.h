#ifndef	ctos
/*
 * Machine dependent macros
 *
 * $Source: /d2/3.7/src/sys/ipII/RCS/sysmacros.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:31:01 $
 */

/* Core clicks (4096 bytes) to segments and vice versa */
#define	ctos(x)	(x)
#define	stoc(x)	(x)

/* Core clicks (4096 bytes) to disk blocks (1k bytes) */
#define	ctod(x)	((x)<<3)
#define	dtoc(x)	((x)>>3)
#define	dtob(x)	((x)<<9)

/* clicks to bytes */
#define	ctob(x)	((x)<<12)

/* bytes to clicks */
#define	btoc(x)	(((unsigned)(x)+4095)>>12)

#endif	ctos
