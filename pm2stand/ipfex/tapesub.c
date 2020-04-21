/*
** tapesub.c 	- For IPFEX
**
** $Date: 89/03/27 17:11:27 $
** $State: Exp $
** $Source: /d2/3.7/src/pm2stand/ipfex/RCS/tapesub.c,v $
** $Author: root $
** $Revision: 1.1 $
** $Log:	tapesub.c,v $
 * Revision 1.1  89/03/27  17:11:27  root
 * Initial check-in for 3.7
 * 
 * Revision 1.1  85/08/26  16:32:21  root
 * Initial revision
 * 
*/

#include <sys/types.h>
#include "disk.h"
#include "ipreg.h"
#include "dsdreg.h"
#include <sys/dklabel.h>
#include "test.h"

ULONG tapeinit[2];
ULONG longinprog = 0;
char md_flags = 0;

tension()
{
	register i;

	if(tpopen()) return 1;
	mdiop->p_func = F_TRETEN;
	mdiop->p_mod = M_NOINT;
	mdiop->p_dev = D_217;
	mdiop->p_unit = 0;
	if(longinprog) return 1;
	longinprog = 1;
	if(i = tapecmd()) {
		if(i == -1) printf("tension: timeout ");
		printf("Tape: tension Failure\n");
		return 1;
	}
	waitlong();
	return 0;
}

erase()
{
	register i;

	if(tpopen()) return 1;
	mdiop->p_func = F_TERASE;
	mdiop->p_mod = M_NOINT;
	mdiop->p_dev = D_217;
	mdiop->p_unit = 0;
	if(longinprog) return 1;
	longinprog = 1;
	if(i = tapecmd()) {
		if(i == -1) printf("Erase: timeout ");
		printf("Erase: Failure\n");
		return 1;
	}
	waitlong();
	return 0;
}

rewind()
{
	register i;

	if(tpopen()) return 1;
	if (tapestatus()) {
		printf("rewind: failure reading status\n");
		return 1;
	}
	if (tpstatus->tb[2]) {
		if (verbose)
			printf("  rewind: already at BOT\n");
		return 0;
	}
	if (verbose)
		printf("  Rewinding...");
	mdiop->p_func = F_TREW;
	mdiop->p_mod = M_NOINT;
	mdiop->p_dev = D_217;
	mdiop->p_unit = 0;
	if(longinprog) return 1;
	longinprog = 1;
	if(tapecmd()) {
		printf("Rewind: Failure\n");
		return 1;
	}
	waitlong();
	if(verbose)
		printf("complete\n");
	else
		printf("\n");
	return 0;
}

tpopen()
{
	if(tapeinit[0]) return 0; /* Already opened */
/*	config();				/***/
	if(tconfig(F_INIT)) {
		printf("Could not init the Controller\n");
		return 1;
	}
	if(tconfig(F_TINIT)) {
		printf("tape: Could not init Tape drive\n");
		return 1;
	}
	if(tconfig(F_TRESET)) {
		printf("Tape: Could not reset Tape drive\n");
		return 1;
	}
	tapeinit[0]++;
	return 0;
}

tapestatus(mode)
UCHAR mode;
{
	register i;

	if(mode) {
		tapeinit[0] = 0;	/* Force a reinit of the Tape drive */
	}
	if(tpopen()) {
		printf("tapestatus: Failure on the open of the tape drive\n");
	}
	for(i = 0; i < 11; i++) tpstatus->tb[i] = 0;
	mdiop->p_dev = D_217;
	mdiop->p_unit = 0;
	mdiop->p_func = F_TDSTAT;
	mdiop->p_dba = 0;
	mdiop->p_mod = M_NOINT;
	mdiop->p_cyl = mdiop->p_hd = mdiop->p_sec = 0;
	mdiop->p_rbc = mdiop->p_atc = 0;
	if(verbose) mdpp();
	if(tapecmd()) {
		printf("Tape: Failure on reading tape status\n");
		return 1;
	}
	mdiop->p_func = F_TSTAT;
	mdiop->p_dba = mdSWAPW(tpstatus);
	if(verbose) mdpp();
	if(tapecmd()) {
		printf("Tape: Failure on transferring tape status\n");
		return 1;
	}
	return 0;
}

printstatus()
{
	register i;

	if(verbose) {
		printf("Tape Status: ");
		for(i=0 ; i< 11; i++) {
			printf("%x ", tpstatus->tb[i]);
		}
		if(tpstatus->tb[2]) printf("AT BOT ");
		else printf("NOT AT BOT ");
		printf("\n");
	} else {
		printf("Tape Status: HB0=%x HB1=%x SB0=%x %s RET=%x\n",
			tpstatus->tb[1], tpstatus->tb[0], tpstatus->tb[3],
			tpstatus->tb[2]?"At BOT":"Not at BOT",
			tpstatus->tb[10]);
	}
}

