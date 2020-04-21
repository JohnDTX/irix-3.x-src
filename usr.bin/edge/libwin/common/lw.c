/*
 * Lightweight process management
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/lw.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:27 $
 */

#include "tf.h"
#include "lw.h"
#include "stdio.h"

/* XXX */
short	lwtype, lwvalue;

static	lwproc	mainproc;
static	lwproc	*current;
static	char	initialized;
static	lwqueue	runq, sleepq, deadq;
static	lwmsg	*freelist;

/*
 * Event registration hashing
 */
#define	REGHASHSIZE	512
#define	REGHASH(type)	((type) & (REGHASHSIZE - 1))
static	lwqueue	reghash[REGHASHSIZE];

void	lwswtch(), lwidle(), lwsleep(), lwwakeup();

/*
 * Allocate a new message
 */
lwmsg *
lwnewmsg()
{
	register lwmsg *msg;
	register int i;

	if (!freelist) {
		msg = MALLOC(lwmsg *, 100 * sizeof(lwmsg));
		if (!msg) {
			fprintf(stderr, "out of memory\n");
			exit(-1);
		}
		for (i = 100; --i >= 0; ) {
			msg->m_next = freelist;
			freelist = msg;
			msg++;
		}
	}
	msg = freelist;
	freelist = msg->m_next;
	return (msg);
}

/*
 * Dispose of a message
 */
void
lwputmsg(msg)
	register lwmsg *msg;
{
	msg->m_next = freelist;
	freelist = msg;
	if (msg->m_data) {
		FREE(msg->m_data);
		msg->m_data = 0;
	}
}

/*
 * Get a message for the current lightweight process
 */
lwmsg *
lwgetmsg()
{
	register lwmsg *msg;

	ASSERT(current);

	/* wait for a message to appear */
	while (!current->lw_queue) {
		current->lw_flags |= LW_QWAIT;
		lwsleep(&current->lw_queue);
	}

	/* pull head message off of queue */
	msg = current->lw_queue;
	current->lw_queue = msg->m_next;
	return (msg);
}

/*
 * Register the current process as receiving a particular message
 */
void
lwregister(msgtype, msgvalue, checkvalue)
	long msgtype;
	long msgvalue;
	int checkvalue;
{
	register lwregblk *reg;
	register lwqueue *hash;

	ASSERT(current);
	reg = MALLOC(lwregblk *, sizeof(lwregblk));
	if (!reg) {
		write(2, "lwregister: out of memory\n",
			 sizeof("lwregister: out of memory\n"));
		exit(-1);
	}
	reg->r_type = msgtype;
	reg->r_value = msgvalue;
	reg->r_checkvalue = checkvalue;
	reg->r_proc = current;

	/* put on hash lists */
	hash = &reghash[REGHASH(msgtype)];
	LWQUEUE(hash, reg);
}

/*
 * Send the given message to every process that's listening
 * XXX fix this to really work
 */
void
lwsendmsg(msg)
	register lwmsg *msg;
{
	register lwqueue *hash;
	register lwregblk *reg;
	register lwproc *proc;
	register lwqueue *head;

	hash = &reghash[REGHASH(msg->m_type)];
	head = hash;
	while (hash->succ != head) {
		reg = (lwregblk *) (hash->succ);
		if ((reg->r_type == msg->m_type) &&
		    (!reg->r_checkvalue || (reg->r_value == msg->m_value))) {
			proc = reg->r_proc;
			msg->m_next = proc->lw_queue;
			proc->lw_queue = msg;
			if (proc->lw_flags & LW_QWAIT) {
				proc->lw_flags &= ~LW_QWAIT;
				lwwakeup(&proc->lw_queue);
			}
			return;		/* XXX */
		}
		hash = hash->succ;
	}

	/* nobody wants me */
	lwputmsg(msg);
}

/*
 * Reaper process.  This process is responsible for freeing up memory
 * consumed by dead processes.  This is done so that we don't have to
 * do anything fancy while running on the dead processes stack.
 */
int
lwreaper()
{
	register lwproc *p;

	for (;;) {
		while (deadq.succ != &deadq) {
			LWDEQUEUE(&deadq, p);
			ASSERT(p);
			ASSERT(p->lw_stack);
			printf("process %s is now dead\n", p->lw_name);
			FREE(p->lw_stack);
			FREE(p);
		}
		lwsleep(&deadq);
	}
}

/*
 * Initialize the lw stuff
 */
void
lwinit()
{
	register int i;

	initialized = 1;

	/* initialize queues */
	LWQINIT(&runq);
	LWQINIT(&sleepq);
	LWQINIT(&deadq);
	for (i = REGHASHSIZE; --i >= 0; ) {
		LWQINIT(&reghash[i]);
	}

	/* fabricate process state for the current process */
	if (setjmp(mainproc.lw_pcb)) {
		/*
		 * Not supposed to be able to get here.
		 */
		ASSERT(1 == 0);
		return;
	}
	mainproc.lw_name = "main";
	current = &mainproc;

	/*
	 * Lastly, start the reaper process
	 */
	lwstart("reaper", lwreaper, 0, 0, 0, 0, 0);
}

