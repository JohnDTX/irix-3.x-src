/*
 * Event dispatch manager
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/dispatch.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:20 $
 */

#define	NULL	0
struct	event {
	long	e_event;			/* event begin caught */
	int	(*e_func)();			/* function to handle it */
	long	e_arg;				/* first arg to function */
	struct	event *e_next, *e_prev;		/* linkage for buckets */
};

/* number of hash buckets */
#define	NBUCKETS	64

/* hash function */
#define	HASH(e)		&eventBucket[(e) & (NBUCKETS-1)]

/* hash buckets for events */
struct	event	*eventBucket[NBUCKETS];

/*
 * Start catching an event
 */
void
catchEvent(event, func, arg)
	long event;
	int (*func)();
	long arg;
{
	register struct event **bucket, *e;

	bucket = HASH(event);
	e = (struct event *) malloc(sizeof(struct event));
	e->e_event = event;
	e->e_func = func;
	if (e->e_next = *bucket)
		e->e_next->e_prev = e;
	e->e_prev = 0;
	e->e_arg = arg;
	*bucket = e;
}

/*
 * Stop catching an event.
 */
void
unCatchEvent(event, func, arg)
	register long event;
	register int (*func)();
	register long arg;
{
	register struct event *e;
	register struct event **bucket;

	bucket = HASH(event);
	e = *bucket;
	while (e) {
		if ((e->e_event == event) &&
		    (e->e_func == func) &&
		    (e->e_arg == arg)) {
			if (e->e_prev)
				e->e_prev->e_next = e->e_next;
			if (e->e_next)
				e->e_next->e_prev = e->e_prev;
			if (*bucket == e)
				*bucket = e->e_next;
			return;
		}
		e = e->e_next;
	}
}

/*
 * Send an event.
 * XXX event handlers can't unCatchEvent
 */
void
sendEvent(event, value)
	register long event;
	long value;
{
	register struct event *e;

	e = *(HASH(event));
	while (e) {
		if (e->e_event == event) {
			(*e->e_func)(e->e_arg, event, value);
		}
		e = e->e_next;
	}
}

/*
 * Manage events.
 * XXX add in "select" handling
 * XXX add in better timer management
 */
void
eventManager()
{
	short t, v;

	for (;;) {
		t = qread(&v);
		sendEvent(t, v);
	}
}
