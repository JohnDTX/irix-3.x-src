/*
** std.c	- Copyright (C) JCS Computer Services - Sunnyvale CA 94089
**		- chase bailey - December 1984
**		- Any use, copy or alteration is strictly prohibited
**		- unless authorized in writing by JCS Computer Services.
**
**	$Source: /d2/3.7/src/sys/multibus/RCS/std.c,v $
**	$Revision: 1.1 $
**	$Date: 89/03/27 17:31:40 $
*/
/*
** Modification History --
**	Started on 12/28/84
*/

#include "sd.h"

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
#include "../h/autoconf.h"
#include "machine/cpureg.h"
#include "../multibus/stdreg.h"
#include "../multibus/mbvar.h"
#include "../multibus/stduib.h"

#define	STD_DEBUG
#undef	METERING

/*
 * Metering
 */
#ifdef	METERING
#define	METER(x)	(x)
struct {
	long	wasdry;
	long	ready;
	long	prefill;
	long	nothing;
	short	buzz[100];
} std;
#else
#define	METER(x)
#endif

/* error list */
struct	dkerror sterrs[] = {
	{ 0x01,	"end of tape encountered during copy command" },
	{ 0x02,	"file mark encountered" },
	{ 0x10,	"disk not ready" },
	{ 0x11,	"invalid disk unit address" },
	{ 0x12,	"seek error" },
	{ 0x13,	"data field ecc/crc error" },
	{ 0x14,	"invalid command code" },
	{ 0x15,	"invalid cylinder address in iopb" },
	{ 0x16,	"invalid sector number in iopb" },
	{ 0x18,	"bus timeout error" },
	{ 0x1A,	"disk write protected" },
	{ 0x1B,	"disk not selected" },
	{ 0x1C,	"no address mark in header field" },
	{ 0x1D,	"no address mark in data field" },
	{ 0x1E,	"drive faulted" },
	{ 0x20,	"disk surface overrun" },
	{ 0x21,	"id field error, wrong sector" },
	{ 0x22,	"crc error in id field" },
	{ 0x23,	"uncorrectable data error" },
	{ 0x26,	"missing sector pulse" },
	{ 0x27,	"format timeout" },
	{ 0x28,	"no index pulse during format" },
	{ 0x29,	"sector not found" },
	{ 0x2A,	"id field error, wrong head" },
	{ 0x2D,	"seek timeout" },
	{ 0x2F,	"not on cylinder" },
	{ 0x30,	"restore/recalibrate timeout" },
	{ 0x40,	"unit not initialized" },
	{ 0x42,	"gap specification error" },
	{ 0x4C,	"mapped header error" },
	{ 0x50,	"sectors per track specification error" },
	{ 0x51,	"bytes per sector specification error" },
	{ 0x52,	"interleave factor specification error" },
	{ 0x53,	"invalid head number in iopb" },
	{ 0x60,	"protection timeout error" },
	{ 0x61,	"maximum cylinder number specification error" },
	{ 0x62,	"number of heads specification error" },
	{ 0x63,	"step pulse specification error" },
	{ 0x64,	"reserved byte specification error" },
	{ 0x65,	"ram failure on odd byte" },
	{ 0x66,	"ram failure on even byte" },
	{ 0x67,	"event ram failure" },
	{ 0x68,	"device not previously recalibrated" },
	{ 0x69,	"controller error" },
	{ 0x6A,	"invalid sector number" },
	{ 0x6B,	"timer failure" },
	{ 0x6C,	"rom failure on odd byte" },
	{ 0x6D,	"rom failure on even byte" },
	{ 0x80,	"tape drive not selected" },
	{ 0x81,	"tape drive not ready" },
	{ 0x82,	"tape drive not online" },
	{ 0x83,	"cartridge not in place" },
	{ 0x84,	"unexpected beginning of tape" },
	{ 0x85,	"unexpected end of tape" },
	{ 0x86,	"unexpected file mark encountered" },
	{ 0x87,	"unrecoverable data error" },
	{ 0x88,	"block in error not located" },
	{ 0x89,	"no data detected" },
	{ 0x8A,	"write protected" },
	{ 0x8B,	"illegal command" },
	{ 0x8C,	"command sequence timeout" },
	{ 0x8D,	"status sequence timeout" },
	{ 0x8E,	"data block transfer timeout" },
	{ 0x8F,	"filemark search timeout" },
	{ 0x90,	"unexpected exception" },
	{ 0x91,	"invalid tape unit address" },
	{ 0x92,	"ready timeout" },
	{ 0x93,	"tape timeout specification error" },
	{ 0x94,	"invalid block count" },
};
#define	NERRS	(sizeof(sterrs) / sizeof(struct dkerror))

