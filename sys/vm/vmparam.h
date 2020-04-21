/*	vmparam.h	6.1	83/07/29	*/

/*
 * Machine dependent constants
 */
#include "machine/vmparam.h"

#if defined(KERNEL) && !defined(LOCORE)
int	klseql;
int	klsdist;
int	klin;
int	kltxt;
int	klout;
#endif
