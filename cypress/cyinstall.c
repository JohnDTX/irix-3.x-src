
/* 
 * cy_install.c - Add routing table entries to the cypress network.
 * 
 * Author:	Gregory H. Smith
 * 		Dept. of Computer Sciences
 * 		Purdue University
 * 
 * Date:	Wed Oct 23 1985
 */


static char rcs_ident[] = "$Header: /d2/3.7/src/cypress/RCS/cyinstall.c,v 1.1 89/03/27 15:03:32 root Exp $";
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <net/if.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>
#include <arpa/inet.h>
#ifdef sgi
#include "if_cy.h"
#else
#include <netinet/if_cy.h>
#endif
#include "defs.h"
extern int errno;
char *sbargv0;

main(argc, argv)

    char **argv;
    int argc;

{
   struct cy_ioctlarg cy_ioctlarg;
   FILE *pf;                         /* file descriptor for input file */
   char *sbroute;                    /* file name for input file */
   char sbaddr[50];                  /* address from input file */
   char sbMsg[90];                   /* error message */
   char *ch;
   int s;                            /* socket number */
   int ln;                           /* line number */
   int verbose = 0;                  /* don't install if true, just   */
                                     /* display on stdout             */
                                     /* install routes, just display  */
   int gothost = 0;                  /* flags if my hostname passed as*/
                                     /* an argument                   */
   int gotserver = 0;                /* flags if the name of the route*/
                                     /* server was passed as an arg   */
                                     /* server passed as argument     */
   char pgm[50];                     /* name of pgm to obtain routes  */
   char hostname[NAMESIZE];
   char server[NAMESIZE];
   struct hostent *phostent;
   int pid;
   union wait *status;
   
   sbargv0 = argv[0];
   sbroute = ROUTEFILE;
   strcpy(pgm, PGM);
   
   /* only valid option at this point is error logging */
   /* which is selected by -l, all others invalid      */
   while (--argc > 0 && (*++argv)[0] == '-')
      for (ch = argv[0] + 1; *ch != '\0'; ch++)
        switch(*ch) {

  	   case 'h' : gothost++;
                      strcpy(hostname, *++argv);
		      argc--;
		      break;

  	   case 's' : gotserver++;
                      strcpy(server, *++argv);
		      argc--;
		      break;
	   
           case 'v' : verbose++;
		      break;

	   default  : usage();
		      exit(1);
		      break;
		   }

   if (argc > 0) {
      usage();
      exit(1);
   }

   if (!gothost)
      gethostname(hostname, sizeof(hostname));

   if (!gotserver)
      strcpy(server, SERVER);

   /* delete an existing error file */
   unlink(ERRORFILE);

   /* get a socket to use for ioctls */
   /* to install routes              */
   s = socket(AF_INET, SOCK_DGRAM, 0);
   if (s < 0) {
      Perror("Cannot install routes, unable to get a socket\n");
      exit(1);
   }
   
   /* install a route to the cypress1 machine via*/
   /* line 0                                     */
   if ((phostent = gethostbyname(server)) == NULL) {
#ifdef BSD42
      sprintf(sbMsg, "%s: routing host %s unknown\n", sbargv0, server);
#else
	    switch (h_errno) {
		case HOST_NOT_FOUND:
		          sprintf(sbMsg, "%s: routing host %s unknown\n", sbargv0, server);
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
   }
   else {
      strcpy(cy_ioctlarg.cyi_name, "cy0");
      cy_ioctlarg.cyi_sin.sin_family = phostent->h_addrtype;
      bcopy(phostent->h_addr, &cy_ioctlarg.cyi_sin.sin_addr.s_addr, phostent->h_length);
      cy_ioctlarg.cyi_ln = 0;
#ifdef sgi
      if (tty_ioctl(s, CYIOC_SROUTE, &cy_ioctlarg) < 0) {
#else
      if (ioctl(s, CYIOC_SROUTE, &cy_ioctlarg) < 0) {
#endif
	 sprintf(sbMsg, "unable to install route to %s\n", server);
	 Perror(sbMsg);
      }
      
      /* fork a child which attempts to contact the */
      /* routing server to obtain new routing info  */
      if (fork() == 0) 
	 execl(pgm, hostname, server, 0);  
      
      /* wait for the child to exit even though we */
      /* don't care what his exit status is        */
      pid = wait(status);
      
   }
   
   if ((pf = fopen(sbroute, "r")) == NULL) {
      sprintf(sbMsg, "%s: Fatal error, can't open %s\n", sbargv0, sbroute);
      Perror(sbMsg);
      exit(1);
   }
   
   /* got the file open, parse it line by line and */
   /* install the routing entry                    */
   while (fscanf(pf, "%s %d", sbaddr, &ln) != EOF) {
      if (verbose) 
	 printf("address %s, line %d\n", sbaddr, ln);
      else {
	 strcpy(cy_ioctlarg.cyi_name, "cy0");
	 cy_ioctlarg.cyi_sin.sin_family = AF_INET;
	 cy_ioctlarg.cyi_sin.sin_addr.s_addr = (u_long) inet_addr(sbaddr);
	 cy_ioctlarg.cyi_ln = ln;
#ifdef sgi
	 if (tty_ioctl(s, CYIOC_SROUTE, &cy_ioctlarg) < 0) {
#else
	 if (ioctl(s, CYIOC_SROUTE, &cy_ioctlarg) < 0) {
#endif
	    Perror("ioctl");
	    exit(1);
	 }
      }
   }

   /* close the file and exit */
   fclose(pf);
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
      switch(errno) {
       case ECYBADADDR: fprintf(perr, "%s: Invalid Cypress address.\n",
				sbargv0);
			printf("%s: Invalid Cypress Address.\n", sbargv0);
			break;
			
       case ELINEBUSY:  fprintf(perr, "%s: Line already in use.\n", 
	  		       sbargv0);
			printf("%s: Line already in use.\n");
			break;

       case ETTYBUSY:   fprintf(perr, "%s: Line discipline not set to CDISC.\n", sbargv0);
			printf("%s: Line discipline not set to CDISC.\n",
			       sbargv0);
			break;

       case EBADLINE:   fprintf(perr, "%s: Invalid line number\n", sbargv0);
			printf("%s: Invalid line number\n", sbargv0);
			break;

       default:         fprintf(perr, "%s: %s", sbargv0, sb);
			printf("%s: %s", sbargv0, sb);
			perror("");
			break;
		     }
      fclose(perr);
   }
}


usage()

{
   printf("usage: cyinstall [-v] [-h hostname] [-s server]\n");
}
