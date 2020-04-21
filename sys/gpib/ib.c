# undef  DEBUG
#include "ib.h"
/*
 * ib.c --
 * driver for NI's GPIB-796 board.
 * provides Talker, Listener, and Controller
 * functions (dependant on initialization).
 *
 * does not use SRQ.
 * when not CIC, talks / listens only to the CIC.
 *
 * must be configured via ioctl's before
 * reading or writing, eg with iib(1).
 */

/*
 * NOTE on GPIB-796 configuration:
 * mandatory jumper settings
	e12-e11		(16-bit port addressing)
	int-5		(intr level 5)
	e4-e5		(serial DMA priority resolution)
	e14-e15		(drives CBRQ_ when required)
	e8-e9		(does not drive LOCK_)
 *
 * cable shield ground option:
	yes:		e1-e2
	no:		e2-e3	(normal case)
 *
 * base address dipswitches:
	board 0:	0000 0000 0010	(normal case)
	board 1:	0000 0001 0000
 */

#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "machine/cpureg.h"

# ifdef DEBUG
# undef DEBUG
# define DEBUG ib_debug
# endif DEBUG
#include "../h/ib_ioctl.h"
#include "../gpib/ib_ieee.h"
#include "../gpib/ib_reg.h"
#include "../gpib/ib.defs"
#include "../gpib/ib_dbg.h"


# ifdef DEBUG
/* debug flags */
int ib_debug = 1;
int ib_q_debug = 0;
int ib_tlc_debug = 1;
int ib_probe_debug = 0;
int ib_machdep_debug = 0;
int ib_subr_debug = 1;
int ib_dbg_debug = 1;
int ib_board_debug = 0;
int ib_trc_debug = 1;
int ib_print_debug = 0;
int ib_hoff_debug = 1;
# endif DEBUG


extern struct buf *Qpop(),*Qget(),*Qalloc();


/* non-debug flags and parameters */
int ib_ppoll_flag = 1;		/*whether or not to do ppolls*/
int ib_srq_mask = msk(MAXIBSLOTS);/*additional mask for spolls*/
int ib_flabort_flag = 0;	/*whether to abort on flush*/
int ib_alert_flag = 0;		/*whether or not to force alert*/


/*
 * ibvars[] --
 * globals per board.
 */
struct ibvars ibvars[NIB];

/*
 * ibconn[] --
 * variables and queues per minor dev.
 */
struct ibconn ibconn[maketlc(NIB,0)];
struct ibconn *ibconnp[maketlc(NIB,0)];		/*ptrs to ibconn structs*/


/*
 * ibopen() --
 * cooked device open routine.
 *
	dev		major / minor of the dev being opened
	rwflag		read / write flag (ignored)
 *
 */
/* ARGSUSED */
ibopen(dev,rwflag)
    dev_t dev;
    int rwflag;
{
    _ibopen(dev,COOKEDF);
}

/*
 * ibclose() --
 * cooked device close routine.
 *
	dev		major / minor of the dev being opened
 *
 */
ibclose(dev)
    dev_t dev;
{
    _ibclose(dev,COOKEDF);
}

/*
 * _ibopen() --
 * common device open routine
 * for raw and cooked devs.
 *
 * - keep track of first open, whether raw or
 * cooked, on each ibconn.
 * - keep track of total number of open ibconns,
 * total number of raw open ibconns, total number
 * of open controllers.
 * - allocate buffers for internal use.
 * - start timer on first open.
 *
	dev		major / minor of the dev being opened
	rcflag		raw / cooked flag
 *
 */
