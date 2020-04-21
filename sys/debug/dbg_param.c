/*
 * Initial debugging state.
 *
 * $Source: /d2/3.7/src/sys/debug/RCS/dbg_param.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:27:43 $
 */
#include "../debug/debug.h"

#ifdef	OS_DEBUG
/*
 * XXX all of this stuff should be setup via a debug monitor, which is run
 * XXX before the kernel is run.  Also, the kernel debugger should be able
 * XXX to get at the flags directly.
 */
short	kswitch = 0;

/*
 * List of initially on debugging flags
 */
short	dbg_inits[] = {
	0,
};

#endif	/* OS_DEBUG */
