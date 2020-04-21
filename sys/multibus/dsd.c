/*
** Disk driver for the dsd 5215/5217 disk/tape controller
**
** Written by: Chase Bailey
** Modified by: Bruce Borden & Kipp Hickman & Scott Carr & a cast of
**		thousands
**
** $Source: /d2/3.7/src/sys/multibus/RCS/dsd.c,v $
** $Revision: 1.1 $
** $Date: 89/03/27 17:31:18 $
**
** Note special test for SECTOR_NOT_FOUND in dsdintr().  The DSD
**  board returns this when it encounters an error reading a sector
**  header.  It does NO retries.  The controller SHOULD try several
**  sectors when it encounters this error, because the track is
**  probably an alternated out bad track... SO, we try many times
**  in software, and throw in a changing delay to prevent constant
**  retries at the same rotational position.
**
**  XXX - To do: fit everything that needs to be multibus accessible into
**		 the wub page.
*/

#include "md.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/buf.h"
#include "../h/iobuf.h"
#include "../h/file.h"
#include "../h/dklabel.h"
#include "../h/mtio.h"
#include "../vm/vm.h"
#include "../h/dk.h"
#include "../h/dkerror.h"
#include "../h/flpio.h"
#include "../h/autoconf.h"
#include "machine/cpureg.h"
#include "machine/cx.h"
#include "../multibus/dsdreg.h"
#include "../multibus/mbvar.h"

long	swapw();

#undef	PARANOID
#undef TAPEDEBUG
	/* Lastaccess was the kludge for control C out of a no rewind tape */
	/* We cannot use this because of other problems */
#undef LASTACCESS

/* lock controller against other usage */
#define	DSDLOCK(active, unit) { \
	register int sps = SPL(); \
	while (mdsoftc.sc_tab.b_actf || (mdsoftc.sc_flags & SC_BUSY)) { \
		mdsoftc.sc_flags |= SC_WANTED; \
		sleep((caddr_t)&mdsoftc, PZERO-1); \
	} \
	mdsoftc.sc_flags |= SC_BUSY; \
	mdsoftc.sc_tab.b_active = active; \
	mdsoftc.sc_tab.io_s1 = unit; \
	splx(sps); \
}

/* unlock controller & give to waiting proc */
#define	DSDUNLOCK() { \
	if (mdsoftc.sc_flags & SC_WANTED) { \
		wakeup((caddr_t)&mdsoftc); \
	} \
	mdsoftc.sc_flags &= ~(SC_BUSY|SC_WANTED); \
	mdsoftc.sc_tab.b_active = 0; \
}

/*
** get controller from tape (used by disk code)
** (assumes running high priority)
*/
#define	DSDGET() { \
	while (mdsoftc.sc_flags & SC_BUSY) { \
		mdsoftc.sc_flags |= SC_WANTED; \
		sleep((caddr_t)&mdsoftc, PZERO-1); \
	} \
}

/* error codes */
struct	dkerror dsderrs[] = {
	{ 0x14,	"ram checksum error" },
	{ 0x15,	"rom checksum error" },
	{ 0x16, "seek in progress" },
	{ 0x17, "illegal format type" },
	{ 0x18, "end of media" },
	{ 0x21, "illegal sector size" },
	{ 0x22, "diagnostic fault" },
	{ 0x23, "no index pulse" },
	{ 0x24, "invalid command" },
	{ 0x25, "sector not found" },
	{ 0x26, "invalid address" },
	{ 0x27, "unit is not ready" },
	{ 0x28, "unit write protected" },
	{ 0x34, "data ECC error" },
	{ 0x35, "id field ECC error" },
	{ 0x36, "drive faulted" },
	{ 0x37, "cylinder address miscompare" },
	{ 0x38, "seek error" },
	{ 0x41, "no data field found" },
	{ 0x42, "wrong type of data" },
	{ 0x43, "index too early" },
	{ 0x44, "index too late" },
	{ 0x45, "read write controller error" },
	{ 0x46, "bus timeout" },
	{ 0x47, "no drive" },
	{ 0x51, "no tape in the drive" },
	{ 0x52, "tape is write protected" },
	{ 0x53, "no tape power" },
	{ 0x54, "tape data error" },
	{ 0x55, "no data on the tape" },
	{ 0x56, "tape data miscompare" },
	{ 0x57, "miscellaneous tape error" },
};
#define	NERRS	(sizeof(dsderrs) / sizeof(struct dkerror))

/* command codes */
struct dkerror dsdcmds[] = {
	{ 0x00, "initialize" },
	{ 0x01, "transfer status" },
	{ 0x02, "format" },
	{ 0x03, "read sector ID" },
	{ 0x04, "read data" },
	{ 0x05, "read to buffer with verify" },
	{ 0x06, "write data" },
	{ 0x07, "write buffer data" },
	{ 0x08, "seek" },
	{ 0x0e, "buffer I/O" },
	{ 0x0f, "diagnostic command" },
	{ 0x10, "tape initialize" },
	{ 0x11, "tape rewind" },
	{ 0x12, "tape forward filemark" },
	{ 0x14, "tape write file mark" },
	{ 0x17, "erase tape" },
	{ 0x1a, "tape forward record" },
	{ 0x1c, "tape reset" },
	{ 0x1d, "tape retension" },
	{ 0x1e, "read tape status" },
	{ 0x1f, "tape read/write terminate" },
};
#define	NCMDS	(sizeof(dsdcmds) / sizeof(struct dkerror))

int	dsdprobe(), dsdattach(), dsdstart(), dsdintr(), mdstrategy();
struct	mb_device *mddinfo[NMD];
struct	mb_ctlr *mdcinfo[NDSD];
struct	mb_driver dsddriver = {
	dsdprobe, dsdattach, (int (*)())0, dsdstart, dsdintr,
	(char *(*)())0, "md", mddinfo, "dsd", mdcinfo,
};

int	qicprobe();
struct	mb_device *qic_dinfo[1];
struct	mb_ctlr *qic_cinfo[1];
struct	mb_driver qicdriver = {
	qicprobe, (int (*)())0, (int (*)())0, dsdstart, dsdintr,
	(char *(*)())0, "qic", qic_dinfo, "qic", qic_cinfo,
};

long	ccb_mbva;	/* multibus address of ccb	*/
long	cib_mbva;	/* multibus address of cib	*/
long	iopb_mbva;	/* multibus address of iopb	*/
long	inist_mbva;	/* multibus address of inist	*/

/*
 * qicprobe:
 *	- probe the controller, and see if its there
 *	- initialize it, to get things going
 *	- stash port address from config
 *	- remember that the port address has to be byte swapped
 *	- try to find the tape drive
 */
