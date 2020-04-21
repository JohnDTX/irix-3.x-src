/*
** Interphase 2190 disk driver
**
** Written by: Chase Bailey
**
**	$Source: /d2/3.7/src/sys/multibus/RCS/iph.c,v $
**	$Revision: 1.1 $
**	$Date: 89/03/27 17:31:25 $
*/

#include "ip.h"

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
#include "../multibus/iphreg.h"
#include "../multibus/mbvar.h"

#undef DEBUG

#ifdef DEBUG
char	ipdebug = 0;
#endif

/*
** IP Flags
*/
#define IPINIT 1

struct	dkerror iperrs[] = {
	{ 0x10,	"disk not ready" },
	{ 0x11,	"invalid disk address" },
	{ 0x12,	"seek error" },
	{ 0x13,	"ecc code error in data field" },
	{ 0x14,	"invalid command code" },
	{ 0x16,	"invalid sector in command" },
	{ 0x18,	"bus timeout" },
	{ 0x1A,	"disk write protected" },
	{ 0x1B,	"unit not selected" },
	{ 0x1C,	"no address mark in header field" },
	{ 0x1E,	"drive faulted" },
	{ 0x23,	"uncorrectable error" },
	{ 0x26,	"no sector pulse" },
	{ 0x27,	"data overrun" },
	{ 0x28,	"no index pulse on write format" },
	{ 0x29,	"sector not found" },
	{ 0x2A,	"id field error (wrong head)" },
	{ 0x2B,	"invalid sync in data field" },
	{ 0x2D,	"seek timeout error" },
	{ 0x2E,	"busy timeout" },
	{ 0x2F,	"not on cylinder" },
	{ 0x30,	"rtz timeout" },
	{ 0x31,	"format overrun on data" },
	{ 0x40,	"unit not initialized" },
	{ 0x42,	"gap specification error" },
	{ 0x4B, "seek error" },
	{ 0x4C,	"mapped header error" },
	{ 0x50,	"sector per track error" },
	{ 0x51, "bytes per sector specification error" },
	{ 0x52,	"interleave specification error" },
	{ 0x53,	"invalid head address" },
	{ 0x5D,	"invalid dma burst count" },
};
#define	NERRS	(sizeof(iperrs) / sizeof(struct dkerror))

#ifdef	DEBUG
struct	dkerror ipcmds[] = {
	{ 0x81,	"read" },
	{ 0x82,	"write" },
	{ 0x83,	"verify" },
	{ 0x84,	"format track" },
	{ 0x85,	"map alternate track" },
	{ 0x86,	"report configuration" },
	{ 0x87,	"initialize" },
	{ 0x89,	"restore" },
	{ 0x8A,	"seek" },
	{ 0x8F,	"reset" },
	{ 0x91,	"direct read" },
	{ 0x92,	"direct write" },
	{ 0x93,	"read absolute" },
	{ 0x94,	"read non-cached" },
};
#define	NCMDS	(sizeof(ipcmds) / sizeof(struct dkerror))
#endif

int	ipstrategy();
int	ipprobe(), ipattach(), ipstart(), ipintr();
struct	mb_device *ipdinfo[NIP];
struct	mb_ctlr *ipcinfo[NIPH];
struct	mb_driver iphdriver = {
	ipprobe, ipattach, (int (*)())0, ipstart, ipintr,
	(char *(*)())0, "ip", ipdinfo, "iph", ipcinfo,
};

char	ip_initing;	/* true when initing the drives */

/*
** ipprobe:
**	- probe the controller, and see if its there
**	- initialize it, to get things going
**	- remember that the port address has to be byte swapped 
*/
ipprobe(reg)
	int reg;
{
	ipsoftc.sc_ioaddr = (caddr_t)MBIO_VBASE + reg;
	CLEAR();
	DELAY(10);
	ipsoftc.sc_flags = SC_ALIVE;
	/*
	** If controller probed, map in iopb.  We assume that we will find
	** at least one drive.  If not, then we just wasted one page of
	** the multibus map
	*/
	ipsoftc.sc_iopb_mbva = mbmapkget((caddr_t)&ipsoftc.sc_iopb,
					 (long)sizeof(struct iopb));

	/*
	** Set up the iopb address in the I/O registers of the Controller
	*/
	*IPR1 = HB(ipsoftc.sc_iopb_mbva);
	*IPR2 = MB(ipsoftc.sc_iopb_mbva);
	*IPR3 = LB(ipsoftc.sc_iopb_mbva);
	return CONF_ALIVE;
}

