/*
** disksub.c	- Copyright (C) Silicon Graphics - Mountain View, CA 94043
**		- Author: chase
**		- Date: April 1985
**		- Any use, copy or alteration is strictly prohibited
**		- and gosh darn awfully -- morally inexcusable
**		- unless authorized by SGI.
**
** $Author: root $
** $State: Exp $
** $Source: /d2/3.7/src/pm2stand/mdfex/RCS/disksub.c,v $
** $Revision: 1.1 $
** $Date: 89/03/27 17:11:40 $
*/

/* Subroutines for the DSD 5215/5217 Controller for the floppy and the Tape */

#include <sys/types.h>
#include <sys/dklabel.h>
#include "disk.h"
#include "dsdreg.h"
#include "fex.h"

#define	VP0(s) if(verbose && !quiet) printf(s)
#define	VP1(s,a) if(verbose && !quiet) printf(s,a)
#define	VP2(s,a,b) if(verbose && !quiet) printf(s,a,b)
#define	VP3(s,a,b,c) if(verbose && !quiet) printf(s,a,b,c)

/* sd_flags bits */
#define	SFINIT	0x01		/* Controller Initted */

struct	wub	*wub;
struct	ccb	*ccb;
struct	cib	*cib;
struct	iopb	*iop;
struct	inib	*iip;
#define	fmb	((struct fmtb *)iip)
#define statbp ((struct statb *)iip)
struct tapest *tpstatus;				/* 217 Tape status */

#define PAUSE for(i=1000000;--i;)
init()
{
	if(drivep->label.d_magic != D_MAGIC) {
		printf("Drive %d not initialized!!\n", dunit);
		return 1;
	}
	if(sd_flags & SFINIT) return 0;
	wub = (struct wub *) (ioport << 4);
	ccb = (struct ccb *) ((UCHAR *)wub + sizeof *wub);
	cib = (struct cib *) ((UCHAR *)ccb + sizeof *ccb);
	iop = (struct iopb *)((UCHAR *)cib + sizeof *cib);
	iip = (struct inib *)((UCHAR *)iop + sizeof *iop);
	tpstatus = (struct tapest *)((UCHAR *)iip + sizeof *iip);

	wub->w_xx	= 0;
	wub->w_ext	= W_EXT;
	wub->w_ccb	= INTEL(ccb);

	ccb->c_busy	= 0xFF;
	ccb->c_ccw1	= 1;
	ccb->c_cib	= INTEL(&cib->i_csa);
	ccb->c_xx	= 0;
	ccb->c_busy2	= 0;
	ccb->c_cp	= INTEL(&ccb->c_ctrlptr);

	cib->i_opstat	= 0;
	cib->i_xx	= 0;
	cib->i_stsem	= 0;
	cib->i_cmdsem	= 0;
	cib->i_iopb	= INTEL(iop);
	cib->i_xx2	= 0;

	pp("wub", (USHORT *)wub, sizeof *wub);
	pp("ccb", (USHORT *)ccb, sizeof *ccb);
	pp("cib", (USHORT *)cib, sizeof *cib);

	if(verbose) { printf("Before RESET~"); getchar(); }
	RESET();
	if(verbose) { printf("After RESET ~"); getchar(); }
	CLEAR();
	if(verbose) { printf("After CLEAR ~"); getchar(); }
	START();
	if(verbose) printf("After START\n");

	sd_flags = SFINIT;
	cib->i_csa	= 0;
	ccb->c_ccw2	= 1;
	ccb->c_ctrlptr	= 0x0004;
	return wbusy();
}

