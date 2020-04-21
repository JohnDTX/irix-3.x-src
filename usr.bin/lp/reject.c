char _Origin_[] = "System V";

/* reject [-r[reason]] dest ...  -- prevent lp from accepting requests */

#include	"lp.h"

SCCSID("@(#)reject.c	3.1")

char errmsg[100];

main(argc, argv)
int argc;
char *argv[];
{
	int i, dests = 0;
	char *arg, *reason = NULL, *trim();

	startup(argv[0]);

	if(! ISADMIN)
		fatal(ADMINMSG, 1);

	if(argc == 1) {
		printf("usage: %s [-r[reason]] dest ...\n", argv[0]);
		exit(0);
	}

	for(i = 1; i < argc; i++) {
		arg = argv[i];
		if(*(arg) == '-') {
			if(*(arg + 1) != 'r') {
				sprintf(errmsg,"unknown option \"%s\"",arg);
				fatal(errmsg, 1);
			}
			else {
				reason = arg + 2;
				if(*trim(reason) == '\0')
					reason = NULL;
			}
		}
		else if(isdest(arg)) {
			dests++;
			reject(arg, reason);
		}
		else {
			dests++;
			sprintf(errmsg, "destination \"%s\" non-existent", arg);
			fatal(errmsg, 0);
		}
	}

	if(dests == 0)
		fatal("no destinations specified", 1);
	exit(0);
}

reject(dest, reason)
char *dest;
char *reason;
{
	struct qstat q;
	char *strcpy(), *strncpy();

	if(getqdest(&q, dest) == EOF) {
		sprintf(errmsg, "destination \"%s\" non-existent", dest);
		fatal(errmsg, 0);
	}
	else if(! q.q_accept) {
		sprintf(errmsg,
		  "destination \"%s\" was already not accepting requests",
		  dest);
		fatal(errmsg, 0);
	}
	else {
		q.q_accept = FALSE;
		time(&q.q_date);
		if(reason != NULL) {
			strncpy(q.q_reason, reason, Q_RSIZE);
			q.q_reason[Q_RSIZE - 1] = '\0';
		}
		else
			strcpy(q.q_reason, "reason unknown");
		putqent(&q);
		printf("destination \"%s\" is no longer accepting requests\n",
			dest);
	}
	endqent();
}

startup(name)
char *name;
{
	int catch(), cleanup();
	extern char *f_name;
	extern int (*f_clean)();

	if(signal(SIGHUP, SIG_IGN) != SIG_IGN)
		signal(SIGHUP, catch);
	if(signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, catch);
	if(signal(SIGQUIT, SIG_IGN) != SIG_IGN)
		signal(SIGQUIT, catch);
	if(signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, catch);

	f_name = name;
	f_clean = cleanup;
	if(chdir(SPOOL) == -1)
		fatal("spool directory non-existent", 1);
}

/* catch -- catch signals */

catch()
{
	int cleanup();
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	cleanup();
	exit(1);
}

cleanup()
{
	endqent();
	tunlock();
}
