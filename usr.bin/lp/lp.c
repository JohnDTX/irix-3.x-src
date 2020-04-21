char _Origin_[] = "System V";

/* lp -- print files on a line printer */

#include	"lp.h"

SCCSID("@(#)lp.c	3.3")
/* $Source: /d2/3.7/src/usr.bin/lp/RCS/lp.c,v $ */
static	char	*Sccsid = "@(#)$Revision: 1.1 $";
/* $Date: 89/03/27 18:04:36 $ */

char work[FILEMAX];	/* Global error message string */
struct stat stbuf;	/* Global stat buffer */
char title[TITLEMAX + 1] = NULL;	/* User-supplied title for output */
int copies = 1;		/* number of copies of output */
int optlen = 0;		/* size of option string for interface  program */
char opts[OPTMAX] = NULL;	/* options for interface program */
int silent = 0;		/* don't run off at the mouth */
short mail = FALSE;	/* TRUE ==> user wants mail, FALSE ==> no mail */
short wrt = FALSE;	/* TRUE ==> user wants notification on tty via write,
			   FALSE ==> don't write */
short copy = FALSE;	/* TRUE ==> copy files, FALSE ==> don't */
char curdir[FILEMAX+1];	/* working directory at time of request */

int nfiles = 0;		/* number of files on cmd line (excluding "-") */
int fileargs = 0;	/* total number of file args */
int stdinp = 0;		/* indicates how many times to print std input
			   -1 ==> standard input empty		*/
short slocked = FALSE;	/* TRUE ==> sequence file is locked, FALSE ==> isn't */

char tname[RNAMEMAX] = NULL;	/* name of temp request file */
FILE *tfile = NULL;		/* stream for temp request file */
char rname[RNAMEMAX] = NULL;	/* name of actual request file */
char stdbase[NAMEMAX];	/* basename of copy of standard input */
char reqid[IDSIZE + 1];	/* request id to be supplied to user */

struct outq o = {	/* output request to be appended to output queue */
	NULL,	/* destination */
	NULL,	/* logname */
	0,	/* sequence # */
	0L,	/* size of request */
	NULL,	/* device where printing */
	0L,	/* date of request */
	0	/* not printing, not deleted */
};

