/*
 * $Source: /d2/3.7/src/sys/multibus/RCS/level.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:31:27 $
 */

/* level 1 devices -- NOT SHARED */
#include "md.h"

/* level 2 devices */
#include "nx.h"
#include "ex.h"
#include "nex.h"
#include "hy.h"

/* level 5 devices */
#include "sd.h"
#include "si.h"
#include "ip.h"
#include "tm.h"
#include "sq.h"
#include "ib.h"
#include "ik.h"
#include "ad.h"
#include "cd.h"

level1()
{
#if NDSD > 0
	dsdintr();
	return;
#else
	panic("stray multibus level 1 interrupt\n");
#endif
}


#define MAX_LEVEL2_STRAYS 20
#define MAX_LEVEL5_STRAYS 20
#define RETURN(cnt) {(cnt) = 0; return;}
int level2_bad, level5_bad;

level2()
{
#if NHY > 0
	if (hyintr())
		RETURN(level2_bad);
#endif
#if NNX > 0 && NNEX == 0
	if (nxintr())
		RETURN(level2_bad);
#endif
#if NEX > 0 || NNEX > 0
	if (exintr())
		RETURN(level2_bad);
#endif

	if (++level2_bad > MAX_LEVEL2_STRAYS) {
		panic("stray multibus level 2 interrupt\n");
	} else {
#ifdef OS_DEBUG
		iprintf("stray multibus level 2 interrupt #%d\n", level2_bad);
#endif
	}
}

/*
 * Call each interrupt routine in sequence until one of them accepts the
 * interrupt.  We order the calls in bandwidth terms, from highest interrupt
 * rate to lowest interrupt rate.
 */
level5()
{
	/*
	 * First list the disk drives
	 */
#if NSI > 0
	if (siiintr())
		RETURN(level5_bad);
#endif
#if NIP > 0
	if (ipintr())
		RETURN(level5_bad);
#endif
#if NSD > 0
	if (stdintr())
		RETURN(level5_bad);
#endif

	/*
	 * Then list the tape drives
	 */
#if NSQ > 0
	if (siqintr())
		RETURN(level5_bad);
#endif
#if NTM > 0
	if (tmtintr())
		RETURN(level5_bad);
#endif

	/*
	 * Then the miscellaneous stuff
	 */
#if NIB > 0
	if (tlcintr())
		RETURN(level5_bad);
#endif

#if NIK > 0
	if (ikcintr())
		RETURN(level5_bad);
#endif
#if NAD > 0
	if (adintr())
		RETURN(level5_bad);
#endif
#if NCD > 0
	if (cdintr())
		RETURN(level5_bad);
#endif

	if (++level5_bad > MAX_LEVEL5_STRAYS) {
		panic("stray multibus level 5 interrupt\n");
	} else {
#ifdef OS_DEBUG
		iprintf("stray multibus level 5 interrupt #%d\n", level5_bad);
#endif
	}
}
