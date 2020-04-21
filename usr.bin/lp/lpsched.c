char _Origin_[] = "System V";

/* LP Scheduler */

#include	"lp.h"
#include	"lpsched.h"
#include	<errno.h>
extern int errno;

SCCSID("@(#)lpsched.c	3.4")

int pgrp;			/* process group and process id of scheduler */
FILE *rfifo = NULL;
FILE *wfifo = NULL;
char errmsg[100];
short wrt;		/* TRUE ==> write to user instead of mailing */
short mail;		/* TRUE ==> user requested mail, FALSE ==> no mail */

/*
	The following global variables are the primary means of passing
	values among the functions of children of the scheduler.
*/

short sigterm = FALSE;		/* TRUE => received SIGTERM from scheduler */
FILE *rfile;			/* Used ONLY for reading request file */
char *pr;			/* Name of printer currently printing */
char *dev;			/* Device to which pr is printing */
char *dst;			/* Destination of current print request */
char *logname;			/* Requestor of current request */
int seqno;			/* Sequence # of current print request */
int pid;			/* process id  of child */
char *cmd[ARGMAX];		/* printer interface command line */
int nargs;			/* # of args in cmd array */
char rname[RNAMEMAX];		/* name of request file being processed */

main(argc, argv)
int argc;
char *argv[];
{
	startup(argv[0]);
	pinit();		/* initialize printers from MEMBER directory */
	cinit();		/* initialize classes from CLASS directory */
	openfifo();		/* open fifo to receive requests */
	enqueue(F_NEWLOG, "");	/* Make new error log */
	cleanoutq();		/* clean up output queue and initialize
				   printer and class output lists */
	cleanfiles();		/* remove orphan request and data files */
	psetup();		/* Clean up printer status file,
				   initialize printer status info,
				   and write "printer ready" messages
				   on fifo for all enabled printers */
	schedule();		/* schedule user requests for printing */
}


/*
 * buildcmd -- builds command line to be given to printer interface program.
 *		 The result is left in cmd.
 *		 Returns: -1 for errors, 0 otherwise.
 */

int
buildcmd()
{
	char arg[FILEMAX], type, file[NAMEMAX];

	sprintf(rname, "%s/%s/r-%d", REQUEST, dst, seqno);
	if((rfile = fopen(rname, "r")) == NULL)
		return(-1);

	wrt = mail = FALSE;
	nargs = 0;
	sprintf(arg, "%s/%s", INTERFACE, pr);
	if(enter(arg, cmd, &nargs, ARGMAX) == -1)
		return(-1);
	sprintf(arg, "%s-%d", dst, seqno);
	if(enter(arg, cmd, &nargs, ARGMAX) == -1)
		return(-1);
	if(enter(logname, cmd, &nargs, ARGMAX) == -1)
		return(-1);

	while(getrent(&type, arg, rfile) != EOF) {
		switch(type) {
		case R_FILE:
			if(arg[0] != '/') {
				/* file under SPOOL/REQUEST/dst */
				strcpy(file, arg);
				sprintf(arg, "%s/%s/%s/%s", SPOOL, REQUEST,
					dst, file);
			}
		case R_TITLE:
		case R_COPIES:
		case R_OPTIONS:
			if(enter(arg, cmd, &nargs, ARGMAX) == -1)
				return(-1);
			break;
		case R_WRITE:
			wrt = TRUE;
			break;
		case R_MAIL:
			mail = TRUE;
			break;
		default:
			break;
		}
	}
	if(enter((char *) NULL, cmd, &nargs, ARGMAX) == -1)
		return(-1);
	return(0);
}


/*
 * cinit -- initialize destination structure from entries in CLASS directory
 */

