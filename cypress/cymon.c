/* 
 * cymon.c -- connects to cypress monitor server and displays the
 *     monitored statistics. Types of statistics displayed are
 *     global, line and status. Global are displayed when the
 *     global window is active, line are displayed when the line
 *     window is active and status are always displayed.
 * 
 * author --  Cathy Privette
 * 
 * date -- 10/4/85
 *
 * modification history
 *     11/24/85 -- allow to monitor more than one physical line at
 *                 a site and allow choice of site to monitor.
 *     12/6/85  -- read data from server in one chunk.
 *     12/19/85 -- make the monitor interactive.
 *     1/13/86  -- add status log records.
 *     1/29/86  -- major changes, no longer reading log records.
 *                 now reading a struct, cystats, which contains
 *                 all the information about a site. this info is
 *                 then separated and stored in the log record structs.
 *
 *     2/7/86   -- add totals window and reset command
 *     3/26/86  -- add avg packet sizes and data rates
 *     4/2/86   -- add avg chars received/interrupt and host name for
 *                 line stats
 *     4/18/86  -- change command line and add percent of chars sent
 *                 that are escape chars.
 *     5/5/86   -- another adjustment to the command line and misc 
 *		   cleanup.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/dk.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <curses.h>
#include <signal.h>
#include <strings.h>
#undef TRUE
#undef FALSE
#ifdef sgi
#include <sys/stream.h>
#include "if_cy.h"
#else
#include <netinet/if_cy.h>
#endif
#include "cypstat.h"
#include "cymon.h"

#ifdef sgi
extern char *ctime();
#endif
int  fd;                                       /* network file descriptor */
WINDOW *wn_title, *wn_types, *wn_graph,        /* windows for drawing stats */
       *wn_prompt, *wn_help;
struct cystats cystatscur, cystatsold,         /* new, old and reset  */
               cystatsreset;                   /* copy of stats       */
char *sbsite;                                  /* cypress site name   */
int curpos = 0;                                /* the current position*/
                                               /* in a window request */
char sbrequest[REQUESTSIZE];                   /* a window request    */

main(argc, argv)
int argc;
char *argv[];

{
	struct  hostent *him;           /* host table entry          */
	struct  servent *cypstat;       /* service file entry        */
	struct  sockaddr_in sin;        /* socket address            */
        int i;                          /* loop control              */
        char *sb;                       /* misc. char. string        */
        int yloc;                       /* y-location of a plot      */
        int die(), ext();               /* routines called on exit   */
	char sbMsg[90];                 /* messages                  */
        FILE *fds;                      /* network stream            */
        int nfds;                       /* no. of file descriptors to*/
                                        /* be checked in select      */
        int readfds, writefds;          /* file descriptors to be    */
        int exceptfds, maskfds;         /* checked in select         */
	int win, arg;                   /* variables set by the cmd  */
                                        /* line args to select       */
					/* monitor options           */
        struct timeval *timeout;        /* length of time to block   */
                                        /* on select                 */

        init();                         /* initialize data structures  */
        setdefaults();                  /* set default monitor options */

        /* get monitor program options - window to display */
        if (--argc > 0 && (*++argv)[0] == '-') {
            switch ((*argv)[1]) {
                case 'g' :		/* global stats, no arg */
		    win = GLOBAL;
		    arg = NOTSET;
		    --argc;
		    ++argv;
		    break;

                case 'l' :		/* line stats, next arg must be ln # */
                     if (--argc > 0 && isdigit((*++argv)[0])) {
                         if (((i=atoi(argv[0])) >= 0) &&
                             (i < CLINEMAX)) {
			     win = LINE;
			     arg = i;
			     --argc;
			     ++argv;
		         }
                         else {
		             printf("cymon: line number out of range\n");
		 	     printstartup();
			 }
		     }
		     else
		         printstartup();
			    
                     break;
 
                case 't' :		/* total stats, next arg must be */
                                        /* a line # or the default is g  */
   		     switch (--argc) {
 		         case 0 :       /* use default, global totals */
			     win = TOTAL;
			     arg = NOTSET;
   			     break;
		         case 1 :       /* line to be monitored    */
                         default:       /* ignore all but next arg */
                             if (isdigit((*++argv)[0])) {
                                 if (((i=atoi(argv[0])) >= 0) &&
                                     (i < CLINEMAX)) {
				     win = TOTAL;
				     arg = i;
				     --argc;
				     ++argv;
			         }
                                 else {
				     printf("cymon: line number out of range\n");
				     printstartup();
				 }
			     }
			     else {	/* use default, global totals */
			         win = TOTAL;
				 arg = NOTSET;
			     }
			     break;
		    }
                    break;
                default :
		    printstartup();
	    }
	}
	else {				/* set default window */
	    win = GLOBAL;
	    arg = NOTSET;
	}
	if (argc > 0)			/* site to connect to */
	    sbsite = (*argv);
	        
        signal(SIGINT, ext);
                    
        /* initialize screen */
        initscr();
        noecho();
        nonl();
        crmode();

	/* irregular size screen causes output to be unreadable */
	if (LINES < 24 || COLS < 80) {
	    move(0,0);
	    printw("SCREEN NOT BIG ENOUGH\n");
	    refresh();
	    ext();
	}

/*        maxyloc = LINES-6; */
        maxyloc = LINES-5;	/* 4 lines at top, one for prompt at bottom */

	/* initialize windows */
        wn_title = newwin(4, 0, 0, 0); /* 4 lines at top of screen */

        wn_graph = newwin(maxyloc, COLS-30, 4, 30); /*14 lines at center left*/
        wn_types = newwin(maxyloc, 30, 4, 0); /* 14 lines at center right */
        wn_prompt = newwin(1, 0, LINES-1, 0);
        wn_help = newwin(maxyloc, 0, 5, 0);

        wmove(wn_title, 0, 14);
        wprintw(wn_title, "CYPRESS MONITOR v8.1: ");
	wmove(wn_title, 1, 11);
        wprintw(wn_title, "Time of the last update: ");
        wmove(wn_title, 2, 5);
        wprintw(wn_title, "Cpu utilization USER:      %%, SYSTEM:      %%, ");
        wprintw(wn_title, "NICE:      %%, IDLE:      %%");
        wmove(wn_title, 3, 23);
        wprintw(wn_title, "Load average:      ,      ,      ");
        wrefresh(wn_title);
	(*rgwindow[win].pfc)(arg);

        /* connect to server */
	if ( !mpsbsin(&sin, sbsite)) {
	    sprintf(sbMsg, "cymon: unknown host %s", sbsite);
            die(sbMsg);
	}

	if ((cypstat = getservbyname("cypress-stat", "tcp")) == (struct servent *)NULL) {
	    die("cymon: cypress-stat/tcp unknown service");
	}

	if ((fd = socket(AF_INET, SOCK_STREAM, 0, 0)) < 0) {
	    die("cymon: socket failed");
	}

	sin.sin_port = cypstat->s_port;

	if (connect(fd, (caddr_t)&sin, sizeof(sin), 0) < 0) {
	    sprintf(sbMsg, "cymon: Can't connect to port %d at %s\n",
		    ntohs(sin.sin_port), sbsite);
	    die(sbMsg);
	}
        wmove(wn_title, 0, 36);
        wprintw(wn_title, "connected to %s", sbsite);
        sb = ctime(&(cystatscur.tv_sec));
        *(index(sb, '\n')) = '\0';
        wmove(wn_title, 1, 36);
        wprintw(wn_title, "%s", sb);
        UpdateStatus();
        wrefresh(wn_title);

        wclear(wn_prompt);
        wprintw(wn_prompt, " ? ");
        wrefresh(wn_prompt);

        /* read and plot responses */
        writefds = 0;
        exceptfds = 0;
        nfds = fd + 1;
        timeout = NULL;

        for (;;) {

            /* want to read from the socket or stdin */
            readfds = (1 << fd) | (1 << 0);
            if (select(nfds, &readfds, &writefds, &exceptfds, timeout) > 0) {
                maskfds = readfds;

                /* read from socket */
                if ((maskfds & (1 << fd)) > 0)
                    readsocket();

                /* read from terminal */
                if ((readfds & 1) > 0) 
                    readterminal();
	    }
	} 
}

/*
 * readsocket -- read input from the socket.
 */

readsocket()
{
    char *ptr, *sb;
    int n, ntot;
    int cystats_size;               /* # of bytes of stats info. to be read */
    struct cystats *pcystatscur,    /* ptr to stats read, old and new       */
                   *pcystatsold;

    /* read cypress stats */
    n = read(fd, &cystats_size, sizeof(cystats_size));
    if (n != sizeof(cystats_size))
        die("cymon: can't read size of stats");
    cystats_size = ntohl(cystats_size);
    pcystatscur = &cystatscur;
    pcystatsold = &cystatsold;
    n = 0;
    ntot = 0;
    ptr = (char *)pcystatscur;

    /* read until we get all the data */
    while (ntot < cystats_size) {
        n = read(fd, ptr, (cystats_size - ntot));
        if (n <= 0)
            die("cymon: read from server failed");
        ntot += n;
        ptr += n;
    }
    FromNetworkOrder(pcystatscur);

    /* update time the log record was made */
    sb = ctime(&(cystatscur.tv_sec));
    *(index(sb, '\n')) = '\0';
    wmove(wn_title, 1, 36);
    wprintw(wn_title, "%s", sb);

    /* update status fields */
    UpdateStatus();
    wrefresh(wn_title);

    /* compute differences in the old and new counts */
    /* and update in the log record structures       */
    computediffs(pcystatsold, pcystatscur);

    /* compute and update tty percents busy and idle */
    ComputeTTYPct();

    /* update the graphs for the displayed window */
    if (fwindow == GLOBAL || fwindow == LINE)
        UpdateGraphs();
    else if (fwindow == TOTAL)
        UpdateTotals();
    wrefresh(wn_prompt);
}

