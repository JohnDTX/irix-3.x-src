char _Origin_[] = "System V";

/* lpstat -- display line printer status */

#include	"lp.h"
#include	"lpsched.h"

SCCSID("@(#)lpstat.c	3.1")

char errmsg[100];

main(argc, argv)
int argc;
char *argv[];
{
	int i;
	int user(), output(), accept(), printer(), device(), Class();
	char *arg, letter;

	startup(argv[0]);

	if(argc == 1) {
		dolist(getname(), user);
		exit(0);
	}

	for(i = 1; i < argc; i++) {
		arg = argv[i];
		if(*arg == '-') {
			letter = tolower(*(arg + 1));
			if(! islower(letter)) {
				sprintf(errmsg, "unknown option \"%s\"",
				   arg);
				fatal(errmsg, 0);
				continue;
			}
			switch(letter) {
			case 'a':	/* acceptance status */
				dolist(arg + 2, accept);
				break;
			case 'c':	/* class to printer mapping */
				dolist(arg + 2, Class);
				break;
			case 'd':	/* default destination */
				def();
				break;
			case 'o':	/* output for destinations */
				dolist(arg + 2, output);
				break;
			case 'p':	/* printer status */
				dolist(arg + 2, printer);
				break;
			case 'r':	/* is scheduler running? */
				running();
				break;
			case 's':	/* configuration summary */
				config();
				break;
			case 't':	/* print all info */
				all();
				break;
			case 'u':	/* output for user list */
				dolist(arg + 2, user);
				break;
			case 'v':	/* printers to devices mapping */
				dolist(arg + 2, device);
				break;
			default:
				sprintf(errmsg, "unknown option \"%s\"",
				   arg);
				fatal(errmsg, 0);
			}
		}
		else
			dolist(arg, output);
	}

	exit(0);
}

config()
{
	int Class(), device();

	def();
	dolist((char *) NULL, Class);
	dolist((char *) NULL, device);
}

all()
{
	int accept(), printer(), output();

	running();
	config();
	dolist((char *) NULL, accept);
	dolist((char *) NULL, printer);
	dolist((char *) NULL, output);
}

def()
{
	char d[DESTMAX + 1];
	FILE *fopen(), *f;

	if(eaccess(DEFAULT, ACC_R) == 0) {
		if((f = fopen(DEFAULT, "r")) != NULL) {
			if(fscanf(f, "%s\n", d) == 1)
				printf("system default destination: %s\n", d);
			else
				printf("no system default destination\n");
			fclose(f);
		}
	}
	else
		printf("no system default destination\n");
}

output(argc, argv)
int argc;
char *argv[];
{
	int i, seqno;
	struct outq o;
	char *arg, dest[DESTMAX + 1];

	if(argc == 0)
		while(getoent(&o) != EOF)
			putoline(&o);
	else {
		for(i = 0; i < argc; i++) {
			arg = argv[i];
			if(isrequest(arg, dest, &seqno)) {
				if(getoid(&o, dest, seqno) != EOF)
					putoline(&o);
				*arg = '\0';
			}
			else if(!isdest(arg)) {
				sprintf(errmsg,
				    "\"%s\" not a request id or a destination",
				    arg);
				fatal(errmsg, 0);
				*arg = '\0';
			}
		}
		setoent();
		while(getoent(&o) != EOF)
			for(i = 0; i < argc; i++)
				if(strcmp(o.o_dest, argv[i]) == 0) {
					putoline(&o);
					break;
				}
	}
	endoent();
}

user(argc, argv)
int argc;
char *argv[];
{
	int i;
	struct outq o;

	if(argc == 0)
		while(getoent(&o) != EOF)
			putoline(&o);
	else
		while(getoent(&o) != EOF)
			for(i = 0; i < argc; i++)
				if(strcmp(o.o_logname, argv[i]) == 0) {
					putoline(&o);
					break;
				}
	endoent();
}

putoline(o)
struct outq *o;
{
	char reqid[IDSIZE + 1], *dt;
	sprintf(reqid, "%s-%d", o->o_dest, o->o_seqno);
	dt = date(o->o_date);
	printf("%-*s %-*s %*ld   %s", IDSIZE, reqid, LOGMAX-1, o->o_logname,
		OSIZE, o->o_size, dt);
	if(o->o_flags & O_PRINT)
		printf(" on %s\n", o->o_dev);
	else
		putchar('\n');
}

accept(argc, argv)
int argc;
char *argv[];
{
	int i, bad = 0;
	struct qstat q;
	char *arg;

	if(argc == 0)
		while(getqent(&q) != EOF)
			putqline(&q);
	else {
		for(i = 0; i < argc; i++) {
			arg = argv[i];
			if(! isdest(arg)) {
				sprintf(errmsg,
				    "destination \"%s\" non-existent", arg);
				fatal(errmsg, 0);
				bad++;
				*arg = '\0';
			}
		}
		if(bad == argc)
			return;
		while(getqent(&q) != EOF)
			for(i = 0; i < argc; i++)
				if(strcmp(q.q_dest, argv[i]) == 0) {
					putqline(&q);
					break;
				}
	}
	endqent();
}

putqline(q)
struct qstat *q;
{
	char *dt;

	dt = date(q->q_date);
	printf("%s ", q->q_dest);
	if(q->q_accept)
		printf("accepting requests since %s\n", dt);
	else
		printf("not accepting requests since %s -\n\t%s\n",
		   dt, q->q_reason);
}