cinit()
{
	FILE *cf;
	DIR *cd;
	char member[DESTMAX + 1], cfile[sizeof(CLASS) + DESTMAX + 1];
	struct dirent *entp;
	struct dest *d, *m, *getp(), *newdest();
	char *c;
	extern struct dest class;

	if((cd = opendir(CLASS)) == NULL)
		fatal("can't open CLASS directory", 1);

	while((entp = readdir(cd)) != NULL) {
		if(entp->d_ino != 0 && entp->d_name[0] != '.') {
			d = newdest(entp->d_name);
			d->d_status = D_CLASS;
			insert(&class, d);
			sprintf(cfile, "%s/%s", CLASS, entp->d_name);
			if((cf = fopen(cfile, "r")) == NULL) {
				sprintf(errmsg,
				  "can't open %s file in CLASS directory",
				  entp->d_name);
				fatal(errmsg, 0);
			}
			else {
				while(fgets(member, FILEMAX, cf) != NULL) {
				   if(*(c=member+strlen(member)-1) == '\n')
					*c = '\0';
				   if((m = getp(member)) == NULL) {
					sprintf(errmsg,
					  "non-existent printer %s in class %s",
					  member, entp->d_name);
					fatal(errmsg, 0);
				   }
				   else
					newmem(d, m);
				}
				fclose(cf);
			}
		}
	}
	closedir(cd);
}


/*
 * cleanfiles -- remove request and data files which have no entry in
 *	the output queue, temporary request files and mysterious files
 */

cleanfiles()
{
	struct dest *d;
	extern struct dest dest;
	DIR *dirf;
	struct dirent *entp;
	char fullfile[RNAMEMAX], *file, *seq, *strchr(), *strncpy();
	struct outq o;
	int s;

	FORALLD(d) {
		sprintf(fullfile, "%s/%s/", REQUEST, d->d_dname);
		if((dirf = opendir(fullfile)) == NULL) {
			sprintf(errmsg, "can't open request directory %s",
			    fullfile);
			fatal(errmsg, 1);
		}

		/* remove any request and data files that are not
		   mentioned in the output queue */

		file = &fullfile[strlen(fullfile)];
		while((entp = readdir(dirf)) != NULL) {
			if(entp->d_ino != 0 && entp->d_name[0] != '.') {
				strcpy(file, entp->d_name);
				switch(*file) {
				case 'd': /* data file */
				case 'r': /* request file */
					if((seq = strchr(file, '-')) == NULL ||
					   (s = atoi(++seq)) <= 0 ||
					   getoid(&o,d->d_dname,s)==EOF)
						unlink(fullfile);
					break;
				default:
					unlink(fullfile);
					break;
				}
			}
		}
		closedir(dirf);
	}
	endoent();
}


/*
 * cleanoutq -- clean up output queue :
 *		remove deleted records
 *		mark printing requests as not printing
 *	Initialize printer and class output lists.
 */

cleanoutq()
{
	struct outq o;
	FILE *otmp = NULL;
	struct dest *d, *getd();

	if((otmp = fopen(TOUTPUTQ, "w")) == NULL)
		fatal("can't open temporary output queue", 1);

	chmod(TOUTPUTQ, 0644);

	while(getoent(&o) != EOF) {
		if(o.o_flags & O_PRINT) {
			o.o_flags &= ~O_PRINT;
			sprintf(o.o_dev, "-");
			time(&o.o_date);
		}

		wrtoent(&o, otmp);
		if((d = getd(o.o_dest)) != NULL)
			inserto(d, o.o_seqno, o.o_logname);
	}

	fclose(otmp);
	endoent();

	if(unlink(OUTPUTQ) == -1 && errno != ENOENT)
		fatal("can't unlink old output queue", 1);
	if(link(TOUTPUTQ, OUTPUTQ) == -1 || unlink(TOUTPUTQ) == -1)
		fatal("can't create new output queue", 1);
}


/*
 * cleanup -- called by fatal
 */

cleanup()
{
	struct dest *p;
	extern struct dest printer;

	endpent();
	endoent();
	unlock(SCHEDLOCK);
	tunlock();

	unlink(FIFO);

#ifdef DEBUG
	dump();
#endif

	FORALLP(p)
		if(p->d_status & D_BUSY)
			waitfor(p);
	fprintf(stderr, "***** STOPPED  %s *****\n", date(time((time_t *)0)));
	exit(0);
}


/*
 * disable(reason) -- disable printer pr because of specified reason
 */

disable(reason)
char *reason;
{
	char xqt[ARGMAX];

	sprintf(xqt, "%s/%s -r\"disabled by scheduler: %s\" %s",
	   USRDIR, DISABLE, reason, pr);
	system(xqt);
}