/*
 *
 * computediffs -- compute the differences in the old and new values
 *     of the line and device fields in the cystats structure and save
 *     the results in the log record data structure to be graphed. This
 *     is the new 5 second data.
 */

 /*
  * NOTE: The line data in the current cystats structure is compressed, if
  *       not all CLINEMAX lines were configured then only the data for the
  *       ones that were configured was sent. The old cystats struct will
  *       contain the correct total values for all lines after this routine
  *       and if a line is not configured the cyl_ln field in the old cystats
  *       struct will be set to NOTCONF.  The log record structure, mptypcylr,
  *       will contain the differences between the old and the current values
  *       for all the fields in the cystats struct. The reset cystats
  *       structure accumulates totals since the last reset command by adding
  *       up 5 second data values.
  */

computediffs(pold, pcur)
struct cystats *pold, *pcur;
{
    register struct cystats *prst;
    register struct ln *plnold, *plncur, *plnrst;
    register int i;

    struct hostent *phost;	/* host on other end of line         */
    unsigned long *paddr;	/* addr of host on other end of line */

    prst = &cystatsreset;

    /* global stats */
    prst->cyd_cipup += mptypcylr[CYD_CIPUP]->new_cpkt = pcur->cyd_cipup - pold->cyd_cipup;
    pold->cyd_cipup = pcur->cyd_cipup;

    prst->cyd_cipln += mptypcylr[CYD_CIPLN]->new_cpkt = pcur->cyd_cipln - pold->cyd_cipln;
    pold->cyd_cipln = pcur->cyd_cipln;

    prst->cyd_copup += mptypcylr[CYD_COPUP]->new_cpkt = pcur->cyd_copup - pold->cyd_copup;
    pold->cyd_copup = pcur->cyd_copup;

    prst->cyd_copln += mptypcylr[CYD_COPLN]->new_cpkt = pcur->cyd_copln - pold->cyd_copln;
    pold->cyd_copln = pcur->cyd_copln;

    prst->cyd_crint += mptypcylr[CYD_ACHINTR]->new_cpktf = pcur->cyd_crint - pold->cyd_crint;
    pold->cyd_crint = pcur->cyd_crint;

    mptypcylr[CYD_CSILO]->new_cpkt = 0;
    mptypcylr[CYD_CCHS]->new_cpkt = mptypcylr[CYD_CCHR]->new_cpkt = 0;

    /* line stats */
    plnold = pold->rgln;
    plncur = pcur->rgln;
    plnrst = prst->rgln;
    for (i=0; i<CLINEMAX; i++) {
        if (plncur->cyl_ln == (char)i) {

            /* line i is configured and new data was sent */
            plnrst->cyl_cpsdirect += mptypcylr[CYL_CPSDIRECT][i].new_cpkt =
                         plncur->cyl_cpsdirect - plnold->cyl_cpsdirect;
            plnold->cyl_cpsdirect = plncur->cyl_cpsdirect;


            plnrst->cyl_cpsNN += mptypcylr[CYL_CPSNN][i].new_cpkt =
                         plncur->cyl_cpsNN - plnold->cyl_cpsNN;
            plnold->cyl_cpsNN = plncur->cyl_cpsNN;

            plnrst->cyl_cpsflood += mptypcylr[CYL_CPSFLOOD][i].new_cpkt =
                         plncur->cyl_cpsflood - plnold->cyl_cpsflood;
            plnold->cyl_cpsflood = plncur->cyl_cpsflood;

            plnrst->cyl_cpsrpf += mptypcylr[CYL_CPSRPF][i].new_cpkt =
                         plncur->cyl_cpsrpf - plnold->cyl_cpsrpf;
            plnold->cyl_cpsrpf = plncur->cyl_cpsrpf;

            plnrst->cyl_cprNN += mptypcylr[CYL_CPRNN][i].new_cpkt =
                         plncur->cyl_cprNN - plnold->cyl_cprNN;
            plnold->cyl_cprNN = plncur->cyl_cprNN;

            plnrst->cyl_cprdirect += mptypcylr[CYL_CPRDIRECT][i].new_cpkt =
                         plncur->cyl_cprdirect - plnold->cyl_cprdirect;
            plnold->cyl_cprdirect = plncur->cyl_cprdirect;

            plnrst->cyl_cprflood += mptypcylr[CYL_CPRFLOOD][i].new_cpkt =
                         plncur->cyl_cprflood - plnold->cyl_cprflood;
            plnold->cyl_cprflood = plncur->cyl_cprflood;

            plnrst->cyl_cprrpf += mptypcylr[CYL_CPRRPF][i].new_cpkt =
                         plncur->cyl_cprrpf - plnold->cyl_cprrpf;
            plnold->cyl_cprrpf = plncur->cyl_cprrpf;

            plnrst->cyl_cchr += mptypcylr[CYL_CCHR][i].new_cpkt =
                        plncur->cyl_cchr - plnold->cyl_cchr;
            plnold->cyl_cchr = plncur->cyl_cchr;

            /* sum up characters received for global stats */
            mptypcylr[CYD_CCHR]->new_cpkt += mptypcylr[CYL_CCHR][i].new_cpkt;

            plnrst->cyl_cchs += mptypcylr[CYL_CCHS][i].new_cpkt =
                         plncur->cyl_cchs - plnold->cyl_cchs;
            plnold->cyl_cchs = plncur->cyl_cchs;

            /* sum up characters sent for global stats */
            mptypcylr[CYD_CCHS]->new_cpkt += mptypcylr[CYL_CCHS][i].new_cpkt;

            plnrst->cyl_cpsip += mptypcylr[CYL_CPSIP][i].new_cpkt =
                        plncur->cyl_cpsip - plnold->cyl_cpsip;
            plnold->cyl_cpsip = plncur->cyl_cpsip;

            plnrst->cyl_cprip += mptypcylr[CYL_CPRIP][i].new_cpkt =
                        plncur->cyl_cprip - plnold->cyl_cprip;
            plnold->cyl_cprip = plncur->cyl_cprip;

            plnrst->cyl_csilo += mptypcylr[CYL_CSILO][i].new_cpkt =
                        plncur->cyl_csilo - plnold->cyl_csilo;
            plnold->cyl_csilo = plncur->cyl_csilo;

	    /* sum up silo overflows for global stats */
	    mptypcylr[CYD_CSILO]->new_cpkt += mptypcylr[CYL_CSILO][i].new_cpkt;

            plnrst->cyl_copdrop += mptypcylr[CYL_COPDROP][i].new_cpkt =
                        plncur->cyl_copdrop - plnold->cyl_copdrop;
            plnold->cyl_copdrop = plncur->cyl_copdrop;

            plnrst->cyl_csend_ifqlen = mptypcylr[CYL_CSEND_IFQLEN][i].new_cpkt
                        = plncur->cyl_csend_ifqlen;
            plnold->cyl_csend_ifqlen = plncur->cyl_csend_ifqlen;

            if (plncur->cyl_ctpbusy < plnold->cyl_ctpbusy)

                /* when the counter wraps around */
                mptypcylr[CYL_CTPBUSY][i].new_cpktf = (float)
                                    ((unsigned int)plncur->cyl_ctpbusy -
                                     (unsigned int)plnold->cyl_ctpbusy);
            else  
                mptypcylr[CYL_CTPBUSY][i].new_cpktf = (float)
                             (plncur->cyl_ctpbusy - plnold->cyl_ctpbusy);
            plnrst->cyl_ctpbusy += (int)mptypcylr[CYL_CTPBUSY][i].new_cpktf;
            plnold->cyl_ctpbusy = plncur->cyl_ctpbusy;

            if (plncur->cyl_ctpidle < plnold->cyl_ctpidle)

                /* when the counter wraps around */
                mptypcylr[CYL_CTPIDLE][i].new_cpktf = (float)
                                    ((unsigned int)plncur->cyl_ctpidle -
                                     (unsigned int)plnold->cyl_ctpidle);
            else
                mptypcylr[CYL_CTPIDLE][i].new_cpktf = (float)
                             (plncur->cyl_ctpidle - plnold->cyl_ctpidle);
 
            plnrst->cyl_ctpidle += (int)mptypcylr[CYL_CTPIDLE][i].new_cpktf;
            plnold->cyl_ctpidle = plncur->cyl_ctpidle;

	    plnrst->cyl_cesc += mptypcylr[CYL_CESC][i].new_cpkt =
	                plncur->cyl_cesc - plnold->cyl_cesc;
	    plnold->cyl_cesc = plncur->cyl_cesc;

	    /* get the name of the host on the other      */
	    /* end of this logical line if it has changed */
	    plncur->cyl_dest.s_addr = ntohl(plncur->cyl_dest.s_addr);
	    if (plnold->cyl_dest.s_addr != plncur->cyl_dest.s_addr) {
                paddr = &plncur->cyl_dest.s_addr;
	        if (*paddr != 0L) {
		    if ((phost = gethostbyaddr(paddr, 4, AF_INET)) != NULL) {
		        strncpy(sbh_name[i], phost->h_name, NMSZ-1);
			sbh_name[i][NMSZ-1] = '\0';
		    }
		    else 
		        strcpy(sbh_name[i], inet_ntoa(*paddr));
		}
		else
		    strcpy(sbh_name[i], "UNINITIALIZED");
	        plnrst->cyl_dest.s_addr = plnold->cyl_dest.s_addr
                           = plncur->cyl_dest.s_addr;
	    }
		
		    

            plnrst->cyl_ln = plnold->cyl_ln = plncur->cyl_ln;
            plncur->cyl_ln = NOTCONF;
            plnold++;
            plnrst++;
            plncur++;
	    computeavgs(i);
	}

        /* line i not configured, no new data was sent */
        else {
            plnrst->cyl_ln = plnold->cyl_ln = NOTCONF;
            plnold++;
            plnrst++;
            if ((fwindow == LINE || fwindow == TOTAL) && (fline == i))

                /* set window to not configured */
                LineNotConf(i);                
	}
    }