int	stdstrategy();
int	stdprobe(), stdattach(), stdstart(), stdintr();
struct	mb_device *stddinfo[NSD];
struct	mb_ctlr *stdcinfo[NSTD];
struct	mb_driver stddriver = {
	stdprobe, stdattach, (int (*)())0, stdstart, stdintr,
	(char *(*)())0, "sd", stddinfo, "st", stdcinfo,
};

/*
** stdprobe:
**	- probe the controller, and see if its there
**	- initialize it, to get things going
**	- remember that the port address has to be byte swapped 
**	- read in controller firmware version and print it out
*/
stdprobe(reg)
	int reg;
{
	register iopb_t *iop;
	register int i;
	long timeout;
	long iopb_mbva;

	stdsoftc.sc_ioaddr = (caddr_t)MBIO_VBASE + reg;
	RESET();
	DELAY(50000);
	CLEAR();
	DELAY(50000);
	timeout = 1000000;
	while ((STATUSREG() & 1) && --timeout)
		;
	if (timeout == 0)
		return (CONF_DEAD);
	stdsoftc.sc_flags = SC_ALIVE;
	/*
	** If controller probed, map in iopbs.  We assume that we will find
	** at least one drive.  If not, then we just wasted one page of
	** the multibus map
	*/
	iopb_mbva = mbmapkget((caddr_t)&stdsoftc.sc_iopb[0],
			      (long)sizeof(stdsoftc.sc_iopb));
	/*
	** Set up the portions of the IOPB's which will not change
	*/
	iop = &stdsoftc.sc_iopb[0];
	stdsoftc.sc_nextiopb = NULL;
	for (i = 0; i < TOTAL_IOPBS; i++, iop++) {
		/*
		 * Clear entire iopb, thus setting all fields to zero.
		 * Only have to set non-zero fields this way.
		 */
		bzero((caddr_t)iop, sizeof(iopb_t));
		iop->i_ioh = MB(stdsoftc.sc_ioaddr);	/* XXX needed? */
		iop->i_iol = LB(stdsoftc.sc_ioaddr);
		iop->i_option = O_OPTIONS;
		iop->i_mbva = iopb_mbva;
		iopb_mbva += sizeof(iopb_t);

		/* link iopb on to head of free list */
		iop->i_next = stdsoftc.sc_nextiopb;
		stdsoftc.sc_nextiopb = iop;
	}

	/*
	** Now ask controller for its firmware revision information.
	** If this doesn't work, mark controller as non-functioning.
	*/
	iop = &stdsoftc.sc_iopb[0];
	if (stdcmd(iop, C_REPORT, 1) == 0) {
		printf("(rev %c.%c, rel %c) ",
			((iop->i_error & 0xF0) >> 4) + '0',
			(iop->i_error & 0xF) + '0',
			iop->i_unit ? iop->i_unit : '0');
	} else
		return (CONF_DEAD);
	return (CONF_ALIVE);
}