/*
** ipattach: disk -- attach a disk to its controller
**	- configure to 1 cyl, 1 head and 64 sectors
**	- read the label
**	- reconfigure to cyl/hd/sec in label
**	- stash file system info into static structure
*/
ipattach(mi)
	struct mb_device *mi;
{
	register struct softc_disk *sd;
	register struct buf *bp;
	register struct disk_label *l;
	register iopb_t *iop = &ipsoftc.sc_iopb;
	register short unit = mi->mi_unit;
	int result = CONF_ALIVE;

ip_initing = 1;
	/*
	 * Configure to read label
	 * ipinit(unit, skew, gap1, gap2, groupsize, ilv, heads, sectors)
	 */
	bp = getdmablk(1);	/* Grab a block to read label into */
	if (ipinit(unit, 11, 20, 30, 11, 1, 1, 64))
		goto error;
	/*
	** Set up the iopb for the read after the basic init
	*/
	iop->i_unit = unit;
	iop->i_bufh = HB(bp->b_iobase);
	iop->i_bufm = MB(bp->b_iobase);
	iop->i_bufl = LB(bp->b_iobase);
	iop->i_head = iop->i_cylh = iop->i_cyll = iop->i_sech = iop->i_secl =0;
	iop->i_scch = 0;
	iop->i_sccl = 1;
	if (ipcmd(C_READNOCACHE, IP_WAIT))
		goto error;
	l = (struct disk_label *)bp->b_un.b_addr;
	if (l->d_magic != D_MAGIC) {
		dk_prname((struct disk_label *)NULL);
		goto error;
	}
					/* Re-configure from label */
	if (ipinit(unit, l->d_cylskew, (int)l->d_misc[0], (int)l->d_misc[1],
			 (int)l->d_misc[2], l->d_interleave, l->d_heads,
			 l->d_sectors))
		goto error;

    /* save label info and init drive software state */
	sd = &ipsoftc.sc_disk[unit];
	sd->sc_flags = SC_ALIVE;
	sd->sc_cyl = l->d_cylinders;
	sd->sc_hd  = l->d_heads;
	sd->sc_sec = l->d_sectors;
	sd->sc_spc = sd->sc_hd * sd->sc_sec;
	bcopy((caddr_t)l->d_map, (caddr_t)sd->sc_fs, sizeof sd->sc_fs);
	dk_prname(l);
	setroot(&iphdriver, mi, unit << 3, 0070, l, ipstrategy);
	goto out;

error:
	result = CONF_FAULTED;

out:
	brelse(bp);
ip_initing = 0;
	return result;
}

/* ARGSUSED */
/*
** ipopen(dev, flag) - Called upon the open to check some very basic things
**		      - About the disk.
*/
/*ARGSUSED*/
ipopen(dev, flag)
	dev_t dev;
	int flag;
{
	register short unit = D_DRIVE(dev);

	if ((unit > MAX_DRIVES) ||
	    !(ipsoftc.sc_disk[unit].sc_flags & SC_ALIVE)) {
		u.u_error = ENODEV;
		return;
	}
}

/*
** ipstrategy(bp) - Called to do the first checking on the disk request and
**		   - queue up the request via a call to disksort and check
**		   - the active before calling ipstart.
*/
ipstrategy(bp)
	register struct buf *bp;
{
	register struct disk_map *fs;
	register short unit = D_DRIVE(bp->b_dev);
	register struct softc_disk *sd;
	daddr_t bn;
	short temp;

	sd = &ipsoftc.sc_disk[unit];
	if (!(sd->sc_flags & SC_ALIVE)) {
		berror(bp);
		return;
	}
	fs = &sd->sc_fs[D_FS(bp->b_dev)];
	if (dk_rangecheck(bp, fs, BLK_SIZE, BLK_SHIFT))
		return;

	/* crunch disk address for start and disksort */
	bn = bp->b_blkno + fs->d_base;
	bp->b_cyl = bn / sd->sc_spc;
	temp = bn % sd->sc_spc;
	bp->b_head = temp / sd->sc_sec;
	bp->b_sector = temp % sd->sc_sec;

	(void) SPL();
	disksort(&ipsoftc.sc_tab, bp);
	if (ipsoftc.sc_tab.b_active == 0)
		ipstart();
	(void) spl0();
}