qicprobe(reg)
	long reg;
{
	register struct softc_tape *st;

	mdsoftc.sc_ioaddr = (caddr_t) ((MBIO_VBASE + reg) ^ 1);
	/*
	** mdsoftc.sc_wub == multibus addr of wub
	*/
	mdsoftc.sc_wub = (struct wub *) (reg << 4);
	if ((mdsoftc.sc_wub < (wub_t *)WUB_MBADDR) ||
	    (mdsoftc.sc_wub >= (wub_t *)WUB_MBLIMIT))
		panic("dsd bad config");

	dsdtouch();			/* do a simple probe first	*/
	dsdinit();
	mdsoftc.sc_flags = SC_ALIVE;

	/* these always work, tape drive installed or not */
	(void) dsdconfig(D_217, F_INIT, 0, 1, 0, 0, 0, 0);
	(void) dsdconfig(D_217, F_TINIT, 0, 1, 0, 0, 0, 0);

	/* this fails, and hangs the controller, if no tape drive */
	if (dsdconfig(D_217, F_TRESET, 0, 1, 0, 0, 0, 0)) {
		/*
		 * Controller will hang if there is no tape drive connected.
		 * Reset the bugger so that the disk probes which happen
		 * next will work.
		 */
		dsdinit();
		return (CONF_FAULTED);
	}

	st = &mdsoftc.sc_tape[0];
	st->sc_flags = SC_ALIVE;
	st->sc_unit = 0;
	return (CONF_ALIVE);
}

/*
 * dsdprobe:
 *	- this is a stub, which is used to see if the "tape"
 *	  controller probed.  Since the tape and disk controller are
 *	  the same beastie, we just return the tape controllers probing
 *	  status
 */
/*ARGSUSED*/
dsdprobe(reg)
	long reg;
{
	if (mdsoftc.sc_flags & SC_ALIVE)
		return (CONF_ALIVE);
	return (CONF_FAULTED);
}

/*
 * dsdattach:
 *	- attach a disk to its controller
 *	- configure to 1 cyl, 1 head and 64 sectors
 *	- read the label
 *	- reconfigure to cyl/hd/sec in label
 *	- stash file system info into static structure
 */
dsdattach(mi)
	struct mb_device *mi;
{
	register struct disk_label *l;
	register struct softc_disk *sd;
	register iopb_t *iop = &mdsoftc.sc_iopb;
	register struct buf *bp;
	register short unit = mi->mi_unit;
	int result = CONF_ALIVE;
	register int x;

    /* configure to read label */
	bp = getdmablk(1);		/* grab a block to read label into */
	if (dsdconfig(D_WIN, F_INIT, unit, 1, 0, 1, 0, 55))
		goto error;

    /* read disk label from block 0 and verify magic # */
	iop->p_dev = D_WIN;		/* set up to read a block */
	iop->p_func = F_READ;
	iop->p_unit = unit;
	iop->p_cyl = 0;			/* cyl/hd/sec=0/0/0 */
	iop->p_sec = 0;
	iop->p_hd = 0;
	iop->p_dba = (u_char *) swapw( (long) bp->b_iobase );
	iop->p_rbc = swapw((long) BLK_SIZE);	/* request one block */
	x = dsdcmd(M_NOINT);		/* read the label */
	if (x)
		goto error;
	l = (struct disk_label *)bp->b_un.b_addr;
	if (l->d_magic != D_MAGIC) {
		dk_prname((struct disk_label *)NULL);
		goto error;
	}

    /* reconfigure from label */
	if (dsdconfig(D_WIN, F_INIT, unit, l->d_cylinders, 0, l->d_heads,
			     0, l->d_sectors))
		goto error;

    /* save label info and init drive software state */
	sd = &mdsoftc.sc_disk[unit];
	sd->sc_flags = SC_ALIVE;
	sd->sc_cyl = l->d_cylinders;
	sd->sc_hd  = l->d_heads;
	sd->sc_sec = l->d_sectors;
	sd->sc_spc = l->d_heads * l->d_sectors;
	sd->sc_blksize = BLK_SIZE;
	sd->sc_blkshift = BLK_SHIFT;
	sd->sc_unit = unit;
	bcopy((caddr_t)l->d_map, (caddr_t)sd->sc_fs, sizeof(sd->sc_fs));
	setroot(&dsddriver, mi, unit << 3, 0070, l, mdstrategy);
	dk_prname(l);
	goto out;

error:
	result = CONF_FAULTED;
out:
	brelse(bp);
	return result;
}

/*
 * mdopen() -- Disk open routine, simply verify drive attached
 */
/*ARGSUSED*/
mdopen(dev, flag)
	dev_t dev;
	int flag;
{
	register short unit = D_DRIVE(dev);

	if ((unit > WIN_DRIVES) ||
	    !(mdsoftc.sc_disk[unit].sc_flags & SC_ALIVE)) {
		u.u_error = ENODEV;
		return;
	}
}

/*
 * mfopen() -- Floppy open routine, verify drive attached
 *	and configure it.
 * MOST LIKELY, floppies with 256 byte sectors won't work.
 */
/*ARGSUSED*/
mfopen(dev, flag)
	dev_t dev;
	int flag;
{
	register short unit = D_DRIVE(dev);
	register struct softc_disk *sd = &mdsoftc.sc_floppy[unit];
	register short type;
	register u_char cyls, dens, sides, spt;

	if (unit > FLP_DRIVES) {
		u.u_error = ENODEV;
		return;
	}

	type = FLP_TYPE(dev);
	sides = (type & FLP_SGLSIDE) ? 1 : 2;
	dens = (type & FLP_SGLDENS) ? 0 : 1;
	spt = (dens==0) ? 8 : 16;
	cyls = 80;
	if (type & FLP_SEC256) {
		sd->sc_blksize = 256;
		sd->sc_blkshift = 8;
	} else {
		sd->sc_blksize = 512;
		sd->sc_blkshift = 9;
		spt >>= 1;			/* 1/2 as many 512byte secs */
	}

    /*
     * Attempt to configure drive again if it didn't probe.  This is done
     * in case the drive was powered off during boot.
     */
	if (!(sd->sc_flags & SC_ALIVE) || (sd->sc_flags & SC_HARDERROR)) {
		DSDLOCK(AC_DISK, unit);
		if (dsdconfig(D_FLP, F_INIT, unit, cyls, dens, 0, sides, spt))
			u.u_error = EIO;
		DSDUNLOCK();
		if (u.u_error)
			return;
	}

    /* now that floppy is alive, init state */
	sd->sc_flags = SC_ALIVE;
	sd->sc_spc = spt*sides;
	sd->sc_cyl = cyls;
	sd->sc_sec = spt;
	sd->sc_hd  = sides;
	sd->sc_fs[0].d_size = cyls * sd->sc_spc;
}