/*
 * Start a tape command --
 * Returns '0' if OK on a short command.
 * Returns '1' if Not OK and it failed.
 * Returns '-1' if short command timed out.
 */
tapecmd()
{
	register tmo = 4000000;
	register i;

	if(verbose) mdpp();
	while ((ccb->c_busy != 0) && --tmo)
		;
	if (tmo == 0) {
		printf("tpcmd: timeout waiting for busy to clear\n");
		return 1;
	}
	tmo = 800000;
	cib->i_stsem = 0;
	cib->i_opstat = 0;
	mdSTART();
	if(verbose) printf("~");

	while(--tmo && (cib->i_stsem == 0)) 
		;
	if(tmo == 0) {
		printf("Qic: timeout\n");
		return 1;
	}
	i = cib->i_opstat;
	cib->i_stsem = 0;
	cib->i_opstat = 0;
	mdCLEAR();
	if(i & (HARD)) {
		printf("tapestart: status %x\n", (UCHAR)i);
		mdpp();
		tapestatus();
		verbose++;
		printstatus();
		verbose--;
		return 1;
	}
	if((i&0xf) == TPLONG) {
		printf("tpcmd: Long Command complete\n");
		longinprog = 0;
	}
	return 0;
}

waitlong()
{
	register int ops;
	register time;
		
restart:
	if (!longinprog) {
		printf("  long command not started\n");
		return;
	}
	if (verbose) {
		mdpp();
		printf("  waiting");
	}
	time = 8000000;
	while (--time && (cib->i_stsem == 0))
		;
	if (!time) {
		if (verbose)
			printf("Timeout waiting for long complete\n");
		goto restart;
	}
	ops = cib->i_opstat;
	if ((ops & 0x0f) == TPLONG) {
		if (verbose)
			printf("long gone ");
		longinprog = 0;
		mdCLEAR();
		cib->i_stsem = 0;
		return;
	} else {
		if (verbose)
			printf("status=%x\n", ops);
		goto restart;
	}
}

/*
 * Tape quick check to see if status is ready
 */
quickcheck()
{
	register i;

	if(cib->i_stsem == 0) return (0xdef);
	i = cib->i_opstat;
	cib->i_stsem = 0;
	cib->i_opstat = 0;
	return (i);
}

tconfig(command)
USHORT command;
{
	register i;

	if(dsdinit()) return 1;
	mdiop->p_xx	= 0;
	mdiop->p_atc	= 0;
	mdiop->p_dev	= D_217;
	mdiop->p_func	= command;
	mdiop->p_unit	= 0;
	mdiop->p_mod	= M_NOINT;
	mdiop->p_cyl	= 0;
	mdiop->p_sec	= 0;
	mdiop->p_hd	= 0;
	mdiop->p_dba	= mdSWAPW(iip);
	mdiop->p_rbc	= 0;
	mdiop->p_gap	= 0;

	iip->i_ncyl	= 0x0001;
	iip->i_rhd	= 0;
	iip->i_fhd	= 0;
	iip->i_bpsl	= 0;
	iip->i_spt	= 0;
	iip->i_nacyl	= 0;
	iip->i_bpsh	= 0;
	if(i = tapecmd()) {
		if(i == -1 ) printf("Tape: timeout ");
		printf("Tape: Failure in Config()\n");
		return 1;
	}
	return 0;
}

rtdata(ns, memp)
char *memp;
{
	return rwtdata(ns, memp, F_READ);
}

wtdata(ns, memp)
char *memp;
{
	return rwtdata(ns, memp, F_WRITE);
}

rwtdata(ns, memp, fn)
char *memp;
{
	register i = ns * secsize;
	register x;

	if(tpopen()) {
		printf("tape: could not open the Tape drive\n");
		return 1;
	}
	mdiop->p_dev = D_217;
	mdiop->p_func = fn;
	mdiop->p_unit = 0;
	mdiop->p_mod = M_NOINT;
	mdiop->p_cyl = mdiop->p_sec = mdiop->p_hd = 0;
	mdiop->p_dba = mdSWAPW(memp);
	mdiop->p_rbc = SWAPW(i);
	mdiop->p_atc = 0;

	if(x = tapecmd()) {
		if(x == -1)
			printf("tape: timeout tapecmd\n");
		else
			printf("tape: failed tapecmd\n");
		return 1;
	}
	return 0;
}

spacef(count)
	USHORT count;
{
	register i = 1;

	if(verbose) printf("Space Files %d\n", count);
	mdiop->p_func = F_TSPFILE;
	mdiop->p_mod = M_NOINT;
	mdiop->p_dev = D_217;
	mdiop->p_unit = 0;
	while(count--) {
		if(longinprog) return 1;
		longinprog = 1;
		if(tapecmd()) {
			printf("spacef: Failure\n");
			return 1;
		}
		waitlong();
		if(verbose) printf(" file mark %d\n", i);
		i++;
	}
	if(verbose) printf("complete\n");
	return 0;
}

