 /* $Header: /d2/3.7/src/cypress/RCS/cyioctl.c,v 1.1 89/03/27 15:03:33 root Exp $ */

 /* 
  * cyioctl.c - quick and dirty ioctl for cypress. used for testing only
  * 
  * Author:	Thomas Narten
  * 		Dept. of Computer Sciences
  * 		Purdue University
  * Date:	Thu Oct 17 1985
  * Copyright (c) 1985 Thomas Narten
  */

 static char rcs_ident[] = "$Header: /d2/3.7/src/cypress/RCS/cyioctl.c,v 1.1 89/03/27 15:03:33 root Exp $";

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

int setroute(), dumproutes(), flushroutes(), routetome(), gnetnum();
int setrouteusage(), routetomeusage(), nullusage(), getstats();

 static struct menu {
     char *sbName;		/* name of function */
     int nargs;			/* how many arguments it takes */
     char *sbDescription;	/* what does it do */
     int (*pfc)();		/* function that implements the ioctl */
     int (*pfcusage)();		/* print message on how to use function */
 } rgmenu[] = {
     {"route", 2, "route packets for host over the specified line",
	setroute, setrouteusage},
     {"dumproutes", 0, "dump contents of routing table", dumproutes, nullusage},
     {"flushroutes", 0, "flush all Cypress level routing entrie",
	flushroutes, nullusage},
     {"routetome", 1, "accept packets for host x as addressed to me",
	routetome, routetomeusage},
     {"getnet", 0, "What network number are we using?", gnetnum, nullusage},
     {"stats", 0, "dump cydev and cyln structures", getstats, nullusage},
	 
 };

int cMenuMax = (sizeof(rgmenu)/sizeof(struct menu));

struct nlist nlst[] = {
    { "_cy_dev" },
#define X_CY_DEV	0
    { "_rgcyln" },
#define X_RGCYLN	1
    { 0 },
};

extern int errno;
char *sbargv0;


main(argc, argv)
int argc;
char *argv[];
{
    register struct menu *pm;
    sbargv0 = argv[0];

    if (argc == 1) {
	PrintMenu();
	exit(0);
    }
    for(pm= rgmenu; pm < &rgmenu[cMenuMax]; pm++) {
	if (strcmp(argv[1], pm->sbName) == 0) {
	    if (argc-2 != pm->nargs) {
		(*pm->pfcusage)();
		exit(1);
	    }

	    switch(argc - 2) {
		case 0: (*pm->pfc)();
			break;
		case 1: (*pm->pfc)(argv[2]);
			break;
		case 2: (*pm->pfc)(argv[2], argv[3]);
			break;
		case 3: (*pm->pfc)(argv[2], argv[3], argv[4]);
			break;
		default: fprintf(stderr, "%s: Can't handle %2d arguments! (internal error)\n",
			 sbargv0, pm->nargs);
			 exit(1);
		
	    }
	    exit(0);
	}
    }
    printf("%s: unknown function %s.\n", sbargv0, argv[1]);
    exit(0);

}


/*
 * ============================================================
 * PrintMenu - print the various ioctl codes we know about
 * 
 * ============================================================
 */
PrintMenu()
{
    register struct menu *pmenu;
    printf("\tName\# args\tDescription\n");
    printf("\t====\t======\t===========\n");
    for (pmenu = rgmenu; pmenu < (struct menu *)(((int)rgmenu)+(sizeof rgmenu)); pmenu++) 
	printf("%14s\t%2d\t%s\n",pmenu->sbName,
	pmenu->nargs, pmenu->sbDescription);
}
/*
 * ============================================================
 * nullusage - null routine
 * ============================================================
 */
nullusage()
{
    fprintf(stderr, "%s: invalid number of arguments.\n", sbargv0);
}
/*
 * ============================================================
 * routetomeusage - usage message for route to me ioctl
 * ============================================================
 */
routetomeusage()
{
    printf("Usage: %s: routetome host\n", sbargv0);
}
/*
 * ============================================================
 * gnetnum - get the network number that the kernel is using for 
 * it Cypress network interface.
 * ============================================================
 */