/*
 * findprinter(d) -- find an available printer which can print a request
 *	that has been queued for destination d.  If such a printer p is found,
 *	then give it a request to print.
 */

findprinter(d)
struct dest *d;
{
	struct destlist *head, *dl;
	int status;

	if(d->d_status & D_PRINTER) {
		if((d->d_status & D_ENABLED) && !(d->d_status & D_BUSY))
			makebusy(d);
	}
	else {
		for(head = d->d_class, dl = head->dl_next;
		    dl != head; dl = dl->dl_next) {
			status = (dl->dl_dest)->d_status;
			if((status & D_ENABLED) && !(status & D_BUSY))
				break;
		}
		if(dl != head)
			makebusy(dl->dl_dest);
	}
}


/*
 * makebusy(p) -- if there is a request to print on printer p, then print it
 */

makebusy(p)
struct dest *p;
{
	struct outlist *o, *nextreq();
	int ret, sig15(), killchild(), status;

	if((o = nextreq(p)) == NULL)	/* No requests to print on printer p */
		return;

	fprintf(stderr, "%s-%d\t%s\t%s\t%s\n", (o->ol_dest)->d_dname,
	   o->ol_seqno, o->ol_name, p->d_dname, date(time((time_t *)0)));
	fflush(stderr);		/* update log file */

	/*
	 *	Fork to do the printing --
	 *	the parent will continue the scheduling
	 *	child #1 will fork and exec interface program
	 *	child #2 will wait for interface program to complete
	 */

	while((pid = fork()) == -1)
		;
	if(pid != 0) {		/* set in-memory status */
		p->d_pid = pid;
		o->ol_print = p;
		p->d_status |= D_BUSY;
		p->d_print = o;
		return;		/* back to scheduling */
	}
	signal(SIGTERM, sig15);		/* Delay the handling of SIGTERM */

	/* Establish values for key global variables */

	pid = getpid();
	pr = p->d_dname;
	dev = p->d_device;
	dst = (o->ol_dest)->d_dname;
	logname = o->ol_name;
	seqno = o->ol_seqno;

	preprint();		/* prepare for printing */

	while((pid = fork()) == -1)	/* 2nd level child does printing */
		if(sigterm)
			exit(0);
	if(pid == 0) {
		pid = setpgrp();
		if((ret = setstatus()) != 0)
			exit((ret < 0) ? EX_SYS : (EX_SYS | EX_READY));
		if(sigterm)
			exit(EX_SYS | EX_TERM);
		execvp(cmd[0], cmd);	/* execute interface program */
		signal(SIGTERM, SIG_IGN);
		disable(EXECMSG);
		exit(EX_SYS | EX_RESET);
	}
	signal(SIGTERM, killchild);	/* kill interface on SIGTERM */
	while(wait(&status) != pid)	/* 1st child waits for 2nd child */
		;
	postprint(status);		/* printing done -- clean up */
	exit(0);
}


/*
 * newdev(p, d) -- associates device d with printer p
 */

newdev(p, d)
struct dest *p;
char *d;
{
	char *c;

	if(p->d_device != NULL)
		free(p->d_device);
	if((c = malloc((unsigned)(strlen(d)+1))) == NULL)
		fatal(CORMSG, 1);
	strcpy(c, d);
	p->d_device = c;
}


/*
 * nextreq(p) -- returns a pointer to the oldest output request that may
 *	be printed on printer p.
 *	This will be either a request queued specifically for p or
 *	one queued for one of the classes which p belongs to.
 *	If there are no requests for p, then nextreq returns NULL.
 */

struct outlist *
nextreq(p)
struct dest *p;
{
	int t;
	struct outlist *o, *ohead, *oldest;
	struct destlist *dl, *dlhead;
	short found;

	t = 0;
	oldest = NULL;

	/* search printer's output list */

	for(ohead = p->d_output, o = ohead->ol_next;
	    t == 0 && o != ohead; o = o->ol_next) {
		if(o->ol_print == NULL) {
			/* o is p's oldest non-printing request */
			oldest = o;
			t = o->ol_time;
		}
	}

	/* search output lists of classes that p belongs to */

