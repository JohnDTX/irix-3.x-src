/*
** $Source: /d2/3.7/src/pm2stand/ipfex/RCS/ipsub.c,v $
** $Date: 89/03/27 17:11:22 $
** $Revision: 1.1 $
*/

/*
** ipsub.c
** Subroutines for the Interphase 2190 Disk SMD Controller with the Amcodyne
** Disk 7110 drive, the Fujitsu 2312K and the Fujitsu Eagle 2351.
*/

#include <sys/types.h>
#include <sys/dklabel.h>
#include "disk.h"

extern long ip_ioaddr;
extern long md_ioaddr;

#include "ipreg.h"
#include "dsdreg.h"
#include "test.h"
#include "iperrlist.h"

iopb_t *iop;
uib_t *uib;
struct	wub	*wub;
struct	ccb	*ccb;
struct	cib	*cib;
mdiopb_t *mdiop;
struct	inib	*iip;
#define statbp ((struct statb *)iip)
struct tapest *tpstatus;


#define	SFINIT	0x01		/* Controller Initted */

ipinit()
{
	register i;
	register ULONG addr;

	if(drivep->label.d_magic != D_MAGIC) {
		printf("Drive %d not initialized!!\n", dunit);
		return 1;
	}
	if(ip_flags & SFINIT) return 0;
	/*
	 * Set up pointers to the DSD structures
	 */
	wub = (struct wub *) (0x7f00 << 4);
	ccb = (struct ccb *) ((UCHAR *)wub + sizeof *wub);
	cib = (struct cib *) ((UCHAR *)ccb + sizeof *ccb);
	mdiop = (struct mdiopb *)((UCHAR *)cib + sizeof *cib);
	iip = (struct inib *)((UCHAR *)mdiop + sizeof *mdiop);
	tpstatus = (struct tapest *)((UCHAR *)iip + sizeof *iip);
	/*
	 * Set up pointers to ip structures
	 */
	iop = (iopb_t *)((UCHAR *)tpstatus + sizeof *tpstatus);
	uib = (uib_t *)((UCHAR *)iop + sizeof *iop);
	/*
	** Write the io registers to the board
	*/
	regload();
	addr = (ULONG)uib;

	iop->i_option	= (O_IOPB | O_BUF);
	iop->i_error	= 0;
	iop->i_status	= 0;
	iop->i_head	= 0;
	iop->i_unit	= dunit;
	iop->i_cyll	= 0;
	iop->i_cylh	= 0;
	iop->i_secl	= 0;
	iop->i_sech	= 0;
	iop->i_sccl	= LB(drivep->label.d_sectors);
	iop->i_scch	= HB(drivep->label.d_sectors);
	iop->i_bufh	= HB(addr);
	/*
	** Make the DMA count at 0 which is maximum.
	*/
	iop->i_dmacount	= IP_DMACOUNT;
	iop->i_bufl	= LB(addr);
	iop->i_bufm	= MB(addr);
	iop->i_iol	= LB(ioport);
	iop->i_ioh	= MB(ioport);
	iop->i_rell = iop->i_relh = 0;
	iop->i_linkl = iop->i_linkh = iop->i_linkm = 0;
	iop->i_reserved	= 0;
#ifdef NOTDEF
	/*
	 * Issue a Reset to the Board and then the Init
	 */
	iop->i_cmd	= C_RESET;
	if(ipcmd()) {
		printf("init: could not reset the board\n");
		debugpp();
		return 1;
	}
#endif
	iop->i_cmd	= C_INIT;

	uib->u_spt	= drivep->label.d_sectors;
	uib->u_hds	= drivep->label.d_heads;
	uib->u_bpsh	= ((secsize >> 8) & 0xff);
	uib->u_bpsl	= (secsize & 0xff);
	uib->u_gap1	= (UCHAR)drivep->label.d_misc[0];
	uib->u_gap2	= (UCHAR)drivep->label.d_misc[1];
	uib->u_retry	= IP_RETRY;
	uib->u_ilv = drivep->label.d_interleave==0?1:drivep->label.d_interleave;
	/*
	** Turn on caching and group enable
	*/
	if(cacheenable)
		uib->u_ilv |= 0xc0;
	uib->u_reseek	= 1;		/* Reseek enabled */
	uib->u_eccon	= 1;		/* ECC correction turned on */
	uib->u_inchd	= 1;		/* Increment by head */
	uib->u_mvbad	= 0;		/* Move Bad Data */
	uib->u_intron	= 0;		/* Interrupt on status change */
	uib->u_dualp	= 0;		/* Dual port */
	/* Skew spirial factor */
	uib->u_skew	= (UCHAR)drivep->label.d_cylskew;
	uib->u_group	= drivep->label.d_misc[2];
	uib->u_resv1 = uib->u_resv2 = uib->u_resv3	= 0;

	if(verbose) {
		debugpp();
		/*ippp("iopb", iop, sizeof *iop);	/*** REMOVE ***/
		/*ippp("uibp", uib, sizeof *uib);	/*** REMOVE ***/
	}
	if(ipcmd()) {
		printf("Ipinit: failure in call to ipcmd()\n");
		return 1;
	}
	ip_flags = SFINIT;
	if(verbose) printf("init: complete on disk %s drive %d\n",
				dtype->tname, dunit);
	return 0;
}

