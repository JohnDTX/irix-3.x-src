char _Origin_[] = "System V";

#include	"lp.h"

SCCSID("@(#)lpadmin.c	3.1")
/* $Source: /d2/3.7/src/usr.bin/lp/RCS/lpadmin.c,v $ */
static	char	*Sccsid = "@(#)$Revision: 1.1 $";
/* $Date: 89/03/27 18:04:40 $ */

#define	TMEMBER "Tmember"
#define	TCLASS "Tclass"
#define	TPSTATUS "Tpstatus"
#define	TQSTATUS "Tqstatus"

char errmsg[FILEMAX];	/* error message */
char curdir[FILEMAX+1];	/* current directory at time of invocation */
short force = FALSE;

char enotmem[] = "printer \"%s\" is not a member of class \"%s\"";
char ememopen[] = "can't open member file";
char eclopen[] = "can't open class file";
char ebadmem[] = "corrupted member file";
char einclass[] = "printer \"%s\" already in class \"%s\"";
char ememcreat[] = "can't create new member file";
char eclcreat[] = "can't create new class file";
char epcreat[] = "can't create new printer status file";
char eqcreat[] = "can't create new acceptance status file";
char eintcreat[] = "can't create new interface program";
char erequest[] = "can't create new request directory";
char epgone[] = "printer status entry for \"%s\" has disappeared";
char enodest[] = "destination \"%s\" non-existent";
char enopr[] = "printer \"%s\" non-existent";
char eprcl[] = "can't create printer \"%s\" -- it is an existing class name";
char eclpr[] = "can't create class \"%s\" -- it is an existing printer name";
char ebaddest[] = "\"%s\" is an illegal destination name";
char emissing[] = "new printers require -v and either -e, -i or -m";
char econflict[] = "keyletters \"-%c\" and \"-%c\" are contradictory";
char eintconf[] = "keyletters -e, -i and -m are mutually exclusive";
char enoclass[] = "class \"%s\" non-existent";
char enomodel[] = "model \"%s\" non-existent";
char enoaccess[] = "can't access file \"%.50s\"";
char ermreq[] = "can't remove request directory";
char ermpr[] = "can't remove printer";
char emove[] = "requests still queued for \"%s\" -- use lpmove";
char eclrm[] = "can't remove class file";
char esame[] = "-\"%c\" and -\"%c\" keyletters have the same value";

char *c = NULL;		/* class name */
char *d = NULL;		/* default destination */
char *e = NULL;		/* existing printer -- copy its interface for p */
short h = FALSE;	/* hardwired terminal */
char *i = NULL;		/* interface pathname */
short l = FALSE;	/* login terminal */
char *m = NULL;		/* model name */
char *p = NULL;		/* printer name */
char *r = NULL;		/* class name to remove printer p from */
char *v = NULL;		/* device pathname */
char *x = NULL;		/* destination to be deleted */

short newp = FALSE;	/* true if creating a new printer */
short newc = FALSE;	/* true if creating a new class */

main(argc, argv)
int argc;
char *argv[];
{
	startup(argv[0]);

	options(argc, argv);	/* process command line options */

	chkopts(argc, argv);	/* check for legality of options */

	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	if(d)
		newdflt(d);
	else {
		if((!p || !v || argc > 3) && enqueue(F_NOOP, ""))
			fatal("can't proceed - scheduler running", 1);
		if(x)
			rmdest();
		else
			printer();
	}
	summary();

	exit(0);
}

/* addtoclass() -- add printer p to class c */

addtoclass()
{
	char class[2*DESTMAX+2];
	FILE *cl;

	sprintf(class, "%s/%s", CLASS, c);
	if((cl = fopen(class, "a")) == NULL)
		fatal(eclopen, 1);
	fprintf(cl, "%s\n", p);
	fclose(cl);
	if(newc)
		newq(c);
}

/* catch -- catch signals */

catch()
{
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	cleanup();
	exit(1);
}

/* chkopts -- check legality of command line options */


