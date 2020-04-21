/*
 *	siq.c	- Copyright (C) SGI
 */

#include "sq.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/iobuf.h"
#include "../h/file.h"
#include "../h/mtio.h"
#include "../h/dkerror.h"
#include "machine/cpureg.h"
#include "../multibus/wait.h"
#include "../multibus/siqreg.h"
#include "../multibus/mbvar.h"


#ifdef OS_DEBUG
#define SIQ_DEBUG 1
#endif

#ifdef SIQ_DEBUG
#define printf trace_printf
#define iprintf trace_printf

int siq_noise = 0;

extern	struct dkerror sii_cmds[], sii_errs[];
extern	short sii_ncmds, sii_nerrs;

#endif

int	siqstrategy(), siqprobe(), siqattach(), siqintr();

struct	mb_device *siqdinfo[NSQ];
struct	mb_ctlr *siqcinfo[NSIQ];
struct	mb_driver siqdriver = {
	siqprobe, siqattach, (int (*)())0, (int (*)())0, siqintr,
	(char *(*)())0, "sq", siqdinfo, "siq", siqcinfo,
};

#define ctlr_res (&sc->sc_ctlr_res)
#define tape_res (&st->st_tape_res)


/*
 * siqprobe
 */
siqprobe(reg, ctlr)
  int reg;
  int ctlr;
{
	register struct softc *sc;
	short i;

	sc = &siqsoftc[ctlr];
	sc->sc_ioaddr = (caddr_t)MBIO_VBASE + reg;
	sc->sc_ctlr = ctlr;

	/* touch controller */
	i = TAPESTATUS(sc);
#ifdef	lint
	i = i;
#endif

	sc->sc_iopb_mbva = mbmapkget((caddr_t)&sc->sc_iopb, sizeof(iopb_t));
	sc->sc_flags = SC_ALIVE;
	printf("(qic02 cartridge tape) ");
	return (CONF_ALIVE);
}

/*
 * siqattach: 
 *	- initialize the drive
 */
siqattach(mi)
  struct mb_device *mi;
{
	register struct softc *sc;
	register struct softc_tape *st;

	sc = &siqsoftc[mi->mi_ctlr];
	st = &sc->sc_tape[mi->mi_unit];
	st->st_unit = mi->mi_unit;
	st->st_ctlr = sc;
	if (siqinit(st, NOSLEEP)) {
		mbmapkput(sc->sc_iopb_mbva, sizeof (iopb_t));
		return (CONF_FAULTED);
	}
	st->st_flags = ST_ALIVE;
	sprintf(st->st_name, "sq%d", st->st_unit); 
	return (CONF_ALIVE);
}

/*
 * siqinit:
 *	- initialize IOPB and send a CONFIGURE cmd to controller.
 */
int
siqinit(st, cansleep)
  register struct softc_tape *st;
  int cansleep;
{
	register struct softc *sc = st->st_ctlr;
	register iopb_t *iop = &sc->sc_iopb;
	int result, s;
	int attempts = 2;

	/*
	 * Write permanent items to IOPB
	 */
	s = spl6();
	bzero(iop, sizeof(iopb_t));		/* first initial to zeros */
	iop->i_ioh = MB(sc->sc_ioaddr);
	iop->i_iol = LB(sc->sc_ioaddr);
	iop->i_option = O_OPTIONS;

	/* point controller at iopb */
	*(STR1(sc)) = HB(sc->sc_iopb_mbva);
	*(STR2(sc)) = MB(sc->sc_iopb_mbva);
	*(STR3(sc)) = LB(sc->sc_iopb_mbva);
	splx(s);

	/*
	 * The ctlr could consider the first attempt at CONFIGURE
	 * to be an illegal command, depending on what was the
	 * immediately preceding command.
	 */
	while ((result = siqcmd(st, C_TPCONFIG, 0, NULL_BP,
	    WAIT, cansleep)) && (--attempts >= 0)) {

		if (RES_ANYERR(ctlr_res) || result == EINTR ||
		   sc->sc_iopb.i_error == ERR_TAPENOTREADY)
			break;
		s = spl6();
		clear_res(tape_res, RES_BITS_ANYERR);
		splx(s);
		siq_delay(cansleep);
	    }

#ifdef SIQ_DEBUG
	if (siq_noise) {
		printf("siqinit: CONFIG result = %x\n", result);
		printf("st = %x\n", (long) st);
		printf("sc = %x\n", (long) sc);
	}
#endif
	return (result);
}


