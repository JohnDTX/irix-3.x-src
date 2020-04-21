#define XTRACE	1
#include <stdio.h>
#include <nlist.h>			/* for nlist */
#include <sys/types.h>
#include <sys/tty.h>
#include <xns/if_xns.h>

#ifdef	notdef
#include <xns/Xns.h>
#include <xns/Xnsconn.h>
#include <xns/nx.h>
#include <xns/Xnstrace.h>
#endif

struct	xtrace xtracebuf[NXCELLS];
struct	mqueues mqueues;
struct	mqueues *mp;
long	mqueues_va;
#define	RSIZE 20
struct	xns_route rtab[RSIZE];
struct	tty nx_tty[NDEV];
struct	xnsmisc xnsmisc[NDEV];
char	nxreads;
struct	netbuf *freenetbufs;

struct	nlist nl[] = {
	{ "_xtp" },
#define	NL_XTP		0
	{ "_xtbuf" },
#define	NL_XTBUF	1
	{ "_nxva" },
#define	NL_NXVA		2
	{ "_xns_rtab" },
#define	NL_RTAB		3
	{ "_curxmp" },
#define	NL_XMP		4
	{ "_nx_tty" },
#define NL_TTY		5
	{ "_xnsmisc" },
#define	NL_XNSMISC	6
	{ "_nxreads" },
#define	NL_NXREADS	7
	{ "_freenetbufs" },
#define	NL_NETBUFS	8
	{ "" },
};

short	prqueues;		/* if 1, print out queue info */
short	prtrace;		/* if 1, print out trace info */
short	prbufs;			/* if 1, print out netbuf info */
short	prtab;			/* if 1, print xns route info */
short	prdev;			/* if 1, print special file info */
short	cltrace;		/* if 1, clear tracing info from kernel */
short	zero = 0;		/* global which always contains 0 */
short	prconn;			/* if 1, print connection structures */
short	doall;			/* if 1, print everything we can */
short	prstats;		/* if 1, print out board stats */
short	doboink;		/* if 1, pop board */
char	*progname;		/* name of this program */
char	*namelist = "/vmunix";	/* namelist for symbols */
int	memfd;			/* fd kmem is open on */
struct	msgbuf *curxmp;

main(argc, argv)
	int argc;
	char *argv[];
{
	register short i;

    /* process command line arguments */

	progname = argv[0];
	if (argc == 1)
		goto usage;
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
			  case 'n':
				if (argv[i][2])
					namelist = &argv[i][2];
				else {
					if (i < (argc - 1))
						namelist = argv[++i];
					else
						goto usage;
				}
				break;
			  case 'a':
				doall = 1;		/* print EVERYTHING */
				break;
			  case 'q':			/* print nx queues */
				prqueues = 1;
				break;
			  case 't':			/* print nxtrace */
				prtrace = 1;
				break;
			  case 'b':			/* print netbufs */
				prbufs = 1;
				break;
			  case 'r':			/* print route table */
				prtab = 1;
				break;
			  case 'z':			/* zero trace buffer */
				cltrace = 1;
				break;
			  case 'd':			/* print out devs */
				prdev = 1;
				break;
			  case 'c':			/* print out conns */
				prconn = 1;
				break;
			  case 'B':
				doboink = 1;
				break;
			  case 's':
				prstats = 1;
				break;
			  default:
usage:
	    fprintf(stderr, "Usage:\n\t%s [-r|-b|-z|-d|-c|-t|-q|-n namelist]\n",
			    progname);
				exit(-1);
			}
		}
	}
	if (doboink) {
		memfd = open("/dev/ttyn0", 0);
		ioctl(memfd, NXBOINK, 0);
		close(memfd);
		printf("boink\n");
	}

    /* try to read namelist from kernel */

	nlist(namelist, nl);
	if (nl[0].n_type==0) {
		fprintf(stderr,
			"%s: \"%s\" has no namelist (or doesn't exist)\n",
			progname, namelist);
		exit(-1);
	}

    /* read in appropriate info from kmem */

	if (cltrace)
		memfd = open ("/dev/kmem", 2);
	else
		memfd = open ("/dev/kmem", 0);
	if (memfd == -1) {
		fprintf(stderr, "%s: unable to open /dev/kmem\n", progname);
		exit(-1);
	}
	if (prtrace || doall) {
		lseek(memfd, (long)nl[NL_XTP].n_value, 0);
		read(memfd, &xtp, sizeof xtp);
		lseek(memfd, (long)nl[NL_XTBUF].n_value, 0);
		read(memfd, xtracebuf, sizeof xtracebuf);
	}
	if (prqueues || prbufs || prconn || doall) {
		lseek(memfd, (long)nl[NL_NXVA].n_value, 0);
		read(memfd, &mqueues_va, sizeof mqueues_va);
		lseek(memfd, (long)mqueues_va, 0);
		read(memfd, &mqueues, sizeof mqueues);
		lseek(memfd, (long)nl[NL_XMP].n_value, 0);
		read(memfd, &curxmp, sizeof curxmp);
		lseek(memfd, (long)nl[NL_NXREADS].n_value, 0);
		read(memfd, &nxreads, sizeof nxreads);
		lseek(memfd, (long)nl[NL_NETBUFS].n_value, 0);
		read(memfd, &freenetbufs, sizeof freenetbufs);
	}
	if (prtab || doall) {
		lseek(memfd, (long)nl[NL_RTAB].n_value, 0);
		read(memfd, rtab, sizeof rtab);
	}
	if (prdev || doall) {
		lseek(memfd, (long)nl[NL_TTY].n_value, 0);
		read(memfd, nx_tty, sizeof nx_tty);
		lseek(memfd, (long)nl[NL_XNSMISC].n_value, 0);
		read(memfd, xnsmisc, sizeof xnsmisc);
	}

    /* print out results */

	if (prtrace || doall)
		do_trace();
	if (prqueues || doall)
		do_queues();
	if (prbufs || doall)
		do_bufs();
	if (prtab || doall)
		do_rtab();
	if (prdev || doall)
		do_dev();
	if (prconn || doall)
		do_conns();
	if (prstats || doall)
		do_stats();

	/*
 	 * Clear out tracing data structures
	 */
	if (cltrace) {
		register short i;

		lseek (memfd, (long)nl[NL_XTP].n_value, 0);
		for (i = 0; i < sizeof xtp/2; i++)
			write (memfd, &zero, 2);
		lseek (memfd, (long)nl[NL_XTBUF].n_value, 0);
		for (i = 0; i < sizeof xtracebuf/2; i++)
			write (memfd, &zero, 2);
	}
}

