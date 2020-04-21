/*
** xy.c		- Copyright (C) Silicon Graphics, Inc; Mountain View, CA
**		- Chase - April 1984
**		- Any use, copy or alteration is strictly prohibited
**		- and morally inexcusable
**		- unless authorized in writing by SGI.
**
**	$Source: /d2/3.7/src/sys/multibus/RCS/xyl.c,v $
**	$Revision: 1.1 $
**	$Date: 89/03/27 17:31:50 $
*/
/*
** Modification History --
**	Chase	April 4, 1984
**	--	Started
*/

#if NXY > 0
#include "../h/param.h"
#include "../h/types.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/iobuf.h"
#include "../h/file.h"
#include "../h/dklabel.h"

#include "../multibus/xyreg.h"
#include "../multibus/mbvar.h"

/* For attach pretty printout */
char *XYtypes[] = {
	"Atasi 3046",	"Vertex V170",	"Fujitsu 2312",	"Fujitsu Eagle 2351",
	"Maxtor 1140",	"New 2",	"New 3",	"New 4"
};

#define b_cylin b_resid
#define WAIT(time)	for(i=time;i;i--)
/** #define DEBUG	/*** DEBUG ***/
#undef DEBUG		/*** DEBUG ***/
#ifdef DEBUG
long xy_debug = 1;
#else
long xy_debug = 0;
#endif

/*
** XY Flags
*/
#define XYINIT 1

#define NOWAIT 1
#define WAITING 0

int	xystrategy();
int	xyprobe(), xyattach(), xystart(), xyintr();
struct	mb_device *xydinfo[NXY];
struct	mb_ctlr *xycinfo[NXYL];
struct	mb_driver xydriver = {
	xyprobe, xyattach, (int (*)())0, xystart, xyintr,
	(char *(*)())0, "xy", xydinfo, "xy", xycinfo,
};

caddr_t	xy_ioaddr;	/* address of i/o port (physical) */
char	xy_inattach;	/* True while in attach */

xy_iopb_t	THEiopb;

/*
** xyprobe:
**	- probe the controller, and see if its there
**	- initialize it, to get things going
**	- remember that the port address has to be byte swapped 
*/
xyprobe(reg)
	int reg;
{
	xy_ioaddr = (caddr_t)MBIOBASE + reg;
	CLEAR();
	return CONF_ALIVE;
}

/*
** xyattach: disk -- attach a disk to its controller
**	- configure to 1 cyl, 1 head and 64 sectors
**	- read the label
**	- reconfigure to cyl/hd/sec in label
**	- stash file system info into static structure
**	- call setroot to see if this drive contains the root
*/
xyattach(mi)
	register struct mb_device *mi;
{
	register struct xyinfo *xy;
	register struct buf *bp;
	register struct disk_label *l;
	register xy_iopb_t *iop;
	register short unit = mi->mi_unit;
	register long s;

	xy = &xydb[unit];
	iop = &THEiopb;
					/* Configure to read label */
	xy_inattach = 1;		/* Quiet errors */
	s = spl6();
	/*
	** xyinit(unit, burstsize, drivetype, ilv, cylinders, heads, sectors)
	*/
	if(xyinit(unit, T_T2, 0, 1, 2, 2, 33)) {
fault:		xy_inattach = 0;
		splx(s);
		return CONF_FAULTED;
	}
	if((bp = geteblk(2)) == 0)	/* Grab a block to read label */
		panic("xyattach: geteblk() failed");
	/*
	** Set up the iopb for the read after the basic init
	*/
	iop->i_drive = unit;
	iop->i_relh = ADDR3(bp->b_un.b_addr);
	iop->i_rell = ADDR2(bp->b_un.b_addr);
	iop->i_bufh = ADDR1(bp->b_un.b_addr);
	iop->i_bufl = ADDR0(bp->b_un.b_addr);
	iop->i_head = iop->i_cylh = iop->i_cyll = iop->i_sector = 0;
	iop->i_scch = 0;
	iop->i_sccl = 33;
	if(xycmd(C_READ, WAITING)) {
		if(xy_debug) printf("xyattach: failed call to xycmd()\n");
oops:		brelse(bp);
		goto fault;
	}
	l = (struct disk_label *)bp->b_un.b_addr;
	if(l->d_magic != D_MAGIC) {
		printf("(***No label***) ");
		goto oops;
	}
	/*
	** xyinit(unit, burstsize, drivetype, ilv, cylinders, heads, sectors)
	*/
	if(xyinit(unit,T_T2,0,l->d_interleave,l->d_cylinders,l->d_heads,l->d_sectors)) {
		goto oops;
	}
					/* Save the file system info */
	xy->xy_cyl = l->d_cylinders;
	xy->xy_hd  = l->d_heads;
	xy->xy_sec = l->d_sectors;
	xy->xy_bpc = xy->xy_hd * xy->xy_sec;
	bcopy(l->d_map, xy->xy_fs, sizeof xy->xy_fs);
	printf("(%s Name: %s) ", XYtypes[l->d_type], l->d_name);
	brelse(bp);
	xy->xy_flags = INITED;

	setroot(&xydriver, mi, mi->mi_unit << 3, 0070, l, xystrategy);
	xy_inattach = 0;		/* Turn errors back on */
	splx(s);
	return CONF_ALIVE;
}

