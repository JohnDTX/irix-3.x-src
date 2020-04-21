/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/std.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:51 $
 */
/*
** std.c	- Copyright (C) JCS Computer Services - Sunnyvale CA 94089
**		- chase bailey - December 1984
**		- Any use, copy or alteration is strictly prohibited
**		- unless authorized in writing by JCS Computer Services.
**
**	$Source: /d2/3.7/src/stand/lib/dev/RCS/std.c,v $
**	$Revision: 1.1 $
**	$Date: 89/03/27 17:14:51 $
*/
/*
** Modification History --
**	Started on 12/28/84
*/

#define KERNEL 

#ifdef UNIX
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
#include "../machine/cpureg.h"
#include "../multibus/stdreg.h"
#include "../multibus/mbvar.h"
#include "../multibus/stduib.h"
#else
#include "stand.h"
#include "mbenv.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/iobuf.h"
#include "sys/dkerror.h"
#include "sys/sysmacros.h"
#include "dklabel.h"
#include "stdreg.h"
#include "stduib.h"
#include "cpureg.h"
#include "ctype.h"
#include "dprintf.h"
#endif

#define MBREG	0x7050	/* XXX - have this passed	*/
#define MBREG2	0x73f8	/* XXX - have this passed	*/
#define NSD	2
#define NSTD	1
#define WAIT	1	/* polled operation	*/
#define NOWAIT	0	/* interrupt operation	*/
#define FLPUNIT	2

/* XXX retch and vomit */
#ifdef	juniper
#define	SPINCOUNT	2500
#else
#define	SPINCOUNT	250
#endif

long	spinbusy = SPINCOUNT;
long	dmacount = 8;
extern	struct dkerror sterrs[];

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
	{ 0,	0},
};
#define	NERRS	(sizeof(sterrs) / sizeof(struct dkerror))

int	stdstrategy();
int	stdprobe(), stdattach(), stdstart();
struct	mb_device *stddinfo[NSD];
struct	mb_ctlr *stdcinfo[NSTD];

iopb_t	*iopb_mbva;			/* multibus address of our iopb */
uib_t	*uib_mbva;			/* multibus address of our uib */
char	*stdbuffer;
struct	buf stdbuf;
u_char  stdrootslice;

extern char *mbmalloc();


/*
** stdprobe:
**	- probe the controller, and see if its there
**	- initialize it, to get things going
**	- remember that the port address has to be byte swapped 
**	- read in controller firmware version and print it out
*/
stdprobe(reg,reg2)
int reg, reg2;
{
	struct buf *bp = &stdbuf;
	register iopb_t *iop = stdsoftc.sc_iopb;
	long timeout;

	/* probe the two possible board positions */
	stdsoftc.sc_ioaddr = (caddr_t)MBIO_VBASE + reg;
	if ( !probewrite(STR0,sizeof (char), ST_RESET | ST_NOINTERRUPT) ) {
		stdsoftc.sc_ioaddr = (caddr_t)MBIO_VBASE + reg2;
		if ( !probewrite(STR0, ST_RESET | ST_NOINTERRUPT) )
			return(CONF_FAULTED);
	}

	DELAY(50000);
	CLEAR();
	DELAY(50000);
	timeout = 1000000;
	while ((STATUSREG() & 1) && --timeout)
		;
	if (timeout == 0)
		return CONF_DEAD;
	stdsoftc.sc_flags = SC_ALIVE;

	if ( stdbuffer == 0 )
		stdbuffer = mbmalloc(MAXBSIZE+sizeof (struct iopb) 
						+ sizeof (struct uib) + 0x8 );
	dprintf(("stdbuffer is 0x%x\n",stdbuffer));
	uib_mbva = (uib_t *)(stdbuffer + MAXBSIZE);
	iopb_mbva = (iopb_t *)(uib_mbva + sizeof ( struct uib ));
	stdsoftc.sc_iopb = iopb_mbva;
	stdsoftc.sc_uib = uib_mbva;
	bp->b_iobase = stdbuffer;

	/*
	** Set up the portions of the IOPB which will not change
	*/
	iop->i_rell = iop->i_relh = 0;
	iop->i_linkl = iop->i_linkh = iop->i_linkm = 0;
	iop->i_dmacount = dmacount;
	iop->i_ioh = MB(stdsoftc.sc_ioaddr);
	iop->i_iol = LB(stdsoftc.sc_ioaddr);
	iop->i_option = O_OPTIONS;
	iop->i_unit = 0;

	/*
	** Set up the iopb address in the I/O of the Controller
	*/
	*STR1 = HB(iopb_mbva);
	*STR2 = MB(iopb_mbva);
	*STR3 = LB(iopb_mbva);
	DELAY(5000);

	/*
	** Now ask controller for its firmware revision information.
	** If this doesn't work, mark controller as non-functioning.
	*/
	if (stdcmd(C_REPORT, WAIT, 0) == 0)
		dprintf(("(rev %c.%c, rel %c) ",
			((iop->i_error & 0xF0) >> 4) + '0',
			(iop->i_error & 0xF) + '0',
			iop->i_unit ? iop->i_unit : '0'));
	else
		return(CONF_DEAD);
	return(CONF_ALIVE);
}

