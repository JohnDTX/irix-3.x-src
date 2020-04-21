/*
 * Metering support.
 *
 * $Source: /d2/3.7/src/sys/debug/RCS/meter.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:27:47 $
 */

#ifdef	OS_METER
#define	METER(x)	(x)
#else
#define	METER(x)
#endif