/*
 * siqopen - open device only if tape STATUS command is successful
 */
/*ARGSUSED*/
siqopen(dev, flag)
  dev_t dev;
  int flag;
{
	register struct softc *sc;
	register struct softc_tape *st;
	register short ctlr = CTLR(dev);
	register short unit = UNIT(dev);
	register short flags = FLAGS(dev);
	int s;

	/*
	 * Insure that controller and unit # are in range, and that the
	 * controller and unit exist.
	 */
	if ((ctlr > NSIQ) || (unit > NSQ) ||
	    (((sc = &siqsoftc[ctlr])->sc_flags & SC_ALIVE) == 0) ||
	    (((st = &sc->sc_tape[unit])->st_flags & ST_ALIVE) == 0)) {
		u.u_error = ENODEV;
		return;
	}

#ifdef	SIQ_DEBUG
	if (siq_noise) {
		printf("st = %x\n", (long) st);
		printf("sc = %x\n", (long) sc);
	}
#endif
	s = spl6();
	if (st->st_flags & ST_OPEN) {
		/* sorry, only one user at a time */
		splx(s);
		u.u_error = EBUSY;
		return;
	}
	st->st_flags |= ST_OPEN;

	/* If ctlr previously had a hard error, reset here. */
reset_ctlr:
	if (RES_ANYERR(ctlr_res)) {

		clear_res(tape_res, RES_CLEARABLE_BITS);
		clear_res(ctlr_res, RES_CLEARABLE_BITS);
		st->st_fileno = 0;
		st->st_flags &= ~(ST_EOT | ST_WRITTEN | ST_ENDOFFILE);
		splx(s);

		/*
		 * Reset the ctlr
		 */
		if (st_hard_reset(sc) || siqinit(st, CANSLEEP)) {
		    uprintf("%s: tape drive inaccessible\n", st->st_name);
		    u.u_error = EIO;
		    goto error;
		}
		s = spl6();
	}

	/*
	 * Wait for previous rewind to complete
	 */
	while (RES_TAKEN(tape_res)) {
		u.u_error = sleepfor(tape_res, PUSER|PCATCH);
		switch (u.u_error) {
		    case EIO:
			if (RES_TIMED_OUT(tape_res)) {
				/* timeout waiting for rewind */
				set_res(ctlr_res, RES_BIT_ERR);
			}
			if (RES_ANYERR(ctlr_res))
				goto reset_ctlr;
			else {
				/*
				 * Don't reset ctlr if somebody just
				 * removed the tape during rewind.
				 */
				splx(s);
				uprintf("%s: tape error during open\n",
					st->st_name);
				goto error;
			}

		    case EINTR:
			splx(s);
			goto error;
		}
	}

	/* If tape errors were previously encountered, we try again here */
	if (RES_ANYERR(tape_res)) {
		clear_res(tape_res, RES_BITS_ANYERR);
		st->st_fileno = 0;
		st->st_flags &= ~(ST_EOT | ST_WRITTEN | ST_ENDOFFILE);
	}

	splx(s);

	if (u.u_error = siqstatus(st, CANSLEEP)) {
		s = spl6();
		set_res(ctlr_res, RES_BIT_ERR);
		goto reset_ctlr;
	}

	/* insure that drive is usable */
	if (st->st_status[0] & STATUS_NOCARTRIDGE) {
		uprintf("%s: no tape cartridge in drive\n", st->st_name);
		u.u_error = EIO;
		goto error;
	}
	if ((st->st_status[0] & STATUS_WRITEPROTECTED) && (flag & FWRITE)) {
		uprintf("%s: tape is write protected\n", st->st_name);
		u.u_error = EIO;
		goto error;
	}
	if (st->st_status[0] & STATUS_NOTONLINE) {
		uprintf("%s: drive is not on line\n", st->st_name);
		u.u_error = EIO;
		goto error;
	}

	/*
	 * If this is the rewind device and we are not at BOT, rewind.
	 */
	if (!(st->st_status[1] & STATUS_BOT) &&
	    !(flags & FLAGS_NOREWIND)) {
		if (u.u_error = siqcmd(st, C_TPREWIND, 1, NULL_BP,
		   NOWAIT, CANSLEEP)) {
			goto error;
		   }
	}
	return;
error:
	s = spl6();
	st->st_flags &= ~ST_OPEN;
	splx(s);
}