/* ARGSUSED */
/*
** xyopen(dev, flag) - Called upon the open to check some very basic things
**		      - About the disk.
*/
xyopen(dev, flag)
	dev_t dev;
{
	register u_char unit = DRIVE(dev);

	if(unit > MAX_DRIVES) {
		printf("xy%d: not a device\n", unit);
		u.u_error = ENODEV;
		return;
	}
	if(!(xydb[unit].xy_flags & INITED)) {
		printf("xy%d: not initialized\n", unit);
		u.u_error = EIO;
		return;
	}
}

/*
** xystrategy(bp) - Called to do the first checking on the disk request and
**		   - queue up the request via a call to disksort and check
**		   - the active before calling xystart.
*/
xystrategy(bp)
	register struct buf *bp;
{
	register struct xyinfo *xy;
	register struct disk_map *fs;

	xy = &xydb[DRIVE(bp->b_dev)];
	if(!(xy->xy_flags&INITED)) {
err:		bp->b_flags |= B_ERROR;
		bp->b_error = 0;
		bp->b_resid = 0;
		iodone(bp);
		return;
	}
	fs = &xy->xy_fs[FS(bp->b_dev)];		/* File system info */
	if(bp->b_blkno + (bp->b_bcount+(BLKSIZE-1))/BLKSIZE > fs->d_size)
		goto err;			/*  Yes */
	bp->b_cylin = (bp->b_blkno + fs->d_base) / xy->xy_bpc;
	bp->av_forw = 0;
	spl5();
	disksort(&xytab, bp);			/* Queue this request */
	if (xytab.b_active == 0)
		xystart();
	spl0();
}

/*
** xystart()
** Now set up the iopb and start the request using the bp pointed to by the
** xytable.
*/
xystart()
{
	register struct buf *bp;
	register struct xyinfo *xy;
	register struct disk_map *fs;
	register xy_iopb_t *iop = &THEiopb;
	register long cylinder, head;
	register u_char sector;
	register daddr_t bn;

	if((bp = xytab.b_actf) == 0)
		return;
	xy = &xydb[DRIVE(bp->b_dev)];
	fs = &xy->xy_fs[FS(bp->b_dev)];

	bn = bp->b_blkno + fs->d_base;
	cylinder = bn / xy->xy_bpc;
	sector = bn % xy->xy_bpc;
	head = sector / xy->xy_sec;
	iop->i_sector = sector % xy->xy_sec;

	xytab.b_active = 1;
	/*
	** Set up the IOPB
	*/
	iop->i_drive = DRIVE(bp->b_dev);
	iop->i_head = head;
	iop->i_cylh = MB(cylinder);
	iop->i_cyll = LB(cylinder);

	cylinder = (bp->b_bcount + BLKSIZE - 1) >> XY_LOG;
	iop->i_scch = MB(cylinder);
	iop->i_sccl = LB(cylinder);

	head = (u_long)(bp->b_un.b_addr);
	iop->i_relh = ADDR3(head);
	iop->i_rell = ADDR2(head);
	iop->i_bufh = ADDR1(head);
	iop->i_bufl = ADDR0(head);

	if(bp->b_flags & B_READ)
		xycmd(C_READ, NOWAIT);
	else
		xycmd(C_WRITE, NOWAIT);
}