/*
** stdattach: disk -- attach a disk to its controller
**	- configure to 1 cyl, 1 head and 64 sectors
**	- read the label
**	- reconfigure to cyl/hd/sec in label
**	- stash file system info into static structure
** XXX	this should be done some other way
*/
stdattach(mi)
	struct mb_device *mi;
{
	register struct disk_label *l;
	register iopb_t *iop = &stdsoftc.sc_iopb[0];
	register struct softc_disk *sd;
	register struct buf *bp;
	register short unit = mi->mi_unit;
	int result;

	/*
	** Get a buffer to read the label into.
	*/
	bp = getdmablk(1);

	/*
	** Configure to read label
	*/
	if (stdinit(unit, &default_uib))
		goto error;
	/*
	** Set up the iopb to read the label, then read it.
	*/
	iop->i_unit = unit;
	iop->i_bufh = HB(bp->b_iobase);
	iop->i_bufm = MB(bp->b_iobase);
	iop->i_bufl = LB(bp->b_iobase);
	iop->i_head = iop->i_cylh = iop->i_cyll = iop->i_sech =
		iop->i_secl = iop->i_scch = 0;
	iop->i_sccl = 1;
	if (stdcmd(iop, C_READNOCACHE, 1))
		goto error;

	/*
	** Now examine the label and insure its good.  If so, then
	** initialize the drive again using the parameters from the
	** label.  If the label is incorrect, then reject the drive.
	*/
	l = (struct disk_label *)bp->b_un.b_addr;
	if (l->d_magic != D_MAGIC) {
		dk_prname((struct disk_label *)NULL);
		goto error;
	}
	sd = &stdsoftc.sc_disk[unit];
	std_build_uib(l, &sd->sc_uib);
	if (stdinit(unit, &sd->sc_uib))
		goto error;

	/*
	** save label info and init drive software state
	*/
	sd->sc_flags = SC_ALIVE;
	sd->sc_cyl = l->d_cylinders;
	sd->sc_hd  = l->d_heads;
	sd->sc_sec = l->d_sectors;
	sd->sc_spc = sd->sc_hd * sd->sc_sec;
	bcopy((caddr_t)l->d_map, (caddr_t)sd->sc_fs, sizeof(sd->sc_fs));
	dk_prname(l);
	setroot(&stddriver, mi, unit << 3, 0070, l, stdstrategy);
	result = CONF_ALIVE;
	goto out;

error:
	result = CONF_FAULTED;

out:
	brelse(bp);
	return (result);
}

/*
 * sdname:
 *	- return an ascii representation for the drive name
 * XXX	this should be done some other way (bdevsw/cdevsw)
 */
char *
sdname(dev)
	dev_t dev;
{
	static char name[5] = "sd";

	name[2] = D_DRIVE(dev) + '0';
	name[3] = D_FS(dev) + 'a';
	return (name);
}

/*
** stdopen - Called upon the open to check some very basic
**	     things About the disk
*/
/*ARGSUSED*/
stdopen(dev, flag)
	dev_t dev;
	int flag;
{
	register short unit = D_DRIVE(dev);

	if ((unit > MAX_WINNYS) ||
	    !(stdsoftc.sc_disk[unit].sc_flags & SC_ALIVE)) {
		u.u_error = ENODEV;
		return;
	}
}

