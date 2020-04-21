char _Origin_[] = "System V";

/* lpshut -- shut the line printer scheduler
	All busy printers will stop printing,
	but no requests will be cancelled because of this.
 */

#include	"lp.h"

SCCSID("@(#)lpshut.c	3.1")

main(argc, argv)
int argc;
char *argv[];
{
	extern char *f_name;

	f_name = argv[0];

	if(! ISADMIN)
		fatal(ADMINMSG, 1);

	if(chdir(SPOOL) == -1)
		fatal("spool directory non-existent", 1);
	if(enqueue(F_QUIT, "")) {
		/* Scheduler is running */
		printf("scheduler stopped\n");
		exit(0);
	}
	else {
		/* Scheduler is not running -- remove the FIFO and SCHEDLOCK */
		unlink(FIFO);
		unlink(SCHEDLOCK);
		fatal("scheduler not running", 1);
	}
}