/*
 * Start a new lightweight process
 */
lwproc *
lwstart(name, func, stacksize, arg0, arg1, arg2, arg3)
	char *name;
	int (*func)();
	int stacksize;
	long arg0, arg1, arg2, arg3;
{
	register lwproc *p;
	register unsigned long *frame;

	if (!initialized)
		lwinit();
printf("start(%s)\n", name);
	/*
	 * Allocate a new process slot and memory for its stack.
	 */
	p = CALLOC(lwproc *, 1, sizeof(lwproc));
	if (p) {
		/*
		 * If user doesn't care about stack size, use default.
		 * Insure that stack size is a multiple of a long.
		 */
		if (stacksize == 0)
			stacksize = LWSTACKSIZE;
		if (stacksize & (sizeof(long)-1))
			stacksize += sizeof(long) -
				(stacksize & (sizeof(long)-1));
		p->lw_stack = MALLOC(char *, stacksize);
		if (!p->lw_stack) {
			FREE(p);
			return ((lwproc *)0);
		}
		p->lw_name = name;
		/*
		 * Build up stack frame.  This requires knowledge of the
		 * machine, unfortunately.  Arrange for lwexit to be
		 * called if the process return's from its function.
		 */
		frame = (unsigned long *) (p->lw_stack + stacksize);
#if defined(IP2) || defined(PM2)
		*--frame = arg3;
		*--frame = arg2;
		*--frame = arg1;
		*--frame = arg0;
		*--frame = (long) p;
		*--frame = (unsigned long) lwexit;	/* return pc */
		/*
		 * One more word is needed to pretend that "setjmp" was
		 * called in the first place.
		 */
		*--frame = 0;
		SETPC(p->lw_pcb, func);
		SETSP(p->lw_pcb, frame);
#endif
		/*
		 * Put this process at the end of the list of processes
		 * waiting to run.
		 */
		LWQUEUE(&runq, p);
	}
	return (p);
}

/*
 * Destroy the current lightweight process.
 */
int
lwexit()
{
	ASSERT(current);
	LWQUEUE(&deadq, current);	/* put on dead process list */
	lwwakeup(&deadq);		/* wakeup the reaper */
	lwswtch();			/* switch to another process */
}

/*
 * Run other processes
 */
lwrun()
{
	for (;;) {
		lwsleep(lwrun);
	}
}

/*
 * Yield the processor.
 */
lwyield()
{
	ASSERT(current);
	LWQUEUE(&runq, current);
	lwswtch();
}

/*
 * Lightweight process scheduler.
 * Pick a new process to run and run it.  If no processes are ready,
 * run the idle process.  Just like unix...
 */
void
lwswtch()
{
	/*
	 * If the current process is not null, put it at the end of the
	 * run queue.  The current process will be null if (a) it has
	 * been put to sleep or (b) it has died.
	 */
	ASSERT(current);
	if (setjmp(current->lw_pcb)) {
		/*
		 * Time to run again...here we go...
		 */
		ASSERT(current);
		return;
	}

	/*
	 * Run next process on the run queue
	 */
	while (runq.succ == &runq) {
		lwidle();
	}

	/* pull first process off head of runq */
	LWDEQUEUE(&runq, current);
	ASSERT(current);

	/* resume the process */
	longjmp(current->lw_pcb, 1);
	/* NOTREACHED */
}

/*
 * Temporary idle code for testing
 */
void
lwidle()
{
	register lwmsg *msg;
	short type, value;

#ifdef	NOISE
printf("Idle...\n");
#endif
	type = qread(&value);
	msg = lwnewmsg();
	msg->m_type = type;
	msg->m_value = value;
	msg->m_data = 0;
	lwsendmsg(msg);
}

/*
 * Put the current lightweight process to sleep on the given
 * event.
 */
void
lwsleep(event)
	char *event;
{
	register lwproc *p;

	p = current;
	ASSERT(p);
	ASSERT(p->lw_event == 0);
#ifdef	NOISE
printf("sleep(%x, %s, %x)\n", current, current->lw_name, event);
#endif
	p->lw_event = event;
	LWQUEUE(&sleepq, p);
	lwswtch();
}

/*
 * Wakeup all processes waiting for the given event.
 */
void
lwwakeup(event)
	char *event;
{
	register lwproc *p, *next;

	p = (lwproc *) sleepq.succ;
	while (&p->qlink != &sleepq) {
		next = (lwproc *) (p->qlink.succ);
		ASSERT(p->lw_event);
		if (p->lw_event == event) {
#ifdef	NOISE
printf("wakeup(%x, %s, %x)\n", p, p->lw_name, event);
#endif
			LWREMOVE(p);
			LWQUEUE(&runq, p);
			p->lw_event = 0;
		}
		p = next;
	}
}