_ibopen(dev,rcflag)
    dev_t dev;
    int rcflag;
{
    register int conn;
    register struct ibvars *vp;
    register struct ibconn *cp;

    dprintf((" _ibopen($%x)",minor(dev)));
    if( (unsigned)(conn = minor(dev)) >= maketlc(NIB,0) )
    {
	u.u_error = ENXIO;
	return;
    }
    cp = ibconnp[conn]; vp = cp->c_if;
    if( !RP->present )
    {
	u.u_error = ENXIO;
	return;
    }

    if( rcflag == RAWF )
	if( !(cp->c_flags&RAWF) )
	    vp->nrawconn++;
    cp->c_flags |= rcflag;

    if( cp->c_flags&OPENF )
	return;

    cp->c_flags |= OPENF;

    /*first open for this minor dev*/
    vp->maxfreebufs++;
    Qfree(Qget(cp->c_dev));

    if( vp->nconn++ > 0 )
	return;

    /*first open for this controller*/
    tlcopen(RP);

    Qfree(Qget(cp->c_dev));
}

/*
 * _ibclose() --
 * common device close routine for
 * raw and cooked devs.
 *
 * - keep track of total number of open ibconns,
 * total number of raw open ibconns, total number
 * of open controllers.
 * - deallocate excess internal buffers.
 * - on last close, deallocate readahead buffers,
 * - and stop timer.
 *
	dev		major / minor of the dev being closed
	rcflag		raw / cooked flag
 *
 */
_ibclose(dev,rcflag)
    dev_t dev;
    int rcflag;
{
    register int conn;
    register struct ibvars *vp;
    register struct ibconn *cp;
    struct buf *bp;
USEPRI;

    dprintf((" _ibclose($%x)",minor(dev)));
    conn = minor(dev);
    cp = ibconnp[conn]; vp = cp->c_if;

    if( rcflag == RAWF )
	vp->nrawconn--;
    cp->c_flags &= ~rcflag;
    if( cp->c_flags&(COOKEDF|RAWF) )
	return;

RAISE;
    /*clean up minor dev*/
    ibcabort(cp);
    ibcdone(cp);	/*just in case*/
    Qfreeq(&cp->c_rq);
    cp->c_flags = 0;

    vp->maxfreebufs--;
    if( (bp = Qalloc(cp->c_dev)) != 0 )
	Qrelse(bp);
LOWER;
    vp->nconn--;
    if( vp->nconn > 0 )
	return;

    tlcclose(RP);

    vp->nconn = 0;
    vp->ibflags &= STARTEDF;
    vp->nfreebufs = 0;
    Qrelseq(&vp->freeq);
RAISE;
    Qrelseq(&vp->raq);
    vp->nbufs = 0;
LOWER;
}

/*
 * ibread() --
 * device read routine.
 *
	dev		major / minor of the dev being read from
 *
 */
ibread(dev)
    dev_t dev;
{
    iblisten(ibconnp[minor(dev)]);
}

/*
 * ibwrite() --
 * device write routine.
 *
	dev		major / minor of the dev being written to
 *
 */
ibwrite(dev)
    dev_t dev;
{
    ibtalk(ibconnp[minor(dev)]);
}

/*
 * ibioctl() --
 * device i/o control routine.
 *
	dev		major / minor of the dev being tweaked
	cmd		the cmd arg to the ioctl
	arg		the args arg to the ioctl (in user space)
	mode		(ignored)
 *
 */
/* ARGSUSED */
ibioctl(dev,cmd,arg,mode)
    dev_t dev;
    int cmd;
    caddr_t arg;
    int mode;
{
    dprintf((" ibioctl($%x,%c,...)",minor(dev),cmd&msk(8)));

    _ibioctl(ibconnp[minor(dev)], cmd, arg);
}


/*
 * ibuintr() --
 * called from abstract-intr routine tlcuintr().
 * always called at hi pri.
 *
 * - advance i/o queue.
 *   - call ibustop() to clean up the previous request if any.
 *   - call ibscan() to scan for the next request.
 *   - call ibustart() to start it.
 *   - possibly wakeup event sleepers.
 *
	vp		ptr to globals for the iface that got the intr
 *
 */