/*
** qicopen() -- QIC open routine, make sure drive exists!!!
*/
qicopen(dev, flag)
	dev_t dev;
	int flag;
{
	register struct softc_tape *st;
	register short unit = TAPEDRIVE(dev);
	register u_char status;
	register device = TAPEMINOR(dev);

    /* validate open request (valid unit && probed) */
	st = &mdsoftc.sc_tape[unit];
	if ((unit > QIC_DRIVES) || !(st->sc_flags & SC_ALIVE)) {
		u.u_error = ENODEV;
		return;
	}
	if (st->sc_flags & SC_TAPEINUSE) {
		u.u_error = EBUSY;
		return;
	}

    /* try to reset drive, if it previously choked */
	DSDLOCK(AC_TAPE, unit);
	mdsoftc.sc_tab.b_actf = mdsoftc.sc_tab.b_actl = NULL;
	if (st->sc_flags & SC_HARDERROR) {	/* drive died during last use */
		st->sc_flags &= ~SC_HARDERROR;
		(void) dsdconfig(D_217, F_INIT, unit, 1, 0, 0, 0, 0);
		(void) dsdconfig(D_217, F_TINIT, unit, 1, 0, 0, 0, 0);

		/* this fails, and hangs the controller, if no tape drive */
		if (dsdconfig(D_217, F_TRESET, unit, 1, 0, 0, 0, 0))
			panic("dsd0: tape reset failed");
/* eventually, put controller reset here */
#ifdef LASTACCESS
		if (device & TAPE_NOREWIND && st->sc_lastaccess)
			goto skip;
#endif LASTACCESS
		if (qicstatus(st)) {
			uprintf("qic%d: can't get status during init\n", unit);
			st->sc_flags |= SC_HARDERROR;
			goto out;
		}
	}
#ifdef LASTACCESS
	if (device & TAPE_NOREWIND && st->sc_lastaccess)
		goto skip;
#endif LASTACCESS
	if (qicstatus(st)) {
		st->sc_flags |= SC_HARDERROR;
		goto error;
	}
	status = mdsoftc.sc_inist.is_sb[HARD_BYTE1];
	if (status & NOTAPE) {
		uprintf("qic%d: no cartridge in drive\n", unit);
		goto error;
	}
	if (status & NOTRDY) {
		uprintf("qic%d: unit not ready\n", unit);
		goto error;
	}
	if ((flag & FWRITE) && (status & WRPROT)) {
		uprintf("qic%d: write protected\n", unit);
		goto error;
	}
	if ((!mdsoftc.sc_inist.is_sb[BOT]) && !(device & TAPE_NOREWIND) &&
	    qicrewind(st))
		goto error;
#ifdef LASTACCESS
skip:
#endif LASTACCESS
	st->sc_blkno = 0;
	st->sc_nxrec = (daddr_t)INFINITY;
	st->sc_flags |= SC_TAPEINUSE;
	goto out;

error:
	u.u_error = EIO;

out:
	DSDUNLOCK();
}

/*
** qicclose() -- end processing (tm) & rewind
*/
qicclose(dev)
	dev_t dev;
{
	register struct softc_tape *st;
	register short unit = TAPEDRIVE(dev);

	st = &mdsoftc.sc_tape[unit];
	if (st->sc_flags & SC_HARDERROR) {
		st->sc_flags &= (SC_ALIVE|SC_HARDERROR);
		st->sc_fileno = 0;
		return;
	}

    /* write closing file mark, and rewind */
	DSDLOCK(AC_TAPE, unit);
	mdsoftc.sc_tab.b_actf = mdsoftc.sc_tab.b_actl = NULL;
	if (st->sc_flags & SC_WRITTEN) {
		if (qicshortcmd(st, F_TFLMK, "write file mark", 1))
			goto out;
	}

	if (st->sc_flags & SC_READ || st->sc_flags & SC_WRITTEN)
		st->sc_fileno += 1;

    /* attempt to rewind tape, if its requested */
	if (!(TAPEMINOR(dev) & TAPE_NOREWIND)) {
#ifdef LASTACCESS
		st->sc_lastaccess = 0;
#endif LASTACCESS
		if (qicrewind(st))
			goto out;
	}
#ifdef LASTACCESS
	else {
		st->sc_lastaccess = 1;
	}
#endif LASTACCESS
	st->sc_flags &= (SC_ALIVE | SC_HARDERROR);

out:
	DSDUNLOCK();
}

/*
** dsdstrategy() -- queue the specified buffer header on the work
**		queue.  Perform a few validity checks.
*/
dsdstrategy(bp, sd)
	register struct buf *bp;
	register struct softc_disk *sd;
{
	register struct disk_map *fs;
	daddr_t bn;
	short temp;

	if (!(sd->sc_flags & SC_ALIVE) || (sd->sc_flags & SC_HARDERROR)) {
		berror(bp);
		return;
	}

	if (bp->b_type == D_FLP)
		fs = &sd->sc_fs[0];
	else
		fs = &sd->sc_fs[(short) D_FS(bp->b_dev)];
	if (dk_rangecheck(bp, fs, sd->sc_blksize, sd->sc_blkshift))
		return;

	/* crunch block address into disk address for start and disksort */
	bn = bp->b_blkno + fs->d_base;
	bp->b_cyl = bn / sd->sc_spc;
	temp = bn % sd->sc_spc;
	bp->b_head = temp / sd->sc_sec;
	bp->b_sector = temp % sd->sc_sec;

	/* put request on queue and fire up a command if nothings going */
	SPL();
	DSDGET();			/* get controller from tape code */
	disksort(&mdsoftc.sc_tab, bp);
	if (mdsoftc.sc_tab.b_active == 0)
		dsdstart();
	spl0();
}

/*
 * mdstrategy() -- disk interface to dsdstrategy
 */
mdstrategy(bp)
	register struct buf *bp;
{
	register short unit;

	unit = D_DRIVE(bp->b_dev);
	bp->b_type = D_WIN;			/* tag the request */
	dsdstrategy(bp, &mdsoftc.sc_disk[unit]);
}

/*
 * mfstrategy() -- floppy interface to dsdstrategy
 */
mfstrategy(bp)
	register struct buf *bp;
{
	register short unit;

	unit = D_DRIVE(bp->b_dev);
	bp->b_type = D_FLP;			/* tag the request */
	dsdstrategy(bp, &mdsoftc.sc_floppy[unit]);
}

/*
** qicstrategy() -- decode and start a tape request
*/
qicstrategy(bp)
	register struct buf *bp;
{
	register short unit = TAPEDRIVE(bp->b_dev);
	register struct softc_tape *st = &mdsoftc.sc_tape[unit];
	register iopb_t *iop = &mdsoftc.sc_iopb;

	if (bp->b_bcount & (BLK_SIZE-1)) {
		berror(bp);
		return;
	}

/*	bp->b_type = D_QIC;			/* tag the request */
	bp->av_forw = 0;
	if ((bp->b_flags & B_READ) == 0)
		st->sc_flags |= SC_WRITTEN;
	else
		st->sc_flags |= SC_READ;
	st->sc_blkno = bp->b_blkno;

	DSDLOCK(AC_TAPE, unit);			/* unlock is done in dsdintr */
	mdsoftc.sc_tab.b_actf = mdsoftc.sc_tab.b_actl = bp;
	(void) SPL();
	iop->p_dev = D_217;
	iop->p_func = (bp->b_flags & B_READ)? F_READ : F_WRITE;
	iop->p_unit = unit;
	iop->p_dba = (u_char *) swapw( (long) bp->b_iobase );
	iop->p_rbc = swapw((long) bp->b_bcount);
	iop->p_atc = 0;

    /* start command up */
#ifdef TAPEDEBUG
	iprintf("(C> %s: c: %d)\n",
		(iop->p_func == F_READ)?"READ":"WRITE",
		swapw((long) iop->p_rbc));
#endif
	if (dsdcmd(0)) {
		printf("qic%d: couldn't start!\n", unit);
		panic("dsd0");
	}
	(void) spl0();
}

