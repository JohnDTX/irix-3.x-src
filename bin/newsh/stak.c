/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sh:stak.c	1.8"
/*
 * UNIX shell
 */

#include	"defs.h"


/* ========	storage allocation	======== */

char *
getstak(asize)			/* allocate requested stack */
int	asize;
{
	register char	*oldstak;
	register int	size;

	size = round(asize, BYTESPERWORD);
	oldstak = stakbot;
	staktop = stakbot += size;
	return(oldstak);
}

/*
 * set up stack for local use
 * should be followed by `endstak'
 */
char *
locstak()
{
	if (brkend - stakbot < BRKINCR)
	{
		if (setbrk(brkincr) == -1)
			error(nostack);
		if (brkincr < BRKMAX)
			brkincr += 256;
	}
	return(stakbot);
}

char *
savstak()
{
	assert(staktop == stakbot);
	return(stakbot);
}

char *
endstak(argp)		/* tidy up after `locstak' */
register char	*argp;
{
	register char	*oldstak;

	*argp++ = 0;
	oldstak = stakbot;
	stakbot = staktop = (char *)round(argp, BYTESPERWORD);
	return(oldstak);
}

tdystak(x)		/* try to bring stack back to x */
register char	*x;
{
	while ((char *)(stakbsy) > (char *)(x))
	{
		free(stakbsy);
		stakbsy = stakbsy->word;
	}
	staktop = stakbot = max((char *)(x), (char *)(stakbas));
	rmtemp(x);
}

stakchk()
{
	if ((brkend - stakbas) > BRKINCR + BRKINCR)
		setbrk(-BRKINCR);
}

char *
cpystak(x)
char	*x;
{
	return(endstak(movstr(x, locstak())));
}
