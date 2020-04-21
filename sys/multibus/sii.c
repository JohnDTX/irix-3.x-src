/*
** sii.c	- Copyright (C) Silicon Graphics Inc.
**		- chase bailey - July 1985
**		- Any use, copy or alteration is strictly prohibited
**		- unless authorized in writing by SGI.
**
**	$Source: /d2/3.7/src/sys/multibus/RCS/sii.c,v $
**	$Revision: 1.1 $
**	$Date: 89/03/27 17:31:32 $
*/

#include "si.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/iobuf.h"
#include "../h/file.h"
#include "../h/dklabel.h"
#include "../h/dk.h"
#include "../h/dkerror.h"
#include "../h/flpio.h"
#include "machine/cpureg.h"
#include "../multibus/siireg.h"
#include "../multibus/mbvar.h"
#include "../multibus/siiuib.h"		/* Default UIBS */
#include "../multibus/siilist.h"	/* The lists are too long */
#include "../h/dkio.h"			/* Mainly for debug */

#undef	SII_DEBUG
#undef	SII_NOISE
#undef	FLP_DEBUG
#if defined(SII_DEBUG) || defined(SII_NOISE)
#define	printf	iprintf
#endif

#ifdef	SII_DEBUG
int	sii_options = (CACHEENABLE | ZEROLATENCY | CBREQENABLE);
#endif

/* XXX when the sed script works... */
#define	swapb	SWAPB

/*
 * Currently, the storager 2 will get "no address mark in header field"
 * errors out of the blue when using a esdi drives - even when they are
 * hard sectored.  If you disable the cache, the bug SEEMS to go away.
 */
#define	SII_BUG1

/*
** Somehow the reading of register 0 reads that the tape unit interrupted
** This should only read the disk stuff and not the tape stuff. Register 4
** is dedicated to tape.
*/
#define SII_BUG2

/* XXX move this junk into siisoftc */
char	sii_dmacount = 8;		/* Fastest access to the bus */
char	sii_ininit = 0;			/* Are we in initialization */
long	sii_iopb_mbva;
long	sii_formatlock = 0;

/* stuff for  Queuing */
char	sii_nextiopb = 0;		/* Always the next iopb to be loaded */
char	sii_activeiopbs = 0;		/* Number of active iopbs */
char	sii_inqueuedmode = 0;		/* Make sure your in the proper mood */

int	sifprobe(), sifattach(), sifioctl(), sifstrategy();
int	siistrategy();
char	*siisname();
int	siiprobe(), siiattach(), siiintr();
struct	mb_device *siidinfo[NSI];
struct	mb_ctlr *siicinfo[NSII];
struct	mb_driver siidriver = {
	siiprobe, siiattach, (int (*)())0, (int (*)())0, siiintr,
	siisname, "si", siidinfo, "sii", siicinfo,
};

/*
 * Configuration stuff.  During attach, the driver tries each of these
 * uib's to find a drive.  This way, the controller supports many types
 * of drives, automagically.  Wheeeeee.
 */
uib_t	*sii_uibs[] = {
	&sii_hesdi_uib,
	&sii_sesdi_uib,
	&sii_st506_uib,
};
#define	NUIBS	(sizeof(sii_uibs) / sizeof(uib_t *))

#ifdef juniper
#define TIMES(x)	( 200000*(x) )
#else
#define TIMES(x)	( 20000*(x) )
#endif