    /* finished storing this set of data, increment for the next set */
    if (++newdata >= NINTV)
        newdata = 0;

    /* save current time value */
    pold->tv_sec = pcur->tv_sec;
}

/*
 * computeavgs -- compute avg stats for each active line.
 */

computeavgs(ln)
int ln;
{
    int j;
    float *pcpkt1, *pcpkt2, time;
    struct avstats *aptr;

    /* new data */
    aptr = &avtab[ln][newdata];
    aptr->chs = mptypcylr[CYL_CCHS][ln].new_cpkt;
    aptr->pkts = mptypcylr[CYL_CPSDIRECT][ln].new_cpkt +
		 mptypcylr[CYL_CPSFLOOD][ln].new_cpkt + 
		 mptypcylr[CYL_CPSNN][ln].new_cpkt +
		 mptypcylr[CYL_CPSRPF][ln].new_cpkt;
    aptr->chr = mptypcylr[CYL_CCHR][ln].new_cpkt;
    aptr->pktr = mptypcylr[CYL_CPRDIRECT][ln].new_cpkt +
		 mptypcylr[CYL_CPRNN][ln].new_cpkt +
		 mptypcylr[CYL_CPRFLOOD][ln].new_cpkt +
		 mptypcylr[CYL_CPRRPF][ln].new_cpkt;
    aptr->escs = mptypcylr[CYL_CESC][ln].new_cpkt;
    aptr->sec = cystatscur.tv_sec - cystatsold.tv_sec;

    /* total over NINTV intervals */
    mptypcylr[CYL_ACHS][ln].new_cpktf = 0.0;
    mptypcylr[CYL_APKTS][ln].new_cpktf = 0.0;
    mptypcylr[CYL_ACHR][ln].new_cpktf = 0.0;
    mptypcylr[CYL_APKTR][ln].new_cpktf = 0.0;
    mptypcylr[CYL_APCTESC][ln].new_cpktf = 0.0;
    time = 0.0;

    for (j=0; j<NINTV; j++) {
        aptr = &avtab[ln][j];
        mptypcylr[CYL_ACHS][ln].new_cpktf += aptr->chs;
        mptypcylr[CYL_APKTS][ln].new_cpktf += aptr->pkts;
        mptypcylr[CYL_ACHR][ln].new_cpktf += aptr->chr;
        mptypcylr[CYL_APKTR][ln].new_cpktf += aptr->pktr;
	mptypcylr[CYL_APCTESC][ln].new_cpktf += aptr->escs;
	time += aptr->sec;
    }

    /* compute the avgs */
    pcpkt1 = &mptypcylr[CYL_ACHS][ln].new_cpktf;  /* total chars sent */
    pcpkt2 = &mptypcylr[CYL_APKTS][ln].new_cpktf; /* total pkts sent  */

    if (*pcpkt2 == 0.0)
        mptypcylr[CYL_ACHPKTS][ln].new_cpktf = 0.0;
    else
        mptypcylr[CYL_ACHPKTS][ln].new_cpktf = *pcpkt1 / *pcpkt2;

    if (time != 0.0)
        *pcpkt2 /= time;

    pcpkt2 = &mptypcylr[CYL_APCTESC][ln].new_cpktf; /* total esc chars sent */

    if (*pcpkt1 == 0.0)
        *pcpkt2 = 0.0;
    else
        *pcpkt2 = (*pcpkt2 / *pcpkt1) * 100.0;
        
    if (time != 0.0)
        *pcpkt1 /= time;


    pcpkt1 = &mptypcylr[CYL_ACHR][ln].new_cpktf;   /* total chars received */
    pcpkt2 = &mptypcylr[CYL_APKTR][ln].new_cpktf;  /* total pkts received  */

    if (*pcpkt2 == 0.0)
        mptypcylr[CYL_ACHPKTR][ln].new_cpktf = 0.0;
    else
        mptypcylr[CYL_ACHPKTR][ln].new_cpktf = *pcpkt1 / *pcpkt2;

    if (time != 0.0) {
        *pcpkt1 /= time;
        *pcpkt2 /= time;
    }
}

/*
 * readterminal -- read input from the user (terminal).
 */

readterminal()
{
    int cntbuf, done, i, j;

    wrefresh(wn_prompt);
    ioctl(0, FIONREAD, &cntbuf);
    if (cntbuf > 0) {
        done = FALSE;
        for (i=0, j=curpos; i<cntbuf && !done; i++) {
            sbrequest[j] = wgetch(wn_prompt);

            /* end of request */
            if (sbrequest[j] == '\n' || sbrequest[j] == '\r') {       
                sbrequest[j] = '\0';
                done = TRUE;
	    }

            /* backspace */
            else if (sbrequest[j] == '\b') 
                j--;

            /* <ctrl> l */
            else if (sbrequest[j] == '\f')  
                wrefresh(curscr);
      
            else {
                if (j < REQUESTSIZE - 1) j++;
            }
	}

        /* reprint request for user */
        curpos = j;
        if (curpos < 0) curpos = 0;
        wclear(wn_prompt);
        wprintw(wn_prompt, " ? ");
        for (i=0; i<curpos; i++) wprintw(wn_prompt, "%c", sbrequest[i]);
        wrefresh(wn_prompt);

        if (done) {
            curpos = 0;
            processrequest();
            wclear(wn_prompt);
            wprintw(wn_prompt, " ? ");
            wrefresh(wn_prompt);
	}
    }
    return;
}

/*
 * processrequest -- process the request read from the terminal.
 */

processrequest()
{
    int i, j, arg[MAXLINES];
    int fbadarg = FALSE;

    for (i=0; sbrequest[i] == ' ' || sbrequest[i] == '\t'; i++)
        /* null - skip leading white spaces */;

    switch (sbrequest[i]) {
        case 'l':

            /* get arguments  - allows easy addition of more arguments */

            for (j=0; j<MAXLINES; j++) arg[j] = NOTSET;
            j = 0;
	    i++;
            while ((sbrequest[i] != '\0') && (j < MAXLINES) && (!fbadarg)) {
                if (sbrequest[i] == ' ')
                    i++;
                else  {
                    arg[j] = sbrequest[i++] - '0';
                    if (arg[j] < 0 || arg[j] >= CLINEMAX)
                        fbadarg = TRUE;
                    j++;
		}
	    }
            if (fbadarg || arg[0] == NOTSET)
                (*rgwindow[HELP].pfc)();
            else
                (*rgwindow[LINE].pfc)(arg[0]);
            break;

        case 't':

            /* get arguments -- allows easy addition of more arguments */
            arg[0] = NOTSET;    /* default value, global totals */
            for (j=1; j<MAXLINES; j++) arg[j] = NOTSET;
            j = 0;
	    i++;
            while ((sbrequest[i] != '\0') && (j < MAXLINES) && (!fbadarg)) {
                if (sbrequest[i] == ' ')
                    i++;
                else  {
                    if (!isdigit(sbrequest[i]) ||
                        (arg[j] = sbrequest[i] - '0') < 0 ||
                         arg[j] >= CLINEMAX)
                        fbadarg = TRUE;
                    i++;
                    j++;
		}
	    }
            if (fbadarg)
                (*rgwindow[HELP].pfc)();
            else
                (*rgwindow[TOTAL].pfc)(arg[0]);
            break;

        case 'r':

            /* get arguments */
            arg[0] = TRUE;         /* default value is to reset */
            j = 0;
	    i++;
            while ((sbrequest[i] != '\0') && (j < 1) && (!fbadarg)) {
                if (sbrequest[i] == ' ')
                    i++;
                else if (sbrequest[i] == 'n' || sbrequest[i] == 'N') {
                    arg[j] = FALSE;
                    j++;
		}
                else if (sbrequest[i] == 'y' || sbrequest[i] == 'Y') {
                    arg[j] = TRUE;
                    j++;
		}
                else
                    fbadarg = TRUE;
	    }
            if (fbadarg)
                (*rgwindow[HELP].pfc)();
            else
                reset(arg[0]);
            break;

        case 'g':
            (*rgwindow[GLOBAL].pfc)();
            break;

        case 'q':
            ext();
            break;

        case 'h':
        case '?':
        default:
            (*rgwindow[HELP].pfc)();
            break;
    }
    return;
}

