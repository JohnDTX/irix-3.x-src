/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/md.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:39 $
 */

#include "stand.h"
#include "mbenv.h"

#include "sys/types.h"
#include "ctype.h"
#define KERNEL
#include "dklabel.h"
#undef KERNEL
#include "md.h"
#include "sysmacros.h"
#include "dprintf.h"



int mdinited;		/* flag if any of the devices have been openned */
int drivesopen;		/* bit mask for openned drives			*/
int retries;
long	md_blkno;
char *privatebuffer;		/* used during initialization	*/
char *mdbuffer;
char *mdvarp;
u_char mdrootslice;

/* room for disk labels:	*/
struct disk_label hlab[D_NWIN];		/* winchester labels	*/
struct disk_label flab[D_NFLOP];	/* floppy labels	*/

extern long SwapW();

/* ST506 and Floppy disk drivers		*/

/*	entry points for stand alone  io	*/
mdopen( io, ext, file )
register struct iob *io;
char *ext, *file;
{
	register struct inode *ip;
	register int dev;
	register int drive;	/* the drive number	*/
	register int slice;	/* the slice		*/
	char c;

	ip = io->i_ip;

	dprintf(("mdopen:(%s)\n",ext));
	/* probe to see if device is there		*/
	if ( mdprobe() != CONF_ALIVE ) 
		goto error;

	/* determine drive and filesystem from extension */
	if ( (drive = exttodrive( ext )) < 0 )
		goto error;
	if ( drive >= D_NWIN )
		goto error;

	dev = drive << 3;

	/* now attach the drive */
	if ( !mdinited ) {
		if ( minit() < 0 ) {
			dprintf(("minit failed\n"));
			goto error;
		}
	}

	/* re-configure according to the label	*/
	if ( ISMDDISK(dev) && !(drivesopen & (1<<D_DRIVE(dev))) ) {
		if ( mconfig(dev) < 0 ) {
			dprintf(("mconfig failed\n"));
			goto error;
		}
	}

	if ( (dev = exttodev(ext,mdrootslice)) < 0 )
		goto error;

	ip->i_dev |= dev;
	dprintf(("dev is 0x%x\n",dev));

	return(0);

error:
	mdinited = 0;
	io->i_error = ENXIO;
	return (-1);
}

mdprobe()
{
	if ( !probewrite(&(MDIOADDR->md_ptr0), sizeof (char), mdZero ) ) {
		return(CONF_FAULTED);
	} else {
		return(CONF_ALIVE);
	}
}

mdclose(io)
register struct iob *io;
{
}

mdioctl()
{
}

mdstrategy(io,flag)
register struct iob *io;
int flag;
{
	register struct buf *bp;
	register int bn;

	bp = io->i_bp;
	bn = bp->b_iobn;

	dprintf(("mdstrategy %s:dev:blk:addr:size 0x%x:0x%x:0x%x:0x%x\n",
		(flag==READ?"r":"w"),bp->b_dev, bn,bp->b_iobase,bp->b_bcount ));
	
	if ( flag == READ ) {
		if ( mddev(FCN_READ,bn,bp->b_iobase,minor(bp->b_dev),
						bp->b_bcount) ) 
			bp->b_error = EIO;
	} else {
		if ( mddev(FCN_WRITE,bn,bp->b_iobase,minor(bp->b_dev),
						bp->b_bcount) ) 
			bp->b_error = EIO;
	}

	if ( bp->b_error )
		return(-1);
	else
		return(bp->b_bcount);
}

/* Floppy driver	*/

mfopen( io, ext, file )
register struct iob *io;
char *ext, *file;
{
	register struct inode *ip;
	register int dev;
	register int drive;	/* the drive number	*/
	register int slice;	/* the slice		*/
	char c;

	ip = io->i_ip;

	dprintf(("mfopen:(%s)\n",ext));
	/* probe to see if device is there		*/
	if ( mdprobe() != CONF_ALIVE ) 
		goto error;

	/* determine drive and filesystem from extension */
	/* only one filesystem on floppy */

	if ( (drive = exttodrive( ext )) < 0 )
		goto error;
	if ( drive >= D_NFLOP )
		goto error;

	dev = (D_FLP << 5) | (drive << 3);

	/* now attach the drive */
	if ( !mdinited ) {
		if ( minit() < 0 ) {
			dprintf(("minit failed\n"));
			goto error;
		}
	}

	if ( mconfig(dev) < 0 ) {
		dprintf(("mconfig failed\n"));
		goto error;
	}


	ip->i_dev |= ( (D_FLP << 5) | dev);
	dprintf(("dev is 0x%x\n",dev));

	return(0);

error:
	io->i_error = ENXIO;
	return (-1);
}