printer(argc, argv)
int argc;
char *argv[];
{
	int i, bad = 0;
	struct pstat p;
	char *arg;

	if(argc == 0)
		while(getpent(&p) != EOF)
			putpline(&p);
	else {
		for(i = 0; i < argc; i++) {
			arg = argv[i];
			if(! isprinter(arg)) {
				sprintf(errmsg,
				    "printer \"%s\" non-existent", arg);
				fatal(errmsg, 0);
				bad++;
				*arg = '\0';
			}
		}
		if(bad == argc)
			return;
		while(getpent(&p) != EOF)
			for(i = 0; i < argc; i++)
				if(strcmp(p.p_dest, argv[i]) == 0) {
					putpline(&p);
					break;
				}
	}
	endpent();
}

putpline(p)
struct pstat *p;
{
	char *dt;

	dt = date(p->p_date);

	printf("printer %s ", p->p_dest);
	if(p->p_flags & P_AUTO)
		printf("(login terminal) ");
	if(p->p_flags & P_ENAB) {
		if(p->p_flags & P_BUSY) {
			printf("now printing %s-%d.  enabled since %s\n",
				p->p_rdest, p->p_seqno, dt);
		}
		else {
			printf("is idle.  enabled since %s\n", dt);
		}
	}
	else {
		printf("disabled since %s -\n\t%s\n",
			dt, p->p_reason);
	}
}

device(argc, argv)
int argc;
char *argv[];
{
	int i;
	struct dirent *entp;
	DIR *dirf;
	char dest[DESTMAX + 1], *strncpy();

	if(argc == 0) {
		if((dirf = opendir(MEMBER)) == NULL)
			fatal("MEMBER directory has disappeared!", 1);
		while((entp = readdir(dirf)) != NULL)
			if(entp->d_ino != 0 && entp->d_name[0] != '.') {
				strncpy(dest, entp->d_name, DESTMAX + 1);
				dest[DESTMAX] = '\0';
				putdline(dest);
			}
		closedir(dirf);
	}
	else
		for(i = 0; i < argc; i++)
			if(isprinter(argv[i]))
				putdline(argv[i]);
			else {
				sprintf(errmsg,
				    "printer \"%s\" non-existent", argv[i]);
				fatal(errmsg, 0);
			}
}

putdline(p)
char *p;
{
	char mfile[sizeof(MEMBER) + DESTMAX + 1];
	char dev[FILEMAX], *fgets();
	FILE *f = NULL, *fopen();

	sprintf(mfile, "%s/%s", MEMBER, p);
	if(eaccess(mfile, ACC_R) == 0 &&
	   (f = fopen(mfile, "r")) != NULL &&
	   fgets(dev, FILEMAX, f) != NULL)
		printf("device for %s: %s", p, dev);
	else {
		sprintf(errmsg, "printer \"%s\" has disappeared!", p);
		fatal(errmsg, 0);
	}
	if(f != NULL)
		fclose(f);
}

Class(argc, argv)
int argc;
char *argv[];
{
	DIR *dirf;
	char cl[DESTMAX + 1], *strncpy();
	struct dirent *entp;
	int i;

	if(argc == 0) {
		if((dirf = opendir(CLASS)) == NULL)
			fatal("CLASS directory has disappeared!", 1);
		while((entp = readdir(dirf)) != NULL)
			if(entp->d_ino != 0 && entp->d_name[0] != '.') {
				strncpy(cl, entp->d_name, DESTMAX + 1);
				cl[DESTMAX] = '\0';
				putcline(cl);
			}
		closedir(dirf);
	}
	else
		for(i = 0; i < argc; i++)
			if(isclass(argv[i]))
				putcline(argv[i]);
			else {
				sprintf(errmsg,
				    "class \"%s\" non-existent", argv[i]);
				fatal(errmsg, 0);
			}
}

putcline(c)
char *c;
{
	char cfile[sizeof(CLASS) + DESTMAX + 1];
	char member[DESTMAX + 2], *fgets();
	FILE *f = NULL, *fopen();

	sprintf(cfile, "%s/%s", CLASS, c);
	if(eaccess(cfile, ACC_R) == 0 &&
	   (f = fopen(cfile, "r")) != NULL) {
		printf("members of class %s:\n", c);
		while(fgets(member, DESTMAX + 1, f) != NULL)
			printf("\t%s", member);
	}
	else {
		sprintf(errmsg, "class \"%s\" has disappeared!", c);
		fatal(errmsg, 0);
	}
	if(f != NULL)
		fclose(f);
}

running()
{
	if(enqueue(F_NOOP, ""))
		printf("scheduler is running\n");
	else
		printf("scheduler is not running\n");
}

/* dolist(func, list) -- apply function "func" to a "list" of arguments
	func is called as follows:
		func(argc, argv)
	where
		argc is number of elements in list
		argv is an array of pointers to the elements in list
*/

dolist(list, func)
char *list;
int (*func)();
{
	int argc = 0;
	char *argv[ARGMAX];
	int i;
	char *value, *argp, c, *malloc();

	if(list == NULL || *list == '\0') {
		(*func)(0, argv);
		return;
	}

	if((argp = malloc((unsigned)(strlen(list)+1))) == NULL)
		fatal(CORMSG, 1);
	strcpy(argp, list);
	while(*argp != '\0') {
		value = argp;
		while((c = *value) == ' ' || c == ',')
			value++;
		if(c == '\0')
			break;
		argp = value + 1;
		while((c = *argp) != '\0' && c != ' ' && c != ',')
			argp++;
		if(c != '\0')
			*(argp++) = '\0';
		if(enter(value, argv, &argc, ARGMAX) == -1)
			fatal(CORMSG, 1);
	}
	if(argc > 0) {
		(*func)(argc, argv);
		for(i = 0; i < argc; i++)
			free(argv[i]);
	}
	else
		(*func)(0, NULL);
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
	endpent();
	endoent();
	tunlock();
}