ibuintr(vp)
    register struct ibvars *vp;
{
    extern struct buf *ibscan();
    register struct buf *bp;

    if( !(vp->ibflags&STARTEDF) )
	return;
    dprintf((" ibuintr[$%x]",vp->ibflags));

    if( vp->use != IDLEU )
    {
	ibustop(vp);
    }

    while( vp->use == IDLEU )
    {
	RP->retryalarm = 0;
	if( (bp = ibscan(vp)) == 0 )
	{
	    dprintf(("-no-work"));
	    break;
	}
	ibustart(vp,bp);
    }

    if( vp->ibflags&EVWANTF && RP->events )
    {
	vp->ibflags &= ~EVWANTF;
	WAKEUP(&RP->events);
    }
}

/*
 * ibstrategy() --
 * called from read / write / ioctl routines.
 *
 * - put a buf on the tail of the i/o queue.
 *
	cp		ibconn ptr for the dev
	bp		buf ptr for the i/o request
 *
 */
ibstrategy(cp,bp)
    register struct ibconn *cp;
    register struct buf *bp;
{
    register struct ibvars *vp;
USEPRI;

    dprintf((" ibstrategy($%x)",cp->c_dev));
RAISE;
    vp = cp->c_if;
    Qque(&vp->ioq,bp);

    tlcruintr(RP);
LOWER;
}

/*
 * ibtalk() --
 * called from ibioctl(), ibwrite().
 * parameters occur in the u.
 *
 * - load buffers and call ibstrategy().
 *
	cp		ibconn ptr for the dev being talked to
 *
 */
ibtalk(cp)
    register struct ibconn *cp;
{
    register struct buf *bp;
    struct ibvars *vp;
    char started,swabin,swabout;
    int icount;
USEPRI;

    icount = u.u_count;
    dprintf((" ibtalk($%x [%d])",cp->c_dev,icount));
    vp = cp->c_if;
    if( !(vp->ibflags&STARTEDF) )
    {
	u.u_error = ENXIO;
	return;
    }
    ibcwait(cp,IBPRI);
    swabin = (vp->ibnodes[MYSLOT].n_flags&IBN_SWAB) != 0;
    swabout = (vp->ibnodes[cp->c_Tslot].n_flags&IBN_SWAB) != 0;

    started = 0;
    while( u.u_count > 0 )
    {
	bp = Qget(cp->c_dev);
	USE(bp) = TALKU;
	TSLOT(bp) = MYSLOT;
	LMAP(bp) = cp->c_Lmap;
	bp->b_bcount = MIN(u.u_count,IBBUFSIZE);
	if( !started )
	{
	    started++;
	    BFLAGS(bp) |= BQ_START;
	}
	iomove(KVADDR(bp), (int)bp->b_bcount, B_WRITE);
	if( u.u_error )
	{
	    Qrelse(bp);
	    break;
	}
	if( swabin )
	{
	    if( !swabout )
		ibswab((short *)KVADDR(bp), (int)bp->b_bcount);
	    else
		if( bp->b_bcount&01 )
		    ibswab((short *)(KVADDR(bp)+bp->b_bcount-1), 2);
	}
	if( u.u_count == 0 )
	    BFLAGS(bp) |= BQ_END;
	BFLAGS(bp) |= BQ_WANTED;
	cp->c_curbuf = bp;
	ibstrategy(cp,bp);
RAISE;
	while( !(BFLAGS(bp)&BQ_DONE) )
	    SLEEP(bp,IBPRI);
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
	if( u.u_error )
	    break;
    }

    ibcdone(cp);
    dprintf((" talkret($%x %d)",cp->c_dev,u.u_error?-1:icount-u.u_count));
}

/*
 * iblisten() --
 * called from ibioctl(), ibread().
 * parameters occur in the u.
 *
 * - call ibstrategy() and wait for bufs
 * to appear in the .c_rq .
 *
	cp		ibconn ptr for the dev being listened to
 *
 */
