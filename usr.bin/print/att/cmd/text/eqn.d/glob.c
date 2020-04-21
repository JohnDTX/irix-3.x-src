/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#include "e.h"

int	dbg;		/* debugging print if non-zero */
int	lp[200];		/* stack for things like piles and matrices */
int	ct;		/* pointer to lp */
int	used[100];	/* available registers */
int	ps;		/* default init point size */
int	deltaps	= 3;	/* default change in ps */
int	dps_set = 0;	/* 1 => -p option used */
int	gsize	= 10;	/* default initial point size */
int	ft	= '2';
Font	ftstack[10] = { '2', "2" };	/* bottom is global font */
Font	*ftp	= ftstack;
int	szstack[10];	/* non-zero if absolute size set at this level */
int	nszstack = 0;
int	display	= 0;	/* 1=>display, 0=>.EQ/.EN */

/* default is APS-5 */
	char	*typesetter = "aps";	/* name for -T */
	int	ttype	= DEVAPS;	/* type of typesetter today */
	int	res	= 723;		/* resolution of typesetter; dflt = aps */
	int	minsize	= 5;		/* min size it can handle; ditto */
#ifdef	202
	char	*typesetter = "202";
	int	ttype	= DEV202;	/* type of typesetter today */
	int	res	= 972;		/* resolution of typesetter; dflt = aps */
	int	minsize	= 5;		/* min size it can handle; ditto */
#endif

int	synerr;		/* 1 if syntax err in this eqn */
float	eht[100];	/* height in ems at gsize */
float	ebase[100];	/* base: where one enters above bottom */
int	eps[100];	/* unused right now */
int	lfont[100];
int	rfont[100];
int	eqnreg;		/* register where final string appears */
float	eqnht;		/* final height of equation */
int	lefteq	= '\0';	/* left in-line delimiter */
int	righteq	= '\0';	/* right in-line delimiter */
int	markline = 0;	/* 1 if this EQ/EN contains mark; 2 if lineup */