/*
** siiprobe:
**	- probe the controller, and see if its there
**	- initialize it, to get things going
**	- remember that the port address has to be byte swapped 
**	- Went throught this probe/reset code with CCantrell and
**	- This is the way it should be done with the storagers.
*/
siiprobe(reg)
	int reg;
{
	long timeout;

	sii_inqueuedmode = 0;
	siisoftc.sc_cioaddr = (caddr_t)MBIO_VBASE + reg;
	siisoftc.sc_sioaddr = (short *)MBIO_VBASE + reg;

	RESET();
	timeout = TIMES(1);
	while (--timeout)
		;			/* Delay a little while */
	ZEROREG0();			/* Write a 0x00 to Register 0 */

	/* Wait for the done bit to go away */
	timeout = TIMES(5);
	while ((STATUSREG() & ST_DONE) && --timeout)
		;
	if (timeout == 0)
		return (CONF_DEAD);

	/* Wait for the done bit to come true */
	timeout = TIMES(5);
	while (((STATUSREG() & ST_DONE) == 0) && --timeout)
		;
	if (timeout == 0)
		return (CONF_DEAD);

	CLEAR();			/* Write a 0x02 to Register 0 */
	timeout = TIMES(1);
	while (--timeout)
		;			/* Delay a little while */

	/* wait for the done bit to come back on */
	timeout = TIMES(50);
	while ((STATUSREG() & ST_DONE) && --timeout)
		;
	if (timeout == 0)
		return (CONF_DEAD);

	sii_iopb_mbva = mbmapkget((caddr_t)&siisoftc.sc_iopb,
				  (long)sizeof(siisoftc.sc_iopb));
	siisoftc.sc_flags = SC_ALIVE;
	return (CONF_ALIVE);
}

char *
siisname(mi)
	struct mb_device *mi;
{
	static char *msg[2] = { "si", "sf" };

	return msg[(short) (mi->mi_flags & 1)];
}

sifattach(unit, cyl, hd, secs)
{
	register struct softc_disk *sf;

	sii_inqueuedmode = 0;
	sii_ininit = 1;
	if (unit != 0) {
error:
		sii_ininit = 0;
		return 1;
	}
	if (siiinit(unit+FLP_OFFSET, &sii_mits_uib))
		goto error;
	/* Don't need to check for a special kind of error to
	** determine whether a floppy drive is present.
	** Don't have time right now.
	*/
	sf = &siisoftc.sc_floppy[unit];
	sf->sc_flags |= SC_ALIVE;
	sf->sc_cyl = cyl;
	sf->sc_sec = secs;
	sf->sc_hd  = hd;
	sf->sc_spc = sf->sc_hd * sf->sc_sec;
	sf->sc_fs[0].d_size = sf->sc_cyl * sf->sc_spc;
	sii_ininit = 0;
	printf("floppy (%d/%d/%d) ", sf->sc_cyl, sf->sc_hd, sf->sc_sec);
	return 0;
}

/*
** siiattach: disk -- attach a disk to its controller
**	- Initialize to a basic 1cylinder disk and read the label.
**	- Reconfigure to the Information found in the label.
**	- Stash file system info into static structure from the label.
**	- We try to read ESDI type disk First -- Hard Sectored.
**	- We try to read ESDI type disk Second -- Soft Sectored.
**	- We try to read ST506 type disk Third and Last.
**	- Otherwise we return an error.
*/
siiattach(mi)
	struct mb_device *mi;
{
	register struct disk_label *l;
	register iopb_t *iop = &siisoftc.sc_iopb;
	register struct softc_disk *si;
	register struct buf *bp;
	register short unit = mi->mi_unit;
	int result = CONF_ALIVE;
	u_short temp;
	long timeout;
	int i;

	if (mi->mi_flags & 1) {
		if (sifattach(unit, 80, 2, 8))
			return CONF_FAULTED;
		si = &siisoftc.sc_floppy[unit];
		si->sc_flags = SC_ALIVE;
		return CONF_ALIVE;
	}

	sii_inqueuedmode = 0;
	sii_ininit = 1;
	/* Configure to read label */
	bp = getdmablk(1);

	for (i = 0; i < NUIBS; i++) {
		if (siiinit(unit, sii_uibs[i]))
			goto error;
		iop->err_stat = 0;
		iop->head_unit = unit;
		iop->cyl = 0;
		iop->sec = 0;
		iop->scc = swapb(1);
		iop->bufh_dma = ((sii_dmacount)|(HB(bp->b_iobase)<<8));
		temp = (u_short)bp->b_iobase;
		iop->bufl = swapb(temp);
		if (siicmd(C_READNOCACHE, unit) == 0)
			goto okie_dokie;
	}
	goto error;

okie_dokie:
	l = (struct disk_label *)bp->b_un.b_addr;
	if (l->d_magic != D_MAGIC) {
		dk_prname((struct disk_label *)NULL);
		goto error;
	}
	si = &siisoftc.sc_disk[unit];
	sii_build_uib(l, &si->sc_uib);
	if (siiinit(unit, &si->sc_uib))
		goto error;

