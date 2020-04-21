/*
 *	tmt.c  -  Copyright (C) SGI
 */

#include "tm.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/iobuf.h"
#include "../h/file.h"
#include "../h/mtio.h"
#include "machine/cpureg.h"
#include "../multibus/wait.h"
#include "../multibus/tmtreg.h"
#include "../multibus/mbvar.h"


#ifdef OS_DEBUG
#define TMT_DEBUG 1
#endif

#ifdef TMT_DEBUG
#define printf trace_printf
#define iprintf trace_printf
#define uprintf trace_printf

int tmt_noise = 0;
int tmt_noise2 = 0;
int tmt_signal = S_DEFAULT;

#include "../h/dkerror.h"
#include "../multibus/tmtlist.h"
extern	struct dkerror tmt_errs[];
extern	short tmt_nerrs, tmt_ncmds;
#endif

int	tmtprobe(), tmtattach(), tmtintr(), tmtstrategy();

static struct mb_device *tmtdinfo[NTM];		/* info per drive */
static struct mb_ctlr *tmtcinfo[NTMT];		/* info per controller */
struct mb_driver tmtdriver = {
	tmtprobe, tmtattach, (int (*)())0, (int (*)())0, tmtintr,
	(char *(*)())0, "mt", tmtdinfo, "tmt", tmtcinfo,
};

#define ctlr_res (&sc->sc_ctlr_res)
#define tape_res (&st->st_tape_res)


/*
 * tmtprobe  -  probe the controller by issuing a reset to the I/O address
 */
tmtprobe(reg)
  int reg;
{
	register struct softc *sc = &tmtsoftc;
	u_long addr;

	sc->sc_ioaddr = (caddr_t)((MBIO_VBASE + reg));
	sc->sc_pblk_mbva = mbmapkget((caddr_t)&sc->sc_pb, sizeof (para_t));
	sc->sc_cblk_mbva = mbmapkget((caddr_t)&sc->sc_cb, sizeof (cblk_t));

	if (tmtreset(sc)) {
		mbmapkput(sc->sc_pblk_mbva, sizeof (para_t));
		mbmapkput(sc->sc_cblk_mbva, sizeof (cblk_t));
		return (CONF_FAULTED);
	}
	sc->sc_flags |= SC_ALIVE;
	printf("(1/2\" tape) ");
	return (CONF_ALIVE);
}

/*
 * Init some values for a particular unit
 */
tmtattach(mi)
	register struct mb_device *mi;
{
	register struct softc *sc = &tmtsoftc;
	register short unit = mi->mi_unit;
	register struct softc_tape *st;
	
	st = &sc->sc_tape[unit];
	st->st_unit = unit;
	st->st_ctlr = sc;
	if (tmtconfig(sc, NOSLEEP))
		return (CONF_FAULTED);
	st->st_flags = SC_ALIVE;
	return CONF_ALIVE;
}

/*
 * tmtreset -  reset the controller
 */
int
tmtreset(sc)
  register struct softc *sc;
{
	register int timer = SPINS_PER_SEC/10;
	register u_long addr = sc->sc_pblk_mbva;

	RESET();
	delay(hz);
	while (--timer && (CSTATUS() & CSR_BUSY))
		;
	if (timer == 0)
		return (1);

	/*
	 * If delays aren't used here, the 12.5 MHZ 68010 will
	 * give a bus error on the second access.
	 */
	delay(1);
	ADDR0(addr);
	delay(1);
	ADDR1(addr);
	delay(1);
	ADDR2(addr); 		/* mask off the top nibble in HB() */
	delay(1);
	ADDR3(0);		/* only 24-bit address */

	sc->sc_cb.tape1 = 0;		/* reset unit 0 & 1 configs */
	sc->sc_cb.tape2 = 0;		/* reset unit 2 & 3 configs */
	return 0;
}

/*
 * tmtconfig -- configure the controller
 *	assumes tape1 & tape2 are already initialized in cblock.
 */
tmtconfig(sc, cansleep)
  register struct softc *sc;
  int cansleep;
{
	register cblk_t *cb = &sc->sc_cb;
	int s;
	
	s = SPL();
	cb->throttle	= CB_THROTTLE;
#ifdef TMT_DEBUG
	cb->signals	= tmt_signal;
#else
	cb->signals	= S_DEFAULT;
#endif
	cb->retries	= CB_RETRIES;
	splx(s);
	if (tmtcmd(CMD_CONFIG, 0, 0, sc->sc_cblk_mbva, WAIT, cansleep))
		return (1);
	return (0);
}

