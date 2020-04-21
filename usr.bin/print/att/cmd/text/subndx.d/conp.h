/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define SLENG 250
#define SCHAR 1500
extern struct ss {
	char	*sp, ic, cc; 
	int	leng;
} sent[SLENG];
extern struct ss *sentp;
extern comma, j, i;
extern int	nsleng;
extern question;
extern int	must;
extern int	be;
extern int	sav;
extern int	prep;
extern int	aflg, bflg, subty, verb, verbty;
extern int	hflg;
extern int	iverb, pverb, done;