/*
 * do_trace:
 *	- print out results of trace
 */
do_trace()
{
	register struct xtrace *p;
	register x;

	x = xtp+1;
	if (x>=NXCELLS)
		x = 0;
	do {
		p = &xtracebuf[x];
		if (p->code)
			xprint(x, p);
		x++;
		if (x>=NXCELLS)
			x = 0;
	} while (x!=xtp);
}

/*
 * getstring:
 *	- given a kernel address, read the string out of /dev/kmem
 *	- read up to 500 characters or a null, whichever comes first
 */
char *
getstring(p)
	char *p;
{
	register short i;
	register int nb;
	char c;
	static char buf[500];

	lseek(memfd, (long)p, 0);		/* seek to string */
	for (i = 0; i < 500; i++) {
		nb = read(memfd, &c, 1);
		if (nb <= 0)
			break;
		buf[i] = c;
		if (c == 0)
			break;
	}
	return buf;
}

xprint(x, p)
	register struct xtrace *p;
{
	static long lastime;
	long t;

	t = p->time;
	if (t!=lastime)
		printf("    (%ld)\n", p->time);
	printf("[%d]\t", x);
	lastime = t;
	printf(getstring(p->code), p->p0, p->p1, p->p2, p->p3, p->p4);
	printf("\n");
}

/*
 * do_queues:
 *	- print out information about message queues, and other relevant
 *	  stuff
 */

#define	KOFFSET(x)	((long)(x) - (long)mp)
#define	KRN_ADDR(x)	(((long)x - (long)&mqueues) + mqueues_va)

do_queues()
{
	register short i;

	mp = &mqueues;
	printf("%06x(%04x): roffset: %04x\n%06x(%04x): woffset: %04x\n",
		    KRN_ADDR(&mp->roffset), KOFFSET(&mp->roffset),
		    (u_short)mp->roffset,
		    KRN_ADDR(&mp->woffset), KOFFSET(&mp->woffset),
		    (u_short)mp->woffset);
	printf("%06x(%04x): rhead: %06x(%04x)\n",
		    KRN_ADDR(&mp->rhead), KOFFSET(&mp->rhead),
		    mp->rhead, (long)mp->rhead - mqueues_va);
	printf("%06x(%04x): whead: %06x(%04x)\n",
		    KRN_ADDR(&mp->whead), KOFFSET(&mp->whead),
		    mp->whead, (long)mp->whead -  mqueues_va);
	printf("Recieve message queue: (nxreads=%d)\n", nxreads);
	printf("  addr offset  # mplink link status req reply     id   conp\n");
	for (i = 0; i < NRBUFS; i++) 
		do_message(&mp->rbufs[i], i);
	printf("Send message queue: curxmp 0x%x\n", curxmp);
	printf("  addr offset  # mplink link status req reply     id   conp\n");
	for (i = 0; i< NWBUFS; i++)
		do_message(&mp->wbufs[i], i);
}

