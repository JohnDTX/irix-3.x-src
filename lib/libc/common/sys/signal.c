/*
 * signal(n, 0);	default action on signal(n)
 * signal(n, odd);	ignore signal(n)
 * signal(n, label);	goto label on signal(n)
 *	returns old label, only one level.
 *
 * sigrte.s and sigtrap.s are also part of signal()
 */

#include <signal.h>
#include <errno.h>

#define NSIGS 32

int (*sigfunc[NSIGS])();	/* functions to be executed on signals */

typedef char	hndl[4];
extern hndl	sighnd[NSIGS];
extern long	errno;

int (*signal(sig, func))()
	register int sig;
	int (*func)();
{
	int (*sigtrap())();
	int (*old)();
	int (*rv)();

	if (sig <= 0 || sig >= NSIGS) {
		errno = EINVAL;
		return((int (*)())-1);
	}
	/* save this */
	old = sigfunc[sig];

	switch (func) {
	  case SIG_DFL:
	  case SIG_IGN:
		/*
		 * If the user is disabling the signal, then call the system
		 * first to disable the signal, then update our vector.
		 * That way, if a signal comes in before we get done, we
		 * don't go off into wonderland.
		 */
		rv = sigtrap(sig, func);
		sigfunc[sig] = func;
		break;
	  default:
	  	/*
		 * If the user is enabling the signal, then call the system
		 * after we have updated our vector.  If the signal was
		 * already enabled, then the correct thing happens before
		 * and after updating the vector.  If the signal was
		 * disabled, then we can't get it, so the update of the
		 * vector and the os call are effectively atomic, which
		 * is what we wanted in the first place.
		 */
		sigfunc[sig] = func;
		rv = sigtrap(sig, (int (*)())&sighnd[sig][0]);
		break;
	}

	/* see if user did a no-no */
	if ((int)rv == -1) {
		/*
		 * Restore our signal vector, since the kernel didn't grok
		 * what we gave it.  There is a race here that we can't
		 * do anything about.
		 */
		sigfunc[sig] = old;
		return (rv);
	}

	/*
	 * Finally, check for a race between trapping the signal and getting
	 * the signal.  If the kernel returns a disabled value for the
	 * signal, then (a) either the signal was disabled before or (b)
	 * the signal occured while we were busy and it is now disabled.
	 * In any case, believe the kernel if this happens.
	 */
	switch (rv) {
	  case SIG_DFL:
	  case SIG_IGN:
		/*
		 * System thinks that signal is now disabled.  Believe it.
		 */
		break;
	  default:
		/*
		 * System is returning last handler.  Use the one we know
		 * about, saved above in old.
		 */
		rv = old;
		break;
	}
	return (rv);
}