/*
 * dsdstart:
 *	- start the top request on the queue, if we aren't already
 *	  doing something
 */
dsdstart()
{
	register struct buf *bp;
	register struct softc_disk *sd;
	register iopb_t *iop = &mdsoftc.sc_iopb;
	register daddr_t bn;
	register short floppy;
	register short unit;

	if ((bp = mdsoftc.sc_tab.b_actf) == NULL)
		return;

    /* use b_type to decode device type */
	unit = D_DRIVE(bp->b_dev);
	if (bp->b_type == D_WIN) {
		floppy = 0;
		sd = &mdsoftc.sc_disk[unit];
	} else {
		floppy = 1;
		sd = &mdsoftc.sc_floppy[unit];
	}

	bn = bp->b_blkno + sd->sc_fs[D_FS(bp->b_dev)].d_base;
	mdsoftc.sc_blkno = bn;			/* save for errors */

	iop->p_mod = 0;
	iop->p_dev = floppy ? D_FLP : D_WIN;
	iop->p_func = (bp->b_flags & B_READ) ? F_READ : F_WRITE;
	mdsoftc.sc_tab.io_s1 = iop->p_unit = unit;
	iop->p_cyl = bp->b_cyl;
	iop->p_sec = bp->b_sector;
	if (floppy)
		iop->p_sec++;			/* quirk in numbering */
	iop->p_hd = bp->b_head;
	iop->p_dba = (u_char *) swapw( (long) bp->b_iobase );
	iop->p_rbc = swapw((long) bp->b_bcount);

    /* start command */
	mdsoftc.sc_tab.b_active = AC_DISK;
	floppy = 200000;			/* use as a timeout counter */
	while (mdsoftc.sc_ccb.c_busy && --floppy)
		;
	if (floppy == 0) {
		printf("m%c%d%c: couldn't start!\n", floppy ? 'f' : 'd',
				 unit, 'a' + D_FS(bp->b_dev));
		panic("dsd0");
	}
	START();			/* Start the controller going */

    /* instrument the transfer while controller is processing command */
	if (bp->b_type == D_WIN) {
		bn = mddinfo[unit]->mi_dk;	/* dk unit # */
		dk_busy |= 1<<bn;
		dk_xfer[bn]++;
		dk_wds[bn] += bp->b_bcount >> 6;
	}
}

mdread(dev)
	dev_t dev;
{
	if (physck(mdsoftc.sc_disk[D_DRIVE(dev)].sc_fs[D_FS(dev)].d_size,
					  B_READ, BLK_SHIFT))
		physio(mdstrategy, (struct buf *)NULL, dev, B_READ, minphys);
}

mfread(dev)
	dev_t dev;
{
	register struct softc_disk *sd;

	sd = &mdsoftc.sc_floppy[D_DRIVE(dev)];
	if (physck(sd->sc_fs[D_FS(dev)].d_size, B_READ, sd->sc_blkshift))
		physio(mfstrategy, (struct buf *)NULL, dev, B_READ, minphys);
}

qicread(dev)
	dev_t dev;
{
	if (u.u_count & (BLK_SIZE - 1))
		u.u_error = EIO;
	else
		physio(qicstrategy, (struct buf *)NULL, dev,
				    B_READ | B_TAPE, minphys);
}

mdwrite(dev)
	dev_t dev;
{
	if (physck(mdsoftc.sc_disk[D_DRIVE(dev)].sc_fs[D_FS(dev)].d_size,
					  B_WRITE, BLK_SHIFT))
		physio(mdstrategy, (struct buf *)NULL, dev, B_WRITE, minphys);
}

mfwrite(dev)
	dev_t dev;
{
	register struct softc_disk *sd;

	sd = &mdsoftc.sc_floppy[D_DRIVE(dev)];
	if (physck(sd->sc_fs[D_FS(dev)].d_size, B_WRITE, sd->sc_blkshift))
		physio(mfstrategy, (struct buf *)NULL, dev, B_WRITE, minphys);
}

qicwrite(dev)
	dev_t dev;
{
	if (u.u_count & (BLK_SIZE - 1))
		u.u_error = EIO;
	else
		physio(qicstrategy, (struct buf *)NULL, dev,
				    B_WRITE | B_TAPE, minphys);
}

mdprint(dev, str)
	dev_t dev;
	char *str;
{
	uprintf("%s on md%d%c\n", str, D_DRIVE(dev), 'a'+D_FS(dev));
}

mfprint(dev, str)
	dev_t dev;
	char *str;
{
	uprintf("%s on mf%d\n", str, D_DRIVE(dev));
}

/*
 * dsdintr:
 *	- process interrupts
 */
