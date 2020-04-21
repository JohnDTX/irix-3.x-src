/* wrtmsg(user, msg) -- write message to user's tty if logged in.
	return codes: TRUE ==> success,
		      FALSE ==> failure
 */

#include	"lp.h"
#include	"lpsched.h"

SCCSID("@(#)wrtmsg.c	3.1")

wrtmsg(user, msg)
char *user;
char *msg;
{
	char *tty, *findtty();
	FILE *f;
	int sigalrm();
	unsigned alarm(), sleep();

	if((tty = findtty(user)) == 0 || (f = fopen(tty, "w")) == NULL)
		return(FALSE);
	signal(SIGALRM, sigalrm);
	alarm(10);
	fputc(BEL, f);
	fflush(f);
	sleep(2);
	fputc(BEL, f);
	fprintf(f, "\nlp: %s\n", msg);
	alarm(0);
	fclose(f);
	return(TRUE);
}

/* sigalrm() -- catch SIGALRM */

static int
sigalrm()
{
}