/*
** stdstrategy(bp) - Called to do the first checking on the disk request and
**		   - queue up the request via a call to disksort and check
**		   - the active before calling stdstart.
*/
stdstrategy(bp)
	register struct buf *bp;
{
	register struct disk_map *fs;
	register short unit = D_DRIVE(bp->b_dev);
	register struct softc_disk *sd;
	register struct iobuf *dp;
	register int s;
	register daddr_t bn;
	register short temp;

	sd = &stdsoftc.sc_disk[unit];
	if (!(sd->sc_flags & SC_ALIVE)) {
		berror(bp);
		return;
	}
	fs = &sd->sc_fs[D_FS(bp->b_dev)];
	if (dk_rangecheck(bp, fs, BLKSIZE, BLKSHIFT))
		return;
	if ((bp->b_bcount == 0) || (bp->b_bcount & (BLKSIZE - 1))) {
		berror(bp);
		return;
	}

	/* crunch disk address for start and disksort */
	sd->sc_bn = bn = bp->b_blkno + fs->d_base;
	bp->b_cyl = bn / sd->sc_spc;
	temp = bn % sd->sc_spc;
	bp->b_head = temp / sd->sc_sec;
	bp->b_sector = temp % sd->sc_sec;

	dp = &sd->sc_tab;
	s = SPL();
	if (dp->b_actf)
		disksort(dp, bp);
	else {
		dp->b_actf = bp;
		dp->b_actl = bp;
		bp->av_forw = NULL;
	}
	if (stdsoftc.sc_cmds++ == 0)
		stdstart();
	splx(s);
}

/*
 * stdrefill:
 *	- refill any empty iopb's from each drives buffer chains
 *	- MUST be called at spl
 *	- return 1 if we filled in an iopb
 */
int
stdrefill()
{
	register iopb_t *iop;
	register struct buf *bp;
	register struct iobuf *dp;
	register struct softc_disk *sd;
	register u_long temp;
	register int filled;

	/*
	 * Check each drive and see if it has some requests that we
	 * can pre-fill in the iopb's for.
	 */
	filled = 0;
	for (sd = &stdsoftc.sc_disk[0]; sd < &stdsoftc.sc_disk[MAX_WINNYS];
		sd++) {
		/*
		 * If driver has already used up its iopb quota, or if
		 * the drive has no commands pending, skip it.
		 */
		while (sd->sc_niopbs < MAX_IOPBS) {
			dp = &sd->sc_tab;
			if (dp->b_actf == NULL)
				break;

			/*
			 * Unlink request from head of active queue.
			 * Allocate one of the iopbs.
			 */
			bp = dp->b_actf;
			dp->b_actf = bp->av_forw;
			iop = stdsoftc.sc_nextiopb;
			stdsoftc.sc_nextiopb = iop->i_next;

			/* fill in iopb */
			iop->i_bp = bp;
			iop->i_unit = D_DRIVE(bp->b_dev);
			iop->i_head = bp->b_head;
			iop->i_cylh = MB(bp->b_cyl);
			iop->i_cyll = LB(bp->b_cyl);
			iop->i_sech = MB(bp->b_sector);
			iop->i_secl = LB(bp->b_sector);
			temp = bp->b_bcount >> BLKSHIFT;
			iop->i_scch = MB(temp);
			iop->i_sccl = LB(temp);
			temp = (u_long)bp->b_iobase;
			iop->i_bufh = HB(temp);
			iop->i_bufm = MB(temp);
			iop->i_bufl = LB(temp);

			/* append to controller iopb queue */
			if (stdsoftc.sc_first)
				stdsoftc.sc_last->i_next = iop;
			else
				stdsoftc.sc_first = iop;
			stdsoftc.sc_last = iop;
			iop->i_next = NULL;

			stdsoftc.sc_niopbs++;	/* one more used */
			sd->sc_niopbs++;	/* ditto */
			filled++;		/* ditto */
		}
	}
	return (filled);
}

