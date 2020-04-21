/*
 * Disk driver for the xylogics 421 st506/qic-20 disk-tape controller.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/sys/multibus/RCS/xyf.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:31:48 $
 */

#include "xf.h"

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
#include "machine/cpureg.h"
#include "../multibus/xyfreg.h"
#include "../multibus/mbvar.h"

/* XXX */
struct {
	long	throttle;
} tune = {
	0x1f,
};

#undef DEBUG

/* errors codes */
struct	dkerror xyferrs[] = {
	{ 0x10,	"illegal cylinder address" },
	{ 0x12,	"soft ecc corrected" },
	{ 0x14,	"hard data ecc error" },
	{ 0x16,	"drive faulted" },
	{ 0x18,	"failed self test 1" },
	{ 0x1A,	"soft ecc error" },
	{ 0x20,	"illegal head address" },
	{ 0x22,	"ecc error ignored" },
	{ 0x24,	"header not found" },
	{ 0x26,	"header error, head/cylinder mismatch" },
	{ 0x28,	"failed self test 2" },
	{ 0x2A,	"tape exception error" },
	{ 0x30,	"illegal sector address" },
	{ 0x32,	"auto seek retry recovered" },
	{ 0x34,	"drive not ready or offline" },
	{ 0x38,	"failed self test 3" },
	{ 0x40,	"busy conflict" },
	{ 0x42,	"read retry recovered" },
	{ 0x44,	"operation timeout" },
	{ 0x48,	"failed self test 4" },
	{ 0x50,	"sector/track/block count zero" },
	{ 0x54,	"slave ack error (non-existent memory)" },
	{ 0x58,	"failed self test 5" },
	{ 0x60,	"unimplemented command" },
	{ 0x64,	"disk sequencer error" },
	{ 0x70,	"illegal command sequence" },
	{ 0x74,	"unexpected file mark detected" },
	{ 0x80,	"illegal drive type" },
	{ 0x84,	"pll lock failure" },
	{ 0x90,	"illegal unit number" },
	{ 0x94,	"write protect error" },
};
#define	NERRS	(sizeof(xyferrs) / sizeof(struct dkerror))

int	xyfstrategy();
int	xyfprobe(), xyfattach(), xyfstart(), xyfintr();
struct	mb_device *xyfdinfo[NXF];
struct	mb_ctlr *xyfcinfo[NXYF];
struct	mb_driver xyfdriver = {
	xyfprobe, xyfattach, (int (*)())0, xyfstart, xyfintr,
	(char *(*)())0, "xy", xyfdinfo, "xyf", xyfcinfo,
};

/*
** xyfprobe:
**	- probe the controller, and see if its there
**	- initialize it, to get things going
**	- remember that the port address has to be byte swapped 
*/
xyfprobe(reg)
	int reg;
{
	register struct xyreg *r;
	register long addr;

	xyfsoftc.sc_reg = r = (struct xyreg *)(MBIO_VBASE + reg);

	/* reset controller */
	r->csr = CCR_CRST;
	DELAY(10000);

	/* load iopb address */
	addr = VTOP(&xyfsoftc.sc_iopb);
	r->rrhigh = 0;
	r->rrlow = HB(addr);
	r->arhigh = MB(addr);
	r->arlow = LB(addr);
	DELAY(10000);

	if (!(r->csr & CSR_ADRM)) {
		printf("[ADRM not strapped right] ");
		return CONF_DEAD;
	}

	/* return status */
	xyfsoftc.sc_flags = SC_ALIVE;
	return CONF_ALIVE;
}

/*
** xyfattach: disk -- attach a disk to its controller
**	- configure to 1 cyl, 1 head and 64 sectors
**	- read the label
**	- reconfigure to cyl/hd/sec in label
**	- stash file system info into static structure
*/
xyfattach(mi)
	struct mb_device *mi;
{
	register struct softc_disk *sd;
#ifdef	notdef
	register struct buf *bp;
	register struct disk_label *l;
#endif
	register struct iopb *iop = &xyfsoftc.sc_iopb;
	register short unit = mi->mi_unit;

	printf("(NO LABEL SUPPORT YET -- ASSUMING VERTEX) ");
	sd = &xyfsoftc.sc_disk[unit];
	sd->sc_flags = SC_ALIVE;
	sd->sc_cyl = 987;
	sd->sc_hd  = 7;
	sd->sc_sec = 17;
	sd->sc_spc = sd->sc_hd * sd->sc_sec;
	sd->sc_fs[0].d_base = 1 * 119;
	sd->sc_fs[0].d_size = 150 * 119;