mfclose(io)
register struct iob *io;
{
}

mfioctl()
{
}

mfstrategy(io,flag)
register struct iob *io;
int flag;
{
	register struct buf *bp;
	register int bn;

	bp = io->i_bp;
	bn = bp->b_iobn;

	dprintf(("mfstrategy %s:dev:blk:addr:size 0x%x:0x%x:0x%x:0x%x\n",
		(flag==READ?"r":"w"),bp->b_dev, bn,bp->b_iobase,bp->b_bcount ));
	
	if ( flag == READ ) {
		if ( mddev(FCN_READ,bn,bp->b_iobase,minor(bp->b_dev),
						bp->b_bcount) ) 
			bp->b_error = EIO;
	} else {
		if ( mddev(FCN_WRITE,bn,bp->b_iobase,minor(bp->b_dev),
						bp->b_bcount) ) 
			bp->b_error = EIO;
	}

	if ( bp->b_error )
		return(-1);
	else
		return(bp->b_bcount);
}

/* QIC tape driver	*/

mtopen( io, ext, file )
register struct iob *io;
char *ext, *file;
{
	register iopb_t *iop = IOPB;
	register struct inode *ip;
	register dev_t device;
	register tapestatb_t *tstat;
	u_char i;

	if ( mdprobe() != CONF_ALIVE )
		goto error;
	ip = io->i_ip;
	device = ip->i_dev | (D_217 << 5); /* XXX should have macro in dk */

	dprintf(("mtopen: dev is 0x%x\n",ip->i_dev));
	
	if ( minit() < 0 ) {
		io->i_error = ENXIO;
		return (-1);
	}

	/* thats right folks 3 commands to initialize the tape	*/
	if ( mconfig(device, FCN_INIT, 0) < 0 )
		goto error;
	if ( mconfig(device, FCN_TINIT, 0) < 0 )
		goto error;
	if ( mconfig(device, FCN_TRESET, 0) < 0 )
		goto error;

	/* now get the status	*/
	if ( mtstat(0) < 0 )
		goto error;

	tstat = TAPEST;

	/*
	** Remember that the bytes are twisted
	*/
	if((i = tstat->sb[1]) ) {
		dprintf(("QIC: Hard Error: 0x%x (byte 0)\n", i));
		if ( i & SBEOM )
			dprintf(("End of tape reached\n"));
		if ( i & ILLFMT )
			dprintf(("Illegal format or unit not present\n"));
		if ( i & LNGTERM )
			dprintf(("Two commands issued at once\n"));
		if ( i & FAILINIT )
			dprintf(("Initialization failed\n"));
		if ( i & BADCMD ) {
			dprintf(("Bad command issued. function 0x%x\n",iop->p_func));
		}
		goto error;
	}
	if((i = tstat->sb[0]) ) {
		if(i&NOTAPE) {
			printf("QIC: No cartridge\n");
			goto error;
		}
		if(i&NOTRDY) {
			dprintf(("QIC: unit Not ready\n"));
			goto error;
		}
	}
	if ( mtrewind(0) ) 
		goto error;

	return(0);

error:
	dprintf(("tape reset error\n"));
	io->i_error = EIO;
	return (-1);
}

mtclose(io)
register struct iob *io;
{
	mtrewind(0);
	delay_ms(2);
	mdclear();
}

mtioctl()
{
}

mtstrategy(io,flag)
register struct iob *io;
int flag;
{
	register struct buf *bp;
	register iopb_t *iop = IOPB;

	bp = io->i_bp;

	switch ( flag ) {

	case WRITE:
		bp->b_error = EIO;
		break;

	case READ:
		iop->p_dev = D_217;
		iop->p_func = FCN_READ;
		iop->p_unit = 0;
		iop->p_mod = M_NOINT;
		iop->p_cyl = iop->p_sec	= iop->p_hd = 0;
		iop->p_dba = (u_char *)SwapW(mbvtop(bp->b_iobase));
		iop->p_rbc = SWAPW(bp->b_bcount);
		iop->p_atc = 0;

		if(mtcmd() < 0) {
			mtstat(iop->p_unit);
			bp->b_error = EIO;
			goto done;
		}
		break;
	}

done:
	if ( bp->b_error ) {
		bp->b_resid = bp->b_bcount - SWAPW(iop->p_atc);
		return(-1);
	} else
		return(0);

}