/*
 * siqclose:
 *	- close the device
 *	- write closing file mark if the tape has been written
 *	- rewind the tape, if this is not the no-rewind device
 */
siqclose(dev)
  dev_t dev;
{
	register struct softc *sc;
	register struct softc_tape *st;
	short unit = UNIT(dev);
	short ctlr = CTLR(dev);
	int s;

	sc = &siqsoftc[ctlr];
	st = &sc->sc_tape[unit];

	if (RES_ANYERR(ctlr_res) || RES_ANYERR(tape_res))
		goto out;

	if (st->st_flags & ST_WRITTEN) {
	    	/* write closing file mark */
		if (u.u_error = siqcmd(st, C_TPWEOF, 1, NULL_BP, WAIT,
		   CANSLEEP)) {
			uprintf("%s: can't write tape EOF\n",
				st->st_name);
		   }
	}

    	/* attempt to rewind tape, if it's requested */
	if (!((FLAGS(dev) & FLAGS_NOREWIND))) {
		if (u.u_error = siqcmd(st, C_TPREWIND, 1, NULL_BP, NOWAIT,
		  CANSLEEP)) {
			uprintf("%s: can't rewind tape\n", st->st_name);
		  }
	}
out:
	/* Clean up all the flags that were used */
	s = spl6();
	st->st_flags &= ~(ST_WRITTEN | ST_ENDOFFILE | ST_OPEN | ST_EOT);
	splx(s);
}


siqread(dev)
  register dev_t dev;
{
	register struct softc *sc;
	register struct softc_tape *st;
	short unit = UNIT(dev);
	short ctlr = CTLR(dev);

	sc = &siqsoftc[ctlr];
	st = &sc->sc_tape[unit];

	/* If at EOF, return 0 byte count */
	if (st->st_flags & ST_ENDOFFILE)
		return;

	physio(siqstrategy, (struct buf *)NULL, dev, B_READ|B_TAPE, minphys);
#ifdef	SIQ_DEBUG
	if (siq_noise)
		printf("siqread: u_error = %d, u_count = %d\n",
			u.u_error, u.u_count);
#endif
}

siqwrite(dev)
  register dev_t dev;
{
	physio(siqstrategy, (struct buf *)NULL, dev, B_WRITE|B_TAPE, minphys);
#ifdef	SIQ_DEBUG
	if (siq_noise)
		printf("siqwrite: u_error = %d, u_count = %d\n",
			u.u_error, u.u_count);
#endif
}

siqstrategy(bp)
  register struct buf *bp;
{
	short ctlr = CTLR(bp->b_dev);
	short unit = UNIT(bp->b_dev);
	register struct softc_tape *st;
	register struct softc *sc;
	int s;

	sc = &siqsoftc[ctlr];
	st = &sc->sc_tape[unit];

	/* Init count not done = request count */
	st->st_resid = bp->b_bcount; 

	/* Give "no space" error if at EOT */
	if (st->st_flags & ST_EOT) {
		u.u_error = ENOSPC;
		berror(bp);
		return;
	}

	if (bp->b_bcount & (BBSIZE - 1)) {
		uprintf("%s: must be multiple of 512 bytes\n", st->st_name);
		u.u_error = EIO;
		berror(bp);
		return;
	}

	if (u.u_error = siqcmd(st, (bp->b_flags & B_READ) ? 
	   C_TPREAD : C_TPWRITE, 0, bp, WAIT, CANSLEEP)) {
		/* WAITing means berror() doesn't have to be done
		 * in the interrupt handler.
		 */
		berror(bp);
		return;
	   }
	if ((bp->b_flags & B_READ) == 0) {
		s = spl6();
		if (bp->b_error == ENOSPC)
			/* if at EOT, keep siqclose from writing EOF */
			st->st_flags &= ~ST_WRITTEN;
		else
			st->st_flags |= ST_WRITTEN;
		splx(s);
	}
}