	if (xyreset(unit))
		return CONF_DEAD;

	/* init vertex in a hardwired way for now */
	iop->i_unit = unit | IU_WINNY;
	iop->i_driveoption = DO_HST_7us | (sd->sc_hd - 1);
	iop->i_maxsector = sd->sc_sec - 1;
	iop->i_maxcyllow = LB(sd->sc_cyl - 1);
	iop->i_maxcylhigh = MB(sd->sc_cyl - 1);
	iop->i_bpslow = LB(BLKSIZE);
	iop->i_bpshigh = MB(BLKSIZE);
	iop->i_rwcsclow = 0;			/* XXX is this right? */
	iop->i_rwcschigh = 0;			/* XXX is this right? */
	if (xyfcmd(IC_SETPARAMS, 1)) {
		printf("(set params) ");
		return CONF_DEAD;
	}
	return CONF_ALIVE;

#ifdef	notdef
	/*
	 * Configure to read label
	 * xyfinit(unit, skew, gap1, gap2, groupsize, ilv, heads, sectors)
	 */
	bp = geteblk(2);	/* Grab a block to read label into */
	if (xyfinit(unit, &default_iopb))
		goto error;
	/*
	** Set up the iopb for the read after the basic init
	*/
	iop->i_unit = unit;
	iop->i_bufh = HB(bp->b_un.b_addr);
	iop->i_bufm = MB(bp->b_un.b_addr);
	iop->i_bufl = LB(bp->b_un.b_addr);
	iop->i_head = iop->i_cylh = iop->i_cyll = iop->i_sech = iop->i_secl =0;
	iop->i_scch = 0;
	iop->i_sccl = 1;
	if (xyfcmd(C_READNOCACHE, 1))
		goto error;
	l = (struct disk_label *)bp->b_un.b_addr;
	if (l->d_magic != D_MAGIC) {
		dk_prname((struct disk_label *)NULL);
		goto error;
	}
					/* Re-configure from label */
	if (xyfinit(unit, l->d_cylskew, (int)l->d_misc[0], (int)l->d_misc[1],
			 (int)l->d_misc[2], l->d_interleave, l->d_heads,
			 l->d_sectors))
		goto error;

    /* save label info and init drive software state */
	sd = &xyfsoftc.sc_disk[unit];
	sd->sc_flags = SC_ALIVE;
	sd->sc_cyl = l->d_cylinders;
	sd->sc_hd  = l->d_heads;
	sd->sc_sec = l->d_sectors;
	sd->sc_spc = sd->sc_hd * sd->sc_sec;
	bcopy((caddr_t)l->d_map, (caddr_t)sd->sc_fs, sizeof sd->sc_fs);
	dk_prname(l);
	setroot(&xyfdriver, mi, unit << 3, 0070, l, xyfstrategy);
	goto out;

error:
	result = CONF_FAULTED;

out:
	brelse(bp);
	return result;
#endif
}

/* ARGSUSED */
/*
** xyfopen(dev, flag) - Called upon the open to check some very basic things
**		      - About the disk.
*/
/*ARGSUSED*/
xyfopen(dev, flag)
	dev_t dev;
	int flag;
{
	register short unit = D_DRIVE(dev);

	if ((unit > MAX_DRIVES) ||
	    !(xyfsoftc.sc_disk[unit].sc_flags & SC_ALIVE)) {
		u.u_error = ENODEV;
		return;
	}

	if (minor(dev) & 0x80)
		xyformat(dev);
}

/*
** xyfstrategy(bp) - Called to do the first checking on the disk request and
**		   - queue up the request via a call to disksort and check
**		   - the active before calling xyfstart.
*/
xyfstrategy(bp)
	register struct buf *bp;
{
	register struct disk_map *fs;
	register short unit = D_DRIVE(bp->b_dev);
	register struct softc_disk *sd;
	daddr_t bn;
	short temp;

	sd = &xyfsoftc.sc_disk[unit];
	if (!(sd->sc_flags & SC_ALIVE)) {
		berror(bp);
		return;
	}
	fs = &sd->sc_fs[D_FS(bp->b_dev)];
	if (dk_rangecheck(bp, fs, BLKSIZE, BLKSHIFT))
		return;

	/* crunch disk address for start and disksort */
	bn = bp->b_blkno + fs->d_base;
	bp->b_cyl = bn / sd->sc_spc;
	temp = bn % sd->sc_spc;
	bp->b_head = temp / sd->sc_sec;
	bp->b_sector = temp % sd->sc_sec;

	(void) SPL();
	disksort(&xyfsoftc.sc_tab, bp);
	if (xyfsoftc.sc_tab.b_active == 0)
		xyfstart();
	(void) spl0();
}

