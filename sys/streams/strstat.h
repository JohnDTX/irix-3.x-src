/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * $Source: /d2/3.7/src/sys/streams/RCS/strstat.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:34:57 $
 */


/*
 * Streams Statistics header file.  This file
 * defines the counters which are maintained for statistics gathering
 * under Streams.
 */

typedef struct {
	int use;			/* current item usage count */
	int total;			/* total item usage count */
	int max;			/* maximum item usage count */
	int fail;			/* count of allocation failures */
} alcdat;

struct  strstat {
	alcdat stream;			/* stream allocation data */
	alcdat queue;			/* queue allocation data */
	alcdat mblock;			/* message block allocation data */
	alcdat dblock;			/* aggregate data block allocation */
	alcdat dblk[NCLASS];		/* data block class allocation */
};


/* in the following macro, x is assumed to be of type alcdat */
#ifdef OS_METER
#define BUMPUP(X)	{(X).use++;  (X).total++;\
	(X).max=((X).use>(X).max?(X).use:(X).max); }
#define BUMPDOWN(X) ((X).use--)
#else
#define BUMPUP(X)
#define BUMPDOWN(X)
#endif


/* per-module statistics structure */
struct module_stat {
	long ms_pcnt;			/* count of calls to put proc */
	long ms_scnt;			/* count of calls to service proc */
	long ms_ocnt;			/* count of calls to open proc */
	long ms_ccnt;			/* count of calls to close proc */
	long ms_acnt;			/* count of calls to admin proc */
	char *ms_xptr;			/* pointer to private statistics */
	short ms_xsize;			/* length of private statistics buf */
};
