#
/* # define IDEBUG		/* additional tlc debugging */
#include "ib.h"
/*
 * ib_tlc.c --
 * low-level device control routines for ib driver.
 * these implement the "abstract device" used by
 * other parts of the driver.  the basic idea is:
 * set up "abstract device" regs, then call tlccmd();
 * eventually tlcuintr() will call ibuintr().
 * status is passed back through the "abstract csr"
 * which tries to compensate for the flakeyness of
 * the real status regs.
 */

#include "../h/param.h"
#include "../h/buf.h"
#include "../vm/vm.h"
#include "machine/cpureg.h"
#include "../h/ib_ioctl.h"

# ifdef DEBUG
# undef DEBUG
# define DEBUG ib_tlc_debug
# endif DEBUG
#include "../gpib/ib_ieee.h"
#include "../gpib/ib_reg.h"
#include "../gpib/ib.defs"
#include "../gpib/ib_dbg.h"


/* csr bits which are flakey (volatile) */
# define CSR_FLAKE (0x0FFFF|CSR_DMADONE)

/* csr bits which correspond to "events" */
# define CSR_EVENT (CSR_LOCHGI|CSR_RMTCHGI|CSR_LO|CSR_RMT\
	|CSR_DCI|CSR_DETI)

/* STROBE() -- macro to strobe in abstract csr */
# define STROBE(R) (\
	R = _INREG(IB_SR) ,\
	R <<= NBBY , R |= _INREG(IB_ADSR) ,\
	R <<= NBBY , R |= _INREG(IB_ISR2) ,\
	R <<= NBBY , R |= _INREG(IB_ISR1) ,\
	R)

/* FUDGE() -- macro to compensate for flakey csr bits */
# define FUDGE(R) (\
	(R&(CSR_ENDI|CSR_ADCHGI) ?(\
		(!(R&CSR_TA) ?rp->csr &= ~CSR_ORI :0),\
		(!(R&CSR_LA) ?rp->csr &= ~CSR_IRI :0),\
		((R&CSR_ENDI)?rp->csr |= CSR_DMADONE :0)) :0),\
	rp->csr &= CSR_FLAKE,\
	rp->csr |= R)

/* CODE() -- macro to add one automaton instr */
# define CODE(c,a,b) (xp->opcode=(c),xp->opa=(a),xp->opb=(b),rp->atcnt++,xp++)

# define CLRWAITOP(a,b)	CODE(XCWAIT,a,b)
# define WAITOP(a,b)	CODE(XWAIT,a,b)
# define STOREOP(a,b)	CODE(XSTORE,a,b)
# define FETCHOP(a)	CODE(XFETCH,a,0)
# define MASKOP(a,b)	CODE(XMASK,a,b)
# define FUNCOP(a,b)	CODE(XFUNC,(long)(a),(long)(b))

# define BCLROP(a)	CODE(XMASK,~(a),0)
# define BWAITOP(a)	CODE(XWAIT,a,a)

/* store step */
# define SSTEP(cm,rr,rv) (CLRWAITOP(cm,cm), STOREOP(rr,rv))

int tlc_initopen = 0;		/*enable init on open*/
int tlc_forceconn = 0;		/*enable force connection every time*/
int tlc_disclose = 1;		/*enable disconnect on close*/
int tlc_fhsopen = 1;		/*enable fhs on open*/

int TLCMAXBUZZ = 3;		/*# max buzz loop in tlcinterp()*/
int TLCTICKS = 30;		/*# system ticks per interval*/

int TLCRETRYTIME = 2;		/*# intervals between retries */
int TLCTIMEOUT = 2;		/*# intervals before timeout*/
int TLCSLAVETIMEOUT = 4;	/*# extra intervals for non-CIC*/
int TLCDEBUGTIME = 6;		/*# extra intervals for debugging*/

int tlc_poll_time = 2;		/*# extra intervals for polling*/
int tlc_talk_time = 8;		/*# extra intervals for talking*/
int tlc_lstn_time = 16;		/*# extra intervals for listening*/
int tlc_connect_time = 2;	/*# extra intervals for connect*/
int tlc_act_time = 1;		/*# extra intervals to go active ctl*/
int tlc_gtsb_time = 1;		/*# extra intervals to go inactive ctl*/
int tlc_tctl_time = 100;	/*# extra intervals for tctl*/
int tlc_pctl_time = 100;	/*# extra intervals for passing ctl*/
int tlc_srq_time = 20;		/*# extra intervals for srq*/

int tlc_func_ticks = 1;		/*# system ticks per func interval*/


/*
 * tlcp[] --
 * pointers to abstract device pseudo regs.
 */
struct tlc *tlcp[NIB];

tlcopen(rp)
    register struct tlc *rp;
{
    register char *ip = rp->base;
USEPRI;

    dprintf((" tlcopen"));
    rp->Cflags |= TLC_RUN;
    rp->Cflags &= ~(TLC_CONNECT|TLC_TALK|TLC_LSTN);

    if( !(rp->Cflags&TLC_GONG) )
    {
	dprintf((" startgong"));
	tlcgong(rp);
    }

    if( !(rp->Cflags&TLC_VALID) )
	return;

RAISE;
    /* optionally re-init on open */
    if( tlc_initopen || rp->Cflags&TLC_AINIT )
	tlccmd(rp,INITU);

    /* optionally fhs on open.  don't if already addressed */
    if( tlc_fhsopen )
	if( !(rp->csr&(CSR_LA|CSR_TA|CSR_CIC)) )
	{
	    tlc_pioinit(rp);
	    _OUTREG(IB_AUXMR,AUXMR_OP|OP_FHS);
	}
LOWER;
}

