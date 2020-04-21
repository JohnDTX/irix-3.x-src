# undef  DEBUG
#include "ib.h"

#include "../h/param.h"
#include "../h/types.h"
#include "../h/dir.h"
#include "../h/signal.h"
#include "../h/proc.h"
#include "../h/user.h"
#include "../h/errno.h"
#include "../h/buf.h"
#include "../h/termio.h"

# ifdef DEBUG
# undef DEBUG
# define DEBUG ib_subr_debug
# endif DEBUG
#include "../h/ib_ioctl.h"
#include "../gpib/ib_ieee.h"
#include "../gpib/ib_reg.h"
#include "../gpib/ib.defs"
#include "../gpib/ib_dbg.h"

extern struct ibconn *ibconnp[];

/*
 * _ibioctl() --
 * internal i/o control routine for ib driver.
 *
	cp		ibconn pointer for this minor dev
	cmd		the cmd arg to the ioctl
	arg		the args arg to the ioctl (in user space)
 *
 */
_ibioctl(cp, cmd, arg)
    register struct ibconn *cp;
    int cmd;
    caddr_t arg;
{
    short lunit;
    register struct ibvars *vp;
    union
    {
	struct poke poke1;
	struct sgnode sg1;
	struct ioreq ioreq1;
	struct tlist t1;
	struct ibconn c1;
	struct { char stats[MAXIBSLOTS]; } poll;
	int i;
    }	j;
    struct ibnode *np;
    dev_t dev;
    char *ap;
    int iii;
USEPRI;

# define COPYIN(x) \
	if( copyin(arg,(char*)&(x),sizeof (x)) ) \
	{ u.u_error = EFAULT; return; }

# define COPYOUT(x) \
	if( copyout((char*)&(x),arg,sizeof (x)) ) \
	{ u.u_error = EFAULT; return; }

    vp = cp->c_if;

    switch(cmd)
    {
    case FIONREAD:
	j.i = ibqcount(&cp->c_rq, &iii);
	if( !iii )	/* END? */
	    j.i += ibqcount(vp->raq, &iii);
	COPYOUT(j.i);
	break;

    case IBIOINIT:
	if( !suser() )
	    return;
RAISE;
	tlc_abort(RP);
	tlc_reset(RP);
	ibdone(vp);
	Qrelseq(&vp->ioq);
	Qrelseq(&vp->freeq);
	vp->nfreebufs = 0;
	Qrelseq(&vp->raq);
	dev = cp->c_dev;
	cp = vp->baseconn;
	for( j.i = NIBCONN; --j.i >= 0; )
	{
	    Qrelseq(&cp->c_rq);
	    cp++;
	}
	ibcinit(vp);

	ibinit(vp);
LOWER;
	ibopen(dev,0);
	break;

    case IBIOGETNODE:
	COPYIN(j.sg1);
	lunit = j.sg1.slotno;
	if( (unsigned)lunit >= MAXIBSLOTS )
	{
	    u.u_error = EINVAL;
	    return;
	}
	arg += ( (char*)&j.sg1.node - (char*)&j.sg1 );
	COPYOUT(vp->ibnodes[lunit]);
	break;

    case IBIOSETNODE:
	if( !suser() )
	    return;
	if( vp->ibflags&STARTEDF )
	{
	    u.u_error = ENXIO;
	    return;
	}
	COPYIN(j.sg1);
	lunit = j.sg1.slotno;
	if( (unsigned)lunit >= MAXIBSLOTS
	 || (unsigned)j.sg1.node.n_talkslot >= MAXIBSLOTS )
	{
	    u.u_error = EINVAL;
	    return;
	}
	j.sg1.node.n_tag1 &= ~(TALK_BASE|LSTN_BASE);
	j.sg1.node.n_tag2 &= ~(TALK_BASE|LSTN_BASE);
	j.sg1.node.n_talkresp = 0;
	j.sg1.node.n_lstnresp = 0;
	j.sg1.node.n_tctlresp = 0;
	j.sg1.node.n_erroresp = 0;
	j.sg1.node.n_idleresp = 0;
	j.sg1.node.n_ppr
		= (j.sg1.node.n_ppr&PPR_REPLYF)|PPR_SSENSE;
	vp->ibnodes[lunit] = j.sg1.node;
	break;

    case IBIOSTART:
	if( !suser() )
	    return;
	COPYIN(j.i);
	if( !j.i )
	{
	    vp->ibflags &= ~STARTEDF;
	}
	else
	{
	    if( !(vp->ibflags&STARTEDF) )
		ibstartup(vp);
	    vp->ibflags |= STARTEDF;
	}
	break;

    case IBIOPASSCTL:
	if( !suser() )
	    return;
	COPYIN(j.i);
	if( (unsigned)j.i >= MAXIBSLOTS
	 || !(vp->ibnodes[j.i].n_flags&IBN_VALID) )
	{
	    u.u_error = EINVAL;
	    return;
	}
	ibfreeze(vp);
	ibcdone(vp->baseconn);
	ibdocmd(vp,PASSCTLU,(1<<j.i));
	ibthaw(vp);
	break;

    case IBIOTAKECTL:
	if( !suser() )
	    return;
	ibfreeze(vp);
	ibcdone(vp->baseconn);
	ibdocmd(vp,TAKECTLU,0);
	ibthaw(vp);
	break;

    case IBIOPPU:
    case IBIOPPC:
	if( !suser() )
	    return;
	COPYIN(j.i);
	ibdocmd(vp,cmd==IBIOPPC?PPEU:PPDU,j.i);
	break;

    case IBIOREN:
    case IBIOGTL:
	if( !suser() )
	    break;
	COPYIN(j.i);
	ibdocmd(vp,cmd==IBIOREN?RENU:GTLU,j.i);
	break;

    case IBIOGETEV:
RAISE;
	while( !RP->events )
	{
	    vp->ibflags |= EVWANTF;
	    SLEEP(&RP->events,IBPRI);
	}
	j.i = RP->events;
	RP->events = 0;
LOWER;
	COPYOUT(j.i);
	break;

    case IBIOPOLL:
RAISE;
	while( !(RP->events&CSR_SRQI) )
	{
	    vp->ibflags |= EVWANTF;
	    SLEEP(&RP->events,IBPRI);
	}
	RP->events &= ~CSR_SRQI;
	ap = j.poll.stats+0;
	np = vp->ibnodes+0;
	for( iii = MAXIBSLOTS; --iii >= 0; )
	{
	    *ap++ = np->n_pollstat;
	    np->n_pollstat &= ~SPMR_RSV;
	    np++;
	}
LOWER;
	if( copyout(j.poll.stats, arg, sizeof j.poll.stats) )
	{ u.u_error = EFAULT; return; }
	break;

    case IBIOSRQ:
	COPYIN(j.i);
	ibdocmd(vp,SRQU,j.i);
	break;

    case IBIOCUTOFF:
	ibdocmd(vp,UNCONNECTU,0);
	break;

    case IBIOFLUSH:
	ibiflush(cp);
	break;

    case IBIOLOCK:
	if( !suser() )
	    return;
	COPYIN(j.i);
	if( j.i )
	    vp->ibflags |= LOCKF;
	else
	    vp->ibflags &= ~LOCKF;
	break;

    case IBIOPEEK:
	if( !suser() )
	    return;
	COPYIN(j.i);
	if( (unsigned)j.i >= NIBIREGS )
	{
	    u.u_error = EINVAL;
	    return;
	}
	j.i = tlc_inreg(RP,j.i);
	COPYOUT(j.i);
	break;

    case IBIOPOKE:
	if( !suser() )
	    return;
	COPYIN(j.poke1);
	if( (unsigned)j.poke1.f >= NIBOREGS )
	{
	    u.u_error = EINVAL;
	    return;
	}
	tlc_outreg(RP,j.poke1.f,j.poke1.v);
	break;

# ifdef IBIOTEST
    case IBIOTEST:
	if( !suser() )
	    return;
	ibdocmd(vp,TESTU,0);
	break;
# endif IBIOTEST

    case IBIOINTR:
	if( !suser() )
	    return;
RAISE;
	tlc_strobe(RP);
	tlcruintr(RP);
LOWER;
	break;

    default:
	u.u_error = EINVAL;
	return;
    }
}