/*ARGSUSED*/
siqioctl(dev, cmd, addr, flag)
  caddr_t addr;
  dev_t dev;
{
	short ctlr = CTLR(dev);
	short unit = UNIT(dev);
	register struct softc_tape *st;
	register struct softc *sc;
	struct mtop mtop;
	struct mtget mtget;

	sc = &siqsoftc[ctlr];
	st = &sc->sc_tape[unit];

	switch(cmd) {
	  case MTIOCTOP:
		if (copyin((caddr_t)addr, (caddr_t)&mtop, sizeof(mtop))) {
			u.u_error = EFAULT;
			break;
		}
		switch (mtop.mt_op) {
		  case MTREW:
			u.u_error = siqcmd(st, C_TPREWIND, 1, NULL_BP,
				NOWAIT, CANSLEEP);
			break;
		  case MTRET:
			u.u_error = siqcmd(st, C_TPRETENSION, 1, NULL_BP,
				NOWAIT, CANSLEEP);
			break;
		  case MTERASE:
			u.u_error = siqcmd(st, C_TPERASE, 1, NULL_BP,
				NOWAIT, CANSLEEP);
			break;
		  case MTWEOF:
			u.u_error = siqcmd(st, C_TPWEOF, 1, NULL_BP,
				WAIT, CANSLEEP);
			break;
		  case MTFSF:
			u.u_error = siqcmd(st, C_TPFSF, mtop.mt_count,
				NULL_BP, WAIT, CANSLEEP);
			break;
		  case MTFSR:
			u.u_error = siqcmd(st, C_TPFSR, mtop.mt_count,
				NULL_BP, WAIT, CANSLEEP);
			break;
		  default:
			u.u_error = EINVAL;
			break;
		}
		break;

	  case MTIOCGET:
		if (copyin((caddr_t)addr, (caddr_t)&mtop, sizeof(mtop))) {
			u.u_error = EFAULT;
			break;
		}
		switch(mtop.mt_op) {
		  case MTBLKSIZE:
			mtget.mt_blkno = 400;
			if (copyout((caddr_t)&mtget, addr, sizeof(mtget)))
				u.u_error = EFAULT;
			break;
		  case MTNOP:
			if (u.u_error = siqstatus(st, CANSLEEP))
				return;
			bzero((caddr_t)&mtget, sizeof(mtget));
			mtget.mt_type = MT_ISSTT;
			mtget.mt_blkno = 0;
			mtget.mt_resid = st->st_resid;
			if (st->st_status[1] & STATUS_BOT)
				mtget.mt_at_bot = 1;
			if (st->st_status[0] & STATUS_NOCARTRIDGE)
				mtget.mt_status = NO_TAPE;
			if (st->st_status[0] & STATUS_NOTONLINE)
				mtget.mt_status |= NOT_ONLINE;
			if (st->st_status[0] & STATUS_WRITEPROTECTED)
				mtget.mt_status |= WR_PROT;
			mtget.mt_fileno = st->st_fileno;
			if (copyout((caddr_t)&mtget, addr, sizeof(mtget)))
				u.u_error = EFAULT;
			break;
		  default:
			u.u_error = EINVAL;
			break;
		}
		break;

	  default:
		u.u_error = EINVAL;
	}
}


/*
 * siqintr
 */
siqintr()
{
	register struct softc *sc;
	register short ctlr;
	register int didsomething;

	sc = &siqsoftc[0];
	didsomething = 0;
	for (ctlr = NSIQ; --ctlr >= 0; sc++) {
		if (sc->sc_flags & SC_ALIVE)
			didsomething += siqcintr(sc);
	}
	return (didsomething);
}