iblisten(cp)
    register struct ibconn *cp;
{
    register struct buf *bp;
    register struct ibvars *vp;
    char swabin,swabout;
    register int nc,bflags,boff,bleft;
    int icount;
USEPRI;

    icount = u.u_count;
    dprintf((" iblisten($%x [%d])",cp->c_dev,icount));
    vp = cp->c_if;
    if( !(vp->ibflags&STARTEDF) )
    {
	u.u_error = ENXIO;
	return;
    }

    swabin = (vp->ibnodes[MYSLOT].n_flags&IBN_SWAB) != 0;
    swabout = (vp->ibnodes[cp->c_Tslot].n_flags&IBN_SWAB) != 0;

    while( u.u_count > 0 )
    {
	bp = Qget(cp->c_dev);	/*preallocate since must be at lo pri*/
RAISE;
	if( cp->c_rq.bq_head == 0 )
	{
	    /*queue the listen request*/
	    USE(bp) = LSTNU;
	    DEV(bp) = cp->c_dev;
	    TSLOT(bp) = cp->c_Tslot;
	    LMAP(bp) = (1<<MYSLOT);
	    BFLAGS(bp) |= BQ_START|BQ_QLSTN;	/*use buffered data first*/
	    ibstrategy(cp,bp);
	}
	else
	{
	    Qrelse(bp);				/*didn't need it*/
	}
	bp = Qpop(&cp->c_rq);
	if( BFLAGS(bp)&BQ_ERROR )
	{
	    /*don't reuse bad buffer*/
	    u.u_error = EIO;
	    bp->b_bcount = 1;
	}
	bflags = BFLAGS(bp);
	boff = bp->b_resid;
	bleft = bp->b_bcount;
	nc = MIN(u.u_count,bleft);
	bp->b_resid += nc;
	bp->b_bcount -= nc;
	if( bp->b_bcount > 0 )
	    Qpush(&cp->c_rq,bp);
	if( boff == 0 )
	if( swabin )	/*ibswab whole buffer on first use*/
	{
	    if( !swabout )
		ibswab((short *)KVADDR(bp), bleft);
	    else
		if( bleft&01 )
		    ibswab((short *)(KVADDR(bp)+bleft-1), 2);
	}
	if( !u.u_error )
	    iomove(KVADDR(bp)+boff,nc,B_READ);
	if( bp->b_bcount <= 0 )
	    Qfree(bp);
LOWER;
	if( bflags&BQ_END )
	    break;
	if( u.u_error )
	    break;
    }

    dprintf((" listenret($%x %d)",cp->c_dev,u.u_error?-1:icount-u.u_count));
}

/*
 * ibscan() --
 * called from ibuintr().
 * - scan the .ioq for work; create work
 * (using internal buffers) when necessary.
 * return ptr to "next" buf, 0 if none.
 *
	vp		ptr to globals for the iface being scanned
 *
 */
