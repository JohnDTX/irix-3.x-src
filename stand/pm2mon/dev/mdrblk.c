/* $Header: /d2/3.7/src/stand/pm2mon/dev/RCS/mdrblk.c,v 1.1 89/03/27 17:16:56 root Exp $ */
/* $Log:	mdrblk.c,v $
 * Revision 1.1  89/03/27  17:16:56  root
 * Initial check-in for 3.7
 * 
 * Revision 1.1  87/05/02  12:44:06  andre
 * Initial revision
 * 
 * Revision 1.2  84/03/28  09:18:56  root
 * release script
 * 
 * Revision 1.1  84/02/27  16:14:29  root
 * release script
 * 
 * Revision 1.1  84/02/27  15:59:04  root
 * release script
 * 
 * Revision 1.1  84/01/12  21:32:47  gb
 * snapshot script
 * 
 * Revision 1.4  83/10/13  06:13:20  chase
 * Debugged and Released Version of the First shipment of PROMS.
 * 
 * Revision 1.3  83/10/13  01:53:11  chase
 * Debugged entry into retries.
 * added some debugging code for test purposes.
 * Cleaned up some stuff(sh_t)!!!!!
 * 
 * Revision 1.2  83/10/11  05:28:29  chase
 * Changed the way to config for the Floppy. redid retries in software.
 * 
 * Revision 1.1  83/10/11  01:32:30  chase
 * Initial revision
 *  */

# include "pmII.h"

# include "sys/types.h"
# include "dklabel.h"
#include "mdrblk.h"


# undef  DEBUG do_debug
# include "dprintf.h"


int retries;
long	md_blkno;
char *mdbuffer;
char *mdvarp;

extern long SwapW();

/*
 * minit() -- initialize the controller
 *	   -- For the floppy and the Atasi.
 */
minit()
{
	extern char *gmalloc();

	if (mdbuffer == 0)
	if ((mdbuffer = gmalloc(MAXBSIZE+MDKLUGESIZE)) == 0)
	    return 1;
	mdvarp = mdbuffer+MAXBSIZE;
	return mddinit();
}

mddinit()
{
	register wub_t *wub = WUB;
	register struct mddevice *rp = MDIOADDR;
	register ccb_t *ccb = CCB;
	register cib_t *cib = CIB;
	register iopb_t *iop = IOPB;

	wub->w_xx	= 0;		/* Set up WUB */
	wub->w_ext	= W_EXT;
	wub->w_ccb	= (UCHAR *)SwapW(((int)vtop((caddr_t)ccb)));

	ccb->c_busy1	= 0xFF;		/* Set up CCB */
	ccb->c_ccw1	= 1;
	ccb->c_cib 	= (UCHAR *)SwapW(((int)vtop((caddr_t)&cib->i_csa)));
	ccb->c_xx	= 0;
	ccb->c_busy2	= 0;
	ccb->c_ccw2	= 1;
	ccb->c_cp 	= (UCHAR *)SwapW(((int)vtop((caddr_t)&ccb->c_ctrlptr)));
	ccb->c_ctrlptr	= 0x0004;

	cib->i_opstat	= 0;		/* Set up CIB */
	cib->i_xx	= 0;
	cib->i_stsem	= 0;
	cib->i_cmdsem	= 0;
	cib->i_csa	= 0;
	cib->i_iopb	= (UCHAR *)SwapW(((int)vtop((caddr_t)iop)));
	cib->i_xx2	= 0;

	RESET(rp); CLEAR(rp); START(rp);	/* Perform reset/initialize */

	if(mbusy())
		return 1;
	return 0;
}

/*
 * mbusy() -- Wait for ccb->c_busy1 to become zero................
 */
mbusy()
{
	register tmo = 1000000;
	register ccb_t *ccb = CCB;

	while(ccb->c_busy1 && --tmo) ;
	if(tmo == 0) {
		printf("md: Busy timeout\n");
		return 1;
	}
	return 0;
}

/*
 * mstatus() -- Wait for controller to post a status byte
 */
mstatus()
{
	register tmo = 1000000;
	register cib_t *cib = CIB;

	while((cib->i_stsem == 0) && --tmo) ;
	if(tmo == 0) {
		printf("md: mstatus Timeout\n");
		return 1;
	}
	return 0;
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
	iop->p_func = F_TSTAT;
	iop->p_dba = (UCHAR *)SwapW(((int)vtop((caddr_t)sb)));
	(void) mcommand();
	if(!retries) {
		printf("md ERR: %d", md_blkno);
		switch(sb[SB_EXS]) {
		case 0x25:	printf("Sector not found"); break;
		case 0x34:	printf("Data ECC"); break;
		case 0x35:	printf("ID ECC"); break;
		case 0x37:
		case 0x38:	printf("Seek Cyl"); break;
		default:	printf("%x", sb[SB_EXS]); break;
		}
		printf("\n");
	}
}

/*
 * mconfig() - 	configure the controller to handle the specified
 *		type of drive on the specified unit
 *	       	Two types of drives are available: Atasi and Tandon Floppy.
 */