stdstart()
{
	register iopb_t *iop;
	register struct buf *bp;
	register int unit;
	register u_long dkn;

top:
	/*
	 * First see if more iopb's should be computed
	 */
	if ((iop = stdsoftc.sc_first) == NULL) {
		if ((stdsoftc.sc_niopbs < TOTAL_IOPBS) && stdrefill()) {
			METER(std.wasdry++);
			goto top;
		}
		METER(std.nothing++);
		return;
	} else {
		METER(std.ready++);
	}

	/*
	** If command won't start, return an error
	*/
	bp = iop->i_bp;
	unit = iop->i_unit;
	if (stdcmd(iop, (bp->b_flags & B_READ) ? C_READ : C_WRITE, 0)) {
		printf("%s: can't start command\n", sdname(bp->b_dev));
		berror(bp);
		stdsoftc.sc_first = iop->i_next;
		stdsoftc.sc_niopbs--;
		if (stdsoftc.sc_nextiopb)
			iop->i_next = stdsoftc.sc_nextiopb;
		else
			iop->i_next = NULL;
		stdsoftc.sc_nextiopb = iop;
		if (--stdsoftc.sc_cmds)
			goto top;
		return;
	}

	/* instrument the transfer now that its going */
	dkn = stddinfo[unit]->mi_dk;		/* dk unit # */
	dk_busy |= 1<<dkn;
	dk_xfer[dkn]++;
	dk_wds[dkn] += bp->b_bcount >> 6;

	/*
	 * While command is going, fill in next iopb if there is room
	 */
	if (stdsoftc.sc_niopbs < TOTAL_IOPBS) {
		(void) stdrefill();
		METER(std.prefill++);
	}
}

stdread(dev)
	dev_t dev;
{
	int unit;

	if (u.u_count & (BLKSIZE - 1))
		u.u_error = EIO;
	else {
		unit = D_DRIVE(dev);
		if (physck(stdsoftc.sc_disk[unit].sc_fs[D_FS(dev)].d_size,
			     B_READ, BLKSHIFT))
			physio(stdstrategy, (struct buf *)NULL, dev,
					    B_READ, minphys);
	}
}

stdwrite(dev)
	dev_t dev;
{
	int unit;

	if (u.u_count & (BLKSIZE - 1))
		u.u_error = EIO;
	else {
		unit = D_DRIVE(dev);
		if (physck(stdsoftc.sc_disk[unit].sc_fs[D_FS(dev)].d_size,
			     B_WRITE, BLKSHIFT))
			physio(stdstrategy, (struct buf *)NULL, dev,
					    B_WRITE, minphys);
	}
}

/*
** stdintr -- handles interrupts for the winchester disk.
*/
stdintr()
{
	register struct iopb *iop;
	register struct buf *bp;
	register struct softc_disk *sd;
	register int unit;

	/*
	 * Quickly check that controller exists, and that it indicates
	 * completion.  This is done so that multiple controllers sharing
	 * the same interrupt line will have less latency.
	 */
	if (!(stdsoftc.sc_flags & SC_ALIVE))
		return (0);				/* nope */
	if (!(STATUSREG() & ST_DONE)) {
		if (ac.a_probing)
			CLEAR();			/* XXX */
		return (0);				/* nope */
	}

	iop = stdsoftc.sc_first;
	bp = iop->i_bp;
	unit = RDINTR();
#ifdef	STD_DEBUG
	iop->i_bp = NULL;
	/*
	 * Insure that we expected an interrupt, and that it matches
	 * the device we issued a command to.
	 * XXX this will be piss poor when we start doing overlapped seeks
	 */
	if ((iop == NULL) || (stdsoftc.sc_cmds == 0)) {
		printf("sd%d: extra interrupt, iop=%x sc_cmds=%d status=%x\n",
			      unit, iop, stdsoftc.sc_cmds,
			      STATUSREG());
		CLEAR();
		return (1);
	}
	if ((unit < 0) || (unit > 4) || (iop->i_unit != unit) ||
	    (unit != UNIT(bp->b_dev))) {
		printf("sd%d: unit=%d iopbunit=%d b_dev=%x\n",
			      unit, iop->i_unit, bp->b_dev);
		CLEAR();
		return (0);
	}
#endif
	dk_busy &= ~(1<<stddinfo[unit]->mi_dk);		/* clean up metering */

	/* print an error if command failed */
	sd = &stdsoftc.sc_disk[(short) unit];
	if (iop->i_status != S_OK) {
	printf("%s: hard error ``%s'' iopstatus=%x status=%x bn=%d cmd=%x\n",
		    sdname(bp->b_dev),
		    dkerror((u_char)iop->i_error, sterrs, NERRS),
		    iop->i_status, STATUSREG(),
		    sd->sc_bn, iop->i_cmd);
		bp->b_flags |= B_ERROR;
	}

	/* remove iopb from todo list */
	stdsoftc.sc_first = iop->i_next;
	stdsoftc.sc_niopbs--;
	sd->sc_niopbs--;

	/* link iopb on to head of free list */
	iop->i_next = stdsoftc.sc_nextiopb;
	stdsoftc.sc_nextiopb = iop;

	/* start next command */
	if (--stdsoftc.sc_cmds)
		stdstart();
	else {
		CLEAR();
		stdsoftc.sc_flags |= SC_NEEDWAIT;
	}

	/* complete previous command */
	iodone(bp);
	return (1);
}

