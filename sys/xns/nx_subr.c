# define MBMAPGET
/*
 * Utility routines and initialization code for Excelan ethernet controller.
 *
 * $Source: /d2/3.7/src/sys/xns/RCS/nx_subr.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:36:23 $
 */

#include "nx.h"
#include "../h/param.h"
#include "../h/tty.h"
#include "../h/termio.h"
#include "machine/cpureg.h"
#include "../multibus/mbvar.h"
#include "../xns/if_xns.h"

# ifndef MBIO_VBASE
# define MBIO_VBASE	MBIOBASE
# endif  MBIO_VBASE


char	nx_nmsgbufs = NWBUFS - 2;	/* max number of avail msgbufs */
char	msgbufs_free;			/* current number of */

struct	netbuf *nx_bufbase;		/* base of array of netbufs */


/*
 * Memory for message queues and packet buffers
 */
char	mqueues[sizeof (struct mqueues) + NXSEGSIZE-1];
long	nxva, nx_mbva, nxeva;
# ifdef MBMAPGET
# define NXVTOP(x)	(((long)(x) - nxva) + nx_mbva)
# else  MBMAPGET
# define NXVTOP(x)	VTOP(x)
# endif MBMAPGET

# ifdef IFXNS

char *nx_xs;				/* address of ex_softc[0] */
char **nx_bufp;				/* ptr to ex recv buffer ptr */
char nx_didsetup;

nxns_probe(a, b, na)
	u_char *a, *b;
	struct physnet *na;
{
	PORTA = a;
	PORTB = b;
	myaddr = *na;
}

nxns_qsetup(xs, bufp)
	char *xs, **bufp;
{
	register struct mqueues *mq, *pmq;

	if (nxva == 0) {
		nxva = ((long)mqueues + NXSEGSIZE-1) & ~NXSEGMASK;
# ifdef MBMAPGET
		nx_mbva = mbmapkget(nxva, sizeof (struct mqueues));
# else  MBMAPGET
		nx_mbva = NXVTOP(nxva);
# endif MBMAPGET
		nxeva = nxva + sizeof (struct mqueues);
	}

	msgbufs_free = nx_nmsgbufs;

	nx_xs = xs;
	nx_bufp = bufp;

	mq = (struct mqueues *)nxva;
	pmq = (struct mqueues *)nx_mbva;
	nbpinit(mq->netbufs, pmq->netbufs, NETBUFS);
	xns_initialize((long)mq->connection, (long)pmq->connection, &myaddr);
	nx_didsetup++;
}

nxns_go()
{
	extern short nx_onnet;

	nx_onnet = 1;
	nxpresent = 1;
}

# else  IFXNS

int	nxprobe(), nxstart(), nxintr();
struct	mb_device *nxdinfo[NNX];
struct	mb_driver nxdriver = {
	nxprobe, (int (*)())0, (int (*)())0, nxstart, nxintr,
	(char *(*)())0, "nx", nxdinfo,
};

/*
 * nxprobe:
 *	- probe for controller at 'reg'
 *	- wait for board to finish reset
 *	- allocate memory for data structures
 */
nxprobe(reg)
	long reg;
{
	register struct mqueues *mq, *pmq;

	/*
	 * Record addresses
	 */
	PORTB = (u_char *)(MBIO_VBASE + reg);
	PORTA = PORTB + 1;

	nxpresent = 0;
	if (nx_dev_reset() < 0)
		return CONF_DEAD;
	/*
	 * Call nxqsetup() to setup message queue's and init board.
	 */
	nxqsetup();

	if (nxinitmsg(1) < 0)
		return CONF_DEAD;
	nxpresent++;

	/*
	 * Now init the software
	 */
	mq = (struct mqueues *)nxva;
	pmq = (struct mqueues *)nx_mbva;
	nbpinit(mq->netbufs, pmq->netbufs, NETBUFS);
	nxinit();
	xns_initialize((long)mq->connection, (long)pmq->connection, &myaddr);
	return CONF_ALIVE;
}

int
nx_dev_reset()
{
	register int x;

	/*
	 * Test for status after board reset.
	 * up to 4 seconds.
	 */
	x = *PORTA;				/* reset board */
#ifdef	lint
	x = x;
#endif
	if (!buzzon(PORTB, NX_ERROR_, 00, 4000))
		return -1;

	return 0;
}

/*
 * nxinit:
 *	- put controller into link level mode
 *	- get our ethernet address
 */
