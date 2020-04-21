/* 
 * cy_routed.c - Cypress routing server.
 * 
 * Author:      Greg Smith
 *              Dept. of Computer Sciences
 *              Purdue University
 * 
 * Date:        Tue Jul 23 1985
 *
 * modification history
 * 2-3-86 -- changed to make logging optional.  If logging is selected gives
 *           more information including time requests for routes are received
 *           and any error messages that occurred.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <netdb.h>
#include <errno.h>

#define NAMELENGTH 50
#define RDR 0                           /* reader side of pipe */
#define WTR 1                           /* writer side of pipe */
#define CYPNET "/etc/cypress/bin/cypnet"/* fully qualified name*/
					/* of the cypress route*/
					/* software	       */
#define LOGFILE "/etc/cypress/cy_routed.log"
#define ERRORFILE "/tmp/cy_routed"

int     p_fd[2];                        /* pipe file descriptors */
int     myside, hisside;                /* more of the same      */
FILE    *pf;                            /* stream for pipe       */
FILE    *log;                           /* stream for server log */
int     logging = 0;                    /* true if logging wanted*/
int     pid;                            /* child of child        */
struct  sockaddr_in sin = { AF_INET } ; /* socket address        */
char    cyppath[100];

main(argc, argv)

char *argv[];
int argc;

{
	int        f;                   /* network file descriptor */
	int        s;                   /* network socket          */
	struct     servent *sp;         /* server entry            */
	struct     sockaddr_in from;    /* from address            */
	extern     int errno;
	int        reapchild();
	char       *ch;

	/* only valid option at this point is error logging */
	/* which is selected by -l, all others invalid      */
	while (--argc > 0 && (*++argv)[0] == '-')
	   for (ch = argv[0] + 1; *ch != '\0'; ch++)
	      switch(*ch) {

	       case 'l' : logging++;
			  break;

    	       default  : printf("Invalid option %c\n", *ch);
			  break;
		       }
	if (argc > 0) 
	   printf("Invalid option %s\n", argv[0]);
	
	strcpy(cyppath, CYPNET);

	sp = getservbyname("cypress", "tcp");
	if (sp == (struct servent *) NULL) {
	   Perror("cy_routed: tcp/cypress: unknown service\n");
           exit(1);
	 }

	if (fork ())
	    exit(0);

	/* redirect error output */
	close(2);
	dup(1);

	/* ignore hangups and interrupts */
	signal (SIGHUP, SIG_IGN);
	signal (SIGINT, SIG_IGN);

	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = sp->s_port;

	s = socket(AF_INET, SOCK_STREAM, 0, 0);

	if (s < 0) {
	   Perror("cy_routed: socket\n");
	   exit(1);
	}

	if (bind(s, &sin, sizeof(sin)) < 0) {
	   Perror("cy_routed: bind\n");
	   exit(1);
	}

	/* when we get a signal from a child */
	/* process wait a while              */
	signal(SIGCHLD, reapchild);

	/* listen for connections, allow a queue */
	/* length of ten connections             */
	listen(s, 10);

	while (1) {
	    int len = sizeof(from);

	    /* accept a connection */
	    f = accept(s, &from, &len);
	    if (f < 0) {
		if (errno == EINTR)
		    continue;
		Perror("cy_routed: accept\n");
		continue;
	    }

	    if (fork() == 0) {
		signal(SIGCHLD, SIG_IGN);
		close(s);
		doit(f, &from);
	    }

	    /* close the connection */
	    close(f);
	}
}

reapchild()

{
	union wait status;

#ifdef sgi
	wait(&status);
	signal(SIGCHLD, reapchild);
#else
	while (wait3(&status, WNOHANG, 0) > 0)
		 ;
#endif
}

/*
 * doit - invoked to set up a pipe to the requestor and execute the
 * routing software with the argument that is pointed to by the file
 * descriptor passed as a parameter.  Writes error messages to the log
 * file if logging is enabled.
 * 
 */

doit(f, fromp)

int     f;
struct  sockaddr_in *fromp;

{
	int     i = 0;                  /* loop control variable   */
	int     accessible;             /* test for presence of log*/
	char    arg[NAMELENGTH];        /* site name for which     */
					/* routing info is desired */
	FILE    *fds;                   /* network stream          */
	char    *fargv[3];              /* arg for execv           */
	char    *envp[2];               /* needed for execve       */
	struct  hostent *who;           /* caller's info           */
	char    buff[100];              /* message buffer          */
	int     c;                      /* read a char             */
	int     error = 0;              /* flags if error occurred */
	FILE    *fderror;
	struct  timeval tv;
	char    *ap;
	char    *ctime();
	char    *asctime();
	struct  tm *localtime(), *tp;

