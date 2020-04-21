/*
**	memtest.c	- Copyright (C), Silicon Graphics, Inc 1985
**			- Author: chase
**			- Date: Feb 1985
**			- Any use, copying or alteration is prohibited
**			- and morally inexcusable,
**			- unless authorized in writing by SGI.
** $Author: root $
** $State: Exp $
** $Source: /d2/3.7/src/stand/cmd/mdfex/RCS/memtest.c,v $
** $Revision: 1.1 $
** $Date: 89/03/27 17:12:54 $
*/

#include <sys/types.h>
#include <sys/dklabel.h>
#include "disk.h"
#include "dsdreg.h"
#include "fex.h"

int checkstatus;
long lba1;
long lba2;
long errhalt = 1;
unsigned long	cpat[] = {
	0xB1B6DB6D,	0xB1F6DB6D,	0xB1B6D96D,	0xD1D9B6BD,
	0xB16BD6BD,	0xB16FD6BD,	0xB1BD669D,	0xD19DBB6D,
	0x1B6BD6BD,	0x1B6FD6BD,	0xB16B9DD6,	0xDD19B6DB,
};
long	cpati;

#undef	DEBUG
#undef 	STATUS
#undef	TEST
#undef INTERRUPT
memtest()
{
	register struct disk_label *l = &drivep->label;
	register long *addr = (long *)BUF1;
	register char *cp;
	register long count = (BUFSIZE/4);
	register unsigned long pattern, loop, i;

	screenclear();
	printf("               *** Memory Test with Multibus Activity ***\n");
	/*
	** Find out the swap area of the disk
	** And set up the write and read areas.
	** Use buf0 for the disk I/O and buf1 for the Memory activity.
	*/
	i = l->d_swapfs;
	lba1 = l->d_map[i].d_base;
	lba2 = (l->d_map[i].d_base + (l->d_map[i].d_size-(HD*SEC)));
#ifdef NOTDEF
	explode(lba1);
	printf("lba1 %d/%d/%d ", dcyl, dhd, dsec);
	explode(lba2);
	printf("lba2 %d/%d/%d\n", dcyl, dhd, dsec);
	if (lba1 < 150*119)
		printf("lba1 %d < %d\n", lba1, 150*119);
	if (lba2 > 300*119)
		printf("lba2 %d > %d\n", lba1, 300*119);
#endif
	seterr();
	printf(" type 'go<return>' to start test...");
	cp = getline();
	if(!uleq(cp, "go")) {
		printf(" Aborted.\n");
		return;
	}
	printf("\n");
	/*
	** Write and read from the Swap area of the disk
	** Do this in interrupt mode.
	** Start the first operation and let the interrupt
	** code do the rest.
	*/
	cpati = 0;
	loop = 0;
	/*
	** Right now just do one lba
	*/
	explode(lba1);
	setup();
	checkstatus = 0;
#ifdef TEST
	printf("\nSTART: %x\n", spl0());
	diskintr();
	checkstatus = 1;
	for(;;) {
		if((i = nwgch()) != -1) {
			if(i == 'q') {
				printf("Quit\n");
				spl5();
				return;
			}
		}
	}
#endif
#ifdef STATUS
	spl5();
	for(;;) {
		diskintr();
		if((i = nwgch()) != -1) {
			if(i == 'q') {
				printf("Quit\n");
				return;
			}
		}
	}
#endif
	diskintr();
	checkstatus = 1;
	/*
	** Memory test for BUF1
	*/
	for (;;) {
		/*
		** Reset the pointer and count.
		** Write out the pattern.
		*/
		count = (BUFSIZE/sizeof(long));
		pattern = cpat[cpati % 12];
		cpati++;
		filll(BUF1, pattern, count);
		/*
		** Compare the data
		*/
		if (cmpl(BUF1, pattern, count)) {
			printf("Compare error: Address 0x%x ", clea);
			printf(" Wrote 0x%x, Read 0x%x (%x)\n",
				pattern, cleis, *(long *)clea);
			if (errhalt) {
				switch(rsq(0)) {
					case RETRY: continue;
					case SKIP:  continue;
					case QUIT:  return;
				}
			}
		}
	    	if(loop && (loop%10) == 0)
		/*	QP2("%4d(%x) ",loop, spl0()); */
			QP1("%4d ",loop);
		loop++;
		if((i = nwgch()) != -1) {
			if(i == 'q') {
				printf("Quit\n");
				spl5();
				diskclean();
				return;
			}
		}
	}
}

diskintr()
{
	/*
	** Check for your interrupt
	*/
	if (checkstatus && (cib->i_stsem == 0)) {
		printf("Panic: Not my interrupt");
		halt();
	}
	if(cib->i_opstat & HARD) {
		printf("mdfex%d: Error OpSt %x, ", dunit, (UCHAR)cib->i_opstat);
		mgstatus();
	}
	cib->i_stsem = 0;
	CLEAR();
#ifdef DEBUG
	if (checkstatus)
		printf(".");
#endif
	/*
	** After the interrupt start the next operation
	*/
#ifdef NOTDEF
/*	setup();
**
**	explode(lba1);
**	if (iop->p_cyl == dcyl) {
**		explode(lba2);
**		iop->p_func = F_READ;
**	} else {
**		iop->p_func = F_READ;
**	}
**	iop->p_cyl	= dcyl;
**	iop->p_sec	= dsec;
**	iop->p_hd	= dhd;
*/
#endif

	if (wbusy()) {
		printf("poop\n");
		mdpp();
		halt();
	}
	cib->i_opstat = 0;
#ifdef NOTDEF
	mdpp();
#endif NOTDEF
#ifdef STATUS
	iop->p_mod = M_NOINT;
	START();
	if (wstatus()) {
		printf("poop 2\n");
		mdpp();
		return 1;
	}
#endif	STATUS
	iop->p_mod = 0;
	START();
}

#define SIZE	((HD*SEC)*(secsize*2))
setup()
{
	register i = SIZE;

	iop->p_func	= F_READ;
	iop->p_mod	= 0;
	iop->p_unit	= dunit;
	iop->p_dev	= drivep->tdev;
	iop->p_rbc	= SWAPW(i);
	iop->p_atc	= 0;
	iop->p_dba	= INTEL(BUF0);
	iop->p_cyl	= dcyl;
	iop->p_sec	= dsec;
	iop->p_hd	= dhd;
	return;
}

seterr()
{
	register char *cp;

	printf("Stop on Error (%s)? ", errhalt?"yes":"no");
	cp = getline();
	if(*cp) {
		if(uleq(cp, "yes"))
			errhalt = 1;
		else if(uleq(cp, "no"))
			errhalt = 0;
		else
			printf(" enter 'yes' or 'on'");
	}
	printf("\n");
}

diskclean()
{
	sd_flags = 0;
	CLEAR();
	config();
}