mdpp()
{
	printf("ccb: B=%x cib: OP%x SM%x ",
		(UCHAR)ccb->c_busy, (UCHAR)cib->i_opstat,
		(UCHAR)cib->i_stsem);
	printf("iopb: D=%x F=%x U=%d M=%x %d/%d/%d a=%x ac=%d rc=%d ",
		(USHORT)iop->p_dev, (UCHAR)iop->p_func,
		(UCHAR)iop->p_unit, (USHORT)iop->p_mod,
		(USHORT)iop->p_cyl, (UCHAR)iop->p_hd, (UCHAR)iop->p_sec,
		UNINTEL(iop->p_dba),
		SWAPW(iop->p_atc), SWAPW(iop->p_rbc));
	printf("inib: nc=%d ", (USHORT)iip->i_ncyl);
#ifdef NOTDEF
	printf("\nfh=%d sc=%d ac=%d spt=%d\n", (UCHAR)iip->i_fhd,
		(USHORT)((iip->i_bpsl)|(iip->i_bpsh<<8)),
		(UCHAR)iip->i_nacyl,(UCHAR)iip->i_spt);
#endif NOTDEF
}

pp(name, ptr, len)
char *name;
USHORT *ptr;
{
	if(!verbose) return;
	len >>= 1;
	printf("%s %X: ", name, ptr);
	while(len--)
		printf("%x ", *ptr++);
	printf("\n");
}

wbusy()
{
	register tmo = 1000000;

	while(ccb->c_busy && --tmo) ;
	if(tmo == 0) {
		QP0("Busy timeout\n");
		sd_flags = 0;
		return 1;
	}
	return 0;
}

wstatus()
{
	register tmo = 1000000;

	while((cib->i_stsem == 0) && --tmo) ;
	CLEAR();
	if(tmo == 0) {
		QP0("WSTATUS Timeout\n");
		sd_flags = 0;
		return 1;
	}
	return 0;
}

mgstatus()
{
	register struct statb *sp = statbp;
	register i;

	for(i = 0; i < 12; i++) sp->sb[i] = 0;
	iop->p_unit	= dunit;
	iop->p_func	= F_TSTAT;
	iop->p_mod	= M_NOINT;
	iop->p_dba	= INTEL(sp);
	(void) xmcommand();
#ifdef NOTDEF
	QP3("%x %x %x ", sp->sb[1], sp->sb[0], sp->sb[3]);
#endif
	QP2(" DES: %d/%d ", sp->sb[2]+(sp->sb[5]<<8), sp->sb[4]);
	QP2("GOT: %d/%d ", sp->sb[6]+((sp->sb[9]&0xF)<<8), sp->sb[8]);
	QP3("FG:%x XS:%x  RTRY:%d\n", (sp->sb[9]&0xF0)>>4,
		sp->sb[13], sp->sb[10]);
	if(sp->sb[13] == 0x25)
		secnotfound++;
	else
		secnotfound = 0;
}

ULONG	dcyl, dhd, dsec;	/* To be operated on */
explode(lba)
{
	dcyl = lba / SPC;
	dhd  = lba % SPC;
	dhd /= SEC;
	dsec = lba % SEC;
#ifdef NOTDEF
	printf("explode [%d-%d]", SPC, SEC);
#endif NOTDEF
}

config()
{
	if(init()) return 1;
	iop->p_xx	= 0;
	iop->p_atc	= 0;
	iop->p_dev	= drivep->tdev;
	iop->p_func	= F_INIT;
	iop->p_unit	= dunit;
	iop->p_mod	= M_NOINT;
	iop->p_cyl	= 0;
	iop->p_sec	= 0;
	iop->p_hd	= 0;
	iop->p_dba	= INTEL(iip);
	iop->p_rbc	= 0;
	iop->p_gap	= 0;

	iip->i_ncyl	= drivep->label.d_cylinders;
	if(drivep->tdev == D_WIN) {
		iip->i_rhd	= 0;
		iip->i_fhd	= HD;
	} else {
		iip->i_rhd	= HD; 
		iip->i_fhd	= 0;
	}
	iip->i_bpsl	= secsize;
	iip->i_spt	= SEC;
	iip->i_nacyl	= drivep->tdev==D_FLP?1:0;
	iip->i_bpsh	= secsize >> 8;

	return xmcommand();
}

