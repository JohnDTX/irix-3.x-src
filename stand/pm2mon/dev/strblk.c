/*
**	stsub.c		- Copyright (C), JCS Computer Services 1983
**			- Author: chase
**			- Date: January 1985
**			- Any use, copying or alteration is prohibited
**			- and morally inexcusable,
**			- unless authorized in writing by JCS.
**	$Source: /d2/3.7/src/stand/pm2mon/dev/RCS/strblk.c,v $
**	$Revision: 1.1 $
**	$Date: 89/03/27 17:17:12 $
** 	Subroutines for the Interphase Storage Disk controller
**
*/
/*
** NOTES:
** there is a bug in the storager firmware in which the status is
** not valid for a short time after done is posted in the register.
** This is a problem in formatting the drive
*/

# include "pmII.h"
# include "sys/types.h"
# include "dklabel.h"
# include "streg.h"

# define DEBUG st_debug
# include "dprintf.h"
char st_debug;

extern char *stcmdlist[];
extern char *sterrlist[];
char isinited[8];

PROMSTATIC
	int st_dunit;
	iopb_t *st_iop;
	iopb_t *st_tiop;
	uib_t *st_uib;
	u_long st_ioaddr;
	short st_initflag;
	char *st_buffer;

#define	ST_INIT	0x02		/* Controller Initted */
#define CMDOFFSET	0x80
#define ERROFFSET	0x10