/*
 * cblk_init - init configuration block entries for a particular device.
 *		if (new config block != old), reconfigure controller.
 */
cblk_init(sc, dev)
  register struct softc *sc;
  dev_t dev;
{
/*
 *  tape control bytes:
 *	tape2:	hi-order nibble for unit 3,  lo-order nibble for unit 2
 *	tape1:	hi-order nibble for unit 1,  lo-order nibble for unit 0
 */


	register cblk_t *cb = &sc->sc_cb;
	register int unit = UNIT(dev);
	u_char *tpctl_byte_ptr;
	u_char tpctl_nibble = 0;
	u_char 	bus_ctl = 0;		/* B_BYTESWAP = 0 */
	u_char tpctl_byte, old_tpctl_byte;
	u_char tap_minor = TAPEMINOR(dev);
	int s;

	tpctl_byte_ptr = (unit < 2) ? &cb->tape1 : &cb->tape2;
	old_tpctl_byte = *tpctl_byte_ptr;

	if (tap_minor & BYTESWAP)
		bus_ctl |= B_BYTESWAP;
	if ((tap_minor & DENS_LO) == 0)
		tpctl_nibble |= T_HIGHDEN;
	if (tap_minor & SPEED_HI)
		tpctl_nibble |= T_HIGHSPD;
	if (unit & 1)			/* if unit 1 or 3 ... */
		tpctl_byte = (tpctl_nibble << 4) + (old_tpctl_byte & 0xf);
	else
		tpctl_byte = tpctl_nibble + (old_tpctl_byte & 0xf0);

#ifdef TMT_DEBUG
#if 0
	tpctl_byte |= (tpctl_nibble << 4);	/* do units 0 & 1 */
#endif
	if (tmt_noise)
		printf("cblk_init: minor = %x, tapctl = %x, bus = %x\n",
			tap_minor, tpctl_byte, bus_ctl);
#endif
	if ((tpctl_byte != old_tpctl_byte) || (bus_ctl != cb->bus)) {
		s = SPL();
		*tpctl_byte_ptr = tpctl_byte;
		cb->bus = bus_ctl;
		splx(s);
		return (tmtconfig(sc, CANSLEEP));
	}
	return 0;
}

/*
 * tmtopen
 */
tmtopen(dev, flag)
  dev_t dev;
  int flag;
{
	register int unit = UNIT(dev);
	register struct buf *bp;
	register struct softc_tape *st;
	register struct softc *sc = &tmtsoftc;
	register long s;

	if ((unit > NTM) ||
	  	(!((st = &sc->sc_tape[unit])->st_flags & SC_ALIVE))) {
		u.u_error = ENODEV;
		return;
	}

#ifdef TMT_DEBUG
	if (tmt_noise) {
		printf("st = %x\n", (long) st);
		printf("sc = %x\n", (long) sc);
	}
#endif
	s = SPL();
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
		if (tmtreset(sc) || tmtconfig(sc, CANSLEEP)) {
			uprintf("tmt%d: tape drive inaccessable\n", unit);
			u.u_error = EIO;
			goto error;
		}
		s = SPL();
	}

	/*
	 * Wait for previous command (rewind) to complete
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
				uprintf("tmt%d: tape error during open\n",
					unit);
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

	/*
	 * Reconfigure if necessary due to opening a wierd device
	 */
	if (cblk_init(sc, dev)) {
		uprintf("tmt%d: can't configure device\n", unit);
		u.u_error = EIO;
		goto error;
	}

	if (u.u_error = tmtstatus(st)) {
		s = SPL();
		set_res(ctlr_res, RES_BIT_ERR);
		goto reset_ctlr;
	}

	/*
	 * Try to put the drive online, if it's not.
	 */
	if (!(st->st_status & DS_ONL)) {
		if (tmtcmd(CMD_ONLINE, unit, 0, NULL_BP, WAIT, CANSLEEP)) {
			uprintf("tmt%d: tape drive not online\n", unit);
			u.u_error = EIO;
			goto error;
		}
		/*
		 * Is the drive online now?
		 */
		if (u.u_error = tmtstatus(st))
			goto error;
		if (!(st->st_status & DS_ONL)) {
			uprintf("tmt%d: tape drive not online\n", unit);
			u.u_error = EIO;
			goto error;
		}
	}

	if (st->st_status & DS_BOT) {
		st->st_fileno = 0;
		st->st_flags &= ~(ST_EOT | ST_WRITTEN | ST_ENDOFFILE);
	}

	if ((st->st_status & DS_FPT) && (flag & FWRITE)) {
		uprintf("tmt%d: tape is write protected\n", unit);
		u.u_error = EIO;
		goto error;
	}

	/*
	 * If this is the rewind device and we are not at BOT, rewind.
	 */
	if (!(TAPEMINOR(dev) & NOREWIND) && !(st->st_status & DS_BOT)) {
		if (u.u_error = tmtcmd(CMD_REWIND, unit, 0, NULL_BP,
		   NOWAIT, CANSLEEP))
			goto error;
	}
	return;
