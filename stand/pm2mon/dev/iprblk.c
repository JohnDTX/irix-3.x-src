/*
** $Date: 89/03/27 17:16:52 $
** $Source: /d2/3.7/src/stand/pm2mon/dev/RCS/iprblk.c,v $
** $Revision: 1.1 $
*/

# include "pmII.h"

# include "sys/types.h"
# include "dklabel.h"
#include "ipreg.h"

# undef DEBUG do_debug
# include "dprintf.h"


iopb_t *Iop;
uib_t *Uib;
char *ipbuffer;

/* externals */
long boffset;


/* Initialize the Board */
ipinit(labptr)
register struct disk_label *labptr;
{
	extern char *gmalloc();

	register int i;
	register ULONG XXXX;
	int retval;
	register char *rp = IP_IOADDR;
	register iopb_t *iop;
	register uib_t *uib;

	/*
	 * allocate space for iop, uib, and dma
	 */
	if (ipbuffer == 0)
	if ((ipbuffer = gmalloc(MAXBSIZE+sizeof *Iop+sizeof *Uib)) == 0)
		return(1);

	Iop = (iopb_t *)(ipbuffer+MAXBSIZE);
	Uib = (uib_t *)(ipbuffer+MAXBSIZE+sizeof *Iop);
	dprintf((" ipinit\n"));
	iop = Iop;
	uib = Uib;

	if (!probe(rp+IP_R0,1)) {
		printf("ip: not installed\n");
		return(1);
	}

	XXXX = vtop(Uib);
	iop->i_cyll	= 0;
	iop->i_cylh	= 0;
	iop->i_bufh	= HB(XXXX);
	iop->i_bufl	= LB(XXXX);
	iop->i_bufm	= MB(XXXX);
	iop->i_sccl	= 0/*xxx*/;
	iop->i_scch	= 0;
	iop->i_secl	= 0;
	iop->i_sech	= 0;
	iop->i_head	= 0;
	iop->i_dmacount	= IP_DMACOUNT;
	XXXX = vtombio(rp);
	iop->i_iol	= LB(XXXX);
	iop->i_ioh	= MB(XXXX);
	iop->i_rell = iop->i_relh = 0;
	iop->i_linkl = iop->i_linkh = iop->i_linkm = 0;
	iop->i_reserved	= 0;
	uib->u_spt	= 64;
	uib->u_hds	= 1;
	uib->u_bpsh	= MB(512);
	uib->u_bpsl	= LB(512);
	uib->u_gap1	= (UCHAR)20;
	uib->u_gap2	= (UCHAR)30;
	uib->u_retry	= IP_RETRY;
	uib->u_ilv = 0xc1;
	uib->u_reseek	= 1;		/* Reseek enabled */
	uib->u_eccon	= 1;		/* ECC correction turned on */
	uib->u_inchd	= 1;		/* Increment by head */
	uib->u_mvbad	= 0;		/* Move Bad Data */
	uib->u_intron	= 0;		/* Interrupt on status change */
	uib->u_dualp	= 0;		/* Dual port */
	/* Skew spirial factor */
	uib->u_skew	= (UCHAR)11;
	uib->u_group	= 11;
	uib->u_resv1 = uib->u_resv2 = uib->u_resv3	= 0;
	retval = ipcmd((short)C_INIT, (long)0, (long *)0);

	if (!retval) {
		/* OK, inited the drive for default.  Go read the label */
		iop->i_sccl = 1;
		if (iprblk(0,labptr,DBLOCK)) 
			printf("ipinit: error reading disk label\n");
		else {
			/* 	re - init the drive according to the label info */
			uib->u_spt = labptr->d_sectors;
			uib->u_hds = labptr->d_heads;
			uib->u_gap1	= (UCHAR)labptr->d_misc[0];
			uib->u_gap2	= (UCHAR)labptr->d_misc[1];
			uib->u_ilv = (labptr->d_interleave | 0xc0);
			uib->u_skew	= labptr->d_cylskew;
			XXXX = vtop(Uib);
			iop->i_bufh	= HB(XXXX);
			iop->i_bufl	= LB(XXXX);
			iop->i_bufm	= MB(XXXX);
		}
		retval = ipcmd((short)C_INIT, (long)0, (long *)0);
		if (retval) printf("error re-initing to label info\n");
	}
	return(retval);

}

/* Read a block */
iprblk(bno, buf, size)
	register long bno;
	long *buf;
	int size;
{
	register int i;
	register ULONG XXXX;
	register int retval;
	register C=(Uib->u_spt * Uib->u_hds);
	register iopb_t *iop = Iop;

	bno += boffset;		/* Offset to start of partition */
dprintf((" iprblk(%d,$%x,%d)\n",bno,buf,size));
	i = bno / C;
	iop->i_cyll	= LB(i);
	iop->i_cylh	= MB(i);
	iop->i_secl = LB(bno % Uib->u_spt);
	iop->i_sech = MB(bno % Uib->u_spt);
	iop->i_sccl	= size>>IP_LOG;	/*???*/
	iop->i_scch	= 0;
	iop->i_head = bno % C / Uib->u_spt;
	if (size <= MAXBSIZE)
		XXXX = vtop(ipbuffer);
	else
		XXXX = vtop(buf);
	iop->i_bufh	= HB(XXXX);
	iop->i_bufl	= LB(XXXX);
	iop->i_bufm	= MB(XXXX);
	retval = ipcmd((short)C_READ, bno, buf);
	if (size <= MAXBSIZE)
		bcopy(ipbuffer,(char *)buf,size);
	return(retval);
}

/* Externals */
short	drive;

ipcmd (cmd, bno, buf)
	register short cmd;
	register long bno;
	register long *buf;
{
	register retries;
	register int i;
	register ULONG XXXX;
	register char *rp = IP_IOADDR;
	register iopb_t *iop = Iop;


	retries = 5;
	/*
	** Write the io registers to the board
	*/
	XXXX = vtop(iop);

	rp[IP_R1] = HB(XXXX);
	rp[IP_R2] = MB(XXXX);
	rp[IP_R3] = LB(XXXX);
	iop->i_unit	= drive;
	iop->i_option	= (O_IOPB | O_BUF);

retry:
	iop->i_error	= 0;
	iop->i_status	= 0;

	iop->i_cmd	= cmd;

#ifdef DEBUG
	if( do_debug )
	    ippp();
#endif
	rp[IP_R0] = IP_GO;

	/* Wait for the Board to complete the operation */
	i = 800000;
	while ((iop->i_status == 0) || (iop->i_status == 0x81) && --i);
	/* Check the status */
	while((rp[IP_R0] & IP_DONE) == 0 && i);
	rp[IP_R0] = IP_CLEAR;
	if (iop->i_status != 0x80) {
		if(--retries) {
			dprintf((" ipretry\n"));
			goto retry;
		} else {
			printf ("ip%d: error 0x%x at block %d\n", drive,
				iop->i_error, bno);
# ifdef DEBUG
		if( do_debug )
			ippp();
# endif DEBUG
			return 1;
		}
	}

	return 0;
}

# ifdef DEBUG
ippp()
{
	register char *rp = IP_IOADDR;

	register iopb_t *iop = Iop;
	register uib_t *uib = Uib;

	printf("iopb: iop %x (%x) uib %x (%x)\n",Iop,vtop(Iop),Uib,vtop(Uib));
	printf("iopb: rp %x (%x)\n",rp,vtombio(rp));

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
# endif DEBUG
