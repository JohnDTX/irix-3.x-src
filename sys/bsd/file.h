/*
|| BSD compatibility header
||
|| Under the SGI scheme, these things live in <sys/fcntl.h>,
|| but in the BSD scheme of things <sys/file.h> is the place.
*/

#ifdef mips
#ifdef KERNEL
#include "sys/types.h"
#include "../../sys/file.h"
#include "sys/fcntl.h"
#else
#include <sys/types.h>
#include "../../sys/file.h"
#include <sys/fcntl.h>
#endif

#else /* mips */
#include "../../sys/types.h"
#include "../../sys/file.h"
#include <sys/fcntl.h>		/* can be relative since BSD doesn't have one */
#endif /* mips */

/*
 * Access call.
 */
#define	F_OK		0	/* does file exist */
#define	X_OK		1	/* is it executable by caller */
#define	W_OK		2	/* writable by caller */
#define	R_OK		4	/* readable by caller */

/*
 * Lseek call.
 */
#define	L_SET		0	/* absolute offset */
#define	L_INCR		1	/* relative to current offset */
#define	L_XTND		2	/* relative to end of file */