/*
** Reload or load up the three registers with the IOPB Address
*/
regload()
{
	register ULONG addr = (ULONG)iop;

	(*((char *)(ip_ioaddr+IP_R1))) = HB(addr);
	(*((char *)(ip_ioaddr+IP_R2))) = MB(addr);
	(*((char *)(ip_ioaddr+IP_R3))) = LB(addr);
}


#ifdef NOTDEF
ippp(name, ptr, len)
	UCHAR *name, *ptr;
{
	printf(" %s %X: ", name, ptr);
	while(len--)
		printf("%x ", *ptr++);
	printf("\n");
}
#endif

long	dcyl, dhd, dsec;	/* To be operated on */
explode(lba)
{
	register i;

	i = drivep->label.d_heads*drivep->label.d_sectors;
	dcyl = lba / i;
	dhd  = lba % i;
	dhd /= drivep->label.d_sectors;
	dsec = lba % drivep->label.d_sectors;
}

rdata(ns, lba, memp)
	char *memp;
{
	if(cacheenable && ns > groupsize)
		return rwdata(ns, lba, memp, C_READNOCACHE);
	else if(directmode)
		return rwdata(ns, lba, memp, C_READDIR);
	else
		return rwdata(ns, lba, memp, C_READ);
}

wdata(ns, lba, memp)
	char *memp;
{
	if(directmode)
		return rwdata(ns, lba, memp, C_WRITEDIR);
	else
		return rwdata(ns, lba, memp, C_WRITE);
}

rwdata(ns, lba, memp, fn)
	char *memp;
{
	register ULONG addr = (ULONG)memp;

	explode(lba);
	iop->i_cmd	= fn;
	iop->i_unit	= dunit;
	iop->i_cylh	= MB(dcyl);
	iop->i_cyll	= LB(dcyl);
	iop->i_sech	= MB(dsec);
	iop->i_secl	= LB(dsec);
	iop->i_head	= dhd;
	iop->i_sccl	= LB(ns);
	iop->i_scch	= MB(ns);
	iop->i_bufl	= LB(addr);
	iop->i_bufm	= MB(addr);
	iop->i_bufh	= HB(addr);

	if(ipcmd()) {
		printf("rwdata: Failure at %d/%d/%d\n", dcyl, dhd, dsec);
		ip_flags = 0;
		if(verbose) debugpp();
		return 1;
	}
	return 0;
}

fmt()
{
	register cyl, hd;

	for(cyl = 0; cyl < drivep->label.d_cylinders; cyl++) {
		for(hd = 0; hd < drivep->label.d_heads; hd++) {
			iop->i_cmd	= C_FORMAT;
			iop->i_unit	= dunit;
			iop->i_cylh	= MB(cyl);
			iop->i_cyll	= LB(cyl);
			iop->i_sech	= 0;
			iop->i_secl	= 0;
			iop->i_sccl	= LB(drivep->label.d_sectors);
			iop->i_scch	= MB(drivep->label.d_sectors);
			iop->i_head	= hd;

			if(ipcmd()) {
				printf("fmt: Failure at %d/%d\n", cyl, hd);
				ip_flags = 0;
				if(verbose) debugpp();
				return 1;
			}
			if(nwgch() != -1) return 1;
		}
    		if(cyl && (cyl%10) == 0) QP1("%3d ",cyl);
	}
	return 0;
}

fmtb(bad, good)
{

	explode(bad);
	iop->i_head	= dhd;
	iop->i_cylh	= MB(dcyl);
	iop->i_cyll	= LB(dcyl);
	QP2("bad %d/%d -> good ", dcyl, dhd);

	explode(good);
	iop->i_scch	= MB(dcyl);		/* New Cylinder */
	iop->i_sccl	= LB(dcyl);		/* New Cylinder */
	iop->i_sech	= 0;			/* 0 */
	iop->i_secl	= dhd;			/* New Head */
	iop->i_unit	= dunit;
	QP2("%d/%d\n", dcyl, dhd);
	
	/*
	 * Now start the formatting
	 */
	iop->i_cmd	= C_MAP;
	if (ipcmd()) {
		printf("fmtb: error Failure at %d/%d\n", dcyl, dhd);
		ip_flags = 0;
		if(verbose) debugpp();
		return 1;
 	}
	return 0;
}

