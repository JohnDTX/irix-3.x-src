/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef	sgi
#ident	"@(#)make:dyndep.c	1.2.1.1"
#endif
#include "defs"
/*
 *	Dynamicdep() checks each dependency by calling runtime().
 *	Runtime() determines if a dependent line contains "$@"
 *	or "$(@F)" or "$(@D)". If so, it makes a new dependent line
 *	and insert it into the dependency chain of the input name, p.
 *	Here, "$@" gets translated to p->namep. That is
 *	the current name on the left of the colon in the
 *	makefile.  Thus,
 *		xyz:	s.$@.c
 *	translates into
 *		xyz:	s.xyz.c
 *
 *	Also, "$(@F)" translates to the same thing without a prededing
 *	directory path (if one exists).
 *	Note, to enter "$@" on a dependency line in a makefile
 *	"$$@" must be typed. This is because `make' expands
 *	macros upon reading them.
 */

#define is_dyn(a)		(any( (a), DOLLAR) )


dynamicdep(p)
register NAMEBLOCK p;
{
	register LINEBLOCK lp, nlp;
	LINEBLOCK backlp=0;

	p->rundep = 1;

	for(lp = p->linep; lp != 0; lp = lp->nextline)
	{
		if( (nlp=runtime(p, lp)) != 0)
			if(backlp)
				backlp->nextline = nlp;
			else
				p->linep = nlp;

		backlp = (nlp == 0) ? lp : nlp;
	}
}


LINEBLOCK runtime(p, lp)
NAMEBLOCK p;
register LINEBLOCK lp;
{
	union
	{
		int u_i;
		NAMEBLOCK u_nam;
	} temp;
	register DEPBLOCK q, nq;
	LINEBLOCK nlp;
	NAMEBLOCK pc;
	CHARSTAR pc1;
	char c;
	CHARSTAR pbuf;
	char buf[128];

	temp.u_i = NO;
	for(q = lp->depp; q != 0; q = q->nextdep)
	{
		if((pc=q->depname) != 0)
		{
			if(is_dyn(pc->namep))
			{
				temp.u_i = YES;
				break;
			}
		}
	}

	if(temp.u_i == NO)
	{
		return(0);
	}

	nlp = ALLOC(lineblock);
	nq  = ALLOC(depblock);

	nlp->nextline = lp->nextline;
	nlp->shp   = lp->shp;
	nlp->depp  = nq;

	for(q = lp->depp; q != 0; q = q->nextdep)
	{
		pc1 = q->depname->namep;
		if(is_dyn(pc1))
		{
			subst(pc1, buf);
			temp.u_nam = srchname(buf);
			if(temp.u_nam == 0)
				temp.u_nam = makename(copys(buf));
			nq->depname = temp.u_nam;
		}
		else
		{
			nq->depname = q->depname;
		}

		if(q->nextdep == 0)
			nq->nextdep = 0;
		else
			nq->nextdep = ALLOC(depblock);

		nq = nq->nextdep;
	}
	return(nlp);
}
