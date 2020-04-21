/*
 * cymonitord.c -- cypress monitor server.
 * 
 * author -- Cathy Privette
 * 
 * date -- 10/4/85
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <netdb.h>
#include <errno.h>

#define TRUE    -1
#define FALSE    0
#define CYPSTAT "/etc/cypress/lib/cypstat"  /* name of the cypress     */
					    /* monitor software        */
#define LOGFILE "/etc/cypress/monitor.log"  /* log file for monitor    */
                                            /* server                  */

int     f;                              /* socket file descriptor  */
FILE    *log;                           /* stream for monitor log  */
struct  sockaddr_in sin = { AF_INET } ; /* socket address          */
char    cyppath[100];                   /* path name of cypstat    */
int     flog;                           /* logging flag, log msgs  */
                                        /* if TRUE                 */
int     wait_while();                   /* procedure called when   */
                                        /* child terminates        */
extern int   errno;

main(argc, argv)
int argc;
char *argv[];

{
	int        s;                   /* network socket          */
	struct     servent *sp;         /* server entry            */
	struct     sockaddr_in from;    /* from address            */

        flog = FALSE;
        if (--argc > 0 && (*++argv)[0] == '-') {
            switch ((*argv)[1]) {
                case 'l' :
                    flog = TRUE;
                    break;
	    }
	}

	/* open the log file */
	log = fopen(LOGFILE, "a");

        /* redirect error output */
        close(fileno(stderr));
        dup2(fileno(log), fileno(stderr));
        close(fileno(log));

	strcpy(cyppath, CYPSTAT);

	sp = getservbyname("cypress-stat", "tcp");
	if (sp == (struct servent *) NULL) {
            if (flog)
	       fprintf(stderr, "cymonitord: cypress-stat/tcp  unknown service\n");
	    exit(1);
	}

	if (fork ())
	    exit(0);

        if (flog)
            fprintf(stderr, "server process id is %d\n", getpid());

	/* ignore control signals from terminal */
        s = open("/dev/tty", 2);
        if (s >= 0) {
            ioctl(s, TIOCNOTTY, 0);
            (void)close(s);
	}

	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = sp->s_port;

        /* create a socket */
	s = socket(AF_INET, SOCK_STREAM, 0, 0);
	if (s < 0) {
            if (flog)
	        perror("cymonitord: socket");
	    exit(1);
	}

        /* bind a name to the socket */
	if (bind(s, &sin, sizeof(sin)) < 0) {
            if (flog)
	        perror("cymonitord: bind");
	    exit(1);
	}

        /* when we get a signal from a child process wait a while */
        signal(SIGCHLD, wait_while);

	/* listen for connections, allow five connections */
	listen(s, 5);

	while (1) {
	    int len = sizeof(from);

	    /* accept a connection */
	    f = accept(s, &from, &len);
	    if (f < 0) {
		if (errno == EINTR)
		    continue;
                if (flog)
                    fprintf(stderr, "cymonitord: accept failed; errno = %d\n", errno);
		continue;
	    }

            doit(f, &from);
            close(f); 
	}
}

/*
 * wait_while -- wait for a child to terminate.
 */

wait_while()
{
    union wait status;
    int pid;

#ifdef sgi
    pid = wait(&status);
    if (flog)
	fprintf(stderr, "process %d exiting\n", pid);
    signal(SIGCHLD, wait_while);
#else
    while ((pid = wait3(&status, WNOHANG, 0)) > 0) {
        if (flog)
            fprintf(stderr, "process %d exiting\n", pid);
    }
#endif
}

/*
 * doit - invoked to execute the monitor software.
 */

doit(f, fromp)

int     f;
struct  sockaddr_in *fromp;

{
	char    *fargv[3];              /* arg for execv           */
	char    *envp[1];               /* needed for execve       */
	struct  hostent *who;           /* caller's info           */
	char    buff[100];              /* message buffer          */
        int     pid;                    /* process id              */

        /* who are we connected to */
	who = gethostbyaddr(&fromp->sin_addr, sizeof(fromp->sin_addr),
			    fromp->sin_family);
        if (flog) {
  	    sprintf(buff, "%s (%s, %d)\n",
		   (who == (struct hostent *) NULL) ? "Unknown" : who->h_name,
		   inet_ntoa(fromp->sin_addr), ntohs(fromp->sin_port));

	    fprintf(stderr, buff);
	}

	/* build arguments to execve program */
	fargv[0] = CYPSTAT;
        if (flog) {
            fargv[1] = "-l";
            fargv[2] = '\0';
	}
        else
	    fargv[1] = '\0';

        envp[0] = '\0';

#ifdef sgi
#define vfork fork
#endif
	if ((pid = vfork()) == 0) {

                /* child */
                if (flog)
	 	    fprintf(stderr, "process created to execve; pid = %d\n", getpid());

		/* send stdout across socket      */
                /* leave stderr going to log file */
		dup2(f, fileno(stdout));
		close(f);

                /* his stdout goes through the socket */
		execve(cyppath, fargv, envp);

		/* if you return here something is seriously wrong */
                sprintf(buff, "cymonitord: execve; errno = %d\n", errno);
		fubar(buff);
	}

	if (pid == -1) {
            if (flog)
	        fprintf(stderr, "cymonitord: vfork; errno = %d\n", errno);
	}
}

/*
 * fubar - fouled up beyond all recovery.
 */

fubar(sbError)
char *sbError;
{
        if (flog)
	    fprintf(stderr, sbError);
	fflush(stderr);
	exit(1);
}