do_message(mmp, bufnum)
	register struct msgbuf *mmp;
	int bufnum;
{
	printf("%6x %6x %2d %6x %4x %6x %3x %5x %6x %6x\n",
		    KRN_ADDR(mmp), KOFFSET(mmp), bufnum,
		    mmp->next, mmp->msg.link, (unsigned char)mmp->msg.status,
		    (unsigned char)mmp->msg.request,
		    (unsigned char)mmp->msg.reply,
		    mmp->msg.id, mmp->msg.block[7].addr);
}

do_bufs()
{
	register short i;
	register struct netbuf *nbp;

	printf("free 0x%x\n", freenetbufs);
printf("netbuf  #   data   len   seq   next   perm xpaddr btype dtype quack\n");
	nbp = &mqueues.netbufs[0];
	for (i = 0; i < NETBUFS; i++) {
		printf("%6x %2d %6x %5d %5d %6x %6x %6x %5x %5x %4x\n",
			    KRN_ADDR(nbp), i,
			    nbp->data, nbp->len, nbp->seq,
			    nbp->next, nbp->perm,
			    nbp->nxpaddr, nbp->btype, nbp->dtype,
			    nbp->quack);
		nbp++;
	}
}

do_rtab()
{
register i;
ROUTE r;

	printf("age\tlock\tname\n");
	for(i=0; i<RSIZE; i++) {
		r = &rtab[i];
		printf("%d\t%d\t%s\n", r->age, r->lock, r->name);
	}
}

/* computes kernel address of tty data structre */
#define	TP_ADDR(tp)	(((long)tp - (long)nx_tty) + nl[NL_TTY].n_value)
do_dev()
{
	register struct tty *tp;
	register struct xnsmisc *xp;
	register n;

	printf("tty + xnsmisc:\n");
	printf("      tp  # t_state x_conp x_socket x_state x_pend x_inpq\n");
	for(n = 0, tp = nx_tty, xp = xnsmisc; n < NDEV; n++, tp++, xp++) {
		printf("%8x %2d 0%6o %6x %8d %7x %6x %6x\n",
			    TP_ADDR(tp), n, (unsigned)tp->t_state,
			    xp->x_conp, xp->x_socket, xp->x_state,
			    xp->x_pend, xp->x_inpq.head);
	}
}

/* computes kernel address of data structure */
do_conns()
{
	register CONN conp;
	register n;

	printf("connections:\n");
	for(n=0, conp = &mqueues.connection[0]; n<NDEV; n++, conp++) {
	    printf("conp=%x idst=%x isrc=%x inpq=%x outq=%x pending=%x\n",
			    KRN_ADDR(conp), conp->header.dstid,
			    conp->header.srcid, conp->inpq.head,
			    conp->outq.head, conp->pending.head);
	    printf("trans=%x release=%x next=%x hbusy=%d asend=%d\n",
			     conp->trans, conp->release, conp->next,
			     conp->hbusy, conp->asend);
	    printf("blocked=%d pgrp=%d nextseq=%d lastseq=%d rackno=%d\n",
			       conp->blocked, conp->c_pgrp, conp->nextseq,
			       conp->lastseq, conp->rackno);
	    printf("ralloc=%d allocno=%d rseq=%d nextrseq=%d rtime=%d\n",
			      conp->ralloc, conp->allocno,
			      conp->rseq, conp->nextrseq, conp->rtime);
	    printf("ttime=%d sockin=%d state=%d ntries=%d mode=%d dir=%d\n",
			     conp->ttime, conp->sockin, conp->state,
			     conp->ntries, conp->mode, conp->dir);
	    printf("hwaiting=%d paddr=%x route=%x utp=%x\n\n",
				conp->hwaiting, conp->paddr, conp->route,
				conp->utp);
	}
}

do_stats()
{
	struct nxstat x;
	int fd;

	fd = open("/dev/ttyn0", 0);
	if (ioctl(fd, NXSTATS, &x) < 0) {
		printf("NXSTATS failed\n");
		return;
	}

	fflush(stdout);
	printf("Board stats:\n");
	printf("okxmit=%d aborts=%d tdr=%d okrcv=%d misaligned=%d\n",
			  x.nxs_okxmit, x.nxs_aborts,
			  x.nxs_tdr, x.nxs_okrcv,
			  x.nxs_misaligned);
	printf("crc=%d lost=%d\n", x.nxs_crc, x.nxs_lost);
}
