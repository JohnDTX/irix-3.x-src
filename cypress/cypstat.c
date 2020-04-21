
/* 
 * cypstat.c -- get statistics from kernel and return them
 * 
 * author -- Cathy Privette
 *
 * date -- 1/29/86
 *
 * modification history
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/param.h>
#include <nlist.h>
#include <strings.h>
#include <netinet/in.h>
#include <net/if.h>

#include <stdio.h>
#include <sys/file.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <netdb.h>
#include <sys/dk.h>
#ifdef sgi
#include "if_cy.h"
#else
#include <netinet/if_cy.h>
#endif
#include "cypstat.h"


#define VMUNIX	"/vmunix"
#define KMEM	"/dev/kmem"

/* wish list for kernel symbols */

struct nlist nlst[] = {
#if defined(sgi) && defined(SVR3)
    { "cy_dev" },
#define X_CY_DEV	0
    { "rgcyln" },
#define X_RGCYLN	1
    { "cp_time" },
#define X_CP_TIME	2
    { "avenrun" },
#define X_AVENRUN	3    
#else
    { "_cy_dev" },
#define X_CY_DEV	0
    { "_rgcyln" },
#define X_RGCYLN	1
    { "_cp_time" },
#define X_CP_TIME	2
    { "_avenrun" },
#define X_AVENRUN	3    
#endif
    { 0 },
};

int fpkmem;			/* file pointer for /dev/kmem */
struct cy_dev cy_dev;		/* copy of kernels cy_dev structure */

struct cyln rgcyln[CLINEMAX];   /* copy of kernels cyln structure */

long cp_timeold[CPUSTATES] = {0, 0, 0, 0};  /* time spent in CPU states */
long cp_timecur[CPUSTATES] = {0, 0, 0, 0};  /* old and new              */
#ifdef vax
double avenrun[3];              /* avg. length of run queue */
                                /* for 1, 5, 15 min.        */
#endif
#ifdef sun 
int avenrun[3];
#endif
#ifdef sgi
long avenrun[3];
#endif

struct cystats cystats;         /* holds new stats to be sent */
int cystats_size;               /* # of bytes of cystats struct */
                                /* to be sent                   */
int flog;                       /* flag that indicates if msgs  */
                                /* should be logged or not      */
int ext();                      /* procedure called on normal exit */
extern int errno;

main(argc, argv)
char *argv[];
int argc;
{
    struct timeval tv;
    struct timezone tz;
    int i;

    if (--argc > 0 && (*++argv)[0] == '-') {
        switch ((*argv)[1]) {
            case 'l':
                flog = TRUE;
                fprintf(stderr,"cypstat: logging for pid = %d\n", getpid());
                break;
        }
    }

    /* call ext to close up when the socket is broken */
    signal(SIGPIPE, ext);
         
    if ((fpkmem=open(KMEM, 0)) < 0) 
        fatal("KMEM open");
    nlist(VMUNIX, nlst);
    if (nlst[0].n_type == 0) 
        fatal("can't nlist /vmunix");

    while(TRUE) {

        /* get global stat values */
 	getkval(nlst[X_CY_DEV].n_value, &cy_dev, sizeof(struct cy_dev));
        copy_gstats(&cystats, &cy_dev);

        /* get line stat values */
 	getkval(nlst[X_RGCYLN].n_value, rgcyln, sizeof(rgcyln));
        copy_lstats(&cystats, rgcyln);

        /* get the avg. lengths of the run queues */
 	getkval(nlst[X_AVENRUN].n_value, avenrun, sizeof(avenrun));
	for (i=0; i<3; i++) {
#ifdef vax
	    cystats.avenrun[i] = (u_long)(avenrun[i] * 100);
#endif
#ifdef sun
	    cystats.avenrun[i] = 
		(u_long) (avenrun[i]/(FSCALE/100));
#endif
#ifdef sgi
	    cystats.avenrun[i] = (u_long) ((avenrun[i] * 100)/1024);
#endif
	}
        /* get % of CPU time spent in ea. state: */
        /* USER, NICE, SYS, IDLE                 */
 	getkval(nlst[X_CP_TIME].n_value, cp_timecur, sizeof(cp_timecur));
	for (i=0; i<CPUSTATES; i++) {
	    if (cp_timecur[i] < cp_timeold[i] )

                /* counter wrapped around */
		cystats.cputime[i] = (int)((unsigned long)cp_timecur[i]
                                           -(unsigned long)cp_timeold[i]);
	    else
		cystats.cputime[i] = (cp_timecur[i] - cp_timeold[i]);
            cp_timeold[i] = cp_timecur[i];
	}

        /* get time this structure was made */
	if (gettimeofday(&tv, &tz) < 0) 
            fatal("gettimeofday");
        else
            cystats.tv_sec = tv.tv_sec;

	Dumpcystats(&cystats);
	
	sleep(INTERVAL);
    }

}

