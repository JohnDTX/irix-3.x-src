/*
 * $Source: /d2/3.7/src/usr.bin/edge/common/RCS/winlib.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:46:12 $
 */
/*
 * Window library stuff
 */
#include "sys/types.h"
#include "gl.h"
#include "stdio.h"
#include "utmp.h"
#include "window.h"
#include "gsh.h"

/*
 * Setup output focus to the given rectangle, assuming a one-to-one mapping
 * of world coordinates to screen coordinates.  Cache the last output
 * mapping, and avoid a new viewport and ortho2 command if the mapping
 * hasn't changed.
 */
setoutput(r)
	rect_t *r;
{
	static rect_t lastr;

	if ((lastr.xlen != r->xlen) || (lastr.ylen != r->ylen) ||
	    (lastr.xorg != r->xorg) || (lastr.yorg != r->yorg)) {
		viewport(r->xorg, r->xorg + r->xlen - 1,
				  r->yorg, r->yorg + r->ylen - 1);
		ortho2(-0.5, (r->xlen - 1) + 0.5, -0.5, (r->ylen - 1) + 0.5);
		lastr = *r;
	}
}

struct	utmp entry;

/*
 * Login in the shell so that it shows up in utmp
 */
void
winlogin(name, tty, pid)
	char *name;
	char *tty;
	int pid;
{
	struct	utmp	*pututline();
	if (flag_debug)
		return;
	strncpy(entry.ut_user, name, sizeof(entry.ut_user));
	entry.ut_id[0] = tty[(sizeof("tty") - 1) + 0];
	entry.ut_id[1] = tty[(sizeof("tty") - 1) + 1];
	entry.ut_id[2] = tty[(sizeof("tty") - 1) + 2];
	entry.ut_id[3] = tty[(sizeof("tty") - 1) + 3];
	strncpy(entry.ut_line, tty, sizeof(entry.ut_line));
	entry.ut_pid = pid;
	entry.ut_type = USER_PROCESS;
	entry.ut_time = time(0);

	setutent();
	if (pututline(&entry) == (struct utmp *)NULL)
		printf("pututline returns NULL\n");
	endutent();
}

/*
 * Logout the shell from utmp
 */
void
winlogout()
{
	struct utmp *ne;
	struct	utmp	*getutline();
	struct	utmp	*pututline();

	if (flag_debug)
		return;
	setutent();
	if ((ne = getutline(&entry)) == (struct utmp *)NULL)
		printf("getutline returns NULL\n");
	else {
		ne->ut_type = DEAD_PROCESS;
		if (pututline(ne) == (struct utmp *)NULL)
			printf("pututline returns NULL\n");
	}
	endutent();
}

/*
 * Figure the users login name
 */
char *
my_loginname()
{
	char *name;
	extern char *getenv();

	/*
	 * Be lazy - get environment variable for now
	 */
	name = getenv("LOGNAME");
	if (name == (char *)NULL)
		return ("nobody");
	else
		return (name);
}