xmcommand()
{
	if(!firmretry) iop->p_mod |= M_NORETRY;
	if(verbose && iop->p_dev != D_TAPE) mdpp();
	if(wbusy()) {
		printf("Midas controller is busy\n");
		if(verbose) mdpp();
		return 1;
	}
	cib->i_stsem = 0;
	START();
	if(wstatus()) {
		printf("Midas controller status not complete\n");
		if(verbose) mdpp();
		return 1;
	}
	cib->i_stsem = 0;
	if(cib->i_opstat & HARD) {
		if(iop->p_dev == D_TAPE) {
			printf("TAPE: HARD ERROR opstat=%x\n",
				(UCHAR)cib->i_opstat);
			if(verbose) mdpp();
			return 1;
		}
		if(incompex && Retry < 5) return 1;
		printf("Disk: HARD ERROR opstat=%x, ",
			(UCHAR)cib->i_opstat);
		mgstatus();
		lbas_left = ((SWAPW(iop->p_atc))/secsize);
		return 1;
	}
	secnotfound = 0;
	return 0;
}

rdata(ns, lba, memp)
char *memp;
{
	return rwdata(ns, lba, memp, F_READ);
}

wdata(ns, lba, memp)
char *memp;
{
	if(drivep->tdev == D_WIN && lba < SEC)
		isuptodate[dunit] = 0;	/* FLAG LABEL CLOBBERED */
	return rwdata(ns, lba, memp, F_WRITE);
}

rwdata(ns, lba, memp, fn)
char *memp;
{
	register i = ns * secsize;

	explode(lba);

	iop->p_dev	= drivep->tdev;
	iop->p_func	= fn;
	iop->p_unit	= dunit;
	iop->p_mod	= M_NOINT;
	iop->p_cyl	= dcyl;
	iop->p_sec	= drivep->tdev==D_WIN?dsec:dsec+1;
	iop->p_hd	= dhd;
	iop->p_dba	= INTEL(memp);
	iop->p_rbc	= SWAPW(i);
	iop->p_atc	= 0;

	return xmcommand();
}

fmt()
{
	register cyl, hd;

	fmb->f_pat1 = 0x55;
	fmb->f_func = 0x00;
	fmb->f_pat3 = 0xFF;
	fmb->f_pat2 = 0xAA;
	fmb->f_ilv  = ilv;
	fmb->f_pat4 = 0x00;

	for(cyl = 0; cyl < drivep->label.d_cylinders; cyl++) {
	    for(hd = 0; hd < HD; hd++) {
		iop->p_dev	= drivep->tdev;
		iop->p_func	= F_FORMAT;
		iop->p_unit	= dunit;
		iop->p_mod	= M_NOINT;
		iop->p_cyl	= cyl;
		iop->p_sec	= 0;
		iop->p_hd	= hd;
		iop->p_dba	= INTEL(fmb);
	again:
		if(xmcommand()) {
			QP2("Format err at %d/%d, ", cyl, hd);
			switch(rsq(0)) {
			case RETRY:	goto again;
			case SKIP:	continue;
			case QUIT:	return 1;
			}
		}
		if(nwgch() != -1) return 1;
	    }
	    if(cyl && (cyl%10) == 0) QP1("%3d ",cyl);
	}
	return 0;
}

#define HELP
#ifdef	HELP
static char ahelp;
#endif

