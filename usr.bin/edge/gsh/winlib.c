/*
 * Window library stuff
 */
#include <sys/types.h>
#include <stdio.h>
#include <utmp.h>
#include <string.h>
#include "window.h"
#include "gsh.h"

#ifdef	SVR3
extern	struct utmp *getutline(), *pututline();
#endif

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

#ifdef	LOGIN
struct	utmp entry;

/* XXX look at system V login and init to figure how the hell to do this
       right, as opposed to how the documentation says you can do it */
/*
 * Login in the shell so that it shows up in utmp
 */
void
winlogin(name, tty, pid)
	char *name;
	char *tty;
	int pid;
{
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
	entry.ut_time = time((long *) 0);

	setutent();
	if (pututline(&entry) == (struct utmp *)NULL) {
		printf("gsh: unable to log this window in\n");
		printf("gsh: make sure gsh is owned by root, mode 4755\n");
	}
	endutent();
}

/*
 * Logout the shell from utmp
 */
void
winlogout()
{
	struct utmp *ne;

	if (flag_debug)
		return;
	setutent();
	if (ne = getutline(&entry)) {
		ne->ut_type = DEAD_PROCESS;
		(void) pututline(ne);
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
#endif
