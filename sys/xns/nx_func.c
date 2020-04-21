#include "../h/param.h"
#include "../h/tty.h"
#include "../xns/if_xns.h"


# undef DEBUG nx_debug
# ifdef DEBUG
short nx_debug = 0;
#	define dprintf(x)	(DEBUG?printf x:0)
#	define IFDEBUG(x)	x
# else  DEBUG
#	define dprintf(x)
#	define IFDEBUG(x)
# endif DEBUG

short	nx_onnet;
short	nx_nexttimeforsure;		/* flag - must be msg next time */
long	nx_n_restarts;			/* # times restarted */
short	nx_play_dead;			/* flag - disable nx (test restart) */
long	nx_n_rmp = -1;			/* # msgs received this interval */
short	nx_timer_holdoff;
short	nx_restart_period = 30;		/* min period between restarts */
					/* ~2 mins */
short	nx_winterval = 15;		/* watch at least this often */
short	nx_wtime;


# ifdef IFXNS

nx_timer()
{
}

# else  IFXNS

/*
 * nx_timer() --
 * called at some interval by xns_timer().
 * called at hi pri.
	- if the queue has moved, do nothing.
	- otherwise, try to make it move by requesting statistics.
	- if already requested statistics last time, and it still
	  didn't move, do a restart.
 */
nx_timer()
{
	USEPRI;

	if (--nx_timer_holdoff >= 0)
		return;

	LOCK;
	nxintflag++;
	nx_timer_holdoff = 0;

	if (--nx_wtime < 0 || nx_n_rmp == 0) {

		nx_wtime = nx_winterval;
		if (!nx_nexttimeforsure) {
			nx_nexttimeforsure++;
			(void) nxgetstats();
		}
		else {
			nx_nexttimeforsure = 0;
			dprintf((" nx controller hung, restarting\n"));
			nxintr();

			if (nx_n_rmp == 0)
				if (nx_restart() == 0)
					dprintf(("nx restart complete\n"));
				else
					dprintf(("nx restart failed\n"));

		}
	}
	else {
		nx_nexttimeforsure = 0;
		nx_n_rmp = 0;
	}

	nxintflag--;
	UNLOCK;
}

int
nx_restart()
{
	nx_play_dead = 0;		/* stop playing dead now */

	nx_n_restarts++;
	nx_timer_holdoff = nx_restart_period;

	if (!nxpresent)
		return -1;

	nxpresent = 0;
	if (nx_dev_reset() < 0) {
		printf("nx dev reset failed!\n");
		return -1;
	}

	nxqsetup();
	nxreads = 0;
	nx_onnet = 0;
	if (nxinitmsg(0) < 0) {
		printf("nx dev restart failed!\n");
		return -1;
	}
	nxpresent++;

	nbp_salvage();
	conp_restart();
	if (nxmode(NET_CONNECT) == 0)
		nx_onnet++;
	if (nx_onnet)
		nx_timer_holdoff >>= 1;

	nxintr();	/* for good measure */

	return 0;
}

/*
 * nbp_salvage() --
 * examine all netbufs; try to fake the done intr.
 * it is up to the protocol code to recover from 
 * this "fake success."
 */
nbp_salvage()
{
	register NBP nbp;

	freenetbufs = NULL;
	nxfreecount = 0;

	for (nbp = permnetbufs; nbp != NULL; nbp = nbp->perm) {
		switch (nbp->btype&B_USAGE) {

		case B_INPQ:
			break;

		case B_NFREE:
		case B_RECV:
			freenbuf(nbp);
			break;

		case B_RAWXMIT:
			/* stats too! */
			wakeup((caddr_t)nbp);
			nbp->btype |= N_DONE;
			freenbuf(nbp);
			break;

		default:
		case B_XMIT:
			wakeup((caddr_t)nbp);
			nbp->btype |= N_DONE;
			break;
		}
	}
}

# endif IFXNS