siqcintr(sc)
  register struct softc *sc;
{
	register struct iopb *iop;
	register char *status;
	register struct softc_tape *st;
	register int unit;
	register u_int resid;
	u_char sr, iunit, tapestatus, cmd;
	struct buf *bp;

	iunit = RDINTR(sc);		/* interrupting unit */
	unit = TAPEUNITVAL(iunit);
	if (unit < 0)			/* ignore non-tape intrs */
		return (0);

	sr = STATUSREG(sc);
	tapestatus = TAPESTATUS(sc);

	iop = &sc->sc_iopb;
	st = &sc->sc_tape[unit];
	bp = sc->sc_buf;
	cmd = iop->i_cmd;

	if (!(sr & (CSR_DONE | CSR_STATUSCHANGE)))
		return (0);

#ifdef	SIQ_DEBUG
	if (siq_noise) {
	    iprintf("siqcintr: cmd=%x  R0=%x  iop_status=%x  iop_err=%x\
  tapestatus=%x  st_flags=%x  iunit#=%x\n", cmd, sr, iop->i_status,
		iop->i_error, tapestatus, st->st_flags, iunit);
	}
#endif

	CLEAR(sc);			/* Clear the interrupt */

	/*
	 * If tapestatus = not ready, then tape_res will already be busy
	 * unless we just received an unsolicited status change interrupt.
	 * The latter is possible:  An illegal command sequence, such as a
	 * READ follwed by a STATUS, causes the ctlr to start a rewind and
	 * also give us a status change interrupt indicating "not ready."
	 */
	if ((tapestatus >> unit) & 1) {
		clear_res(tape_res, RES_BIT_BUSY);
	} else {
			/* following is NOP if tape_res already busy */
		(void) busy_res(tape_res, TIME_REWIND * hz);
	}

	/*
	 * If status change interrupt,
	 *  no need to go any further.
	 */
	if (!(sr & CSR_DONE)) {
		siq_waitforack(sc, sr, iunit, tapestatus);
		return (1);
	}

	/*
	 * Check IOPB status
	 */
	if (iop->i_status == S_OK) {
		switch (cmd) {
		  case C_TPREAD:
		  case C_TPWRITE:
			bp->b_resid = 0;
			iodone(bp);
			break;

		  case C_TPSTATUS:
			/*
			 * Copy data out of buffer into software struct.
			 * Byte swap data while we are at it.
			 */
			status = (char *)bp->b_un.b_addr;
			st->st_status[1] = *status++;
			st->st_status[0] = *status++;
			st->st_status[3] = *status++;
			st->st_status[2] = *status++;
			st->st_status[5] = *status++;
			st->st_status[4] = *status;
#ifdef	SIQ_DEBUG
			if (siq_noise) {
			    iprintf("siqcintr/STATUS: %x %x\n",
				st->st_status[0], st->st_status[1]);
			}
#endif
			break;
		}
	} else if ((iop->i_status == S_ERROR) || (iop->i_status == S_BUSY)) {

		/*
		 * An exception occured
		 * - any waiters for the tape should be woken up.
		 */
		clear_res(tape_res, RES_BIT_BUSY);

		if ((cmd == C_TPREAD) || (cmd == C_TPWRITE)) {

			/*
			 * Convert ctlr's residual block count to bytes
			 */
			resid = (int)((iop->i_blkcnth<<8) | iop->i_blkcntl);
			st->st_resid = bp->b_resid = resid << BBSHIFT;
#ifdef SIQ_DEBUG
			if (siq_noise)
			    iprintf("siqintr: blkcnt h=%x l=%x, resid = %d\n",
			(u_long) iop->i_blkcnth, (u_long) iop->i_blkcntl,
			(u_long) st->st_resid);
#endif
			/*
			 * Check for filemark encountered during READ.
			 */
			if ((cmd == C_TPREAD) &&
			    ((iop->i_error == ERR_FILEMARK)
			      || (iop->i_error == ERR_PREMATURE_EOF))) {
				st->st_flags |= ST_ENDOFFILE;
				iodone(bp);
				goto out;
			}

			/*
			 * If EOT encountered, set B_ERROR for buffer
			 *    and return u.u_error = ENOSPC to user.
			 *
			 * But don't declare an error on tape_res, since
			 *   then siqcmd would return EIO till next siqopen.
			 */
			if ((iop->i_error == ERR_PREMATURE_EOT) ||
			    ((cmd == C_TPREAD) &&
			    (iop->i_error == ERR_NODATA))) {
				bp->b_flags |= B_ERROR;
				bp->b_error = ENOSPC;
				st->st_flags |= ST_EOT;
				iodone(bp);
				goto out;
			    }
		}

		/*
		 * A real error was encountered.  Declare a tape error
		 * and possibly a controller error.
		 */
		set_res(tape_res, RES_BIT_ERR);
		if (cmd == C_TPREAD || cmd == C_TPWRITE)
			st->st_flags |= ST_ENDOFFILE;
		switch (iop->i_error) {
			/*
			 * The following are tape errors.
			 */
			case ERR_FILEMARK:
			case ERR_NOTSELECTED:
			case ERR_TAPENOTREADY:
			case ERR_NOTONLINE:
			case ERR_NOCARTRIDGE:
			case ERR_PREMATURE_BOT:
			case ERR_PREMATURE_EOF:
			case ERR_DATAERROR:
			case ERR_NODATA:
			case ERR_WRITEPROTECTED:
			case ERR_ILLEGALCMD:	/* wrong tape cmd for
						 * current ctlr state.
						 */
			case ERR_STATUSTIMEOUT:
			case ERR_EXCEPTION:
			case ERR_READYTIMEOUT:
			case ERR_INVALID_BLKCNT:
				break;

			/* Here assume a controller error.
			 *
			 */
			default:
				set_res(ctlr_res, RES_BIT_ERR);
#ifdef	SIQ_DEBUG
	iprintf("%s: hard error, cmd=\"%s\" error=\"%s\"\n",
		st->st_name, dkerror((u_char)cmd, sii_cmds, sii_ncmds),
		dkerror((u_char)iop->i_error, sii_errs, sii_nerrs));
				break;
#endif
		}
	} else {
		set_res(tape_res, RES_BIT_ERR);
		if (cmd == C_TPREAD || cmd == C_TPWRITE)
			st->st_flags |= ST_ENDOFFILE;
		set_res(ctlr_res, RES_BIT_ERR);
#ifdef	SIQ_DEBUG
		iprintf("%s: bad ctlr status (%x), csr=%x cmd=%x err=%x\n",
			st->st_name, iop->i_status, STATUSREG(sc),
			cmd, iop->i_error);
#endif
	}

out:
#ifdef	SIQ_DEBUG
	if (siq_noise)
		iprintf("siqcintr/out: st_flags = %x \n",
			(u_long) st->st_flags);
#endif
	clear_res(ctlr_res, RES_BIT_BUSY);
	siq_waitforack(sc, sr, iunit, tapestatus);
	return (1);
}