/*
** xyfstart()
** Now set up the iopb and start the request using the bp pointed to by the
** xyfsoftc.sc_table.
*/
xyfstart()
{
	register struct buf *bp;
	register struct disk_map *fs;
	register struct iopb *iop = &xyfsoftc.sc_iopb;
	register struct softc_disk *sd;
	register daddr_t bn;
	register short unit;

top:
	if ((bp = xyfsoftc.sc_tab.b_actf) == 0)
		return;
	unit = D_DRIVE(bp->b_dev);
	sd = &xyfsoftc.sc_disk[unit];
	fs = &sd->sc_fs[D_FS(bp->b_dev)];
	bn = bp->b_blkno + fs->d_base;

	xyfsoftc.sc_tab.b_active = 1;
	xyfsoftc.sc_tab.io_unit = unit;

	/*
	** Set up the IOPB
	*/
	iop->i_unit = unit | IU_WINNY;
	iop->i_head = bp->b_head;
	iop->i_sector = bp->b_sector;
	iop->i_cyllow = LB(bp->b_cyl);
	iop->i_cylhigh = MB(bp->b_cyl);

	bn = (bp->b_bcount + BLKSIZE - 1) >> BLKSHIFT;
	iop->i_scclow = LB(bn);
	iop->i_scchigh = MB(bn);

	bn = VTOP(bp->b_un.b_addr);
	iop->i_buflow = LB(bn);
	iop->i_bufhigh = MB(bn);
	iop->i_rellow = HB(bn);
	iop->i_relhigh = 0;
	iop->i_throttle = tune.throttle;
	iop->i_mode = IM_RTRY | IM_ASR | IM_ECM_CORRECT;

	/* fire off command */
	if (xyfcmd((bp->b_flags & B_READ) ? IC_READ : IC_WRITE, 0)) {
		printf("xy%d%c: can't start command\n",
				unit, D_FS(bp->b_dev) + 'a');
		bp->b_flags |= B_ERROR;
		xyfsoftc.sc_tab.b_actf = bp->av_forw;
		xyfsoftc.sc_tab.b_errcnt = 0;
		xyfsoftc.sc_tab.b_active = 0;
		bp->b_resid = 0;
		iodone(bp);
		goto top;
	}

	/* instrument the transfer now that its going */
	bn = xyfdinfo[unit]->mi_dk;		/* dk unit # */
	dk_busy |= 1<<bn;
	dk_xfer[bn]++;
	dk_wds[bn] += bp->b_bcount >> 6;
}

xyfread(dev)
	dev_t dev;
{
	physio(xyfstrategy, &xyfsoftc.sc_diskbuf, dev, B_READ, minphys);
}

xyfwrite(dev)
	dev_t dev;
{
	physio(xyfstrategy, &xyfsoftc.sc_diskbuf, dev, B_WRITE, minphys);
}

/*
** xyfintr() - process disk interrupts. return non-zero if the interrupt
**	was for this controller.
*/
xyfintr()
{
	register struct iopb *iop = &xyfsoftc.sc_iopb;
	register struct xyreg *r = xyfsoftc.sc_reg;
	register struct buf *bp;
	register short unit;

#ifdef	notdef
duputchar('x');
#endif
	/* see if the interrupt was for us */
	if (!(xyfsoftc.sc_flags & SC_ALIVE))
		return;				/* nope */

	/* check controller registers */
	if (!(r->csr & CSR_CRDY) || !(iop->i_csb0 & IS0_DONE)) {
		printf("xyf0: interrupt and csr=%x csb0=%x\n",
			      r->csr, iop->i_csb0);
		r->csr = CCR_CLRI;
		return;
	}

	/* clear interrupt and validate interrupt */
	r->csr = CCR_CLRI;
	unit = xyfsoftc.sc_tab.io_unit;
	if (xyfsoftc.sc_tab.b_active == 0) {
		printf("xyf0: unrequested interrupt\n");
		return;
	}
	bp = xyfsoftc.sc_tab.b_actf;
	dk_busy &= ~(1<<xyfdinfo[unit]->mi_dk);

	/* check error status */
	if ((r->csr & (CSR_ERR | CSR_DERR)) || (iop->i_csb0 & IS0_ERSM)) {
		printf("xy%d: error, command=%x csb0=%x csb1=%x error=%s\n",
			      iop->i_unit & IU_UNITMASK,
			      iop->i_command,
			      iop->i_csb0, iop->i_csb1,
			      dkerror(iop->i_csb1, xyferrs, NERRS));
		bp->b_flags |= B_ERROR;
		r->csr = CCR_CLRE;
	}

	/* complete command and start next one */
	xyfsoftc.sc_tab.b_actf = bp->av_forw;
	xyfsoftc.sc_tab.b_errcnt = 0;
	xyfsoftc.sc_tab.b_active = 0;
	if (xyfsoftc.sc_tab.b_actf)
		xyfstart();
	bp->b_resid = 0;
	iodone(bp);
}