nxinit()
{
	extern short nx_onnet;

	nxgetslot(MYADDRSLOT, &myaddr);
	*(struct physnet *)&localnet.host = myaddr;
	printf("(%02x%02x.%02x%02x.%02x%02x) ",
		 (unsigned char)myaddr.addr[0], (unsigned char)myaddr.addr[1],
		 (unsigned char)myaddr.addr[2], (unsigned char)myaddr.addr[3],
		 (unsigned char)myaddr.addr[4], (unsigned char)myaddr.addr[5]);

	if (nxmode(NET_CONNECT) == 0)
		nx_onnet++;
	if (!nx_onnet)
		printf("OFF NET\n");
}

/*
 * nxqsetup:
 *	- setup data structures
 *	- send controller the init message
 */
nxqsetup()
{
	register struct mqueues *mq;

	if (nxva == 0) {
		nxva = ((long)mqueues + NXSEGSIZE-1) & ~NXSEGMASK;
# ifdef MBMAPGET
		nx_mbva = mbmapkget(nxva, sizeof (struct mqueues));
# else  MBMAPGET
		nx_mbva = NXVTOP(nxva);
# endif MBMAPGET
	}

	mq = (struct mqueues *)nxva;
/* XXX get rid of this */
nxmp = mq;
	mq->roffset = HTOX(mq->rbufs);
	mq->woffset = HTOX(mq->wbufs);
	lastrmp = mq->rbufs;
	curxmp = mq->wbufs;
	mq->rhead = mq->rbufs;
	mq->whead = mq->wbufs;

	/* exos-to-host buffers */
	linkbufs(mq->rbufs, NRBUFS, MX_EXOS|MX_BUSY);

	/* host-to-exos buffers */
	linkbufs(mq->wbufs, NWBUFS, MX_HOST);

	msgbufs_free = nx_nmsgbufs;
}


struct testp thetestpat =
{ 0x01,0x03,0x07,0x0F, 0x0103,0x070F, 0x0103070F };

nxinitmsg(verbose)
	int verbose;
{
	register struct init *im;

	/* construct initialization message (link level) */
	im = &nxmp->imsg;

	bzero((caddr_t)im, sizeof *im);
	im->res0 = 1;
	im->amode = 3;
	im->nproc = im->nmbox = im->code = 0xff;
	im->naddr = NDEV;			/* # multicast slots */
	*(struct testp *)im->mmap = thetestpat;
	im->format[0] = im->format[1] = im->nhost = 1;
	im->hxitype = 0;
	im->xhitype = 3;
	im->mvblk = 0xFFFFFFFF;
	im->hxseg = NXVTOP(nxva);
	im->hxhead = HTOX(&nxmp->woffset);
	im->xhseg = NXVTOP(nxva);
	im->xhhead = HTOX(&nxmp->roffset);
	/*
	 * Some versions of the excelan board have a firmware problem dealing
	 * with the amount of memory installed in the board.  The firmware will
	 * detect extra memory beyond what it is configured for, and continue
	 * to run normally, for about 20 minutes, then it will die
	 * "mysteriously". Unfortunately, some of the firmware went out with
	 * the detection method inverted, which means a perfectly legitimate
	 * board with 128k of memory may mysteriously break after 20 minutes.
	 * The fix is to put a "1" into the first byte of the second reserved
	 * field in the initialization message. 
	 */
	im->res1[0] = 1;		/* for screwy firmware versions */

	/* send init message to board */
	sendlong((long)0x0000FFFF);		/* send some junk */
	sendlong((long)NXVTOP(im));		/* send init message */

	/*
	 * Wait for board to deal with init message; up to 2 seconds.
	 */
	if (!buzzon(&im->code, 0xFF, 0xFF, 4000)) {
		printf("nx init timeout ");
		return -1;
	}
	if (im->code != 0) {
		printf("nx init bad return code %x ",im->code);
		return -1;
	}

	if (verbose)
		printf("(FW %c.%c HW %c.%c) ", im->vers[0], im->vers[1],
			im->vers[2], im->vers[3]);
	return 0;
}

# endif  IFXNS


/* ----- netbuf subroutines */
nbpinit(nbp, pnbp, n)
	register NBP nbp, pnbp;
	int n;
{
	while (--n >= 0) {
		nbp->perm = permnetbufs;
		permnetbufs = nbp;
		nbp->nxpaddr = (long)pnbp->info;
		freenbuf(nbp);
		nbp++;
		pnbp++;
	}
}

/*
 * return nbp to buffer pool.
 */
