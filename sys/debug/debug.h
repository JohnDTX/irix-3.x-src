#ifndef	__DEBUG_H__
#define	__DEBUG_H__

/*
 * Kernel debugging support
 *
 * $Source: /d2/3.7/src/sys/debug/RCS/debug.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:27:45 $
 */
#include "../debug/trace.h"
#include "../debug/assert.h"
#include "../debug/flags.h"
#include "../debug/meter.h"

/* globals */
extern	short kswitch;
extern	short kdebug;

/* backwards compatability */
#ifdef	OS_DEBUG
#undef	DEBUG
#define	DEBUG
#endif

#endif	/* __DEBUG_H__ */