tlcclose(rp)
    register struct tlc *rp;
{
    register char *ip = rp->base;
USEPRI;
    dprintf((" tlcclose"));

    if( !(rp->Cflags&TLC_VALID) )
	return;

RAISE;
    /* optionally disconnect on close */
    if( tlc_disclose )
    {
	if( rp->csr&CSR_CIC )
	{
	    rp->Lmap = 0;
	    rp->Tslot = -1;
	    tlccmd(rp,CONNECTU);
	    while( !rp->atdone )
	    {
		tlcuintr(rp);
		delay(1);
	    }
	    if( rp->error )
		tlccmd(rp, INITU);
	}
	if( !(rp->csr&(CSR_LA|CSR_TA|CSR_CIC)) )
	{
	    tlc_pioinit(rp);
	    _OUTREG(IB_AUXMR,AUXMR_OP|OP_FHS);
	}
    }
LOWER;
    rp->Cflags &= ~(TLC_RUN|TLC_CONNECT|TLC_TALK|TLC_LSTN);
}


/*
 * tlccmd() --
 * GO routine for abstract device.
 * the caller has already set up various
 * abstract device pseudo-regs.
 *
 * - setup for tlcinterp() if necessary (the
 * caller will somehow call tlcinterp() sometime).
 * - setup for fast / slow timers if necessary.
 *
	rp		ptr to abstract dev pseudo-regs
	cmd		command
 *
 */
tlccmd(rp,cmd)
    register struct tlc *rp;
    int cmd;
{
    rp->csr &= ~(CSR_ERRI|CSR_NXM);
    rp->icmd = cmd;

    TLC_org(rp);

    switch(cmd)
    {
    case INITU:
	tlc_reset(rp);
	tlc_auxinit(rp);
	tlc_setaddr(rp,rp->itag);
	tlc_pioinit(rp);
	tlc_ifc(rp);
	break;

    case TALKU:
    case ETALKU:
    case LSTNU:
	if( rp->bc <= 0 )
	{
	    rp->error = 1;
	    break;
	}
	if( rp->cic )
	{
	    if( tlc_forceconn )
	    {
		TLC_takectl(rp);
		TLC_connect(rp,-1,0);
	    }

	    if( !(rp->Cflags&TLC_CONNECT
	     && rp->Lmap == rp->iLmap
	     && rp->Tslot == rp->iTslot) )
	    {
		TLC_takectl(rp);
		TLC_connect(rp,(int)rp->Tslot,(int)rp->Lmap);
	    }
	    TLC_standby(rp);
	}
	if( cmd == LSTNU )
	{
	    /*
	     * KLUGE -
	     * take care of the special condition
	     * where the board has already handshaked
	     * the data...
	     */
	    if( rp->csr&CSR_ENDI )
	    {
		tlc_unstash(rp);
		break;
	    }
	}

	TLC_startdma(rp,rp->maddr,(unsigned)rp->bc,cmd);
	break;

    case PPOLLU:
	TLC_takectl(rp);
	TLC_ppoll(rp);
	break;

    case SPOLLU:
	TLC_takectl(rp);
	TLC_sstep(rp,0,IB_DOR,SPE_CMD);
	TLC_connect(rp,(int)rp->Tslot,(int)(1<<MYSLOT));
	TLC_standby(rp);
	TLC_getspoll(rp);
	TLC_takectl(rp);
	TLC_sstep(rp,0,IB_DOR,SPD_CMD);
	rp->Cflags &= ~(TLC_CONNECT|TLC_TALK|TLC_LSTN);
	break;

    case SERVU:
	/*unspoll*/
	TLC_takectl(rp);
	rp->cmdalarm += tlc_poll_time;
	TLC_sstep(rp,0,IB_DOR,SPD_CMD);
	break;

    case SRQU:
	TLC_srq(rp,rp->spmr);
	break;

    case TAKECTLU:
	TLC_takectl(rp);
	TLC_connect(rp,-1,0);
	rp->Cflags &= ~(TLC_CONNECT|TLC_TALK|TLC_LSTN);
	break;

    case PASSCTLU:
	TLC_takectl(rp);
	TLC_connect(rp,rp->Tslot,0);
	TLC_passctl(rp);
	rp->Cflags &= ~(TLC_CONNECT|TLC_TALK|TLC_LSTN);
	break;

    case CONNECTU:
	if( !rp->cic )
	    break;
	TLC_takectl(rp);
	TLC_connect(rp,(int)rp->Tslot,(int)rp->Lmap);
	break;

    case PPEU:
    case PPDU:
	TLC_takectl(rp);
	TLC_connect(rp,-1,(int)rp->Lmap);
	rp->cmdalarm += tlc_poll_time;
	if( cmd == PPEU )
	{
	    TLC_sstep(rp,CSR_CRI,IB_DOR,PPC_CMD);
	    TLC_sstep(rp,CSR_CRI,IB_DOR,(int)SCGMSG(rp->ppbit));
	}
	else
	{
	    TLC_sstep(rp,CSR_CRI,IB_DOR,PPD_CMD);
	}
	rp->Cflags &= ~(TLC_CONNECT|TLC_TALK|TLC_LSTN);
	break;

    case RENU:
    case GTLU:
	TLC_takectl(rp);
	TLC_connect(rp,-1,(int)rp->Lmap);
	rp->cmdalarm += tlc_poll_time;
	if( cmd == RENU )
	    TLC_sstep(rp,0,IB_AUXMR,AUXMR_OP|OP_SREN);
	else
	    TLC_sstep(rp,CSR_CRI,IB_DOR,GTL_CMD);
	TLC_standby(rp);
	break;

# ifdef TESTU
    case TESTU:
	break;
# endif TESTU

    case RESETU:
	tlc_reset(rp);
	break;

    case ABORTU:
    case PEEKU:
    case UNCONNECTU:
    case POKEU:
    case IFCU:
    case TRIGU:
    case DCLRU:
    default:
	dprintf((" unimpl-cmd%d",cmd));
	break;
    }
}