main(argc, argv)
int argc;
char *argv[];
{
	struct qstat q;		/* acceptance status */

	startup(argv[0]);

	options(argc, argv);	/* process command line options */

	if(chdir(SPOOL) == -1) {
		printf("lp:can't chdir (%s)\n",SPOOL);
		fatal("spool directory non-existent", 1);
	}

	defaults();		/* establish default parameters */

	if(getqdest(&q, o.o_dest) == EOF) {	/* get acceptance status */
		sprintf(work,
		  "acceptance status of destination \"%s\" unknown", o.o_dest);
		fatal(work, 1);
	}
	endqent();
	if(! q.q_accept) {	/* accepting requests ? */
		sprintf(work,
			"can't accept requests for destination \"%s\" -\n\t%s",
			o.o_dest, q.q_reason);
		fatal(work, 1);
	}

	openreq();		/* create and init request file */

	putrent(R_TITLE, title, tfile);
	sprintf(work, "%d", copies);
	putrent(R_COPIES, work, tfile);
	putrent(R_OPTIONS, opts, tfile);

	if(stdinp > 0)
		savestd();	/* save standard input */

	files(argc, argv);	/* process command line file arguments */

	closereq();		/* complete and then close request file */

	time(&o.o_date);
	o.o_size *= copies;
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	addoent(&o);		/* enter request in output queue */
	endoent();
	sprintf(work, "%s %d %s", o.o_dest, o.o_seqno, o.o_logname);
	enqueue(F_REQUEST, work);	/* notify scheduler of queued output */

	qmesg();		/* issue request id message */

	exit(0);
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
/* cleanup -- called by catch() after interrupts or by fatal() after errors */

cleanup()
{
	/* Unlink request and data files (if any) */

	rmreq(o.o_dest, o.o_seqno);

	/* Unlock lock files and temp lock file */

	tunlock();
	if(slocked)
		unlock(SEQLOCK);
	endoent();
	endqent();
}
/* closereq -- complete and close request file */

closereq()
{
	if(fileargs == 0)
		putrent(R_FILE, stdbase, tfile);

	if(mail)
		putrent(R_MAIL, o.o_logname, tfile);
	if(wrt)
		putrent(R_WRITE, o.o_logname, tfile);

	/* Rename temporary request file to real request file name */

	fclose(tfile);
	if(link(tname, rname) == -1) {
		printf("lp:link(%s,%s) failed\n",tname,rname);
		sprintf(work, "can't create request file %s", rname);
		fatal(work, 1);
	}
	unlink(tname);
}
/*
 * copyfile(stream, name) -- copy stream to file "name"
 *	name is actually just the basename of a file
 *	the file is created under REQUEST/o.o_dest/
 */

copyfile(stream, name)
FILE *stream;
char *name;
{
	FILE *ostream;
	int i;
	char buf[BUFSIZ];

	sprintf(work, "%s/%s/%s", REQUEST, o.o_dest, name);
	if((ostream = fopen(work, "w")) == NULL) {
		printf("lp:can't open %s for writing\n",work);
		sprintf(work, "cannot create temp file %s", name);
		fatal(work, 1);
	}

	chmod(work, 0444);
	while((i = fread(buf, sizeof(char), BUFSIZ, stream)) > 0)
		fwrite(buf, sizeof(char), (unsigned) i, ostream);

	fclose(ostream);
}
/* defaults -- establish default destination if not set on command line */

defaults()
{
	char *d, *getenv(), *c, *strcpy();
	FILE *dflt;	/* stream for reading default destination file */

	if(o.o_dest[0] == '\0') {	/* avoid optimiser bug */
		if((d = getenv(LPDEST)) != NULL && *d != '\0') {
			if(strlen(d) > DESTMAX) {
				sprintf(work,
				  "%s destination \"%s\" illegal",
				  LPDEST, d);
				fatal(work, 1);
			}
			strcpy(o.o_dest, d);
		}
		else {
			if((dflt = fopen(DEFAULT, "r")) == NULL) {
				printf("lp:can't open %s\n",DEFAULT);
				fatal("can't open default destination file",1);
			}
			if(fgets(o.o_dest, DESTMAX+1, dflt) == NULL) {
				fatal("no system default destination", 1);
			}
			fclose(dflt);
			if(*(c = o.o_dest + strlen(o.o_dest) -1) == '\n')
				*c = '\0';
		}

		if(! isdest(o.o_dest)) {
			fprintf(stderr,"lp:can't access %s/%s/%s:",
			  SPOOL,REQUEST,o.o_dest);
			perror("");
			sprintf(work,
				"default destination \"%s\" non-existent",
				o.o_dest);
			fatal(work, 1);
		}
	}
}
/* files -- process command line file arguments */

files(argc, argv)
int argc;
char *argv[];
{
	int i;
	char *fullpath(), *file, *dname, *newname(), *full;
	FILE *f;

	for(i = 1; i < argc; i++) {
		file = argv[i];
		if(file == NULL)
			continue;
		if(strcmp(file, "-") == 0) {
			if(stdinp > 0)
				putrent(R_FILE, stdbase, tfile);
		}
		else {
			if((full = fullpath(file, curdir)) == NULL)
				fatal("can't read current directory", 1);
			dname = newname();
			if(copy) {
				if((f=fopen(full,"r")) != NULL) {
					copyfile(f, dname);
					fclose(f);
				}
				else {
					sprintf(work, "can't open file %s",
					    full);
					fatal(work, 1);
				}
				putrent(R_FILE, dname, tfile);
			}
			else {
				sprintf(work, "%s/%s/%s", REQUEST,
				    o.o_dest, dname);
				if(link(file, work) < 0)
					putrent(R_FILE, full, tfile);
				else
					putrent(R_FILE, dname, tfile);
			}
		}
	}
}
/* getseq(snum) -- get next sequence number */

getseq(snum)
int *snum;
{
	FILE *fp;

	if (trylock(SEQLOCK, LOCKTIME, LOCKTRIES, LOCKSLEEP) == -1) {
		printf("lp:can't grab sequence file lock %s\n",SEQLOCK);
		fatal("can't lock sequence number file", 1);
	}
	slocked = TRUE;
	if ((fp = fopen(SEQFILE, "r")) != NULL) {
		/* read sequence number file */
		fscanf(fp, "%d\n", snum);
		fp = freopen(SEQFILE, "w", fp);
	}
	else {
		/* can not read file - create a new one */
		if ((fp = fopen(SEQFILE, "w")) == NULL) {
			printf("lp:can't open %s for writing\n",SEQFILE);
			fatal("can't create new sequence number file", 1);
		}
		*snum = 0;
	}

	chmod(SEQFILE, 0644);
	if(++(*snum) == SEQMAX)
		*snum = 1;
	fprintf(fp, "%d\n", *snum);
	fclose(fp);
	unlock(SEQLOCK);
	slocked = FALSE;
}
/* newname() -- create new name for data file
	returns the new name, which is just the base name for the file.
	The file will ultimately be created under SPOOL/REQUEST/o.o_dest/
*/

char *
newname()
{
	static int n = 0;
	static char name[NAMEMAX];

	sprintf(name, "d%d-%d", n++, o.o_seqno);
	return(name);
}
/* openreq -- open and initialize request file */

openreq()
{
	while(tfile == NULL) {
		getseq(&o.o_seqno);
		sprintf(tname, "%s/%s/t-%d", REQUEST, o.o_dest, o.o_seqno);
		sprintf(rname, "%s/%s/r-%d", REQUEST, o.o_dest, o.o_seqno);

		if(eaccess(tname, 0) == -1 && eaccess(rname, 0) == -1 &&
		   (tfile = fopen(tname, "w")) != NULL)
			chmod(tname, 0444);
	}

	sprintf(reqid, "%s-%d", o.o_dest, o.o_seqno);
}
/* options -- process command line options */

#define	OPTMSG	"too many options for interface program"

options(argc, argv)
int argc;
char *argv[];
{
	int i;
	char letter;		/* current keyletter */
	char *value;		/* value of current keyletter (or NULL) */
	char *strcat(), *strncpy(), *file;

	for(i = 1; i < argc; i++) {
		if(argv[i][0] == '-' && (letter = argv[i][1]) != '\0') {
			if(! isalpha(letter)) {
				sprintf(work, "unknown keyletter \"%c\"",
					letter);
				fatal(work, 1);
			}
			letter = tolower(letter);

			value = &argv[i][2];

			switch(letter) {

			case 'c':	/* copy files */
				copy = TRUE;
				break;
			case 'd':	/* destination */
				if(! isdest(value)) {
					fprintf(stderr,
					  "lp:can't access %s/%s/%s:",
					  SPOOL,REQUEST,value);
					sprintf(work,
					  "destination \"%s\" non-existent",
					  value);
					fatal(work, 1);
				}
				strncpy(o.o_dest, value, DESTMAX);
				o.o_dest[DESTMAX] = NULL;
				break;

			case 'm':	/* mail */
				mail = TRUE;
				break;
			case 'n':	/* # of copies */
				if(*value == '\0' || (copies=atoi(value)) <= 0)
					copies = 1;
				break;
			case 'o':	/* option for interface program */
				if(*value != '\0') {
					if(optlen == 0)
						optlen = strlen(value);
					else
						optlen = strlen(value) + 1;
					if(optlen >= OPTMAX)
						fatal(OPTMSG, 1);
					if(*opts != '\0')
						strcat(opts, " ");
					strcat(opts, value);
				}
				break;
			case 's':	/* silent */
				silent = 1;
				break;
			case 't':	/* title */
				strncpy(title, value, TITLEMAX);
				title[TITLEMAX] = '\0';
				break;
			case 'w':	/* write */
				wrt = TRUE;
				break;
			default:
				sprintf(work, "unknown keyletter \"-%c\"",
				  letter);
				fatal(work, 1);
				break;
			}

			argv[i] = NULL;
		}
		else {		/* file name or - */
			fileargs++;
			file = argv[i];
			if(strcmp(file, "-") == 0)
				stdinp++;
			else {
				if(stat(file, &stbuf) == -1) {
					sprintf(work,
					    "can't access file \"%s\"", file);
					goto badf;
				}
				else if((stbuf.st_mode & S_IFMT) == S_IFDIR) {
					sprintf(work,
					    "\"%s\" is a directory", file);
					goto badf;
				}
				else if(stbuf.st_size == 0) {
					sprintf(work, 
					    "file \"%s\" is empty", file);
					goto badf;
				}
				else if(eaccess(argv[i], ACC_R) == -1) {
					sprintf(work,
					    "can't access file \"%s\"", file);
					goto badf;
				}
				else {
					nfiles++;
					o.o_size += stbuf.st_size;
				}
			}
		}
		continue;

badf:		fatal(work, 0);
		argv[i] = NULL;
	}

	if(fileargs == 0)
		stdinp = 1;
	else if(nfiles == 0 && stdinp == 0)
		fatal("request not accepted", 1);
}
/* qmesg -- issue request id message */

qmesg()
{
	if(silent) return;	/* not so elegant but 'twill do */
	printf("request id is %s (", reqid);
	if(nfiles > 0) {
		printf("%d file", nfiles);
		if(nfiles > 1)
			printf("s");
	}
	if(stdinp > 0) {
		if(nfiles > 0)
			printf(" and ");
		printf("standard input");
	}
	printf(")\n");
}
/* savestd -- save standard input */

savestd()
{
	char *newname();

	strcpy(stdbase, newname());
	copyfile(stdin, stdbase);
	sprintf(work, "%s/%s/%s", REQUEST, o.o_dest, stdbase);
	stat(work, &stbuf);
	if(stbuf.st_size == 0) {
		fatal("standard input is empty", 0);
		unlink(work);
		if(nfiles == 0)		/* no files to queue */
			fatal("request not accepted", 1);
		else	/* inhibit printing of std input */
			stdinp = -1;
	}
	else
		o.o_size += (stdinp * stbuf.st_size);
}
/* startup -- initialization routine */

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

	umask(0000);
	f_name = name;
	f_clean = cleanup;
	gwd(curdir);			/* get current directory */

	strcpy(o.o_logname, getname());
	strcpy(o.o_dev, "-");
}
