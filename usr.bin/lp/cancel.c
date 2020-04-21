char _Origin_[] = "System V";

/*cancel id ... printer ...  --  cancel output requests */


#include	"lp.h"

SCCSID("@(#)cancel.c	3.1")

int interrupt = FALSE;
char errmsg[100];

main(argc, argv)
int argc;
char *argv[];
{
	char dest[DESTMAX + 1], *arg;
	int seqno, i;

	startup(argv[0]);

	if(argc == 1) {
		printf("usage: cancel id ... printer ...\n");
		exit(0);
	}

	for(i = 1; i < argc; i++) {
		arg = argv[i];
		if(isprinter(arg))
			restart(arg);
		else if(isrequest(arg, dest, &seqno))
			cancel(dest, seqno);
		else {
			sprintf(errmsg,
			    "\"%s\" is not a request id or a printer", arg);
			fatal(errmsg, 0);
		}
	}
	exit(0);
}

restart(printer)
char *printer;
{
	struct outq o;
	struct pstat p;
	char *l;

	setoent();
	if(getpdest(&p, printer) == EOF) {
		sprintf(errmsg, "printer \"%s\" has disappeared!", printer);
		fatal(errmsg, 0);
	}
	else if(! (p.p_flags & P_BUSY)) {
		sprintf(errmsg, "printer \"%s\" was not busy", printer);
		fatal(errmsg, 0);
	}
	else {
		setsigs();
		if(getoid(&o, p.p_rdest, p.p_seqno) != EOF) {
			o.o_flags |= O_DEL;
			putoent(&o);
			rmreq(p.p_rdest, p.p_seqno);
		}
		killit(&p);
		sprintf(errmsg, "%s %d", o.o_dest, o.o_seqno);
		enqueue(F_CANCEL, errmsg);
		printf("request \"%s-%d\" cancelled\n", o.o_dest, o.o_seqno);
		if(strcmp((l=getname()), o.o_logname) != 0) {
			sprintf(errmsg,
			  "your printer request %s-%d was cancelled by %s.",
			  o.o_dest, o.o_seqno, l);
			sendmail(o.o_logname, errmsg);
		}
		reset();
	}
	endpent();
	endoent();
}

cancel(dest, seqno)
char *dest;
int seqno;
{
	struct outq o;
	struct pstat p;
	char *l;

	if(getoid(&o, dest, seqno) == EOF) {
		sprintf(errmsg,"request \"%s-%d\" non-existent", dest, seqno);
		fatal(errmsg, 0);
	}
	else {
		setsigs();
		o.o_flags |= O_DEL;
		putoent(&o);
		rmreq(dest, seqno);
		if(o.o_flags & O_PRINT) {
			if(getpdest(&p, o.o_dev) != EOF)
				killit(&p);
			endpent();
		}
		sprintf(errmsg, "%s %d", dest, seqno);
		enqueue(F_CANCEL, errmsg);
		printf("request \"%s-%d\" cancelled\n", dest, seqno);
		if(strcmp((l=getname()), o.o_logname) != 0) {
			sprintf(errmsg,
			  "your printer request %s-%d was cancelled by %s.",
			  o.o_dest, o.o_seqno, l);
			sendmail(o.o_logname, errmsg);
		}
		reset();
	}
	endoent();
}

killit(p)
struct pstat *p;
{
	char *strcpy();

	if(p->p_pid != 0)
		kill(-(p->p_pid), SIGTERM);
	p->p_flags &= ~P_BUSY;
	p->p_pid = p->p_seqno = 0;
	strcpy(p->p_rdest, "-");
	putpent(p);
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