stinit(unit)
	int unit;
{
	extern char *gmalloc();

	register u_long addr;

	if (st_buffer == 0)
	if ((st_buffer = gmalloc(MAXBSIZE
			+ sizeof *st_uib
			+ sizeof *st_tiop + sizeof *st_iop)) == 0)
		return 0;
	(st_uib = (uib_t *)st_buffer+MAXBSIZE);
	st_iop = (iopb_t *)(st_uib + 1);
	st_tiop = (iopb_t *)(st_iop + 1);

	st_dunit = unit;
	if(isinited[st_dunit]) {
		return 0;
	} else {
		dprintf((" ST%d: Initializing", st_dunit));
	}
	st_initflag = 1;
	msdelay(49); /*20000*/

	/*
	** Write the io registers to the board
	*/
	regload();
	addr = (u_long)st_uib;

	st_iop->i_option = OP_OPTIONS;
	st_iop->i_status = 0;
	st_iop->i_error = 0;
	st_iop->i_unit = st_dunit;
	st_iop->i_head = 0;
	st_iop->i_cylh = 0;
	st_iop->i_cyll = 0;
	st_iop->i_sech = 0;
	st_iop->i_secl = 0;
	st_iop->i_scch = 0;
	st_iop->i_sccl = 0;
	/*
	st_iop->i_scch = MB(drivep->label.d_sectors);
	st_iop->i_sccl = LB(drivep->label.d_sectors);
	*/
	/*
	** Make the DMA count at 0 which is maximum.
	*/
	st_iop->i_dmacount = ST_DMACOUNT;
	st_iop->i_bufh = HB(addr);
	st_iop->i_bufm = MB(addr);
	st_iop->i_bufl = LB(addr);
	st_iop->i_ioh = MB(st_ioport);
	st_iop->i_iol = LB(st_ioport);
	st_iop->i_rell = st_iop->i_relh = 0;
	st_iop->i_linkl = st_iop->i_linkh = st_iop->i_linkm = 0;
	st_iop->i_tapeunit = 0;
	st_iop->i_cmd = C_INIT;

	st_uib->u_hds = drivep->label.d_heads;
	st_uib->u_spt = drivep->label.d_sectors;
	st_uib->u_bpsl = (secsize[st_dunit] & 0xff);
	st_uib->u_bpsh = ((secsize[st_dunit] >> 8) & 0xff);

	/*
	** interphase claims gap[12] must be an odd value
	** gap1 as vertex ships their drives is 16
	** gap2 as vertex ships their drives is 16
	** gap3 is specified as 8 bytes min for a 256 sector size
	*/
	gap1 = (u_char)drivep->label.d_misc[0];
	st_uib->u_gap1 = gap1;
	gap2 = (u_char)drivep->label.d_misc[1];
	st_uib->u_gap2 = gap2;
	gap3 = (u_char)drivep->label.d_misc[2];
	st_uib->u_gap3 = gap3;
	ilv = (u_char)drivep->label.d_interleave;
	if(ilv == 0)
		ilv = 1;
	st_uib->u_ilv = ilv;
	st_uib->u_retry = stretries;
	steccon = (u_char)drivep->label.d_misc[9];
	st_uib->u_eccon = steccon;

	st_uib->u_reseek = streseek;
	if(floppy)
		st_uib->u_reseek = 0;

	st_uib->u_mvbad = stmvbad;	/* Move Bad Data */
	st_uib->u_inchd = stinchd;	/* Increment by head */
	st_uib->u_resv0 = 0;
	st_uib->u_intron = 0;		/* Interrupt on status change */

	/* Skew spirial factor */
	st_uib->u_skew = (u_char)drivep->label.d_cylskew;
	st_uib->u_resv1 = 0;		/* Group Size (2190) */
	/*
	** Motor On and Head Unload
	** will be 0 for the Winchesters and
	** 0x11 for the Floppies
	** Motor off is 5 seconds and Head Unload is 5 seconds.
	*/
	st_uib->u_mohu = (u_char)drivep->label.d_misc[10];

	/*
	** Turn on caching and zero latency enable
	** Zero latency will always be on.
	** Zero latency will be togglable.
	** Also always set Update IOPB. 0x04
	** CBREQ is the 0x40 bit.
	*/
# define zerolatency 0
	st_uib->u_options = (zerolatency)?6:4; 		/* zero latency */
# define cacheenable 0
	if(cacheenable && !floppy)
		st_uib->u_options	|= 1;			/* cache enable */
	/*
	** ddb drive descriptor byte is:
	** Tandon Floppy: 0 1 0 0 0 1 0 0
	** Vertex Disk:   0 0 0 0 0 1 1 0
	** Atasi Disk:    0 0 0 0 0 1 1 0
	** Maxtor(ESDI):  0 0 1 0 0 1 1 1 - Address Mark
	** ESDI		  0 0 1 0 1 1 1 1 - Hard Sector
	** Maxtor(506) :  0 0 0 0 0 1 1 0
	**
	** smc step motor control is:
	** Tandon Turn on time is 250ms (set to 0x03(300ms))
	** All Winchesters with Buf Steps set to 0x20
	** else Winchesters set to 0
	*/
	esditype[st_dunit] = (u_char)drivep->label.d_misc[11];
	st_uib->u_ddb = esditype[st_dunit];
	st_uib->u_smc = (u_char)drivep->label.d_misc[12];

	/*
	** vertex specifies 2 us min step pulse width
	** Each 1 = 5us
	** vertex specifies min of 5 us pulse interval for buffered
	** pulses and max of 39.  Normal pulses are min or 25 us
	** The Tandon Floppy has 3ms spi and spw of 200ns
	*/
	st_uib->u_spw = (u_char)drivep->label.d_misc[3];
	st_uib->u_spil = (((u_short)drivep->label.d_misc[4])&0xff);
	st_uib->u_spih = ((((u_short)drivep->label.d_misc[4])>>8)&0xff);
	/*
	** the head load/settling time seems to be included in the
	** track-to-track time
	** Making the Vertex at 1ms and the Atasi at 3ms
	** The Tandon Floppy is 15ms.
	*/
	st_uib->u_hlst = (u_char)drivep->label.d_misc[8];
	/*
	** vertex specifies the max track-to-track time as 5 ms
	*/
	st_uib->u_ttst = (u_char)drivep->label.d_misc[5];
	st_uib->u_ncl = LB(drivep->label.d_cylinders);
	st_uib->u_nch = MB(drivep->label.d_cylinders);
	/*
	** Write Precompensation -- Can't use this.
	** The Tandon Floppy has 0.
	*/
	st_uib->u_wpscl = (((u_short)drivep->label.d_misc[6])&0xff);
	st_uib->u_wpsch = ((((u_short)drivep->label.d_misc[6])>>8)&0xff);
	/*
	** cant find info about reduced write current being needed.  
	** assume not (ffff)
	*/
	st_uib->u_rwcscl = (((u_short)drivep->label.d_misc[7])&0xff);
	st_uib->u_rwcsch = ((((u_short)drivep->label.d_misc[7])>>8)&0xff);

	ifdebug((stpp()));

	if(stcmd()) {
		st_initflag = 0;
		return 1;
	}
	isinited[st_dunit] = 1;
	dprintf((" ST%d: INIT complete disk: %s", st_dunit, dtype->tname));
	st_initflag = 0;
	return 0;
}

