/* $Header: /d2/3.7/src/cypress/RCS/cyconfig.c,v 1.1 89/03/27 15:03:30 root Exp $ */

/* 
 * cyconfig.c - Attach and intialize a tty line to be used by the Cypress
 *		network.
 * 
 * Author:	Thomas Narten
 * 		Dept. of Computer Sciences
 * 		Purdue University
 * Date:	Thu Sep 26 1985
 * Copyright (c) 1985 Thomas Narten
 */

#ifndef lint
static char rcs_ident[] = "$Header: /d2/3.7/src/cypress/RCS/cyconfig.c,v 1.1 89/03/27 15:03:30 root Exp $";
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <stdio.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <sys/time.h>
#ifdef sgi
#include <sys/termio.h>
#include <sys/stropts.h>
#include <sys/stream.h>
#include "if_cy.h"
#else
#include <netinet/if_cy.h>
#endif
#include <ctype.h>
#include <strings.h>
#include <arpa/inet.h>		/* make sure that inet_makeaddr is */
				/* declared correctly. As distributed in 4.2 */
				/* there was a mismatch between the  */
				/* declaration here and in the C lib. */
struct speeds {
	char *str;
	int  speed;
};

struct speeds speedtable[] = {
	"50",	B50,
	"75",	B75,
	"110",	B110,
	"134",	B134,
	"150",	B150,
	"200",	B200,
	"300",	B300,
	"600",	B600,
	"1200",	B1200,
	"1800",	B1800,
	"2400",	B2400,
	"4800",	B4800,
	"9600",	B9600,
	"19200",EXTA,		/* may be implementation dependent */
	"38400",EXTB,		/* may be implementation dependent */
	(char *)0, 0
};
char *sbargv0;
extern int errno;

main(argc, argv)
int argc;
char **argv;
{
	sbargv0 = argv[0];
	argv++; argc--;
	if (*argv[0] == '/' || *argv[0] == '-')
	    ConfigTty(argc, argv);
	else
#ifdef sgi
	    {
		fprintf(stderr,"invalid device name %s\n", argv[0]);
		usage(sbargv0);
	    }
#else
	    ConfigInterface(argc, argv);
#endif

}



/*
 * ========================================================================
 * ConfigTty - Line to be used is a tty line.
 * ========================================================================
 */