/*
 * ext -- called on normal exit to close up.
 */

ext()
{
    signal(SIGINT, SIG_IGN);
    close(fd);
    mvcur(0, COLS-1, LINES-1, 0);
    endwin();
    exit(0);
}

/*
 * die() -- called on error exit to print error msg and close up. 
 */

die(sberror)
char *sberror;
{
    wmove(wn_prompt, 0, 0);
    wprintw(wn_prompt, "%s", sberror);
    wclrtoeol(wn_prompt);
    wrefresh(wn_prompt);
    signal(SIGINT, SIG_IGN);
    close(fd);
    mvcur(0, COLS-1, LINES-1, 0);
    endwin();
    printf("\n");
    perror("cymon");
    exit(1);
}

/*
 * init -- initialize data structures for monitor program.
 */

init()
{
    int i, j;
    struct cystats *pold, *pcur, *prst;
    register struct ln *plnold, *plncur, *plnrst;
    struct timeval tv;
    struct timezone tz;
    int gyloc = 1;
    int lyloc = 1;
    struct avstats *aptr;

    pold = &cystatsold;
    pcur = &cystatscur;
    prst = &cystatsreset;
    plnold = cystatsold.rgln;
    plncur = cystatscur.rgln;
    plnrst = cystatsreset.rgln;
    fwindow = NOTSET;
    fline = NOTSET;
    freset = FALSE;
    newdata = 0;
/*    maxyloc = 24 - 6; */
    maxyloc = 24 - 4;

    if (gettimeofday(&tv, &tz) < 0)
        pcur->tv_sec = 0;
    else
        pcur->tv_sec = tv.tv_sec;

    pcur->avenrun[0] = 0;
    pcur->avenrun[1] = 0;
    pcur->avenrun[2] = 0;

    for (i=0; i<CPUSTATES; i++)
        pcur->cputime[i] = 0;
    
    prst->cyd_cipup = pcur->cyd_cipup = pold->cyd_cipup = 0;
    prst->cyd_cipln = pcur->cyd_cipln = pold->cyd_cipln = 0;
    prst->cyd_copln = pcur->cyd_copln = pold->cyd_copln = 0;
    prst->cyd_copup = pcur->cyd_copup = pold->cyd_copup = 0;
    prst->cyd_crint = pcur->cyd_crint = pold->cyd_crint = 0;

    for (i=0; i<CLINEMAX; i++, plncur++, plnold++, plnrst++) {

        /* Assume initially that all lines are cinfigured. Then if the */
        /* new data indicates a line is not configured the correct     */
        /* value will be saved in cystatsold.                          */
        plncur->cyl_ln = NOTCONF;
        plnrst->cyl_ln = plnold->cyl_ln = (char)i;

	plnrst->cyl_dest.s_addr = plncur->cyl_dest.s_addr
                       = plnold->cyl_dest.s_addr = 0L;
        plnrst->cyl_cpsdirect = plncur->cyl_cpsdirect
            =  plnold->cyl_cpsdirect = 0;
        plnrst->cyl_cpsNN = plncur->cyl_cpsNN
            =  plnold->cyl_cpsNN = 0;
        plnrst->cyl_cpsflood = plncur->cyl_cpsflood
            = plnold->cyl_cpsflood = 0;
        plnrst->cyl_cpsrpf = plncur->cyl_cpsrpf
            = plnold->cyl_cpsrpf = 0;
        plnrst->cyl_cprNN = plncur->cyl_cprNN
            = plnold->cyl_cprNN = 0;
        plnrst->cyl_cprdirect = plncur->cyl_cprdirect
            = plnold->cyl_cprdirect = 0;
        plnrst->cyl_cprflood = plncur->cyl_cprflood
            = plnold->cyl_cprflood = 0;
        plnrst->cyl_cprrpf = plncur->cyl_cprrpf
            = plnold->cyl_cprrpf = 0;
        plnrst->cyl_cchr = plncur->cyl_cchr
            = plnold->cyl_cchr = 0;
        plnrst->cyl_cchs = plncur->cyl_cchs
            = plnold->cyl_cchs = 0;
        plnrst->cyl_cpsip = plncur->cyl_cpsip
            = plnold->cyl_cpsip = 0;
        plnrst->cyl_cprip = plncur->cyl_cprip
            = plnold->cyl_cprip = 0;
        plnrst->cyl_csilo = plncur->cyl_csilo
            = plnold->cyl_csilo = 0;
        plnrst->cyl_flags = plncur->cyl_flags
            = plnold->cyl_flags = 0;
        plnrst->cyl_copdrop = plncur->cyl_copdrop
            = plnold->cyl_copdrop = 0;
        plnrst->cyl_ctpbusy = plncur->cyl_ctpbusy
            = plnold->cyl_ctpbusy = 0;
        plnrst->cyl_ctpidle = plncur->cyl_ctpidle
            = plnold->cyl_ctpidle = 0;
        plnrst->cyl_cesc = plncur->cyl_cesc
            = plnold->cyl_cesc = 0;
        plnrst->cyl_csend_ifqlen = plncur->cyl_csend_ifqlen 
            = plnold->cyl_csend_ifqlen = 0;

	/* initialize table to be used to compute avgs */
	for (j=0; j<NINTV; j++) {
	    aptr = &avtab[i][j];
	    aptr->chs = 0;
	    aptr->pkts = 0;
	    aptr->chr = 0;
	    aptr->pktr = 0;
	    aptr->escs = 0;
	    aptr->sec = 0;
	}
	strcpy(sbh_name[i], "UNINITIALIZED");
    }

    /* allocate storage for log record structures and assiign y locations */

    AllocGStor(CYD_CIPUP, gyloc++);
    AllocGStor(CYD_CIPLN, gyloc++);
    AllocGStor(CYD_COPUP, gyloc++);
    AllocGStor(CYD_COPLN, gyloc++);
    AllocGStor(CYD_CSILO, gyloc++);
    AllocGStor(CYD_ACHINTR, gyloc++);
    AllocGStor(CYD_CCHS, gyloc);
    AllocGStor(CYD_CCHR, gyloc++);

    AllocLStor(CYL_CPSNN, lyloc++);
    AllocLStor(CYL_CPSDIRECT, lyloc++);
    AllocLStor(CYL_CPSFLOOD, lyloc++);
    AllocLStor(CYL_CPSRPF, lyloc++);
    AllocLStor(CYL_CPRNN, lyloc++);
    AllocLStor(CYL_CPRDIRECT, lyloc++);
    AllocLStor(CYL_CPRFLOOD, lyloc++);
    AllocLStor(CYL_CPRRPF, lyloc++);
    AllocLStor(CYL_CPSIP, lyloc++);
    AllocLStor(CYL_CPRIP, lyloc++);
    AllocLStor(CYL_COPDROP, lyloc++);
    AllocLStor(CYL_CSEND_IFQLEN, lyloc++); 
    AllocLStor(CYL_CSILO, lyloc++);  

    /* char. count log records and avgs             */
    /* assign them the same yloc to be displayed on */
    AllocLStor(CYL_CTPBUSY, lyloc);
    AllocLStor(CYL_CTPIDLE, NOTSET);
    AllocLStor(CYL_APCTESC, lyloc++);
    AllocLStor(CYL_CESC, NOTSET);
    AllocLStor(CYL_CCHS, lyloc);
    AllocLStor(CYL_CCHR, lyloc++);
    AllocLStor(CYL_ACHPKTS, lyloc);
    AllocLStor(CYL_ACHPKTR, lyloc++);
    AllocLStor(CYL_ACHS, lyloc);
    AllocLStor(CYL_ACHR, lyloc++);
    AllocLStor(CYL_APKTS, lyloc);
    AllocLStor(CYL_APKTR, lyloc++);
    
}

/*
 *  AllocGStor -- allocate storage for global log records
 */

AllocGStor(i, yloc)
int i, yloc;
{
    register struct cylr *pcylr;

    if (yloc >= maxyloc) {
        printf("AllocGStor: screen not big enough for all the desired stats\n");
	exit(1);
    }
    pcylr = (struct cylr *)malloc(sizeof(*pcylr));
    if (pcylr == NULL) {
        printf("AllocGStor: can't malloc storage for log record");
	exit(1);
    }
    pcylr->yloc = yloc;
    pcylr->old_cpkt = 0;
    pcylr->new_cpkt = 0;
    pcylr->new_cpktf = 0.0;
    pcylr->ln = 0;
    pcylr->fwindow = GLOBAL;
    mptypcylr[i] = pcylr;
}

