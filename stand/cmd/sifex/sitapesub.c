/*
**
**
*/

#include <sys/types.h>
#include "disk.h"
#include "streg.h"
#include <sys/dklabel.h>
#include "fex.h"
#include "tapereg.h"

extern u_long st_ioaddr;

/* Flags */
int tape_flags = 0;

int tapeunit = 4;			/* Hard coded device */
int tape_status0;
int tape_status1;
int tape_status2;
int tape_status3;
int tape_status4;
int tape_status5;
int tape_status6;
int infilemark = 0;
int sitapefilemark = 0;
int inrewind = 0;

tapeinit()
{
	register int recursing;

	if (tape_flags & TAPE_INITED)
		return 0;
	tiop->i_unit = tapeunit;
	tiop->i_bufh = 0;
	tiop->i_bufm = 0;
	tiop->i_bufl = 0;

	/* set up the pbytes for 0 */
	tiop->i_pbyte0 = 0;
	tiop->i_pbyte1 = 0;
	tiop->i_pbyte2 = 0;
	tiop->i_pbyte3 = 0;
	tiop->i_pbyte4 = 0;
	tiop->i_pbyte5 = 0;
	tiop->i_timeout = 30;

	/* Set up some of the IOPB which will not change */
	tiop->i_rell = tiop->i_relh = 0;
	tiop->i_linkl = tiop->i_linkh = tiop->i_linkm = 0;
	tiop->i_dmacount = 0;

	/* update iopb & interrupt on status change */
	tiop->i_tpoption = 0x04;
	tiop->i_option = OP_OPTIONS;

	recursing = 0;
again:
	tiop->i_cmd = C_TPCONFIG;
	if (tapecmd()) {
		if (!recursing) {
			recursing = 1;
			goto again;
		}
		printf("failure in configuration\n");
		tape_flags |= TAPE_DEAD;
		return 1;
	} 
	tiop->i_tpoption = 0;			/* zero out the option */
	tape_flags |= TAPE_INITED;
	return 0;

}

rtdata(ns, memp)
	char *memp;
	u_long ns;
{
	register u_long addr = (u_long)memp;
	register i;

	tiop->i_cmd	= C_TPREAD;
	tiop->i_unit	= tapeunit;
	tiop->i_sccl	= LB(ns);
	tiop->i_scch	= MB(ns);
	tiop->i_bufl	= LB(addr);
	tiop->i_bufm	= MB(addr);
	tiop->i_bufh	= HB(addr);
	tiop->i_option 	= OP_OPTIONS;

	if(tapecmd()) {
		i = (tiop->i_sccl | (tiop->i_scch<<8));
		if (tiop->i_error == 0x02) {
			tape_flags |= FILE_MARK_FOUND;
			return 0;
		}
		if (tiop->i_error == 0x86) {
			tape_flags |= FILE_MARK_FOUND;
			if (verbose)
				printf("  Found a filemark\n");	
			return (i);
		}
		printf("++ failure reading from the tape drive\n");
		return -1;
	}
	return 0;
}

rewind()
{
	register i;
	
	if ((tape_flags & TAPE_INITED) == 0)
		if (tapeinit())
			return 1;
	tiop->i_cmd	= C_TPREWIND;
	tiop->i_unit	= tapeunit;

	inrewind = 1;
	if(tapecmd()) {
		printf("failure rewinding the tape drive\n");
		inrewind = 0;
		return 1;
	}
	i = 20000;
	while(--i);
	tape_flags |= TAPE_INREWIND;
	if (tapewait()) {
		tape_flags &= ~(TAPE_INREWIND);
		inrewind = 0;
		return 1;
	}
	tape_flags &= ~(TAPE_INREWIND);
	inrewind = 0;
	sitapefilemark = 0;
	return 0;
}

spacef(count)
	int count;
{
	register i;

	if ((tape_flags & TAPE_INITED) == 0)
		if (tapeinit())
			return 1;
	tiop->i_cmd	= C_TPRDFLMK;
	tiop->i_unit	= tapeunit;
	tiop->i_filel	= LB(count);
	tiop->i_fileh	= MB(count);

	infilemark = 1;
	if(tapecmd()) {
		printf("Tape failure spacing %d file mark%c\n",
			count, count>1?"s":" ");
		infilemark = 0;
		sitapefilemark = -1;
		return 1;
	}
	i = 20000;
	while (--i);
	tape_flags |= TAPE_INSPACE;
	if (tapewait()) {
		tape_flags &= ~(TAPE_INSPACE);
		infilemark = 0;
		sitapefilemark = -1;
		return 1;
	}
	tape_flags &= ~(TAPE_INSPACE);
	sitapefilemark = count;
	infilemark = 0;
	return 0;
}

