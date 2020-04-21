/* $Header: /d2/3.7/src/cypress/RCS/utils.c,v 1.1 89/03/27 15:03:44 root Exp $ */

/* 
 * utils.c - Misc. utilities shared by programs
 * 
 * Author:	Thomas Narten
 * 		Dept. of Computer Sciences
 * 		Purdue University
 * Date:	Wed Oct 15 1986
 * Copyright (c) 1986 Thomas Narten
 */


#ifndef lint
static char rcs_ident[] = "$Header: /d2/3.7/src/cypress/RCS/utils.c,v 1.1 89/03/27 15:03:44 root Exp $";
#endif

extern int errno;


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <net/if.h>
#ifdef sgi
#include <sys/stream.h>
#include "if_cy.h"
#else
#include <netinet/if_cy.h>
#endif
#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#include <ctype.h>
/*
 * ============================================================
 * Perror - print error from syscall. Maybe one of our own errors
 * ============================================================
 */
Perror(sb)
char *sb;
{
    switch (errno) {
	case ECYBADADDR : fprintf(stderr, "%s: Invalid Cypress address.\n",
			  sb);
			  break;
	case ELINEBUSY:	    fprintf(stderr, "%s: Line already in use.\n", sb);
			    break;
	case ETTYBUSY:	    fprintf(stderr, "%s: Line discipline not set to CDISC.\n",
			    sb);
			    break;
	case EBADLINE:	    fprintf(stderr, "%s: Invalid line number\n", sb);
			    break;
	case ECYBADNET:	    fprintf(stderr,"%s: Network address unknown\n", sb);
			    break;
	default:    perror(sb);
		    break;
    }
}

/*
 * ============================================================
 * mpsbsin - convert the string to a sockaddr_in structure.
 * ============================================================
 */
mpsbsin(psin, sb)
register struct sockaddr_in *psin;
register char *sb;
{
    register struct hostent *phostent;
    unsigned int w;
    
    if (isdigit(*sb)) {
	w = inet_addr(sb);
	psin->sin_addr.s_addr =  w;
	psin->sin_family = AF_INET;
	if (w == -1) 
	    return(FALSE);
	return(TRUE);
    } else if ((phostent=gethostbyname(sb)) == NULL) {
	    printf("gethostbyname: %s: ", sb);
#ifdef BSD42
	    printf("host not found\n");
#else
	    switch (h_errno) {
		case HOST_NOT_FOUND: printf("host not found\n");
				     break;
		case TRY_AGAIN: printf("Try again later\n");
				break;
		case NO_RECOVERY: printf("No recovery possible\n");
				  break;
		case NO_ADDRESS: printf("No IP address\n");
				 break;
		default: printf("Unknown error: %d\n", h_errno);
			 break;
	    }
#endif	    
	    return(FALSE);
    } 
    psin -> sin_family = phostent -> h_addrtype;
    bcopy(phostent->h_addr, (char *) &psin->sin_addr, phostent->h_length);
    return(TRUE);
}



/*
 * ========================================================================
 * Fatal - print message and die...
 * ========================================================================
 */

Fatal(fmt, a1, a2, a3)
char *fmt;
{
    fprintf(stderr, fmt, a1, a2, a3);
    perror("\nFatal");
    fflush(stderr);
    exit(1);
}
