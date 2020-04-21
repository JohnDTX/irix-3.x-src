/* mach_dep.h @+++@ */
/*
 * machine dependicies
 *
 * 8/29/84 (Berkeley) @(#)mach_dep.h	1.2
 */
/*
 * where scorefile should live
 */
#define SCOREFILE	"/usr/games/lib/rogue_gal"

/*
 * Variables for checking to make sure the system isn't too loaded
 * for people to play
 */

/* #define MAXUSERS	05	/* max number of users for this game */
/* #define MAXLOAD		25	/* 10 * max 15 minute load average */

#if MAXUSERS|MAXLOAD
#define	CHECKTIME	05	/* number of minutes between load checks */
				/* if not defined checks are only on startup */
#endif

#ifdef MAXLOAD
#define	LOADAV			/* defined if rogue should provide loadav() */

#ifdef LOADAV
#define	NAMELIST "/vmunix"	/* where the system namelist lives */
#endif
#endif

#ifdef MAXUSERS
#define	UCOUNT			/* defined if rogue should provide ucount() */

#ifdef UCOUNT
/* #define	UTMP	"/etc/utmp"	/* where the utmp file lives */
#endif
#endif