stdcmd(iop, cmd, waiting)
	register iopb_t *iop;
	int cmd, waiting;
{
	register long timeout;
	register int s;
	register long i;
	register int unit;

	unit = iop->i_unit;
	s = spl6();

	/*
	 * If previous interrupt didn't have another command to start
	 * right away, this flag is set.  This is done to allow the
	 * board to complete the interrupt in parallel with the host
	 * cpu.  If we spin here, it will be a shorter time than
	 * if we wait in the interrupt routine.
	 */
	if (stdsoftc.sc_flags & SC_NEEDWAIT) {
		METER(timeout = 0);
		while (STATUSREG() & ST_DONE)
			METER(timeout++);
		METER(std.buzz[timeout]++);
		stdsoftc.sc_flags &= ~SC_NEEDWAIT;
	}
#ifdef	STD_DEBUG
	timeout = 50000;
	while (((i = STATUSREG()) & ST_BUSY) && --timeout)
		;
	if (timeout <= 0) {
		printf("sd%d: timeout, idle controller, status=%x\n",
			      iop->i_unit, i);	
		return (1);
	}
#endif
	iop->i_cmd = cmd;
	iop->i_status = 0;
	iop->i_error = 0;

	/*
	** Set up the iopb address in the I/O of the Controller
	*/
	timeout = iop->i_mbva;
	*STR1 = HB(timeout);
	*STR2 = MB(timeout);
	*STR3 = LB(timeout);

	if (!waiting) {
		START();			/* Start the command */
		splx(s);
		return (0);
	}

	DELAY(500);
	STARTWO();
	/* Wait for status */
	timeout = 10000000;
	for (;;) {
		if (iop->i_status == S_OK) {
			CLEAR();
			splx(s);
			return (0);
		}
		if (iop->i_status == S_ERROR) {
			printf("[sd%d: ERROR(%x) Cmd=%x status %x err %s] ",
				       unit, iop->i_error, iop->i_cmd,
				       iop->i_status,
				       dkerror((u_char)iop->i_error,
					       sterrs, NERRS));
			CLEAR();
			splx(s);
			return (1);
		}
		if ((--timeout) == 0) {
			printf("[sd%d: Cmd=%x Timeout: s %x err %x] ", unit,
				       iop->i_cmd, iop->i_status,
				       iop->i_error);
			CLEAR();
			splx(s);
			return (1);
		}
	}
}

/*
** stdinit() -- unit type and mode (check to see if inited)
*/
stdinit(unit, uib)
	int unit;
	register uib_t *uib;
{
	register iopb_t *iop = &stdsoftc.sc_iopb[0];
	long uib_mbva;
	int x;

	/* get multibus map space that covers the uib */
	uib_mbva = mbmapkget((caddr_t)uib, (long)sizeof(uib_t));

	iop->i_unit = unit;
	iop->i_bufh = HB(uib_mbva);
	iop->i_bufm = MB(uib_mbva);
	iop->i_bufl = LB(uib_mbva);
	iop->i_head = iop->i_cylh = iop->i_cyll = iop->i_sech =
		iop->i_secl = iop->i_scch = iop->i_sccl = 0;

	x = stdcmd(iop, C_INIT, 1);
	mbmapkput(uib_mbva, (long)sizeof(uib_t));
	return (x);
}