/*
** stdattach: disk -- attach a disk to its controller
**	- configure to 1 cyl, 1 head and 64 sectors
**	- read the label
**	- reconfigure to cyl/hd/sec in label
**	- stash file system info into static structure
*/
stdattach(unit)
{
	register struct disk_label *l;
	register iopb_t *iop = stdsoftc.sc_iopb;
	register struct softc_disk *sd;
	register struct buf *bp = &stdbuf;
	int result;
	int i, j, diskflag;
	uib_t	*uibs[3];

	if ( unit > NSD )
		return(CONF_FAULTED);

	/*
	** Configure to read label. Try hesdi, sesdi, then st 506
	*/
	uibs[0] = &sii_hesdi_uib;
	uibs[1] = &sii_sesdi_uib;
	uibs[2] = &sii_st506_uib;

	diskflag = 0;
	for ( i = 0, j = 0; i < 3; i++) {
fuckingmaxtor:
		if (stdinit(unit, uibs[i]))
			return(CONF_FAULTED);
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
		if (!stdcmd(C_READNOCACHE, WAIT, unit)) {
			diskflag = 1;
			break;
		} else {		/* restore the drive	*/
			if ( iop->i_error == 0x1B ) 	/* no drive */
				return(CONF_FAULTED);
			streset();
			delay_sec(4);
			/* This is for the fucking maxtor: */
			if ( i == 2 && j++ < 2 )
				goto fuckingmaxtor;
		}
	}
	if ( !diskflag )
		return(CONF_FAULTED);

	/*
	** Now examine the label and insure its good.  If so, then
	** initialize the drive again using the parameters from the
	** label.  If the label is incorrect, then reject the drive.
	*/
	l = (struct disk_label *)stdbuffer;
	if (l->d_magic != D_MAGIC) {
		dk_prname((struct disk_label *)NULL);
		return(CONF_FAULTED);
	}
	sd = &stdsoftc.sc_disk[unit];
	std_build_uib(l, uib_mbva);
	if (stdinit(unit, uib_mbva))
		return(CONF_FAULTED);

	/* now do a restore as a safety factor for the fujitsu drives */
	if (stdcmd(C_RESTORE, WAIT, unit))
		return(CONF_FAULTED);

	/*
	** save label info and init drive software state
	*/
	sd->sc_flags = SC_ALIVE;
	sd->sc_cyl = l->d_cylinders;
	sd->sc_hd  = l->d_heads;
	sd->sc_sec = l->d_sectors;
	sd->sc_spc = sd->sc_hd * sd->sc_sec;
	bcopy((caddr_t)l->d_map, (caddr_t)sd->sc_fs,
	      (long) sizeof(sd->sc_fs));
	stdrootslice = (l->d_rootnotboot) ? l->d_rootfs : l->d_bootfs;
	return(CONF_ALIVE);
}

streset()
{
	int timeout;

	RESET();
	timeout=200000;
	while(--timeout);		/* Delay a little while */
	ZEROREG0();			/* Write a 0x00 to Register 0 */
	timeout = 1000000;
	/* Wait for the done bit to go away */
	while ((STATUSREG() & ST_DONE) && --timeout) ;
	if (!timeout)
		return CONF_DEAD;
	timeout = 1000000;
	/* Wait for the done bit to come true */
	while (((STATUSREG() & ST_DONE) == 0) && --timeout);
	if (!timeout)
		return CONF_DEAD;
	CLEAR();			/* Write a 0x02 to Register 0 */
	timeout=200000;
	while(--timeout);		/* Delay a little while */
	CLEARREG4();			/* Clear the tape side registers also */
	timeout = 1000000;
	while ((STATUSREG() & ST_DONE) && --timeout);
	if (timeout == 0)
		return CONF_DEAD;
}

/*
 * sdname:
 *	- return an ascii representation for the drive name
 */
char _sdname_name[5];	/* MUSTN'T BE STATIC in PROM */

