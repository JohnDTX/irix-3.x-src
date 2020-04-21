/*
 *	isrequest(s, dest, seqno) - predicate which returns true if
 *		s is a syntactically correct request id as returned by lp(1).
 *		No check is made to see if it is in the output queue.
 *		if s is a request id, then
 *			dest is set to the destination
 *			seqno is set to the sequence number
 */

#include	"lp.h"

SCCSID("@(#)isrequest.c	3.1")

isrequest(s, dest, seqno)
char *s;
char *dest;
int *seqno;
{
	char *strchr(), *p, *strncpy();

	if((p = strchr(s, '-')) == NULL || p == s || p - s > DESTMAX ||
	    (*seqno = atoi(p + 1)) <= 0 || *seqno >= SEQMAX)
		return(FALSE);
	strncpy(dest, s, p - s);
	*(dest + (p - s)) = '\0';
	return(isdest(dest));
}