chkopts(argc, argv)
int argc;
char *argv[];
{
	char *tmp, *fullpath();

	if(d) {
		if(argc > 2)
			usage();
		if(*d != '\0' && !isdest(d)) {
			fprintf(stderr,"lp:can't access %s/%s/%s:",
			  SPOOL,REQUEST,d);
			perror("");
			sprintf(errmsg, enodest, d);
			fatal(errmsg, 1);
		}
	}
	else if(x) {
		if(argc > 2)
			usage();
		if(!isdest(x)) {
			fprintf(stderr,"lp:can't access %s/%s/%s:",
			  SPOOL,REQUEST,x);
			perror("");
			sprintf(errmsg, enodest, x);
			fatal(errmsg, 1);
		}
	}
	if(d || x)
		return;
	if(!p)
		usage();
	if(!isprinter(p)) {
		if(isclass(p)) {
			sprintf(errmsg, eprcl, p);
			fatal(errmsg, 1);
		}
		if(! legaldest(p)) {
			sprintf(errmsg, ebaddest, p);
			fatal(errmsg, 1);
		}
		if(!v || !(e || i || m))
			fatal(emissing, 1);
		if(r) {
			sprintf(errmsg, enotmem, p, r);
			fatal(errmsg, 1);
		}
		newp = TRUE;
	}

	if(c) {
		if(! isclass(c)) {
			if(isprinter(c)) {
				sprintf(errmsg, eclpr, c);
				fatal(errmsg, 1);
			}
			if(! legaldest(c)) {
				sprintf(errmsg, ebaddest, c);
				fatal(errmsg, 1);
			}
			newc = TRUE;
		}
	}
	if((i && m) || (i && e) || (m && e))
		fatal(eintconf, 1);

	if(e) {
		if(!isprinter(e)) {
			sprintf(errmsg, enopr, e);
			fatal(errmsg, 1);
		}
		if(strcmp(e, p) == 0) {
			sprintf(errmsg, esame, 'e', 'p');
			fatal(errmsg, 1);
		}
	}

	if(i) {
		tmp = fullpath(i, curdir);
		if(tmp == NULL)
			fatal("can't read current directory", 1);
		if(eaccess(tmp, ACC_R) != 0) {
			sprintf(errmsg, enoaccess, tmp);
			fatal(errmsg, 1);
		}
	}

	if(m && !ismodel(m)) {
		sprintf(errmsg, enomodel, m);
		fatal(errmsg, 1);
	}

	if(h && l) {
		sprintf(errmsg, econflict, 'h', 'l');
		fatal(errmsg, 1);
	}

	if(r && !isclass(r)) {
		sprintf(errmsg, enoclass, r);
		fatal(errmsg, 1);
	}

	if(c && r && strcmp(c, r) == 0) {
		sprintf(errmsg, esame, 'c', 'r');
		fatal(errmsg, 1);
	}

	if(v) {
		tmp = fullpath(v, curdir);
		if(tmp == NULL)
			fatal("can't read current directory", 1);
		if(eaccess(tmp, ACC_W) != 0) {
			sprintf(errmsg, enoaccess, tmp);
			fatal(errmsg, 1);
		}
	}
}

/*
 *	chkreq() -- make sure that no requests are pending for destination x.
 *	Deleting printer x may imply the deletion of one or more
 *	classes.  Make sure that no requests are pending for these
 *	classes, also.
*/

chkreq()
{
	struct outq out;
	FILE *mf, *cf;
	char member[2*DESTMAX+2], class[2*DESTMAX+2];
	char mem[DESTMAX+2], cl[DESTMAX+2], *getline();
	int nmem;

	if(getodest(&out, x) != EOF) {
		sprintf(errmsg, emove, x);
		fatal(errmsg, 1);
	}

	sprintf(member, "%s/%s", MEMBER, x);
	if((mf = fopen(member, "r")) == NULL)
		fatal(ememopen, 1);

	getline(errmsg, FILEMAX, mf);
	while(getline(cl, DESTMAX, mf) != NULL) {
		sprintf(class, "%s/%s", CLASS, cl);
		if((cf = fopen(class, "r")) == NULL)
			fatal(eclopen, 1);

		nmem = 0;
		while(getline(mem, DESTMAX, cf) != NULL)
			if(strcmp(mem, x) != 0)
				nmem++;
		if(nmem == 0) {
			setoent();
			if(getodest(&out, cl) != EOF) {
				sprintf(errmsg, emove, cl);
				fatal(errmsg, 1);
			}
		}
		fclose(cf);
	}
	fclose(mf);
	endoent();
}

