/*
 * BSD compatibility header
 */

#define	MAXPATHLEN	1024	/* maximum pathname length */


#ifdef mips			/* only necessary for S5R3 stuff */

#ifdef KERNEL
#include "sys/types.h"
#else
#include <sys/types.h>
#endif
#include "../../sys/param.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#else	/* mips */
				/* prevent problems in IRIS kernels */
#ifdef KERNEL
? ? ?	/* do not use this in the IRIS kernel */
#else
/*
|| Include the real /usr/include/sys/param.h.
|| This is ugly, but the alternatives are uglier still.
*/
#include "../../sys/param.h"

#endif /* KERNEL */
#endif /* mips */