	/* save label info and init drive software state */
	si->sc_flags = SC_ALIVE;
	si->sc_cyl = l->d_cylinders;
	si->sc_hd  = l->d_heads;
	si->sc_sec = l->d_sectors;
	si->sc_spc = si->sc_hd * si->sc_sec;
	bcopy((caddr_t)l->d_map, (caddr_t)si->sc_fs, sizeof si->sc_fs);
	dk_prname(l);
	setroot(&siidriver, mi, unit << 3, 0070, l, siistrategy);
	goto out;

error:
	timeout = TIMES(5);
	SIIABORT();
	/* Wait for the done bit to go away */
	while ((STATUSREG() & ST_DONE) && --timeout) ;
	result = CONF_FAULTED;

out:
	sii_ininit = 0;
	brelse(bp);
	return (result);
}

/*
** si:
**	- return an ascii representation for the drive name
*/
char *
siname(dev)
	dev_t dev;
{
	static char name[5] = "si";

	name[2] = D_DRIVE(dev) + '0';
	name[3] = D_FS(dev) + 'a';
	return (name);
}

/*
** siiopen - Called upon the open to check some very basic
**	     things About the disk
*/
/*ARGSUSED*/
siiopen(dev, flag)
	dev_t dev;
	int flag;
{
	register short unit = D_DRIVE(dev);

	if ((unit > MAX_WINNYS) ||
	    !(siisoftc.sc_disk[unit].sc_flags & SC_ALIVE)) {
		u.u_error = ENODEV;
		return;
	}
}

/*ARGSUSED*/
sifclose(dev)
	dev_t dev;
{
#ifdef	notdef
	register short unit = D_DRIVE(dev);
	/* Not implemented yet */
#endif

}
/*ARGSUSED*/
sifopen(dev, flag)
	dev_t dev;
	int flag;
{
	register short unit = D_DRIVE(dev);

	if (!(siisoftc.sc_floppy[unit-FLP_OFFSET].sc_flags & SC_ALIVE)) {
		u.u_error = ENODEV;
		return;
	}
}
/*
** sifstrategy(bp) - Called to do the first checking on the disk request and
*/
sifstrategy(bp)
	register struct buf *bp;
{
	register struct disk_map *fs;
	register struct softc_disk *sf;
	register daddr_t bn;
	register short temp;
	register short unit;
	register long s;

	unit = D_DRIVE(bp->b_dev);
	if (unit < FLP_OFFSET) {
		berror(bp);
		return;
	}
	sf = &siisoftc.sc_floppy[unit-FLP_OFFSET];
	if (!(sf->sc_flags & SC_ALIVE)) {
		berror(bp);
		return;
	}

	fs = &sf->sc_fs[D_FS(bp->b_dev)];
	if (dk_rangecheck(bp, fs, BLKSIZE, BLKSHIFT)) {
#ifdef	SII_DEBUG
		printf("rangecheck failed, bn=%d d_size=%d\n",
				   bp->b_blkno, fs->d_size);
#endif
		return;
	}

	/* crunch disk address */
	bn = bp->b_blkno + fs->d_base;
	bp->b_cyl = bn / sf->sc_spc;
	temp = bn % sf->sc_spc;
	bp->b_head = temp / sf->sc_sec;
	bp->b_sector = temp % sf->sc_sec;

	s = spl6();		/* High priority */
#ifdef	ASSERT
	bp->b_error = B_TOBEDONE;
#endif
	if (siisoftc.sc_tab.b_actf)
		disksort(&siisoftc.sc_tab, bp);
	else {
		siisoftc.sc_tab.b_actf = bp;
		siisoftc.sc_tab.b_actl = bp;
		bp->av_forw = NULL;
	}
	if (sii_activeiopbs < NUMBERIOPBS)
		siistart();
	splx(s);
}