/*
 * ibstartup() --
 *
 * - initialize the iface, enable i/o.
 *
	vp		ptr to globals for the iface being started up

 *
 */
ibstartup(vp)
    register struct ibvars *vp;
{
    register int iii;
    register struct ibnode *np;
USEPRI;
    dprintf((" ibstartup($%x)",vp->baseconn->c_dev));
RAISE;
    np = vp->ibnodes+0;
    for( iii = 0; iii < MAXIBSLOTS; iii++ )
	RP->tags[iii] = np++ ->n_tag1;

    RP->Cflags &= ~(TLC_SC|TLC_AINIT|TLC_SRQ);
    iii = vp->ibnodes[MYSLOT].n_flags;
    if( iii&IBN_SC )
	RP->Cflags |= TLC_SC;
    if( iii&IBN_AINIT )
	RP->Cflags |= TLC_AINIT;
    if( iii&IBN_SRQ )
	RP->Cflags |= TLC_SRQ;

    RP->iregb = REGB_MAG;
    if( iii&IBN_NOTRI )
	RP->iregb &= ~REGB_TRI;
    RP->irege = REGE_MAG;
    RP->icr1 = RP->Cflags&TLC_SC?CR1_MAG|CR1_SC:CR1_MAG;
    RP->ppbit = RP->ippr = vp->ibnodes[MYSLOT].n_ppr;
    if( !(iii&IBN_PPE) )
	RP->ppbit = PPR_UNC;
    if( iii&IBN_PPC )
	RP->ppbit = 0;
    RP->ispmr = vp->ibnodes[MYSLOT].n_idleresp;

    RP->Cflags |= TLC_VALID;
    tlccmd(RP,INITU);
LOWER;
}