char *
sdname(dev)
	dev_t dev;
{

	_sdname_name[0] = 's';
	_sdname_name[1] = 'd';
	_sdname_name[2] = D_DRIVE(dev) + '0';
	_sdname_name[3] = D_FS(dev) + 'a';
	_sdname_name[4] = '\0';
	return _sdname_name;
}

/*
** stdopen - Called upon the open to check some very basic
**	     things About the disk
*/
/*ARGSUSED*/
stdopen(io, ext, file)
register struct iob *io;
register char *ext;
char *file;
{
	register struct inode *ip;
	register int dev;
	register int drive;	/* the drive number	*/
	register int slice;	/* the slice		*/
	char c;

	ip = io->i_ip;

	dprintf(("stdopen:(%s)\n",ext));
	/* probe to see if device is there		*/
	if ( stdprobe(MBREG,MBREG2) != CONF_ALIVE )
		goto error;

	/* determine drive and filesystem from extension */
	if ( (drive = exttodrive( ext )) < 0 )
		goto error;
	if ( drive >= MAX_WINNYS )
		goto error;
	/* now attach the drive */
	if ( stdattach(drive) != CONF_ALIVE )
		goto error;
	if ( (dev = exttodev(ext,stdrootslice)) < 0 )
		goto error;
	ip->i_dev |= dev;

	return(0);
error:
	io->i_error = ENXIO;
	return(-1);
}

/* clear any interrupts */
stdclose()
{
}

stdioctl()
{
}

/*
** stdstrategy(bp) - Called to do the first checking on the disk request and
*/
stdstrategy(iop, flag)
register struct iob *iop;
{
	register struct buf *bp;
	register struct disk_map *fs;
	register short unit = D_DRIVE(bp->b_dev);
	register struct softc_disk *sd;
	register int s;
	register daddr_t bn;
	register short temp;

	bp = iop->i_bp;
	bp->b_flags = flag;
	sd = &stdsoftc.sc_disk[unit];
	fs = &sd->sc_fs[D_FS(bp->b_dev)];
	if ((bp->b_bcount == 0) || (bp->b_bcount & (BLKSIZE - 1))) {
		dprintf(("std count fuck up"));
		return( -1 );
	}

	/* crunch disk address for start and disksort */
	sd->sc_bn = bn = bp->b_iobn + fs->d_base;
	bp->b_cyl = bn / sd->sc_spc;
	temp = bn % sd->sc_spc;
	bp->b_head = temp / sd->sc_sec;
	bp->b_sector = temp % sd->sc_sec;

	dprintf(("stdstrategy unit:bn:cyl:hd:sec 0x%x:0x%x:0x%x:0x%x:0x%x\n",
		unit,bn,bp->b_cyl,bp->b_head,bp->b_sector));

	s = SPL();
	stdsoftc.sc_tab.b_active = 0;
	stdsoftc.sc_tab.b_actf = bp;
	if (stdsoftc.sc_tab.b_active == 0) {
		stdstart();
	}
	splx(s);
	
	if ( bp->b_flags & B_ERROR ) {
		bp->b_error = EIO;
		return ( -1 );
	} else { 
		return ( bp->b_bcount );
	}
}

stdstart()
{
	register iopb_t *iop;
	register struct buf *bp = &stdbuf;
	register struct iobuf *dp;
	register int unit;
	register u_long bn;

	dp = &stdsoftc.sc_tab;
top:
	if ((bp = dp->b_actf) == 0)
		return;
	unit = D_DRIVE(bp->b_dev);
	dp->b_active = 1;
	dp->io_unit = unit;

	/*
	** Set up the IOPB
	*/
	iop = stdsoftc.sc_iopb;
	iop->i_unit = unit;
	iop->i_head = bp->b_head;
	iop->i_cylh = MB(bp->b_cyl);
	iop->i_cyll = LB(bp->b_cyl);
	iop->i_sech = MB(bp->b_sector);
	iop->i_secl = LB(bp->b_sector);
	bn = bp->b_bcount >> BLKSHIFT;
	iop->i_scch = MB(bn);
	iop->i_sccl = LB(bn);
	bn = (u_long)bp->b_iobase;
	iop->i_bufh = HB(bn);
	iop->i_bufm = MB(bn);
	iop->i_bufl = LB(bn);
	dprintf(("stdstart: unit:head:cyl:sec:cnt:buf 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x\n",
		iop->i_unit,iop->i_head,(iop->i_cylh<<8)|(iop->i_cyll),
		(iop->i_sech<<8)|(iop->i_secl),(iop->i_scch<<8)|(iop->i_sccl),
		(iop->i_bufh<<16)|(iop->i_bufm<<8)|(iop->i_bufl) ));
	/*
	** If command won't start, return an error
	*/
	if (stdcmd((bp->b_flags & B_READ) ? C_READ : C_WRITE, WAIT, unit)) {
		dprintf(("%s: can't start command\n", sdname(bp->b_dev)));
		bp->b_flags |= B_ERROR;
		dp->b_actf = bp->av_forw;
		dp->b_errcnt = 0;
		iodone(bp);
		goto top;
	}

}