/*
 * std_build_uib:
 *	- build up a uib for a drive
 */
std_build_uib(l, uib)
	register struct disk_label *l;
	register uib_t *uib;
{
	uib->u_spt = l->d_sectors;		/* sectors per track */
	uib->u_hds = l->d_heads;		/* heads per cylinder */
	uib->u_bpsl = LB(BLKSIZE);		/* bytes per sector */
	uib->u_bpsh = MB(BLKSIZE);
	/*
	** interphase claims gap[12] must be an odd value
	** gap1 as vertex ships their drives is 16
	** gap2 as vertex ships their drives is 16
	** gap3 is specified as 8 bytes min for a 256 sector size
	*/
	uib->u_gap1 = l->d_misc[0];
	uib->u_gap2 = l->d_misc[1];
	uib->u_gap3 = l->d_misc[2];
	uib->u_ilv = l->d_interleave;
	uib->u_retry = 5;		/* lots */
	uib->u_eccon = 1;		/* XXX was in misc[9] */
	uib->u_reseek	= 1;
#ifdef	fixme
	if (floppy)
		uib->u_reseek = 0;
#endif
	uib->u_mvbad	= 0;		/* Move Bad Data */
	uib->u_inchd	= 1;		/* Increment by head */
	uib->u_resv0 	= 0;
	uib->u_intron	= 0;		/* Interrupt on status change */

	/* Skew spirial factor */
	uib->u_skew = l->d_cylskew;
	uib->u_resv1 = 0;		/* Group Size (2190) */
	/*
	** Motor On and Head Unload
	** will be 0 for the Winchesters, 0x11 for the Floppies
	** Motor off is 5 seconds and Head Unload is 5 seconds.
	*/
	uib->u_mohu = l->d_misc[10];

	/*
	** Turn on caching and zero latency enable
	** Zero latency will always be on.
	*/
	uib->u_options = U_OPTIONS;		/* Zero Latency */
#ifdef	fixme
	if (floppy)
		uib->u_options = 2;		/* Zero Latency */
#endif
	/*
	** ddb drive descriptor byte is:
	** Tandon Floppy: 0 1 0 0 0 1 0 0
	** Vertex Disk:   0 0 0 0 0 1 1 0
	** Atasi Disk:    0 0 0 0 0 1 1 0
	** Maxtor(ESDI):  0 0 1 0 0 1 1 1
	** Maxtor(506) :  0 0 0 0 0 1 1 0
	**
	** smc step motor control is:
	** Tandon Turn on time is 250ms (set to 0x03(300ms))
	** All Winchesters with Buf Steps set to 0x20
	** else Winchesters set to 0
	*/
	uib->u_ddb = l->d_misc[11];
	uib->u_smc = l->d_misc[12];
	/*
	** vertex specifies 2 us min step pulse width
	** Each 1 = 5us
	** vertex specifies min of 5 us pulse interval for buffered
	** pulses and max of 39.  Normal pulses are min or 25 us
	** The Tandon Floppy has 3ms spi and spw of 200ns
	*/
	uib->u_spw = l->d_misc[3];
	uib->u_spil = LB(l->d_misc[4]);
	uib->u_spih = MB(l->d_misc[4]);
	/*
	** the head load/settling time seems to be included in the
	** track-to-track time
	** Making the Vertex at 1ms and the Atasi at 3ms
	** The Tandon Floppy is 15ms.
	*/
	uib->u_hlst = l->d_misc[8];
	/*
	** vertex specifies the max track-to-track time as 5 ms
	*/
	uib->u_ttst = l->d_misc[5];
	uib->u_ncl = LB(l->d_cylinders);
	uib->u_nch = MB(l->d_cylinders);
	/*
	** Write Precompensation -- Can't use this.
	** The Tandon Floppy has 0.
	*/
	uib->u_wpscl = LB(l->d_misc[6]);
	uib->u_wpsch = MB(l->d_misc[6]);
	/*
	** cant find info about reduced write current being needed.  
	** assume not (ffff)
	*/
	uib->u_rwcscl = LB(l->d_misc[7]);
	uib->u_rwcsch = MB(l->d_misc[7]);
	/* GASP.  Thank goodness that's done */
}