struct buf *
ibscan(vp)
    register struct ibvars *vp;
{
    register struct buf *bp;

    if( !RP->cic )
    {
	char la,ta;

	ta = (RP->csr&(CSR_SPMS|CSR_ATN_|CSR_TA|CSR_ORI))
		== (CSR_ATN_|CSR_TA|CSR_ORI);

	bp = 0;
	/*
	 *(not CIC) scan for a command compatible
	 *with bus state.
	 *
	 *note that LSTNU with flags&BQ_QLSTN can
	 *be satisfied (and must be, to prevent
	 *running out of bufs) when there is any
	 *buffered data in the .raq, regardless
	 *of addressed status.  such LSTNU's always
	 *have to get data from .raq when not CIC.
	 *
	 *kluge - it doesn't interrupt if
	 *the ATN_ condition happens after
	 *the ADSC interrupt.  it SHOULD
	 *interrupt for the DI or DO bit,
	 *but it doesn't always.  so use
	 *.retryalarm to check periodically.
	 */
	for( bp = vp->ioq.bq_head; bp != 0; bp = bp->b_tail )
	    if( BFLAGS(bp)&BQ_ABORT )
	    {	return bp; }
	    else
	    if( USE(bp) == TALKU )
	    {
		if( ta ) return bp;
		/*kluge, the controller doesn't interrupt sometimes*/
		if( ib_alert_flag || RP->csr&CSR_TA )
		    tlc_alert(RP);
	    }
	    else
	    if( USE(bp) == LSTNU )
	    {
		if( BFLAGS(bp)&BQ_QLSTN )
		    if( vp->raq.bq_head != 0 ) return bp;
		if( BFLAGS(bp)&BQ_PHYS ) return bp;
	    }
	    else
	    if( USE(bp) == TAKECTLU )
	    {	if( RP->csr&CSR_CIC ) return bp; }
	    else
	    { 	return bp; }

	if( vp->nconn <= 0 || vp->nrawconn > 0 )
	    return bp;

	/*input ready?  start dma (to .raq)*/
	la = (RP->csr&(CSR_SPMS|CSR_ATN_|CSR_LA|CSR_IRI))
		== (CSR_ATN_|CSR_LA|CSR_IRI) || RP->csr&CSR_ENDI;
	if( la )
	{
	    /* XXX */
	    if( vp->nbufs > vp->maxfreebufs
	     || (bp = Qalloc(vp->basedev)) == 0 )
	    {
		/*hope to find a free buf later*/
		tlc_alert(RP);
		return bp;
	    }
	    USE(bp) = LSTNU;
	    BFLAGS(bp) |= BQ_ASYNC;
	    bp->b_bcount = IBBUFSIZE;
	    dprintf((" qasync"));
	    Qpush(&vp->ioq,bp);
	    return bp;
	}
	/*kluge, the controller doesn't interrupt sometimes*/
	if( RP->csr&CSR_LA )
	    tlc_alert(RP);
    }
    else
    {
	bp = vp->ioq.bq_head;

	if( vp->nconn <= 0 )
	    return bp;

	/*
	 *(am CIC) generate fake requests as needed,
	 *then take next cmd from .ioq.
	 *
	 *if got an SRQ, start a poll as soon as possible.
	 */
	if( RP->csr&CSR_SRQI )
	{
	    if( (bp = Qalloc(vp->basedev)) == 0 )
	    {
		/*hope to find a free buf later*/
		tlc_alert(RP);
		return 0;
	    }
	    USE(bp) = ib_ppoll_flag?PPOLLU:SPOLLU;
	    NPOLL(bp) = 0;
	    POLLMASK(bp) = ib_srq_mask;
	    BFLAGS(bp) |= BQ_ASYNC;
	    RP->csr &= ~CSR_SRQI;
	    dprintf((" qpoll"));
	    Qpush(&vp->ioq,bp);
	    return bp;
	}
    }

    return bp;
}

/*
 * ibustop() --
 * called from ibuintr().
 *
 * - find current buffer if any,
 *   - check for doneness.
 *   - remove from the .ioq, or restart.
 *
	vp		ptr to globals for the iface being stopped
 *
 */