/*
** siistrategy(bp) - Called to do the first checking on the disk request and
**		   - queue up the request via a call to disksort and check
**		   - the active before calling siistart.
*/
siistrategy(bp)
	register struct buf *bp;
{
	register struct disk_map *fs;
	register struct softc_disk *si;
	register daddr_t bn;
	register short temp;
	register short unit;
	register long s;

	unit = D_DRIVE(bp->b_dev);
	si = &siisoftc.sc_disk[unit];
	if (!(si->sc_flags & SC_ALIVE)) {
		berror(bp);
		return;
	}

	fs = &si->sc_fs[D_FS(bp->b_dev)];
	if (dk_rangecheck(bp, fs, BLKSIZE, BLKSHIFT)) {
#ifdef	SII_DEBUG
		printf("rangecheck failed, bn=%d d_size=%d\n",
				   bp->b_blkno, fs->d_size);
#endif
		return;
	}

	/* crunch disk address */
	bn = bp->b_blkno + fs->d_base;
	bp->b_cyl = bn / si->sc_spc;
	temp = bn % si->sc_spc;
	bp->b_head = temp / si->sc_sec;
	bp->b_sector = temp % si->sc_sec;

	s = spl6();		/* High priority */
#ifdef	ASSERT
	bp->b_error = B_TOBEDONE;
#endif
	if (sii_formatlock || siisoftc.sc_tab.b_actf)
		disksort(&siisoftc.sc_tab, bp);
	else {
		siisoftc.sc_tab.b_actf = bp;
		siisoftc.sc_tab.b_actl = bp;
		bp->av_forw = NULL;
	}
	if (sii_formatlock || sii_activeiopbs < NUMBERIOPBS)
		siistart();
	splx(s);
}

siistart()
{
	register struct buf *bp;
	register iopb_t *iop;
	register u_long bn;
	register unit;
	register u_long dkn;

	for (;;) {
		/*
		 * If there are no more buffers on the active queue, or there
		 * are already too many commands going on the controller,
		 * bust out of the loop.
		 */
		if (((bp = siisoftc.sc_tab.b_actf) == NULL) ||
		    (sii_activeiopbs >= NUMBERIOPBS))
			break;

#ifdef	ASSERT
		/* Calculate the iopb address */
		if (bp->b_error != B_TOBEDONE) {
			printf("not to be done, bp=%x\n", bp);
			debug("siistart");
		}
#endif

		iop = (struct iopb *)(SC_IOPBBASE + (sii_nextiopb + 1) * 0x20);
		if (!sii_inqueuedmode) {
			*STR1 = 0xFF;
			*STR2 = 0xFF;
			*STR3 = 0xFF;
			dkn = 5000;
			while (--dkn)
				;
			SETNUMIOPBS(sii_nextiopb);
			START();
			sii_inqueuedmode = 1;
		}

		unit = D_DRIVE(bp->b_dev);
		iop->cyl = swapb(bp->b_cyl);
		iop->head_unit = (bp->b_head << 8) | unit;
		iop->sec = swapb(bp->b_sector);
		iop->err_stat = 0;
		bn = ((bp->b_bcount + BLKSIZE - 1) >> BLKSHIFT);
		iop->scc = swapb(bn);
		bn = (u_long)bp->b_iobase;
		iop->bufh_dma = sii_dmacount | ((bn & 0xFF0000) >> 8);
		iop->bufl = swapb(bn);
		if (bp->b_flags & B_READ)
			iop->option_cmd = (O_OPTIONS << 8) | C_READ;
		else
			iop->option_cmd = (O_OPTIONS << 8) | C_WRITE;
/*
iop->bufh_dma = sii_dmacount | (HB(bn) << 8);
iop->option_cmd = ((O_OPTIONS<<8)|(bp->b_flags&B_READ?C_READ:C_WRITE));
*/
		iop->iopb_bp = bp;

		/* Start the Sucker a goin' */
		SC_ENABLECW(sii_nextiopb);
		sii_activeiopbs++;

		/* instrument the transfer now that its going */
		if (unit < FLP_OFFSET) {
			dkn = siidinfo[unit]->mi_dk;		/* dk unit # */
			dk_busy |= 1<<dkn;
			dk_xfer[dkn]++;
			dk_wds[dkn] += bp->b_bcount >> 6;
		}

		/* get the next iopb to use calculated */
		if (++sii_nextiopb > (NUMBERIOPBS-1))
			sii_nextiopb = 0;
		siisoftc.sc_tab.b_actf = bp->av_forw;
	}
}

