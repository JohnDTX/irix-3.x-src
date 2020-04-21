/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#include "tdef.h"
#ifdef NROFF
#include "tw.h"
#endif
#include "ext.h"

struct 	s *frame, *stk, *ejl;
struct	s *nxf;

int	pipeflg;
int	hflg;	/* used in nroff only */
int	eqflg;	/* used in nroff only */

#ifndef NROFF
int	xpts;
int	ppts;
int	pfont;
int	mpts;
int	mfont;
int	cs;
int	ccs;
int	bd;
#endif

int	stdi;
int	nofeed;
int	quiet;
int	stop;
char	ibuf[IBUFSZ];
char	xbuf[IBUFSZ];
char	*ibufp;
char	*xbufp;
char	*eibuf;
char	*xeibuf;
tchar	pbbuf[NC];	/* pushback buffer for arguments, \n, etc. */
tchar	*pbp = pbbuf;	/* next free slot in pbbuf */
tchar	*lastpbp = pbbuf;	/* pbp in previous stack frame */
int	nx;
int	mflg;
tchar	ch = 0;
int	ibf;
int	ttyod;
int	iflg;
char	*enda;
int	rargc;
char	**argp;
int	trtab[NTRTAB];
int	lgf;
int	copyf;
filep	ip;
int	nlflg;
int	donef;
int	nflush;
int	nfo;
int	ifile;
int	padc;
int	raw;
int	ifl[NSO];
int	ifi;
int	flss;
int	nonumb;
int	trap;
int	tflg;
int	ejf;
int	gflag;
int	dilev;
filep	offset;
int	em;
int	ds;
filep	woff;
int	app;
int	ndone;
int	lead;
int	ralss;
filep	nextb;
tchar	nrbits;
int	nform;
int	oldmn;
int	newmn;
int	macerr;
filep	apptr;
int	diflg;
filep	roff;
int	wbfi;
int	evi;
int	vflag;
int	noscale;
int	po1;
int	nlist[NTRAP];
int	mlist[NTRAP];
int	evlist[EVLSZ];
int	ev;
int	tty;
int	sfont	= FT;	/* appears to be "standard" font; used by .ul */
int	sv;
int	esc;
int	widthp;
int	xfont;
int	setwdf;
int	over;
int	nhyp;
tchar	**hyp;
tchar	*olinep;
int	dotT;
char	*unlkp;
int	no_out;
struct	widcache widcache[NWIDCACHE];
struct	d d[NDI];
struct	d *dip;