fmtb(bad, good)
{
/*
 * First, format the bad track pointing to the good one.
 */
	if(!good) {
		printf("fmtb(%d, %d)!!\n", bad, good);
		return 1;
	}
	explode(good);
	fmb->f_pat1 = dcyl;
	fmb->f_func = 0x80;
	fmb->f_pat3 = dhd;
	fmb->f_pat2 = dcyl>>8;
	fmb->f_ilv  = ilv;
	fmb->f_pat4 = 0x00;

	explode(bad);
	iop->p_dev	= drivep->tdev;
	iop->p_func	= F_FORMAT;
	iop->p_unit	= dunit;
	iop->p_mod	= M_NOINT;
	iop->p_cyl	= dcyl;
	iop->p_sec	= 0;
	iop->p_hd	= dhd;
	iop->p_dba	= INTEL(fmb);
again:
	if(xmcommand()) switch(rsq("Error formatting bad track")) {
	case RETRY:	goto again;
	case SKIP:	break;
	case QUIT:	return;
	}

/*
 * Now go format the good track as an alternate
 */
#ifdef HELP
	printf("%d/%d:", dcyl, dhd);
#endif
	explode(good);
	fmb->f_pat1 = 0x55;
	fmb->f_func = 0x40;
	fmb->f_pat3 = 0xFF;
	fmb->f_pat2 = 0xAA;
	fmb->f_ilv  = ilv;
	fmb->f_pat4 = 0x00;
#ifdef HELP
	printf("%d/%d  ", dcyl, dhd);
	if(++ahelp == 4) {
		printf("\n");
		ahelp = 0;
	}
#endif

	iop->p_dev	= drivep->tdev;
	iop->p_func	= F_FORMAT;
	iop->p_unit	= dunit;
	iop->p_mod	= M_NOINT;
	iop->p_cyl	= dcyl;
	iop->p_sec	= 0;
	iop->p_hd	= dhd;
	iop->p_dba	= INTEL(fmb);
again2:
	if(xmcommand()) switch(rsq("Error formatting good track")) {
	case RETRY:	goto again2;
	case SKIP:	break;
	case QUIT:	return;
	}
	return;
}

ULONG sslba;
dseek(cyl)
{
	iop->p_dev	= drivep->tdev;
	iop->p_func	= F_SEEK;
	iop->p_unit	= dunit;
	iop->p_mod	= M_NOINT;
	iop->p_cyl	= cyl;
	iop->p_hd	= 0;
	return xmcommand();
}

dwait()
{
	iop->p_dev	= drivep->tdev;
	iop->p_func	= F_RDID;
	iop->p_unit	= dunit;
	iop->p_mod	= M_NOINT;
	iop->p_dba	= INTEL(statbp);
	return xmcommand();
}

#ifdef SOMEDAY
rdsecid(lba, flag)
UCHAR flag;
ULONG lba;
{
	register ULONG i;
	register alttrk_t *alt = (alttrk_t *)BUF0;

	if(verbose) printf(". %d", lba);
	explode(lba);
	iop->p_cyl	= dcyl;
	iop->p_sec	= drivep->tdev==D_WIN?dsec:dsec+1;
	iop->p_hd	= dhd;
	iop->p_dev	= drivep->tdev;
	iop->p_func	= F_SEEK;
	iop->p_unit	= dunit;
	iop->p_mod	= M_NOINT;
	iop->p_atc = iop->p_rbc = 0;
	if(xmcommand()) {
		printf("Readsec: Failure on xmcommand in SEEK\n");
		return 0;
	}
	if(waitseek()) {
		printf("Readsec: Timeout on seek\n");
		return 0;
	}
	iop->p_dba	= INTEL(alt);
	iop->p_func	= F_RDID;
	iop->p_atc = iop->p_rbc = 0;
	if(xmcommand()) {
		printf("Readsec: Failure on xmcommand in READ ID\n");
		return 0;
	}
	if(flag == BAD) {
		if((i = ALTTRK(alt->flags)) == NORMAL) return -1;
		else if( i == DEFECTIVE) {
			i = alt->cylinder*alt->head*SEC;
			return i;
		} else {
			printf("ReadID Failure with flags %x at lba %d\n",
				i, lba);
			return 0;
		}
	} else {	/* Checking for a good track emulating a bad track */
		if((i = ALTTRK(alt->flags)) == ALTERNATE) {
			i = alt->cylinder*alt->head*SEC;
			return i;
		} else if(i == NORMAL || i == DEFECTIVE || i ==INVALID) {
			printf("ReadID Fail with flags %x at good track %d\n",
				i, lba);
			return 0;
		}
	}
}
#endif

waitseek()
{
	register tmo = 100000;

	while(((cib->i_opstat & SKCOMP)== 0) && --tmo) ;
	if(tmo == 0) {
		sd_flags = 0;
		return 1;
	}
	return 0;
}