/*
 * AllocLStor -- allocate storage for line log records
 */

AllocLStor(i, yloc)
int i, yloc;
{
    register struct cylr *pcylr;
    register int j;

    if (yloc >= maxyloc) {
        printf("AllocLStor: screen not big enough for all the desired stats\n");
	exit(1);
    }
    pcylr = (struct cylr *)malloc(CLINEMAX * sizeof(*pcylr));
    if (pcylr == NULL) {
        printf("AllocLStor: can't malloc storage for log record");
	exit(1);
    }
    mptypcylr[i] = pcylr;
    for (j=0; j<CLINEMAX; j++, pcylr++) {
        pcylr->yloc = yloc;
        pcylr->old_cpkt = 0;
        pcylr->new_cpkt = 0;
	pcylr->new_cpktf = 0.0;
        pcylr->ln = j;
        pcylr->fwindow = LINE;
    }
}  

/*
 * removeplot -- blank-out (oldx - newx) stars from the plot at yloc.
 */

removeplot(oldx, newx, yloc)
int oldx, newx, yloc;
{
    int i;

    for (i=oldx; i>newx; i--)
        mvwaddch(wn_graph, yloc, i-1, ' ');
}

/*
 * addplot -- add (newx - oldx) stars to the plot at yloc.
 */

addplot(oldx, newx, yloc)
int oldx, newx, yloc;
{
    int i;

    for (i=oldx+1; i<=newx; i++)
        mvwaddch(wn_graph, yloc, i-1, '*');
}

/*
 * printstartup -- prints information to start the monitor program
 */

printstartup()
{
    printf("Usage: cymon [[-g] or [-l ln#] or [-t [ln#]]] [cypress site name]\n");
    exit(1);
}

/*
 * setdefaults -- sets default monitor program options
 */

setdefaults()
{
    sbsite = "localhost";
    return;
}

/*
 * isglobal -- returns TRUE if its argument is a global type
 */

isglobal(cyl_type)
int cyl_type;
{
    switch (cyl_type) {
        case CYD_CIPUP       :
        case CYD_CIPLN       :
        case CYD_COPUP       :
        case CYD_COPLN       :
        case CYD_CSILO       :
        case CYD_CCHS        :
        case CYD_CCHR        :
	case CYD_ACHINTR     :
            return(TRUE);
            break;
    }
    return(FALSE);
}

/*
 * iserror -- returns TRUE if its argument is an error type
 */

iserror(cyl_type)
int cyl_type;
{
    return(FALSE);
}

/*
 * isline -- returns TRUE if its argument is a line type
 */

isline(cyl_type)
int cyl_type;
{
    switch (cyl_type) {
        case CYL_CPSNN        :
        case CYL_CPSDIRECT        :
        case CYL_CPSFLOOD         :
        case CYL_CPSRPF           :
        case CYL_CPRNN        :
        case CYL_CPRDIRECT        :
        case CYL_CPRFLOOD         :
        case CYL_CPRRPF           :
        case CYL_CPSIP            :
        case CYL_CPRIP            :
        case CYL_COPDROP          :
        case CYL_CSEND_IFQLEN     :
        case CYL_CSILO            :
        case CYL_CTPBUSY          :
        case CYL_CTPIDLE          :
        case CYL_CCHS             :
        case CYL_CCHR             :
	case CYL_ACHS		  :
	case CYL_APKTS		  :
	case CYL_ACHPKTS	  :
	case CYL_ACHR		  :
	case CYL_APKTR            :
	case CYL_ACHPKTR          :
        case CYL_APCTESC	  :
        case CYL_CESC             :
            return(TRUE);
            break;
    }
    return(FALSE);
}

/*
 * globalstats -- set-up global statistics window for monitoring
 */

globalstats()
{
    int cyloc = 0;             /* clear yloc to 0 */

    /* window already global, don't change it */
    if (fwindow == GLOBAL)
        return;

    fwindow = GLOBAL;      /* set window to global */
    wclear(wn_help);
    wclear(wn_types);      /* clear windows */
    wclear(wn_graph);
    wrefresh(wn_help);

    /* print title and scale */
    wmove(wn_types, cyloc, 0);
    wprintw(wn_types, "%s", rgwindow[GLOBAL].sbTitle);
    mvwaddch(wn_types, cyloc, 29, ':');
    wmove(wn_graph, cyloc++, 0);
    wprintw(wn_graph, "1        10        20        30        40");

    /* display title for the global log record types */
    InitDisplay(CYD_CIPUP, 0);
    InitDisplay(CYD_CIPLN, 0);
    InitDisplay(CYD_COPUP, 0);
    InitDisplay(CYD_COPLN, 0);
    InitDisplay(CYD_CSILO, 0);
    InitDisplay(CYD_ACHINTR, 0);
    InitDisplay(CYD_CCHS, 0);
    InitDisplay(CYD_CCHR, 0);
    wrefresh(wn_types);
    wrefresh(wn_graph);

    /* update the graphs for the global log records */
    UpdateGraphs();
}

/*
 * linestats -- set-up line statistics window for monitoring.
 *     A maximum of 1 line can be monitored at once.
 */

linestats(ln)
int ln;
{
    int cyloc = 0;             /* clear yloc to 0 */

    /* window does not need to be changed */
    if (fwindow == LINE && fline == ln)
        return;

    /* line not configured so don't plot */
    if (cystatsold.rgln[ln].cyl_ln == NOTCONF) {
        LineNotConf(ln);
        return;
    }

    fwindow = LINE;        /* set window to line */
    fline = ln;            /* set line being monitored */
    wclear(wn_help);
    wclear(wn_types);      /* clear windows */
    wclear(wn_graph);
    wrefresh(wn_help);

    /* print title and scale if the line is configured */
    wmove(wn_types, cyloc, 0);


    /* line is configured */
    wprintw(wn_types, "%s TO %-.20s", rgwindow[LINE].sbTitle, sbh_name[ln]);
    mvwaddch(wn_types, cyloc, 29, ':');
    wmove(wn_graph, cyloc++, 0);
    wprintw(wn_graph, "1        10        20        30        40");

    /* display titles for the line log record types */
    InitDisplay(CYL_CPSNN, ln);
    InitDisplay(CYL_CPSDIRECT, ln);
    InitDisplay(CYL_CPSFLOOD, ln);
    InitDisplay(CYL_CPSRPF, ln);
    InitDisplay(CYL_CPRNN, ln);
    InitDisplay(CYL_CPRDIRECT, ln);
    InitDisplay(CYL_CPRFLOOD, ln);
    InitDisplay(CYL_CPRRPF, ln);
    InitDisplay(CYL_CPSIP, ln);
    InitDisplay(CYL_CPRIP, ln);
    InitDisplay(CYL_COPDROP, ln);
    InitDisplay(CYL_CSEND_IFQLEN, ln);
    InitDisplay(CYL_CSILO, ln);
    InitDisplay(CYL_CTPBUSY, ln);
    InitDisplay(CYL_APCTESC, ln);
    InitDisplay(CYL_CCHS, ln);
    InitDisplay(CYL_CCHR, ln);
    InitDisplay(CYL_ACHPKTS, ln);
    InitDisplay(CYL_ACHPKTR, ln);
    InitDisplay(CYL_ACHS, ln);
    InitDisplay(CYL_ACHR, ln);
    InitDisplay(CYL_APKTS, ln);
    InitDisplay(CYL_APKTR, ln);

    wrefresh(wn_types);
    wrefresh(wn_graph);

    /* update the graphs for the line log records */
    UpdateGraphs();
}

/*
 * totalstats -- set-up total statistics window for monitoring. A maximum of
 *     one line's totals or the global totals can be monitored at once.
 */

