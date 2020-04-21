/*
 * Software queue
 */

typedef struct qentry {
	short	type;
	short	value;
	struct	qentry *next;
} qentry;

qentry	*qhead;
qentry	*qtail;

#define	MINALLOC	50	/* minimum to allocate */
qentry	*qfree;			/* free list of queue entries */

extern	char	*calloc();

/*
 * softqtest:
 *	- test and see if the queue has anything in it
 *	- if it does, return type of first entry
 */
short
softqtest()
{
	short t, v;

	if (qhead)
		return (qhead->type);
	if (qtest()) {
		/* read in graphics queue */
		do {
			t = qread(&v);
			softqenter(t, v);
		} while (qtest());
		return (qhead->type);
	}
	return (0);
}

/*
 * softqread:
 *	- read first item out of queue
 *	- if queue is empty, block until something shows up
 */
short
softqread(v)
	register short *v;
{
	register short t;
	register qentry *thisone;

	for (;;) {
		if (thisone = qhead) {
			t = thisone->type;	/* capture type and value */
			*v = thisone->value;
			qhead = thisone->next;	/* advance to next entry */
			thisone->next = qfree;	/* put qentry on free list */
			qfree = thisone;
			return (t);
		}

		/* wait for something to show up in the queue */
		t = qread(v);
		softqenter(t, *v);

		/* read remainder of queue */
		while (qtest()) {
			t = qread(v);
			softqenter(t, *v);
		}
	}
}

/*
 * softqenter:
 *	- add something to the queue
 */
#define	OUTOFMEM	"softqenter: out of memory\n"
softqenter(t, v)
	short t, v;
{
	register qentry *new;
	register int i;

	while ((new = qfree) == (qentry *)0) {
		new = (qentry *)calloc(MINALLOC, sizeof(qentry));
		if (new == (qentry *)0) {
			write(2, OUTOFMEM, (unsigned) strlen(OUTOFMEM));
			return;
		}

		/* build free list with newly allocated entries */
		for (i = MINALLOC; --i >= 0; ) {
			new->next = qfree;
			qfree = new;
			new++;
		}
	}
	qfree = new->next;
	new->type = t;
	new->value = v;
	new->next = (qentry *)0;

	if (qhead)
		qtail->next = new;
	else
		qhead = new;
	qtail = new;
}