/*
 * tlcintr() --
 * called at hi pri by system, at intr time.
 *
 * - poll, and call unit intr routines.
 */
int
tlcintr()
{
    register long icsr;
    register int unit;
    register char *ip;
    register struct tlc *rp;
    int didsomething;

    dprintf(("\n{{"));
    didsomething = 0;
    for( unit = 0; unit < NIB; unit++ )
    {
	rp = tlcp[unit];
	if( rp == 0 || !(rp->present) )
	    continue;
	ip = rp->base;
	icsr = _INREG(IB_SR);
	if( icsr&SR_INTR_ && icsr&SR_DMAFIN_ )
	    continue;

	didsomething = 1;
	/*like STROBE(icsr);*/
	icsr <<= NBBY , icsr |= _INREG(IB_ADSR) ,
	icsr <<= NBBY , icsr |= _INREG(IB_ISR2) ,
	icsr <<= NBBY , icsr |= _INREG(IB_ISR1);
	FUDGE(icsr);
	/*kluge - an intr puts off pending timeout*/
	if( rp->cmdalarm )
	    rp->cmdalarm++;
	tlcuintr(rp);
    }
    dprintf((" }}\n"));
    return didsomething;
}

/*
 * tlcruintr() --
 * always called at hi pri (already).
 * strobe in current csr, then call the
 * unit interrupt routine.  this is like
 * the "start" routine of some drivers.
 */
tlcruintr(rp)
    register struct tlc *rp;
{
    register char *ip = rp->base;
    register long icsr;

    STROBE(icsr);
    FUDGE(icsr);
    tlcuintr(rp);
}

/*
 * tlcuintr() --
 * abstract device unit interrupt routine.
 * called at hi pri to advance abstract
 * device state.
 *
 * - keep track of state changes.
 * - call tlcinterp() if necessary.
 * - call ibuintr() if necessary.
 */
tlcuintr(rp)
    register struct tlc *rp;
{
    dprintf((" *%d[$%08x] $%04x",rp->unit,rp->csr,rp->Cflags));
    ifdebug((ib_q_debug++,ib_subr_debug++));

    /*keep track of events*/
    if( rp->csr&CSR_LOCHGI )
	rp->events &= ~CSR_LO;
    if( rp->csr&CSR_RMTCHGI )
	rp->events &= ~CSR_RMT;
    rp->events |= rp->csr&(CSR_EVENT|CSR_ATN_|CSR_LA|CSR_TA);
    rp->csr &= ~CSR_EVENT;

    /*
     *keep going until there's nothing else to do,
     *or assured of getting another intr.
     */
    for( ;; )
    {
	if( rp->csr&(CSR_ERRI|CSR_NXM) )
	{
	    rp->csr &= ~(CSR_ERRI|CSR_NXM);
	    rp->error = 1;
	}

	/*advance i/o queue, possibly starting another cmd*/
	if( rp->atdone )
	{
	    if( rp->atgonged )
	    {
		rp->error = 1;
		rp->atgonged = 0;
	    }
	    (*rp->func)(rp->funcarg);
	    if( rp->error )
		tlc_recover(rp);
	    rp->csr &= ~(CSR_ANYI|CSR_ERRI|CSR_NXM);
	    if( rp->atdone )
		break;
	}

	/*advance automaton, possibly finishing*/
	if( !rp->atdone )
	{
	    tlcinterp(rp);
	    if( rp->error )
		tlc_recover(rp);
	    rp->csr &= ~(CSR_ANYI|CSR_ERRI|CSR_NXM);
	    if( !rp->atdone )
		break;
	}
    }
    ifdebug((ib_q_debug--,ib_subr_debug--));
    dprintf((" ."));
}

/*
 * tlcinterp() --
 * called by interrupt routine.
 *
 * - advance automaton:
 */
