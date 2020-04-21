/* @(#)sysmacros.h	1.3 */
/* $Source: /d2/3.7/src/include/RCS/sysmacros.h,v $ */
/* @(#)$Revision: 1.1 $ */
/* $Date: 89/03/27 16:12:06 $ */
/*
 * Some macros for units conversion
 */

#ifndef	SYSMACROS_H

#define	SYSMACROS_H

/* Core clicks to segments and vice versa */
#define ctos(x) (((int)(x)+((1<<(SEGSHIFT-PAGESHIFT))-1))>>(SEGSHIFT-PAGESHIFT))
#define stoc(x) ((int)(x)<<(SEGSHIFT-PAGESHIFT))

/* Core clicks to disk blocks and vice versa */
#define	ctod(x)	((x)<<(PAGESHIFT-BSHIFT))
#define dtoc(x) ((x)>>(PAGESHIFT-BSHIFT))

/* Bytes to disk blocks nd vice versa */
#define	btod(x)	(((x)+(BSIZE-1))>>BSHIFT)
#define	dtob(x)	((x)<<BSHIFT)

/* inumber to disk address */
#define	itod(x)	(daddr_t)(((unsigned)(x)+(2*INOPB-1))>>INOSHIFT)

/* inumber to disk offset */
#define	itoo(x)	(int)(((unsigned)(x)+(2*INOPB-1))&(INOPB-1))

/* clicks to bytes */
#define	ctob(x)	((x)<<PAGESHIFT)

/* bytes to clicks */
#define btoc(x)		((((x)+((1<<PAGESHIFT)-1))>>PAGESHIFT)&ADDRMASK)
#define	btoct(x)	(((x)>>PAGESHIFT)&ADDRMASK)

/* major part of a device */
#define	major(x)	(int)((unsigned)(x)>>8)
#define	bmajor(x)	(int)(((unsigned)(x)>>8)&037)
#define	brdev(x)	((x)&0x1fff)

/* minor part of a device */
#define	minor(x)	(int)((x)&0377)

/* make a device number */
#define	makedev(x,y)	(dev_t)(((x)<<8) | (y))

#endif	SYSMACROS_H