ibustop(vp)
    register struct ibvars *vp;
{
    register struct buf *bp;
    register struct ibconn *cp;

    /*tentatively assume done*/
    vp->use = IDLEU;

    if( (bp = vp->curbuf) == 0 )
	return;

    dprintf((" ibustop($%x) %d",DEV(bp),USE(bp)));
    if( BFLAGS(bp)&BQ_ABORT )
	RP->error = 1;

    if( RP->error )
	BFLAGS(bp) |= BQ_ERROR;

    switch(USE(bp))
    {
    case TALKU:
    case LSTNU:
	BFLAGS(bp) |= BQ_DONE;
	bp->b_resid = bp->b_bcount - RP->tc;
	bp->b_bcount = RP->tc;
	dprintf((" -%s %d,",USE(bp)==TALKU?"sent":"got",bp->b_bcount));
	ifdebug((BFLAGS(bp)&BQ_PHYS?0:prdata(KVADDR(bp),bp->b_bcount)));
	if( bp->b_bcount > IBBUFSIZE )
	    RP->error = 1;
	if( bp->b_resid != 0
	 && (USE(bp) == TALKU || !RP->endi) )
	    RP->error = 1;
	if( USE(bp) == LSTNU && BFLAGS(bp)&BQ_PHYS )
	    RP->endi = 1;
	if( RP->error )
	{
	    dprintf((" -ioerr"));
	    BFLAGS(bp) |= BQ_ERROR;
	    RP->endi = 1;
	}
	if( USE(bp) == TALKU )
	{
	    if( BFLAGS(bp)&BQ_END )
		RP->endi = 1;
	}
	else
	{
	    cp = ibconnp[DEV(bp)];
	    bp->b_resid = 0;
	    if( RP->endi )
		BFLAGS(bp) |= BQ_END;
	    if( vp->nconn > 0 )
	    if( !(BFLAGS(bp)&(BQ_WANTED|BQ_ABORT))
	     && (bp->b_bcount > 0 || !(BFLAGS(bp)&BQ_ASYNC)) )
	    {
		Qdel(&vp->ioq,bp);
		vp->curbuf = 0;		/* so ibdone() won't free */
		if( BFLAGS(bp)&BQ_ASYNC )
		    Qque(&vp->raq,bp) , vp->nbufs++;
		else
		    Qque(&cp->c_rq,bp);
	    }
	}
	if( RP->endi )
	{
	    dprintf((" end"));
	}
	RP->endi = 0;
	ibdone(vp);

	break;

    case PPOLLU:
	if( !(RP->error) )
	    vp->ppollstat = RP->dir;
	dprintf((" -$%x-",vp->ppollstat));
	USE(bp) = SPOLLU;
	POLLMASK(bp) = ppmask(vp);
	NPOLL(bp) = 0;
	/*continue with this buf on the .ioq*/
	break;

    case SPOLLU:
	if( !(RP->error) )
	{
	    register struct ibnode *np;
	    np = vp->ibnodes+NPOLL(bp);
	    if( RP->dir&SPMR_RSV )
		np->n_pollstat = RP->dir;
	}
	dprintf((" -$%x-",RP->dir));
	/*continue with this buf on the .ioq*/
	break;

    case SERVU:
	ibdone(vp);
	service(vp);
	break;

    case SRQU:
	tlc_pioinit(RP);
	ibdone(vp);
	break;

    case TAKECTLU:
	if( !(RP->csr&CSR_CIC) )
	    BFLAGS(bp) |= BQ_ERROR;
	else
	    RP->cic = 1;
	ibdone(vp);
	break;

    case PASSCTLU:
	if( RP->csr&CSR_CIC )
	    BFLAGS(bp) |= BQ_ERROR;
	else
	    RP->cic = 0;
	ibdone(vp);
	break;

    case UNCONNECTU:
	ibdone(vp);
	break;

    case PPEU:
    case PPDU:
    case RENU:
    case GTLU:
	/*continue with this buf on the .ioq*/
	break;

# ifdef TESTU
    case TESTU:
	ibdone(vp);
	break;
# endif TESTU

    default:
	dprintf(("-garbage-use%d",USE(bp)));
	ibdone(vp);
	break;
    }
    RP->cmdalarm = 0;
}

/*
 * ibustart() --
 * called from ibuintr().
 *
 * - start work on a buffer.
 *
	vp		ptr to globals for the iface being started
	bp		buf ptr for the i/o being started
 *
 */
