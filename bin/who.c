char _Origin_[] = "System V";

/*	@(#)who.c	1.4	*/

/* Program to read the "/etc/utmp" file and list the various */
/* entries. */

#include <sys/types.h>
#include <sys/stat.h>
#include <utmp.h>
#include <stdio.h>
#include <pwd.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>

#define TRUE	1
#define FALSE	0
#define FAILURE	-1

#define MAXLINE	100

#define FLAGS	UTMAXTYPE

#define SIZEUSER	8

char username[SIZEUSER + 1];

int shortflag;		/* Set when fast form of "who" is to run. */
int ttystatus;		/* Set when write to tty status desired. */
int allflag;		/* Set when all entries are to be dumped. */

main(argc,argv)
int argc;
register char **argv;
{
	static int entries[FLAGS+1];	/* Flag for each type entry */
	register struct utmp *utmp;
	extern struct utmp *getutent();
	register int i;
	char *ptr,*eptr,*tptr;
	extern char *file,*strncpy(),*user(),*mode(),*line(),*etime(),*id();
	extern char *loc(),*ttyname(),*trash();
	char termstat[4],exitstat[4];
	struct passwd *passwd;
	extern char username[];
	extern struct passwd *getpwuid();
	char outbuf[BUFSIZ];
	extern int shortflag,ttystatus,allflag;

/* Make "stdout" buffered to speed things up. */

	setbuf(stdout, outbuf);

/* Look up our own name for reference purposes. */

	if((passwd = getpwuid(getuid())) == (struct passwd *)NULL) {
		 username[0] = '\0';
	} else {
/* Copy the user's name to the array "username". */
		 strncpy(&username[0],passwd->pw_name,SIZEUSER);
		 username[SIZEUSER] = '\0';
	}

/* Set "ptr" to null pointer so that in normal case of "who" */
/* the "ut_line" field will not be looked at.  "ptr" is used */
/* only for "w me" command. */

	ptr = NULL;
	shortflag = 0;		/* not set yet */
	ttystatus = FALSE;

	++argv;
	--argc;

	while(argc > 0) {
/* Is this the "who am i" sequence? */

		if(argc >= 2 && strcmp(*argv,"am") == 0 &&
		   strcmp(*(argv+1),"i") == 0) {
			entries[USER_PROCESS] = TRUE;

/* Which tty am I at?  Get the name and set "ptr" to just past */
/* the "/dev/" part of the pathname. */

			if((ptr = ttyname(fileno(stdin))) == NULL &&
			   (ptr = ttyname(fileno(stdout))) == NULL &&
			   (ptr = ttyname(fileno(stderr))) == NULL) {
				usage("process not attached to terminal");
			}
			ptr += sizeof("/dev/") - 1;
			argc -= 2;
			argv += 2;
			continue;
		} else if(**argv == '-') {

/* Analyze the switches and set the flags for each type of entry */
/* that the user requests to be printed out. */

			while(*++*argv) {
				switch(**argv) {
				case 'r' :
					entries[RUN_LVL] = TRUE;
					break;
				case 'b' :
					entries[BOOT_TIME] = TRUE;
					break;
				case 't' :
					entries[OLD_TIME] = TRUE;
					entries[NEW_TIME] = TRUE;
					break;
				case 'p' :
					entries[INIT_PROCESS] = TRUE;
					break;
				case 'l' :
					entries[LOGIN_PROCESS] = TRUE;
					if(shortflag == 0) shortflag = -1;
					break;
				case 'u' :
					entries[USER_PROCESS] = TRUE;
					if(shortflag == 0) shortflag = -1;
					break;
				case 'd' :
					entries[DEAD_PROCESS] = TRUE;
					break;
				case 'A' :
					entries[ACCOUNTING] = TRUE;
					break;
				case 'a' :
					for(i = 1; i < FLAGS; i++) entries[i] = TRUE ;
					if(shortflag == 0) shortflag = -1;
					allflag = TRUE;
					break;
				case 's' :
					shortflag = 1;
					break;
				case 'T' :
					ttystatus = TRUE;
					break;
				case '?' :
				case 'h' :
					usage("");
					break;
				default :
					usage("bad switch");
				}
			}		/* End of "while (*++*argv)" */

/* Advance to next argument and reduce count of arguments left. */

			++argv;
			--argc;
			continue;
		} else if(access(*argv,0) != FAILURE) {
/* Is there an argument left?  If so, assume that it is a utmp */
/* like file.  If there isn't one, use the default file. */

			utmpname(*argv);
			++argv;
			--argc;
			continue;
		} else {
			usage("%s doesn't exist or isn't readable",*argv);
		}
	}

/* Make sure at least one flag is set. */

	for(i=0; entries[i] == FALSE && i <= FLAGS;i++) ;
	if(i > FLAGS) entries[USER_PROCESS] = TRUE;

/* Now scan through the entries in the utmp type file and list */
/* those matching the requested types. */

	while((utmp = getutent()) != NULL) {
/* Are we looking for this type of entry? */

		if(allflag || ((utmp->ut_type >= 0 && utmp->ut_type <= FLAGS) &&
		   entries[utmp->ut_type] == TRUE) ) {
			if(utmp->ut_type == EMPTY) {
				fprintf(stdout, "Empty slot.\n");
				continue;
			}
			if(ptr != NULL && strncmp(utmp->ut_line,ptr,sizeof(utmp->ut_line)) != 0)
				continue;
			fprintf(stdout, "%8.8s %s %12.12s %s",
				user(utmp), mode(utmp), line(utmp), etime(utmp));
			if(utmp->ut_type == RUN_LVL) {
				fprintf(stdout, "    %c    %d    %c", utmp->ut_exit.e_termination,
					utmp->ut_pid, utmp->ut_exit.e_exit);
			}
			if(shortflag < 0 &&
			   (utmp->ut_type == LOGIN_PROCESS
			    || utmp->ut_type == USER_PROCESS
			    || utmp->ut_type == INIT_PROCESS
			    || utmp->ut_type == DEAD_PROCESS)) {
				fprintf(stdout, "  %5d", utmp->ut_pid);
				if(utmp->ut_type == INIT_PROCESS
				   || utmp->ut_type == DEAD_PROCESS) {
					fprintf(stdout, "  id=%4.4s", id(utmp));
					if(utmp->ut_type == DEAD_PROCESS) {
						fprintf(stdout, " term=%-3d exit=%-3d",
							utmp->ut_exit.e_termination, utmp->ut_exit.e_exit);
					}
				} else if(utmp->ut_type == LOGIN_PROCESS
				    || utmp->ut_type == USER_PROCESS) {
					 fprintf(stdout, "  %s", loc(utmp));
				}
			}
		 	fprintf(stdout,"\n");
		}

	}		/* End of "while ((utmp = getutent()" */
	fclose(stdout);
	exit(0);
}