error:
	s = SPL();
	st->st_flags &= ~ST_OPEN;
	splx(s);
}

/*
 * tmtclose:
 *	- close the device
 *	- write closing file mark if the tape has been written
 *	- rewind the tape, if this is not the no-rewind device
 */
/*ARGSUSED*/
tmtclose(dev)
  dev_t dev;
{
	register int unit = UNIT(dev);
	register struct softc *sc = &tmtsoftc;
	register struct softc_tape *st = &sc->sc_tape[unit];
	int s;

	if (RES_ANYERR(ctlr_res) || RES_ANYERR(tape_res))
		goto out;

	if (st->st_flags & ST_WRITTEN) {
	    	/* write closing file mark */
#ifdef	TMT_DEBUG
		if (tmt_noise)
			printf("tmtclose: writing filemarks\n");
#endif
		if (u.u_error = tmtcmd(CMD_WFM, unit, 1, NULL_BP, WAIT,
		   CANSLEEP))
			goto rew;
		if (u.u_error = tmtcmd(CMD_WFM, unit, 1, NULL_BP, WAIT,
		   CANSLEEP))
			goto rew;
		if (TAPEMINOR(dev) & NOREWIND) {
		    if (u.u_error = tmtcmd(CMD_FSF, unit, -1, NULL_BP, WAIT,
			CANSLEEP))
			goto rew;
		}
	}
rew:
    	/* attempt to rewind tape, if it's requested */
	if (!(TAPEMINOR(dev) & NOREWIND)) {
		if (u.u_error = tmtcmd(CMD_REWIND, unit, 0, NULL_BP, NOWAIT,
		   CANSLEEP)) {
			uprintf("tmt%d: can't rewind tape\n", unit);
			goto out;
		  }
	}
out:
	/* Clean up all the flags that were used */
	s = SPL();
	st->st_flags &= ~(ST_WRITTEN | ST_ENDOFFILE | ST_OPEN | ST_EOT);
	splx(s);
}


tmtread(dev)
  register dev_t dev;
{
	short unit = UNIT(dev);
	register struct softc *sc = &tmtsoftc;
	register struct softc_tape *st = &sc->sc_tape[unit];

#ifdef	TMT_DEBUG
	if (tmt_noise)
		printf("tmtread: START\n");
#endif
	if (rw_parm_error(st)) {
		u.u_error = EIO;
#ifdef	TMT_DEBUG
		if (tmt_noise)
			printf("tmtread: parm error\n");
#endif
		return;
	}

	/* if at end-of-file, return 0 byte count */
	if (st->st_flags & ST_ENDOFFILE) {
#ifdef	TMT_DEBUG
		if (tmt_noise)
			printf("tmtread: EOF flag alaready set\n");
#endif
		return;
	}

	physio(tmtstrategy, (struct buf *)NULL, dev, B_READ|B_TAPE, minphys);
#ifdef	TMT_DEBUG
	if (tmt_noise)
		printf("tmtread: u_error = %d, u_count = %d\n",
			u.u_error, u.u_count);
#endif
}


