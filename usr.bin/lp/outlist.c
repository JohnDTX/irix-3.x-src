/* routines to manipulate output request list */

#include	"lp.h"
#include	"lpsched.h"

SCCSID("@(#)outlist.c	3.1")

/* newol() -- returns pointer to empty output list */

struct outlist *
newol()
{
	struct outlist *ol;

	if((ol = (struct outlist *) malloc(sizeof(struct outlist))) == NULL)
		fatal(CORMSG, 1);

	ol->ol_seqno = ol->ol_time = 0;
	ol->ol_name = NULL;
	ol->ol_print = ol->ol_dest = NULL;
	ol->ol_next = ol->ol_prev = ol;

	return(ol);
}

/* inserto(d, seqno, name) -- inserts output request with sequence number
	'seqno' and logname 'name' at the tail of the output list for
	destination 'd' */

inserto(d, seqno, name)
struct dest *d;
int seqno;
char *name;
{
	static int timer = 1;	/* counter to order entry of requests */
	struct outlist *new, *head;
	char *s, *strcpy();

	if((new = (struct outlist *) malloc(sizeof(struct outlist))) == NULL ||
	   (s = malloc((unsigned)(strlen(name) + 1))) == NULL)
		fatal(CORMSG, 1);

	new->ol_seqno = seqno;
	new->ol_time = timer++;
	strcpy(s, name);
	new->ol_name = s;
	new->ol_print = NULL;
	new->ol_dest = d;

	head = d->d_output;
	new->ol_next = head;
	new->ol_prev = head->ol_prev;
	head->ol_prev = new;
	(new->ol_prev)->ol_next = new;
}

/* deleteo(o) -- delete output entry o */

deleteo(o)
struct outlist *o;
{
	free(o->ol_name);

	(o->ol_next)->ol_prev = o->ol_prev;
	(o->ol_prev)->ol_next = o->ol_next;

	free((char *) o);
}

/* geto(d, seqno) -- returns pointer to the output list structure for
	destination d with sequence number seqno.  If this doesn't exist,
	then geto returns NULL.
*/

struct outlist *
geto(d, seqno)
struct dest *d;
int seqno;
{
	struct outlist *head, *o;

	for(head = d->d_output, o = head->ol_next;
	    o != head && o->ol_seqno != seqno;
	    o = o->ol_next)
		;
	return(o == head ? NULL : o);
}