/*
 * ibcabort() --
 * called to kill i/o for a given connection.
 * it is ASSUMED that any process sleeping on
 * completion of the i/o has already been disabled.
 *
 * - flush i/o pending for this ibconn,
 * preparatory to closing.
 *
	cp		ibconn ptr for the dev being closed
 *
 */
ibcabort(cp)
    register struct ibconn *cp;
{
    register struct ibvars *vp;
    register struct buf *bp;
    char aborted;
USEPRI;

    aborted = 0;
    vp = cp->c_if;
RAISE;
    Qrelseq(&cp->c_rq);
    /*
     * just mark anything still in the queue.
     * really abort the current request if it
     * hasn't finished.
     */
    for( bp = vp->ioq.bq_head; bp != 0; bp = bp->b_tail )
	if( DEV(bp) == cp->c_dev && !(BFLAGS(bp)&BQ_ASYNC) )
	{
	    if( bp == cp->c_curbuf )	/*if not completed, handle here*/
		cp->c_curbuf = 0;
	    aborted = 1;
	    BFLAGS(bp) |= BQ_ABORT|BQ_END;
	    BFLAGS(bp) &= ~BQ_WANTED;
	}
    if( (bp = vp->curbuf) != 0 )
    if( DEV(bp) == cp->c_dev && !(BFLAGS(bp)&BQ_ASYNC) )
    {
	extern int ib_flabort_flag;
	if( ib_flabort_flag || !RP->atdone )
	{
	    tlc_abort(RP);
	}
    }

    if( (bp = cp->c_curbuf) != 0 )
    {
	Qrelse(bp);	/*completed, but consumer may be kaput*/
	cp->c_curbuf = 0;
    }

    if( aborted )
	tlcruintr(RP);
LOWER;
}