long	dcyl, dhd, dsec;	/* To be operated on */
explode(lba)
{
	register i;

	i = drivep->label.d_heads*drivep->label.d_sectors;
	dcyl = lba / i;
	dhd = lba % i;
	dhd /= drivep->label.d_sectors;
	dsec = lba % drivep->label.d_sectors;
}

rdata(ns, lba, memp)
	char *memp;
{
	return rwdata(ns, lba, memp, C_READ);
}

rwdata(ns, lba, memp, fn)
	char *memp;
{
	register u_long addr = (u_long)memp;

	explode(lba);

	st_iop->i_cmd = fn;
	st_iop->i_unit = st_dunit;
	st_iop->i_cylh = MB(dcyl);
	st_iop->i_cyll = LB(dcyl);
	st_iop->i_sech = MB(dsec);
	st_iop->i_secl = LB(dsec);
	st_iop->i_head = dhd;
	st_iop->i_sccl = LB(ns);
	st_iop->i_scch = MB(ns);
	st_iop->i_bufl = LB(addr);
	st_iop->i_bufm = MB(addr);
	st_iop->i_bufh = HB(addr);
	st_iop->i_option = OP_OPTIONS;
	st_iop->i_iol = LB(st_ioport);
	st_iop->i_ioh = MB(st_ioport);

	if(stcmd()) {
		if (errhalt) {
			printf("rwdata: Failure at %d/%d/%d\n",
				dcyl, dhd, dsec);
			isinited[st_dunit] = 0;
		}
		return 1;
	}
	return 0;
}

long sslba;
dseek(cyl)
{
	st_iop->i_unit = st_dunit;
	st_iop->i_cylh = MB(cyl);
	st_iop->i_cyll = LB(cyl);
	st_iop->i_sech = 0;
	st_iop->i_secl = 0;
	st_iop->i_sccl = LB(drivep->label.d_sectors);
	st_iop->i_scch = MB(drivep->label.d_sectors);
	st_iop->i_head = 0;
	st_iop->i_cmd = C_SEEK;
	st_iop->i_option = OP_OPTIONS;		/* bus buffer word mode */
	st_iop->i_iol = LB(st_ioport);
	st_iop->i_ioh = MB(st_ioport);
	/*
	 * Start the operation
	 */
	if(stcmd()) {
		printf("seek: Failure at %d/%d\n", cyl, st_iop->i_head);
		isinited[st_dunit] = 0;

		ifdebug((stpp()));

		return 1;
	}
	return 0;
}

/*
** stcmd() - Command to start and check controller for operation
**	    - It assumes the complete iopb is set up.
*/
stcmd()
{
	register char *rp = (char *)st_ioaddr;
	register long timeout;

	if (st_busywait(800000))
		return 1;
	st_iop->i_error = 0;
	st_iop->i_status = 0;

	ifdebug((stpp()));

	DSTART(rp);
	timeout = 1000000;
	for(;;) {
		if(st_iop->i_status == S_OK) {
			DCLEAR(rp);
			return 0;
		}
		if(st_iop->i_status == S_ERROR) {
			st_error();

			if (errhalt)
				ifdebug((stpp()));

			DCLEAR(rp);
			if (!errhalt) {
				streset();
				isinited[st_dunit] = 0;
				stinit();
			}
			return 1;
		}
		if((--timeout) == 0) {
			st_toerr(st_iop);

			if (errhalt)
				ifdebug((stpp()));

			DCLEAR(rp);
			if (!errhalt) {
				streset();
				isinited[st_dunit] = 0;
				stinit();
			}
			return 1;
		}
	}
}

stwait()
{
	register char *rp = (char *)st_ioaddr;
	register int timeout = 1000000;

	for(;;) {
		if(st_iop->i_status == S_OK) {
			DCLEAR(rp);
			msdelay(5);	/*2000*/
			return 0;
		}
		if(st_iop->i_status == S_ERROR) {
			st_error();

			DCLEAR(rp);
			msdelay(5);	/*2000*/

			ifdebug((stpp()));

			return 1;
		}
		if((--timeout) == 0) {
			switch (st_iop->i_status) {
		 	case S_BUSY:
		 	case S_ERROR:
				st_toerr();

				DCLEAR(rp);
				msdelay(5);	/*2000*/

				ifdebug((stpp()));
				return 1;

		 	default:
				DCLEAR(rp);
				msdelay(5);	/*2000*/

				st_toerr();
				ifdebug((stpp()));
				return 1;
			}	
		}
	}
}