tlcinterp(rp)
    register struct tlc *rp;
{
    extern int tlc_ticker();

    register char *ip = rp->base;
    register struct step *xp;
    register int buzzed;
    register long icsr;

    dprintf((" tlcinterp%d/%d-",rp->atcnt,rp->atac-rp->steps));
    dassert(rp->atcnt<MAXINSTR);

    while( rp->atcnt > 0 && !rp->error )
    {
	xp = rp->atpc;
	switch(xp->opcode)
	{
	case XCWAIT:
	case XWAIT:
	    buzzed = TLCMAXBUZZ;
# ifdef IDEBUG
	    dprintf((" [$%08x|$%08x]",xp->opa,xp->opb));
# endif IDEBUG
	    for( ;; )
	    {
		STROBE(icsr);
		FUDGE(icsr);
		dprintf((" !%d[$%08x+$%08x]",rp->unit,icsr,rp->csr));

		if( (rp->csr&xp->opa) == xp->opb )
		{
		    if( xp->opcode == XCWAIT )
		    {
# ifdef IDEBUG
			dprintf((" &$%08x",~(xp->opb&CSR_FLAKE)));
# endif IDEBUG
			rp->csr &= ~(xp->opb&CSR_FLAKE);
		    }
		    break;
		}
		  
		if( rp->csr&(CSR_ERRI|CSR_NXM) )
		    break;
		if( --buzzed < 0 )
		{
		    dprintf((" buz-atcnt%d [$%08x|$%08x]",
			rp->atcnt,xp->opa,xp->opb));
		    return;
		}
	    }
	    break;
	case XSTORE:
# ifdef IDEBUG
	    dprintf((" $%x<-$%02x",xp->opa,(u_char)xp->opb));
# endif IDEBUG
	    _OUTREG(xp->opa,xp->opb);
	    break;
	case XFETCH:
	    rp->dir = _INREG(xp->opa);
# ifdef IDEBUG
	    dprintf((" $%x:$%02x",xp->opa,(u_char)rp->dir));
# endif IDEBUG
	    break;
	case XMASK:
# ifdef IDEBUG
	    dprintf((" &$%08x|$%08x",xp->opa,xp->opb));
# endif IDEBUG
	    rp->csr = (rp->csr & xp->opa) | xp->opb;
	    break;
	case XFUNC:
# ifdef IDEBUG
	    dprintf((" (*$%08x())",xp->opa));
# endif IDEBUG
	    if( (*(int (*)())xp->opa)(rp,xp->opb) < 0 )
	    {
		dprintf(("-atcnt%d",rp->atcnt));
		/*
		 * if the function fails, start ticker
		 * to check every so often (unless already started).
		 */
		rp->Cflags |= TLC_TICKER;
		if( !(rp->Cflags & TLC_TICKING) )
		{
		    rp->Cflags |= TLC_TICKING;
		    timeout(tlc_ticker,(caddr_t)rp,tlc_func_ticks);
		}
		return;
	    }
	    rp->Cflags &= ~TLC_TICKER;
	    break;
	}
	if( rp->csr&(CSR_ERRI|CSR_NXM) )
	    rp->error = 1;
	rp->atpc++;
	rp->atcnt--;
    }

    rp->cmdalarm = 0;		/*disable tlcgong*/
    rp->atdone = 1;
    if( rp->error )
    {
	rp->csr &= ~(CSR_ERRI|CSR_NXM);
	dprintf((" err-atcnt%d",rp->atcnt));
	return;
    }
    dprintf((" !%d[$%08x]-ok",rp->unit,rp->csr));
    return;
}

/*
 * tlcgong() --
 * slow timer, mainly for timing things out.
 * called from open routine to start timer,
 * thereafter called via timeout.
 *
 * - stop timer if ib driver is closed.
 * otherwise arrange to be called again later.
 * - abort a cmd whose time (.cmdalarm) has
 * run out.
 * - recheck for conditions that don't cause
 * interrupts if it's time (.retryalarm) to
 * recheck.
 */
tlcgong(rp)
    register struct tlc *rp;
{
USEPRI;

    if( !(rp->Cflags&TLC_RUN || rp->cmdalarm) )
    {
	dprintf((" stopgong%d", rp->unit));
	rp->Cflags &= ~TLC_GONG;	/*no more timeout()s*/
	return;
    }
    rp->Cflags |= TLC_GONG;
    timeout(tlcgong,(caddr_t)rp,TLCTICKS);

    if( rp->cmdalarm )
    {
	if( !--rp->cmdalarm )
	{
	    dprintf((" timeout%d",rp->unit));
RAISE;
	    rp->atgonged = 1;
	    tlc_abort(rp);
	    tlcruintr(rp);
LOWER;
	}
	return;
    }
    if( rp->retryalarm )
    {
	if( !--rp->retryalarm )
	{
	    dprintf((" recheck%d",rp->unit));
RAISE;
	    /*
	     * KLUGE -
	     * if addressed to talk and have waited a long time
	     * for permission, just go ahead.
	     */
	    if( !rp->cic )
	    {
		if( (rp->csr&(CSR_TA|CSR_ATN_|CSR_ORI))
			== (CSR_TA|CSR_ATN_) )
		    rp->csr |= CSR_ORI;
	    }
	    tlcruintr(rp);
LOWER;
	}
    }
}