tmtwrite(dev)
  register dev_t dev;
{
	short unit = UNIT(dev);
	register struct softc *sc = &tmtsoftc;
	register struct softc_tape *st = &sc->sc_tape[unit];

	if (rw_parm_error(st)) {
		u.u_error = EIO;
#ifdef	TMT_DEBUG
		if (tmt_noise)
			printf("tmtwrite: parm error\n");
#endif
		return;
	}
#ifdef	TMT_DEBUG
	if (tmt_noise)
		printf("tmtwrite: op started w/ no errors\n");
#endif
	physio(tmtstrategy, (struct buf *)NULL, dev, B_WRITE|B_TAPE, minphys);
}


rw_parm_error(st)
	register struct softc_tape *st;
{
	/*
	 * Count must be even.
	 */
	if (u.u_count & 1) {
		uprintf("tmt%d: odd byte count of %d\n",
			st->st_unit, u.u_count);
		u.u_error = EIO;
		return 1;
	}

	/*
	 * 32K blocksize is hdw limitation of some Cipher tape drives.
	 */
	if(u.u_count > (32*1024)) {
		uprintf("tmt%d: byte count of %d is > 32 KB\n",
			st->st_unit, u.u_count);
		u.u_error = EIO;
		return 1;
	}

	/* Give "no space" error if at EOT unless suppressed
	 * by MTIOCIEOT.
	 */
	if ((st->st_flags & (ST_EOT | ST_IGNEOT)) == ST_EOT) {
		u.u_error = ENOSPC;
#ifdef	TMT_DEBUG
	if (tmt_noise)
		printf("tmtr/w: EOT found\n");
#endif
		return 1;
	}
	return 0;
}

tmtstrategy(bp)
  register struct buf *bp;
{
	short unit = UNIT(bp->b_dev);
	register struct softc *sc = &tmtsoftc;
	register struct softc_tape *st = &sc->sc_tape[unit];
	register int cmd, s;

#ifdef	TMT_DEBUG
	if (tmt_noise)
		printf("tmtstrategy: bp = %x, count = %d\n",
			(u_long) bp, bp->b_bcount);
#endif

	/* Init count not done = request count */
	st->st_resid = bp->b_bcount; 

	cmd = (bp->b_flags & B_READ) ? CMD_READ : CMD_WRITE;
	if (u.u_error = tmtcmd(cmd, unit, 0, bp, WAIT, CANSLEEP)) {
		/* WAITing means berror() doesn't have to be done
		 * in the interrupt handler.
		 */
		berror(bp);
		return;
	   }

	if ((bp->b_flags & B_READ) == 0) {
		s = SPL();
		if (bp->b_error == ENOSPC) {
			/* if at EOT, keep tmtclose from writing EOF */
			st->st_flags &= ~ST_WRITTEN;
		} else
			st->st_flags |= ST_WRITTEN;
		splx(s);
	}
}