totalstats(ln)
int ln;
{
    int cyloc = 0;             /* clear yloc to 0 */

    /* window does not need to be changed */
    if (fwindow == TOTAL && fline == ln)
        return;

    /* line not configured so don't plot */
    if (ln != NOTSET && cystatsold.rgln[ln].cyl_ln == NOTCONF) {
        LineNotConf(ln);
        return;
    }

    fwindow = TOTAL;        /* set window to total */
    fline = ln;             /* set line being monitored */
    wclear(wn_help);
    wclear(wn_types);       /* clear windows */
    wclear(wn_graph);
    wrefresh(wn_help);

    /* print title and scale if the line is */
    /* configured or if global totals       */
    wmove(wn_types, cyloc, 0);

    if (ln == NOTSET)      /* global totals */
        wprintw(wn_types, "GLOBAL %s", rgwindow[TOTAL].sbTitle);
    else                   /* line ln totals */
        wprintw(wn_types, "LINE %s TO %-.14s",
                rgwindow[TOTAL].sbTitle, sbh_name[ln]);
    mvwaddch(wn_types, cyloc, 29, ':');
    wmove(wn_graph, cyloc++, 0);
    wprintw(wn_graph, " TOTAL COUNTS");

    /* display titles for global log record types */
    if (fline == NOTSET) {
        InitDisplay(CYD_CIPUP, 0);
        InitDisplay(CYD_CIPLN, 0);
        InitDisplay(CYD_COPUP, 0);
        InitDisplay(CYD_COPLN, 0);
        InitDisplay(CYD_CSILO, 0);
	InitDisplay(CYD_ACHINTR, 0);
        InitDisplay(CYD_CCHS, 0);
        InitDisplay(CYD_CCHR, 0);
    }

    /* display titles for the line log record types */
    else  {
        InitDisplay(CYL_CPSNN, ln);
        InitDisplay(CYL_CPSDIRECT, ln);
        InitDisplay(CYL_CPSFLOOD, ln);
        InitDisplay(CYL_CPSRPF, ln);
        InitDisplay(CYL_CPRNN, ln);
        InitDisplay(CYL_CPRDIRECT, ln);
        InitDisplay(CYL_CPRFLOOD, ln);
        InitDisplay(CYL_CPRRPF, ln);
        InitDisplay(CYL_CPSIP, ln);
        InitDisplay(CYL_CPRIP, ln);
        InitDisplay(CYL_COPDROP, ln);
        InitDisplay(CYL_CSEND_IFQLEN, ln);
        InitDisplay(CYL_CSILO, ln);
        InitDisplay(CYL_CTPBUSY, ln);
        InitDisplay(CYL_APCTESC, ln);
        InitDisplay(CYL_CCHS, ln);
        InitDisplay(CYL_CCHR, ln);
        InitDisplay(CYL_ACHPKTS, ln);
        InitDisplay(CYL_ACHPKTR, ln);
    }

    wrefresh(wn_types);
    wrefresh(wn_graph);

    /* update the total counts for the log records */
    UpdateTotals();
}

/*
 * reset -- if flag is TRUE, reset totals to 0 and start accumulating,
 *     if flag is FALSE, use real totals as sent from monitored site.
 */
 
reset(flag)
int flag;
{
    int i;
    register struct cystats *pcy;
    register struct ln *pln;

    freset = flag;
    if (freset) {
        pcy = &cystatsreset;
        pcy->cyd_cipup = 0;
        pcy->cyd_cipln = 0;
        pcy->cyd_copup = 0;
        pcy->cyd_copln = 0;
        pcy->cyd_crint = 0;
        pln = cystatsreset.rgln;
        for (i=0; i<CLINEMAX; i++, pln++) {
            pln->cyl_cpsNN = 0;
            pln->cyl_cpsdirect = 0;
            pln->cyl_cpsflood = 0;
            pln->cyl_cpsrpf = 0;
            pln->cyl_cprNN = 0;
            pln->cyl_cprdirect = 0;
            pln->cyl_cprflood = 0;
            pln->cyl_cprrpf = 0;
            pln->cyl_cchr = 0;
            pln->cyl_cchs = 0;
            pln->cyl_cpsip = 0;
            pln->cyl_cprip = 0;
            pln->cyl_csilo = 0;
            pln->cyl_copdrop = 0;
            pln->cyl_ctpbusy = 0;
            pln->cyl_ctpidle = 0;
	    pln->cyl_cesc = 0;
            pln->cyl_csend_ifqlen = 0;
	}
    }
    if (fwindow == TOTAL)
        UpdateTotals();
}
            
/*
 * helpwindow -- provides a help function, displays the windows
 *     that are available.
 */

helpwindow()
{
    int i;
    int cyloc = 0;

    fwindow = HELP;       /* set window to help */
    wclear(wn_help);
    wclear(wn_types);     /* clear windows */
    wclear(wn_graph);
    wrefresh(wn_types);
    wrefresh(wn_graph);

    /*print title and options */
    wmove(wn_help, cyloc++, 0);
    wprintw(wn_help, "%s", rgwindow[HELP].sbTitle);
    wmove(wn_help,++cyloc, 0);
    for (i=0; i<MAXWINDOWS; i++) 
        wprintw(wn_help, "%s\n", rgwindow[i].sbDescription);
    wprintw(wn_help, "r - reset totals, followed by y or nothing use reset totals, followed\n\tby n use totals since last reboot. Reset totals are only displayed\n\tif one of the total options is selected.\n\n");
    wprintw(wn_help, "q - exit the monitor program\n\n");
    wprintw(wn_help, "<ctrl> l - redraw the screen");
    wrefresh(wn_help);
    return;
}

/*
 * LineNotConf -- clears the screen and prints a msg if the line to be
 *     monitored is not configured.
 */

LineNotConf(ln)
int ln;
{
    wclear(wn_help);
    wclear(wn_graph);
    wclear(wn_types);
    wrefresh(wn_help);
    wmove(wn_types, 0, 0);
    wprintw(wn_types, "SERIAL LINE %d NOT CONFIGURED", ln);
    fwindow = HELP;
    wrefresh(wn_types);
    wrefresh(wn_graph);
    return;
}

/*
 * InitDisplay -- initializes one type of log record's data structure
 *     for the line to be displayed and display the title.
 */

InitDisplay(cyl_type, cyl_ln)
int cyl_type, cyl_ln;
{
    register struct cylr *pcylr;

    pcylr = &mptypcylr[cyl_type][cyl_ln];
    pcylr->old_cpkt = 0;

    switch (cyl_type) {
        case CYL_CTPBUSY:
        case CYL_CCHS:
        case CYD_CCHS:
	case CYL_ACHS:
	case CYL_APKTS:
	case CYL_ACHPKTS:
            wmove(wn_types, pcylr->yloc, 0);
            wprintw(wn_types, "%s", mptysb[cyl_type]);
            break;
        case CYL_APCTESC:
        case CYL_CCHR:
        case CYD_CCHR:
	case CYL_ACHR:
	case CYL_APKTR:
	case CYL_ACHPKTR:
            wmove(wn_graph, pcylr->yloc, 0);
            wprintw(wn_graph, "%s", mptysb[cyl_type]);
            break;
        default:
            wmove(wn_types, pcylr->yloc, 0);
            wprintw(wn_types, "%s", mptysb[cyl_type]);
            mvwaddch(wn_types, pcylr->yloc, 29, ':');
            break;
    }
}

/*
 * UpdateStatus -- update the system status lines
 */

UpdateStatus()
{
    float pct[CPUSTATES];
    int i;
    int w = 0;

    /* sum up the time spent in each cpu state and initialize pct */
    for (i=0; i<CPUSTATES; i++) {
        pct[i] = 0.0;
        w += cystatscur.cputime[i];
    }

    /* compute the percent of time spent in each cpu state */
    if (w != 0) {
        for (i=0; i<CPUSTATES; i++)
            pct[i] = ((float)cystatscur.cputime[i] / (float)w) * 100.0;
    }

    wmove(wn_title, 2, 26);
    wprintw(wn_title, "%6.2f", pct[0]);
    wmove(wn_title, 2, 42);
    wprintw(wn_title, "%6.2f", pct[2]);
    wmove(wn_title, 2, 56);
    wprintw(wn_title, "%6.2f", pct[1]);
    wmove(wn_title, 2, 70);
    wprintw(wn_title, "%6.2f", pct[3]);
    wmove(wn_title, 3, 37);
    wprintw(wn_title, "%5.2f", ((float)cystatscur.avenrun[0])/100.);
    wmove(wn_title, 3, 44);
    wprintw(wn_title, "%5.2f", ((float)cystatscur.avenrun[1])/100.);
    wmove(wn_title, 3, 51);
    wprintw(wn_title, "%5.2f", ((float) cystatscur.avenrun[2])/100.);
}

/*
 * UpdateGraphs -- update the plots in the active window. Does not
 *     update the old_cpkt of any log record not being displayed or
 *     that prints a floating point number.
 */