/* cleanup -- called by catch() after interrupts or by fatal() after errors */

cleanup()
{
	tunlock();
	endqent();
	endpent();
	endoent();
}

/* copy(name1, name2) -- copy file name1 to name2 */

copy(name1, name2)
char *name1, *name2;
{
	FILE *f1, *f2;
	int nch;
	char buf[BUFSIZ];

	if((f1 = fopen(name1, "r")) == NULL ||
	    (f2 = fopen(name2, "w")) == NULL) {
		if (f1 == NULL)
			printf("lpadmin:copy:can't open %s for reading\n",
			  name1);
		if (f2 == NULL)
			printf("lpadmin:copy:can't open %s for writing\n",
			  name2);
		return(-1);
	}
	while((nch = fread(buf, 1, BUFSIZ, f1)) > 0)
		fwrite(buf, 1, nch, f2);
	fclose(f1);
	fclose(f2);
	return(0);
}

/* fromclass() -- remove printer p from class r */

fromclass()
{
	char class[2*DESTMAX + 2], member[DESTMAX + 2];
	char *getline(), *getdflt();
	FILE *ocl, *cl;
	int nmem = 0;

	sprintf(class, "%s/%s", CLASS, r);
	if((ocl = fopen(class, "r")) == NULL)
		fatal(eclopen, 1);
	if((cl = fopen(TCLASS, "w")) == NULL)
		fatal(eclopen, 1);

	while(getline(member, DESTMAX, ocl) != NULL) {
		if(strcmp(member, p) != 0) {
			nmem++;
			fprintf(cl, "%s\n", member);
		}
	}

	if(nmem == 0) {
		if(rmreqdir(r) != 0) {
			unlink(TCLASS);
			sprintf(errmsg, emove, r);
			fatal(errmsg, 1);
		}
		if(unlink(class) < 0)
			fatal(eclrm, 1);
		rmqent(r);
		if(strcmp(getdflt(), r) == 0)
			newdflt(NULL);
	}
	else {
		unlink(class);
		if(link(TCLASS, class) < 0)
			fatal(eclcreat, 1);
	}
	unlink(TCLASS);
	fclose(ocl);
	fclose(cl);
}

/* getdflt() -- return system default destination */

char *
getdflt()
{
	static char dflt[DESTMAX + 2];
	char *getline();
	FILE *df;

	if((df = fopen(DEFAULT, "r")) == NULL)
		dflt[0] = '\0';
	else if(getline(dflt, DESTMAX, df) == NULL)
		dflt[0] = '\0';
	if(df)
		fclose(df);
	return(dflt);
}

/* getline(str, max, file) -- get string str of max length from file */

char *
getline(str, max, file)
char *str;
int max;
FILE *file;
{
	char *fgets();
	register char *c;

	if(fgets(str, max, file) == NULL)
		return(NULL);
	if(*(c = str + strlen(str) - 1) == '\n')
		*c = '\0';
	return(str);
}

/* ismodel(name) -- predicate which returns TRUE iff name is a model,
		 FALSE, otherwise.		*/

#define	MODEL	"model"

ismodel(name)
char *name;
{
	char model[FILEMAX];

	if(*name == '\0' || strlen(name) > DESTMAX)
		return(FALSE);

	/* Check model directory */

	sprintf(model, "%s/%s/%s", SPOOL, MODEL, name);
	return(eaccess(model, ACC_R | ACC_W) != -1);
}

/* legaldest(d) -- returns TRUE if d is a syntactically correct destination
	name, FALSE otherwise.			*/

legaldest(d)
char *d;
{
	char c;

	if(strlen(d) > DESTMAX)
		return(FALSE);
	while(c = *d++)
		if(! isalnum(c) && c != '_')
			return(FALSE);
	return(TRUE);
}

