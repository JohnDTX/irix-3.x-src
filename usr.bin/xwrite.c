#ifndef	lint
/* @(#) from write.c	1.4	*/
/* $Source: /d2/3.7/src/usr.bin/RCS/xwrite.c,v $ */
static	char	Sccsid[] = "@(#)$Revision: 1.1 $";
/* $Date: 89/03/27 17:41:03 $ */
#endif


/*	Program to communicate with other users of the system.		*/
/*	Usage:	xwrite [system:]user [line]				*/

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <utmp.h>
#include <pwd.h>
#include <termio.h>
#include <string.h>

#define	TRUE	1
#define	FALSE	0
#define	FAILURE	(-1)
#define	reg	register

FILE	*fp;			/* File pointer for receipient's terminal */
char	*rterm, *receipient;	/* Pointer to receipient's terminal and name */
				/* too long to indent string correctly */
char	xwritefmt[] = "/usr/bin/xwrite%s -R %s %s %s %s%s%s";
int	raw;			/* raw mode */
struct	termio	term;
struct	termio	termsav;

main(argc,argv)
int	argc;
char	**argv;
{
	reg	int	i;
	reg	struct	utmp *ubuf;
	static	struct	utmp self;
	char	ownname[sizeof(self.ut_user) + 1];
	static	char	rterminal[] = "/dev/\0 2345678901";
	static	char	*system;
	static	char	*remote;
	static	char	remmsg[80];
	static	char	hostnm[30] = "Dummy didn't set hostname!";
	static	char	cmd[200];
	static	int	bol = 1;		/* start of line */
	extern	char	*rterm,*receipient;
	char	*terminal,*ownterminal,*ttyname();
	short	count;
	extern	FILE	*fp;
	extern	int	openfail(),eof();
	char	input[135];
	reg	char	*ptr;
	long	tod;
	char	*time_of_day;
	char	*p2;
	struct	passwd *passptr;
	extern	struct	utmp *getutent();
	extern	struct	passwd *getpwuid();
	extern	char	*ctime();

/*	Set "rterm" to location where receipient's terminal will go.	*/

	rterm = &rterminal[sizeof("/dev/") - 1];
	terminal = NULL;

	if (--argc <= 0) {
	    fprintf(stderr,"Usage: xwrite [system:]user [terminal]\n");
	    exit(1);
	}
	if (strcmp(argv[1],"-") == 0) {
		argv++;			/* skip "-" */
		argc--;
		if (isatty(0) || (argc > 0 && strcmp(argv[1],"-R") == 0))
			raw++;
	}
	self.ut_pid = 0;		/* Don't know thyself yet */
	if (argc >= 5 && strcmp(argv[1],"-R") == 0) {
		argv++;			/* skip "-R" */
		argc--;
		strcpy(self.ut_user,*++argv);
		self.ut_pid = 1;	/* non-zero to flag known */
		argc--;
		ownterminal = *++argv;
		argc--;
		remote = *++argv;
		sprintf(remmsg," on %s",remote);
		argc--;
	}
	receipient = *++argv;
	p2 = strchr(receipient,':');
	if (p2) {
		system = receipient;
		*p2++ = '\0';
		receipient= p2;
	}

/*	Was a terminal name supplied?  If so, save it.			*/

	if (--argc > 1) {
	    fprintf(stderr,"Usage: xwrite [system:]user [terminal]\n");
	    exit(1);
	}
	terminal = *++argv;

/*	One of the standard file descriptors must be attached to a	*/
/*	terminal in "/dev".						*/

	if (!ownterminal) {
		if ((ownterminal = ttyname(fileno(stdin))) == NULL &&
		  (ownterminal = ttyname(fileno(stdout))) == NULL &&
		  (ownterminal = ttyname(fileno(stderr))) == NULL) {
			fprintf(stderr,"I cannot determine your terminal name.\
  No reply possible.\n");
			ownterminal = "/dev/???";
		}
/*	Set "ownterminal" past the "/dev/" at the beginning of		*/
/*	the device name.						*/
		ownterminal += sizeof("/dev/")-1;
	}
	if (raw) {
		ioctl(0,TCGETA,&term);
		if (ioctl(0,TCGETA,&termsav) >= 0) {
			term.c_cc[VMIN] = 1;
			term.c_cc[VTIME] = 1;
			term.c_lflag &= ~ICANON;
			ioctl(0,TCSETAW,&term);
		} else
			raw = 0;
	}

/*	Scan through the "utmp" file for your own entry and the		*/
/*	entry for the person we want to send to.			*/

	for (count=0;(ubuf = getutent()) != NULL;) {
/*	Is this a USER_PROCESS entry?					*/

		if (ubuf->ut_type == USER_PROCESS) {
/*	Is it our entry?  (i.e.  The line matches ours?			*/

			if (strncmp(&ubuf->ut_line[0],ownterminal,
			  sizeof(ubuf->ut_line)) == 0 && !remote) {
				self = *ubuf;
				if (system)
					switch (fork()) {
					  case -1:
						fprintf(stderr,"Can't Fork\n");
						Exit(-1);
					  case 0:
						gethostname(hostnm,
						  sizeof(hostnm)-1);
						sprintf(cmd,
						 xwritefmt,
						  raw?" -":"",
						  self.ut_user,
						  ownterminal,
						  hostnm,
						  receipient,
						  terminal?" ":"",
						  terminal?terminal:"",
						  0);
						execl("/bin/xx",
						  "REXEC",system,cmd);
						execl("/usr/bin/xx",
						  "REXEC",system,cmd);
						execl("/usr/ucb/xx",
						  "REXEC",system,cmd);
						execl("/etc/xx",
						  "REXEC",system,cmd);
						execl("/usr/local/bin/xx",
						  "REXEC",system,cmd);
						fprintf(stderr,
						  "Can't Execl xx\n");
						Exit(-1);
					  default:
						wait((int *)0);
						Exit(0);
					}
			}

/*	Is this the person we want to send to?				*/

			if (strncmp(receipient,&ubuf->ut_user[0],
			  sizeof(ubuf->ut_user)) == 0 && !system) {
/*	If a terminal name was supplied, is this login at the correct	*/
/*	terminal?  If not, ignore.  If it is right place, copy over the	*/
/*	name.								*/
				if (terminal != NULL) {
					if (strncmp(terminal,&ubuf->ut_line[0],
					  sizeof(ubuf->ut_line)) == 0) {
						strncpy(rterm,&ubuf->ut_line[0],
						  sizeof(ubuf->ut_line)+1);
					}
				} else {
/*	If no terminal was supplied, then take this terminal if no	*/
/*	other terminal has been encountered already.			*/
/*	If this is the first encounter, copy the string into		*/
/*	"rterminal".							*/
					if (*rterm == '\0')
						strncpy(rterm,
						  &ubuf->ut_line[0],
						  sizeof(ubuf->ut_line)+1);
/*	If this is the second terminal, print out the first.  In all	*/
/*	cases of multiple terminals, list out all the other terminals	*/
/*	so the user can restart knowing what her/his choices are.	*/
					else if (terminal == NULL) {
						if (count == 1) {
							fprintf(stderr, "%s is logged on more than one\
 place.\nYou are connected to \"%s\".\nOther locations are:\n",
				    receipient,rterm);
						}
						fwrite(&ubuf->ut_line[0],
						  sizeof(ubuf->ut_line),
						  1,stderr);
						fprintf(stderr,"\n");
					}
					count++;
				}
			}
		}
	}

/*	Did we find a place to talk to?  If we were looking for a	*/
/*	specific spot and didn't find it, complain and quit.		*/

	if (terminal != NULL && *rterm == '\0') {
	    fprintf(stderr,"%s is not at \"%s\".\n",receipient,terminal);
	    Exit(1);
	} else if (*rterm == '\0') {
/*	If we were just looking for anyplace to talk and didn't find	*/
/*	one, complain and quit.						*/

		fprintf(stderr,"%s is not logged on.\n",receipient);
		Exit(1);
	} else if (self.ut_pid == 0) {

/*	Did we find our own entry?					*/
/*	Use the user id instead of utmp name if the entry in the	*/
/*	utmp file couldn't be found.					*/
		if ((passptr = getpwuid(getuid())) == (struct passwd *)NULL) {
			fprintf(stderr,"Cannot determine who you are.\n");
			Exit(1);
		}
		strncpy(&ownname[0],&passptr->pw_name[0],sizeof(ownname));
	} else {
		strncpy(&ownname[0],self.ut_user,sizeof(self.ut_user));
	}
	ownname[sizeof(ownname)-1] = '\0';

/*	Try to open up the line to the receipient's terminal.		*/

	signal(SIGALRM,openfail);
	alarm(5);
	fp = fopen(&rterminal[0],"w");
	alarm(0);

/*	If open failed, then permissions must be preventing us from	*/
/*	sending to this person.						*/

	if (fp == NULL) {
		fprintf(stderr,"Permission denied.\n");
		Exit(1);
	}

/*	Catch signals SIGHUP, SIGINT, SIGQUIT, and SIGTERM, and send	*/
/*	<EOT> message to receipient before dying away.			*/

	setsignals(eof);

/*	Get the time of day, convert it to a string and throw away the	*/
/*	year information at the end of the string.			*/

	time(&tod);
	time_of_day = ctime(&tod);
	*(time_of_day + 19) = '\0';
	fprintf(fp,"\n\007\007\007\tMessage from %s (%s)%s [ %s ] ...\n",
	  &ownname[0],ownterminal,remmsg,time_of_day);
	fflush(fp);
	fprintf(stderr,"\007\007");	

/*	Get input from user and send to receipient unless it begins	*/
/*	with a !, when it is to be a shell command.			*/

	while ((i=read(0,input,sizeof input-1)) > 0 && !(i==1 && *input == 4)) {
/*	Is this a shell command?					*/
		if (*input == '!' && bol) {
						/* handle raw mode	*/
			input[i] = '\0';	/* null terminate	*/
			while (!strchr(input,'\n') && !strchr(input,'\r')
			  && strlen(input) < sizeof input - 2) {
						/* needed for strchr()	*/
				input[strlen(input)+1] = '\0';
				if (read(0,&input[strlen(input)],1) != 1) {
					input[strlen(input)] = '\0';
					break;
				}
			}
			printf("Remotely doing %s",input+1);
			if (raw)
				ioctl(0,TCSETAW,&termsav);
			shellcmd(input+1);
			if (raw)
				ioctl(0,TCSETAW,&term);
			bol = 1;
		} else {
/*	Send line to the receipient.					*/
			if (input[i-1] == '\n') {
				input[i-1] = '\r';
				input[i] = '\n';
				i++;
			}
			write(fileno(fp),input,i);
			if (raw)
				bol = (*input == '\n');
			else
				bol = 1;
					
		}
	}
/*	Since "end of file" received, send <EOT> message to receipient.	*/
	eof();
}