long sslba;
dseek(cyl)
{
	iop->i_unit	= dunit;
	iop->i_cylh	= MB(cyl);
	iop->i_cyll	= LB(cyl);
	iop->i_sech	= 0;
	iop->i_secl	= 0;
	iop->i_sccl	= LB(drivep->label.d_sectors);
	iop->i_scch	= MB(drivep->label.d_sectors);
	iop->i_head	= 0;
	iop->i_cmd	= C_SEEK;
	/*
	 * Start the operation
	 */
	if(ipcmd()) {
		printf("seek: Failure at %d/%d\n", cyl, iop->i_head);
		ip_flags = 0;
		if(verbose) debugpp();
		return 1;
	}
	return 0;
}

report()
{
	iop->i_cmd	= C_REPORT;
	iop->i_unit	= dunit;
	/*
	 * Start the operation
	 */
	if(ipcmd()) {
		printf("report: failure\n");
		ip_flags = 0;
		debugpp();
		return 1;
	}
	/*
	 * Print out the report Information
	 */
	printf("\nThe Report information on the 2190 Controller is:\n");
	printf("\nFirmware Revision: %d.%d ",
		((iop->i_error>>4)&0xf), iop->i_error&0xf);
	if(iop->i_unit)
		printf("Ext: %c ", iop->i_unit);
	else
		printf("Ext: %d ", iop->i_unit);
	printf("Product Code: %x ", iop->i_head);
	if(iop->i_cylh || iop->i_cyll)
		printf("Options: %x %x\n", iop->i_cyll, iop->i_cylh);
	else
		printf("\n");
	return 0;
}

/*
** ipcmd() - Command to start and check controller for operation
**	    - It assumes the complete iopb is set up.
*/
ipcmd()
{
	register long i;
	register timeout = 40000;

	while(((i = (*(char *)(ip_ioaddr+IP_R0))) & IP_BUSY) && --timeout) ;
	if(timeout == 0) {
		printf("ipcmd: timeout waiting for idle controller stat %x\n",
				i);	
		goto error;
	}
	iop->i_error = 0;
	iop->i_status = 0;
	debugpp();
	START();
	timeout = 2000000;
	for(;;) {
		if(iop->i_status == S_ERROR) break;
		if(iop->i_status == S_OK) {
			CLEAR();
			return 0;
		}
		if((--timeout) == 0) {
			break;	
		}
	}
	if(timeout) {
		switch (iop->i_status) {
		 case S_OK:
			printf("What?");
			CLEAR();
			return 0;
		 default:
			CLEAR();
			printf("funny status %x\n", iop->i_status);
			return 1;
		 case S_BUSY:
		 case S_ERROR:
			printf("ipcmd: %s Error: %s\n",
				(iop->i_status==S_BUSY)?"Busy":"Error",
				iperrlist[iop->i_error - ERROFFSET]);
			goto error;
		}
	}
	printf("ipcmd(TIMEOUT): cmd: %s\n",
			ipcmdlist[iop->i_cmd - CMDOFFSET]);
error:
	CLEAR();
	timeout = (*((char *)(ip_ioaddr+IP_R0)));
	if(verbose) printf("ipcmd: error stat %x err: %s iostat %x\n",
			iop->i_status, iperrlist[iop->i_error-ERROFFSET],
			timeout);
	return 1;
}

debugpp()
{
	if(!verbose) return;		/** debug **/
	printf("iopb: c %x o %x u %x %d/%d/%d s %x e %x sc %d buf %x\n",
		iop->i_cmd, iop->i_option, iop->i_unit,
		((iop->i_cylh << 8) | iop->i_cyll), iop->i_head,
		((iop->i_sech << 8) | iop->i_secl), iop->i_status,
		iop->i_error, ((iop->i_scch << 8) | iop->i_sccl),
		(((iop->i_bufh << 16) | (iop->i_bufm << 8)) | iop->i_bufl));
	printf("uipb: h %d s %d b %d g1 %d g2 %d i %d sk %d group %d\n",
		uib->u_hds, uib->u_spt, ((uib->u_bpsh <<8) | uib->u_bpsl),
		uib->u_gap1, uib->u_gap2, (uib->u_ilv&0x3f),
		uib->u_skew, uib->u_group);
}

verify()
{
	register cyl, hd;

	printf("Verify disk %d\n", dunit);
	for(cyl = 0; cyl < drivep->label.d_cylinders; cyl++) {
		for(hd = 0; hd < drivep->label.d_heads; hd++) {
			iop->i_cmd	= C_VERIFY;
			iop->i_unit	= dunit;
			iop->i_cylh	= MB(cyl);
			iop->i_cyll	= LB(cyl);
			iop->i_sech	= 0;
			iop->i_secl	= 0;
			iop->i_sccl	= LB(drivep->label.d_sectors);
			iop->i_scch	= MB(drivep->label.d_sectors);
			iop->i_head	= hd;

			if(ipcmd()) {
				printf("verify: Failure at %d/%d\n", cyl, hd);
				ip_flags = 0;
				debugpp();
				return 1;
			}
			if(nwgch() != -1) return 1;
    		}
    		if(cyl && (cyl%10) == 0) QP1("%3d ",cyl);
	}
	return 0;
}
