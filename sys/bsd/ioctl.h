/* 4.3 compatibility hack
 */

#ifndef	_IOCTL_
#define	_IOCTL_

#ifdef KERNEL
#ifdef mips
#include "../bsd/sys/ttychars.h"
#include "../net/soioctl.h"
#else /* mips */
#include "sys/ttychars.h"
#include "net/soioctl.h"
#endif /* mips */

#else /* kernel */
#include <sys/ttychars.h>
#include <net/soioctl.h>
#endif

/* 4.2-like IOCTLs defined in termio.h */
#ifndef TIOCFLUSH
#define TIOCFLUSH TCFLSH
#endif
#endif