/* DSD controller functions for tape and disk		*/

/*
 * minit() -- initialize the controller
 */
minit()
{
	register wub_t *wub = WUB;
	register struct mddevice *rp = MDIOADDR;
	register ccb_t *ccb = CCB;
	register cib_t *cib = CIB;
	register iopb_t *iop = IOPB;
	extern char *mbmalloc();

	mdinited = 1;

	if ( !privatebuffer )
		if ((privatebuffer = mbmalloc(MAXBSIZE+MDKLUGESIZE)) == 0)
	    		return(-1);
	dprintf(("privatebuffer is 0x%x\n",privatebuffer));
	mdbuffer = privatebuffer;
	mdvarp = mdbuffer + MAXBSIZE;

	/* now setup the hardware	*/

	wub->w_xx	= 0;		/* Set up WUB */
	wub->w_ext	= W_EXT;
	wub->w_ccb	= (UCHAR *)SwapW(((int)mbvtop((caddr_t)ccb)));

	ccb->c_busy1	= 0xFF;		/* Set up CCB */
	ccb->c_ccw1	= 1;
	ccb->c_cib 	= (UCHAR *)SwapW(((int)mbvtop((caddr_t)&cib->i_csa)));
	ccb->c_xx	= 0;
	ccb->c_busy2	= 0;
	ccb->c_ccw2	= 1;
	ccb->c_cp 	= (UCHAR *)SwapW(((int)mbvtop((caddr_t)&ccb->c_ctrlptr)));
	ccb->c_ctrlptr	= 0x0004;

	cib->i_opstat	= 0;		/* Set up CIB */
	cib->i_xx	= 0;
	cib->i_stsem	= 0;
	cib->i_cmdsem	= 0;
	cib->i_csa	= 0;
	cib->i_iopb	= (UCHAR *)SwapW(((int)mbvtop((caddr_t)iop)));
	cib->i_xx2	= 0;

	RESET(rp); CLEAR(rp); START(rp);	/* Perform reset/initialize */

	if(mbusy())
		return(-1);
	return(0);
}

/*
 * mbusy() -- Wait for ccb->c_busy1 to become zero................
XXXXXX - change to delay call
 */
mbusy()
{
	register tmo = 10000000;
	register ccb_t *ccb = CCB;

	while(ccb->c_busy1 && --tmo) ;
	if(tmo == 0) {
		dprintf(("md: Busy timeout\n"));
		return 1;
	}
	return 0;
}

/*
 * mstatus() -- Wait for controller to post a status byte
 */
mstatus()
{
	register tmo = 10000000;
	register cib_t *cib = CIB;

	while((cib->i_stsem == 0) && --tmo) ;
	if(tmo == 0) {
		dprintf(("md: mstatus Timeout\n"));
		return(-1);
	}
	return(0);
}


mtrewind(unit)
	UCHAR unit;
{
	register iopb_t *iop = IOPB;
	int i;

	/* this controller really sucks. If you get an error on
	 * a rewind try again. (see unix dsd.c comment )
	*/
	for ( i=0; i < 2 ; i++ ) {
		iop->p_mod = M_NOINT;
		iop->p_dba = 0;
		iop->p_rbc = iop->p_atc = 0;
		iop->p_dev = D_217;
		iop->p_unit = unit;
		iop->p_func = FCN_TREW;
		if(mtcmd() < 0 ) 
			continue;
		/*
		** Have to add call to check for the end of the long command
		*/
		if ( dsdwait() < 0 )
			continue;
		else
			return(0);
	}
	dprintf(("mtrewind house\n"));
	return(-1);
}