/*
** Used by unix to print a diagnostic
*/
stdprint(dev, str)
	dev_t dev;
	char *str;
{
	uprintf("%s on %s\n", str, sdname(dev));
}

/*
** stddump:
**	- dump kernel memory out to disk starting at block ``bn'' at
**	  multibus addr mbva of length count
** XXX	this can be done in a better fashion
*/
stddump(dev, bn, mbva, count)
	dev_t dev;
	daddr_t bn;
	long mbva;
	int count;
{
	register struct disk_map *fs;
	register short unit = D_DRIVE(dev);
	register struct softc_disk *sd;
	register iopb_t *iop = &stdsoftc.sc_iopb[0];
	short temp;
	int head, cyl, sector;

	sd = &stdsoftc.sc_disk[unit];
	if (!(sd->sc_flags & SC_ALIVE)) {
		printf("%s: device is not alive\n", sdname(dev));
		return (ENODEV);
	}
	fs = &sd->sc_fs[D_FS(dev)];
	count = (count + BLKSIZE - 1) >> BLKSHIFT;
	if (((unsigned)bn > fs->d_size) ||
	    ((unsigned)bn + count > fs->d_size)) {
		printf("%s: bn out of range(bn=%d)\n", sdname(dev), bn);
		return (ENXIO);
	}

	/* crunch disk address */
	bn += fs->d_base;
	cyl = bn / sd->sc_spc;
	temp = bn % sd->sc_spc;
	head = temp / sd->sc_sec;
	sector = temp % sd->sc_sec;

	/*
	** Set up the IOPB
	*/
	iop->i_unit = unit;
	iop->i_head = head;
	iop->i_cylh = MB(cyl);
	iop->i_cyll = LB(cyl);
	iop->i_sech = MB(sector);
	iop->i_secl = LB(sector);
	iop->i_scch = MB(count);
	iop->i_sccl = LB(count);
	iop->i_bufh = HB(mbva);
	iop->i_bufm = MB(mbva);
	iop->i_bufl = LB(mbva);

	/*
	** If command won't start, return an error
	*/
	if (stdcmd(iop, C_WRITE, 1)) {
		printf("%s: can't start command\n", sdname(dev));
		return (EIO);
	}
	return (0);
}

#ifdef	notdef
stdpp(unit)
{
	register iopb_t *iop = &stdsoftc.sc_iopb;
	register uib_t *uib = &stdsoftc.sc_disk[unit].sc_uib;

	printf("st%d: ", iop->i_unit);
	printf("iop: c:%x o:%x %d/%d/%d s:%x e:%x scc:%d buf:%x\n",
		 iop->i_cmd, iop->i_option,
		 ((iop->i_cylh << 8) | iop->i_cyll), iop->i_head,
		 ((iop->i_sech << 8) | iop->i_secl), iop->i_status,
		 iop->i_error, ((iop->i_scch << 8) | iop->i_sccl),
		 (((iop->i_bufh << 16) | (iop->i_bufm << 8)) | iop->i_bufl));
	printf("uib: h %d, s %d, bps %d, gaps %d %d %d, ilv (%d)%d, sk %d\n",
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
#endif	notdef
