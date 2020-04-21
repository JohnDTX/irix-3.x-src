/*
 * Processor status register
 *
 * $Source: /d2/3.7/src/sys/pmII/RCS/psr.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:33:47 $
 */

#define	PS_C	0x1		/* carry bit */
#define	PS_V	0x2		/* overflow bit */
#define	PS_Z	0x4		/* zero bit */
#define	PS_N	0x8		/* negative bit */
#define	PS_X	0x10		/* extend bit */
#define	PS_IPL	0x700		/* interrupt priority level */
#define	PS_SUP	0x2000		/* supervisor mode */
#define	PS_T	0x8000		/* trace enable bit */