usage(format,arg1)
char *format;
int arg1;
{
	fprintf(stderr,format,arg1);
	fprintf(stderr,"\nUsage:\twho [-rbtpludAasT] [am i] [utmp_like_file]\n");
	fprintf(stderr,"\nr\trun level\nb\tboot time\nt\ttime changes\n");
	fprintf(stderr,"p\tprocesses other than getty or users\n");
	fprintf(stderr,"l\tlogin processes\nu\tuseful information\n");
	fprintf(stderr,"d\tdead processes\nA\taccounting information\n");
	fprintf(stderr,"a\tall (rbtpludA options)\n");
	fprintf(stderr,"s\tshort form of who (no time since last output\
 or pid)\n");
	fprintf(stderr,"T\tstatus of tty (+ writable, - not writable,\
 x exclusive open, ? hung)\n");
	exit(1);
}

char *user(utmp)
struct utmp *utmp;
{
	static char uuser[sizeof(utmp->ut_user)+1];

	copypad(&uuser[0],&utmp->ut_user[0],sizeof(uuser));
	return(&uuser[0]);
}

char *line(utmp)
struct utmp *utmp;
{
	static char uline[sizeof(utmp->ut_line)+1];

	copypad(&uline[0],&utmp->ut_line[0],sizeof(uline));
	return(&uline[0]);
}