xyread(dev)
	dev_t dev;
{
	physio(xystrategy, &xyrtab, dev, B_READ);
}

xywrite(dev)
	dev_t dev;
{
	physio(xystrategy, &xyrtab, dev, B_WRITE);
}

xyintr()
{
	register xy_iopb_t *iop = &THEiopb;
	register struct buf *bp;
	register short status;
	register i;

	CLEAR();
	if(xytab.b_active == 0) {
		if(xy_inattach) return;
		printf("xyintr: xytab.b_active == 0\n");
		xypp();
		return;
	}
	if(((status = iop->i_status1) & S_DONE) == 0) {
		if(xy_debug) printf("@ ");
		xypp();
		return;
	}
	bp = xytab.b_actf;
	if(bp == NULL) panic("xyintr: Null buffer\n");
	/*
	** You know the operation finished from the DONE bit above, so
	** Check to see if you had an error dummy.
	*/
	if((iop->i_status1 & S_ERROR) || ((XYSTATUS()) & CSR_DOUBLEERROR)) {
		if(++xytab.b_errcnt <= XY_RETRY) {
			if(xy_debug) printf("!");
			if(xy_debug) xypp();
			xystart();
			return;
		}
		xyerror(iop->i_cmd);
		bp->b_flags |= B_ERROR;
	}
	xytab.b_actf = bp->av_forw;
	xytab.b_errcnt = 0;
	xytab.b_active = 0;
	bp->b_resid = 0;
	iodone(bp);
	xystart();
}

xycmd(cmd, waiting)
	long cmd, waiting;
{
	register xy_iopb_t *iop = &THEiopb;
	register long timeout = 20000;
	register long s,i;

	s = spl6();
	/* 
	** Set iopb to be updated and use 24 bit addressing
	** Make sure the status bytes are zero.
	*/
	while(--timeout) {
		if(((XYSTATUS()) & CSR_GOBUSY) == 0)
			break;
	}
	if(timeout == 0) {
		if(xy_debug)
			printf("xycmd: timeout waiting for the controller\n");
		splx(s);
		return 1;
	}
	iop->i_cmd = (cmd | C_RELOCATION | C_IOPBUPDATE);
	iop->i_status1 = 0;
	iop->i_status2 = 0;
#ifdef NOTDEF
	if(xy_debug) xypp();
#endif
	if(waiting == NOWAIT) {
		iop->i_imode |= IM_ENABLEINT;
		iop->i_cmd |= C_INTERRUPT;
		START();
		splx(s);
		return 0;
	}
	START();
	/*
	** Otherwise, Wait for the command to complete
	*/
	timeout = 100000;
	while((((i = iop->i_status1) & S_DONE) == 0) && --timeout) ;
	if(timeout == 0) {
		if(xy_debug)
			printf("xycmd: timeout on command: %s status: %x\n",
				xycmdlist[cmd&0x0f], i);	
		splx(s);
		return 1;
	}
	/*
	** Check to see if you had an error dummy.
	*/
	if((iop->i_status1 & S_ERROR) || ((XYSTATUS()) & CSR_DOUBLEERROR)) {
		xyerror(cmd);
		splx(s);
		return 1;
	}
	return 0;
}