setsignals(catch)
int	(*catch)();
{
	signal(SIGHUP,catch);
	signal(SIGINT,catch);
	signal(SIGQUIT,catch);
	signal(SIGTERM,catch);
}

shellcmd(command)
char	*command;
{
	reg	int	child;
	extern	int	eof();

	if ((child = fork()) == FAILURE) {
		fprintf(stderr,"Unable to fork.  Try again later.\n");
		return;
	} else if (child == 0) {
/*	Reset the signals to the default actions and exec a shell.	*/
		execl("/bin/sh","sh","-c",command,0);
		Exit(0);
	} else {
/*	Allow user to type <del> and <quit> without dying during	*/
/*	commands.							*/
		signal(SIGINT,SIG_IGN);
		signal(SIGQUIT,SIG_IGN);
/*	As parent wait around for user to finish spunoff command.	*/
		while(wait(NULL) != child)
			;
/*	Reset the signals to their normal state.			*/
		setsignals(eof);
	}
	fprintf(stdout,"!\n");
}

openfail()
{
	extern	char	*rterm,*receipient;

	fprintf(stderr,"Timeout trying to open %s's line(%s).\n",
	  receipient,rterm);
	Exit(1);
}

eof()
{
	extern	FILE	*fp;

	fprintf(fp,"<EOT>\n");
	Exit(0);
}

Exit(status)
{
	if (raw)
		ioctl(0,TCSETAW,&termsav);
	exit(status);
}
