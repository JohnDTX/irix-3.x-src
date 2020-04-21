# undef  DEBUG
# undef RAWIB		/* THIS DOESN'T WORK ANYMORE */
/*
 * ib_rawib.c --
 * raw ib driver.
 *
 * note that physio() is not killable.
 */

#include "ib.h"

#include "../h/param.h"
#include "../h/types.h"
#include "../h/dir.h"
#include "../h/signal.h"
#include "../h/user.h"
#include "../h/errno.h"
#include "../h/buf.h"

# ifdef DEBUG
# undef DEBUG
# define DEBUG ib_debug
# endif DEBUG
#include "../h/ib_ioctl.h"
#include "../gpib/ib_ieee.h"
#include "../gpib/ib_reg.h"
#include "../gpib/ib.defs"
#include "../gpib/ib_dbg.h"

# ifdef RAWIB
# ifdef MBMAPGET
THISDOESNOTWORK
# endif MBMAPGET

extern struct ibconn *ibconnp[];


struct buf ribbuf[NIB];		/* buf passed to physio() */
struct buf qibbuf[NIB];		/* buf passed to cooked ib driver */
# define qtor(bp) ((struct buf*)\
	((char*)ribbuf+((char*)bp-(char*)qibbuf)))
# define rtoq(bp) ((struct buf*)\
	((char*)qibbuf+((char*)bp-(char*)ribbuf)))

ibropen(dev,rwflag)
    dev_t dev;
    int rwflag;
{
    _ibopen(dev,RAWF);
}

ibrclose(dev)
    dev_t dev;
{
    _ibclose(dev,RAWF);
}

ibrread(dev)
    dev_t dev;
{
    extern ibrstrat();

    register struct ibconn *cp;

    cp = ibconnp[minor(dev)];
    if( !(cp->c_if->ibflags&STARTEDF) )
    {
	u.u_error = ENXIO;
	return;
    }
    physio(ibrstrat,ribbuf+unitnum(minor(dev)),dev,B_READ,2);
}

ibrwrite(dev)
    dev_t dev;
{
    extern ibrstrat();

    register struct ibconn *cp;

    cp = ibconnp[minor(dev)];
    if( !(cp->c_if->ibflags&STARTEDF) )
    {
	u.u_error = ENXIO;
	return;
    }
    physio(ibrstrat,ribbuf+unitnum(minor(dev)),dev,B_WRITE,2);
}

/*
 * ibrstrat() --
 * called by physio(), with one of ribbuf[];
 * translates the request to the corresponding qibbuf[],
 * and calls the "real" strategy routine.
 */
ibrstrat(rbp)
    register struct buf *rbp;
{
    register struct buf *qbp;
    register struct ibconn *cp;

dprintf((" ibrstrat([%d %d])",minor(rbp->b_dev),rbp->b_bcount));
    qbp = rtoq(rbp);
    cp = ibconnp[minor(rbp->b_dev)];
    DEV(qbp) = cp->c_dev;
    KVADDR(qbp) = KVADDR(rbp);
# ifdef MBMAPGET
    MBVADDR(qbp) = MBVADDR(rbp);
# endif MBMAPGET
    qbp->b_bcount = rbp->b_bcount;

    BFLAGS(qbp) = BQ_START|BQ_END | BQ_WANTED|BQ_PHYS;
    qbp->b_head = qbp->b_tail = 0;
    if( BFLAGS(rbp)&B_READ )
    {
	USE(qbp) = LSTNU;
	TSLOT(qbp) = cp->c_Tslot;
	LMAP(qbp) = (1<<MYSLOT);
    }
    else
    {
	USE(qbp) = TALKU;
	TSLOT(qbp) = MYSLOT;
	LMAP(qbp) = cp->c_Lmap;
    }
    ibstrategy(cp,qbp);
}

/*
 * ibrdone() --
 * done routine for raw requests.
 * given a completed request via one of qibbuf[],
 * translate into the corresponding ribbuf[].
 */
ibrdone(qbp)
    register struct buf *qbp;
{
    register struct buf *rbp;

    rbp = qtor(qbp);
dprintf((" ibrdone([%d %d/%d]",DEV(qbp),qbp->b_resid,qbp->b_bcount));
dprintf((" flags==$%x",BFLAGS(qbp)));
    BFLAGS(rbp) &= ~(B_ERROR|B_DONE);
    BFLAGS(rbp) |= BFLAGS(qbp)&(B_ERROR|B_DONE);
    rbp->b_resid = rbp->b_bcount - qbp->b_bcount;
    wakeup((char*)rbp);
}

# else  RAWIB

/* ARGSUSED */
ibropen(dev, flag) dev_t dev; int flag; { }

/* ARGSUSED */
ibrclose(dev) dev_t dev; { }

/* ARGSUSED */
ibrread(dev) dev_t dev; { }

/* ARGSUSED */
ibrwrite(dev) dev_t dev; { }

/* ARGSUSED */
ibrstrat(bp) struct buf *bp; { }

/* ARGSUSED */
ibrdone(bp) struct buf *bp; { }

# endif RAWIB