freenbuf(nbp)
	register NBP nbp;
{
	register s;

	if (nbp==NULL)
		return;
	LOCK;
# ifdef IFXNS
	UNMBUFIZE(nbp);
# else  IFXNS
	nbp->data = nbp->info;
# endif IFXNS
	nbp->next = freenetbufs;
	freenetbufs = nbp;
	nbp->btype = N_BUF;
#ifdef	XTRACE
	nbp->len = 9988;			/* XXX debug for xstat */
#endif
	if (bwaiting) {
		wakeup((caddr_t)&bwaiting);
		bwaiting = 0;
	}
	nxfreecount++;
	UNLOCK;
}

/*
 * getnbuf:
 *	- get a netbuf, waiting if none are available and allowed to
 *	- if musthavelast is true, then getnbuf will return the last
 *	  nbuf, otherwise, the caller must wait or get none
 *	- this avoids deadlock, since netbuf's are needed to be able
 *	  to receive packets from the excelan board
 */
NBP
getnbuf(musthavelast)
	int musthavelast;
{
	register NBP nbp;
	register int s;
	register int freethresh;

	LOCK;
	freethresh = musthavelast ? 1 : 2 + NREADS;
	while (nxfreecount < freethresh) {
		/*
		 * Check interrupt flag to see if we can wait for
		 * a buffer or not.
		 */
		if (nxintflag) {
			UNLOCK;
			return NULL;
		}
		bwaiting++;
		sleep((caddr_t)&bwaiting, TTOPRI);
	}

	nbp = freenetbufs;
	freenetbufs = nbp->next;
	nbp->dtype = 0;
	nbp->control = 0;
	nbp->len = 0;
	nbp->next = NULL;
	nxfreecount--;
	UNLOCK;
	return nbp;
}

/*
 * append buf to the end of q
 */
nbappend(nbp, q)
	register NBP nbp;
	register NBQ q;
{
	register s;
	LOCK;
	if (q->head==NULL) { 
		q->head = q->tail = nbp; 
	} else { 
		q->tail->next = nbp; 
		q->tail = nbp; 
	} 
	q->nbufs++; 
	nbp->next = NULL;
	UNLOCK;
}

/*
 * insert buf to the start of q
 * can only be called at hi pri.
 */
nbinsert(nbp, q)
	register NBP nbp;
	register NBQ q;
{
	nbp->next = q->head;
	if (q->head==NULL)
		q->tail = nbp; 
	q->head = nbp;
	q->nbufs++; 
}

/*
 * put a buffer's worth of data on the tail of a queue,
 * combining buffers of like type if possible (the given
 * buffer is freed in this case).
 *
 * called at interrupt time.
 * returns 1 on success (always).
 */
int
nbbcat(nbp, q)
	register NBP nbp;
	register NBQ q;
{
	register NBP tail;
	register char *tailend;

	if ((short)nbp->len <= 0) {
		freenbuf(nbp);
		return 1;
	}

	/*
	 * look at the tail buffer if any.  if its type and
	 * control are compatible with the given buffer and
	 * there is room in the tail buffer, copy the given
	 * buffer's data to the unused space at the end and
	 * free the given buffer.  for historical reasons,
	 * don't put more than 1024 bytes in a buffer.
	 */
	if (q->head != NULL && (tail = q->tail) != NULL
	 && nbp->dtype == tail->dtype && nbp->control == tail->control) {
# ifdef IFXNS
		/*
		 * If the data is in an mbuf,
		 * copy it out for joining.
		 */
		if (tail->m != 0) {
			if ((short)nbp->len <= (short)
					(1024 - tail->len)) {
				bcopy(tail->data, tail->info, tail->len);
				UNMBUFIZE(tail);
				tailend = tail->data + tail->len;
				goto merge;
			}
		}
		else {
			tailend = tail->data + tail->len;
			if ((short)nbp->len <= (short)
					(tail->info+1024 - tailend))
				goto merge;
		}
# else  IFXNS
		tailend = nxmtod(tail, caddr_t) + tail->len;
		if ((short)nbp->len <= (short)
				(tail->info+1024 - tailend))
			goto merge;
# endif IFXNS
	}

	nbp->btype = B_INPQ;
	nbappend(nbp, q);
	return 1;

merge:
	bcopy(nxmtod(nbp,caddr_t), tailend, nbp->len);
	tail->len += nbp->len;
	freenbuf(nbp);
	return 1;
}

/*
 * Return the first buffer on q to the freelist.
 */
