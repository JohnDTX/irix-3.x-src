/*
 * Pty ioctls
 *
 * $Source: /d2/3.7/src/sys/h/RCS/pty_ioctl.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:29:54 $
 */

#define	PTIOC(x)		(('p'<<8)|(x))
#define	PTIOC_QUEUE		PTIOC(0)	/* mark pty as queued */

/*
 * Queue defines...These should be in "device.h"
 */
#define	QPTYCOUNT	3
#define	QPTYOFFSET	2000
#define	QPTY_CANREAD	(2000)	/* can read from slave */
#define	QPTY_STOP	(2001)	/* user typed ^S */
#define	QPTY_START	(2002)	/* user typed ^Q */