/*
 * fatal -- fatel error, print msg if logging and exit.
 */

fatal(sb)
char *sb;
{
    if (flog)
        perror(sb);
    fflush(stderr);
    exit(1);
}

/*
 * getkval -- read the requested offset from kernel memory.
 */

getkval(offset, pch, cch)
register long offset;
register int cch;
register char *pch;
{
    if (lseek(fpkmem, offset, 0) == -1)
	fatal("lseek");
    if (read(fpkmem, pch, cch) == -1)
	fatal("read(kmem)");
}

/*
 * copy_gstats -- copy certain fields from the cy_dev structure into
 *     the global variables in the cystats structure
 */

copy_gstats(pcystats, pcy_dev)
register struct cystats *pcystats;
register struct cy_dev *pcy_dev;
{
    pcystats->cyd_cipup = pcy_dev->cyd_cipup;
    pcystats->cyd_cipln = pcy_dev->cyd_cipln;
    pcystats->cyd_copln = pcy_dev->cyd_copln;
    pcystats->cyd_copup = pcy_dev->cyd_copup;
    pcystats->cyd_crint = pcy_dev->cyd_crint;
}

/*
 * copy_lstats -- copy certain fields from the cyln structure into the
 *     line variables in the cystats structure.
 */

copy_lstats(pcystats, pcyln)
struct cystats *pcystats;
register struct cyln *pcyln;
{
    register int i;
    register struct ln *pln = pcystats->rgln;

    /* size of cystats structure without the line variables */
    cystats_size = sizeof(cystats.tv_sec) + sizeof(cystats.avenrun)
                   + sizeof(cystats.cputime) + (5 * sizeof(int));

    for (i=0; i<CLINEMAX; i++, pcyln++) {
        if ((CYF_LINEUP & pcyln->cy_flags) != 0) {
            cystats_size += (sizeof(struct ln));
            pln->cyl_ln = (char)i;
  	    pln->cyl_cpsNN = pcyln->cyl_cpsNN;
  	    pln->cyl_cpsdirect = pcyln->cyl_cpsdirect;
	    pln->cyl_cpsflood = pcyln->cyl_cpsflood;
	    pln->cyl_cpsrpf = pcyln->cyl_cpsrpf;
	    pln->cyl_cprNN = pcyln->cyl_cprNN;
	    pln->cyl_cprdirect = pcyln->cyl_cprdirect;
	    pln->cyl_cprflood = pcyln->cyl_cprflood;
	    pln->cyl_cprrpf = pcyln->cyl_cprrpf;
	    pln->cyl_cchr = pcyln->cyl_cchr;
	    pln->cyl_cchs = pcyln->cyl_cchs;
	    pln->cyl_cpsip = pcyln->cyl_cpsip;
	    pln->cyl_cprip = pcyln->cyl_cprip;
	    pln->cyl_csilo = pcyln->cyl_csilo;
	    pln->cyl_flags = pcyln->cy_flags;
            pln->cyl_copdrop = pcyln->cy_copdrop;
            pln->cyl_ctpbusy = pcyln->cy_ctpbusy;
            pln->cyl_ctpidle = pcyln->cy_ctpidle;
	    pln->cyl_cesc = pcyln->cy_cesc;
            pln->cyl_csend_ifqlen = pcyln->cy_send.ifq_len;
	    pln->cyl_dest.s_addr = htonl(pcyln->cyl_dest.s_addr);
            pln++;
	}
    }
}

