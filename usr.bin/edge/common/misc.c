/*
 * $Source: /d2/3.7/src/usr.bin/edge/common/RCS/misc.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:46:00 $
 */
#include <stdio.h>
#include "signal.h"
#include "gsh.h"
#include "gl.h"
#include "device.h"
#include "window.h"
#include "tf.h"
#include "manage.h"

extern	int child_pid;
extern	int child_dead;

/*
 * Hack to support fixed length charstr's
 */
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

byebye(wtmp)
WINTTYMAP	*wtmp;
{
	if (!child_dead)
		kill(child_pid, SIGKILL);
	winclose(wtmp->wt_gid);
	if (wtmp == dbx_win) {
		gexit();
		myexit(-1);
	}
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
	child_dead = 1;
	qenter(WMTXCLOSE, result);

out:
	/*
	 * Lastly, re-enable the signal
	 */
	signal(SIGCLD, child_died);
}

myexit(i)
int	i;
{

	shm_rm();
	exit(i);
}