	who = gethostbyaddr(&fromp->sin_addr, sizeof(fromp->sin_addr),
			    fromp->sin_family);

	sprintf(buff, "%s (%s.%d)",
		(who == (struct hostent *) NULL) ? "Unknown" : who->h_name,
		inet_ntoa(fromp->sin_addr), ntohs(fromp->sin_port));

	/* write a message to the log file if logging enabled */
	if (logging && ((log = fopen(LOGFILE, "a")) != NULL)) {
	   fprintf(log, "Request from %s on ", buff);

	   /* get the time */
	   gettimeofday(&tv, 0);
	   time(&tv.tv_sec);
	   tp = localtime(&tv.tv_sec);
	   ap = asctime(tp);
	   sprintf(buff, "%.20s\n", ap);
	   fprintf(log, buff);
	   fflush(log);
	   fclose(log);
	}

	/* get the argument */
	i = read(f, &arg[0], NAMELENGTH);
	if (i < 0) {
	    Perror("Unable to read args from requestor, exiting\n");
	    exit(1);
	}

	/* build arguments to routing program */
	arg[i] = '\0';
	fargv[0] = CYPNET;
	fargv[1] = arg;
	fargv[2] = '\0';

	fds = fdopen(f, "w");

	if (pipe(p_fd) < 0) {
	   Perror("pipe\n");
	   exit(1);
	}

	myside = p_fd[RDR];
	hisside = p_fd[WTR];

#ifdef sgi
#define vfork fork
#endif
	if ((pid = vfork()) == 0) {
		/* myside and hisside reverse roles in child */
		close(myside);
		dup2(hisside, fileno(stdout));
		close(hisside);

		close(fileno(stderr));
		dup2(fileno(stdout), fileno(stderr));
		execve(cyppath, fargv, envp);

		/* if you return here something is seriously wrong */
		Perror("execve\n");
		exit(1);
	}

	if (pid == -1) {
	   Perror("vfork\n");
	   exit(1);
	}

	close(hisside);

	pf = fdopen(myside, "r");
	if (pf == NULL) {
	   Perror("pipe file descriptor\n");
	   exit(1);
	}

	/* copy the results of the routing program both to */
	/* the pipe and to the error file                  */
	if ((fderror = fopen(ERRORFILE, "w")) == NULL)
	   Perror("cy_routed: Unable to open error file\n");

	while ((c = getc (pf)) != EOF) {
	   if (c == '\n')
	      putc('\r', fds);
	   if (c > 'a' && c < 'z') 
	      error++;
	   putc(c, fds);
	   if (fderror)
	      putc(c, fderror);
	}

	fflush(fds);
	fclose(fds);

	if (fderror) {
	   fflush(fderror);
	   fclose(fderror);
	}

	/* if an error occurred, append it to the log */
	/* if logging enabled                         */
	if (error) {
	   fcopy();
	   exit(1);
	}
	else
           if (logging && ((log = fopen(LOGFILE, "a")) != NULL)) {
	      fprintf(log, "normal termination\n");
              fflush(log);
              fclose(log);
	   }
	exit(0);
}


/*
 * Perror - display an error message on the standard output file.  If 
 * logging is turned on write it to the log also.
 *
 */

Perror(sb)

char *sb;

{
   printf(sb);
   if (logging && ((log = fopen(LOGFILE, "a")) != NULL)) {
      fprintf(log, sb);
      fflush(log);
      fclose(log);
   }
}

/*
 * fcopy - append the contents of the file whose descriptor is passed 
 * to the log file if logging is enabled.
 *
 */
      
fcopy()


{
   FILE *fderr;
   int c;
   
   if (logging && ((log = fopen(LOGFILE, "a")) != NULL)) {
      if ((fderr = fopen(ERRORFILE, "r")) == NULL) {
	 fprintf(log, "cy_routed: unable to open error file\n");
	 fprintf(log, "See file /etc/cypress/ROUTE.ERR at requesting site\n");
      }

      /* got both files open, copy */
      while ((c = getc(fderr)) != EOF) 
	 putc(c, log);

      fflush(log);
      fclose(log);
      fclose(fderr);
   }
}
      
      