/* memberfile() -- make new member file for printer p */

memberfile()
{
	char member[2*DESTMAX + 2], class[DESTMAX + 2], dev[FILEMAX];
	char *getline(), *fullpath(), *fp;
	short removed = FALSE;
	FILE *mf, *omf;

	sprintf(member, "%s/%s", MEMBER, p);
	if(! newp && (omf = fopen(member, "r")) == NULL)
		fatal(ememopen, 1);
	if((mf = fopen(TMEMBER, "w")) == NULL)
		fatal(ememopen, 1);

	if(! newp && getline(dev, FILEMAX, omf) == NULL)
		fatal(ebadmem, 1);
	fprintf(mf, "%s\n", v ? (fp = fullpath(v, curdir)) : dev);
	if(v) {
		sprintf(errmsg, "%s %s", p, fp);
		enqueue(F_DEV, errmsg);
	}

	if(!newp) {
		removed = FALSE;
		while(getline(class, DESTMAX, omf) != NULL) {
			if(r && strcmp(class, r) == 0)
				removed = TRUE;
			else {
				if(c && strcmp(class, c) == 0) {
					sprintf(errmsg, einclass, p, c);
					fatal(errmsg, 0);
					c = NULL;
				}
				fprintf(mf, "%s\n", class);
			}
		}
	}
	if(c)
		fprintf(mf, "%s\n", c);
	if(r && !removed) {
		sprintf(errmsg, enotmem, p, r);
		fatal(errmsg, 0);
		r = NULL;
	}
	if(!newp) {
		fclose(omf);
		unlink(member);
	}
	fclose(mf);
	if(link(TMEMBER, member) < 0)
		fatal(ememcreat, 1);
	unlink(TMEMBER);
}

/* mkrequest(name) -- make new request directory for name */

mkrequest(name)
char *name;
{
	char request[2*DESTMAX + 2];

	sprintf(request, "%s/%s", REQUEST, name);
	sprintf(errmsg, "mkdir %s", request);
	if(system(errmsg) != 0)
		fatal(erequest, 1);
}

/* newdflt(name) -- change system default destination to name */

newdflt(name)
char *name;
{
	FILE *df;

	if((df = fopen(DEFAULT, "w")) == NULL)
		fatal("can't open system default destination file", 1);
	if(name && *name != '\0')
		fprintf(df, "%s\n", name);
	fclose(df);
}

/* newinter() -- change interface for printer p */

newinter()
{
	char interface[2*DESTMAX+2], file[2*DESTMAX+2];
	char *new;

	sprintf(interface, "%s/%s", INTERFACE, p);
	if(m || e) {
		sprintf(file, "%s/%s", m ? MODEL : INTERFACE, m ? m : e);
		new = file;
	}
	else
		new = fullpath(i, curdir);
	if(! newp)
		unlink(interface);
	if(copy(new, interface) != 0)
		fatal(eintcreat, 1);
	chmod(interface, 0755);
}

/* newmode() -- change mode of existing printer to hardwired or login */

newmode()
{
	struct pstat pr;

	if(getpdest(&pr, p) == EOF) {
		sprintf(errmsg, epgone, p);
		fatal(errmsg, 1);
	}
	if(h)
		pr.p_flags &= ~P_AUTO;
	else	/* l must be set */
		pr.p_flags |= P_AUTO;
	putpent(&pr);
	endpent();
}

/* newprinter() -- add new printer p */

newprinter()
{
	struct pstat pr;

	strcpy(pr.p_dest, p);
	pr.p_rdest[0] = '\0';
	pr.p_pid = pr.p_seqno = 0;
	time(&pr.p_date);
	sprintf(pr.p_reason, "new printer");
	pr.p_flags = 0;
	if(l)
		pr.p_flags |= P_AUTO;
	addpent(&pr);
	newq(p);
	endpent();
}

/* newq(name) -- create new qstatus entry for name */

newq(name)
char *name;
{
	struct qstat acc;

	strcpy(acc.q_dest, name);
	acc.q_accept = FALSE;
	time(&acc.q_date);
	sprintf(acc.q_reason, "new destination");
	addqent(&acc);
	endqent();
}