sifread(dev)
	dev_t dev;
{
	int unit;

	if (u.u_count & (BLKSIZE-1)) {
		u.u_error = EIO;
		return;
	}
	unit = D_DRIVE(dev) - FLP_OFFSET;
	if (physck(siisoftc.sc_floppy[unit].sc_fs[D_FS(dev)].d_size,
						      B_READ, BLKSHIFT))
		physio(sifstrategy, (struct buf *)NULL, dev, B_READ, minphys);
}

sifwrite(dev)
	register dev_t dev;
{
	int unit;

	if (u.u_count & (BLKSIZE-1)) {
		u.u_error = EIO;
		return;
	}
	unit = D_DRIVE(dev) - FLP_OFFSET;
	if (physck(siisoftc.sc_floppy[unit].sc_fs[D_FS(dev)].d_size,
						      B_WRITE, BLKSHIFT))
		physio(sifstrategy, (struct buf *)NULL, dev, B_WRITE, minphys);
}
siiread(dev)
	dev_t dev;
{
	if (u.u_count & (BLKSIZE-1)) {
		u.u_error = EIO;
		return;
	}
	if (physck(siisoftc.sc_disk[D_DRIVE(dev)].sc_fs[D_FS(dev)].d_size,
					   B_READ, BLKSHIFT))
		physio(siistrategy, (struct buf *)NULL, dev, B_READ, minphys);
}

siiwrite(dev)
	register dev_t dev;
{
	if (u.u_count & (BLKSIZE-1)) {
		u.u_error = EIO;
		return;
	}
	if (physck(siisoftc.sc_disk[D_DRIVE(dev)].sc_fs[D_FS(dev)].d_size,
					   B_WRITE, BLKSHIFT))
		physio(siistrategy, (struct buf *)NULL, dev, B_WRITE, minphys);
}

#ifdef	notyet
/*
 * siiintr:
 *	- for each controller that probed, call its interrupt routine to
 *	  see if it interrupted
 */
siiintr()
{
	register struct softc *sc;
	register short ctlr;
	register int didsomething;

	sc = &siqsoftc[0];
	didsomething = 0;
	for (ctlr = NSII; --ctlr >= 0; sc++) {
		if (sc->sc_flags & SC_ALIVE)
			didsomething += siicintr(sc);
	}
	return (didsomething);
}
#endif

#ifdef	SII_DEBUG
/*
** siizot:
**	- called by level5 routine to print out registers when a stray
**	  interrupt occurs
*/
siizot()
{
	printf("sii: r0=%x r1=%x r2=%x\n",
		     (unsigned)*STR0, (unsigned)*STR1, (unsigned)*STR2);
}
#endif

/*
** siiintr -- handles both interrupts for the floppy and the disk.
*/
siiintr()
{
	register struct iopb *iop;
	register struct buf *bp;
	register u_char unit;
	register u_char thisiopb;
	register u_char error;
	u_char command;
	u_char sr, iunit, tapestatus;

	/* Board is not here */
	if (!(siisoftc.sc_flags & SC_ALIVE))
		return (0);

	/* capture board registers */
	sr = STATUSREG();
	iunit = RDINTR();
	tapestatus = TAPESTATUS();

	/* I did not cause the interrupt */
	if (!(sr & (ST_DONE | ST_STCHANGE)))
		return (0);

	unit = iunit;
	thisiopb = (unit >> 3) & 0x0F;
	unit &= 0x07;

#ifdef SII_BUG2
	if (unit > 3) {
#ifdef	notdef
		printf("siiintr: unit=%d, csr=%x\n", unit, sr);
#endif
		return (0);
	}
#endif

	iop = (struct iopb *)(SC_IOPBBASE + (thisiopb + 1) * 0x20);
	bp = iop->iopb_bp;
	iop->iopb_bp = NULL;
	if (bp == NULL) {
#ifdef	SII_DEBUG
		printf("si0: r0=%x r1=%x r2=%x\n", sr, iunit, tapestatus);
		siipp(unit, thisiopb, 0);
#endif
		panic("null bp in siiintr");
	}

#ifdef	ASSERT
	if (bp->b_error != B_TOBEDONE) {
		printf("bp=%x\n", bp);
		debug("siiintr");
	}
#endif
	CLEAR();
	if (unit < FLP_OFFSET)
		dk_busy &= ~(1<<siidinfo[unit]->mi_dk);	/* clean up metering */

	/* Check to see it there is an error */
	if (sr & ST_ERROR) {
		iop = (struct iopb *)(SC_IOPBBASE + (thisiopb + 1) * 0x20);
		command = iop->option_cmd & 0xFF;
		error = (iop->err_stat >> 8) & 0xFF;
		printf("%s: hard error: %s, at: %d/%d/%d, cmd: %s\n",
			    siname(bp->b_dev),
			    dkerror(error, sii_errs, sii_nerrs),
			    swapb(iop->cyl), ((iop->head_unit>>8)&0xff),
			    swapb(iop->sec),
			    dkerror(command, sii_cmds, sii_ncmds));
#ifdef	FLP_DEBUG
		if (unit == 2)
			siipp(unit, thisiopb, &sii_mits_uib);
#endif
		sii_inqueuedmode = 0;
		bp->b_flags |= B_ERROR;

		/* XXX why is this done */
		if (++thisiopb > (NUMBERIOPBS-1))
			thisiopb = 0;
		SETNUMIOPBS(thisiopb);
	}
	sii_activeiopbs--;
	sii_waitforack(sr, iunit, tapestatus);
	if (siisoftc.sc_tab.b_actf && (sii_activeiopbs == 0))
		siistart();
	iodone(bp);
	return (1);
}

