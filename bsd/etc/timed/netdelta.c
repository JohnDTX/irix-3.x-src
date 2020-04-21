/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)networkdelta.c	2.1 (Berkeley) 12/10/85";
#endif not lint

#include "globals.h"
#include <protocols/timed.h>

#if defined(sgi) || defined(sgi_vax)
/* This entire function is not very impressive.  It is not only slow, but
 *	not quite correct.  It should be re-written.
 */
#else
extern int machup;
#endif

/*
 * `networkdelta' selects the largest set of deltas that fall within the
 * interval RANGE, and uses them to compute the network average delta 
 */

long networkdelta()
{
#if defined(sgi) || defined(sgi_vax)
	int machup;
	register int i, j, maxind, minind;
#else
	int i, j, maxind, minind;
#endif
	int ext;
	int tempind;
	long tempdata;
	long x[NHOSTS];
	long average;

#if defined(sgi) || defined(sgi_vax)
	x[0] = 0;
	machup = 1;
	for (i=1; i<slvcount; i++) {	/* ignore strange hosts */
		if (hp[i].good && hp[i].delta != HOSTDOWN) {
			x[i] = hp[i].delta;
			machup++;
		} else {
			x[i] = HOSTDOWN;
		}
	}
	if (machup <= 1)		/* quit if no trustworthy hosts */
		return(0);
#else
	for (i=0; i<slvcount; i++)
		x[i] = hp[i].delta;
#endif
	for (i=0; i<slvcount-1; i++) {
		tempdata = x[i];
		tempind = i;
		for (j=i+1; j<slvcount; j++) {
			if (x[j] < tempdata) {
				tempdata = x[j];
				tempind = j;
			}
		}
		x[tempind] = x[i];
		x[i] = tempdata;
	}

	/* this piece of code is critical: DO NOT TOUCH IT! */
/****/
	i=0; j=1; minind=0; maxind=1;
	if (machup == 2)
		goto compute;
	do {
		if (x[j]-x[i] <= RANGE)
			j++;
		else {
			if (j > i+1) 
 				j--; 
			if ((x[j]-x[i] <= RANGE) && (j-i >= maxind-minind)) {
				minind=i;
				maxind=j;
			}	
			i++;
			if(i = j)
				j++;
		}
	} while (j < machup);
	if ((x[machup-1] - x[i] <= RANGE) && (machup-i-1 >= maxind-minind)) {
		minind=i; maxind=machup-1;
	}
/****/
compute:
	ext = maxind - minind + 1;
	average = 0;
	for (i=minind; i<=maxind; i++)
		average += x[i];
	average /= ext;
	return(average);
}