/* options -- process command line options */

options(argc, argv)
int argc;
char *argv[];
{
	int j;
	char letter;		/* current keyletter */
	char *value;		/* value of current keyletter (or NULL) */
	char *strchr();

	if(argc == 1)
		usage();

	for(j = 1; j < argc; j++) {
		if(argv[j][0] != '-' || (letter = argv[j][1]) == '\0')
			usage();
		if(! isalpha(letter)) {
			sprintf(errmsg, "illegal keyletter \"%c\"", letter);
			fatal(errmsg, 1);
		}
		letter = tolower(letter);
		value = &argv[j][2];

		switch(letter) {
		case 'c':	/* class in which to insert printer p */
			c = value;
			break;
		case 'd':	/* system default destination */
			d = value;
			break;
		case 'e':	/* copy existing printer interface */
			e = value;
			break;
		case 'h':	/* hardwired terminal */
			h = TRUE;
			break;
		case 'i':	/* interface pathname */
			i = value;
			break;
		case 'l':	/* login terminal */
			l = TRUE;
			break;
		case 'm':	/* model interface */
			m = value;
			break;
		case 'p':	/* printer name */
			p = value;
			break;
		case 'r':	/* class name to remove printer p from */
			r = value;
			break;
		case 'v':	/* device pathname */
			v = value;
			break;
		case 'x':	/* destination to be deleted */
			x = value;
			break;
		default:
			sprintf(errmsg, "unknown keyletter \"-%c\"", letter);
			fatal(errmsg, 1);
			break;
		}

		if(*value == '\0' && strchr("dhl", letter) == NULL) {
			sprintf(errmsg, "keyletter \"%c\" requires a value",
			    letter);
			fatal(errmsg, 1);
		}

	}
}

/* printer -- add new printer p or modify printer p */

printer()
{
	if(r)
		fromclass();
	if(newp)
		newprinter();
	else if(h || l)
		newmode();
	if(c || r || v)
		memberfile();
	if(c)
		addtoclass();
	if(e || i || m)
		newinter();
	if(newp)
		mkrequest(p);
	if(newc)
		mkrequest(c);
}

/* rmclass() -- remove class file for class x */

rmclass()
{
	char member[2*DESTMAX+2], class[2*DESTMAX+2];
	char mem[DESTMAX+2], cl[DESTMAX+2];
	char *getline();
	FILE *cf, *mf, *omf;

	sprintf(class, "%s/%s", CLASS, x);
	if((cf = fopen(class, "r")) == NULL)
		fatal(ememopen, 1);

	while(getline(mem, DESTMAX, cf) != NULL) {
		sprintf(member, "%s/%s", MEMBER, mem);
		if((omf = fopen(member, "r")) == NULL ||
		    (mf = fopen(TMEMBER, "w")) == NULL)
			fatal(ememopen, 1);
		fprintf(mf, "%s\n", getline(errmsg, FILEMAX, omf));
		while(getline(cl, DESTMAX, omf) != NULL)
			if(strcmp(cl, x) != 0)
				fprintf(mf, "%s\n", cl);
		fclose(mf);
		fclose(omf);
		unlink(member);
		if(link(TMEMBER, member) != 0)
			fatal(ememcreat, 1);
		unlink(TMEMBER);
	}
	fclose(cf);
	if(unlink(class) != 0)
		fatal(eclrm, 1);
}

/* rmdest -- remove destination x */

rmdest()
{
	char *getdflt();

	if(isclass(x)) {
		if(rmreqdir(x) != 0) {
			sprintf(errmsg, emove, x);
			fatal(errmsg, 1);
		}
		rmqent(x);
		rmclass();
	}
	else {
		chkreq();
		rmprinter();
	}

	if(strcmp(getdflt(), x) == 0)
		newdflt(NULL);
}

/* rmpent(name) -- remove name's pstatus entry */