copypad(to,from,size)
register char *to;
char *from;
int size;
{
	register int i;
	register char *ptr;
	short printable;
	int halfway;

	size--;	/* Leave room for null */

/* Scan for something textual in the field.  If there is */
/* nothing except spaces, tabs, and nulls, then substitute */
/* '-' for the contents. */

	for(printable = FALSE,i=0,ptr = from;
	    *ptr != '\0' && i < size;i++,ptr++) {
/* Break out if a printable character found.  Note that the test */
/* for "> ' '" eliminates all control characters and spaces from */
/* consideration as printing characters. */

		if(*ptr > ' ') {
			printable = TRUE;
			break;
		}
	}
	i = 0;
	halfway = 0;
	if(printable) {
		for(; *from != '\0' && i < size;i++) *to++ = *from++ ;
	} else {
		halfway = (size+1)/2 - 1;	/* Where to put "-" */
	}

/* Add pad at end of string consisting of spaces and a '\0'. */
/* Put an asterisk at the halfway position.  (This only happens */
/* when padding out a null field.) */

	for(; i < size;i++) {
		*to++ = (i == halfway ? '.' : ' ');
	}
	*to = '\0';	/* Add null at end. */
}

char *id(utmp)
register struct utmp *utmp;
{
	static char uid[sizeof(utmp->ut_id)+1];
	register char *ptr;
	register int i;

	for(ptr= &uid[0],i=0; i < sizeof(utmp->ut_id);i++) {
		if(isprint(utmp->ut_id[i]) || utmp->ut_id[i] == '\0') {
			*ptr++ = utmp->ut_id[i];
		} else {
			*ptr++ = '^';
			*ptr = (utmp->ut_id[i] & 0x17) | 0100;
		}
	}
	*ptr = '\0';
	return(&uid[0]);
}

char *etime(utmp)
struct utmp *utmp;
{
	extern char *ctime();
	register char *ptr;
	long lastactivity;
	char device[20];
	static char eetime[24];
	extern long time();
	struct stat statbuf;
	extern int shortflag;

	ptr = ctime(&utmp->ut_time);

/* Erase the seconds, year and \n at the end of the string. */

	*(ptr + 16) = '\0';

/* Advance past the day of the week. */

	ptr += 4;

	strcpy(&eetime[0],ptr);
	if(shortflag < 0
	   && (utmp->ut_type == INIT_PROCESS
	   || utmp->ut_type == LOGIN_PROCESS
	   || utmp->ut_type == USER_PROCESS
	   || utmp->ut_type == DEAD_PROCESS)) {
		sprintf(&device[0],"/dev/%s",&utmp->ut_line[0]);

/* If the device can't be accessed, add a hyphen at the end. */

		if(stat(&device[0],&statbuf) == FAILURE) {
			strcat(&eetime[0],"   .  ");
		} else {
/* Compute the amount of time since the last character was sent to */
/* the device.  If it is older than a day, just exclaim, otherwise */
/* if it is less than a minute, put in an '-', otherwise put in */
/* the hours and the minutes. */

			lastactivity = time(NULL) - statbuf.st_mtime;
			if(lastactivity > 24L*3600L) {
				strcat(&eetime[0],"  old ");
			} else if(lastactivity < 60L) {
				strcat(&eetime[0],"   .  ");
			} else {
				sprintf(&eetime[12]," %2u:%2.2u",
					(unsigned)(lastactivity/3600),
					(unsigned)((lastactivity/60)%60));
			}
		}
	}
	return(&eetime[0]);
}