/*----- device sub-routines*/
tlc_reset(rp)
    register struct tlc *rp;
{
    register char *ip = rp->base;
    register long icsr;

    dprintf((" tlc_reset"));
    _OUTREG(IB_CR0,CR0_LMRESET);
    msdelay(1);
    _OUTREG(IB_CR0,0);
    _OUTREG(IB_AUXMR,AUXMR_OP|OP_RESET);
    STROBE(icsr);
    rp->csr = icsr;
    FUDGE(icsr);
    rp->Cflags &= ~(TLC_CONNECT|TLC_TALK|TLC_LSTN|TLC_TICKER);
    rp->events = 0;
    dprintf((" !%d[$%08x+$%08x]",rp->unit,icsr,rp->csr));
}

tlc_auxinit(rp)
    register struct tlc *rp;
{
    register char *ip = rp->base;

    _OUTREG(IB_CR0,0);
    _OUTREG(IB_CR1,rp->icr1);
    _OUTREG(IB_AUXMR,AUXMR_OP|OP_IXP);
    msdelay(1);
    _OUTREG(IB_AUXMR,AUXMR_CDR|CDR_MAG);
    _OUTREG(IB_AUXMR,AUXMR_PPR|rp->ippr);
    _OUTREG(IB_AUXMR,AUXMR_REGA|REGA_MAG|REGA_HOFF_ALL);
    _OUTREG(IB_AUXMR,AUXMR_REGB|rp->iregb);
    _OUTREG(IB_AUXMR,AUXMR_REGE|rp->irege);
    rp->Cflags &= ~(TLC_CONNECT|TLC_TALK|TLC_LSTN);
}

tlc_ifc(rp)
    register struct tlc *rp;
{
    register char *ip = rp->base;
    register long icsr;

    dprintf((" tlc_ifc"));
    if( rp->Cflags&TLC_SC )
    {
	_OUTREG(IB_AUXMR,AUXMR_OP|OP_SIFC);
	msdelay(1);
	_OUTREG(IB_AUXMR,AUXMR_OP|OP_CIFC);
	_OUTREG(IB_AUXMR,AUXMR_OP|OP_CREN);
	msdelay(1);
	_OUTREG(IB_AUXMR,AUXMR_OP|OP_SREN);
	_OUTREG(IB_AUXMR,AUXMR_OP|OP_GTSB);
    }
    else
    {
	_OUTREG(IB_AUXMR,AUXMR_OP|OP_DSC);
    }
    tlc_strobe(rp);
    rp->Cflags &= ~(TLC_CONNECT|TLC_TALK|TLC_LSTN);

    rp->cic = 0;
    if( rp->csr&CSR_CIC )
    {
	rp->cic = 1;
	rp->ppbit = rp->ippr = PPR_UNC;
	_OUTREG(IB_AUXMR,AUXMR_PPR|rp->ippr);
    }
}

tlc_pioinit(rp)
    register struct tlc *rp;
{
    register char *ip = rp->base;

    dprintf((" tlc_pioinit"));
    _OUTREG(IB_CR0,CR0_MAG_PIO);
    _OUTREG(IB_IMR1,IMR1_MAG_PIO);
    _OUTREG(IB_IMR2,IMR2_MAG_PIO);
    _OUTREG(IB_AUXMR,AUXMR_REGA|REGA_MAG|REGA_HOFF_ALL);
    _OUTREG(IB_SPMR,rp->ispmr&~SPMR_RSV);
    rp->Cflags |= TLC_PIO;
}

tlc_setaddr(rp,a1)
    register struct tlc *rp;
    char a1;
{
    register char *ip = rp->base;

    dprintf((" tlc_setaddr(...%d)",a1));

    _OUTREG(IB_ASLR,((0<<7)&ASLR_SLF)|(a1<<0));
    _OUTREG(IB_ASLR,((1<<7)&ASLR_SLF)|(ASLR_DT|ASLR_DL));

    _OUTREG(IB_ADMR,ADMR_MAG);
}

tlc_strobe(rp)
    register struct tlc *rp;
{
    register char *ip = rp->base;
    register long icsr;

    STROBE(icsr);
    FUDGE(icsr);
    dprintf((" !%d[$%08x+$%08x]",rp->unit,icsr,rp->csr));
}

tlc_srqcheck(rp, n)
    register struct tlc *rp;
{
    register char *ip = rp->base;

    if( !(_INREG(IB_SPSR)&SPSR_POLLING) )
    {
	rp->Cflags &= ~TLC_TICKER;
	_OUTREG(IB_SPMR,rp->ispmr&~SPMR_RSV);
	return 0;
    }
    return -1;
}
/*
 * tlc_ticker() --
 * used to poll the board rather frequently.
 * turns polling on/off according to .Cflags&TLC_TICKER.
 * to be called ONLY from timeout(); this prevents timeouts
 * from proliferating accidentally.
 */
tlc_ticker(rp)
    register struct tlc *rp;
{
    if( !(rp->Cflags & TLC_TICKER) || !(rp->Cflags & TLC_RUN) )
    {
	rp->Cflags &= ~TLC_TICKING;
	return;
    }
    tlcruintr(rp);
    timeout(tlc_ticker,(caddr_t)rp,tlc_func_ticks);
}