/*
 * siq_waitforack:
 *	Wait for controller to acknowledge clear interrupt.
 */
siq_waitforack(sc, r0, r1, r2)
  register struct softc *sc;
  u_char r0, r1, r2;
{
	u_char tr0, tr1, tr2;
	long waiting;

	r0 &= ~CSR_BUSY;
	waiting = 1000;
	while (--waiting) {
		tr0 = STATUSREG(sc) & ~CSR_BUSY;
		tr1 = RDINTR(sc);
		tr2 = TAPESTATUS(sc);
		if ((tr0 != r0) || (tr1 != r1) || (tr2 != r2))
			return;
	}
#ifdef	SIQ_DEBUG
	iprintf("sq0: timeout waiting for interrupt to clear!\n");
	iprintf("sq0: r0=%x r1=%x r2=%x\n", r0, r1, r2);
	iprintf("sq0: tr0=%x tr1=%x tr2=%x\n", tr0, tr1, tr2);
#endif
}


/*
 * siqstatus:
 *	- get status about tape drive from controller
 *	- return unix error code or 0
 */
int
siqstatus(st, cansleep)
  register struct softc_tape *st;
  register int cansleep;
{
	register struct buf *bp;
	register int i;
	struct softc *sc = st->st_ctlr;
	int ret_val, s;
	int attempts = 50;
	int nocartridge_tries = 3;

	s = spl6();
	for (i = 0; i < 6;)
		st->st_status[i++] = 0;
	splx(s);
	bp = getdmablk(1);

	/* For some tape drives, if a cartridge is taken out and then
	 * re-inserted, this isn't detected by the 1st status command.
	 */
	do {
		while ((ret_val = siqcmd(st, C_TPSTATUS, 0, bp,
		    WAIT, cansleep)) && (--attempts > 0)) {

			if (RES_ANYERR(ctlr_res) || ret_val == EINTR)
				break;
			s = spl6();
			clear_res(tape_res, RES_BITS_ANYERR);
			splx(s);
			siq_delay(cansleep);
		    }

		if (RES_ANYERR(ctlr_res) || RES_ANYERR(tape_res)
		  || (ret_val == EINTR))
			break;
	} while ((st->st_status[0] & (STATUS_NOCARTRIDGE|STATUS_NOTONLINE))
	   && (--nocartridge_tries > 0));

	brelse(bp);
#ifdef	SIQ_DEBUG
	if (siq_noise)
		printf("siqstatus: rtn value = %d\n", ret_val);
#endif
	return (ret_val);
}