static int badsys;	/* Set by alarmclk() if open or close times out. */

char *mode(utmp)
struct utmp *utmp;
{
	char device[20];
	int fd;
	struct stat statbuf;
	register char *answer;
	extern char username[];
	extern alarmclk();
	extern ttystatus;

	if(ttystatus == FALSE
	   || utmp->ut_type == RUN_LVL
	   || utmp->ut_type == BOOT_TIME
	   || utmp->ut_type == OLD_TIME
	   || utmp->ut_type == NEW_TIME
	   || utmp->ut_type == ACCOUNTING
	   || utmp->ut_type == DEAD_PROCESS) return(" ");

	sprintf(&device[0],"/dev/%s",&utmp->ut_line[0]);

/* Check "access" for writing to the line. */
/* To avoid getting hung, set up any alarm around the open.  If */
/* the alarm goes off, print a question mark. */

	badsys = FALSE;
	signal(SIGALRM,alarmclk);
	alarm(3);
#ifdef	CBUNIX
	fd = open(&device[0],O_WRONLY);
#else
	fd = open(&device[0],O_WRONLY|O_NDELAY);
#endif
	alarm(0);
	if(badsys) return("?");

	if(fd == FAILURE) {
/* If our effective id is "root", then send back "x", since it */
/* must be exclusive use. */

		if(geteuid() == 0) return("x");
		else return("-");
	}

/* If we are effectively root or this is a login we own then we */
/* will have been able to open this line except when the exclusive */
/* use bit is set.  In these cases, we want to report the state as */
/* other people will experience it. */

	if(geteuid() == 0 || strncmp(&username[0],&utmp->ut_user[0],
	   SIZEUSER) == 0) {
		if(fstat(fd,&statbuf) == FAILURE) answer = "-";
		if(statbuf.st_mode&2) answer = "+";
		else answer = "-";
	} else answer = "+";

/* To avoid getting hung set up any alarm around the close.  If */
/* the alarm goes off, print a question mark. */

	badsys = FALSE;
	signal(SIGALRM,alarmclk);
	alarm(3);
	close(fd);
	alarm(0);
	if(badsys) answer = "?";
	return(answer);
}

alarmclk()
{
/* Set flag saying that "close" timed out. */

	badsys = TRUE;
}

struct ttyline
{
	char *t_id;		/* Id of the inittab entry */
	char *t_comment;	/* Comment field if one found */
};

static FILE *fpit;
static long start,current;

char *loc(utmp)
struct utmp *utmp;
{
	struct ttyline *parseitab();
	register struct ttyline *pitab;
	long start,current;
	register int wrap;
	extern long ftell();

/* Open /etc/inittab if it is not already open. */

	if(fpit == NULL) {
		if((fpit = fopen("/etc/inittab","r")) == NULL) return("");
	}

/* Save the start position in /etc/inittab. */

	start = ftell(fpit);

/* Look through /etc/inittab for the entry relating to */
/* the line of interest.  To save time, do not start at beginning */
/* of /etc/inittab, but at the current point.  Care must be taken */
/* to stop after the whole file is scanned. */

	for(current=start,wrap=FALSE; wrap == FALSE || current < start;
	    current = ftell(fpit)) {
/* If a line of interest was found, see if it was the line we */
/* were looking for. */

		if((pitab = parseitab()) != (struct ttyline *)NULL) {
			if(strncmp(pitab->t_id,utmp->ut_id,sizeof(utmp->ut_id)) == 0)
				return(pitab->t_comment);
		} else {

/* If a line wasn't found, we've hit the end of /etc/inittab.  Set */
/* the wrap around flag.  This will start checking to prevent us */
/* from cycling through /etc/inittab looking for something that is */
/* not there. */

			wrap = TRUE;
		}
	}
	return("");
}

struct ttyline *parseitab()
{
	static struct ttyline answer;
	static char itline[512];
	register char *ptr,*optr;
	register int lastc;
	int i,cflag;
	extern char *strrchr();