#ifdef NOTDEF
config()
{
	if(dsdinit()) return 1;
	mdiop->p_xx	= 0;
	mdiop->p_atc	= 0;
	mdiop->p_dev	= D_WIN;
	mdiop->p_func	= F_INIT;
	mdiop->p_unit	= 0;
	mdiop->p_mod	= M_NOINT;
	mdiop->p_cyl	= 0;
	mdiop->p_sec	= 0;
	mdiop->p_hd	= 0;
	mdiop->p_dba	= mdSWAPW(iip);
	mdiop->p_rbc	= 0;
	mdiop->p_gap	= 0;

	iip->i_ncyl	= 987;
	iip->i_rhd	= 0;
	iip->i_fhd	= 7;
	iip->i_bpsl	= 512;
	iip->i_spt	= 17;
	iip->i_nacyl	= 0;
	iip->i_bpsh	= 512 >> 8;

	return dsdcmd();
}

dsdcmd()
{
	if(wbusy()) {
		printf("DSD controller is busy\n");
		if(verbose) mdpp();
		return 1;
	}
	if(verbose) mdpp();
	mdSTART();
	if(wstatus()) {
		printf("DSD controller status not complete\n");
		if(verbose) mdpp();
		return 1;
	}
	cib->i_stsem = 0;
	mdCLEAR();
	if(cib->i_opstat & HARD) {
		if(mdiop->p_dev == D_TAPE) {
			printf("TAPE: HARD ERROR opstat=%x\n",
				(UCHAR)cib->i_opstat);
			if(verbose) mdpp();
			return 1;
		} else {
			printf("dsdcmd: Error ??????\n");
		}
		return 1;
	}
	return 0;
}

wstatus()
{
	register tmo = 1000000;

	while((cib->i_stsem == 0) && --tmo) ;
	mdCLEAR();
	if(tmo == 0) {
		QP0("WSTATUS Timeout\n");
		md_flags = 0;
		return 1;
	}
	return 0;
}
#endif					/* NOTDEF */

dsdinit()
{
#ifdef FOO
	if(drivep->label.d_magic != D_MAGIC) {
		printf("Drive %d not initialized!!\n", dunit);
		return 1;
	}
#endif
	if(md_flags) return 0;
	wub->w_xx	= 0;
	wub->w_ext	= W_EXT;
	wub->w_ccb	= mdSWAPW(ccb);

	ccb->c_busy	= 0xFF;
	ccb->c_ccw1	= 1;
	ccb->c_cib	= mdSWAPW(&cib->i_csa);
	ccb->c_xx	= 0;
	ccb->c_busy2	= 0;
	ccb->c_cp	= mdSWAPW(&ccb->c_ctrlptr);
	ccb->c_ccw2	= 1;
	ccb->c_ctrlptr	= 0x0004;

	cib->i_opstat	= 0;
	cib->i_xx	= 0;
	cib->i_stsem	= 0;
	cib->i_cmdsem	= 0;
	cib->i_iopb	= mdSWAPW(mdiop);
	cib->i_xx2	= 0;
	cib->i_csa	= 0;

	if(verbose) {
		pp("wub", (USHORT *)wub, sizeof *wub);
		pp("ccb", (USHORT *)ccb, sizeof *ccb);
		pp("cib", (USHORT *)cib, sizeof *cib);
		/* mdpp(); */
	}

	mdRESET();
	mdCLEAR();
	mdSTART();

	md_flags = 1;
	return wbusy();
}

mdpp()
{
	printf("ccb: b=%x cib: op=%x sm=%x\n",
		(UCHAR)ccb->c_busy, (UCHAR)cib->i_opstat,
		(UCHAR)cib->i_stsem);
	printf("iopb: d=%x f=%x u=%d m=%x %d/%d/%d ad=%x ac=%d rc=%d\n",
		(USHORT)mdiop->p_dev, (UCHAR)mdiop->p_func,
		(UCHAR)mdiop->p_unit, (USHORT)mdiop->p_mod,
		(USHORT)mdiop->p_cyl, (UCHAR)mdiop->p_hd, (UCHAR)mdiop->p_sec,
		mdSWAPW(mdiop->p_dba),
		SWAPW(mdiop->p_atc), SWAPW(mdiop->p_rbc));
	printf("inib: nc=%d fh=%d sc=%d ac=%d spt=%d\n",
		(USHORT)iip->i_ncyl, (UCHAR)iip->i_fhd,
		(USHORT)((iip->i_bpsl)|(iip->i_bpsh<<8)),
		(UCHAR)iip->i_nacyl,(UCHAR)iip->i_spt);
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
		md_flags = 0;
		return 1;
	}
	return 0;
}