dsdintr()
{
	register struct buf *bp;
	register short unit, status, func;

	/* see if this interrupt is for me */
	if (mdsoftc.sc_cib.i_stsem == 0)
		return (0);			/* not for me */

	unit = mdsoftc.sc_tab.io_s1;
	status = mdsoftc.sc_cib.i_opstat;
	mdsoftc.sc_cib.i_stsem = 0;
	bp = mdsoftc.sc_tab.b_actf;
	func = mdsoftc.sc_iopb.p_func;
	CLEAR();
#ifdef	PARANOID
	if (((status & 0x30) >> 4) != unit)
		iprintf("dsdintr: bogus unit, hw=%d sw=%d\n",
				      (status & 0x30) >> 4, unit);
#endif
	if (status == 0)
		printf("dsd0: zero status on cmd=%x\n", func);

    /* dispatch to sub-device interrupt routines */
	if (mdsoftc.sc_tab.b_active == AC_TAPE) {
		register struct softc_tape *st;

		st = &mdsoftc.sc_tape[unit];
		st->sc_status = status;			/* save for later */
#ifdef TAPEDEBUG	
	iprintf("(I> c: %s s: %x fg: %x atc: %d)\n",
	     dkerror(func, dsdcmds, NCMDS),
	     status, st->sc_flags, swapw((long) mdsoftc.sc_iopb.p_atc));
#endif TAPEDEBUG	

	    /* wakeup anybody waiting on command completion */
		if (st->sc_flags & SC_WANTED)
			wakeup((caddr_t)st);
		st->sc_flags &= ~SC_WANTED;

	    /* take care of any errors */
		if (status & O_HARD) {
			/*
			** See if we recieved an error during a rewind
			** command.  If this happens, then the tape is
			** quiescent, and thus we can try the rewind
			** again.  The reason this seems to happen
			** is when the tape is reading, the next command
			** after a read (if the tape has not read a file
			** mark) must be another read or a read/write
			** terminate.  If the tape has read a file mark
			** then any old command is kosher.  The controller,
			** as usual, sucks face.
			*/
			if (func == F_TREW) {		/* rewind fuckup */
				st->sc_flags &= ~(SC_SHORTCMD|SC_LONGCMD);
				return (1);
			}

			st->sc_flags &= ~(SC_LONGCMD|SC_SHORTCMD);
			st->sc_flags |= SC_HARDERROR;	/* bad bad bad */
			/*
			** The dsd controller will not allow any status
			** reading during the tape stuff while rewind
			** is in progress and I guarantee that it is right
			** at this time.  Note that the drive rewinds itself
			** under just about any error condition
			*/
			if (qicstatus(st))
				printf("qic%d: unable to read status\n", unit);
			printf("qic%d: hard error during ``%s'' stat=%x\n",
				       unit, dkerror(func, dsdcmds, NCMDS),
				       status);
			if (bp)
				berror(bp);
			mdsoftc.sc_tab.b_actf = mdsoftc.sc_tab.b_actl = NULL;
			DSDUNLOCK();
			return (1);
		}

	    /* check command completion */
		if (st->sc_flags & SC_SHORTCMD) {	/* short */
#ifdef TAPEDEBUG	
			iprintf(" short ");
			if (!(!status || ((status&0xf) == O_FLQICDN)))
	    		iprintf("qic%d: huh? func=%x status=%x flags=%x\n",
			    	func, status, st->sc_flags);
#endif TAPEDEBUG	
			st->sc_flags &= ~SC_SHORTCMD;
		} else
		if (st->sc_flags & SC_LONGCMD) {	/* long */
#ifdef TAPEDEBUG	
			iprintf(" long ");
			if (!((status&0xf) == O_TPLONG))
	    		iprintf("qic%d: huh? func=%x status=%x flags=%x\n",
			    	func, status, st->sc_flags);
#endif TAPEDEBUG	
			st->sc_flags &= ~SC_LONGCMD;
		} else {				/* read/write */
			mdsoftc.sc_tab.b_actf = mdsoftc.sc_tab.b_actl = NULL;
			DSDUNLOCK();
			goto its_done;
		}
	} else {
#ifdef	PARANOID
		if (bp == NULL || mdsoftc.sc_tab.b_active == 0) {
			printf("dsd0: interrupt with empty queue: func=%x\n",
				      func);
			return (1);
		}
#endif
		if (bp->b_type == D_WIN)
			dk_busy &= ~(1<<mddinfo[unit]->mi_dk);

		/* process a hard error (with a possible retry) */
		if ((status & O_HARD) && mderror(bp))
			return (1);

		/* start up next command */
		mdsoftc.sc_tab.b_actf = bp->av_forw;
		mdsoftc.sc_tab.b_errcnt = 0;
		mdsoftc.sc_tab.b_active = 0;
		if (mdsoftc.sc_tab.b_actf)
			dsdstart();
		else
			DSDUNLOCK();
its_done:
		/* indicate completion on previous command */
		iodone(bp);
	}
	return (1);
}

/*
 * mderror:
 *	- process an error from the controller (for disk or floppy)
 *	- if we get a SECTOR_NOT_FOUND error, then delay a ``random''
 *	  amount of time to cause the controller to exhibit different
 *	  behaviour.  This is due to a botch that causes the controller
 *	  to not issue retries when an error occurs which is more
 *	  than four bit lengths long and resides in sector 2 of the track
 *	  And you believe we found this??????
 */
mderror(bp)
	register struct buf *bp;
{
	register iopb_t *iop = &mdsoftc.sc_iopb;
	register u_char *sb = mdsoftc.sc_inist.is_sb;
	u_short cyl, hd, sec, func;
	u_long rbc, atc;
	short result, unit;
	short floppy = 0;

    /* figure out what to do for the disk cases */
	unit = D_DRIVE(bp->b_dev);
	func = iop->p_func;		/* save these */
	cyl = iop->p_cyl;
	hd = iop->p_hd;
	sec = iop->p_sec;
	rbc = swapw((long) iop->p_rbc);
	atc = swapw((long) iop->p_atc);

	dsdstatus();			/* fetch error status */
	result = sb[SB_EXS];

    /* floppy unit */
	if (bp->b_type == D_FLP) {
		floppy++;
		if(result == 0x28 && ((bp->b_flags & B_READ) == 0)) {
			printf("mf%d: write protected\n", unit);
			mdsoftc.sc_floppy[unit].sc_flags |= SC_HARDERROR;
			goto error;
		}
		if(result == 0x25 || result == 0x38) {
			printf("mf%d: is diskette formatted?\n", unit);
			mdsoftc.sc_floppy[unit].sc_flags |= SC_HARDERROR;
			goto printit;
		}
	} else
    /* winchester disk */
	if (++mdsoftc.sc_tab.b_errcnt <= NRETRY) {
		if (result == SECTOR_NOT_FOUND)		/* See NOTE at top */
			for (cyl = 1000*mdsoftc.sc_tab.b_errcnt; --cyl; )
				;
		/*
		** when you can do something intelligent about
		** soft errors, then some sort message should probably
		** be printed
		*/
		dsdstart();
		return 1;
	}

printit:
	printf("m%c%d%c: hard error, cmd='%s', error='%s', ",
			 floppy? 'f' : 'd',
			 unit, D_FS(bp->b_dev) + 'a',
	     		 dkerror(func, dsdcmds, NCMDS),
			 dkerror(result, dsderrs, NERRS));
	printf("bn=%d at %d/%d/%d req%d atc%d\n",
		      mdsoftc.sc_blkno, cyl, hd, sec, rbc, atc);
	if (!floppy) {
		if ((sec + (rbc/512)) > 17)
			printf("Error could have crossed track boundary\n");
	}

error:
	bp->b_flags |= B_ERROR;
	return 0;
}

/*
 * dsdcmd() -- issue the command previously set into the iopb.
 *		if WAIT, then disable done interrupt, and wait
 *		for completion, otherwise, enable the interrupt and
 *		return immediately.
 */