/*
** Start a tape command --
** Returns '0' if OK on a short command.
** Returns '1' if Not OK and it failed.
** Returns '-1' if long command is running and is OK
** -- or -- short command timed out.
*/
mtcmd()
{
	register ccb_t *ccb = CCB;
	register cib_t *cib = CIB;
	register iopb_t *iop = IOPB;
	register struct mddevice *rp = MDIOADDR;

	while(ccb->c_busy1 != 0)
		;
	cib->i_stsem = 0;
	iop->p_mod = M_NOINT;

dprintf(("qic%d: dev %x cmd %x addr %x rbc %x atc %x\n", iop->p_unit, iop->p_dev, iop->p_func, SWAPW(iop->p_dba), SWAPW(iop->p_rbc), SWAPW(iop->p_atc)));

	START(rp);
	return(dsdwait());
}

dsdwait()
{
	register tmo = 400000;
	register struct cib *cib = CIB;
	register iopb_t *iop = IOPB;
	register struct mddevice *rp = MDIOADDR;
	register j;

	while(--tmo)
		if((j = quickcheck()) != 0xdef) break;
	CLEAR(rp);
	if(tmo <= 0) {
		dprintf(("QIC: timeout\n"));
		return (-1);
	}
	if(j & (HARD)) {

		dprintf(("qic%d error: dev %x cmd %x addr %x rbc %x atc %x\n",
			iop->p_unit, iop->p_dev, iop->p_func,
			SWAPW(iop->p_dba), SWAPW(iop->p_rbc),
			SWAPW(iop->p_atc)));

		if ( iop->p_func != FCN_TREW )
			dprintf(("QIC: error 0x%x(cib obstat)\n", (UCHAR)j));
		return(-1);
	}
	dprintf(("qic%d OK: dev %x cmd %x addr %x rbc %x atc %x\n",
		iop->p_unit, iop->p_dev, iop->p_func,
		SWAPW(iop->p_dba), SWAPW(iop->p_rbc),
		SWAPW(iop->p_atc)));
	return(0);
}


/*
** Tape quick check to see if status is ready
*/
quickcheck()
{
	register i;
	register struct cib *cib = CIB;

	if(cib->i_stsem == 0) return (0xdef);
	i = cib->i_opstat;
	cib->i_stsem = 0;
	cib->i_opstat = 0;
	return (i);
}


/*
** mtstat(unit) - Read the tape status and put it into the local buffer for
**	   - analysis
*/
mtstat(unit)
{
	register i;
	register iopb_t *iop = IOPB;
	register tapestatb_t *tapest = TAPEST;

	for(i = 0; i < 11; i++) tapest->sb[i] = 0;
	iop->p_dev = D_217;
	iop->p_unit = unit;
	iop->p_func = FCN_TDSTAT;
	iop->p_dba = (u_char *)SwapW(mbvtop(tapest));
	iop->p_mod = M_NOINT;
	iop->p_cyl = iop->p_hd = iop->p_sec = 0;
	iop->p_rbc = iop->p_atc = 0;
	if( mtcmd() < 0 )
		return(-1);
	iop->p_func = FCN_TSTAT;
	iop->p_mod = M_NOINT;
	iop->p_dba = (u_char *)SwapW(mbvtop(tapest));
	if(mtcmd() < 0)
		return(-1);

	for(i = 0; i < 11; i++) {
		dprintf(("%x ", tapest->sb[i]));
	}
	dprintf(("\n"));
	return(0);
}

/*
 * mgpstatus() -- get status from controller after an error
 *		and print it out prettily.
 */
mgpstatus()
{
	register UCHAR *sb = STATB;
	register iopb_t *iop = IOPB;
	register i;

	for(i = 0; i < 12; i++) sb[i] = 0;
	iop->p_func = FCN_TSTAT;
	iop->p_dba = (UCHAR *)SwapW(((int)mbvtop((caddr_t)sb)));
	(void) mcommand();
	if(!retries) {
		dprintf(("md ERR: blk - %d err - ", md_blkno));
		switch(sb[SB_EXS]) {
		case 0x25:	dprintf(("Sector not found")); break;
		case 0x34:	dprintf(("Data ECC")); break;
		case 0x35:	dprintf(("ID ECC")); break;
		case 0x37:
		case 0x38:	dprintf(("Seek Cyl")); break;
		default:	dprintf(("%x", sb[SB_EXS])); break;
		}
		dprintf(("\n"));
	}
}

/*
 * mconfig() - 	configure the controller to handle the specified
 *		type of drive on the specified unit
 *		read the label to set up correctly
 */