	for(dlhead = p->d_class, dl = dlhead->dl_next;
	    dl != dlhead; dl = dl->dl_next) {
		for(found=FALSE,ohead=(dl->dl_dest)->d_output,o=ohead->ol_next;
		    !found && o != ohead; o = o->ol_next) {
			if(o->ol_print == NULL) {
				/* o is dl's oldest non-printing request */
				found = TRUE;
				if(o->ol_time < t || t == 0) {
					/* o is oldest of all non-printing
					   requests found so far */
					oldest = o;
					t = o->ol_time;
				}
			}
		}
	}
	return(oldest);
}


/*
 * opendev -- reopen stdin, stdout and stderr and make sure that interface
 *	is executable.
 */

opendev()
{
	char xqt[sizeof(INTERFACE) + DESTMAX + 1];
	int sig14(), fd1, fd2;
	char mode[3];
	FILE *f;

	sprintf(xqt, "%s/%s", INTERFACE, pr);
	errmsg[0] = '\0';

	if(eaccess(dev, ACC_W) != 0)
		sprintf(errmsg, "can't write to %s", dev);
	else if(eaccess(xqt, ACC_X) != 0)
		strcpy(errmsg, EXECMSG);
	else {
		freopen("/dev/null", "r+", stdin);
		mode[0] = 'a';
		mode[1] = mode[2] = '\0';
		if(eaccess(dev, ACC_R) == 0)	/* open for r+w, if possible */
			mode[1] = '+';
		signal(SIGALRM, sig14);
		f = NULL;
		alarm(OPENTIME);
		f = freopen(dev, mode, stdout);
		alarm(0);
		signal(SIGALRM, SIG_DFL);
		if(f == NULL)
			sprintf(errmsg, "can't open %s", dev);
		else {
			fclose(stderr);
			fd1 = fileno(stdout);
			if((fd2 = dup(fd1)) < 0 || fdopen(fd2, mode) != stderr)
				strcpy(errmsg, "can't reopen stderr");
		}
	}

	if(errmsg[0] != '\0') {
		disable(errmsg);
		exit(0);
	}
}


sig14()
{
}


/*
 * openfifo -- open fifo to queue printer requests for data
 */

openfifo()
{
	if(eaccess(FIFO,ACC_R|ACC_W) == -1 && mknod(FIFO,S_IFIFO|0600,0) != 0)
		fatal("Can't access FIFO", 1);

	if((wfifo=fopen(FIFO, "a+"))==NULL || (rfifo=fopen(FIFO, "r"))==NULL)
		fatal("can't open FIFO", 1);
}


/*
 * pinit -- initialize destination structure from entries in MEMBER directory
 */

pinit()
{
	FILE *m;
	DIR *md;
	char device[FILEMAX], memfile[sizeof(MEMBER) + DESTMAX + 1];
	struct dirent *entp;
	struct dest *d, *newdest();
	char *c;
	extern struct dest printer;

	if((md = opendir(MEMBER)) == NULL)
		fatal("can't open MEMBER directory", 1);

	while((entp = readdir(md)) != NULL) {
		if(entp->d_ino != 0 && entp->d_name[0] != '.') {
			d = newdest(entp->d_name);
			d->d_status = D_PRINTER;
			insert(&printer, d);

			sprintf(memfile, "%s/%s", MEMBER, entp->d_name);
			if((m = fopen(memfile, "r")) == NULL) {
				sprintf(errmsg,
				  "can't open %s file in MEMBER directory",
				  entp->d_name);
				fatal(errmsg, 0);
				d->d_device = NULL;
			}
			else {
				if(fgets(device, FILEMAX, m) == NULL)
					d->d_device = NULL;
				else {
				   if(*(c=device+strlen(device)-1) == '\n')
					*c = '\0';
				   newdev(d, device);
				}
				fclose(m);
			}
		}
	}
	closedir(md);
}


/*
 * postprint(status) -- clean up after printing a request
 *	status is the return code from interface program
 */