dsdcmd(modifier)
	short modifier;
{
	register iopb_t *iop = &mdsoftc.sc_iopb;
	register cib_t *cib;
	register ccb_t *ccb = &mdsoftc.sc_ccb;
	register u_char ops;
	register long timo = 2000000;

	iop->p_mod = modifier;
	while (ccb->c_busy && --timo)
		;
	if (timo == 0) {
		printf("dsd0: ccb timeout, cmd='%s' dev=%d unit=%d\n",
			      dkerror(iop->p_func, dsdcmds, NCMDS),
			      iop->p_dev, mdsoftc.sc_tab.io_s1);
		return 1;
	}
#ifdef TAPEDEBUG	
	if (iop->p_dev == D_QIC) {
		iprintf("(CMD: %s: c: %d)",
	     		dkerror(mdsoftc.sc_iopb.p_func, dsdcmds, NCMDS),
			swapw((long) mdsoftc.sc_iopb.p_rbc));
	}
#endif TAPEDEBUG	
	START();			/* start the ctlr going */
	if ((modifier & M_NOINT) == 0)
		return 0;

/*
 * XXX
 * We parameterize the number of passes of the cib timeout loop below
 * for each processor, and each type of timeout
 * XXX
 */
#ifdef	juniper
#define	PASSES_PER_SECOND	750000
#else
#define	PASSES_PER_SECOND	75000
#endif
	/*
	 * If there is no drive attached, then a F_TRESET will hang
	 * forever.  Set delay during probe to be short for this
	 * case.
	 *
	 * If there is a drive, then F_TRESET will be quick, but
	 * a tape status following the F_TRESET may take a long
	 * time (maximum rewind time for a 600ft tape is approx 90sec)
	 */
	timo = PASSES_PER_SECOND * 90;		/* 600ft tape, full rewind */
	if ((iop->p_func == F_TRESET) && ac.a_probing)
		timo = PASSES_PER_SECOND * 2;	/* no rewind, just reset */

	cib = &mdsoftc.sc_cib;
	while ((cib->i_stsem == 0) && --timo)
		;
	if (timo == 0) {
		if (!ac.a_probing)
			printf("dsd0: cib timeout, cmd=``%s'' dev=%d unit=%d\n",
				      dkerror(iop->p_func, dsdcmds, NCMDS),
				      iop->p_dev, mdsoftc.sc_tab.io_s1);
		return 1;
	}

	/*
	** Clear the status semaphore *** ONLY *** in the NON INTERRUPT mode.
	*/
	cib->i_stsem = 0;
	CLEAR();			/* Clear possible interrupt (SEEK) */
	ops = cib->i_opstat;		/* use operation status here */
	if (ops & O_HARD) {
		if (!ac.a_probing) {
			printf("dsd0: hard error, cmd=``%s'' dev=%d unit=%d\n",
				      dkerror(iop->p_func, dsdcmds, NCMDS),
				      iop->p_dev, mdsoftc.sc_tab.io_s1);
		}
		return 1;
	}
	return 0;
}

/*
 * dsdtouch()
 *	- simply probe the controller.  This is to test if 
 *	  board is there before allocating mbmap resources.
 *	- If the board is there allocate Multibus space for ccb, cib, iopb
 */
dsdtouch()
{
	CLEAR();

	/* if we are here the board probed. */

	/* map in ccb, cib, and iopb	*/
	ccb_mbva = mbmapkget((caddr_t)&mdsoftc.sc_ccb,
			((long)&mdsoftc.sc_ioaddr - (long)&mdsoftc.sc_ccb) );
	cib_mbva  = ccb_mbva + ((long)&mdsoftc.sc_cib  - (long)&mdsoftc.sc_ccb);
	iopb_mbva = ccb_mbva + ((long)&mdsoftc.sc_iopb - (long)&mdsoftc.sc_ccb);
	inist_mbva = ccb_mbva +
			      ((long)&mdsoftc.sc_inist - (long)&mdsoftc.sc_ccb);

}

/*
** dsdinit() -- initialize the controller, no drive configuration
**	      implied.
*/
dsdinit()
{
	register ccb_t *ccb = &mdsoftc.sc_ccb;
	register cib_t *cib = &mdsoftc.sc_cib;
	register wub_t *wub;
	register long timo;

    /* setup the control blocks */
	wub = (wub_t *)(WUB_VBASE + ((long)mdsoftc.sc_wub - WUB_MBADDR));
	wub->w_xx	= 0;		/* Set up WUB */
	wub->w_ext	= W_EXT;
	wub->w_ccb	= (u_char *) swapw( (long) ccb_mbva );

	ccb->c_busy	= 0xFF;		/* Set up CCB */
	ccb->c_ccw1	= 1;
	ccb->c_cib 	= (u_char *) swapw( (long) (cib_mbva +
					    ((long)&cib->i_zero - (long)cib )) );
	ccb->c_xx	= 0;
	ccb->c_busy2	= 0;
	ccb->c_ccw2	= 1;
	ccb->c_cp 	= (u_char *) swapw( (long) (ccb_mbva +
					 ((long)&ccb->c_ctrlptr - (long)ccb )) );
	ccb->c_ctrlptr	= 0x0004;
	
	cib->i_opstat	= 0;		/* Set up CIB */
	cib->i_xx	= 0;
	cib->i_stsem	= 0;
	cib->i_cmdsem	= 0;
	cib->i_zero	= 0;		/* CCB points Here */
	cib->i_iopb	= (u_char *) swapw( (long) iopb_mbva );
	cib->i_xx2	= 0;


	RESET();
	CLEAR();
	START();	/* Perform reset/initialize */

	timo = 10000000;
	while (ccb->c_busy && --timo)
		;
	if (timo == 0) {
		printf("dsd0: ccb timeout during init\n");
		panic("dsd0");
	}
}

/*
** dsdstatus() -- get status from controller after an error and print it.
*/
dsdstatus()
{
	register u_char *sb = mdsoftc.sc_inist.is_sb;
	static short recursing;

	/* We really don't need a panic here */
	if (recursing)
		panic("dsdstatus");
	bzero((caddr_t)sb, sizeof(inist_t));
	mdsoftc.sc_iopb.p_func = F_TSTAT;
	mdsoftc.sc_iopb.p_dba = (u_char *) swapw( (long) (inist_mbva +
		((long)mdsoftc.sc_inist.is_sb - (long)&mdsoftc.sc_inist)) );
	recursing = 1;
	(void) dsdcmd(M_NOINT);
	recursing = 0;
}

/*
** dsdconfig() - configure the controller to handle the specified
**		type of drive on the specified unit
**	NOTE: For floppy, acyl is single/double density (0/1)
*/
dsdconfig(type, cmd, unit, cyl, acyl, fhd, rhd, spt)
	int type, cmd, unit, cyl, acyl, fhd, rhd, spt;
{
	register iopb_t *iop = &mdsoftc.sc_iopb;
	register struct inib *iip = &mdsoftc.sc_inist.is_inib;

	iop->p_xx	= 0;
	iop->p_atc	= 0;
	iop->p_dev	= type;
	iop->p_func	= cmd;
	iop->p_unit	= unit;
	iop->p_cyl	= 0;
	iop->p_sec	= 0;
	iop->p_hd	= 0;
	iop->p_dba	= (u_char *) swapw( (long) (inist_mbva +
		((long)&mdsoftc.sc_inist.is_inib - (long)&mdsoftc.sc_inist)) );
	iop->p_rbc	= 0;
	iop->p_gap	= 0;

	iip->i_ncyl	= cyl;
	iip->i_fhd	= fhd;
	iip->i_rhd	= rhd;
	iip->i_bpsl	= (BLK_SIZE & 0xFF);
	iip->i_spt	= spt;
	iip->i_nacyl	= acyl;
	iip->i_bpsh	= BLK_SIZE >> 8;

	return dsdcmd(M_NOINT);
}