mconfig(dev)
{
	register iopb_t *iop;
	register inib_t *iip;
	register struct disk_label *dl;
	int retval=0,i;
	int device;

	dprintf(("mconfig: initingdev 0x%x\n",dev));

	if ( minit() < 0 )			/* XXXXX Why every time? */
		dprintf(("mconfig minit failed\n"));

	switch ( D_CONTROLLER(dev) ) {

	case D_WIN:
		device = D_WIN;
		if ( D_DRIVE(dev) <= D_NWIN )
			dl = &hlab[D_DRIVE(dev)];
		else
			return(-1);
		break;
	case D_FLP:
		device = D_FLP;
		if ( D_DRIVE(dev) <= D_NFLOP )
			dl = &flab[D_DRIVE(dev)];
		else
			return(-1);
		break;
	case D_217:
		device = D_217;
		break;
	}

	dprintf(("device 0x%x, unit 0x%x\n",device,D_DRIVE(dev)));
	iop = IOPB; iip = INIB;

	md_blkno = -1;		/* None really */
	retries = 5; /*30000;	/* Make sure retries are set to five */
retry:
	iop->p_xx	= 0;
	iop->p_atc	= 0;
	iop->p_dev	= device;
	iop->p_func	= FCN_INIT;
	iop->p_unit	= D_DRIVE(dev);
	iop->p_cyl	= 0;
	iop->p_sec	= 0;
	iop->p_hd	= 0;
	iop->p_dba	= (UCHAR *)SwapW(((int)mbvtop((caddr_t)iip)));
	iop->p_rbc	= 0;
	iop->p_gap	= 0;

	if(device == D_FLP) {
		iip->i_ncyl	= 80;
		iip->i_rhd	= 2;
		iip->i_fhd	= 0;
		iip->i_spt	= 8;
		iip->i_nacyl    = 1;
	} else {
		iip->i_ncyl	= 2000; /* A lot of cylinders for the label */
		iip->i_rhd	= 0;
		iip->i_fhd	= 1;	/* Just one head for the label read */
		iip->i_spt	= 17;	/* There will always be 17 spt????? */
		iip->i_nacyl	= 10;	/* Just a number doesn't matter */
	}
	iip->i_bpsl  = (512 & 0xff);
	iip->i_bpsh  = ((512>>8) & 0xff);

	dprintf((" mconfig: primary init --- "));
	if(mcommand()) {
		if(retries--) {
			delay_sec(1);
			goto retry;
		} else
			return(-1);
	}
	dprintf((" complete\n"));

	/* XXX turn to switch	*/
	/* OK, inited the drive for default.  Go read the label */
	if(device == D_WIN) {
		dl->d_map[PHYSDEV].d_base = 0;
		if (retval = mdrblk(dev|PHYSDEV,0,dl,sizeof *dl)) {
			dprintf(("mdconfig: error reading disk label\n"));
			return(-1);
		} else {
			/* 	re-init the drive according to the label info */
			dprintf((" mconfig: labelread. cyl=%d sec=%d hds=%d\n"
				,dl->d_cylinders
				,dl->d_sectors
				,dl->d_heads));
			drivesopen |= (1 << D_DRIVE(dev));
			/*
			** Have to reset up for init.
			** mdrblk has messed up the iopb.
			** Have to reconfigure with label information.
			*/
			iop->p_xx	= 0;
			iop->p_atc	= 0;
			iop->p_dev	= device;
			iop->p_func	= FCN_INIT;
			iop->p_unit	= D_DRIVE(dev);
			iop->p_cyl	= 0;
			iop->p_sec	= 0;
			iop->p_hd	= 0;
			iop->p_dba = (UCHAR *)SwapW(((int)mbvtop((caddr_t)iip)));
			iop->p_rbc	= 0;
			iop->p_gap	= 0;

			iip->i_ncyl = dl->d_cylinders;
			iip->i_spt = dl->d_sectors;
			iip->i_fhd = dl->d_heads;
			iip->i_nacyl = dl->d_nalternates;
			iip->i_rhd   = 0;
			iip->i_bpsl  = (512 & 0xff);
			iip->i_bpsh  = ((512>>8) & 0xff);
			mdrootslice = (dl->d_rootnotboot) ? dl->d_rootfs
				      : dl->d_bootfs;

			retries = 5;
			while (retries--){
				if (!(retval = mcommand())) {
					dprintf((" drive re-init complete\n"));
					return(0);
				}
			}
		}
		printf("error re-initing to label info\n");
	} else if ( device == D_217) {
		;
	} else {
		bzero((char *)dl,sizeof *dl);
	}
	return(retval);
}

