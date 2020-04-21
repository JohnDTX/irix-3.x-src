/*
** tapesub.c	- Copyright (C) Silicon Graphics - Mountain View, CA 94043
**		- Author: chase
**		- Date: April 1985
**		- Any use, copy or alteration is strictly prohibited
**		- and gosh darn awfully -- morally inexcusable
**		- unless authorized by SGI.
**
** $Author: root $
** $State: Exp $
** $Source: /d2/3.7/src/stand/cmd/mdfex/RCS/tapesub.c,v $
** $Revision: 1.1 $
** $Date: 89/03/27 17:12:59 $
*/
#include <sys/types.h>
#include "disk.h"
#include "dsdreg.h"
#include <sys/dklabel.h>
#include "fex.h"

extern ULONG init();

ULONG tapeinit[2];
ULONG longinprog = 0;

tension()
{
	register i;

	if(tpopen()) return 1;
	iop->p_func = F_TRETEN;
	iop->p_mod = M_NOINT;
	iop->p_dev = D_217;
	iop->p_unit = tunit;
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
	iop->p_func = F_TERASE;
	iop->p_mod = M_NOINT;
	iop->p_dev = D_217;
	iop->p_unit = tunit;
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
	printf("  Rewinding...");
	iop->p_func = F_TREW;
	iop->p_mod = M_NOINT;
	iop->p_dev = D_217;
	iop->p_unit = 0;
	if (longinprog) return 1;
	longinprog = 1;
	if(tapecmd()) {
		printf("Rewind: Failure\n");
		return 1;
	}
	waitlong();
	if (verbose)
		printf(" complete\n");
	return 0;
}

tpopen()
{
	tunit = 0;
	if(tapeinit[tunit]) return 0; /* Already opened */
/*	config();		*/
	if(tconfig(F_INIT)) {
		printf("Could not init the Controller\n");
		return 1;
	}
	if(tconfig(F_TINIT)) {
		printf("tape: Could not init the Tape drive %d\n",
			tunit);
		return 1;
	}
	if(tconfig(F_TRESET)) {
		printf("Tape: Could not reset the Tape drive %d\n",
			tunit);
		return 1;
	}
	tapeinit[tunit]++;
	return 0;
}

tapestatus()
{
	register i;

	if (tpopen()) {
		printf("tapestatus: Failure on the open of the tape drive\n");
		return 1;
	}
	for (i = 0; i < 11; i++)
		tpstatus->tb[i] = 0;
	iop->p_dev = D_217;
	iop->p_unit = tunit;
	iop->p_func = F_TDSTAT;
	iop->p_dba = 0;
	iop->p_mod = M_NOINT;
	iop->p_cyl = iop->p_hd = iop->p_sec = 0;
	iop->p_rbc = iop->p_atc = 0;
	if (tapecmd()) {
		printf("Tape: Failure on reading tape status\n");
		return 1;
	}
	iop->p_func = F_TSTAT;
	iop->p_dba = INTEL(tpstatus);
	if (tapecmd()) {
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
	register tmo = 8000000;
	register i;

	if(verbose) tdpp();
	while ((ccb->c_busy != 0) && --tmo)
		;
	if (tmo == 0) {
		printf("tpcmd: timeout waiting for busy to clear\n");
		return 1;
	}
	tmo = 0x8000000;
	cib->i_stsem = 0;
/*	cib->i_opstat = 0;	*/
	START();
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
	CLEAR();
	if(i & (HARD)) {
		printf("start: status %x\n", (UCHAR)i);
		tdpp();
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
		tdpp();
		printf("  waiting");
	}
	time = 4000000;
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
		CLEAR();
		cib->i_stsem = 0;
		return;
	} else {
		if (verbose)
			printf("status=%x\n", ops);
		goto restart;
	}
}


tconfig(command)
	unsigned short command;
{
	register i;

	if(init()) return 1;
	iop->p_xx	= 0;
	iop->p_atc	= 0;
	iop->p_dev	= D_217;
	iop->p_func	= command;
	iop->p_unit	= tunit;
	iop->p_mod	= M_NOINT;
	iop->p_cyl	= 0;
	iop->p_sec	= 0;
	iop->p_hd	= 0;
	iop->p_dba	= INTEL(iip);
	iop->p_rbc	= 0;
	iop->p_gap	= 0;

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
	register unsigned long bytes = ns * secsize;

	if(tpopen()) {
		printf("Qic%d %s: Failed open Drive\n", tunit,
			fn == F_WRITE? "Writing":"Reading");
		return 1;
	}
	iop->p_dev = D_217;
	iop->p_func = fn;
	iop->p_unit = tunit;
	iop->p_mod = M_NOINT;
	iop->p_cyl = iop->p_sec	= iop->p_hd = 0;
	iop->p_dba = INTEL(memp);
	iop->p_rbc = SWAPW(bytes);
	iop->p_atc = 0;

	if(tapecmd()) {
		printf("Qic%d %s: failed tapecmd call (0x%x)\n", tunit,
			fn == F_WRITE? "Writing":"Reading", bytes);
		return 1;
	}
	return 0;
}

spacef(count)
	USHORT count;
{
	register i = 1;

	if(verbose)
		printf("Space Files %d\n", count);
	iop->p_func = F_TSPFILE;
	iop->p_mod = M_NOINT;
	iop->p_dev = D_217;
	iop->p_unit = tunit;
	while(count--) {
		if (longinprog) {
			printf("spacef: long in progress\n");
			return 1;
		}
		longinprog = 1;
		if (tapecmd()) {
			printf("spacef: Failure\n");
			return 1;
		}
		waitlong();
		if(verbose)
			printf(" file mark %d\n", i);
		i++;
	}
	if(verbose)
		printf("complete\n");
	return 0;
}

tdpp()
{
	printf("ccb: B%x OP%x SM%x ",
		(UCHAR)ccb->c_busy, (UCHAR)cib->i_opstat,
		(UCHAR)cib->i_stsem);
	printf("iop: D%x F%x U%d M%x %d/%d ad=%x ac=%d rc=%d ",
		(USHORT)iop->p_dev, (UCHAR)iop->p_func,
		(UCHAR)iop->p_unit, (USHORT)iop->p_mod,
		(USHORT)iop->p_cyl, (UCHAR)iop->p_hd,
		UNINTEL(iop->p_dba),
		SWAPW(iop->p_atc), SWAPW(iop->p_rbc));
	printf("iip: nc=%d\n", (USHORT)iip->i_ncyl);
}