xyfcmd(cmd, waiting)
	int cmd, waiting;
{
	register struct iopb *iop = &xyfsoftc.sc_iopb;
	register struct xyreg *r = xyfsoftc.sc_reg;
	register long timo;

	/*
	 * setup remaining parts of iopb and start controller
	 */
	iop->i_csb0 = iop->i_csb1 = 0;		/* XXX */
	iop->i_command = cmd | IC_AUD;
	if (!waiting)
		iop->i_command |= IC_ITI;
	r->csr = CCR_GO;
	if (!waiting)
		return 0;

    	/*
	 * wait for command to finish
	 */
	timo = 10000000;
	while (!(r->csr & CSR_CRDY) && --timo)
		;
	if (timo == 0) {
		iprintf("xy%d: timeout, cmd=%x csb0=%x csb1=%x csr=%x\n",
				   iop->i_unit & IU_UNITMASK,
				   iop->i_command,
				   iop->i_csb0, iop->i_csb1, r->csr);
		return 1;
	}

	/*
	 * command is done, look at status
	 */
#ifdef	DEBUG
	iprintf("csb0=%x csb1=%x csr=%x\n",
			     iop->i_csb0, iop->i_csb1, r->csr);
#endif
	if ((r->csr & (CSR_ERR | CSR_DERR)) || (iop->i_csb0 & IS0_ERSM)) {
		iprintf("xy%d: cmd=%x csb0=%x csb1=%x csr=%x, error=%s\n",
				   iop->i_unit & IU_UNITMASK,
				   iop->i_command,
				   iop->i_csb0, iop->i_csb1, r->csr,
				   dkerror(iop->i_csb1, xyferrs, NERRS));
		r->csr = CCR_CLRE;
		return 1;
	}
	return 0;
}

/*
** Used by unix to print a diagnostic
*/
xyfprint(dev, str)
char *str;
{
	printf("%s on xy%d%c\n", str, D_DRIVE(dev), 'a'+D_FS(dev));
}

/*
 * xyformat:
 *	- format the given drive
 */
xyformat(dev)
	dev_t dev;
{
	register int cyl, unit;
	register struct iopb *iop = &xyfsoftc.sc_iopb;
	register struct softc_disk *sd;

	unit = D_DRIVE(dev);
	sd = &xyfsoftc.sc_disk[unit];
	if (xyreset(unit)) {
		iprintf("xy%d: reset failed\n", unit);
		u.u_error = EIO;
		return;
	}

	/* format each cylinder */
	for (cyl = 0; cyl < sd->sc_cyl; cyl++) {

		/* fill in iopb for format command */
		iop->i_unit = unit | IU_WINNY;
		iop->i_head = 0;
		iop->i_sector = 0;
		iop->i_cyllow = LB(cyl);
		iop->i_cylhigh = MB(cyl);
		iop->i_scclow = LB(sd->sc_hd);
		iop->i_scchigh = MB(sd->sc_hd);
		iop->i_mode = IM_RTRY | IM_ASR | IM_ECM_CORRECT;

		if (xyfcmd(IC_FORMAT, 1)) {
			iprintf("xy%d: format error on cyl=%d\n",
					   unit, cyl);
			u.u_error = EIO;
			return;
		}
		if ((cyl % 10) == 0)
			iprintf("%3d ", cyl);
	}
}

/*
 * xyreset:
 *	- reset the given drive
 */
xyreset(unit)
	int unit;
{
	/* reset whatever drive is out there */
	xyfsoftc.sc_iopb.i_unit = unit | IU_WINNY;
	return xyfcmd(IC_DRIVERESET, 1);
}