tlc_alert(rp)
    register struct tlc *rp;
{
    dprintf((" alert%d", rp->unit));
    rp->retryalarm = TLCRETRYTIME;
}

tlc_unstash(rp)
    register struct tlc *rp;
{
    register char *ip = rp->base;

    rp->dir = _INREG(IB_DIR);
    ((char *)rp->vaddr)[busfix(0)] = rp->dir;
    rp->endi = 1;
    if( rp->cic )
	_OUTREG(IB_AUXMR,AUXMR_OP|OP_TCS);
    _OUTREG(IB_AUXMR,AUXMR_OP|OP_FHS);
    rp->csr &= ~(CSR_IRI|CSR_ENDI);
    dprintf((" unstash$%x",rp->dir));
    rp->tc = 1;
    rp->bc--;
}

/*
 * tlc_stopdma() --
 * clean up after a dma.  if listening, finish handshake
 * (MUST be holding off).
 */
int
tlc_stopdma(rp)
    register struct tlc *rp;
{
    register char *ip = rp->base;

    tlc_pioinit(rp);

    rp->endi = 0;
    if( rp->Cflags&TLC_LSTN )
    {
	rp->endi = (rp->csr & CSR_ENDI) != 0;
	if( rp->endi )				/* XXX */
	    if( rp->cic )
		_OUTREG(IB_AUXMR,AUXMR_OP|OP_TCS);
	_OUTREG(IB_AUXMR,AUXMR_OP|OP_FHS);
	rp->csr &= ~(CSR_IRI|CSR_ENDI);
	dprintf(("-fhs"));
    }
    rp->tc = rp->bc;
    rp->bc = - (short) (_INREG(IB_BCR1)<<NBBY | _INREG(IB_BCR0));

    if( rp->bc > rp->tc )
    {
	/*shouldn't happen*/
	rp->bc = rp->tc = 0;
	rp->error = 1;
    }
    rp->tc -= rp->bc;

    rp->csr &= ~(CSR_DMADONE|CSR_ADCHGI);
    return 0;
}

tlc_recover(rp)
    register struct tlc *rp;
{
    register char *ip = rp->base;

    dprintf((" tlc_recov"));

    rp->Cflags &= ~(TLC_CONNECT|TLC_TALK|TLC_LSTN);
    rp->atcnt = 0;
    rp->cmdalarm = 0;
    tlc_strobe(rp);
    tlc_pioinit(rp);
    tlc_strobe(rp);
    ;;;;;;;

    if( rp->cic )
    {
	_OUTREG(IB_AUXMR,AUXMR_OP|OP_TCA);
	rp->csr |= CSR_CIC;
	rp->csr &= ~CSR_ATN_;
    }
}

/*
 * tlc_abort() --
 * always called at hi pri (already).
 * may be called from timeout routine.
 *
 * try to abort the current command.
 * - send h/w cmds to abort and clean up.
 * - set s/w flags to mark current cmd as aborted.
 * - if CIC, start a new command to get back control.
 */
tlc_abort(rp)
    register struct tlc *rp;
{
    register char *ip = rp->base;

    if( !rp->atdone )
    {
	dprintf((" %dscotching %d",rp->unit,rp->icmd));
	if( !(rp->Cflags&TLC_PIO) )
	    _OUTREG(IB_CR0,rp->icr0&~CR0_DMAEN);
    }

    tlc_strobe(rp);
    tlc_pioinit(rp);
    tlc_strobe(rp);
    rp->error = 1;
    rp->Cflags &= ~(TLC_CONNECT|TLC_TALK|TLC_LSTN|TLC_TICKER);
}

int
tlc_inreg(rp,field)
    register struct tlc *rp;
{
    register char *ip = rp->base;

    return _INREG(field);
}

tlc_outreg(rp,field,val)
    register struct tlc *rp;
{
    register char *ip = rp->base;

    _OUTREG(field,val);
}
/*----- */


/*----- routines which build the automaton*/
TLC_org(rp)
    register struct tlc *rp;
{
    /* set assembly point to 0 */
    rp->error = 0;
    rp->atcnt = 0;
    rp->atac = rp->atpc = rp->steps;

    /* initialize assembly csr */
    rp->atcsr = rp->csr;

    /* mark not done */
    rp->atdone = 0;

    /* disable cmd timeout */
    rp->cmdalarm = TLCTIMEOUT;
    rp->Cflags &= ~TLC_TICKER;
    if( !rp->cic )
	rp->cmdalarm += TLCSLAVETIMEOUT;
    ifdebug((rp->cmdalarm += TLCDEBUGTIME));
}

TLC_takectl(rp)
    register struct tlc *rp;
{
    register struct step *xp;

    dprintf((" Takectl"));

    /* if already active controller, do nothing */
    if( (rp->atcsr&(CSR_CIC|CSR_ATN_)) == CSR_CIC )
	return;

    xp = rp->atac;

    if( rp->Cflags&TLC_TALK )
	BWAITOP(CSR_ORI);
    /*
     * determine how to take control:
     * if standby controller, then async (XXX).
     * if being passed control, then synchronously.
     */
    if( rp->atcsr&CSR_CIC )
    {
	rp->cmdalarm += tlc_act_time;
	STOREOP(IB_AUXMR,AUXMR_OP|OP_TCA);
    }
    else
    {
	rp->cmdalarm += tlc_tctl_time;
	STOREOP(IB_AUXMR,AUXMR_OP|OP_TCS);
    }

    BCLROP(CSR_ERRI);

    /* wait for it to take effect */
    WAITOP(CSR_CIC|CSR_ATN_,CSR_CIC);

    rp->atac = xp;

    /* mark as CIC */
    rp->cic = 1;

    /* update assembly csr */
    rp->atcsr &= ~CSR_ATN_; rp->atcsr |= CSR_CIC;
}