/*
 * sii_waitforack:
 *	- wait for controller to acknowledge clear interrupt
 */
sii_waitforack(r0, r1, r2)
	u_char r0, r1, r2;
{
	u_char tr0, tr1, tr2;
	long waiting;

	r0 &= ~ST_BUSY;
	waiting = 1000;
	while (--waiting) {
		tr0 = STATUSREG() & ~ST_BUSY;
		tr1 = RDINTR();
		tr2 = TAPESTATUS();
		if ((tr0 != r0) || (tr1 != r1) || (tr2 != r2))
			return;
	}
	printf("si0: timeout waiting for interrupt to clear!\n");
	printf("si0: r0=%x r1=%x r2=%x\n", r0, r1, r2);
	printf("si0: tr0=%x tr1=%x tr2=%x\n", tr0, tr1, tr2);
}

/*ARGSUSED*/
siicmd(cmd, unit)
	int cmd, unit;
{
	register iopb_t *iop;
	register long timeout;
	register long s;
	u_char status;

/***/
	s = spl6();
	timeout = TIMES(10);
	while ((STATUSREG() & ST_BUSY) && --timeout)
		;
	if(!timeout) {
		splx(s);
		return 1;
	}
	iop = &siisoftc.sc_iopb;

	iop->option_cmd = ((O_OPTIONS<<8) | cmd);
	iop->err_stat = 0;
	STARTWO();
#ifdef juniper
	timeout = 5000;
#else

	timeout = 500;
#endif
	while (--timeout);

	/* Wait for status */
	timeout = TIMES(200);
	for (;;) {
		status = (iop->err_stat) & 0xFF;
		if (status == S_OK) {
			splx(s);
			return 0;
		}
		if (status == S_ERROR) {
#ifdef	SII_DEBUG
			command = (iop->option_cmd) & 0xFF;
		        iprintf("[si%d: cmd: %s stat %x err %s statreg %x]\n",
			  unit,
			  dkerror((u_char)command, sii_cmds, sii_ncmds), status,
			  dkerror((u_char)(iop->err_stat>>8)&0xFF,
				sii_errs, sii_nerrs), STATUSREG());
#endif
			splx(s);
			return 1;
		}
		if ((--timeout) == 0) {
#ifdef	SII_DEBUG
			command = (iop->option_cmd) & 0xFF;
			printf("[si%d: Cmd=%x Timeout: s %x err %x] ", unit,
				command, status, (iop->err_stat>>8) & 0xFF);
#endif
			splx(s);
			return 1;
		}
	}
}

