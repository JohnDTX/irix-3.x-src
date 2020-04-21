/*
* $Source: /d2/3.7/src/stand/lib/dev/RCS/iph.c,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:14:34 $
*/

#define KERNEL

#include "stand.h"
#include "mbenv.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/iobuf.h"
#include "sys/dkerror.h"
#include "sys/sysmacros.h"
#include "dklabel.h"
#include "iphreg.h"
#include "cpureg.h"
#include "dprintf.h"


#define MBREG	0x7010	/* XXX - have this passed	*/
#define NIP	4
#define NIPH	1
#define WAIT	1	/* polled operation	*/
#define NOWAIT	0	/* interrupt operation	*/

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
	{ 0x00, 0x00 },
};

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
	{ 0x00, 0x00 },
};
#endif

struct	mb_device *ipdinfo[NIP];
struct	mb_ctlr *ipcinfo[NIPH];

iopb_t	*ipiopb_mbva;			/* multibus address of our iopb */
uib_t	*ipuib_mbva;			/* multibus address of our uib */
char	*ipbuffer;
struct	buf ipbuf;
u_char	iprootslice;

extern char *mbmalloc();

ipopen( io, ext, flag )
register struct iob *io;
char *ext;
int flag;
{
	register struct inode *ip;
	register int dev;
	register int drive;	/* the drive number	*/
	register int slice;	/* the slice		*/
	char c;

	ip = io->i_ip;

	dprintf(("ipopen:(%s)\n",ext));
	/* probe to see if device is there		*/
	if ( ipprobe(MBREG) != CONF_ALIVE )
		goto error;

	/* determine drive and filesystem from extension */
	if ( (drive = exttodrive( ext )) < 0 )
		goto error;
	if ( drive >= MAX_DRIVES )
		goto error;
	/* now attach the drive */
	if ( ipattach(drive) != CONF_ALIVE )
		goto error;
	if ( (dev = exttodev(ext,iprootslice)) < 0 )
		goto error;
	ip->i_dev |= dev;

	return(0);
error:
	io->i_error = ENXIO;
	return(-1);
}


/* 
 * close - must clear interrupts
*/
ipclose(io)
register struct iob *io;
{
	dprintf(("ipclose\n"));
	delay_ms(2);
	CLEAR();
	delay_ms(2);
}

ipioctl()
{
}