/*
** Tape unit ioctl's
** struct mtop and mtget
*/
/*ARGSUSED*/
qicioctl(dev, cmd, addr, flag)
	caddr_t addr;
	dev_t dev;
{
	register short unit = TAPEDRIVE(dev);
	register struct softc_tape *st = &mdsoftc.sc_tape[unit];
	register u_char *sb = mdsoftc.sc_inist.is_sb;
	struct mtop mtop;
	struct mtget mtget;

	DSDLOCK(AC_TAPE, unit);
	mdsoftc.sc_tab.b_actf = mdsoftc.sc_tab.b_actl = NULL;
	switch(cmd) {
	  case MTIOCTOP:
		if(copyin((caddr_t)addr, (caddr_t)&mtop, sizeof(mtop))) {
			u.u_error = EFAULT;
			break;
		}
		switch(mtop.mt_op) {
		  case MTREW:
			u.u_error = qicrewind(st);
			break;
		  case MTRET:
			u.u_error = qicreten(st);
			break;
		  case MTERASE:
			u.u_error = qicerase(st);
			break;
		  case MTWEOF:
			if (qicshortcmd(st, F_TFLMK, "write file mark",
					    (int)mtop.mt_count))
				u.u_error = EIO;
			else
				st->sc_fileno += mtop.mt_count;
			break;
		  case MTFSF:
			u.u_error = qicfsf(st, (int)mtop.mt_count);
			break;
		  case MTFSR:
			u.u_error = qicshortcmd(st, F_TSPREC,
						    "forward space file",
						    (int)mtop.mt_count);
			break;
		  default:
			u.u_error = EINVAL;
			break;
		}
		break;
	  case MTIOCGET:
		if(copyin((caddr_t)addr, (caddr_t)&mtop, sizeof(mtop))) {
			u.u_error = EFAULT;
			break;
		}
		switch(mtop.mt_op) {
		  case MTBLKSIZE:
		     mtget.mt_blkno = 400;
		     if (copyout((caddr_t)&mtget, addr, sizeof(mtget))) {
		     	u.u_error = EFAULT;
		     	break;
		     }
		     break;
		  case MTNOP:
			if (st->sc_flags & SC_HARDERROR) {
				u.u_error = EIO;
				break;
			}
			if (u.u_error = qicstatus(st))
				break;
			mtget.mt_type = MT_ISDSD;
			mtget.mt_hard_error0 = sb[HARD_BYTE0];
			mtget.mt_hard_error1 = sb[HARD_BYTE1];
			mtget.mt_soft_error0 = sb[SOFT_BYTE0];
			mtget.mt_at_bot = sb[BOT];
/* 			mtget.mt_resid = st->sc_resid; */
			mtget.mt_fileno = st->sc_fileno;
			mtget.mt_blkno = st->sc_blkno;
			if (copyout((caddr_t)&mtget, addr, sizeof(mtget))) {
				u.u_error = EFAULT;
				break;
			}
		     	break;
		  default:
		     	u.u_error = EINVAL;
		     	break;
		}
		break;
	  default:
		u.u_error = EINVAL;
	}
	DSDUNLOCK();
}

/*
** qicrewind() -- try to rewind the specified tape
**	Assumes controller is locked by caller
**	Try to do this about 10 times (more or less) and then give up
*/
qicrewind(st)
	register struct softc_tape *st;
{
	int rewinds;

	rewinds = 0;
	for (;;) {
		mdsoftc.sc_iopb.p_func = F_TREW;
		mdsoftc.sc_iopb.p_dev = D_217;
		mdsoftc.sc_iopb.p_unit = st->sc_unit;
		st->sc_flags |= (SC_SHORTCMD | SC_LONGCMD);
		if (dsdcmd(0)) {
blewit:
			st->sc_flags |= SC_HARDERROR;
			printf("qic%d: rewind failed\n", st->sc_unit);
			return EIO;
		}
		rewinds++;
		st->sc_fileno = 0;
		st->sc_blkno = 0;
		st->sc_nxrec = INFINITY;
		/*
		** For some reason the controller returns this status
		** if the previous command was a read.  Try again, BOZO.
		** See comments in the interrupt routine.
		*/
		if (qicwait(st, SC_SHORTCMD) || (st->sc_status & O_SUMMARY)) {
			if (rewinds > 10)
				goto blewit;
			continue;
		}
		/*
		** See above comment
		*/
		if (qicwait(st, SC_LONGCMD) || (st->sc_status & O_SUMMARY)) {
			if (rewinds > 10)
				goto blewit;
			continue;
		}
		return 0;
	}
}

/*
** qicfsf() -- try to space the tape forward one file
**	Long two interrupt commands on the controller.
**	Assumes controller is locked by caller.
*/
qicfsf(st, count)
	register struct softc_tape *st;
	register int count;
{
	if (st->sc_flags & SC_HARDERROR)
		return EIO;

	mdsoftc.sc_iopb.p_func = F_TSPFILE;
	mdsoftc.sc_iopb.p_dev = D_217;
	mdsoftc.sc_iopb.p_unit = st->sc_unit;
	while (count--) {
		st->sc_flags |= (SC_SHORTCMD | SC_LONGCMD);
		if (dsdcmd(0)) {
			printf("qic%d: forward space file failed", st->sc_unit);
			st->sc_flags &= ~(SC_SHORTCMD | SC_LONGCMD);
			st->sc_flags |= SC_HARDERROR;
			return EIO;
		}
		if (qicwait(st, SC_LONGCMD))
			return 1;
		st->sc_fileno++;
	}
	return 0;
}

/*
** qicreten() -- try to rewind the specified tape
**	Assumes controller is locked by caller
**	Try to do this about 10 times (more or less) and then give up
*/
qicreten(st)
	register struct softc_tape *st;
{
	for (;;) {
		mdsoftc.sc_iopb.p_func = F_TRETEN;
		mdsoftc.sc_iopb.p_dev = D_217;
		mdsoftc.sc_iopb.p_unit = st->sc_unit;
		st->sc_flags |= (SC_SHORTCMD | SC_LONGCMD);
		if (dsdcmd(0)) {
blewit:
			st->sc_flags |= SC_HARDERROR;
			printf("qic%d: retension failed\n", st->sc_unit);
			return EIO;
		}
		st->sc_fileno = 0;
		st->sc_blkno = 0;
		st->sc_nxrec = INFINITY;
		/*
		** For some reason the controller returns this status
		** if the previous command was a read.  Try again, BOZO.
		** See comments in the interrupt routine.
		*/
		if (qicwait(st, SC_SHORTCMD) || (st->sc_status & O_SUMMARY)) {
			goto blewit;
		}
		/*
		** See above comment
		*/
		if (qicwait(st, SC_LONGCMD) || (st->sc_status & O_SUMMARY)) {
			goto blewit;
		}
		return 0;
	}
}