mconfig(dev,cmd,drive,labptr)
register struct disk_label *labptr;
{
	register iopb_t *iop;
	register inib_t *iip;
	int retval=0,i;

	dprintf(("mconfig: initing\n"));
	if(minit())		/* INIT The Board every time */
		return 1;

	iop = IOPB; iip = INIB;

	md_blkno = -1;		/* None really */
	retries = 30000;	/* Make sure retries are set to five */
retry:
	iop->p_xx	= 0;
	iop->p_atc	= 0;
	iop->p_dev	= dev;
	iop->p_func	= cmd;
	iop->p_unit	= drive;
	iop->p_cyl	= 0;
	iop->p_sec	= 0;
	iop->p_hd	= 0;
	iop->p_dba	= (UCHAR *)SwapW(((int)vtop((caddr_t)iip)));
	iop->p_rbc	= 0;
	iop->p_gap	= 0;

	if(dev == D_FLP) {
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

	if(mcommand()) {
		if(retries--)
			goto retry;
		else
			return 1;
	}
	dprintf((" mconfig: primary init complete\n"));
	/* OK, inited the drive for default.  Go read the label */
	if(dev == D_WIN) {
		if (retval = mdrblk(0,labptr,DBLOCK)) {
			printf("mdinit: error reading disk label\n");
			return(retval);
		} else {
			/* 	re - init the drive according to the label info */
		dprintf((" mconfig: labelread. cyl=%d sec=%d hds=%d\n"
			,labptr->d_cylinders
			,labptr->d_sectors
			,labptr->d_heads));
			/*
			** Have to reset up for init.
			** mdrblk has messed up the iopb.
			** Have to reconfigure with label information.
			*/
			iop->p_xx	= 0;
			iop->p_atc	= 0;
			iop->p_dev	= dev;
			iop->p_func	= cmd;
			iop->p_unit	= drive;
			iop->p_cyl	= 0;
			iop->p_sec	= 0;
			iop->p_hd	= 0;
			iop->p_dba = (UCHAR *)SwapW(((int)vtop((caddr_t)iip)));
			iop->p_rbc	= 0;
			iop->p_gap	= 0;

			iip->i_ncyl = labptr->d_cylinders;
			iip->i_spt = labptr->d_sectors;
			iip->i_fhd = labptr->d_heads;
			iip->i_nacyl = labptr->d_nalternates;
			iip->i_rhd   = 0;
			iip->i_bpsl  = (512 & 0xff);
			iip->i_bpsh  = ((512>>8) & 0xff);

			retries = 5;
			while (retries--){
				if (!(retval = mcommand())) {
				dprintf((" drive re-init complete\n"));
					return(0);
				}
			}
		}
		printf("error re-initing to label info\n");
	
	} else if(dev == D_217) {
		;
	} else {
		bzero((char *)labptr,sizeof *labptr);
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
	if(mstatus()) return 1;		/* Wait for completion */
	cib->i_stsem = 0;		/* Note we've received the status */
	CLEAR(rp);			/* Clear possible interrupt (SEEK) */
	if(cib->i_opstat & (HARD|SUMMARY)) {
		if(!retries)
			printf("md Error ");	/* OOPS, hard error */
		if(mdrecurse == 0) {
			mdrecurse = 1;
			mgpstatus();
		} else
			if(!retries)
				printf(": mdstatus Error\n");
		return 1;
	}
	return 0;
}

/*
 * mddev(cmd, bno, buf) -- device call to the midas disk driver.
 */
mddev(cmd, bno, buf, type, size)
short type;
register short cmd;
register long bno;
register long *buf;
int size;
{
	register struct iopb *iop = IOPB;
	register bn,nbpt;
	register inib_t *iip = INIB;
	register char *XXXX;

	bno += boffset;		/* boffset is a  global */
	md_blkno = bno;
	retries = 5;		/* reset to 5 each time entering this palace */
retry:
	bn = bno;
	iop->p_dev = type;
	iop->p_func = cmd;
	iop->p_unit = drive;
	nbpt = (iip->i_fhd * iip->i_spt);
	if(type == D_WIN) {
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
	if (size <= MAXBSIZE)
		XXXX = mdbuffer;
	else
		XXXX = (char *)buf;
	iop->p_dba = (UCHAR *)SwapW((int)vtop(XXXX));
	iop->p_rbc = SWAPW(size);
	if(mcommand()) {
		if(--retries) {
	/*		printf("bn=%d.md: retry\n", bno);	/****/
			goto retry;
		} else {
			printf("%s: MAX retries\n", (type==D_FLP)? "mf" : "md");
			return 1;
		}
	}
	if(cmd == F_READ) {
		dprintf((" dev:read"));
		if (size <= MAXBSIZE)
			bcopy(mdbuffer,(char *)buf,size);
	}
	return 0;
}

mdinit(labptr)
register struct disk_label *labptr;
{
	return mconfig(D_WIN,F_INIT,drive,labptr);
}

mfinit(labptr)
register struct disk_label *labptr;
{
	dprintf((" mf: init\n"));
	return mconfig(D_FLP,F_INIT,drive,labptr);
}

mdrblk(bno, buf, size)
long bno;
long *buf;
int size;
{
	dprintf((" md:%d.",bno));
	return mddev((short)F_READ, bno, buf, D_WIN, size);
}

mfrblk(bno, buf, size)
long bno;
long *buf;
int size;
{
	dprintf((" mf:%d.",bno));
	return mddev((short)F_READ, bno, buf, D_FLP, size);
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