postprint(status)
int status;
{
	int term, excode;

	excode = status >> 8;
	term = status & 0177;
	if(term == SIGTERM || excode == (EX_SYS | EX_TERM))
		/* interface was killed */
		resetstatus(0, 1);
	else if(excode & EX_SYS) {	/* system error exit */
		if(excode & EX_RESET)
			resetstatus(0, 1);
		if(excode & EX_READY)
			enqueue(F_MORE, pr);
	}
	else {			/* printer interface exited */
		resetstatus(1, 1);
		if(excode != 0) {
			sprintf(errmsg,
			  "error code %d in request %s-%d on printer %s",
			  excode, dst, seqno, pr);
			wrtmail(logname, errmsg);
		}
		else {		/* no errors detected by interface pgm */
			fclose(rfile);
			unlink(rname);
			sprintf(errmsg,
			"printer request %s-%d has been printed on printer %s",
			  dst, seqno, pr);
			if(mail)
				sendmail(logname, errmsg);
			if(wrt && !wrtmsg(logname, errmsg) && !mail)
				sendmail(logname, errmsg);
		}
		enqueue(F_MORE, pr);	/* pr ready for more requests */
	}
}


/*
 * preprint -- prepare for printing:
 *	build interface program command line
 *	make sure interface program is executable
 *	open device for writing and reading (if possible)
 */

preprint()
{
	if(sigterm)
		exit(0);
	if(buildcmd() == -1) {		/* Format interface pgm command line */
		sprintf(errmsg, "error in printer request %s-%d", dst, seqno);
		wrtmail(logname, errmsg);
		resetstatus(1, 0);
		enqueue(F_MORE, pr);
		exit(0);
	}
	if(sigterm)
		exit(0);
	opendev();	/* open printer's device */
	if(sigterm)
		exit(0);
}


/*
 * psetup -- set up printer status file and place names of enabled printers
 *	     on fifo
 */

psetup()
{
	struct pstat p;
	struct dest *d, *getp();

	while(getpent(&p) != EOF) {
		if(p.p_flags & P_ENAB) {
			if(p.p_flags & P_AUTO) {
				/* automatic disable */
				p.p_flags &= ~(P_BUSY | P_ENAB);
				p.p_pid = p.p_seqno = 0;
				sprintf(p.p_rdest, "-");
				sprintf(p.p_reason,
				"disabled by scheduler: login terminal");
				time(&p.p_date);
				putpent(&p);
			}
			else if(p.p_flags & P_BUSY) {
				p.p_flags &= ~P_BUSY;
				p.p_pid = p.p_seqno = 0;
				sprintf(p.p_rdest, "-");
				putpent(&p);
			}
		}

		if((d = getp(p.p_dest)) == NULL) {
			sprintf(errmsg, "non-existent printer %s in PSTATUS",
					p.p_dest);
			fatal(errmsg, 0);
		}
		else if(p.p_flags & P_ENAB) {
			d->d_status |= D_ENABLED;
			enqueue(F_MORE, p.p_dest);
		}
	}
	endpent();
}


/*
 * resetstatus(oflag, pflag) -- reset entries in outputq and pstatus to show
 *	that printer pr is no longer printing dst-seqno.
 *
 *	if dflag != 0 then delete outputq entry and remove associated data
 *		and request files.
 *	if pflag != 0 then reset pstatus entry for printer pr.
 */

resetstatus(oflag, pflag)
int oflag, pflag;
{
	struct outq o;
	struct pstat p;

	if(getoid(&o, dst, seqno) != EOF) {
		if(oflag != 0) {
			o.o_flags |= O_DEL;
			rmreq(dst, seqno);
		}
		else {
			o.o_flags &= ~O_PRINT;
			strcpy(o.o_dev, "-");
		}
		putoent(&o);
	}
	if(pflag != 0) {
		if(getpdest(&p, pr) != EOF) {
			p.p_flags &= ~P_BUSY;
			p.p_pid = p.p_seqno = 0;
			strcpy(p.p_rdest, "-");
			putpent(&p);
		}
		endpent();
	}
	endoent();
}