TLC_connect(rp,talkslot,lstnmap)
    register struct tlc *rp;
    int talkslot;
    register int lstnmap;
{
    register u_char *np;
    register struct step *xp;
    u_char mta;

    dprintf((" Connect(...%d,0%o)",talkslot,lstnmap));
    xp = rp->atac;

    rp->cmdalarm += tlc_connect_time;
    lstnmap &= msk(MAXIBSLOTS);
    np = rp->tags+0;
    mta = rp->itag;

    /* turn off serial poll mode, just in case */
    if( rp->atcsr&CSR_SPMS )
	SSTEP(CSR_CRI,IB_DOR,SPD_CMD);
    rp->atcsr &= ~CSR_SPMS;

    /* stop listening, just in case */
    if( rp->atcsr&CSR_LA )
	STOREOP(IB_AUXMR,AUXMR_OP|OP_LUL);

    /* mark as connected, talking, listening, as appropriate */
    rp->Cflags &= ~(TLC_TALK|TLC_LSTN);
    rp->Cflags |= TLC_CONNECT;
    rp->iTslot = talkslot;
    rp->iLmap = lstnmap;

    if( (unsigned)talkslot < MAXIBSLOTS
     && rp->tags[talkslot] == mta )
	rp->Cflags |= TLC_TALK;

    /* turn off all listeners */
    SSTEP(CSR_CRI,IB_DOR,UNL_CMD);

    /* turn all requested listeners */
    while( lstnmap > 0 )
    {
	if( lstnmap&01 )
	{
	    if( *np == mta )
		rp->Cflags |= TLC_LSTN;
	    else
		SSTEP(CSR_CRI,IB_DOR,LSTN_BASE|*np);
	}
	np++; lstnmap >>= 1;
    }

    /* turn on the requested talker */
    if( (unsigned)talkslot < MAXIBSLOTS )
	SSTEP(CSR_CRI,IB_DOR,TALK_BASE|rp->tags[talkslot]);
    else
	SSTEP(CSR_CRI,IB_DOR,UNT_CMD);

    /* ensure consistency of status */
    if( rp->Cflags&TLC_TALK )
	rp->Cflags &= ~TLC_LSTN;
    rp->atac = xp;
}

TLC_standby(rp)
    register struct tlc *rp;
{
    register struct step *xp;

    if( (rp->atcsr&(CSR_CIC|CSR_ATN_)) == (CSR_CIC|CSR_ATN_) )
	return;
    rp->cmdalarm += tlc_gtsb_time;
    dprintf((" Standby()"));
    xp = rp->atac;

    STOREOP(IB_CR1,rp->icr1);

    /* turn off various flakey bits, just in case */
    BCLROP(CSR_DMADONE|CSR_ENDI|CSR_ADCHGI);

    /*
     * go to standby controller in the appropriate manner:
     * if getting ready to talk, just do it.
     * if getting ready to listen, start listening first.
     * if neither, start listening in "continuous mode" first.
     */
    if( rp->Cflags&TLC_TALK )
    {
	SSTEP(CSR_CRI,IB_AUXMR,AUXMR_OP|OP_GTSB);
    }
    else
    if( rp->Cflags&TLC_LSTN )
    {
	SSTEP(CSR_CRI,IB_AUXMR,AUXMR_OP|OP_LSTN);
	SSTEP(CSR_LA,IB_AUXMR,AUXMR_OP|OP_GTSB);
    }
    else
    {
	SSTEP(CSR_CRI,IB_AUXMR,AUXMR_OP|OP_LICM);
	STOREOP(IB_AUXMR,AUXMR_OP|OP_GTSB);
	rp->Cflags |= TLC_LSTN;
    }

    WAITOP(CSR_ATN_|CSR_CIC,CSR_ATN_|CSR_CIC);

    rp->atac = xp;

    /* update the assembly csr */
    rp->atcsr |= CSR_CIC|CSR_ATN_;
}

