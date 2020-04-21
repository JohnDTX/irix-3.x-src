/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)dbm.h	5.2 (Berkeley) 85/06/26
 */

#ifndef NULL
/*
 * this is lunacy, we no longer use it (and never should have
 * unconditionally defined it), but, this whole file is for
 * backwards compatability - someone may rely on this.
 */
#define	NULL	((char *) 0)
#endif

#include "ndbm.h"

datum	fetch();
datum	firstkey();
datum	nextkey();
void	dbmclose();
#if 0
datum	makdatum();
datum	firsthash();
long	calchash();
long	hashinc();
#endif
