char _Origin_[] = "System V";

/* disable [-c] [-rreason] printer ...  --  disable printers */

#include	"lp.h"

SCCSID("@(#)disable.c	3.1")

int interrupt = FALSE;
char errmsg[100];

main(argc, argv)
int argc;
char *argv[];
{
	int i, cancel = FALSE, printers = 0;
	char *reason, *trim(), *arg;

	startup(argv[0]);

	if(argc == 1) {
		printf("usage: %s [-c] [-r[reason]] printer ...\n", argv[0]);
		exit(0);
	}

	for(i = 1; i < argc; i++) {
		if(strncmp(argv[i], "-c", 2) == 0) {
			cancel = TRUE;
			argv[i] = NULL;
		}
	}

	for(i = 1; i < argc; i++) {
		arg = argv[i];
		if(arg == NULL)
			continue;
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
		else if(! isprinter(arg)) {
			printers++;
			sprintf(errmsg,
			    "printer \"%s\" non-existent", arg);
			fatal(errmsg, 0);
		}
		else {
			printers++;
			disable(arg, reason, cancel);
		}
	}

	if(printers == 0)
		fatal("no printers specified", 1);

	exit(0);
}

disable(pr, reason, cancel)
char *pr;
char *reason;
int cancel;
{
	struct pstat p;
	struct outq o;
	char *strncpy(), *strcpy();

	setoent();
	if(getpdest(&p, pr) == EOF) {
		sprintf(errmsg, "printer \"%s\" has disappeared!", pr);
		fatal(errmsg, 0);
	}
	else if(! (p.p_flags & P_ENAB)) {
		sprintf(errmsg, "printer \"%s\" was already disabled", pr);
		fatal(errmsg, 0);
	}
	else {
		setsigs();
		if(p.p_flags & P_BUSY) {
			kill(-p.p_pid, SIGTERM);
			if(getoid(&o, p.p_rdest, p.p_seqno) != EOF) {
				if(cancel) {
					o.o_flags |= O_DEL;
					printf("request \"%s-%d\" cancelled\n",
						p.p_rdest, p.p_seqno);
					mail(o.o_logname,p.p_rdest,p.p_seqno);
				}
				else {
					o.o_flags &= ~O_PRINT;
					strcpy(o.o_dev, "-");
				}
				putoent(&o);
			}
		}
		time(&p.p_date);
		p.p_flags &= ~P_ENAB;
		if(reason != NULL) {
			strncpy(p.p_reason, reason, P_RSIZE);
			p.p_reason[P_RSIZE - 1] = '\0';
		}
		else
			strcpy(p.p_reason, "reason unknown");
		p.p_flags &= ~P_BUSY;
		p.p_seqno = p.p_pid = 0;
		sprintf(p.p_rdest, "-");
		putpent(&p);

		/* notify scheduler of new printer status */

		if(cancel) {
			enqueue(F_ZAP, pr);
			rmreq(o.o_dest, o.o_seqno);
		}
		else
			enqueue(F_DISABLE, pr);
		printf("printer \"%s\" now disabled\n", pr);
		reset();
	}

	endpent();
	endoent();
}

mail(logname, dest, seqno)
char *logname;
char *dest;
int seqno;
{
	char *name;

	if(strcmp((name=getname()), logname) != 0) {
		sprintf(errmsg,
		  "your printer request %s-%d was cancelled by %s.",
		  dest, seqno, name);
		sendmail(logname, errmsg);
	}
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

setsigs()
{
	int saveint();

	if(signal(SIGHUP, SIG_IGN) != SIG_IGN)
		signal(SIGHUP, saveint);
	if(signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, saveint);
	if(signal(SIGQUIT, SIG_IGN) != SIG_IGN)
		signal(SIGQUIT, saveint);
	if(signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, saveint);
}

reset()
{
	int catch();

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

saveint()
{
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	interrupt = TRUE;
}

cleanup()
{
	endpent();
	endoent();
	tunlock();
}