gnetnum()
{
    struct cy_ioctlarg cy_ioctlarg;
    int s;
    struct in_addr net;		/* network number of Cypress interface */
    if ((s = socket(AF_INET, SOCK_DGRAM,0)) < 0) {
	perror("socket");
	exit(1);
    }
    strcpy(cy_ioctlarg.cyi_name, "cy0");
#ifdef sgi
    if (tty_ioctl(s, CYIOC_GNETNUM, &cy_ioctlarg) < 0) {
#else
    if (ioctl(s, CYIOC_GNETNUM, &cy_ioctlarg) < 0) {
#endif
	Perror("ioctl");
	exit(1);
    }
    printf("Network number is %s.\n",
    	inet_ntoa(cy_ioctlarg.cyi_saddr));
}

/*
 * ============================================================
 * setrouteusage - print usage message for set route ioctl
 * ============================================================
 */
setrouteusage()
{
    fprintf(stderr, "usage: %s setroute implet linenumber\n");
}

/*
 * ============================================================
 * setroute - set a routing table entry
 * ============================================================
 */
setroute(sbaddr, sbln)
char *sbln, *sbaddr;
{
    struct cy_ioctlarg cy_ioctlarg;
    int ln = atoi(sbln);
    struct netent *pnetent;
    int s;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
	perror("ifconfig: socket");
	exit(1);
    }
    strcpy(cy_ioctlarg.cyi_name, "cy0");
    if (!mpsbsin(&cy_ioctlarg.cyi_sin, sbaddr)) {
	fprintf(stderr, "%s: bad dest addr %s.\n", sbargv0, sbaddr);
	exit(1);
    }
    cy_ioctlarg.cyi_ln = ln;
    printf("Routing packets for implet %s out over line %2d\n", sbaddr, ln);
#ifdef sgi
    if (tty_ioctl(s, CYIOC_SROUTE, &cy_ioctlarg) < 0) {
#else
    if (ioctl(s, CYIOC_SROUTE, &cy_ioctlarg) < 0) {
#endif
	Perror("ioctl");
	exit(1);
    }
}

/*
 * ============================================================
 * dumproutes - dump out the contents of the routing table
 * ============================================================
 */
struct cy_dev cy_dev;
int fpkmem;
dumproutes()
{
    register int i;
    if ((fpkmem=open(KMEM, 0)) < 0)
	fatal(KMEM);
    nlist(VMUNIX, nlst);
    if (nlst[0].n_type == 0)
	fatal("can't nlist /vmunix:");
    getkval(nlst[X_CY_DEV].n_value, &cy_dev, sizeof(cy_dev));
    for (i=0; i< 255; i++) {
	if (cy_dev.MPhln[i] == CYR_NOROUTE)
	    continue;
	if (cy_dev.MPhln[i] == CYR_THISHOST)
	    printf("Packets for host %3d accepted locally.\n", i);
	else
	    printf("Packets for host %3d sent out on line %1d.\n", i, cy_dev.MPhln[i]);
    }
}
/*
 * ============================================================
 * getkval - read the requested offset from kernel memory.
 * ============================================================
 */
getkval(offset, pch, cch)
register long offset;
register int cch;
register char *pch;
{
    if (lseek(fpkmem, offset, 0) == -1)
	fatal("lseek");
    if (read(fpkmem, pch, cch) <= 0)
	fatal("read(kmem)");
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
 * flushroutes - Flush all Cypress level routes from the routing table
 * ============================================================
 */
flushroutes()
{
    struct cy_ioctlarg cy_ioctlarg;
    int s;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
	perror("ifconfig: socket");
	exit(1);
    }
    strcpy(cy_ioctlarg.cyi_name, "cy0");
#ifdef sgi
    if (tty_ioctl(s, CYIOC_FLUSHROUTES, &cy_ioctlarg) < 0) {
#else
    if (ioctl(s, CYIOC_FLUSHROUTES, &cy_ioctlarg) < 0) {
#endif
	Perror("ioctl");
	exit(1);
    }
}
/*
 * ============================================================
 * srouteip - Route packet destined for the specified Cypress host 
 * back up to IP
 * ============================================================
 */
routetome(sb)
register char *sb;
{
    struct cy_ioctlarg cy_ioctlarg;
    int s;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
	perror("ifconfig: socket");
	exit(1);
    }
    strcpy(cy_ioctlarg.cyi_name, "cy0");
    if (!mpsbsin(&cy_ioctlarg.cyi_sin, sb)) {
	fprintf(stderr, "%s: bad dest addr %s.\n", sbargv0, sb);
	exit(1);
    }
    printf("Packets addressed to host %s, will be accepted locally.\n",sb);
#ifdef sgi
    if (tty_ioctl(s, CYIOC_SROUTEIP, &cy_ioctlarg) < 0) {
#else
    if (ioctl(s, CYIOC_SROUTEIP, &cy_ioctlarg) < 0) {
#endif
	Perror("ioctl");
	exit(1);
    }
    
}


/*
 * ========================================================================
 * getstats - Dump out the cyln and cydev structures
 * ========================================================================
 */

#ifdef sgi
static struct cy_dev cydev;
static struct cyln rgcyln[CLINEMAX];
#endif
getstats()
{
#ifdef sgi
    struct cyln *pcyln;
#else
    struct cy_dev cydev;
    struct cyln rgcyln[CLINEMAX], *pcyln;
#endif
    if ((fpkmem=open(KMEM, 0)) < 0)
	fatal(KMEM);
    nlist(VMUNIX, nlst);
    if (nlst[0].n_type == 0)
	fatal("can't nlist /vmunix:");
    getkval(nlst[X_CY_DEV].n_value, &cydev, sizeof(cydev));
    printf("  cipup\tcipln\tcopln\tcopup\n");
    printf("%7d %7d %7d %7d\n", cydev.cyd_cipup, cydev.cyd_cipln,
	   cydev.cyd_copln, cydev.cyd_copup);
    printf("  crint logrechd logrectl\n");
    printf("%7d\t%7d\t%7d\n", cydev.cyd_crint, cydev.cyd_logrechd, 
	   cydev.cyd_logrectl);
    printf("flags = 0X%x \n", cydev.cyd_flags);
    printf("haddr = 0X%x, ", cydev.cyd_haddr.s_addr);
    printIPaddr(cydev.cyd_haddr.s_addr);
    printf("\n");
    printf("baddr = 0X%x, ", cydev.cyd_baddr.s_addr);
    printIPaddr(cydev.cyd_baddr.s_addr);
    printf("\n");
    printf("naddr = 0X%x, ", cydev.cyd_naddr.s_addr);
    printIPaddr(cydev.cyd_naddr.s_addr);
    printf("\n");
    printf("hid = %d\n", cydev.cyd_hid);
    
    getkval(nlst[X_RGCYLN].n_value, rgcyln, sizeof(rgcyln));
    for (pcyln = rgcyln; pcyln < &rgcyln[CLINEMAX]; pcyln++) {
	if ((pcyln->cy_flags&CYF_LINEUP) == 0)
	    printf("line %d not configured\n", pcyln-rgcyln);
	else {
	    printf("\nLine %d\n", pcyln-rgcyln);
	    DumpCyln(pcyln);
	}
    }

}



/*
 * ========================================================================
 * DumpCyln - Dump the contents of a cyln structure
 * ========================================================================
 */

DumpCyln(pcyln)
register struct cyln *pcyln;
{
    printf("Output packet counts:\nNN\tdirect\tflood\trpf\n");
    printf("%d\t%d\t%d\t%d\n", pcyln->cyl_cpsNN, pcyln->cyl_cpsdirect,
	   pcyln->cyl_cpsflood, pcyln->cyl_cpsrpf);
    printf("Input packet counts:\nNN\tdirect\tflood\trpf\n");
    printf("%d\t%d\t%d\t%d\n", pcyln->cyl_cprNN, pcyln->cyl_cprdirect,
	   pcyln->cyl_cprflood, pcyln->cyl_cprrpf);
    printf("Characters received: %d, sent %d\n", pcyln->cyl_cchr, 
	   pcyln->cyl_cchs);
    printf("Packets to IP: %d, from IP: %d\n", pcyln->cyl_cpsip,
	   pcyln->cyl_cprip);
    printf("flags = 0X%x, <", pcyln->cy_flags);
    if (pcyln->cy_flags & CYF_LINEUP)
	printf("LINEUP,");
    if (pcyln->cy_flags & CYF_ISTTY)
	printf("ISTTY,");
    if (pcyln->cy_flags & CYF_MONITORON)
	printf("MONITORON,");
    printf(">\n");
    printf("Queue: max len %d, current %d, drops %d\n",
	   pcyln->cy_send.ifq_maxlen, pcyln->cy_send.ifq_len,
	   pcyln->cy_send.ifq_drops);
}



/*
 * ========================================================================
 * printIPaddr - Print out an IP address.
 * ========================================================================
 */

printIPaddr(w)
unsigned long w;
{
    char *pch;
    w = ntohl(w);
    pch = (char *)&w;
    
    printf("%u.%u.%u.%u", pch[0]&0xff, pch[1]&0xff, pch[2]&0xff, pch[3]&0xff);
}



