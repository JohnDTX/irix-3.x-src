/*
 * Kernel tracing package.
 *
 * $Source: /d2/3.7/src/sys/debug/RCS/trace.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:27:47 $
 */

#ifdef	OS_TRACE

#define	DBG(x)		dbgprintf x

extern	void	dbgprintf();

#else	/* OS_TRACE */

#define	DBG(x)

#endif	/* OS_TRACE */