/*ARGSUSED*/
tmtioctl(dev, cmd, addr, flag)
  caddr_t addr;
  dev_t dev;
{
	int unit = UNIT(dev);
	register struct softc *sc = &tmtsoftc;
	register struct softc_tape *st = &sc->sc_tape[unit];
	struct mtop mtop;
	struct mtget mtget;

	switch(cmd) {
	  case MTIOCTOP:
		if (copyin((caddr_t)addr, (caddr_t)&mtop, sizeof(mtop))) {
			u.u_error = EFAULT;
			break;
		}
		switch (mtop.mt_op) {
		  case MTREW:
			u.u_error = tmtcmd(CMD_REWIND, unit, 0, NULL_BP,
				NOWAIT, CANSLEEP);
			break;
		  case MTWEOF:
			while (mtop.mt_count) {
				if (tmtcmd(CMD_WFM, unit, 1, NULL_BP, WAIT,
				   CANSLEEP)) {
					u.u_error = EIO;
					break;
				}
				mtop.mt_count--;
			}
			break;
		  case MTFSF:
			u.u_error = tmtcmd(CMD_FSF, unit, mtop.mt_count,
				NULL_BP, WAIT, CANSLEEP);
			break;
		  case MTBSF:
			u.u_error = tmtcmd(CMD_FSF, unit, -mtop.mt_count,
				NULL_BP, WAIT, CANSLEEP);
			break;
		  case MTFSR:
			u.u_error = tmtcmd(CMD_FSR, unit, mtop.mt_count,
				NULL_BP, WAIT, CANSLEEP);
			break;
		  case MTBSR:
			u.u_error = tmtcmd(CMD_FSR, unit, -mtop.mt_count,
				NULL_BP, WAIT, CANSLEEP);
			break;
		  case MTOFFL:
			u.u_error = tmtcmd(CMD_OFFLINE, unit, 0, NULL_BP,
				WAIT, CANSLEEP);
			break;
		  case MTRESET:
			u.u_error = tmtcmd(CMD_RESET, unit, 0, NULL_BP,
				WAIT, CANSLEEP);
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
			mtget.mt_blkno = 60;
			if (copyout((caddr_t)&mtget, addr, sizeof(mtget)))
				u.u_error = EFAULT;
			break;
		  case MTNOP:
			if (u.u_error = tmtstatus(st))
				return;
			bzero((caddr_t)&mtget, sizeof(mtget));
			mtget.mt_type = MT_ISTMT;

			mtget.mt_hard_error0 = st->st_status&0xff;
			mtget.mt_soft_error0 = ((st->st_status>>8)&0xff);
			mtget.mt_status = st->st_status;
			mtget.mt_fileno = st->st_fileno;
			mtget.mt_resid = st->st_resid;
			if (st->st_status & DS_BOT)
				mtget.mt_at_bot = 1;
			if (copyout((caddr_t)&mtget, addr, sizeof(mtget)))
				u.u_error = EFAULT;
			break;
		  default:
			u.u_error = EINVAL;
			break;
		}
		break;

#ifdef 0
	/*
	 * MTIOCIEOT and MTIOCEEOT are not yet implemented.
	 * All that is needed is to take out the above "ifdef 0"
	 * and add the following to mtio.h:
	 */
--- start mtio.h
/*
 * The following ioctl's determine whether I/O is allowed after an EOT
 * condition on a 1/2" tape.  Any such I/O still causes the ENOSPC error,
 * but I/O gets done to/from the tape anyway, and mt_resid (returned by
 * an MTIOCGET ioctl) will tell the #bytes not transferred.
 */
#define MTIOCIEOT	(('m'<<8)|3)	/* ignore EOT error (1/2" tape) */
#define MTIOCEEOT	(('m'<<8)|4)	/* enable EOT error (1/2" tape) */
--- end mtio.h

	  case MTIOCIEOT:		/* ignore EOT condition */
		st->st_flags |= ST_IGNEOT;
		break;


	  case MTIOCEEOT:		/* enable EOT condition */
		st->st_flags &= ~ST_IGNEOT;
		break;
#endif
	  default:
		u.u_error = EINVAL;
	}
}