/*
** siiinit() -- unit type and mode (check to see if inited)
*/
siiinit(unit, uib)
	register int unit;
	register uib_t *uib;
{
	register iopb_t *iop = &siisoftc.sc_iopb;
	int zero = 0;
	long uib_mbva;

	uib_mbva = mbmapkget((caddr_t)uib, (long)sizeof(uib_t));

	iop->head_unit = unit;
	iop->bufh_dma = (sii_dmacount)|((HB(uib_mbva))<<8);
	iop->bufl = swapb(uib_mbva);
	iop->cyl = 0;
	iop->sec = 0;
	iop->scc = 0;

	/* Set up the iopb address in the I/O of the Controller */
	*STR1 = HB(sii_iopb_mbva);
	*STR2 = MB(sii_iopb_mbva);
	*STR3 = LB(sii_iopb_mbva);

	zero = TIMES(1);
	while (--zero);
	/*
	** Set up some standard stuff for Queuing to work.
	** This is only the enable (DISABLE of overlapped seeks.
	** And the set up of the # of iopbs with a starting iopb.
	*/
	*(short *)(siisoftc.sc_cioaddr+0x1C)=zero;
#ifdef juniper
	zero = 100;
#else
	zero = 2;
#endif
	while (--zero);
	SETNUMIOPBS(sii_nextiopb);
/*
	*(short *)(siisoftc.sc_cioaddr+0x1E)=((sii_nextiopb<<8)|(NUMBERIOPBS&0xFF));
*/

	zero = siicmd(C_INIT, unit);
	mbmapkput(uib_mbva, (long)sizeof(uib_t));
	return (zero);
}

/*
 * sii_build_uib:
 *	- build up a uib for a drive
 */
sii_build_uib(l, uib)
	register struct disk_label *l;
	register uib_t *uib;
{
	uib->u_spt = l->d_sectors;	/* sectors per track */
	uib->u_hds = l->d_heads;	/* heads per cylinder */
	uib->u_bpsl = LB(BLKSIZE);	/* bytes per sector */
	uib->u_bpsh = MB(BLKSIZE);	/* bytes per sector */
	uib->u_gap1 = l->d_misc[0];	/* Gap 1 */
	uib->u_gap2 = l->d_misc[1];	/* Gap 2 */
	uib->u_gap3 = l->d_misc[2];	/* Gap 3 */
	uib->u_ilv = l->d_interleave;	/* Interleave */
	uib->u_retry = 0x10;		/* retries for the Firmware */
	uib->u_eccon = 1;		/* ECC enabled */
	uib->u_reseek	= 1;		/* Reseek on Error */
	uib->u_mvbad	= 0;		/* Move Bad Data */
	uib->u_inchd	= 1;		/* Increment by head */
	uib->u_resv0 	= 0;		/* Reserved */
	uib->u_intron	= 0;		/* Interrupt on status change */

	uib->u_skew = l->d_cylskew;	/* Cylinder Skew */
	uib->u_resv1 = 0;		/* Group Size (2190) */
	uib->u_mohu = l->d_misc[10];	/* Motor On and Head Unload */
#ifdef	SII_DEBUG
	uib->u_options = sii_options;
#else
	uib->u_options = l->d_misc[13];	/* Cache and Zero Latency */
#endif
	uib->u_ddb = l->d_misc[11];	/* Drive Descriptor Byte */
	uib->u_smc = l->d_misc[12];
	uib->u_spw = l->d_misc[3];
	uib->u_spil = LB(l->d_misc[4]);
	uib->u_spih = MB(l->d_misc[4]);
	uib->u_hlst = l->d_misc[8];
	uib->u_ttst = l->d_misc[5];
	uib->u_ncl = LB(l->d_cylinders);
	uib->u_nch = MB(l->d_cylinders);
	uib->u_wpscl = LB(l->d_misc[6]);
	uib->u_wpsch = MB(l->d_misc[6]);
	uib->u_rwcscl = LB(l->d_misc[7]);
	uib->u_rwcsch = MB(l->d_misc[7]);
}

/*
** Used by unix to print a diagnostic
*/
siiprint(dev, str)
char *str;
{
	printf("%s on %s\n", str, siname(dev));
}

/*ARGSUSED*/
sifprint(dev, str)
char *str;
{
	printf("%s on sf0a\n", str);
}