/*
 * ibfreeze() --
 *
 * - get control of all ibconn's.
 *
	vp		ptr to globals for the iface being frozen
 *
 */
ibfreeze(vp)
    register struct ibvars *vp;
{
    register int iii;
    register struct ibconn *cp;

    cp = vp->baseconn;
    for( iii = 0; iii < NIBCONN; iii++ )
    {
	ibcwait(cp,PRIBIO);
	ibcabort(cp);
	cp++;
    }
}

/*
 * ibthaw() --
 *
 * - give up all ibconn's.
 *
	vp		ptr to globals for the iface being thawed
 *
 */
ibthaw(vp)
    register struct ibvars *vp;
{
    register int iii;
    register struct ibconn *cp;

    cp = vp->baseconn;
    for( iii = 0; iii < NIBCONN; iii++ )
    {
	ibcdone(cp);
	cp++;
    }
}

/*
 * ibiflush() --
 *
 * - flush input on the receive q.
 *
	cp		ibconn ptr for the dev being flushed
 *
 */
ibiflush(cp)
    register struct ibconn *cp;
{
    register struct ibvars *vp;
USEPRI;

    vp = cp->c_if;
    Qfreeq(&vp->raq);
RAISE;
    Qfreeq(&cp->c_rq);
    vp->nbufs = 0;
LOWER;
}

/*
 * ibinit() --
 *
 * - (re-)initialise driver variables.
 *
	vp		ptr to globals for iface being inited
 *
 */
ibinit(vp)
    register struct ibvars *vp;
{
    register int iii;

    /*init abstract device (.base, .unit already set)*/
    {
	extern int ibuintr();

	RP->Tslot = -1;
	RP->func = ibuintr;
	RP->funcarg = (int)vp;
	RP->Lmap = 0;
	RP->Cflags &= (TLC_RUN|TLC_GONG);
	RP->error = 0;
	RP->cic = 0;
	RP->present = 1;
	RP->ppbit = RP->ippr = 0;
	RP->spmr = RP->ispmr = 0;
	RP->icr1 = CR1_MAG;
	RP->cmdalarm = RP->retryalarm = 0;
	RP->events = 0;

	RP->atclkint = 0;
	RP->atgonged = 0;
	RP->atdone = 1;
	for( iii = 0; iii < MAXIBSLOTS; iii++ )
	    RP->tags[iii] = iii;

	RP->csr = RP->lcsr = 0;
	tlc_strobe(RP);
    }

    /*init iface vars (.tlc already set)*/
    {
	vp->ibflags = 0;
	vp->curbuf = 0;

	vp->use = IDLEU;
	vp->udev = -1;
	vp->uTslot = 0;
	vp->uLmap = 0;

	Qinit(&vp->ioq);
	Qinit(&vp->raq);
	Qinit(&vp->freeq);
	vp->nfreebufs = 0;
	vp->maxfreebufs = 2;
	vp->nbufs = 0;
	vp->nconn = 0;
	vp->nrawconn = 0;
    }

    /*init ibnode info*/
    {
	register struct ibnode *np;

	np = vp->ibnodes+0;
	for( iii = 0; iii < MAXIBSLOTS; iii++ )
	{
	    np->n_tag1 = iii; np->n_tag2 = 0;
	    np->n_flags = 0;
	    np->n_pollstat = 0;
	    np->n_talkresp = 0;
	    np->n_lstnresp = 0;
	    np->n_tctlresp = 0;
	    np->n_idleresp = 0;
	    np->n_erroresp = 0;
	    np->n_ppr = (iii&PPR_REPLYF)|PPR_SSENSE;
	    np->n_talkslot = 0;
	    np->n_lstnmap = (1<<0);
	    np++;
	}
    }
}