# ifdef DEBUG
stpp()
{
	register char *rp = (char *)st_ioaddr;

	printf("st_iop%d: c:%s o:%x %d/%d/%d s:%x e:%x scc:%d buf:%x\n",
		st_iop->i_unit, stcmdlist[st_iop->i_cmd - CMDOFFSET], st_iop->i_option,
		((st_iop->i_cylh << 8) | st_iop->i_cyll), st_iop->i_head,
		((st_iop->i_sech << 8) | st_iop->i_secl), st_iop->i_status,
		st_iop->i_error, ((st_iop->i_scch << 8) | st_iop->i_sccl),
		(((st_iop->i_bufh << 16) | (st_iop->i_bufm << 8)) | st_iop->i_bufl));
	if (!st_initflag)
		return;

	printf("st_uib: h %d, s %d, bps %d, gaps %d %d %d, ilv %d ret %d\n",
		st_uib->u_hds, st_uib->u_spt, ((st_uib->u_bpsh <<8) | st_uib->u_bpsl),
		st_uib->u_gap1, st_uib->u_gap2, st_uib->u_gap3, st_uib->u_ilv,
		st_uib->u_retry);

	printf("st_uib: ecc %x rsk %x mvbad %x inchd %x res %x intst %x sk %x\n",
		st_uib->u_eccon, st_uib->u_reseek, st_uib->u_mvbad, st_uib->u_inchd,
		st_uib->u_resv0, st_uib->u_intron, st_uib->u_skew);

	printf("uib2: mohu %x, opt %x, ddb %x, smc %x, spw %x, spi %x\n",
		st_uib->u_mohu, st_uib->u_options, st_uib->u_ddb, st_uib->u_smc,
		st_uib->u_spw, ((st_uib->u_spih <<8) | st_uib->u_spil));
	printf("uib3: hlst %x ttst %x nc %d wpsc %x rwcsc %x \n",
		st_uib->u_hlst, st_uib->u_ttst, ((st_uib->u_nch<<8) | st_uib->u_ncl),
		((st_uib->u_wpsch<<8) | st_uib->u_wpscl),
		((st_uib->u_rwcsch<<8) | st_uib->u_rwcscl));
	dprintf(("error iopb stat %x iopb err %x reg0 %x\n",
			st_iop->i_status, st_iop->i_error,
			DSTATUS(rp)));
}

streset()
{
	register char *rp = (char *)st_ioaddr;

	dprintf((" reset"));
	DRESET(rp);
	msdelay(49);	/*20000*/
	DCLEAR(rp);
	msdelay(49);	/*20000*/
	isinited[st_dunit] = 0;
	dprintf(("\n"));
}

/*
** Reload or load up the three registers with the IOPB Address
** Adjusted to set up both iopb's.......
*/
regload()
{
	register char *rp = (char *)st_ioaddr;
	register u_long addr = (u_long)st_iop;

	rp[ST_R1] = HB(addr);
	rp[ST_R2] = MB(addr);
	rp[ST_R3] = LB(addr);
	addr = (u_long)st_tiop;
	rp[ST_R5] = HB(addr);
	rp[ST_R6] = MB(addr);
	rp[ST_R7] = LB(addr);
}

int
st_busywait(timeout)
	long timeout;
{
	register char *rp = (char *)st_ioaddr;
	register int i;

	while((i = DSTATUS(rp)) & ST_BUSY) {
		if(!(--timeout)) {
			printf("ST%d: Busy fmt()(%x)(%x)\n",
				st_dunit, i, DSTATUS(rp));
			isinited[st_dunit] = 0;
			return 1;
		}
	}

	return 0;
}
# endif DEBUG

st_toerr()
{
	printf("\nst%d: TIMEOUT: s %x err %x c: %s\n",
		st_dunit, st_iop->i_status, st_iop->i_error,
		stcmdlist[st_iop->i_cmd - CMDOFFSET]);
}

st_error()
{
	printf("\nst%d: e: %x s: %x c: %s ERROR: %s\n",
		st_dunit, st_iop->i_error,
		st_iop->i_status,
		stcmdlist[st_iop->i_cmd - CMDOFFSET],
		sterrlist[st_iop->i_error - ERROFFSET]);
}