#ifdef FLP_DEBUG
siipp(unit, thisiopb, uib)
	u_char unit, thisiopb;
	register uib_t *uib;
{
	register iopb_t *iop;

	if (!sii_inqueuedmode) {
		iop = &siisoftc.sc_iopb;
	} else {
		iop = (struct iopb *)(SC_IOPBBASE + (thisiopb + 1) * 0x20);
	}
	printf("SI%d: ", (iop->head_unit)&0xFF);
	printf("iop: c:%s %d/%d/%d e_s:%x scc:%d buf:%x\n",
		dkerror((iop->option_cmd&0xFF), sii_cmds, sii_ncmds),
		swapb(iop->cyl), (iop->head_unit>>8) & 0xFF,
		swapb(iop->sec), iop->err_stat, swapb(iop->scc),
		(((iop->bufh_dma & 0xFF00) << 8) | swapb(iop->bufl)));
#ifdef NOTDEF
	return;
#endif NOTDEF
	printf("uib %x: h %d, s %d, bps %d, gaps %d %d %d, ilv (%d)%d, sk %d\n",
		uib,
		uib->u_hds, uib->u_spt, ((uib->u_bpsh <<8) | uib->u_bpsl),
		uib->u_gap1, uib->u_gap2, uib->u_gap3, uib->u_ilv,
		(uib->u_ilv&0x3f), uib->u_skew);
	printf("uib2: mohu %x, opt %x, ddb %x, smc %x, spw %x, spi %x\n",
		uib->u_mohu, uib->u_options, uib->u_ddb, uib->u_smc,
		uib->u_spw, ((uib->u_spih <<8) | uib->u_spil));
	printf("uib3: ttst %x nc %d wpsc %x rwcsc %x \n",
		uib->u_ttst, ((uib->u_nch<<8) | uib->u_ncl),
		((uib->u_wpsch<<8) | uib->u_wpscl),
		((uib->u_rwcsch<<8) | uib->u_rwcscl));
}
#endif FLP_DEBUG

/*ARGSUSED*/
sifioctl(dev, cmd, addr, flag)
	caddr_t addr;
	dev_t dev;
{
	register short unit = D_DRIVE(dev);
	register struct softc_disk *sf = &siisoftc.sc_floppy[unit-FLP_OFFSET];
	struct flpop flpop;

	switch(cmd) {
	  case FLP_IOCTOP:
		if(copyin((caddr_t)addr, (caddr_t)&flpop, sizeof(flpop))) {
			u.u_error = EFAULT;
			break;
		}
		switch(flpop.flp_op) {
		  case FLP_FORMAT:
			if (siiflpformat(unit,sf->sc_cyl,sf->sc_hd)) {
				u.u_error = EIO;
			}
			break;
		  default:
			u.u_error = EINVAL;
			break;
		}
		break;
	  case FLP_IOCGET:
	  	default:
			u.u_error = EINVAL;
	}
}

siiflpformat(unit,cyl,hd)
	char unit;
	u_short cyl,hd;
{
	register cylinders, heads;
	register iopb_t *iop = &siisoftc.sc_iopb;

	sii_formatlock = 1;

	printf(" *** Storager Format Floppy %d: cyls=%d sides=%d ***\n",
			unit, cyl, hd);
	for(cylinders = 0; cylinders < cyl; cylinders++) {
	    for(heads = 0; heads < hd; heads++) {
		iop->cyl = swapb(cylinders);
		iop->head_unit = (heads << 8) | unit;
		iop->sec = 0;
		iop->scc = swapb(8);
		iop->bufh_dma = sii_dmacount;
		iop->bufl = 0;

		sii_inqueuedmode = 0;
		/* Set up the iopb address in the I/O of the Controller */
		*STR1 = HB(sii_iopb_mbva);
		*STR2 = MB(sii_iopb_mbva);
		*STR3 = LB(sii_iopb_mbva);
		if (siicmd(C_FORMAT, unit)) {
			printf("  Format err at %d/%d\n", cylinders, hd);
			sii_formatlock = 0;
			return 1;
		}
	    }
	    if(cylinders && (cylinders%10) == 0)
		printf("%3d ",cylinders);
	}
	printf("\n");
	sii_formatlock = 0;
	return 0;
}