UpdateGraphs()
{
    struct cylr *pcylr, *pcylr2;

    if (fwindow == GLOBAL) {     /* update all global plots */
        UpdatePlot(mptypcylr[CYD_CIPUP]); 
        UpdatePlot(mptypcylr[CYD_CIPLN]); 
        UpdatePlot(mptypcylr[CYD_COPUP]); 
        UpdatePlot(mptypcylr[CYD_COPLN]); 
        UpdatePlot(mptypcylr[CYD_CSILO]);

        pcylr = mptypcylr[CYD_CCHS];
        PrintNum(wn_types, pcylr->new_cpkt, pcylr->yloc, 17);
        pcylr->old_cpkt = pcylr->new_cpkt;

        pcylr = mptypcylr[CYD_CCHR];
        PrintNum(wn_graph, pcylr->new_cpkt, pcylr->yloc, 21);
        pcylr->old_cpkt = pcylr->new_cpkt;

	pcylr2 = mptypcylr[CYD_ACHINTR];
	if (pcylr2->new_cpktf != 0.0)
	    pcylr2->new_cpktf = (float)pcylr->new_cpkt / pcylr2->new_cpktf;
	PrintNumf(wn_graph, pcylr2->new_cpktf, pcylr2->yloc, 0);
    }

    else if (fwindow == LINE) {  /* update the displayed line's stats */
        wmove(wn_types, 0, 8);
	wclrtoeol(wn_types);
	wmove(wn_types, 0, 8);
	wprintw(wn_types, "%-.20s", sbh_name[fline]);
	mvwaddch(wn_types, 0, 29, ':');
        UpdatePlot(&(mptypcylr[CYL_CPSNN][fline]));
        UpdatePlot(&(mptypcylr[CYL_CPSDIRECT][fline]));
        UpdatePlot(&(mptypcylr[CYL_CPSFLOOD][fline]));
        UpdatePlot(&(mptypcylr[CYL_CPSRPF][fline]));
        UpdatePlot(&(mptypcylr[CYL_CPRNN][fline]));
        UpdatePlot(&(mptypcylr[CYL_CPRDIRECT][fline]));
        UpdatePlot(&(mptypcylr[CYL_CPRFLOOD][fline]));
        UpdatePlot(&(mptypcylr[CYL_CPRRPF][fline]));
        UpdatePlot(&(mptypcylr[CYL_CPSIP][fline]));
        UpdatePlot(&(mptypcylr[CYL_CPRIP][fline]));
        UpdatePlot(&(mptypcylr[CYL_COPDROP][fline]));
        UpdatePlot(&(mptypcylr[CYL_CSEND_IFQLEN][fline]));
        UpdatePlot(&(mptypcylr[CYL_CSILO][fline]));
/*        PrintPct(&(mptypcylr[CYL_CTPBUSY][fline]));*/
/*	PrintPct(&(mptypcylr[CYL_APCTESC][fline]));*/

        pcylr = &mptypcylr[CYL_CTPBUSY][fline];
        PrintPct(wn_types, pcylr->new_cpktf, pcylr->yloc, 19);

        pcylr = &mptypcylr[CYL_CCHS][fline];
        PrintNum(wn_types, pcylr->new_cpkt, pcylr->yloc, 17);
        pcylr->old_cpkt = pcylr->new_cpkt;

        pcylr = &mptypcylr[CYL_ACHPKTS][fline];
        PrintNumf(wn_types, pcylr->new_cpktf, pcylr->yloc, 19);

        pcylr = &mptypcylr[CYL_ACHS][fline];
        PrintNumf(wn_types, pcylr->new_cpktf, pcylr->yloc, 19);

        pcylr = &mptypcylr[CYL_APKTS][fline];
        PrintNumf(wn_types, pcylr->new_cpktf, pcylr->yloc, 19);

        pcylr = &mptypcylr[CYL_APCTESC][fline];
        PrintPct(wn_graph, pcylr->new_cpktf, pcylr->yloc, 28);

        pcylr = &mptypcylr[CYL_CCHR][fline];
        PrintNum(wn_graph, pcylr->new_cpkt, pcylr->yloc, 26);
        pcylr->old_cpkt = pcylr->new_cpkt;

        pcylr = &mptypcylr[CYL_ACHPKTR][fline];
        PrintNumf(wn_graph, pcylr->new_cpktf, pcylr->yloc, 28);

        pcylr = &mptypcylr[CYL_ACHR][fline];
        PrintNumf(wn_graph, pcylr->new_cpktf, pcylr->yloc, 28);

        pcylr = &mptypcylr[CYL_APKTR][fline];
        PrintNumf(wn_graph, pcylr->new_cpktf, pcylr->yloc, 28);
    }
    wrefresh(wn_types);
    wrefresh(wn_graph);
}

/*
 * UpdateTotals -- update the total counts being displayed
 */

UpdateTotals()
{
    int i, yloc;
    register struct ln *pln;
    float ttysum = 0.0,
          apktsz = 0.0;
    long totchr = 0,
         totchs = 0,
	 pktsum = 0,
	 totsilo = 0;
    register struct cystats *pcystats;

    if (freset)
        pcystats = &cystatsreset;	/* contains reset totals */
    else
        pcystats = &cystatsold;		/* contains totals since */
                                        /* the last reboot       */

    if (fline == NOTSET) {  /* global totals */
        PrintNum(wn_graph, pcystats->cyd_cipup,
                   mptypcylr[CYD_CIPUP]->yloc, 0);
        PrintNum(wn_graph, pcystats->cyd_cipln,
                   mptypcylr[CYD_CIPLN]->yloc, 0);
        PrintNum(wn_graph, pcystats->cyd_copup,
                   mptypcylr[CYD_COPUP]->yloc, 0);
        PrintNum(wn_graph, pcystats->cyd_copln,
                   mptypcylr[CYD_COPLN]->yloc, 0);

        /* total chars received and sent and total silo overflows */
        for (i=0; i<CLINEMAX; i++) {
            totchs += pcystats->rgln[i].cyl_cchs;
            totchr += pcystats->rgln[i].cyl_cchr;
	    totsilo += pcystats->rgln[i].cyl_csilo;
	}
	PrintNum(wn_graph, totsilo, mptypcylr[CYD_CSILO]->yloc, 0);
        if (pcystats->cyd_crint != 0)
	    PrintNumf(wn_graph, ((float)totchr / (float)pcystats->cyd_crint),
                      mptypcylr[CYD_ACHINTR]->yloc, 0);
        else
	    PrintNumf(wn_graph, 0.0, mptypcylr[CYD_ACHINTR]->yloc, 0);
        PrintNum(wn_types, totchs, mptypcylr[CYD_CCHS]->yloc, 17);
        PrintNum(wn_graph, totchr, mptypcylr[CYD_CCHR]->yloc, 21);
    }

    else {                  /* displayed line totals */
        wmove(wn_types, 0, 15);
	wclrtoeol(wn_types);
        wmove(wn_types, 0, 15);
	wprintw(wn_types, "%-.14s", sbh_name[fline]);
	mvwaddch(wn_types, 0, 29, ':');
        pln = &(pcystats->rgln[fline]);
        PrintNum(wn_graph, pln->cyl_cpsNN,
                   mptypcylr[CYL_CPSNN][fline].yloc, 0);
        PrintNum(wn_graph, pln->cyl_cpsdirect,
                   mptypcylr[CYL_CPSDIRECT][fline].yloc, 0);
        PrintNum(wn_graph, pln->cyl_cpsflood,
                   mptypcylr[CYL_CPSFLOOD][fline].yloc, 0);
        PrintNum(wn_graph, pln->cyl_cpsrpf,
                   mptypcylr[CYL_CPSRPF][fline].yloc, 0);
        PrintNum(wn_graph, pln->cyl_cprNN,
                   mptypcylr[CYL_CPRNN][fline].yloc, 0);
        PrintNum(wn_graph, pln->cyl_cprdirect,
                   mptypcylr[CYL_CPRDIRECT][fline].yloc, 0);
        PrintNum(wn_graph, pln->cyl_cprflood,
                   mptypcylr[CYL_CPRFLOOD][fline].yloc, 0);
        PrintNum(wn_graph, pln->cyl_cprrpf,
                   mptypcylr[CYL_CPRRPF][fline].yloc, 0);
        PrintNum(wn_graph, pln->cyl_cpsip,
                   mptypcylr[CYL_CPSIP][fline].yloc, 0);
        PrintNum(wn_graph, pln->cyl_cprip,
                   mptypcylr[CYL_CPRIP][fline].yloc, 0);
        PrintNum(wn_graph, pln->cyl_copdrop,
                   mptypcylr[CYL_COPDROP][fline].yloc, 0);
        PrintNum(wn_graph, pln->cyl_csend_ifqlen,
                   mptypcylr[CYL_CSEND_IFQLEN][fline].yloc, 0);
        PrintNum(wn_graph, pln->cyl_csilo,
                   mptypcylr[CYL_CSILO][fline].yloc, 0);
#ifdef notdef
        /* total percent tty busy */
        ttysum = (float)(pln->cyl_ctpbusy + pln->cyl_ctpidle);
        yloc = mptypcylr[CYL_CTPBUSY][fline].yloc;
        wmove(wn_graph, yloc, 0);
        wclrtoeol(wn_graph);
        wmove(wn_graph, yloc, 0);
        if (ttysum != 0.0)
            wprintw(wn_graph, "%10.5f%%",
              ((float)pln->cyl_ctpbusy / (float)ttysum) * 100.0);
	else
	    wprintw(wn_graph, "%10.5f%%", ttysum);
#endif notdef
        /* total percent tty busy */
        ttysum = (float)(pln->cyl_ctpbusy + pln->cyl_ctpidle);
        if (ttysum != 0.0)
	    PrintPct(wn_types, ((float)pln->cyl_ctpbusy / (float)ttysum) * 100.0,
		 mptypcylr[CYL_CTPBUSY][fline].yloc, 19);
	else
	    PrintPct(wn_types, 0.0, mptypcylr[CYL_CTPBUSY][fline].yloc, 19);

#ifdef notdef
	/* percent escape chars sent */
	yloc = mptypcylr[CYL_APCTESC][fline].yloc;
	wmove(wn_graph, yloc, 0);
	wclrtoeol(wn_graph);
	wmove(wn_graph, yloc, 0);
        if (pln->cyl_cchs != 0)
	    wprintw(wn_graph, "%10.5f%%",
	      ((float)pln->cyl_cesc / (float)pln->cyl_cchs) * 100.0);
	else
	    wprintw(wn_graph, "%10.5f%%", (float)pln->cyl_cchs);

#endif
	/* percent escape chars sent */
	if (pln->cyl_cchs != 0)
	    PrintPct(wn_graph, 
		     ((float)pln->cyl_cesc/(float)pln->cyl_cchs) * 100.0, 
		     mptypcylr[CYL_APCTESC][fline].yloc, 28);
	else
	    PrintPct(wn_graph, 0.0, mptypcylr[CYL_APCTESC][fline].yloc, 28);

	/* characters sent and received */
	PrintNum(wn_types, pln->cyl_cchs,
                   mptypcylr[CYL_CCHS][fline].yloc, 17);
        PrintNum(wn_graph, pln->cyl_cchr,
                   mptypcylr[CYL_CCHR][fline].yloc, 26);

	/* avg packet size */
        pktsum = pln->cyl_cpsdirect + 
	    pln->cyl_cpsflood + 
	    pln->cyl_cpsrpf
	    + pln->cyl_cpsNN;
	if (pktsum == 0)
	    apktsz = 0.0;
	else
            apktsz = (float)pln->cyl_cchs / (float)pktsum;
	PrintNumf(wn_types, apktsz, mptypcylr[CYL_ACHPKTS][fline].yloc, 19);
	pktsum = pln->cyl_cprdirect + 
	    pln->cyl_cprflood + 
	    pln->cyl_cprNN
	    + pln->cyl_cprrpf;
        if (pktsum == 0)
	    apktsz = 0.0;
	else
            apktsz = (float)pln->cyl_cchr / (float)pktsum;
	PrintNumf(wn_graph, apktsz, mptypcylr[CYL_ACHPKTR][fline].yloc, 28);
				/* average chars sent per sec */
    }
    wrefresh(wn_types);
    wrefresh(wn_graph);
}