ibcinit(vp)
    register struct ibvars *vp;
{
    register struct ibconn *cp;
    register char cdev;
    register int iii;

    cp = vp->baseconn;
    cdev = maketlc(RP->unit,0);
    vp->basedev = cdev;
    for( iii = 0; iii < NIBCONN; iii++ )
    {
	cp->c_if = vp;
	cp->c_flags = 0;
	Qinit(&cp->c_rq);
	cp->c_dev = cdev;
	cp->c_Tslot = iii;
	cp->c_Lmap = (1<<iii);
	cp->c_curbuf = 0;
	ibconnp[cdev] = cp;
	cdev++;
	cp++;
    }
}

/*
 * ibdocmd() --
 *
 * - create a cmd buf and call ibstrategy().
 *
	vp		ptr to globals for iface being used
	cmd		what is wanted
	map		a cmd parameter
 *
 */
ibdocmd(vp,cmd,map)
    register struct ibvars *vp;
    int cmd;
    int map;
{
    extern struct buf *Qget();

    register struct ibconn *cp;
    register struct buf *bp;
USEPRI;

    if( !(vp->ibflags&STARTEDF) )
    {
	u.u_error = ENXIO;
	return;
    }

    cp = vp->baseconn;
    dprintf((" ibdocmd($%x,%d...)",cp->c_dev,cmd));
    ibcwait(cp,IBPRI);
    bp = Qget(cp->c_dev);
    USE(bp) = cmd;
    NPOLL(bp) = 0;
    POLLMASK(bp) = map;

    BFLAGS(bp) |= BQ_WANTED;
    cp->c_curbuf = bp;
    ibstrategy(cp,bp);
RAISE;
    while( !(BFLAGS(bp)&BQ_DONE) )
	SLEEP(bp,PRIBIO);		/*not killable*/
LOWER;

    if( cp->c_curbuf == bp )
    {
	if( BFLAGS(bp)&BQ_ERROR )
	    u.u_error = EIO;
	Qrelse(bp);
    }
    else
    {
	u.u_error = EIO;
    }
    cp->c_curbuf = 0;
    ibcdone(cp);
}


/*
 * ibcwait() --
 *
 * - get exclusive use of a minor device.
 *
	cp		ibconn ptr for the desired dev
	pri		sleep / wait pri to use if must sleep
 *
 */
ibcwait(cp,pri)
    register struct ibconn *cp;
    int pri;
{
    while( cp->c_flags&BUSYF )
    {
	cp->c_flags |= WAITF;
	SLEEP(cp,pri);
    }
    cp->c_flags |= BUSYF;
}

/*
 * ibcdone() --
 *
 * - give up use of a minor device.
 * wakeup any waiting.
 *
	cp		ibconn ptr for the dev being relinquished
 *
 */
ibcdone(cp)
    register struct ibconn *cp;
{
    if( cp->c_flags&WAITF )
	WAKEUP(cp);
    cp->c_flags &= ~(BUSYF|WAITF);
}

/*
 * nextvalid() --
 *
 * - find (index of) next valid ibnode slot
 * with matching flag.
 *
	vp		ptr to globals for iface being used
	n		(index of) first node to consider
	map		mask of possible nodes
	flag		flag which must match
 *
 */
int
nextvalid(vp,n,map,flag)
    register struct ibvars *vp;
    register int n;
    register int map,flag;
{
    register struct ibnode *np;

    flag |= IBN_VALID;
    map >>= n; np = vp->ibnodes+n;
    while( n < MAXIBSLOTS )
    {
	/*np->n_pollstat = np->n_idleresp&~SPMR_RSV;*/
	if( map&01 )
	if( (np->n_flags&flag) == flag )
	{
	    dprintf((" nxt %d",n));
	    return n;
	}
	map >>= 1; np++;
	n++;
    }
    dprintf((" nxt -1"));
    return-1;
}

/*
 * service() --
 *
 * - after a poll, queue up any requested i/o.
 * - cause wakeup of any waiting for poll info.
 *
	vp		ptr to globals for iface being serviced
 *
 */
