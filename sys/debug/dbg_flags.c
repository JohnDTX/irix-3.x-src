/*
 * Debugging flags support.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/sys/debug/RCS/dbg_flags.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:27:42 $
 */
#define	DBG_NAMES
#include "../debug/debug.h"
#include "../h/param.h"

#ifdef	OS_DEBUG
char	dbg_flags[(MAXFLAGS + BITSPERBYTE - 1) / BITSPERBYTE];

extern	short dbg_inits[];

/*
 * dbgname:
 *	- return the symbolic name for the given flag
 */
char *
dbgname(flag)
	int flag;
{
	register struct dbg_name *dn;

	dn = &dbg_names[0];
	while (dn->symbol) {
		if (dn->flag == flag)
			return (dn->symbol);
		dn++;
	}
	return ("unknown name");
}

/*
 * dbginit:
 *	- initialize flags
 */
void
dbginit()
{
	register int i;
	register int flag;

	for (i = 0; (flag = dbg_inits[i]); i++) {
		iprintf("dbginit: '%s' flag is now on\n", dbgname(flag), flag);
		dbgon(flag);
	}
}

/*
 * dbgflagcheck:
 *	- return non-zero if the given flag is legit
 */
int
dbgflagcheck(flag)
	int flag;
{
	static char firsttime = 1;

	if (firsttime) {
		firsttime = 0;
		dbginit();
	}
	if ((flag < 0) || (flag > MAXFLAGS)) {
		printf("dbgflagcheck: illegal flag %d\n", flag);
		return (0);
	}
	return (1);
}

/*
 * dbgtest:
 *	- return non-zero if the given flag is set, otherwise return 0
 */
int
dbgtest(flag)
	register int flag;
{
	if (dbgflagcheck(flag))
		return (btst(dbg_flags, flag));
	return (0);
}

/*
 * dbgon:
 *	- turn a flag on
 */
void
dbgon(flag)
	register int flag;
{
	if (dbgflagcheck(flag))
		bset(dbg_flags, flag);
}

/*
 * dbgoff:
 *	- turn a flag off
 */
void
dbgoff(flag)
	register int flag;
{
	if (dbgflagcheck(flag))
		bclr(dbg_flags, flag);
}

#endif	/* OS_DEBUG */