/*
 * PrintNum -- print the number in the specified window at the specified
 *     yloc and xloc.
 */

PrintNum(wn, num, yloc, xloc)
WINDOW *wn;
long num;
int yloc, xloc;
{
    wmove(wn, yloc, xloc);
    wclrtoeol(wn);
    wmove(wn, yloc, xloc);
    wprintw(wn, "%10d", num);
}
/*
 * PrintNumf -- print the float in the specified window at the specified
 *     yloc and xloc.
 */

PrintNumf(wn, num, yloc, xloc)
WINDOW *wn;
float num;
int yloc, xloc;
{
    wmove(wn, yloc, xloc);
    wclrtoeol(wn);
    wmove(wn, yloc, xloc);
    wprintw(wn, "%8.2f  ", num);
}
    
/*
 * PrintPct -- prints the percent of time a tty is busy.
 */
PrintPct(wn, num, yloc, xloc)
WINDOW *wn;
float num;
int yloc, xloc;
{
    wmove(wn, yloc, xloc);
    wclrtoeol(wn);
    wmove(wn, yloc, xloc);
    wprintw(wn, "%8.2f %%", num);
}

/*
 * PrintError -- Print an error msg.
 */

PrintError(sb)
char *sb;
{
    wmove(wn_prompt, 0, 0);
    wprintw(wn_prompt, "%s", sb);
    wrefresh(wn_prompt);
}

/*
 * UpdatePlot -- update the plot for the log record passed in.
 */

UpdatePlot(pcylr)
struct cylr *pcylr;
{

   if (pcylr->new_cpkt == pcylr->old_cpkt)
       return;

   else if (pcylr->new_cpkt < 0) {
       wmove(wn_graph, pcylr->yloc, 0);
       wclrtoeol(wn_graph);
       pcylr->new_cpkt = 0;
       pcylr->old_cpkt = 0;
   }

   else if (pcylr->new_cpkt > MAXPLOTS && pcylr->old_cpkt > MAXPLOTS) {
       PrintNum(wn_graph, pcylr->new_cpkt, pcylr->yloc, 0);
       pcylr->old_cpkt = pcylr->new_cpkt;
   }

   else if (pcylr->new_cpkt > MAXPLOTS && pcylr->old_cpkt <= MAXPLOTS) {
       PrintNum(wn_graph, pcylr->new_cpkt, pcylr->yloc, 0);
       pcylr->old_cpkt = pcylr->new_cpkt;
   }

   else if (pcylr->new_cpkt <= MAXPLOTS && pcylr->old_cpkt > MAXPLOTS) {
       wmove(wn_graph, pcylr->yloc, 0);
       wclrtoeol(wn_graph);
       addplot(0, pcylr->new_cpkt, pcylr->yloc);
       pcylr->old_cpkt = pcylr->new_cpkt;
   }

   else if (pcylr->new_cpkt > pcylr->old_cpkt) {

       /* add to old graph */
       addplot(pcylr->old_cpkt, pcylr->new_cpkt, pcylr->yloc);
       pcylr->old_cpkt = pcylr->new_cpkt;
   }

   else if (pcylr->new_cpkt < pcylr->old_cpkt) {

       /* remove from old graph */
       removeplot(pcylr->old_cpkt, pcylr->new_cpkt, pcylr->yloc);
       pcylr->old_cpkt = pcylr->new_cpkt;
   }
}

/*
 * ComputeTTYPct -- compute tty percents per line
 */

ComputeTTYPct()
{
    float ttysum;                     /* sum of tty new counts */
    float pct_ttybusy, pct_ttyidle;   /* percent of time tty busy or idle */
    int i;
    register struct cylr *pcylr_busy, *pcylr_idle; /* ptrs to tty log records */
    pcylr_busy = mptypcylr[CYL_CTPBUSY];
    pcylr_idle = mptypcylr[CYL_CTPIDLE];

    for (i=0; i<CLINEMAX; i++, pcylr_busy++, pcylr_idle++) {
        ttysum = pcylr_busy->new_cpktf + pcylr_idle->new_cpktf;
        if (ttysum == 0.0)
            continue;
        pct_ttybusy = (pcylr_busy->new_cpktf / ttysum) * 100.0;
        pct_ttyidle = (pcylr_idle->new_cpktf / ttysum) * 100.0;
        pcylr_busy->new_cpktf = pct_ttybusy;
        pcylr_idle->new_cpktf = pct_ttyidle;

    }
}
	
/*
 * ========================================================================
 * FromNetworkOrder - Convert fields in struct to host order
 * ========================================================================
 */

FromNetworkOrder(pcystats)
register struct cystats *pcystats;
{
    register int i;
    register struct ln *pcyln;
    pcystats->tv_sec = ntohl(pcystats->tv_sec);
    for (i=0; i<3; i++)
	pcystats->avenrun[i] = ntohl(pcystats->avenrun[i]);
    for (i=0; i<CPUSTATES; i++) 
	pcystats->cputime[i] = ntohl(pcystats->cputime[i]);
    pcystats->cyd_cipup = ntohl(pcystats->cyd_cipup);
    pcystats->cyd_cipln = ntohl(pcystats->cyd_cipln);
    pcystats->cyd_copln = ntohl(pcystats->cyd_copln);
    pcystats->cyd_copup = ntohl(pcystats->cyd_copup);
    pcystats->cyd_crint = ntohl(pcystats->cyd_crint);
    for (pcyln = pcystats->rgln; pcyln < &pcystats->rgln[CLINEMAX]; 
	 pcyln++) {
	pcyln->cyl_cpsNN = ntohl(pcyln->cyl_cpsNN);
	pcyln->cyl_cpsdirect = ntohl(pcyln->cyl_cpsdirect);
	pcyln->cyl_cpsflood = ntohl(pcyln->cyl_cpsflood);
	pcyln->cyl_cpsrpf = ntohl(pcyln->cyl_cpsrpf);
	pcyln->cyl_cprNN = ntohl(pcyln->cyl_cprNN);
	pcyln->cyl_cprdirect = ntohl(pcyln->cyl_cprdirect);
	pcyln->cyl_cprflood = ntohl(pcyln->cyl_cprflood);
	pcyln->cyl_cprrpf = ntohl(pcyln->cyl_cprrpf);
	pcyln->cyl_cchr = ntohl(pcyln->cyl_cchr);
	pcyln->cyl_cchs = ntohl(pcyln->cyl_cchs);
	pcyln->cyl_cpsip = ntohl(pcyln->cyl_cpsip);
	pcyln->cyl_cprip = ntohl(pcyln->cyl_cprip);
	pcyln->cyl_csilo = ntohl(pcyln->cyl_csilo);
	pcyln->cyl_flags = ntohl(pcyln->cyl_flags);
	pcyln->cyl_copdrop = ntohl(pcyln->cyl_copdrop);
	pcyln->cyl_ctpbusy = ntohl(pcyln->cyl_ctpbusy);
	pcyln->cyl_ctpidle = ntohl(pcyln->cyl_ctpidle);
	pcyln->cyl_cesc = ntohl(pcyln->cyl_cesc);
	pcyln->cyl_csend_ifqlen = ntohl(pcyln->cyl_csend_ifqlen);
	pcyln->cyl_dest.s_addr = ntohl(pcyln->cyl_dest.s_addr);
	pcyln->cyl_ln = ntohl(pcyln->cyl_ln);
    }
}