/*
** qicerase() -- try to rewind the specified tape
**	Assumes controller is locked by caller
**	Try to do this about 10 times (more or less) and then give up
*/
qicerase(st)
	register struct softc_tape *st;
{
	for (;;) {
		mdsoftc.sc_iopb.p_func = F_TERASE;
		mdsoftc.sc_iopb.p_dev = D_217;
		mdsoftc.sc_iopb.p_unit = st->sc_unit;
		st->sc_flags |= (SC_SHORTCMD | SC_LONGCMD);
		if (dsdcmd(0)) {
blewit:
			st->sc_flags |= SC_HARDERROR;
			printf("qic%d: erase failed\n", st->sc_unit);
			return EIO;
		}
		st->sc_fileno = 0;
		st->sc_blkno = 0;
		st->sc_nxrec = INFINITY;
		/*
		** For some reason the controller returns this status
		** if the previous command was a read.  Try again, BOZO.
		** See comments in the interrupt routine.
		*/
		if (qicwait(st, SC_SHORTCMD) || (st->sc_status & O_SUMMARY)) {
			goto blewit;
		}
		/*
		** See above comment
		*/
		if (qicwait(st, SC_LONGCMD) || (st->sc_status & O_SUMMARY)) {
			goto blewit;
		}
		return 0;
	}
}

/*
** qicshortcmd() -- issue a short command to the controller
**	Assumes controller is locked by caller
*/
qicshortcmd(st, func, msg, count)
	register struct softc_tape *st;
	short func;
	char *msg;
	register int count;
{
	if (st->sc_flags & SC_HARDERROR)		/* no can do */
		return EIO;

	mdsoftc.sc_iopb.p_func = func;
	mdsoftc.sc_iopb.p_dev = D_217;
	mdsoftc.sc_iopb.p_unit = st->sc_unit;
	while (count--) {
		st->sc_flags |= SC_SHORTCMD;
#ifdef TAPEDEBUG	
		iprintf("(SHORT> %s: c: %d)",
	     		dkerror(mdsoftc.sc_iopb.p_func, dsdcmds, NCMDS),
			swapw((long) mdsoftc.sc_iopb.p_rbc));
#endif TAPEDEBUG	
		if (dsdcmd(0)) {
			printf("qic%d: %s failed", st->sc_unit, msg);
			st->sc_flags &= ~SC_SHORTCMD;
			st->sc_flags |= SC_HARDERROR;
			return EIO;
		}
		if (qicwait(st, SC_SHORTCMD))
			return EIO;
	}
	return 0;
}

/*
** qicstatus() - read the tape unit status
*/
qicstatus(st)
	register struct softc_tape *st;
{
	/*
	** Actually make sure that you don't read status after a hard
	** error and return.
	*/
	bzero((caddr_t)mdsoftc.sc_inist.is_sb, sizeof(inist_t));

	/*
	** Can't read xfer status from drive to controller if the drive
	** is rewinding.  The drive ususally rewinds during an error
	** which is why we were trying to get status. Sigh.
	*/
	if (st->sc_flags & SC_HARDERROR)
		goto skip;
	/*
	** Can't allow you to read status if the last open was the
	** use of the NO rewind device.
	*/
#ifdef LASTACCESS
	if (st->sc_lastaccess)
		return 0;
#endif LASTACCESS
	/*
	** To read the status from this moronic controller you have to
	** tell it to go get it from the drive
	*/
	mdsoftc.sc_iopb.p_func = F_TDSTAT;
	mdsoftc.sc_iopb.p_dev = D_217;
	mdsoftc.sc_iopb.p_dba = (u_char *) swapw( (long) (inist_mbva +
		((long)mdsoftc.sc_inist.is_sb - (long)&mdsoftc.sc_inist)) );
	mdsoftc.sc_iopb.p_unit = st->sc_unit;
#ifdef TAPEDEBUG	
	iprintf("(STAT1: %s: c: %d)\n",
		dkerror(mdsoftc.sc_iopb.p_func, dsdcmds, NCMDS),
		swapw((long) mdsoftc.sc_iopb.p_rbc));
#endif TAPEDEBUG	
	if (dsdcmd(M_NOINT))
		return EIO;
skip:
	mdsoftc.sc_iopb.p_func = F_TSTAT;
#ifdef TAPEDEBUG	
	iprintf("(STAT2: %s: c: %d)\n",
		dkerror(mdsoftc.sc_iopb.p_func, dsdcmds, NCMDS),
		swapw((long) mdsoftc.sc_iopb.p_rbc));
#endif TAPEDEBUG	
	if (dsdcmd(M_NOINT))
		return EIO;
	return 0;
}

/*
** qicwait() -- wait for a tape command to finish
*/
int
qicwait(st, bits)
	register struct softc_tape *st;
{
	register int sps;

	sps = SPL();
	while (st->sc_flags & bits) {
		st->sc_flags |= SC_WANTED;
		sleep((caddr_t)st, PZERO-1);
	}
	splx(sps);
	return (st->sc_status & O_HARD);
}

/* ARGSUSED */
mfioctl(dev, cmd, addr, flag)
	caddr_t addr;
	dev_t dev;
{
	register short unit = D_DRIVE(dev);
	register struct softc_disk *sd = &mdsoftc.sc_floppy[unit];
	struct flpop flpop;

	switch(cmd) {
	  case FLP_IOCTOP:
		if(copyin((caddr_t)addr, (caddr_t)&flpop, sizeof(flpop))) {
			u.u_error = EFAULT;
			break;
		}
		switch(flpop.flp_op) {
		  case FLP_FORMAT:
			if (flpformat(unit,sd->sc_cyl,sd->sc_hd)) {
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
	/* DSDUNLOCK(); */
}

flpformat(unit,cyl,hd)
	int unit;
	short cyl,hd;
{
	register cylinders, heads;
	register struct fmtb *fmb = &mdsoftc.sc_inist.is_fmtb;
	register iopb_t *iop = &mdsoftc.sc_iopb;

	fmb->f_pat1 = 0x55;
	fmb->f_func = 0x00;
	fmb->f_pat3 = 0xFF;
	fmb->f_pat2 = 0xAA;
	fmb->f_ilv  = 1;
	fmb->f_pat4 = 0x00;

	printf(" *** Format Floppy %d: cyls=%d sides=%d ***\n", unit, cyl, hd);
	for(cylinders = 0; cylinders < cyl; cylinders++) {
	    for(heads = 0; heads < hd; heads++) {
		iop->p_dev	= D_FLP;
		iop->p_func	= F_FORMAT;
		iop->p_unit	= unit;
		iop->p_cyl	= cylinders;
		iop->p_sec	= 0;
		iop->p_hd	= heads;
		iop->p_dba	= (u_char *) swapw( (long) (inist_mbva +
		 ((long)&mdsoftc.sc_inist.is_fmtb - (long)&mdsoftc.sc_inist)) );

		if (dsdcmd(M_NOINT)) {
			printf("  Format err at %d/%d\n", cylinders, hd);
			return 1;
		}
	    }
	    if(cylinders && (cylinders%10) == 0)
		printf("%3d ",cylinders);
	}
	printf("\n");
	return 0;
}
