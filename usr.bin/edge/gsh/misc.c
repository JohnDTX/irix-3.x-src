#include "signal.h"
#include "gsh.h"
#include "gl.h"
#include "device.h"
#include "window.h"

extern	int child_pid;
extern	int child_dead;

/*
 * Hack to support fixed length charstr's
 */
void
xcharstr(s, len)
	char *s;
	int len;
{
	char oldc;

	oldc = *(s + len);
	*(s + len) = 0;
	charstr(s);
	*(s + len) = oldc;
}

int
byebye()
{
	if (!child_dead && child_pid)
		kill(child_pid, SIGKILL);
	blink(0, txport[0].tx_cursorcolor, 0, 0, 0);
#ifdef	LOGIN
	winlogout();
#endif
	gexit();
	exit(-1);
}

/*
 * This procedure is called when a SIGCLD signal occurs
 */
child_died()
{
	int status;
	int result;

	result = wait(&status);
	if (result < 0)
		goto out;
	/*
	 * Make sure this is OUR direct child that is exiting, not one
	 * of our forks...
	 */
	if (result != child_pid)
		goto out;
	child_dead = 1;
	qenter(WMTXCLOSE, (short) 777);

out:
	/*
	 * Lastly, re-enable the signal
	 */
	signal(SIGCLD, child_died);
}