ConfigTty(argc, argv)
int argc;
char **argv;
{
    int ln;
    int ldisc;
#ifdef sgi
    static struct termio vec = {0};
#else
    struct sgttyb vec;
#endif
    int fd, speed;
    struct cy_ioctlarg cy_ioctlarg;
    struct sockaddr_in sin_dest;
    struct speeds *sp;
    char *sbTty;
    if ((argc != 3) && (argc != 5))
	usage(sbargv0);
    if (strcmp(*argv,"-s") == 0) {
	argv++;
	for (sp = speedtable ; sp->str ; sp++)
	    if (strcmp(*argv, sp->str) == 0) {
		speed = sp->speed;
		break;
	    }
	if (!sp->str) {
	    fprintf(stderr,"Bad line speed %s.\n", *argv);
	    usage(sbargv0);
	}
	printf("%s: setting line speed to %s.\n", sbargv0, *argv);
	argv++;
    } else
	speed = B9600;
    
    
    if ((fd = open("/dev/tty", O_RDWR)) >= 0) {
	(void) ioctl(fd, TIOCNOTTY, (char *) 0);
	if (close(fd) < 0) 
	    Fatal("close");
    }
    if (setpgrp(0, 0) < 0) {
	perror("setpgrp");
	exit(1);
    }
    for (fd = getdtablesize() ; fd >= 0 ; fd--)
	if (fd != 2)
	    (void) close(fd);
    sbTty = *argv;
    
    /* 
     * lets check for valid line number.
     */
    
    if ((ln=atoi(*++argv)) < 0 ) {
	fprintf(stderr, "%s: Invalid line number %d.\n", sbargv0, ln);
	exit(1);
    }
    
    if (!mpsbsin(&sin_dest, *++argv)) {
	fprintf(stderr,"%s: bad dest addr %s.\n", sbargv0, *argv);
	exit(1);
    }
    
    if ((fd = open(sbTty, O_RDWR)) < 0) {
	perror(sbTty);
	exit(1);
    }
#ifdef sgi
    (void) chmod(sbTty, 0600);
    vec.c_cflag = CS8|CREAD|HUPCL|CLOCAL|speed; /* set the speed */
    vec.c_iflag = IGNPAR|IGNBRK;
    while (0 <= ioctl(fd,I_POP,0))	/* pop off all other modules */
	continue;
    if (ioctl(fd,I_PUSH,"if_cy") < 0) {	/* push ours */
	fprintf(stderr, "%s: ", sbargv0);
	perror("I_PUSH");
	exit(2);
    }
#else
    (void) fchmod(fd, 0600);
    ldisc = CYDISC;
    vec.sg_ispeed = vec.sg_ospeed = speed;
    vec.sg_flags = RAW | EVENP | ODDP;
    if (ioctl(fd, TIOCSETP, (char *) &vec) < 0) {
	fprintf(stderr, "%s: ", sbargv0);
	perror("TIOCSETP");
	exit(2);
    }
    if (ioctl(fd, TIOCSETD, (char *) &ldisc) < 0) {
	fprintf(stderr, "%s: ", sbargv0);
	perror("TIOCSETD");
	exit(2);
    }
#endif
    
    
    cy_ioctlarg.cyi_ln = ln;
    bcopy((char *) &sin_dest, (char *)&cy_ioctlarg.cyi_sin, sizeof (struct sockaddr_in));
#ifdef sgi
    if (tty_ioctl(fd, CYIOC_ADDLINE, (char *) &cy_ioctlarg) < 0) {
#else
    if (ioctl(fd, CYIOC_ADDLINE, (char *) &cy_ioctlarg) < 0) {
#endif
	Perror(sbargv0);
	exit(1);
    }
#ifdef sgi
    if (tty_ioctl(fd, CYIOC_SDESTADDR, (char *) &cy_ioctlarg) < 0) {
#else
    if (ioctl(fd, CYIOC_SDESTADDR, (char *) &cy_ioctlarg) < 0) {
#endif
	fprintf(stderr, "Line %d", ln);
	Perror("");
	exit(1);
    }
    /* we can't exit, the /dev/tty whose discipline got just got
     * set gets closed (and the discipline get reset) when we die.
     */
    if (fork())
	exit(0);  
    
    (void) close(2);
    pause();	  
}



#ifndef sgi
/*
 * ========================================================================
 * ConfigInterface - Configure the network interface to be a Cypress line
 * ========================================================================
 */

ConfigInterface(argc, argv)
int argc;
char **argv;
{
    
    struct cy_ioctlarg cy_ioctlarg;
    struct sockaddr_in sin_dest;
    int s, ln;
    if (argc != 3)
	usage(sbargv0);

    (void) strcpy(cy_ioctlarg.cyi_ifname, *argv);
    (void) strcpy(cy_ioctlarg.cyi_name, "cy0");

    /* 
     * lets check for valid line number.
     */

    if ((ln=atoi(*++argv)) < 0 ) {
	fprintf(stderr, "%s: Invalid line number %d.\n", sbargv0, ln);
	exit(1);
    }
    cy_ioctlarg.cyi_ln = ln;
    
    if (!mpsbsin(&sin_dest, *++argv)) {
	fprintf(stderr,"%s: bad dest addr %s.\n", sbargv0, *argv);
	exit(1);
    }
    bcopy((char *) &sin_dest, (char *)&cy_ioctlarg.cyi_sin, sizeof (struct sockaddr_in));

    if ((s=socket(AF_CYPRESS, SOCK_RAW, CYPROTO_RAW)) < 0) {
	perror("cyconfig: socket");
	exit(1);
    }
    
    printf("%s\n", cy_ioctlarg.cyi_name);
    if (ioctl(s, CYIOC_ATTACHIF, (caddr_t)&cy_ioctlarg) < 0) {
       	Perror("ioctl(CYIOC_ATTACHIF)");
	exit(1);
    }
}
#endif

usage(p)
     char *p;
{
    printf("%s: Usage: %s [-s speed] dev line# dest\n",sbargv0, p);
    exit(1);
}