stdcmd(cmd, waiting, unit)
	int cmd, waiting, unit;
{
	register iopb_t *iop = stdsoftc.sc_iopb;
	register long timeout;
	register int s;
	register long i;

	dprintf(("stdcmd 0x%x\n",cmd));
	s = spl6();
	timeout = 50000;
	while (((i = STATUSREG()) & ST_BUSY) && --timeout)
		;
	if (timeout <= 0) {
		dprintf(("ist%d: timeout, idle controller, status=%x\n",
			       unit, i));
		return (1);
	}
	iop->i_cmd = cmd;
	iop->i_status = 0;
	iop->i_error = 0;
	if (waiting == 0) {
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
			dprintf(("[sd%d: ERROR(%x) Cmd=%x status %x err %s] ",
				       unit, iop->i_error, iop->i_cmd,
				       iop->i_status,
				       dkerror((u_char)iop->i_error,
					       sterrs, NERRS)));
			CLEAR();
			splx(s);
			return (1);
		}
		if ((--timeout) == 0) {
			dprintf(("[sd%d: Cmd=%x Timeout: s %x err %x] ", unit,
				       iop->i_cmd, iop->i_status,
				       iop->i_error));
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
	register iopb_t *iop = stdsoftc.sc_iopb;
	register uib_t *nuib = stdsoftc.sc_uib;
	int x;

	/* copy uib */
	*nuib = *uib;

	dprintf(("stdinit of drive type "));
	switch ( nuib->u_ddb ) {
	case UIB_VERTEX:
		dprintf(("ST506\n"));
		break;
	case UIB_HITACHI:
		dprintf(("Hard ESDI\n"));
		break;
	case UIB_MAXTOR:
		dprintf(("Soft ESDI\n"));
		break;
	default:
		dprintf(("0x%x\n",nuib->u_ddb));
	}

	iop->i_unit = unit;
	iop->i_bufh = HB(uib_mbva);
	iop->i_bufm = MB(uib_mbva);
	iop->i_bufl = LB(uib_mbva);
	iop->i_head = iop->i_cylh = iop->i_cyll = iop->i_sech =
		iop->i_secl = iop->i_scch = iop->i_sccl = 0;

	x = stdcmd(C_INIT, WAIT, unit);
	return x;
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
	uib->u_retry = STRETRIES;
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

stfopen(io, ext, file)
register struct iob *io;
register char *ext;
char *file;
{
	register struct inode *ip;
	register uib_t  *uip;
	register struct softc_disk *sd;
	register int drive;	/* the drive number	*/
	register int slice;	/* the slice		*/
	char c;

	ip = io->i_ip;

	dprintf(("stfopen:(%s)\n",ext));
	/* probe to see if device is there		*/
	if ( stdprobe(MBREG,MBREG2) != CONF_ALIVE )
		goto error;

	/* determine drive from extension */
	/* there is only one filesystem on a floppy */
	if ( (drive = exttodrive( ext )) < 0 )
		goto error;
	if ( drive >= MAX_FLOPPYS )
		goto error;

	drive += FLPUNIT;
	ip->i_dev |= (drive << 3);
	dprintf(("dev is 0x%x\n",ip->i_dev));


	/* now attach the drive */
	uip = &sii_flp_uib;
	if ( stdinit( drive, uip ) )
		goto error;

	/* now set up the drive software state */
	sd = &stdsoftc.sc_disk[drive];
	sd->sc_flags = SC_ALIVE;
	sd->sc_cyl = uip->u_nch << 8 | uip->u_ncl;
	sd->sc_hd  = uip->u_hds;
	sd->sc_sec = uip->u_spt;
	sd->sc_spc = sd->sc_hd * sd->sc_sec;
	sd->sc_fs[0].d_base = 0;
	sd->sc_fs[0].d_size = sd->sc_spc * sd->sc_cyl;

	if (stdcmd(C_RESTORE, WAIT, drive))
		goto error;

	return(0);

error:
	io->i_error = ENXIO;
	return (-1);
}

stfclose()
{
}

stfioctl()
{
}

/*
** stfstrategy(bp) - Called to do the first checking on the disk request and
*/
stfstrategy(io, flag)
register struct iob *io;
{
	return(stdstrategy(io, flag));
}