/*
 * ext -- called on normal exit to close up
 */

ext()
{
    signal(SIGPIPE, SIG_DFL);
    exit(0);
}

/*
 * Dumpcystats -- dump the contents of the cystats structure
 */

Dumpcystats(pcystats)
register struct cystats *pcystats;
{
    register int n;
    ToNetworkOrder(pcystats);
    cystats_size = htonl(cystats_size);	/* to network order */
    write(fileno(stdout), &cystats_size, sizeof(cystats_size));
    cystats_size = ntohl(cystats_size);	/* and back to host order */
    n = write(fileno(stdout), pcystats, cystats_size);
    fflush(stdout);
    if (n < 0)
        fatal("Dumpcystats: write");
}



/*
 * ========================================================================
 * ToNetworkOrder - Conver fields in struct to network order
 * ========================================================================
 */

ToNetworkOrder(pcystats)
register struct cystats *pcystats;
{
    register int i;
    register struct ln *pcyln;
    pcystats->tv_sec = htonl(pcystats->tv_sec);
    for (i=0; i<3; i++)
	pcystats->avenrun[i] = htonl(pcystats->avenrun[i]);
    for (i=0; i<CPUSTATES; i++) 
	pcystats->cputime[i] = htonl(pcystats->cputime[i]);
    pcystats->cyd_cipup = htonl(pcystats->cyd_cipup);
    pcystats->cyd_cipln = htonl(pcystats->cyd_cipln);
    pcystats->cyd_copln = htonl(pcystats->cyd_copln);
    pcystats->cyd_copup = htonl(pcystats->cyd_copup);
    pcystats->cyd_crint = htonl(pcystats->cyd_crint);
    for (pcyln = pcystats->rgln; pcyln < &pcystats->rgln[CLINEMAX]; 
	 pcyln++) {
	pcyln->cyl_cpsNN = htonl(pcyln->cyl_cpsNN);
	pcyln->cyl_cpsdirect = htonl(pcyln->cyl_cpsdirect);
	pcyln->cyl_cpsflood = htonl(pcyln->cyl_cpsflood);
	pcyln->cyl_cpsrpf = htonl(pcyln->cyl_cpsrpf);
	pcyln->cyl_cprNN = htonl(pcyln->cyl_cprNN);
	pcyln->cyl_cprdirect = htonl(pcyln->cyl_cprdirect);
	pcyln->cyl_cprflood = htonl(pcyln->cyl_cprflood);
	pcyln->cyl_cprrpf = htonl(pcyln->cyl_cprrpf);
	pcyln->cyl_cchr = htonl(pcyln->cyl_cchr);
	pcyln->cyl_cchs = htonl(pcyln->cyl_cchs);
	pcyln->cyl_cpsip = htonl(pcyln->cyl_cpsip);
	pcyln->cyl_cprip = htonl(pcyln->cyl_cprip);
	pcyln->cyl_csilo = htonl(pcyln->cyl_csilo);
	pcyln->cyl_flags = htonl(pcyln->cyl_flags);
	pcyln->cyl_copdrop = htonl(pcyln->cyl_copdrop);
	pcyln->cyl_ctpbusy = htonl(pcyln->cyl_ctpbusy);
	pcyln->cyl_ctpidle = htonl(pcyln->cyl_ctpidle);
	pcyln->cyl_cesc = htonl(pcyln->cyl_cesc);
	pcyln->cyl_csend_ifqlen = htonl(pcyln->cyl_csend_ifqlen);
	pcyln->cyl_dest.s_addr = htonl(pcyln->cyl_dest.s_addr);
	pcyln->cyl_ln = htonl(pcyln->cyl_ln);
    }
}