service(vp)
    register struct ibvars *vp;
{
    register struct ibnode *np;
    register int iii;

    dprintf((" service"));
    for( iii = 0 , np = vp->ibnodes+0
	    ; iii < MAXIBSLOTS; iii++ , np++ )
    if( np->n_flags&IBN_VALID )
    {
	if( !(np->n_pollstat&SPMR_RSV) )
	    continue;
	dprintf((" SRQ%d -$%x-",iii,np->n_pollstat));

# ifdef NOISE
	if( np->n_pollstat == np->n_erroresp )
	{
	    dprintf((" -error%d",iii));
	}
	else
	if( np->n_pollstat == np->n_idleresp )
	{
	    dprintf((" -idle%d",iii));
	}
	else
	if( np->n_pollstat == np->n_talkresp )
	{
	    dprintf(("-talk%d",iii));
	}
	else
	if( np->n_pollstat == np->n_lstnresp )
	{
	    dprintf(("-lstn%d",iii));
	}
	else
	if( np->n_pollstat == np->n_tctlresp )
	{
	    dprintf(("-tctl%d",iii));
	}
	else
# endif NOISE

	{
	    RP->events |= CSR_SRQI;
	    continue;
	}
#ifdef	NOISE
	np->n_pollstat &= ~SPMR_RSV;
#endif	NOISE
    }
}

/*
 * ppmask() --
 *
 * - return mask of who might have done an SRQ
 * based on the current .pollstat .
 *
	vp		ptr to globals of iface being used
 *
 */
int
ppmask(vp)
    register struct ibvars *vp;
{
    register struct ibnode *np;
    short iii;
    register int pollstat,mask;

    dprintf((" ppmask"));
    np = vp->ibnodes+MAXIBSLOTS;
    pollstat = vp->ppollstat;
    mask = 0;
    for( iii = MAXIBSLOTS; --iii >= 0; )
    {
	np--;
	mask <<= 1;
	if( !(np->n_flags&IBN_VALID) )
	    continue;
	if( np->n_flags&IBN_SRQ )
	    /*
	     *automatically include slots
	     *that can't ppoll but can SRQ.
	     */
	    mask |= 01;
	if( np->n_flags&IBN_PPE )
	{
	    register int line;
	    /*
	     *if the sense of the bit
	     *corresp. to the current slot
	     *matches the asserted sense
	     *declared at integration time,
	     *add this to the mask.
	     */
	    line = np->n_ppr&PPR_REPLYF;
	    if( !( (pollstat>>line)&01 )
		    != !( np->n_ppr&PPR_SSENSE ) )
		mask &= ~01;
	}
    }
    if( mask == 00 ) mask = msk(MAXIBSLOTS);	/*kluge!*/

    dprintf(("$%x",mask));
    return mask;
}

/*
 * ibdone() --
 *
 * - mark buffer done, free it or wakeup.
 * always called at hi pri.
 *
	vp		ptr to globals for iface being used
 *
 */
ibdone(vp)
    register struct ibvars *vp;
{
    register struct buf *bp;

    if( (bp = vp->curbuf) == 0 )
	return;

    dprintf((" ibdone"));
    vp->curbuf = 0;
    Qdel(&vp->ioq,bp);
    if( BFLAGS(bp)&BQ_PHYS )
    {
	ibrdone(bp);
	return;
    }
    if( !(BFLAGS(bp)&BQ_WANTED) )
    {
	Qfree(bp);
	return;
    }
    BFLAGS(bp) |= BQ_DONE;
    BFLAGS(bp) &= ~BQ_WANTED;
    WAKEUP(bp);
}

int
ibqcount(qp, _eofflag)
    register struct bufq *qp;
    int *(_eofflag);
{
    register struct buf *bp;
    register int n;
    USEPRI;

    *_eofflag = 0;
    n = 0;
    RAISE;
    for( bp = qp->bq_head; bp != 0; bp = bp->b_tail )
    {
	n += bp->b_bcount;
	if( BFLAGS(bp)&BQ_END )
	{
	    *_eofflag = 1;
	    break;
	}
    }
    LOWER;
    return n;
}