tapecmd()
{
	register int timeout = 100000;
	register int i;
	register recursive = 0;

	while((i = STATUS()) & ST_BUSY) {
		if((--timeout) == 0) {
			printf("SQ%d: Busy on entering tapecmd()(%x)(%x)\n",
				tapeunit, i, STATUS());
			if (verbose)
				extrapp();
			return 1;
		}
	}
	tiop->i_error = 0;
	tiop->i_status = 0;
	if (verbose)
		tppp();
	TSTART();
	timeout = 2000;		/* A short timeout for what??? */
	while(--timeout);
again:
	timeout = 4000000;
	for(;;) {
		if(tiop->i_status == S_OK) {
			TCLEAR();
			if (verbose)
				tppp();
			return 0;
		}
		if(tiop->i_status == S_ERROR) {
			sitapefilemark = -1;
			TCLEAR();
			if (tiop->i_error == 0x86 || tiop->i_error == 0x02)
				return 1;
			printf("\nSQ%d: e: %x s: %x c: 0x%x ERROR: 0x%x\n",
				tiop->i_unit, tiop->i_error, tiop->i_status,
				tiop->i_cmd, tiop->i_error);
			if (verbose)
				tppp();
			return 1;
		}
		if((--timeout) == 0) {
			if (infilemark || inrewind) {
				recursive++;
				if (recursive < 100)
					goto again;
			}
			TCLEAR();
			printf("\nSQ%d: TIMEOUT: s %x err %x c: 0x%x\n",
				tiop->i_unit, tiop->i_status, tiop->i_error,
				tiop->i_cmd);
			if (verbose) {
				tppp();
				extrapp();
			}
			tape_flags = 0;
			return 1;
		}
	}
}

tapestatus()
{
	register long addr = (long)BUF0;
	register char *status;

	if ((tape_flags & TAPE_INITED) == 0)
		if (tapeinit())
			return 1;
	tiop->i_unit = tapeunit;
	tiop->i_bufh = HB(addr);
	tiop->i_bufm = MB(addr);
	tiop->i_bufl = LB(addr);
	tiop->i_option = OP_OPTIONS;
	tiop->i_cmd = C_TPSTATUS;
	if (tapecmd()) {
		printf("SQ%d: failure in reading tape status\n", tiop->i_unit);
		return 1;
	}
	status = (char *)BUF0;
	tape_status1 = *status++;
	tape_status0 = *status++;
	tape_status3 = *status++;
	tape_status2 = *status++;
	tape_status5 = *status++;
	tape_status4 = *status;
	if (verbose)
		printstatus();
	return 0;
}

printstatus()
{
	printf("   ***** STORAGER 2 QIC STATUS\n");
	printf("   ***** %s -- %s -- %s -- %s.\n",
		tape_status0&NOCARTRIDGE?"no tape":"ready",
		tape_status0&NOTONLINE?"not on line":"on line",
		tape_status0&WRITEPROTECTED?
			"write protected":"writable tape",
		tape_status0&ENDOFTAPE?"at eot":"");
	printf("   ***** %s -- %s -- %s.\n",
		tape_status0&DATAERROR?"data error":"",
		tape_status0&BOTNOTFOUND?"NO BOT FOUND":"",
		tape_status0&FILEFOUND?"file mark":"");
	printf("   ***** %s -- %s -- %s -- %s -- %s.\n",
		tape_status1&BOT?"AT BOT":"NOT AT BOT,",
		tape_status1&ILLEGALCMD?"illegal command":"",
		tape_status1&NODATAFOUND?"no tape data":"",
		tape_status1&MAXRETRIES?"max retries":"",
		tape_status1&RESETOCCURRED?"reset happened":"");
	if (sitapefilemark == -1)
		printf("   ***** at file mark: Failed\n");
	else if (tape_status1&BOT && sitapefilemark)
		printf("   ***** at file mark: Not sure????\n");
	else
		printf("   ***** at file mark: %d\n", sitapefilemark);
}

tppp()
{
	printf(" + SQ%d: c:0x%x o:%x filecount:%d tpo:%x st:%x err:%x scc:%d buf:%x\n",
		(tiop->i_unit - 4),
		tiop->i_cmd, tiop->i_option,
		((tiop->i_fileh << 8) | tiop->i_filel),
		tiop->i_tpoption,
		tiop->i_status,
		tiop->i_error, ((tiop->i_scch << 8) | tiop->i_sccl),
		(((tiop->i_bufh << 16) | (tiop->i_bufm << 8)) | tiop->i_bufl));
}

extrapp()
{
	register i;

	if (tiop->i_error)
		printf(" + ERROR iopb stat=%x, iopb err=%x, reg4=%x\n",
			tiop->i_status, tiop->i_error, TSTATUS());
	else
		printf(" + iopb status=%x, reg4=%x\n",
			tiop->i_status, TSTATUS());
	printf(" + Reg 4 at %x, tapeready = %x, tapeint = %x\n",
		st_ioport+ST_R4, TAPERDY(), TRDINT());
/*	printf("tiop at %x, ioaddr at %x\n", tiop, st_ioport); */
}

tapewait()
{
	register timeout = 10000000;
	register i;

	while ((i = TAPERDY()) != 1) {
		--timeout;
		if (!timeout)
			return 1;
	}
	if (verbose)
		extrapp();
	return 0;
}
