/*
 * This is a collection of functions which may be used for waiting on
 * resources in device drivers.
 *
 * An example of usage is waiting the previous command to complete before
 * sending new commands to the device.  If dvc is declared as a resource,
 * the driver would do:
 *
 *		ret_val = RES_ERRNO(dvc);
 *		while ((ret_val == 0) && RES_TAKEN(dvc))
 *			ret_val = sleepfor(dvc, pri);
 *		if (ret_val)
			<exit>
 *		<more staments, which may exit on error>
 *		busy_res(dvc, maxtime);
 *
 * before starting a new command.  RES_ERRNO() returns EIO if the resource
 * has had a timeout or error.  The interrupt handler would then do:
 *
 *		clear_res(dvc, RES_BIT_BUSY);
 *
 * (In the above, if sleeping is not allowed, spinfor must be used
 *  instead of sleepfor.)
 *
 * busy_res starts up a timeout, which will be cleared when RES_BIT_BUSY is
 * cleared.  If RES_BIT_BUSY never gets cleared (and the timeout expires),
 * then sleepfor (or spinfor) will return EIO, and RES_TIMED_OUT(x) will
 * be TRUE.
 *
 * ----------------------------------------------------------------------
 *
 * In addition to the above, setting and clearing RES_BIT_LCK may be used to
 * bracket a critical section.  (Setting RES_BIT_LCK will cause RES_TAKEN()
 * to return FALSE, so that normal waiters will continue to wait.)  Inside
 * the critical section, use RES_BUSY_IGNLCK() instead of RES_TAKEN() to
 * check for busy.
 *
 * ----------------------------------------------------------------------
 *
 * Synopsis:
 *	set_res(x, flag)
 *	busy_res(x, maxticks)
 *	clear_res(x, flag)
 *	sleepfor(x, pri)
 *	spinfor(x)
 *	sleepmax(chan, pri, maxticks)
 */

#include "../h/types.h"
#include "../multibus/wait.h"
#include "../h/errno.h"

#ifdef OS_DEBUG
#define WAIT_DEBUG 1
#endif
#ifdef WAIT_DEBUG
int wait_noise = 0;
#endif

void expire_resource();
extern int hz;


/*
 * set_res
 *	- called at spl device or at interrupt level.
 *	- attempting to set RES_BIT_LCK when already set returns 1.
 *
 * permissible bits to set:	RES_BIT_LCK
 *				RES_BIT_ERR
 */
set_res(x, flag)
  register resource *x;
  int flag;
{
	if (flag & x->flag & RES_BIT_LCK)
		return 1;
	x->flag |= (flag & RES_SETTABLE_BITS);
	return 0;
}


/*
 * busy_res
 *	- sets RES_BIT_BUSY
 *	- attempting to set RES_BIT_BUSY when already set returns 1.
 *	- called at spl device or at interrupt level
 */
busy_res(x, maxticks)
  register resource *x;
  int maxticks;			/* how long before timeout, 0 => forever. */
{
	if (x->flag & RES_BIT_BUSY)
		return 1;
	x->flag |= RES_BIT_BUSY;

	/* start a timeout if maxticks > 0 */
	x->chan = (caddr_t) x;
	x->timeout_val = maxticks;
	if (maxticks > 0)
		x->id = timeout(expire_resource, x, maxticks);
	return 0;
}


/*
 * clear_res
 *	- called at spl device or at interrupt level.
 *	- note the side effects of clearing RES_BIT_BUSY or RES_BIT_LCK.
 *
 * permissible bits to clear:	RES_BIT_BUSY
 *				RES_BIT_LCK
 *				RES_BIT_TO
 *				RES_BIT_ERR
 */
void
clear_res(x, flag)
  register resource *x;
  int flag;
{
	flag &= RES_CLEARABLE_BITS;

	if (flag & RES_BIT_BUSY) {
		if (x->wanted) {
			x->wanted = 0;
			wakeup(x->chan);
		}
		if (x->id) {
			untimeout_id(x->id);
			x->id = 0;
		}
		x->timeout_val = 0;
	}

	if (flag & RES_BIT_LCK) {
		if (x->wanted) {
			x->wanted = 0;
			wakeup(x->chan);
		}
	}

	x->flag &= ~flag;
}


/*
 * sleepfor
 *	- returns 0, EINTR, or EIO (for timeout or error).
 *	- called at spl device.
 */
sleepfor(x, pri)
  register resource *x;
  int pri;		/* sleep pri */
{
	if (RES_ANYERR(x))
		return (EIO);

	x->wanted++;
#ifdef	WAIT_DEBUG
	if (wait_noise)
		iprintf("sleepfor: %x, flag = %d\n", (long) x, x->flag);
#endif
	if (sleep(x->chan, pri)) {
		/* Interrupted by user */
		return (EINTR);
	}
	if (RES_ANYERR(x))
		return (EIO);
	/* Don't call untimeout; we wouldn't be here unless
	 * called by wakeup_res, and it called untimeout.
	 */
	return 0;
}

static void
expire_resource(x)
  register resource *x;
{

#ifdef	WAIT_DEBUG
	if (wait_noise)
		iprintf("expire_resource: %x\n", (long) x);
#endif
	x->flag |= RES_BIT_TO;
	x->flag &= ~RES_BIT_BUSY;	/* another busy/wakeup can begin */
	x->id = 0;
	x->timeout_val = 0;
	wakeup(x->chan);
}


/*
 * spinfor
 *	- like sleepfor, except a spin loop is done instead of sleeping.
 *	- called at spl device
 */
spinfor(x)
  register resource *x;
{
	register int tval;

	if (RES_ANYERR(x))
		return (EIO);

		/* be careful about 32 bit ovfl */
	if ((tval = x->timeout_val * (SPINS_PER_SEC / hz)) == 0)
		tval = TIME_FOREVER * SPINS_PER_SEC;
	while ((x->flag & RES_BIT_BUSY) && (--tval > 0))
		;
	if (RES_ANYERR(x))
		return (EIO);
	if (tval == 0) {
		if (x->id)
			untimeout_id(x->id);
		expire_resource(x);
		return (EIO);
	}
	return 0;
}


/*
 * sleepmax
 *
 *	- sleep for no more than maxticks, on any arg valid for sleep().
 *	- returns 0, EINTR, or EIO (for timeout).
 *	- called at spl device
 */
sleepmax(chan, pri, maxticks)
  register caddr_t chan;
  int pri;
  int maxticks;			/* #ticks, 0 means forever (no timeout) */
{
	resource r;

	r.chan = chan;
	r.flag = 0;
	if (maxticks > 0)
		r.id = timeout(expire_resource, &r, maxticks);
	else
		r.id = 0;
	if (sleep(chan, pri)) {
		/* interrupted by user */
		if (r.id)
			untimeout_id(r.id);
		return (EINTR);
	}
	if (RES_TIMED_OUT(&r))
		return (EIO);
	if (r.id)
		untimeout_id(r.id);
	return 0;
}
