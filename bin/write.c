char _Origin_[] = "System V";

/*	@(#)write.c	1.4	*/

/*	Program to communicate with other users of the system.		*/
/*	Usage:	write user [line]					*/

#include	<stdio.h>
#include	<signal.h>
#include	<sys/types.h>
#include	<utmp.h>
#include	<pwd.h>

#define		TRUE	1
#define		FALSE	0
#define		FAILURE	(-1)

FILE	*fp;			/* File pointer for receipient's terminal */
char *rterm,*receipient;	/* Pointer to receipient's terminal and name */

main(argc,argv)
int argc;
char **argv;
{
	register int i;
	register struct utmp *ubuf;
	static struct utmp self;
	char ownname[sizeof(self.ut_user) + 1];
	static char rterminal[] = "/dev/\0 2345678901";
	extern char *rterm,*receipient;
	char *terminal,*ownterminal,*ttyname();
	short count;
	extern FILE *fp;
	extern int openfail(),eof();
	char input[134];
	register char *ptr;
	long tod;
	char *time_of_day;
	struct passwd *passptr;
	extern struct utmp *getutent();
	extern struct passwd *getpwuid();
	extern char *ctime();

/*	Set "rterm" to location where receipient's terminal will go.	*/

	rterm = &rterminal[sizeof("/dev/") - 1];
	terminal = NULL;

	if (--argc <= 0) {
	    fprintf(stderr,"Usage: write user [terminal]\n");
	    exit(1);
	} else {
	    receipient = *++argv;
	}

/*	Was a terminal name supplied?  If so, save it.			*/

	if (--argc > 1) {
	    fprintf(stderr,"Usage: write user [terminal]\n");
	    exit(1);
	} else
		terminal = *++argv;

/*	One of the standard file descriptors must be attached to a	*/
/*	terminal in "/dev".						*/

	if ((ownterminal = ttyname(fileno(stdin))) == NULL &&
	    (ownterminal = ttyname(fileno(stdout))) == NULL &&
	    (ownterminal = ttyname(fileno(stderr))) == NULL)
	  {
	    fprintf(stderr,"I cannot determine your terminal name.\
  No reply possible.\n");
	    ownterminal = "/dev/???";
	  }

/*	Set "ownterminal" past the "/dev/" at the beginning of		*/
/*	the device name.						*/

	ownterminal += sizeof("/dev/")-1;

/*	Scan through the "utmp" file for your own entry and the		*/
/*	entry for the person we want to send to.			*/

	for (self.ut_pid=0,count=0;(ubuf = getutent()) != NULL;)
	  {
/*	Is this a USER_PROCESS entry?					*/

	    if (ubuf->ut_type == USER_PROCESS)
	      {
/*	Is it our entry?  (ie.  The line matches ours?			*/

		if (strncmp(&ubuf->ut_line[0],ownterminal,
		    sizeof(ubuf->ut_line)) == 0) self = *ubuf;

/*	Is this the person we want to send to?				*/

		if (strncmp(receipient,&ubuf->ut_user[0],
		    sizeof(ubuf->ut_user)) == 0)
		  {
/*	If a terminal name was supplied, is this login at the correct	*/
/*	terminal?  If not, ignore.  If it is right place, copy over the	*/
/*	name.								*/

		    if (terminal != NULL)
		      {
			if (strncmp(terminal,&ubuf->ut_line[0],
			    sizeof(ubuf->ut_line)) == 0)
			  {
			    strncpy(rterm,&ubuf->ut_line[0],
				sizeof(ubuf->ut_line)+1);
			  }
		      }

/*	If no terminal was supplied, then take this terminal if no	*/
/*	other terminal has been encountered already.			*/

		    else
		      {
/*	If this is the first encounter, copy the string into		*/
/*	"rterminal".							*/

			if (*rterm == '\0') strncpy(rterm,
			    &ubuf->ut_line[0],sizeof(ubuf->ut_line)+1);

/*	If this is the second terminal, print out the first.  In all	*/
/*	cases of multiple terminals, list out all the other terminals	*/
/*	so the user can restart knowing what her/his choices are.	*/

			else if (terminal == NULL)
			  {
			    if (count == 1)
			      {
				fprintf(stderr, "%s is logged on more than one\
 place.\nYou are connected to \"%s\".\nOther locations are:\n",
				    receipient,rterm);
			      }
			    fwrite(&ubuf->ut_line[0],sizeof(ubuf->ut_line),
				1,stderr);
			    fprintf(stderr,"\n");
			  }

			count++;
		      }			/* End of "else" */
		  }			/* End of "else if (strncmp" */
	      }			/* End of "if (USER_PROCESS" */
	  }		/* End of "for(count=0" */

/*	Did we find a place to talk to?  If we were looking for a	*/
/*	specific spot and didn't find it, complain and quit.		*/

	if (terminal != NULL && *rterm == '\0')
	  {
	    fprintf(stderr,"%s is not at \"%s\".\n",receipient,terminal);
	    exit(1);
	  }

/*	If we were just looking for anyplace to talk and didn't find	*/
/*	one, complain and quit.						*/

	else if (*rterm == '\0')
	  {
	    fprintf(stderr,"%s is not logged on.\n",receipient);
	    exit(1);
	  }

/*	Did we find our own entry?					*/

	else if (self.ut_pid == 0)
	  {
/*	Use the user id instead of utmp name if the entry in the	*/
/*	utmp file couldn't be found.					*/

	    if ((passptr = getpwuid(getuid())) == (struct passwd *)NULL)
	      {
		fprintf(stderr,"Cannot determine who you are.\n");
		exit(1);
	      }
	    strncpy(&ownname[0],&passptr->pw_name[0],sizeof(ownname));
	  }
	else
	  {
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

	if (fp == NULL)
	  {
	    fprintf(stderr,"Permission denied.\n");
	    exit(1);
	  }

/*	Catch signals SIGHUP, SIGINT, SIGQUIT, and SIGTERM, and send	*/
/*	<EOT> message to receipient before dying away.			*/

	setsignals(eof);

/*	Get the time of day, convert it to a string and throw away the	*/
/*	year information at the end of the string.			*/

	time(&tod);
	time_of_day = ctime(&tod);
	*(time_of_day + 19) = '\0';
	fprintf(fp,"\n\007\007\007\tMessage from %s (%s) [ %s ] ...\n",
	    &ownname[0],ownterminal,time_of_day);
	fflush(fp);
	fprintf(stderr,"\007\007");	

/*	Get input from user and send to receipient unless it begins	*/
/*	with a !, when it is to be a shell command.			*/

	while ((ptr = fgets(&input[0],sizeof(input),stdin)) != NULL)
	  {
/*	Is this a shell command?					*/

	    if (*ptr == '!')
	      {
		shellcmd(++ptr);
	      }

/*	Send line to the receipient.					*/

	    else
	      {
		fputs(ptr,fp);
		fflush(fp);
	      }
	  }

/*	Since "end of file" received, send <EOT> message to receipient.	*/

	eof();
  }

setsignals(catch)

int (*catch)();

  {
	signal(SIGHUP,catch);
	signal(SIGINT,catch);
	signal(SIGQUIT,catch);
	signal(SIGTERM,catch);
  }

shellcmd(command)

char *command;

  {
	register int child;
	extern int eof();

	if ((child = fork()) == FAILURE)
	  {
	    fprintf(stderr,"Unable to fork.  Try again later.\n");
	    return;
	  }
	else if (child == 0)
	  {
/*	Reset the signals to the default actions and exec a shell.	*/

	    execl("/bin/sh","sh","-c",command,0);
	    exit(0);
	  }
	else
	  {
/*	Allow user to type <del> and <quit> without dying during	*/
/*	commands.							*/

	    signal(SIGINT,SIG_IGN);
	    signal(SIGQUIT,SIG_IGN);

/*	As parent wait around for user to finish spunoff command.	*/

	    while(wait(NULL) != child);

/*	Reset the signals to their normal state.			*/

	    setsignals(eof);
	  }
	fprintf(stdout,"!\n");
  }

openfail()
  {
	extern char *rterm,*receipient;

	fprintf(stderr,"Timeout trying to open %s's line(%s).\n",
	    receipient,rterm);
	exit(1);
  }

eof()
{
	extern FILE *fp;

	fprintf(fp,"<EOT>\n");
	exit(0);
}