/*
** ipprobe:
**	- probe the controller, and see if its there
**	- initialize it, to get things going
**	- remember that the port address has to be byte swapped 
*/
ipprobe(reg)
int reg;
{
	struct buf *bp = &ipbuf;

	ipsoftc.sc_ioaddr = (caddr_t)MBIO_VBASE + reg;

	if ( !probewrite(IPR0, sizeof (char), IP_CLEAR) )
		return(CONF_FAULTED);

	DELAY(10);
	ipsoftc.sc_flags = SC_ALIVE;

	if ( ipbuffer == 0 )
		ipbuffer = mbmalloc(MAXBSIZE+sizeof (struct iopb) 
						+ sizeof (struct uib) + 0x8 );
	dprintf(("ipbuffer is 0x%x\n",ipbuffer));
	ipuib_mbva = (uib_t *)(ipbuffer + MAXBSIZE);
	ipiopb_mbva = (iopb_t *)(ipuib_mbva + sizeof ( struct uib ));
	ipsoftc.sc_uib = ipuib_mbva;
	ipsoftc.sc_iopb = ipiopb_mbva;
	ipsoftc.sc_iopb_mbva = (long)ipiopb_mbva;
	bp->b_iobase = ipbuffer;

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
ipattach(unit)
register short unit;
{
	register struct softc_disk *sd;
	register struct buf *bp;
	register struct disk_label *l;
	register iopb_t *iop = ipsoftc.sc_iopb;

	/*
	 * Configure to read label
	 * ipinit(unit, skew, gap1, gap2, groupsize, ilv, heads, sectors)
	 */
	bp = &ipbuf;
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
	l = (struct disk_label *)bp->b_iobase;
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
	iprootslice = (l->d_rootnotboot) ? l->d_rootfs : l->d_bootfs;
	return CONF_ALIVE;

error:
	return CONF_FAULTED;
}

/*
** ipstrategy(bp) - Called to do the io
*/
ipstrategy(io,flag)
register struct iob *io;
int flag;
{
	register struct buf *bp = io->i_bp;
	register struct disk_map *fs;
	register short unit = D_DRIVE(bp->b_dev);
	register struct softc_disk *sd;
	daddr_t bn;
	short temp;

	dprintf(("ipstrategy: unit %d, blk 0x%x\n",unit,bp->b_iobn));
	sd = &ipsoftc.sc_disk[unit];
	fs = &sd->sc_fs[D_FS(bp->b_dev)];
	bp->b_flags = flag;

	/* crunch disk address for start and disksort */
	bn = bp->b_iobn + fs->d_base;
	bp->b_cyl = bn / sd->sc_spc;
	temp = bn % sd->sc_spc;
	bp->b_head = temp / sd->sc_sec;
	bp->b_sector = temp % sd->sc_sec;

	ipsoftc.sc_tab.b_actf = bp;
	ipstart();

	if ( bp->b_flags & B_ERROR ) {
		bp->b_error = EIO;
		return(-1);
	} else {
		return(bp->b_bcount);
	}
}

/*
** ipstart()
** Now set up the iopb and start the request using the bp pointed to by the
** ipsoftc.sc_table.
*/
ipstart()
{
	register iopb_t *iop = ipsoftc.sc_iopb;
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

	dprintf(("ipstart: unit %d, base 0x%x, cnt %d\n",unit, bp->b_iobase, bp->b_bcount));
	dprintf(("ipstart: head %d, cyl 0x%x, sector %d\n",bp->b_head, bp->b_cyl, bp->b_sector));
	dprintf(("multibus address 0x%x%x,%x\n",iop->i_bufh&0xff,iop->i_bufm&0xff,iop->i_bufl&0xff));
	/* fire off command */
	(void) ipcmd((bp->b_flags & B_READ) ? C_READ : C_WRITE, IP_WAIT);
}


ipcmd(cmd, waiting)
int cmd, waiting;
{
	register iopb_t *iop = ipsoftc.sc_iopb;
	register long timo;
	register long timo2;
	register int s;

	dprintf(("ipcmd: cmd (0x%x) %s\n",cmd,dkerror(cmd, ipcmds)));
	s = spl6();
	timo = 200000;
	while ((*IPR0 & IP_BUSY) && --timo)
		;
	if (timo == 0) {
		dprintf(("iph0: busy timeout\n"));
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
	/* clear the interrupt */
	timo2 = 200000;
	while (!(*IPR0 & IP_DONE) && --timo2)
		;
	CLEAR();
	if (timo) {
		switch (iop->i_status) {
		 case S_OK:
			splx(s);
			return 0;
		 default:
			dprintf(("iph0: strange status %x\n", iop->i_status));
			splx(s);
			return 1;
		 case S_BUSY:
		 case S_ERROR:
			dprintf(("iph0: status=%x error=%x\n",
					      iop->i_status, iop->i_error));
			goto error;
		}	
	}
#ifdef	DEBUG
	dprintf(("iph0: cmd timeout, cmd='%s'\n",
		      dkerror(iop->i_cmd, ipcmds)));
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
	register iopb_t *iop = ipsoftc.sc_iopb;
	register uib_t *uib = ipsoftc.sc_uib;
	int x;

	dprintf(("ipinit: unit %d, heads %d, sectors %d\n",unit,heads,sectors));
	iop->i_unit = unit;
	iop->i_bufh = HB(uib);
	iop->i_bufm = MB(uib);
	iop->i_bufl = LB(uib);
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
	return (x);
}