qfreenbuf(q)
	register NBQ q;
{
	register NBP nbp;
	register s;

	LOCK;
	if ((nbp=q->head)==NULL) {
		goto out;
	} else {
		q->head = nbp->next;
		q->nbufs--;
	}
# ifdef IFXNS
	UNMBUFIZE(nbp);
# else  IFXNS
	nbp->data = nbp->info;
# endif IFXNS
	nbp->next = freenetbufs;
	freenetbufs = nbp;
	nbp->btype = N_BUF;
#ifdef	XTRACE
	nbp->len = 9988;			/* XXX debug for xstat */
#endif
	if (bwaiting) {
		wakeup((caddr_t)&bwaiting);
		bwaiting = 0;
	}
	nxfreecount++;
out:
	UNLOCK;
}

/*
 * return first buffer on q.
 */
NBP
qfirst(q)
	register NBQ q;
{
	register int s;
	register NBP nbp;

	LOCK;
	if ((nbp=q->head)==NULL)
		goto out;
	if ((q->head=nbp->next)==NULL) {
		q->tail = NULL;
		q->nbufs = 0;
	} else
		q->nbufs--;
	nbp->btype = N_BUF;
out:
	UNLOCK;
	return(nbp);
}


/* ----- msgbuf routines */
/*
 * getmsgbuf:
 *	- locate next host-to-exos msgbuf
 *	- return NULL if none available and nxintflag is true, otherwise
 *	  wait until a msgbuf is free
 *
 * status byte interpretation:
 *	3 - is busy and owned by exos
 *	2 - is busy and owned by host
 *	1 - is not busy but owned by exos
 *	0 - is available
 * XXX put this status byte stuff into constants
 */
MP
getmsgbuf()
{
	register MP mp;
	register int s;

	LOCK;
	for (;;) {
# ifdef IFXNS
		if (msgbufs_free > 0) {
			extern MP exgetcbuf();
			msgbufs_free --;
			mp = exgetcbuf(nx_xs);
			UNLOCK;
			return mp;
		}
# else  IFXNS
		mp = curxmp;
		if (msgbufs_free && (mp->msg.status == 0)) {
			msgbufs_free--;
			mp->msg.status = 2;
			mp->msg.res = 0;
			curxmp = mp->next;
			UNLOCK;
			return(mp);
		}
# endif IFXNS
		/* if we can't sleep, return a null */
		if (nxintflag) {
			UNLOCK;
			return NULL;
		}
		mwaiting++;
		sleep((caddr_t)&mwaiting, TTOPRI);
	}
	/* NOTREACHED */
}


putmsg(mp, nbp)
	register NxMsg *mp;
	NBP nbp;
{
	extern short nx_play_dead;

	USEPRI;

	mp->id = (long)nbp;
	if (nx_play_dead)
		return;

	LOCK;
	mp->status = MX_EXOS|MX_BUSY;
	*PORTB = 0;
	UNLOCK;
}

wputmsg(mp, nbp, pri)
	register NxMsg *mp;
	NBP nbp;
{
	extern short nx_play_dead;

	USEPRI;

	mp->id = (long)nbp;
	if (nx_play_dead)
		return;

	LOCK;
	mp->status = MX_EXOS|MX_BUSY;
	*PORTB = 0;
	sleep((caddr_t)nbp, pri);
	UNLOCK;
}


# ifndef IFXNS
linkbufs(bp, n, status)
	struct msgbuf *bp;
	int n;
	int status;
{
	register struct msgbuf *l, *p;

	/*
	 * intialize the list of host-to-exos buffers message buffers
	 */
	p = bp;
	l = p+n-1;
	while (--n >= 0) {
		l->next = p;
		l->msg.link = HTOX(p);
		l->msg.length = MSGLEN;
		l->msg.status = status;
		l = p;
		p++;
	}
}

/*----- */
/*
 * buzzon() --
 * wait for the given condition to change,
 * up to N milliseconds.
 */
int
buzzon(cp, mask, val, ms)
	register unsigned char *cp;
	int mask, val;
	int ms;
{
	while (--ms != 0) {
		if ((*cp&mask) != val)
			return 1;
		msdelay(1);
	}

	return 0;
}

/*
 * sendlong:
 *	- send a long to the excelan over its PORTB
 */
static
sendlong(l)
	register long l;
{
	register int i;

	for (i = sizeof l; --i >= 0;) {
		while (*PORTB & NX_READY_)
			;
		*PORTB = (u_char)l;
		l >>= BITSPERBYTE; 
	}
}
# endif  IFXNS