char mdrecurse;
/*
 * mcommand() -- issue the command previously set into the iopb.
 */
mcommand()
{
	register iopb_t *iop = IOPB;
	register cib_t *cib = CIB;
	register struct mddevice *rp = MDIOADDR;

	iop->p_mod = M_NOINT;
	if(mbusy()) return 1;		/* At this point, busy must == 0 */
	cib->i_opstat = 0;		/* Set stat to know state */
	cib->i_stsem = 0;
	START(rp);			/* Start the ctlr going */
	if(mstatus() < 0) return 1;		/* Wait for completion */
	cib->i_stsem = 0;		/* Note we've received the status */
	CLEAR(rp);			/* Clear possible interrupt (SEEK) */
	if(cib->i_opstat & (HARD|SUMMARY)) {
		if(!retries)
			dprintf(("md Error "));	/* OOPS, hard error */
		if(mdrecurse == 0) {
			mdrecurse = 1;
			mgpstatus();
			mdrecurse = 0;
		} else
			if(!retries)
				dprintf((": mdstatus Error\n"));
		drivesopen = 0;		/* need to reinit after error */
		return 1;
	}
	return 0;
}

/*
 * mddev() -- device call to the midas disk driver.
 */
mddev(cmd, bno, buf, dev, size)
int dev;
register short cmd;
register long bno;
register long *buf;
int size;
{
	register struct iopb *iop = IOPB;
	register bn,nbpt;
	register inib_t *iip = INIB;
	register struct disk_label *dl;


if ( cmd == FCN_WRITE )
dprintf(("WRITE!!!\n"));
	if ( ISMDWIN(dev) )
		dl = &hlab[D_DRIVE(dev)];

	if ( ISMDFLP(dev) )
		dl = &flab[D_DRIVE(dev)];

	bno += dl->d_map[D_FS(dev)].d_base;
dprintf(("bno %d\n",bno));
	md_blkno = bno;
	retries = 5;		/* reset to 5 each time entering this palace */
retry:
	bn = bno;
	iop->p_dev = D_CONTROLLER(dev);
	iop->p_func = cmd;
	iop->p_unit = D_DRIVE(dev);
	nbpt = (iip->i_fhd * iip->i_spt);
	if(D_CONTROLLER(dev) == D_WIN) {
		iop->p_cyl = bn / nbpt;
		bn %= nbpt;
		iop->p_sec = bn % iip->i_spt;
		iop->p_hd = bn / iip->i_spt;
	} else {
		iop->p_cyl = bn / 16;
		bn %= 16;
		iop->p_sec = ((bn % 8) + 1);	 /* Floppies start at one */
		iop->p_hd = bn / 8;
	}
	iop->p_dba = (UCHAR *)SwapW((int)mbvtop(buf));
	iop->p_rbc = SWAPW(size);
	if(mcommand()) {
		if(--retries) {
			goto retry;
		} else {
			dprintf(("%s: MAX retries\n",
				(D_CONTROLLER(dev)==D_FLP)? "mf" : "md"));
			return 1;
		}
	}
	if(cmd == FCN_READ) {
		dprintf(("read dev:unit:phybno:size:addr %d:0x%x:0x%x:0x%x:0x%x\n",iop->p_dev, iop->p_unit, bno, size, iop->p_dba));
	}
	return 0;
}

mdrblk(dev,bno, buf, size)
long bno;
long *buf;
int size;
{
	int x;

	dprintf((" md.%d.%d:%d.",D_DRIVE(dev),D_FS(dev),bno));
	x = mddev((short)FCN_READ, bno, privatebuffer, dev, SECSIZE);
	bcopy(privatebuffer, (char *)buf, size);
	return(x);
}

mfrblk(bno, buf, size)
long bno;
long *buf;
int size;
{
	dprintf((" mf:%d.",bno));
	return mddev((short)FCN_READ, bno, buf, D_FLP, size, 0);
}

mdclear() {
	register struct mddevice *rp = MDIOADDR;

	CLEAR(rp) ;
}

long
SwapW(x)
	long x;
{
	return SWAPW(x);
}