/*
 * schedule() --
 *	This routine reads a fifo (FIFO) forever.  The input on the fifo
 *	is a mix of the following messages and commands:
 *	F_ENABLE	enable printer
 *	F_DISABLE	disable printer
 *	F_ZAP		disable printer and cancel the request which
 *			it is currently printing
 *	F_MORE		printer ready for more input
 *	F_REQUEST	an output request has been received
 *	F_CANCEL	cancel output request
 *	F_STATUS	status dump
 *	F_QUIT		shut down the scheduler
 *	F_NOOP		check if scheduler is running
 *	F_NEWLOG	create new error log
 */

schedule()
{
	struct dest *d, *p;
	char msg[MSGMAX], dst[DESTMAX + 1], name[LOGMAX+1];
	char *c, *arg, cmd, dev[FILEMAX];
	int seqno;
	struct outlist *o, *geto();

	while(TRUE) {
		/* get next message from fifo */
		fgets(msg, MSGMAX-1, rfifo);
		if(*(c = msg + strlen(msg) -1) == '\n')
			*c = '\0';
		cmd = msg[0];
		arg = &msg[2];

#ifdef DEBUG
	{
		fprintf(stderr, "%s  %s\n", msg, date(time((time_t *)0)));
		fflush(stderr);
	}
#endif

		switch(cmd) {
		case F_ENABLE: /* enable printer.  arg1 = printer */
			if((d=getp(arg)) != NULL && !(d->d_status & D_ENABLED)){
				d->d_status |= D_ENABLED;
				makebusy(d);
			}
			break;
		case F_DISABLE: /* disable printer.  arg1 = printer */
		case F_ZAP:	/* disable printer and cancel what's printing */
			if((p=getp(arg)) != NULL && (p->d_status & D_ENABLED)) {
				o = NULL;
				if(p->d_status & D_BUSY) {
					waitfor(p);
					p->d_status &= ~D_BUSY;
					o = p->d_print;
					o->ol_print = NULL;
					p->d_print = NULL;
				}
				p->d_status &= ~D_ENABLED;
				if(o != NULL) {
					if(cmd == F_DISABLE)
						findprinter(o->ol_dest);
					else
						deleteo(o);
				}
			}
			break;
		case F_MORE: /* printer ready for next request.
				 arg1 = printer */
			if((d=getp(arg)) != NULL && (d->d_status & D_ENABLED)) {
				if(d->d_status & D_BUSY) {
					waitfor(d);
					d->d_status &= ~D_BUSY;
					deleteo(d->d_print);
					d->d_print = NULL;
				}
				makebusy(d);
			}
			break;
		case F_REQUEST: /* output request received.  arg1 = destination,
				  arg2 = sequence #, arg3 = logname */
			if(sscanf(arg, "%s %d %s", dst, &seqno, name) == 3 &&
			   (d=getd(dst)) != NULL) {
				inserto(d, seqno, name);
				findprinter(d);
			}
			break;
		case F_CANCEL: /* cancel output request.  arg1 = destination,
				arg2 = sequence #  */
			if(sscanf(arg, "%s %d", dst, &seqno) == 2 &&
			   (d = getd(dst)) != NULL &&
			   (o = geto(d, seqno)) != NULL) {
				if(o->ol_print != NULL) {
					deleteo(o);
					p = o->ol_print;
					waitfor(p);
					p->d_status &= ~D_BUSY;
					p->d_print = NULL;
					makebusy(p);
				}
				else
					deleteo(o);
			}
			break;
		case F_DEV: /* change device for printer.  arg1 = printer,
				arg2 = new device pathname  */
			if(sscanf(arg, "%s %s", dst, dev) == 2 &&
			   (d = getp(dst)) != NULL)
				newdev(d, dev);
			break;
		case F_QUIT: /* shut down scheduler */
			kill(pgrp, SIGTERM);
			break;
		case F_STATUS: /* status dump to error log */
			dump();
			break;
		case F_NOOP: /* no-op */
			break;
		case F_NEWLOG: /* new error log */
			unlink(OLDLOG);
			link(ERRLOG, OLDLOG);
			chmod(OLDLOG, 0644);
			unlink(ERRLOG);
			if(freopen(ERRLOG, "w", stderr) == NULL) {
				system("echo lpsched: cannot create log>>log");
				exit(1);
			}

			chmod(ERRLOG, 0644);
			fprintf(stderr,"***** LP LOG: %s *****\n",
			    date(time((time_t *)0)));
			fflush(stderr);
			break;
		default:
			sprintf(errmsg, "FIFO: '%s' ?", msg);
			fatal(errmsg, 0);
			break;
		}
	}
}


