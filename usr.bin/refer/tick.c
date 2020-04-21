/* time programs */

#ifndef lint
static char sccsid[] = "@(#)tick.c	4.2 (Berkeley) 8/11/83";
/* $Source: /d2/3.7/src/usr.bin/refer/RCS/tick.c,v $ */
static	char	*Sccsid = "@(#)$Revision: 1.1 $";
/* $Date: 89/03/27 18:22:57 $ */
#endif

#ifdef	BSD42
#ifndef	V7
#define	V7
#endif	V7
#endif	BSD42

#ifdef	BSD41
#ifndef	V7
#define	V7
#endif	V7
#endif	BSD41

#include "stdio.h"
#include "sys/types.h"

#ifdef	V7
#include "sys/timeb.h"
#define	 time_t long
#else
#include "sys/param.h"
#include "sys/times.h"
long	time();
long	times();
struct	tms	junk;
#endif

struct tbuffer {
	time_t	proc_user_time;
	time_t	proc_system_time;
	time_t	child_user_time;
	time_t	child_system_time;
};
static long start, user, system;

tick()
{
	struct tbuffer tx;
#ifdef	V7
	struct timeb tp;
#endif
	times(&tx);
	user  = tx.proc_user_time;
	system= tx.proc_system_time;
#ifdef	V7
	ftime(&tp);
	start = tp.time*1000+tp.millitm;
#else
	start = time((long *)0)*HZ+times(&junk);
#endif
}

tock()
{
	struct tbuffer tx;
#ifdef	V7
	struct timeb tp;
#endif
	float lap, use, sys;
	if (start==0)
		return;
	times(&tx);
#ifdef	V7
	ftime(&tp);
	lap = (tp.time*1000+tp.millitm-start)/1000.;
#else
	lap = (time((long *)0)*HZ+times(&junk)-start)/((double)HZ);
#endif
	use = (tx.proc_user_time - user)/60.;
	sys = (tx.proc_system_time - system)/60.;
	printf("Elapsed %.2f CPU %.2f (user %.2f, sys %.2f)\n",
	  lap, use+sys, use, sys);
}
