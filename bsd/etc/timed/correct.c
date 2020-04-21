/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)correct.c	2.3 (Berkeley) 5/28/86";
#endif not lint

#include "globals.h"
#include <protocols/timed.h>

#ifdef MEASURE
extern FILE *fp;
#endif

/* 
 * `correct' sends to the slaves the corrections for their clocks
 */

correct(avdelta)
long avdelta;
{
	int i;
	int corr;
	struct timeval adjlocal;
	struct tsp msgs;
	struct timeval mstotvround();
	struct tsp *answer, *acksend();

#ifdef MEASURE
	for(i=0; i<slvcount; i++) {
		if (hp[i].delta == HOSTDOWN)
			fprintf(fp, "%s\t", "down");
		else { 
			fprintf(fp, "%d\t", hp[i].delta);
		}
	}
	fprintf(fp, "\n");
#endif
	corr = avdelta - hp[0].delta;
	adjlocal = mstotvround(&corr);
	adjclock(&adjlocal);
#ifdef MEASURE
	fprintf(fp, "%d\t", corr);
#endif

	for(i=1; i<slvcount; i++) {
		if (hp[i].delta != HOSTDOWN)  {
			corr = avdelta - hp[i].delta;
#if defined(sgi) || defined(sgi_vax)
/* If the other machine is off in the weeds, set its time directly.
 * 	If a slave gets the wrong day, the BSD code would simply
 *	fix the minutes.  If you fix a network partition, you can get
 *	into such situation. 
 */
			if (corr > (MAXADJ*2*1000) || corr < -(MAXADJ*2*1000)) {
				(void)gettimeofday(&msgs.tsp_time,
						   (struct timezone *)0);
				msgs.tsp_time.tv_sec += corr/1000;
				msgs.tsp_type = TSP_SETTIME;
			} else {
				msgs.tsp_time = mstotvround(&corr);
				msgs.tsp_type = TSP_ADJTIME;
			}
#else
			msgs.tsp_time = mstotvround(&corr);
			msgs.tsp_type = (u_char)TSP_ADJTIME;
#endif
			(void)strcpy(msgs.tsp_name, hostname);
			answer = acksend(&msgs, &hp[i].addr, hp[i].name,
			    TSP_ACK, (struct netinfo *)NULL);
			if (answer == NULL) {
				hp[i].delta = HOSTDOWN;
#ifdef MEASURE
				fprintf(fp, "%s\t", "down");
			} else {
				fprintf(fp, "%d\t", corr);
#endif
			}
		} else {
#ifdef MEASURE
			fprintf(fp, "%s\t", "down");
#endif
		}
	}
#ifdef MEASURE
	fprintf(fp, "\n");
#endif
}

/* 
 * `mstotvround' rounds up the value of the argument to the 
 * nearest multiple of five, and converts it into a timeval 
 */
 
struct timeval mstotvround(x)
int *x;
{
	int temp;
	struct timeval adj;

#ifndef sgi   /* silly for our clocks, and badly coded */
	temp = *x % 5;
	if (temp >= 3)
		*x = *x-temp+5;
	else {
		if (temp <= -3)
			*x = *x - temp -5;
		else 
			*x = *x-temp;
	}
#endif
	adj.tv_sec = *x/1000;
	adj.tv_usec = (*x-adj.tv_sec*1000)*1000;
	if (adj.tv_usec < 0) {
		adj.tv_usec += 1000000;
		adj.tv_sec--;
	}
	return(adj);
}

adjclock(corr)
struct timeval *corr;
{
	struct timeval now;

	if (timerisset(corr)) {
		if (corr->tv_sec < MAXADJ && corr->tv_sec > - MAXADJ) {
#ifdef sgi
			register long delta;
			delta = corr->tv_sec*1000000 + corr->tv_usec;
			if (delta > -RANGE*1000 && delta < RANGE*1000) {
				struct timeval trim;
				if (trace)
					fprintf(fd,
						"trimming delta of %d usec\n",
						delta);
				trim.tv_sec = 0;
				if (delta > -RANGE*500 && delta < RANGE*500)
					delta /=2;
				trim.tv_usec = delta/2;
				(void)adjtime(&trim, (struct timeval *)0);
			} else
#endif
			(void)adjtime(corr, (struct timeval *)0);
		} else {
			syslog(LOG_WARNING,
			    "clock correction too large to adjust (%d sec)",
			    corr->tv_sec);
			(void) gettimeofday(&now, (struct timezone *)0);
			timevaladd(&now, corr);
#ifdef sgi
			if (stime(&now.tv_sec) < 0)
				syslog(LOG_ERR, "can't set time");
#else
			if (settimeofday(&now, (struct timezone *)0) < 0)
				syslog(LOG_ERR, "can't set time");
#endif
		}
	}
}

timevaladd(tv1, tv2)
	register struct timeval *tv1, *tv2;
{
	
	tv1->tv_sec += tv2->tv_sec;
	tv1->tv_usec += tv2->tv_usec;
	if (tv1->tv_usec >= 1000000) {
		tv1->tv_sec++;
		tv1->tv_usec -= 1000000;
	}
	if (tv1->tv_usec < 0) {
		tv1->tv_sec--;
		tv1->tv_usec += 1000000;
	}
}