	for(;;) {
/* Read in a full line, including continuations if there are any. */

		ptr = &itline[0];
		do {
			if(ptr == &itline[0]
			   && fgets(ptr, &itline[sizeof(itline)]-ptr,fpit) == (char *)NULL) {
				fseek(fpit,0L,0);
				return((struct ttyline *)NULL);
			}

/* Search for the <nl> at the end of the line, keeping track of */
/* quoting so that we will know if the <nl> is quoted. */

			for(lastc='\0'; *ptr && *ptr != '\n';
			    lastc = (lastc == '\\'? '\0':*ptr) , ptr++);

/* If there is no <nl> at the end of the line or the <nl> is not */
/* quoted, then set "cflag" to FALSE so that the "do" will */
/* terminate. */

			if(*ptr == '\0' || lastc != '\\') {
				cflag = FALSE;
				if(*ptr) *ptr = '\0';	/* Cover the <nl> */
				else {
					cflag = TRUE;

/* Cover up the quoted <nl> by backing up to the backslash. */

					--ptr;
				}
			}
		} while(cflag);

/* Save pointer to the id field of the inittab entry. */

		answer.t_id = &itline[0];
		answer.t_comment = "";

/* Skip over the first three fields (delimited by ':'). */

		for(i=3,ptr= &itline[0],cflag=TRUE; i;ptr++) {
/* If the line is badly formatted, ignore it. */

			if(*ptr == '\0') {
				cflag = FALSE;
				break;
			}

/* Turn colons into nulls so that fields are seperate strings. */

			if(*ptr == ':') {
				*ptr = '\0';
				i--;
			}
		}

		if(cflag == FALSE) continue;

/* Search for a comment field.  This may be one of the form */
/* ";: comment <null>" or one of the form "# comment <null>" */
/* Note that while ";: comment; command" is legal shell syntax, */
/* it is not expected on a getty line in inittab and is actually */
/* improperly formed since only the first command in inittab */
/* will be executed since there is an implicit "exec" to the */
/* shell command line. */

		for(lastc='\0';
		    *ptr && *ptr != ';' && lastc != '\\' && *ptr != '#';
		    lastc = (lastc == '\\'?'\0':*ptr), ptr++);

/* If there is no comment, return immediately. */

		if(*ptr == '\0') return(&answer);

/* If there is an unquoted semicolon look to see if the argument */
/* following it is colon and a space or a tab. */

		else if(*ptr == ';') {
			while(*++ptr == ' ' || *ptr == '\t');
			if(*ptr != ':' || (*++ptr != ' ' && *ptr != '\t'))
				return(&answer);
		}

/* After the "#" or ":; " sequence, skip over white space and */
/* return the remainder as the comment. */

		while(*++ptr == ' ' || *ptr == '\t');
		if(*ptr == '\0') return(&answer);
		answer.t_comment = ptr;
		return (&answer);
	}
}

/* "trash" prints possibly garbage strings so that non-printing */
/* characters appear as visible characters. */

char *trash(ptrin,size)
register char *ptrin;
register int size;
{
	static char answer[128];
	register char *ptrout;

	ptrout = &answer[0];
	while(--size >= 0) {
/* If the character to be printed is negative, print it as <-x>. */

		if(*ptrin & 0x80) {
			*ptrout++ = '<';
			*ptrout++ = '-';
		}
		if(isprint(*ptrin&0x7f)) *ptrout++ = (*ptrin&0x7f);

/* If the low seven bits of the character are not printable, */
/* print as ^x, where 'x' is the low seven bits plus 0x40. */

		else {
			*ptrout++ = '^';
			*ptrout++ = (*ptrin&0x7f) + 0x40;
		}

/* Finish up the corner brackets if the character was negative. */

		if(*ptrin & 0x80) *ptrout++ = '>';
		ptrin++;
	}
	return(&answer[0]);
}