/*
 * siqcmd
 *	- issue a command to the tape drive
 *	- return a unix error code if the command fails, 0 if it succeeds
 */
int
siqcmd(st, cmd, count, bp, wait, cansleep)
  register struct softc_tape *st;
  u_char cmd;
  u_int count;
  register struct buf *bp;
  int wait;				/* wait for completion or not */
  int cansleep;				/* do we poll or sleep? */
{
	register struct softc *sc = st->st_ctlr;
	register iopb_t *iop;
	register u_int blk_cnt;
	int s, tape_timeout;
	int ret_val = 0;

	/* If errors already encountered, don't allow any more i/o
	 * until the next open.
	 */
	if (RES_ANYERR(ctlr_res) || RES_ANYERR(tape_res))
		return (EIO);

	/* Note:  Most commands operate as follows:  We load up the IOPB,
	 *  and issue START to the controller.  The controller then
	 *  interrupts us with a done interrupt.  If we read the tape
	 *  status register at that point (see siqintr()), we would find
	 *  that the tape unit has a ready status.
	 *
	 *  However, for some commands (CONFIG, REWIND, RETENSION), the
	 *  controller responds with 2 interrupts.  After we start the
	 *  command, the board 1st gives us a "done" interrupt, at which
	 *  time the tape status register will show "tape not ready."
	 *  When the operation is actually completed some time later,
	 *  the board issues a status change interrupt, at which time the
	 *  tape status register will then show tape ready.
	 */

#ifdef SIQ_DEBUG
	if (siq_noise)
		printf("siqcmd/START\n");
#endif
	if (cansleep)
		s = spl6();
	/*
	 * Wait for IOPB to be free
	 */
again:
	ret_val = RES_ERRNO(ctlr_res);
	while (ret_val == 0 && RES_TAKEN(ctlr_res))
		ret_val = WAITFOR(ctlr_res, PRIBIO, cansleep);

	/*
	 * Wait for unit to become available
	 */
	if (ret_val == 0)
		ret_val = RES_ERRNO(tape_res);
	while (ret_val == 0 && RES_TAKEN(tape_res))
		ret_val = WAITFOR(tape_res, PRIBIO, cansleep);

	if (!cansleep) {
		s = spl6();
		if (RES_TAKEN(ctlr_res) || RES_TAKEN(tape_res)) {
			splx(s);
			goto again;
		}
	}
	if (ret_val) {
		splx(s);
		if (RES_TIMED_OUT(ctlr_res))
			printf("sq: controller timeout\n");
		goto out;
	}

	/*
	 * Load up IOPB
	 */
	iop = &sc->sc_iopb;
	if (bp) {
		iop->i_bufh = HB(bp->b_iobase);
		iop->i_bufm = MB(bp->b_iobase);
		iop->i_bufl = LB(bp->b_iobase);
		if ((cmd == C_TPREAD) || (cmd == C_TPWRITE)) {
			blk_cnt = bp->b_bcount >> BBSHIFT;	/* #blocks */
			iop->i_blkcnth = MB(blk_cnt);
			iop->i_blkcntl = LB(blk_cnt);
			/*
			 * XXX could add a count here to skip
			 *     file marks before read/write.
			 */
		}
	}
	iop->i_cmd = cmd;
	iop->i_status = 0;
	iop->i_error = 0;
	iop->i_unit = R1FMT(st->st_unit);
#ifdef	SIQ_DEBUG
	if (siq_noise) {
		splx(s);
		printf("siqcmd: issuing cmd = %x\n", cmd);
		s = spl6();
	}
#endif
	iop->i_tpoption = 0;
	iop->i_dmaburst = O_DMABURST;
	switch (cmd) {
		case C_TPCONFIG:
			iop->i_tpoption = OC_TPOPTIONS;
			iop->i_timeout = O_TIMEOUT;
			break;

		case C_TPWRITE:
			/* no filemark after write */
			iop->i_tpoption &= ~OW_FMARK; 
			break;

		case C_TPFSR:
			iop->i_tpoption &= ~OR_FOR_REV;   /* forward */
			break;

		case C_TPREWIND:
		case C_TPRETENSION:
		case C_TPERASE:
			st->st_fileno = 0;
			st->st_flags &= ~(ST_EOT | ST_WRITTEN | ST_ENDOFFILE);
			break;

		case C_TPWEOF:
			st->st_fileno++;
			st->st_flags &= ~(ST_WRITTEN | ST_ENDOFFILE);
			break;

		case C_TPFSF:
			st->st_fileno += count;
			st->st_flags &= ~(ST_WRITTEN | ST_ENDOFFILE);
			break;
	}
	iop->i_filecountl = LB(count);
	iop->i_filecounth = MB(count);
	sc->sc_buf = bp;		/* must be valid addr or 0
					 * for the interrupt handler
					 */

	/*
	 *  Issue the command
	 */
	if (ret_val = csr_write(sc, ST_START)) {
		set_res(ctlr_res, RES_BIT_ERR);
		splx(s);
		printf("sq: timeout writing to CSR, cmd = %x\n", cmd);
		goto out;
	}

	/*
	 * Setup timeout values for anyone who waits
	 * on the command just started.
	 */
	switch (cmd) {
		case C_TPREWIND:
			tape_timeout = TIME_REWIND;
			break;

		case C_TPREAD:
		case C_TPWRITE:
		case C_TPVERIFY:
			tape_timeout = TIME_RDWR;
			break;

		case C_TPFSF:
		case C_TPFSR:
		case C_TPERASE:
			tape_timeout = TIME_FSF;
			break;

		case C_TPRETENSION:
			tape_timeout = TIME_RESET;
			break;

		default:
			tape_timeout = TIME_WAIT;
			break;
	}

	busy_res(ctlr_res, tape_timeout*hz);
	busy_res(tape_res, tape_timeout*hz);

	if (wait) {
		set_res(ctlr_res, RES_BIT_LCK);
		set_res(tape_res, RES_BIT_LCK);
		if (!cansleep)
			splx(s);

		ret_val = RES_ERRNO(ctlr_res);
		while (ret_val == 0 && RES_BUSY_IGNLCK(ctlr_res))
			ret_val = WAITFOR(ctlr_res, PRIBIO, cansleep);

		if (ret_val == 0)
			ret_val = RES_ERRNO(tape_res);
		while (ret_val == 0 && RES_BUSY_IGNLCK(tape_res))
			ret_val = WAITFOR(tape_res, PRIBIO, cansleep);

		if (!cansleep)
			s = spl6();
		if (RES_TIMED_OUT(ctlr_res)) {
			splx(s);
			printf("sq: controller timeout\n");
			s = spl6();
		}
		clear_res(ctlr_res, RES_BIT_LCK);
		clear_res(tape_res, RES_BIT_LCK);
	}
	splx(s);
out:
#ifdef	SIQ_DEBUG
	if (siq_noise)
		printf("siqcmd: ret_val = %x\n", ret_val);
#endif
	return (ret_val);
}