/*
 * setstatus -- update outputq and pstatus entries to reflect the printing
 *	of the current request
 */

int
setstatus()
{
	struct outq q;
	struct pstat ps;

	if(getoid(&q, dst, seqno) == EOF) {
		endoent();
		return(1);
	}
	if(getpdest(&ps, pr) == EOF) {
		endpent();
		endoent();
		disable("entry gone from printer status file");
		return(-1);
	}
	ps.p_flags |= P_BUSY;
	ps.p_pid = pid;
	strcpy(ps.p_rdest, dst);
	ps.p_seqno = seqno;

	q.o_flags |= O_PRINT;
	strcpy(q.o_dev, pr);
	putpent(&ps);
	putoent(&q);
	endpent();
	endoent();

	return(0);
}


/* sig15 -- catch SIGTERM */

sig15()
{
	signal(SIGTERM, SIG_IGN);
	sigterm = TRUE;
}



killchild()
{
	signal(SIGTERM, SIG_IGN);
	kill(-pid, SIGTERM);
}


/* startup -- Initialize */

startup(name)
char *name;
{
	extern char *f_name;
	extern int (*f_clean)();
	int i, cleanup();
	struct passwd *adm, *getpwnam();

	f_name = name;

	if(chdir(SPOOL) == -1)
		fatal("spool directory non-existent", 1);

	/* Make sure that the user is an LP Administrator */

	if(! ISADMIN)
		fatal(ADMINMSG, 1);
	if((adm = getpwnam(ADMIN)) == NULL)
		fatal("LP Administrator not in password file", 1);
	if(setgid(adm->pw_gid) == -1 ) {
		printf("pw_gid = 0x%x\n",adm->pw_gid);
		fatal("can't set group id to LP Administrator's user id", 1);
	}
	if(setuid(adm->pw_uid) == -1) {
		printf("pw_uid = 0x%x\n",adm->pw_uid);
		fatal("can't set user id to LP Administrator's user id", 1);
	}

	/* Fork here so that the parent can return to free
	   the calling process */

	if((i = fork()) == -1)
		fatal("can't fork", 1);
	else
		if(i != 0)
			exit(0);

	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, cleanup);
	signal(SIGQUIT, SIG_IGN);

	f_clean = cleanup;
	pgrp = setpgrp();
	umask(000);

	/* Lock the scheduler lock in case lpsched is invoked again */

	if(onelock(getpid(), "tmplock", SCHEDLOCK) != 0)
		/* lpsched is already running */
		exit(0);

	freopen("/dev/null", "r", stdin);
	freopen("/dev/null", "w", stdout);
	freopen(ERRLOG, "a", stderr);
	chmod(ERRLOG, 0644);
}


/*
 * waitfor(p) -- wait for termination of the process associated with printer p.
 *	In case it has not exited, it will be killed with SIGTERM.
 *	While waiting for this specific pid, the scheduler may learn of the
 *	deaths of processes associated with other printers.  In such cases,
 *	the process id field of the appropriate printer structure will be
 *	zeroed so that the scheduler doesn't make the mistake of trying to wait
 *	for a process more than once.
 */

waitfor(p)
struct dest *p;
{
	int ppid, status;
	struct dest *p1;
	extern struct dest printer;

	if((ppid = p->d_pid) != 0) {
		kill(ppid, SIGTERM);
		while(wait(&status) != ppid)
			for(p1 = printer.d_tnext; p1 != &printer;
			     p1 = p1->d_tnext)
				if(p1->d_pid == ppid) {
					p1->d_pid = 0;
					break;
				}
		p->d_pid = 0;
	}
}


/*
 * wrtmail(user, msg) -- write msg to user's tty if logged in.
 *	If not, and user hasn't explicitly requested mail, then
 *	send msg to user's mailbox.
 */

wrtmail(user, msg)
char *user;
char *msg;
{
	if(wrt) {
		if(! wrtmsg(user, msg))
			sendmail(user, msg);
	}
	else
		sendmail(user, msg);
}
