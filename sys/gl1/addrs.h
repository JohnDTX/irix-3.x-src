/*
 * Common addressing constants for the gl1 graphics system
 *
 * $Source: /d2/3.7/src/sys/gl1/RCS/addrs.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:28:08 $
 */

#ifndef	__ADDRS__

/*
 * Location of the gf1 on the multibus
 */
#define GFMBM(x)	(MBIO_VBASE + (int)(x))

/*
 * Location of the ge port on the pmII
 */
#define	GEPORT		0xFD4000

/*
 * Definition for the DC3 display controller.
 */
#define DCMBM(x)	(MBIO_VBASE + (x))

#ifdef	KERNEL

/*
 * If the compiler worked, this is what we would do:
#define gewait()	while(GEFlags < 0)
 */

/*
 * If the assembler worked, this is what we would do:
#define	gewait()	asm("	tstw	0xDE1C00"); \
			asm("	jmi	.-6");
 */

/*
 * Since the assembler doesn't work, this is what we do:
 */
#if	MBIO_VBASE == 0xDE0000
#define	gewait()	asm("	.word	0x3039"); \
			asm("	.long	0xDE1C00"); \
			asm("	.word	0x6BF8");
#endif	MBIO_VBASE
#if	MBIO_VBASE == 0x50000000
#define	gewait()	asm("	.word	0x3039"); \
			asm("	.long	0x50001C00"); \
			asm("	.word	0x6BF8");
#endif	MBIO_VBASE
#ifndef gewait
HELP GEWAIT
#endif  gewait

#endif	KERNEL

#endif	__ADDRS__