/*
** xyinit()
** Initialize the blasted controller for the individual drives associated with
** the controller.
*/
xyinit(unit, burstsize, drivetype, ilv, cylinders, heads, sectors)
	u_short unit, burstsize, drivetype, ilv, cylinders, heads, sectors;
{
	register xy_iopb_t *iop = &THEiopb;
	register long i;
	register timeout = 20000;

	RESET();
	WAIT(20);
	while(--timeout) {
		if(((XYSTATUS()) & CSR_GOBUSY) == 0)
			break;
	}
	if(timeout == 0) {
		if(xy_debug)	
			printf("xyinit: waiting for the controller\n");
		return 1;
	}
	CLEAR();
	/*
	** Load registers to point to the iopb
	*/
	xyregload(&THEiopb);
	WAIT(20);
	/*
	** Describe drive params: # sectors, heads, and cylinders
	** And set a default iopb for later use.
	** note:  The xy450 lets you have up to 4 different drive types at
	**  one time.  Thus the config (or 'drive type') command, associates
	**  # sectors, heads, and cylinders with a drive type number.  The
	**  drive type number is 0..3.  Then, when you give a command, you
	**  set the drive type according to which parameters you want to 
	**  associate with.  FOR NOW, we just let xycmd always choose drive
	**  type 0.  Thus, when this 'config' command is done, we are setting
	**  up the parameters of drive type 0.
	*/
	iop->i_imode = IM_RETRY;
	iop->i_cmd = 0;
	iop->i_status2 = 0;
	iop->i_status1 = 0;
	iop->i_drive = (unit | (drivetype<<6));
	if(ilv == 0) i = 0;
	else i = ilv - 1;
	iop->i_throttle = ((burstsize | (i<<3)) & 0x7f);
	/*
	** Controller wants the cyl/hd/sec to be as counted from 0
	*/
	iop->i_sector = (sectors - 1);
	iop->i_head = (heads - 1);
	iop->i_cylh = MB(cylinders - 1);
	iop->i_cyll = LB(cylinders - 1);
	iop->i_scch = 0;
	iop->i_sccl = 0;
	iop->i_bufh = 0;
	iop->i_bufl = 0;
	iop->i_relh = 0;
	iop->i_rell = 0;
	iop->i_headoffset = 0;
	iop->i_reserved = 0;
	iop->i_nextioph = 0;
	iop->i_nextiopl = 0;
	iop->i_eccmaskh = 0;
	iop->i_eccmaskl = 0;
	iop->i_eccaddrh = 0;
	iop->i_eccaddrl = 0;

	if(xycmd(C_SETSIZE, WAITING)) {
		if(xy_debug)
			printf("xyinit: Initialization of Disk %x Failed\n",
				iop->i_drive);
		return 1;
	}
	if(xycmd(C_RESET, WAITING)) {
		if(xy_debug) printf("xyinit: rezero drive failed\n");
		return 1;
	}
	return 0;
}

#ifdef DEBUG
xypp()
{
	register xy_iopb_t *iop = &THEiopb;

	if(!xy_debug) return 0;
	printf("iopb: u:%d c(%x):%s a:%x i:%x s:%x",
		iop->i_drive, iop->i_cmd, xycmdlist[iop->i_cmd&0x0f],
		(iop->i_bufl | (iop->i_bufh<<8) | (iop->i_rell<<16)),
		iop->i_imode, iop->i_status1);
	if(iop->i_status2)
		printf(" e(%x):%s", iop->i_status2, xyerrlist[iop->i_status2]);
	else
		printf(" e:%x", iop->i_status2);
	printf(" %d/%d/%d scc:%d t:%d\n",
		(iop->i_cyll | (iop->i_cylh<<8)), iop->i_head,
		iop->i_sector,
		(iop->i_sccl |(iop->i_scch<<8)), iop->i_throttle);
}
#else
xypp()
{
}
#endif

/*
** Regload or load up the four registers with the IOPB Address
** The WAIT() is a timeout problem with the board for more information
** refer to section 2.3.3 of the Xylogics Manual.
*/
xyregload(addr)
	xy_iopb_t *addr;
{
	register long i;

	WAIT(20);
	(*((char *)(xy_ioaddr+XY_R0))) = ADDR2(addr);
	WAIT(20);
	(*((char *)(xy_ioaddr+XY_R1))) = ADDR3(addr);
	WAIT(20);
	(*((char *)(xy_ioaddr+XY_R2))) = ADDR0(addr);
	WAIT(20);
	(*((char *)(xy_ioaddr+XY_R3))) = ADDR1(addr);
	WAIT(20);
}

/*
** Added for God knows what right now 
*/
xyprint(dev, str)
char *str;
{
	printf("%s on xy%d, slice %d\n", str, DRIVE(dev), FS(dev));
}

xyerror(cmd)
	register u_char cmd;
{
	register xy_iopb_t *iop = &THEiopb;
	/*
	** Add checking for hard or soft error and whether to do retry
	*/
	if(xy_inattach) return;
	printf("\nXylogics 450 Error Code(%x): %s Cmd(%x): %s Drive: %d\n",
		iop->i_status2, xyerrlist[iop->i_status2],
		cmd, xycmdlist[cmd&0x0f], iop->i_drive);
	return;
}
#endif
