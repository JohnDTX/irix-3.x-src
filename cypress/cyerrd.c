/* $Header: /d2/3.7/src/cypress/RCS/cyerrd.c,v 1.1 89/03/27 15:03:31 root Exp $ */

/* 
 * cyerrd.c - Cypress error logging daemon
 * 
 * Author:	Thomas Narten
 * 		Dept. of Computer Sciences
 * 		Purdue University
 * Date:	Sun Feb  9 1986
 * Copyright (c) 1986 Thomas Narten
 */

static char rcs_ident[] = "$Header: /d2/3.7/src/cypress/RCS/cyerrd.c,v 1.1 89/03/27 15:03:31 root Exp $";


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <nlist.h>
#include <strings.h>
#include <netinet/in.h>
#include <net/if.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>
#ifdef sgi
#include <sys/stream.h>
#include "if_cy.h"
#else
#include <netinet/if_cy.h>
#endif
#define VMUNIX	"/vmunix"
#define KMEM	"/dev/kmem"
#define LOGFILE "/etc/cypress/cyerrd.log"
#define PIDFILE "/etc/cypress/cyerrd.pid"
extern int errno;
char *sbargv0;

int DumpLogRec();

main(argc, argv)
int argc;
char *argv[];
{
    struct cy_ioctlarg cy_ioctlarg;
    int s, fp;
    FILE *fppid;

#ifndef DEBUG
    if (fork())
	exit(0);
    if ((s = open("/dev/tty", 2)) >= 0) {
	ioctl(s, TIOCNOTTY, (char *)0);
	(void) close(s);
    }
#endif
    if ((fppid=fopen(PIDFILE, "w")) == NULL) {
	perror(PIDFILE);
	exit(1);
    }
    fprintf(fppid,"%d", getpid());
    if (fclose(fppid) == EOF) {
	perror(PIDFILE);
	exit(1);
    }
    
    if (freopen(LOGFILE, "a", stdout) < 0)
	fatal(LOGFILE);
    if ((s=socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	perror("cyioctl: socket");
	exit(1);
    }
    while (TRUE) {
	strcpy(cy_ioctlarg.cyi_name, "cy0");
#ifdef sgi
	if (tty_ioctl(s, CYIOC_GETLOGREC, &cy_ioctlarg) < 0) {
#else
	if (ioctl(s, CYIOC_GETLOGREC, &cy_ioctlarg) < 0) {
#endif
	    Perror("ioctl(CYIOC_GETLOGREC)");
	    exit(1);
	}
	DumpLogRec(&cy_ioctlarg.cyi_logrec);
    }
}
/*
 * ============================================================
 * DumpLogRec - print out the contents of the log record
 * ============================================================
 */
int DumpLogRec(plr)
register struct cy_logrec *plr;
{
    char *sb;
    
#ifdef sgi
    extern char* ctime();
    sb = ctime(&(plr->cyl_time));
#else
    sb = ctime(&(plr->cyl_time.tv_sec));
#endif
    *(index(sb, '\n')) = '\0';
    printf("%s: ", sb);
    switch (plr->cyl_type & 0xff) {
    case CYL_QOVERFLOW:
	printf("Log queue overflow\n");
	break;
    case CYL_SILOOVERFLOW:
	printf("Silo overflow line %2d.\n", plr->cyl_ln);
	break;
    case CYL_NOBUFFERAVAIL:
	printf("No buffer available line %2d, packet dropped of size: ***.\n",
	    plr->cyl_ln);
	break;
    case CYL_BADROUTE:
	printf("Circular route line %2d, packet =",
	    plr->cyl_ln);
	DumpPacket(plr->cyl_data, plr->cyl_len);
	break;
    case CYL_BADPKTTYPE:
	printf("Bad packet type, line %2d, packet: ",
	    plr->cyl_ln);
	DumpPacket(plr->cyl_data, plr->cyl_len);
	break;
    case CYL_BADPKTLEN:
	printf("Bad packet length, line %2d, packet: ",
	    plr->cyl_ln, plr->cyl_len);
	DumpPacket(plr->cyl_data, plr->cyl_len);
	break;
    case CYL_NOMBUFS:
	printf("No mbufs, line %2d, packet: ",
	    plr->cyl_ln);
	DumpPacket(plr->cyl_data, plr->cyl_len);
	break;
    case CYL_PKTTOOBIG:
	printf("Pkt too big for output ttybuf, line %2d, len %3d.\n",
	    plr->cyl_ln, plr->cyl_len);
	break;
      case CYL_HOPCNTZERO:
	printf("Hopcount went to zero, line %d, packet: ",
	       plr->cyl_ln);
	DumpPacket(plr->cyl_data, plr->cyl_len);
	break;
      case CYL_MTUEXCEEDED:
	printf("attempt to forward packet larger than MTU, line %d, packet: ",
	       plr->cyl_ln);
	DumpPacket(plr->cyl_data, plr->cyl_len);
	break;
      case CYL_LOSTCARRIER:
	printf("Carrier lost, line %d\n", plr->cyl_ln);
	break;
      case CYL_REGAINCARRIER:
	printf("Carrier restored, line %d\n", plr->cyl_ln);
	break;
      case CYL_BADSTATECHANGE:
	printf("Unknown stat change, line %d\n", plr->cyl_ln);
	break;
      case CYL_INVALIDDMVADDR:
	printf("Bad Address?? (dmv) line %d\n", plr->cyl_ln);
	break;
    default :
	printf("?????? type %2d, line %2d, packet: ", plr->cyl_type,
	    plr->cyl_ln);
	DumpPacket(plr->cyl_data, plr->cyl_len);
    }
    fflush(stdout);
}

/*
 * ============================================================
 * fatal - print error message and exit
 * ============================================================
 */
fatal(sb)
char *sb;
{
    perror(sb);
    exit(1);
}
/*
 * ============================================================
 * DumpPacket - print out the first len bytes in the packet
 * ============================================================
 */
DumpPacket(pch, len)
register short len;
register char *pch;
{
    printf(" %d bytes follow:0X", len);
    if (len < 0) 
	return;
    while (len--)
	printf("%2x.", (*pch++) & 0xff);
    printf("\n");
}