/*
** ipstart()
** Now set up the iopb and start the request using the bp pointed to by the
** ipsoftc.sc_table.
*/
ipstart()
{
	register iopb_t *iop = &ipsoftc.sc_iopb;
	register struct buf *bp;
	register daddr_t bn;
	register short unit;

	if ((bp = ipsoftc.sc_tab.b_actf) == 0)
		return;
	unit = D_DRIVE(bp->b_dev);
	ipsoftc.sc_tab.b_active = 1;
	ipsoftc.sc_tab.io_unit = unit;

	/*
	** Set up the IOPB
	*/
	iop->i_unit = unit;
	iop->i_head = bp->b_head;
	iop->i_cylh = MB(bp->b_cyl);
	iop->i_cyll = LB(bp->b_cyl);
	iop->i_sech = MB(bp->b_sector);
	iop->i_secl = LB(bp->b_sector);

	bn = (bp->b_bcount + BLK_SIZE - 1) >> BLK_SHIFT;	/* sec count */
	iop->i_scch = MB(bn);
	iop->i_sccl = LB(bn);

	bn = (long)bp->b_iobase;
	iop->i_bufh = HB(bn);
	iop->i_bufm = MB(bn);
	iop->i_bufl = LB(bn);

	/* fire off command */
	(void) ipcmd((bp->b_flags & B_READ) ? C_READ : C_WRITE, IP_NOWAIT);

	/* instrument the transfer now that its going */
	bn = ipdinfo[unit]->mi_dk;		/* dk unit # */
	dk_busy |= 1<<bn;
	dk_xfer[bn]++;
	dk_wds[bn] += bp->b_bcount >> 6;
}

ipread(dev)
	dev_t dev;
{
	if (u.u_count & (BLKSIZE - 1))
		u.u_error = EIO;
	else
	if (physck(ipsoftc.sc_disk[D_DRIVE(dev)].sc_fs[D_FS(dev)].d_size,
					  B_READ, BLK_SHIFT))
		physio(ipstrategy, (struct buf *)NULL, dev, B_READ, minphys);
}

ipwrite(dev)
	dev_t dev;
{
	if (u.u_count & (BLKSIZE - 1))
		u.u_error = EIO;
	else
	if (physck(ipsoftc.sc_disk[D_DRIVE(dev)].sc_fs[D_FS(dev)].d_size,
					  B_WRITE, BLK_SHIFT))
		physio(ipstrategy, (struct buf *)NULL, dev, B_WRITE, minphys);
}

/*
** ipintr() - process disk interrupts. return non-zero if the interrupt
**	was for this controller.
*/
ipintr()
{
	register struct iopb *iop;
	register struct buf *bp;
	register short status, unit;

	/*
	 * Quickly check that controller exists, and that it indicates
	 * completion.  This is done so that multiple controllers sharing
	 * the same interrupt line will have less latency.
	 */
	if (!(ipsoftc.sc_flags & SC_ALIVE))
		return (0);				/* nope */
	if (ip_initing) {
		CLEAR();
		return (1);
	}
	if (!(*IPR0 & IP_DONE)) {
		if (ac.a_probing)
			CLEAR();
		return (0);				/* nope */
	}

    /* clear interrupt and validate interrupt */
	iop = &ipsoftc.sc_iopb;
	unit = ipsoftc.sc_tab.io_unit;
	status = iop->i_status;
	CLEAR();
	if (ipsoftc.sc_tab.b_active == 0) {
		if (ac.a_probing)
			return (0);
		printf("iph0: unrequested interrupt\n");
		return (0);
	}
#ifdef	PARANOID
	if (status != S_OK && status != S_ERROR) {
		printf("iph0: funny status during interrupt=%x\n", status);
		return (0);
	}
#endif

    /* check error status */
	bp = ipsoftc.sc_tab.b_actf;
	dk_busy &= ~(1<<ipdinfo[unit]->mi_dk);
	switch(status) {
	  case S_ERROR:
		if(++ipsoftc.sc_tab.b_errcnt <= IP_RETRY) {
			ipstart();
			return (1);
		}
#ifdef	DEBUG
    printf("ip%d%c: hard error, err='%s'(%x) block=%d cmd='%s' (%d/%d/%d)\n",
		    unit, D_FS(bp->b_dev) + 'a',
		    dkerror(iop->i_error, iperrs, NERRS),
		    iop->i_error, bp->b_blkno,
		    dkerror(iop->i_cmd, ipcmds, NCMDS),
		    bp->b_cyl, bp->b_head, bp->b_sector);
#else
		printf("ip%d%c: hard error, err=`%s' bn=%d(%d/%d/%d) cmd=%x\n",
				unit, D_FS(bp->b_dev) + 'a',
				dkerror(iop->i_error, iperrs, NERRS),
				bp->b_blkno, bp->b_cyl, bp->b_head,
				bp->b_sector, iop->i_cmd);
#endif
		bp->b_flags |= B_ERROR;
		/* FALL THROUGH */

	  case S_OK:
		ipsoftc.sc_tab.b_actf = bp->av_forw;
		ipsoftc.sc_tab.b_errcnt = 0;
		ipsoftc.sc_tab.b_active = 0;
		if (ipsoftc.sc_tab.b_actf)
			ipstart();
		iodone(bp);
		break;
#ifdef	PARANOID
	  case S_BUSY:
	  default:
		ipstart();
		break;
#endif
	}
	return (1);
}