/*
 * csr_write
 *	- write to command register R0
 *	- return 0 or EIO
 */
csr_write(sc, cmd)
  register struct softc *sc;
  int cmd;
{
	int attempts, i;
	register iopb_t *iop = &sc->sc_iopb;

	/*
	 * Wait for CSR busy to clear
	 */
	attempts = SPINS_PER_SEC/200;
	while (((i = STATUSREG(sc)) & CSR_BUSY) && (--attempts > 0))
			DELAY(20);
	if (attempts == 0)
		return EIO;

	*(STR0(sc)) = (u_char) cmd;

	/*
	 * Wait for controller to become busy before proceding.
	 */
	attempts = SPINS_PER_SEC/200;
	while ((((i = STATUSREG(sc)) & CSR_BUSY) == 0)
	  && (iop->i_status == 0) && (--attempts > 0))
		DELAY(20);
	if (attempts == 0)
		return EIO;

	/* Delay a while longer after a RESET */
	if (cmd == ST_RESET) {
		attempts = SPINS_PER_SEC / 1000;
		DELAY(attempts);
#ifdef	lint
	i = i;
#endif
	}

	return 0;
}


st_hard_reset(sc)
  register struct softc *sc;
{
	return 0;
}


siq_delay(cansleep)			/* delay a bit */
  int cansleep;
{
	int ticks;

	if (cansleep)
		delay(hz/4);
	else {
		ticks = SPINS_PER_SEC / 1000;
		DELAY(ticks);
	}
}