TLC_startdma(rp,maddr,count,use)
    register struct tlc *rp;
    long maddr;
    unsigned count;
    int use;
{
    extern int tlc_stopdma();

    register struct step *xp;
    register long iii;
    char auxmr, imr1, imr2, cr0;
    long waitbits;

    dprintf((" Start%sdma [$%x %d]",use==LSTNU?"R":"S",maddr,count));
    xp = rp->atac;

    /* store the multibus mem addr */
    iii = maddr;
    STOREOP(IB_MAR0,(u_char)iii);
    iii >>= NBBY;
    STOREOP(IB_MAR1,(u_char)iii);
    iii >>= NBBY;
    STOREOP(IB_MAR2,(u_char)iii);

    /* store the negative count */
    iii = -count;
    STOREOP(IB_BCR0,(u_char)iii);
    iii >>= NBBY;
    STOREOP(IB_BCR1,(u_char)iii);

    rp->Cflags &= ~(TLC_PIO|TLC_TALK|TLC_LSTN);
    /*
     * set up end conditions and actions.
     * if listening, holdoff as appropriate.
     * MUST hold off just before the end of each dma,
     * in order to be sure that the status registers
     * refer to the last dma byte (not the following).
     * if talking, prepare to send END as appropriate.
     */
    if( use == LSTNU )
    {
	rp->cmdalarm += tlc_lstn_time;
	rp->Cflags |= TLC_LSTN;
	cr0 = CR0_MAG_DMAI | CR0_CCEN;
	auxmr = AUXMR_REGA|REGA_MAG|REGA_HOFF_END;
	STOREOP(IB_CCFR,AUXMR_REGA|REGA_MAG|REGA_HOFF_ALL);
	waitbits = CSR_ATN_|CSR_LA;
	imr1 = IMR1_MAG_DMAI;
	imr2 = IMR2_MAG_DMAI;
    }
    else
    {
	rp->cmdalarm += tlc_talk_time;
	rp->Cflags |= TLC_TALK;
	cr0 = CR0_MAG_DMAO & ~CR0_CCEN;
	auxmr = AUXMR_REGA|REGA_MAG|REGA_HOFF_ALL;
	if( use == ETALKU )
	{
	    STOREOP(IB_CCFR,AUXMR_OP|OP_XEOI);
	    cr0 |= CR0_CCEN;
	}
	waitbits = CSR_ATN_|CSR_TA|CSR_ORI;
	imr1 = IMR1_MAG_DMAO;
	imr2 = IMR2_MAG_DMAO;
    }

    STOREOP(IB_AUXMR,auxmr);
    BWAITOP(waitbits);

    /* set up interrupt conditions */
    STOREOP(IB_IMR1,imr1);
    STOREOP(IB_IMR2,imr2);

    rp->icr0 = cr0;
    STOREOP(IB_CR0,cr0);
    BCLROP(CSR_DMADONE|CSR_ADCHGI|CSR_IRI|CSR_ORI|CSR_CRI);	/* XXX */
    cr0 |= CR0_GO;

    /* go */
    STOREOP(IB_CR0,cr0);
    BWAITOP(CSR_DMADONE);
    FUNCOP(tlc_stopdma,0);

    rp->atac = xp;
}

TLC_passctl(rp)
    register struct tlc *rp;
{
    register struct step *xp;

    if( !rp->cic )
	return;
    dprintf((" Passctl"));
    xp = rp->atac;

    rp->cmdalarm += tlc_pctl_time;
    /* when iface ready, write out the passctl command */
    SSTEP(CSR_CRI,IB_DOR,TCT_CMD);

    /* wait for it to take effect */
    WAITOP(CSR_CIC,0);

    /* cleanup */
    BCLROP(CSR_ENDI);

    rp->atac = xp;

    /* update status */
    rp->Cflags &= ~(TLC_CONNECT|TLC_TALK|TLC_LSTN);
}

TLC_getspoll(rp)
    register struct tlc *rp;
{
    register struct step *xp;

    dprintf((" Getspoll"));
    xp = rp->atac;

    /* wait for serial poll response */
    BWAITOP(CSR_ATN_|CSR_SPMS|CSR_IRI);
    BCLROP(CSR_IRI|CSR_ENDI);

    /* read it in */
    FETCHOP(IB_DIR);

    /* handshake it */
    STOREOP(IB_AUXMR,AUXMR_OP|OP_FHS);

    rp->atac = xp;
}

TLC_ppoll(rp)
    register struct tlc *rp;
{
    register struct step *xp;

    dprintf((" Ppoll"));
    xp = rp->atac;

    rp->cmdalarm += tlc_poll_time;
    /* when iface is ready, execute parallel poll */
    SSTEP(CSR_CRI,IB_AUXMR,AUXMR_OP|OP_XPP);
    BWAITOP(CSR_CRI);
    BCLROP(CSR_ENDI);

    /* read in response */
    FETCHOP(IB_CPTR);

    /* handshake it */
    STOREOP(IB_AUXMR,AUXMR_OP|OP_FHS);

    rp->atac = xp;
}

TLC_srq(rp,spmr)
    register struct tlc *rp;
{
    extern int tlc_srqcheck();

    register struct step *xp;

    xp = rp->atac;
    dprintf((" Srq"));

    rp->cmdalarm += tlc_srq_time;

    /* when iface ready, set serial poll response reg */
    WAITOP(CSR_SPMS,0);
    STOREOP(IB_SPMR,spmr|SPMR_RSV);
    /*
     * KLUGE -
     * the controller doesn't interrupt
     * when IB_SPSR changes.  so arrange
     * to have a subroutine check every
     * so often.
     */
    FUNCOP(tlc_srqcheck,0);
    rp->atac = xp;
}

TLC_sstep(rp,cmask,rreg,rval)
    register struct tlc *rp;
{
    register struct step *xp;

    xp = rp->atac;
    SSTEP(cmask,rreg,rval);
    rp->atac = xp;
}
/*----- */
