char _Origin_[] = "System V";

/* enable printer ... - enable specified printers */

#include	"lp.h"

SCCSID("@(#)enable.c	3.1")

int interrupt = FALSE;
char errmsg[100];

main(argc, argv)
int argc;
char *argv[];
{
	int i, saveint(), catch();
	struct pstat p;
	char reason[P_RSIZE], *trim(), *pr, *strcpy();

	startup(argv[0]);

	if(argc == 1) {
		printf("usage: %s printer ...\n", argv[0]);
		exit(0);
	}

	sprintf(reason, "enabled");

	for(i = 1; i < argc; i++) {
		pr = argv[i];
		if(! isprinter(pr)) {
			sprintf(errmsg,
			   "printer \"%s\" non-existent", pr);
			fatal(errmsg, 0);
		}
		else if(getpdest(&p, pr) == EOF) {
			sprintf(errmsg,
			    "printer \"%s\" has disappeared!", pr);
			fatal(errmsg, 0);
		}
		else if(p.p_flags & P_ENAB) {
			sprintf(errmsg,
			  "printer \"%s\" was already enabled", pr);
			fatal(errmsg, 0);
		}
		else {
			time(&p.p_date);
			p.p_flags |= P_ENAB;
			p.p_flags &= ~P_BUSY;
			p.p_seqno = p.p_pid = 0;
			sprintf(p.p_rdest, "-");
			strcpy(p.p_reason, reason);
			if(signal(SIGHUP, SIG_IGN) != SIG_IGN)
				signal(SIGHUP, saveint);
			if(signal(SIGINT, SIG_IGN) != SIG_IGN)
				signal(SIGINT, saveint);
			if(signal(SIGQUIT, SIG_IGN) != SIG_IGN)
				signal(SIGQUIT, saveint);
			if(signal(SIGTERM, SIG_IGN) != SIG_IGN)
				signal(SIGTERM, saveint);
			putpent(&p);

			/* notify scheduler of new printer status */

			enqueue(F_ENABLE, pr);
			printf("printer \"%s\" now enabled\n", pr);
			if(signal(SIGHUP, SIG_IGN) != SIG_IGN)
				signal(SIGHUP, catch);
			if(signal(SIGINT, SIG_IGN) != SIG_IGN)
				signal(SIGINT, catch);
			if(signal(SIGQUIT, SIG_IGN) != SIG_IGN)
				signal(SIGQUIT, catch);
			if(signal(SIGTERM, SIG_IGN) != SIG_IGN)
				signal(SIGTERM, catch);
			if(interrupt) {
				cleanup();
				exit(1);
			}
		}
		endpent();
	}

	exit(0);
}

startup(name)
char *name;
{
	int catch(), cleanup();
	extern char * f_name;
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
	endpent();
	tunlock();
}

saveint()
{
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	interrupt = TRUE;
}