ibustart(vp,bp)
    register struct ibvars *vp;
    register struct buf *bp;
{
    register int nxt;

    dprintf((" ibustart($%x) %d",DEV(bp),USE(bp)));
    vp->curbuf = bp;
    if( BFLAGS(bp)&BQ_ABORT )
    {
	dprintf(("-discard"));
	ibdone(vp);
	return;
    }

    /*tentatively assume it will start ok (return)*/
    vp->udev = DEV(bp);
    vp->use = USE(bp);

    switch(vp->use)
    {
    case TALKU:
	RP->Tslot = TSLOT(bp);
	RP->Lmap = LMAP(bp);
	RP->maddr = (long)MBVADDR(bp);
	RP->vaddr = (long)KVADDR(bp);
	RP->bc = bp->b_bcount;
	tlccmd(RP,BFLAGS(bp)&BQ_END?ETALKU:TALKU);
	return;

    case LSTNU:
	if( BFLAGS(bp)&BQ_QLSTN && vp->raq.bq_head != 0 )
	{
	    /*
	     *use buffered data first.
	     *already checked that Qpop()
	     *won't hang here.
	     */
	    dprintf((" unbuf"));
	    Qque(&ibconnp[DEV(bp)]->c_rq,Qpop(&vp->raq));
	    vp->nbufs--;
	    break;
	}

	vp->uTslot = TSLOT(bp);
	vp->uLmap = LMAP(bp);
	RP->Tslot = TSLOT(bp);
	RP->Lmap = LMAP(bp);
	RP->maddr = (long)MBVADDR(bp);
	RP->vaddr = (long)KVADDR(bp);
	RP->bc = IBBUFSIZE;
	tlccmd(RP,vp->use);
	return;

    case PPOLLU:
	if( !RP->cic )
	{
	    BFLAGS(bp) |= BQ_ERROR;
	    break;
	}
	tlccmd(RP,vp->use);
	return;

    case SPOLLU:
    case SERVU:
	if( !RP->cic )
	{
	    BFLAGS(bp) |= BQ_ERROR;
	    break;
	}
	if( (nxt = nextvalid(vp,
		(int)NPOLL(bp)+1, (int)POLLMASK(bp), IBN_SRQ)) <= 0 )
	{
	    USE(bp) = vp->use = SERVU;
	    tlccmd(RP,SERVU);
	    return;
	}
	NPOLL(bp) = nxt;
	RP->Tslot = nxt;
	RP->Lmap = (1<<MYSLOT);
	tlccmd(RP,SPOLLU);
	return;

    case SRQU:
	RP->spmr = POLLMASK(bp);	/*kluge*/
	tlccmd(RP,vp->use);
	return;

    case TAKECTLU:
	if( RP->cic )
	    break;
	tlccmd(RP,vp->use);
	return;

    case PASSCTLU:
	if( !RP->cic )
	{
	    BFLAGS(bp) |= BQ_ERROR;
	    break;
	}
	if( (nxt = nextvalid(vp,
		(int)NPOLL(bp)+1, (int)POLLMASK(bp), 0)) <= 0 )
	    break;
	NPOLL(bp) = nxt;
	RP->Tslot = nxt;
	tlccmd(RP,vp->use);
	return;

    case UNCONNECTU:
	if( !RP->cic )
	{
	    BFLAGS(bp) |= BQ_ERROR;
	    break;
	}
	RP->Tslot = -1;
	RP->Lmap = 0;
	tlccmd(RP,CONNECTU);
	return;

    case PPEU:
    case PPDU:
	if( !RP->cic )
	{
	    BFLAGS(bp) |= BQ_ERROR;
	    break;
	}
	if( (nxt = nextvalid(vp,
		(int)NPOLL(bp)+1, (int)POLLMASK(bp), IBN_PPC)) <= 0 )
	    break;
	{
	    register struct ibnode *np;
	    NPOLL(bp) = nxt;
	    np = vp->ibnodes+nxt;
	    if( vp->use == PPEU )
		np->n_flags |= IBN_PPE;
	    else
		np->n_flags &= ~IBN_PPE;

	    RP->Tslot = -1;
	    RP->Lmap = (1<<nxt);
	    RP->ppbit = np->n_ppr;
	    tlccmd(RP,vp->use);
	    return;
	}

    case RENU:
    case GTLU:
	if( !RP->cic )
	{
	    BFLAGS(bp) |= BQ_ERROR;
	    break;
	}
	if( (nxt = nextvalid(vp,
		(int)NPOLL(bp)+1, (int)POLLMASK(bp), 0)) <= 0 )
	    break;
	{
	    register struct ibnode *np;
	    NPOLL(bp) = nxt;
	    np = vp->ibnodes+nxt;

	    RP->Tslot = -1;
	    RP->Lmap = (1<<nxt);
	    RP->ppbit = np->n_ppr;
	    tlccmd(RP,vp->use);
	    return;
	}

# ifdef TESTU
    case TESTU:
	break;
# endif TESTU

    default:
	dprintf(("-garbage-use%d",vp->use));
	break;
    }

    /*(failed to start)*/
    vp->use = IDLEU;
    ibdone(vp);
}