ipcmd(cmd, waiting)
	int cmd, waiting;
{
	register iopb_t *iop = &ipsoftc.sc_iopb;
	register long timo;
	register int s;

	s = spl6();
	timo = 20000;
	while ((*IPR0 & IP_BUSY) && --timo)
		;
	if (timo == 0) {
		if (!ac.a_probing)
			printf("iph0: busy timeout\n");
		goto error;
	}
	iop->i_cmd = cmd;
	iop->i_status = iop->i_error = 0;
	START();
	if (!waiting) {
		splx(s);
		return 0;
	}

    /* wait for command to finish */
	timo = 1000000;
	while ((iop->i_status != S_ERROR) &&
	       (iop->i_status != S_OK) && --timo)
		;
	CLEAR();
	if (timo) {
		switch (iop->i_status) {
		 case S_OK:
			splx(s);
			return 0;
		 default:
			printf("iph0: strange status %x\n", iop->i_status);
			splx(s);
			return 1;
		 case S_BUSY:
		 case S_ERROR:
			if (!ac.a_probing)
				printf("iph0: status=%x error=%x\n",
					      iop->i_status, iop->i_error);
			goto error;
		}	
	}
	if (!ac.a_probing)
#ifdef	DEBUG
		printf("iph0: cmd timeout, cmd='%s'\n",
			      dkerror(ipcmds, iop->i_cmd, NCMDS);
#else
		printf("iph0: cmd timeout, cmd=%x\n", iop->i_cmd);
#endif
error:
	timo = *IPR0;
	CLEAR();
	splx(s);
	return 1;
}

/*
** ipinit()
** Initialize the blasted controller for the individual drives associated with
** the controller.
*/
ipinit(unit, skew, gap1, gap2, groupsize, ilv, heads, sectors)
	int unit, skew, gap1, gap2, ilv, heads, sectors;
{
	register iopb_t *iop = &ipsoftc.sc_iopb;
	register uib_t *uib = &ipsoftc.sc_uib;
	long uib_mbva;
	int x;

	uib_mbva = mbmapkget((caddr_t)uib, (long)sizeof(uib_t));
	iop->i_unit = unit;
	iop->i_bufh = HB(uib_mbva);
	iop->i_bufm = MB(uib_mbva);
	iop->i_bufl = LB(uib_mbva);
	iop->i_head = iop->i_cylh = iop->i_cyll = iop->i_sech =
		iop->i_secl = iop->i_scch = iop->i_sccl = 0;
	/*
	** Set up some of the IOPB which will not change.
	*/
	iop->i_rell = iop->i_relh = 0;
	iop->i_linkl = iop->i_linkh = iop->i_linkm = 0;
	iop->i_dmacount = IP_DMACOUNT;
	iop->i_ioh = MB(ipsoftc.sc_ioaddr - MBIO_VBASE);
	iop->i_iol = LB(ipsoftc.sc_ioaddr - MBIO_VBASE);
	iop->i_option = (O_IOPB | O_BUF);

	uib->u_skew = skew;
	uib->u_gap1 = gap1;
	uib->u_gap2 = gap2;
	uib->u_hds = heads;
	uib->u_spt = sectors;
	uib->u_bpsl = LB(BLK_SIZE);
	uib->u_bpsh = MB(BLK_SIZE);
	uib->u_ilv = (ilv | IP_GROUPENABLE | IP_CACHEENABLE);
	uib->u_retry = IP_RETRY;
	uib->u_eccon = uib->u_reseek = uib->u_inchd = 1;
	uib->u_mvbad = uib->u_dualp = uib->u_intron = 0;
	uib->u_group = groupsize;
	uib->u_resv1 = uib->u_resv2 = uib->u_resv3 = 0;

	x = ipcmd(C_INIT, IP_WAIT);
	DELAY(10000);			/* wait for interrupt */
	mbmapkput(uib_mbva, (long)sizeof(uib_t));
	return (x);
}

/*
** Used by unix to print a diagnostic
*/
ipprint(dev, str)
char *str;
{
	printf("%s on ip%d%c\n", str, D_DRIVE(dev), 'a'+D_FS(dev));
}