rmpent(name)
char *name;
{
	FILE *pf;
	struct pstat ps;

	if((pf = fopen(TPSTATUS, "w")) == NULL)
		fatal(epcreat, 1);
	while(getpent(&ps) != EOF)
		if(strcmp(ps.p_dest, name) != 0)
			wrtpent(&ps, pf);
	fclose(pf);
	unlink(PSTATUS);
	if(link(TPSTATUS, PSTATUS) < 0)
		fatal(epcreat, 1);
	unlink(TPSTATUS);
	endpent();
}

/* rmprinter() -- remove printer x */

rmprinter()
{
	char member[2*DESTMAX+2], file[2*DESTMAX+2];
	char mem[DESTMAX+2], cl[2*DESTMAX+2], *getline();
	FILE *mf, *cf, *ocf;
	int nmem;

	force = TRUE;
	rmreqdir(x);
	rmqent(x);
	rmpent(x);

	sprintf(member, "%s/%s", MEMBER, x);
	if((mf = fopen(member, "r")) == NULL)
		fatal(ememopen, 1);

	getline(errmsg, FILEMAX, mf);
	while(getline(cl, DESTMAX, mf) != NULL) {
		sprintf(file, "%s/%s", CLASS, cl);
		if((ocf = fopen(file, "r")) == NULL ||
		    (cf = fopen(TCLASS, "w")) == NULL)
			fatal(eclopen, 1);

		nmem = 0;
		while(getline(mem, DESTMAX, ocf) != NULL) {
			if(strcmp(mem, x) != 0) {
				fprintf(cf, "%s\n", mem);
				nmem++;
			}
		}
		unlink(file);
		if(nmem == 0) {
			rmreqdir(cl);
			rmqent(cl);
		}
		else
			if(link(TCLASS, file) != 0)
				fatal(eclcreat, 1);
		unlink(TCLASS);
		fclose(ocf);
		fclose(cf);
	}
	fclose(mf);
	if(unlink(member) != 0)
		fatal(ermpr, 1);
	sprintf(file, "%s/%s", INTERFACE, x);
	unlink(file);
}

/* rmqent(name) -- remove name's qstatus entry */

rmqent(name)
char *name;
{
	FILE *qf;
	struct qstat qs;

	if((qf = fopen(TQSTATUS, "w")) == NULL)
		fatal(eqcreat, 1);
	while(getqent(&qs) != EOF)
		if(strcmp(qs.q_dest, name) != 0)
			wrtqent(&qs, qf);
	fclose(qf);
	unlink(QSTATUS);
	if(link(TQSTATUS, QSTATUS) < 0)
		fatal(eqcreat, 1);
	unlink(TQSTATUS);
	endqent();
}

/* rmreqdir(name) -- remove name's request directory */

rmreqdir(name)
char *name;
{
	struct outq oq;

	if(!force && getodest(&oq, name) != EOF) {
		endoent();
		return(-1);
	}
	sprintf(errmsg, "rm -fr %s/%s", REQUEST, name);
	if(system(errmsg) != 0)
		fatal(ermreq, 1);
	endoent();
	return(0);
}

/* startup -- initialization routine */

startup(name)
char *name;
{
	int catch(), cleanup();
	struct passwd *adm, *getpwnam();
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

	umask(0022);
	f_name = name;
	f_clean = cleanup;

	if(! ISADMIN)
		fatal(ADMINMSG, 1);

	if((adm = getpwnam(ADMIN)) == NULL)
		fatal("LP Administrator not in password file\n", 1);

	if(setgid(adm->pw_gid) == -1 || setuid(adm->pw_uid) == -1)
		fatal("can't set user id to LP Administrator's user id", 1);

	gwd(curdir);			/* get current directory */

	if(chdir(SPOOL) == -1)
		fatal("spool directory non-existent", 1);

}

/* summary -- print summary of actions taken */

summary()
{
}

/* usage -- print command usage message and exit */

usage()
{
	printf("usages:\tlpadmin -pprinter [-vdevice] [-cclass] [-rclass]\n");
	printf("\t     [-eprinter|-iinterface|-mmodel] [-h|-l]\n");
	printf("\t\t-or-\n");
	printf("\tlpadmin -d[destination]\n");
	printf("\t\t-or-\n");
	printf("\tlpadmin -xdestination\n");
	exit(0);
}