tmtintr()
{
	register struct softc *sc = &tmtsoftc;
	register struct buf *bp;
	register para_t *pb;
	register struct softc_tape *st;
	int unit, cmd, sr, tapestatus, i;
	int error = 0;

	if (!(sc->sc_flags & SC_ALIVE))
		return (0);
	/*
	 * return 0 if not your interrupt, this will allow interrupt
	 * interleaving.
	 */
	sr = CSTATUS();
	if((sr & CSR_CCI) == 0) {
		CLEAR();
		return (0);
	}

	tapestatus = TPSTATUS();
	unit = tapestatus & TSR_UMASK;

	bp = sc->sc_buf;
	st = &tmtsoftc.sc_tape[unit];
	pb = &sc->sc_pb;
	cmd = pb->cmd;
	CLEAR();

#ifdef TMT_DEBUG
	if (tmt_noise) {
		iprintf("tmtintr: cmd=%x  R0(adaptor)=%x  R2(tape)=%x\
  pb_gate=%x  pb_err=%x  bp=%x\n",
		cmd, sr, tapestatus, pb->gate, pb->error, (u_long) bp);
	}
#endif
	ASSERT(unit == sc->sc_unit);
	if ((pb->gate & G_ERROR) || (CSTATUS() & CSR_ERROR)) {
		error = GETERROR();
		if (error == 0)
			error = pb->error;
	}

	if (error == 0) {
		switch (cmd) {
		  case CMD_READ:
		  case CMD_WRITE:
			bp->b_resid = 0;
			iodone(bp);
			break;

		  case CMD_STATUS:
			i = *(u_short *)bp->b_un.b_addr;
			st->st_status = SWAPB(i);
#ifdef TMT_DEBUG
			if (tmt_noise)
			    iprintf("tmtintr/STATUS: %x\n", st->st_status);
#endif
			break;
		}
	} else {	/* Controller returned an error code */

		if (cmd == CMD_READ || cmd == CMD_WRITE) {
			bp->b_resid = bp->b_bcount - SWAPB(pb->transfer);
			st->st_resid = bp->b_resid;
#ifdef TMT_DEBUG
			if (tmt_noise)
			    iprintf("tmtintr: pb_count = %d, pb_xfer = %d, \
new resid = %d\n", pb->count, pb->transfer, (u_int) st->st_resid);
#endif
			/*
			 * Check for non-error conditions
			 */
			if ((error == ERR_SOFTTERR) ||
			    ((error == ERR_READ_UNFL) && (cmd == CMD_READ))) {
				iodone(bp);
				goto out;
			}

			if ((error == ERR_FILEMARK) && (cmd == CMD_READ)) {
				st->st_flags |= ST_ENDOFFILE;
				st->st_fileno++;
				iodone(bp);
				goto out;
			}

			/*
			 * If EOT encountered, set B_ERROR for buffer
			 *    and return u.u_error = ENOSPC to user.
			 *
			 * But don't declare an error on tape_res, since
			 *   then tmtcmd would return EIO till next siqopen.
			 */
			if (error == ERR_EOT) {
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
		if (cmd == CMD_READ || cmd == CMD_WRITE)
			st->st_flags |= ST_ENDOFFILE;
		set_res(tape_res, RES_BIT_ERR);
		if (error > MAX_RECOV_ERR)
			set_res(ctlr_res, RES_BIT_ERR);
#ifdef TMT_DEBUG
		if (tmt_noise2) {
		    iprintf("tmt%d: hard error, cmd=\"%s\" err=\"%s\"\n",
			unit, dkerror((u_long)pb->cmd, tmt_cmds, tmt_ncmds),
			dkerror((u_long)error, tmt_errs, tmt_nerrs));
		}
#endif
	}

out:
#ifdef	TMT_DEBUG
	if (tmt_noise)
		iprintf("tmtcintr/out: st_flags = %x \n",
			(u_long) st->st_flags);
#endif
	clear_res(ctlr_res, RES_BIT_BUSY);
	clear_res(tape_res, RES_BIT_BUSY);
	return (1);
}

/*
 * tmtstatus:
 *	- get status about tape drive from controller
 *	- return unix error code or 0
 */
int
tmtstatus(st)
  register struct softc_tape *st;
{
	register struct buf *bp;
	struct softc *sc = st->st_ctlr;
	int ret_val, s;
	int attempts = 50;

	s = SPL();
	st->st_status = 0;
	splx(s);
	bp = getdmablk(1);

	while ((ret_val = tmtcmd(CMD_STATUS, st->st_unit, 0, bp,
	    WAIT, CANSLEEP)) && (--attempts > 0)) {

		if (RES_ANYERR(ctlr_res) || ret_val == EINTR)
			break;
		s = SPL();
		clear_res(tape_res, RES_BITS_ANYERR);
		splx(s);
		delay(hz/4);
	    }

	brelse(bp);
#ifdef TMT_DEBUG
	if (tmt_noise)
		printf("tmtstatus: rtn value = %d\n", ret_val);
#endif
	return (ret_val);
}

/*
 * tmtcmd
 *	- issue a command to the tape drive
 *	- return a unix error code if the command fails, 0 if it succeeds
 */
int
tmtcmd(cmd, unit, count, bp, wait, cansleep)
  int cmd, unit, count;
  register struct buf *bp;
  int wait;				/* wait for completion or not */
  int cansleep;				/* do we poll or sleep? */
{
	register struct softc *sc = &tmtsoftc;
	register struct softc_tape *st = &sc->sc_tape[unit];
	register para_t *pb = &sc->sc_pb;
	register u_long addr;
	int s, tape_timeout, direction;
	int ret_val = 0;

	/* If errors already encountered, don't allow any more i/o
	 * until the next open.
	 */
	if (RES_ANYERR(ctlr_res) || RES_ANYERR(tape_res))
		return (EIO);

#ifdef TMT_DEBUG
	if (tmt_noise)
		printf("tmtcmd/START, bp = %x\n", (u_long) bp);
#endif

	if (cansleep)
		s = SPL();
	/*
	 * Wait for pblock to be free
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
		s = SPL();
		if (RES_TAKEN(ctlr_res) || RES_TAKEN(tape_res)) {
			splx(s);
			goto again;
		}
	}
	if (ret_val) {
		splx(s);
		if (RES_TIMED_OUT(ctlr_res))
			printf("tmt: controller timeout\n");
		goto out;
	}

	/*
	 * Load up the paramater block
	 */
	pb->cmd = cmd;
	if (count >= 0)
		direction = C_FOR;
	else {
		count = -count;
		direction = C_REV;
	}
	pb->control = (0x80 | unit | direction | C_INTERRUPT);
	if (bp) {
		if (cmd == CMD_CONFIG) {
			pb->addr = swapitall((u_long) bp);
			pb->count = 0;
		} else {
			pb->addr = swapitall((u_long) bp->b_iobase);
			pb->count = SWAPB(bp->b_bcount);
		}
	} else {
		pb->addr = 0;
		pb->count = SWAPB(count);
	}
	pb->pblink = 0;
	pb->transfer = 0;
	pb->gate = 0;
	pb->error = 0;

	sc->sc_unit = unit;
	sc->sc_buf = bp;

	switch (cmd) {
		case CMD_REWIND:
		case CMD_RESET:
		case CMD_SECURE:
		case CMD_VARERASE:
			st->st_fileno = 0;
			st->st_flags &= ~(ST_EOT | ST_WRITTEN | ST_ENDOFFILE);
			break;

		case CMD_WFM:
			st->st_fileno++;
			st->st_flags &= ~(ST_WRITTEN | ST_ENDOFFILE);
			break;

		case CMD_FSF:
			if (direction == C_FOR)
				st->st_fileno += count;
			else {
				st->st_fileno -= count;
				if (st->st_fileno < 0)
					st->st_fileno = 0;
			}
			st->st_flags &= ~(ST_WRITTEN | ST_ENDOFFILE);
			break;
	}
	if ((direction == C_REV) &&
	    ((cmd == CMD_FSF) || (cmd == CMD_FSR)))
		st->st_flags &= ~ST_EOT;

	/*
	 *  Issue the command
	 */
#ifdef	TMT_DEBUG
	if (tmt_noise) {
		splx(s);
		if (bp && cmd != CMD_CONFIG) count = bp->b_bcount;
		printf("tmtcmd: issuing cmd = %x, count = %d\n", cmd, count);
		s = SPL();
	}
#endif
	if (ret_val = cmd_start(sc)) {
		set_res(ctlr_res, RES_BIT_ERR);
		splx(s);
		printf("tmt: timeout writing to CSR, cmd = %x\n", cmd);
		goto out;
	}

	/*
	 * Setup timeout values for anyone who waits
	 * on the command just started.
	 */
	switch (cmd) {
		case CMD_REWIND:
			tape_timeout = TIME_REWIND;
			break;

		case CMD_READ:
		case CMD_WRITE:
		case CMD_ERASE:
			tape_timeout = TIME_RDWR;
			break;

		case CMD_FSF:
		case CMD_FSR:
		case CMD_SECURE:
		case CMD_VARERASE:
			tape_timeout = TIME_FSF;
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
			s = SPL();
		if (RES_TIMED_OUT(ctlr_res)) {
			splx(s);
			printf("tmt: controller timeout\n");
			s = SPL();
		}
		clear_res(ctlr_res, RES_BIT_LCK);
		clear_res(tape_res, RES_BIT_LCK);
	}
	splx(s);
out:
#ifdef TMT_DEBUG
	if (tmt_noise)
		printf("tmtcmd: ret_val = %x\n", ret_val);
#endif
	return (ret_val);
}

/*
 * cmd_start
 *	- write to command register R0
 *	- return 0 or EIO
 */
cmd_start(sc)
  register struct softc *sc;
{
	int attempts;

	/*
	 * Wait for CSR busy to clear
	 */
	attempts = SPINS_PER_SEC/200;
	while ((CSTATUS() & CSR_BUSY) && (--attempts > 0))
			DELAY(20);
	if (attempts == 0)
		return EIO;

	START();
	return 0;
}
