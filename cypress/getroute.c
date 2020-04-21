/* 
 * Author:      Greg Smith
 *              Dept. of Computer Sciences
 *              Purdue University
 * 
 * Date:        wed 9 October 1985
 *
 * getroute.c - contact cypress routing server and write routing information
 *   to the file defined by ROUTEFILE.  This is done as a separate program
 *   so it can be run whenever it is desired to update the local file of
 *   cypress routes.
 *
 *   New routing information is written to a temporary file which is renamed
 *   to the name defined by ROUTEFILE upon successful termination of the
 *   server.  This is done so old routing information will not be destroyed
 *   until the new routing information is obtained.
 *
 * modification history
 *
 * 1 August - modified so getroute gets the hostnameof the machine it is being
 *            run on and the name of the routing server (in that order) as
 *            arguments from cyinstall.
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include "defs.h"

extern int errno;

main(argc, argv)

int argc;
char **argv;

{
	struct  hostent *him;           /* host table entry          */
        struct  hostent *me;            /* host info for this host   */
	struct  servent *cypnet;        /* service file entry        */
	struct  sockaddr_in sin;        /* socket address            */
	int     fd;                     /* network file descriptor   */
	FILE *fdtemp;		        /* fd for temporary file     */
	register int i,n;               /* loop control              */
	int c;                          /* get a character           */
	int status;                     /* return status	     */
	int error = 0;		        /* flags error               */
	int GotSomething = 0;           /* did we get anything?      */
	char sbMsg[90];                 /* messages                  */
	char buff[BUFFSIZE];            /* small buffer for response */
	char hostname[NAMESIZE];        /* argument to routing pgm   */
        char server[NAMESIZE];          /* name of route server      */
        char *tempfile;			/* temporary file to which   */
					/* new routing information is*/
					/* to be written             */

	tempfile = TEMPFILE;
	strcpy(hostname, argv[0]);
	strcpy(server, argv[1]);

	if ((him = gethostbyname(server)) == (struct hostent *) NULL) {
#ifdef BSD42
      sprintf(sbMsg, "getroute: routing host %s unknown\n", server);
#else
	    switch (h_errno) {
		case HOST_NOT_FOUND:
		          sprintf(sbMsg, "getroute: routing host %s unknown\n", server);
			  break;
		case TRY_AGAIN:
		    sprintf(sbMsg, "gethostbyname: %s: Try again later\n", server);
				break;
		case NO_RECOVERY:
		    sprintf(sbMsg, "gethostbyname: %s: No recovery possible\n", server);
		    break;
		case NO_ADDRESS:
		    sprintf(sbMsg, "gethostbyname: %s:No IP address\n", server);
		    break;
		default: sprintf(sbMsg, "gethostbyname: %s:Unknown error: %d\n", server, h_errno);
			 break;
	    }
#endif	    
	   Perror(sbMsg);
           exit(1);
	}

	if ((cypnet = getservbyname("cypress", "tcp")) == (struct servent *)NULL) {
	   sprintf(sbMsg, "getroute: cypress/tcp: unknown service\n");
	   Perror(sbMsg);
	   exit(1);
	}

	if ((fd = socket(AF_INET, SOCK_STREAM, 0, 0)) < 0) {
	   sprintf(sbMsg, "getroute: socket\n");
	   Perror(sbMsg);
	   exit(1);
	}

	sin.sin_family = him->h_addrtype;
	bcopy(him->h_addr, (caddr_t)&sin.sin_addr, him->h_length);
	sin.sin_port = cypnet->s_port;

	if (connect(fd, (caddr_t)&sin, sizeof(sin), 0) < 0) {
	   sprintf(sbMsg, "getroute: Can't connect to port %d at %s\n",
		   ntohs(sin.sin_port), server);
	   Perror(sbMsg);
           fflush(stderr);
	   close(fd);
	   exit(1);
	}

	/* open the temporary file to write the routing */
        /* information to				*/
	if ((fdtemp = fopen(tempfile, "w")) == NULL) {
	   sprintf(sbMsg, "getroute: can't open %s\n", tempfile);
	   Perror(sbMsg);
	   exit(1);
	}

	/* write my name as an argument to the routing server */
	write(fd, hostname, strlen(hostname));

	/* read the response */
	while (n = read(fd, buff, BUFFSIZE)) {
	    if (n < 0) {                /* connection screwed up */
		sprintf(sbMsg, "cypnet: read from purdue-cypress1 failed");
		perror(sbMsg);
		Perror(sbMsg);
		close(fd);
		exit(1);
	    }
	    GotSomething++;
	    for (i = 0; i < n; i++) {
		 c = buff[i] & 0177;
                 if (c >= ' ' || c == '\n' || c == '\007' || c == '\t')
		     fputc(c, fdtemp);
                 if (c >= 'a' && c <= 'z')
		     error++;
	    }
            fflush(stdout);
	}

	close(fd);
	if (!GotSomething) {
	   sprintf(sbMsg, "getroute: cannot read response from server\n");
	   Perror(sbMsg);
	   exit(1);
	}

        /* close and rename the temporary file */
        fclose(fdtemp);

        /* check for errors */
	if (error) {
	   printf("getroute: routing error, check %s for status\n", ERRORFILE);

	   rename(tempfile, ERRORFILE);
	   exit(1);
	 }

        /* the ROUTEFILE may not exist, not a fatal error */
        /* if it doesn't.  Just print a diagnostic        */
        if ((status = unlink(ROUTEFILE)) < 0) {
	   sprintf(sbMsg, "getroute: %s does not exist\n", ROUTEFILE);
	   fprintf(stderr, sbMsg);
	}

        if ((status = rename(tempfile, ROUTEFILE)) < 0) {
	   sprintf(sbMsg, "getroute: rename\n");
	   Perror(sbMsg);
	   exit(1);
	}
}

/*
 * Perror - print error message from syscall.  Maybe one of ours.  Message
 * is written both to standard out and to the file defined by ERRORFILE.
 *
 */

Perror(sb)
     
  char *sb;
     
{
   FILE *perr;
   char errorfile[50];

   strcpy(errorfile, ERRORFILE);

   /* if we can't even open the errorfile just */
   /* display the message to standard out      */
   if ((perr = fopen(errorfile, "a")) == NULL) 
      printf(sb);
   else {
      fprintf(perr, sb);
      printf(sb);
      fclose(perr);
   }
   perror("");
}


