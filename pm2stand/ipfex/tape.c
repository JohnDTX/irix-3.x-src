/*
** tape.c 	- For IPFEX
**
** $Date: 89/03/27 17:11:26 $
** $State: Exp $
** $Source: /d2/3.7/src/pm2stand/ipfex/RCS/tape.c,v $
** $Author: root $
** $Revision: 1.1 $
** $Log:	tape.c,v $
 * Revision 1.1  89/03/27  17:11:26  root
 * Initial check-in for 3.7
 * 
 * Revision 1.1  85/08/26  16:32:09  root
 * Initial revision
 * 
*/

#include <sys/types.h>
#include "disk.h"
#include "ipreg.h"
#include "dsdreg.h"
#include <sys/dklabel.h>
#include "test.h"

#undef TAPEDEBUG

#ifdef TAPEDEBUG
char extra = 1;
#endif

/*
 * The new tape commands
 */
tapex()
{
	int tfile = 2;
	register i;
	int fs = 1;		/* NEW SWAP STUFF */
	int swapper = 0;
	register unsigned int count, lba;
	register char *cp;
	register unsigned short *sp = (unsigned short *)BUF0;
	int ns, n = 0;

	printf("  Tape to Disk Copy\n");
	printf("  Tape file (%d)? ", tfile);
	if((i = getnum(10, 1, 20)) != -1)
		tfile = i;
	printf("\n");
	unitd();
getfs:
	printf("  File system (%c)? ", fs+'a');
	cp = getline();
	if(*cp) {
		i = *cp - 'a';
		if(i < 0 || i > 7) {
			printf(" Invalid; enter a-h.\n");
			goto getfs;
		}
		fs = i;
	}
	lba = drivep->label.d_map[fs].d_base;
	if (fs == drivep->label.d_swapfs) {
		swapper = 1;
		count = 2000;		/* NEW SWAP STUFF */
		ns = 500;
	} else {
		swapper = 0;
		count = drivep->label.d_map[fs].d_size;
		i = ns = drivep->label.d_heads * drivep->label.d_sectors;
		while(((ns + i) * 512) < (BIGBUFSIZE))
			ns += i;
		if((ns * 512) > BIGBUFSIZE)
			ns = BIGBUFSIZE/512;
	}
	printf("\n");
	printf("  Copying %dK in %dK chunks from tape file %d to ip%d%c\n",
		count/2, ns/2, tfile, dunit, fs+'a');
	printf("  Type 'go<return>' to begin...");
	cp = getline();
	printf("\n");
	if(!uleq(cp, "go")) {
		printf("  ABORTED\n");
		return;
	}
	printf("  Started\n");
	rewind();			/***/
	tapestatus();
#ifdef TAPEDEBUG
	printstatus();
#endif
	if(tfile != 1)
		spacef(tfile-1);
	printf("  Copy started....\n");
	while(count) {
		i = 0;
		if(ns > count) ns = count;
		if(rtdata(ns, (char *)(BUF0))) {
			printf("Read from Tape Failed at lba %d\n", lba);
			return;
		}
wagain:
		if(wdata(ns, lba, (char *)(BUF0))) {
			explode(lba);
			if(++i == 5) {
				printf("Write to ip%d%c Failed at %d/%d/%d\n",
					dunit, fs+'a', dcyl, dhd, dsec);
				ip_flags = 0;	/* Force reset */
				return;
			}
			goto wagain;
		}
		n++;
		if (fs == drivep->label.d_swapfs)
	    		if (n) QP1("%3d ",n);
		else
	    		if(n && (n%2) == 0) QP1("%3d ",n);
		count -= ns;
		lba += ns;
	}
	printf("\n");
	md_flags = 0;	/* Force ctlr reset */
	tapestatus(1);	/* Force tape reset */
	rewind();	/* Now, rewind the tape */
	printf("\n  Tape to Disk Copy complete ***\n");
}

#ifndef GOODSTUFF
ttcmp(tapebuf, diskbuf, ns, lba)
long *tapebuf, *diskbuf;
register ns, lba;
{
	register long *tp = tapebuf;
	register long *dp = diskbuf;
	register errs = 0;

	ns *= 512/4;
	ns--;
	do {
	    if(*tp++ != *dp++) {
		--tp; --dp;
		explode(lba);
		printf("Data miscompare: xfer start: %d/%d/%d ",
			dcyl, dhd, dsec);
		lba += (tp - tapebuf)/128;
		explode(lba);
		printf("err @ %d/%d/%d\n", dcyl, dhd, dsec);
		printf("Offset from start 0x%x: t=%x -> d=%x\n", (tp-tapebuf)*4,
			*tp, *dp);
		if(++errs > 10)
			return 1;
		tp++; dp++;
	    }
	} while(--ns != -1);
	return(errs);
}
#endif

tapeops()
{
	printf("Tape operations\n");
	for(;;) {
		printf("TAPE> ");
		switch(gch()) {
		case 'e':
			printf("Erase\n");
			erase();
			break;
		default:
		case 'h':
			tapehelp();
			break;
		case '\n':
			printf("\n");
			break;
		case 'q':
			printf("Quit\n");
			return;
		case 'r':
			printf("Rewind\n");
			rewind();
			break;
		case 's':
			printf("Read the Status of the Tape\n");
			tapestatus();
			printstatus();
			break;
		case 't':
			printf("Tension\n");
			tension();
			break;
		case 'Q':
			printf("Quick Check for Operation Status\n");
			opcheck();
			break;
		case 'R':
			printf("Read Write Test\n");
			taperw();
			break;
		}
	}
}

tapehelp()
{
	printf("Tape commands:\n");
	printf("  e..rase	- Erase Tape\n");
	printf("  h..elp	- Help, this message\n");
	printf("  q..uit	- quit\n");
	printf("  r..ewind	- Rewind Tape\n");
	printf("  s..tatus      - Read Status with NO RE-INIT of Controller\n");
	printf("  t..ension     - Retension Tape\n");
	printf("  Q..uick	- Check the Operation Status\n");
	printf("  R..ead Write	- Read Write Tape Test\n");
	printf("  S..tatus      - Read Status with a RE-INIT of Controller\n");
	printf("\n");
}

/*
** Tape read or write exercise()
*/
taperw()
{
	register unsigned long loop, ns;
	register lba = 0;
	unsigned long cmd, c;
	register char *cp;

	printf("TAPE READ OR WRITE TEST\n");
	printf("Repeat how many blocks to transfer? ");
	loop = getnum(10, 1, 500000);
	printf(" ");
	if(loop == -1) {
		loop = 500000;
		printf("forever ");
	}
	printf("\n");
t1:	printf("Tape read or write (r/w/q)? ");
	switch(gch()) {
		case 'q': printf("  Quit\n"); return;
		case 'r': printf("  READ\n"); cmd = 0; break;
		case 'w': printf("  WRITE\n"); cmd = 1; break;
		default:  printf("  Answer r/w/q please\n"); goto t1;
	}
	printf("Tape test for %s\n", (cmd == 0)? "READING": "WRITING");
	do {
		printf("# of sectors (%d bytes each): ", secsize);
		ns = getnum(10, 1, 256);
	} while(ns == -1);
	printf("\n");
	if(tapestatus()) {
		printf("Taperw(): Failure on reading status\n");
		return;
	}
	printstatus();
	if(!tpstatus->tb[2]) {
		printf("Tape: not at BOT > REWINDING\n");
		if(rewind()) {
			printf("Taperw() Failure on rewind\n");
			return;
		}
	}
	if(!ip_flags){
		printf("Disk is not initialized\n");
		return;
	}
	while(loop--) {
		if(cmd == 0) {	/* READ */
			if(rtdata(ns, (char *)(BUF0))) {
				printf("Error on reading the Tape\n");
				mdpp();
				return;
			}
		} else {	/* WRITE */
			/*
			 * Read from the Disk
			 */
			if(rdata(ns, lba, (char *)(BUF0))) {
				printf("Error on reading the Disk\n");
				mdpp();
				return;
			}
			/*
			 * Write to the Tape
			 */
			if(wtdata(ns, (char *)(BUF0))) {
				printf("Error on writing to the Tape\n");
				mdpp();
				return;
			}
		}
		printf(".");
		if((c = nwgch()) != -1)
			if(c == ' ')
				printf("At %d\n", lba);
			else
				return;
		lba += ns;
		if(lba <  0 || lba >= CYL*HD*SEC) printf("Pass\n");
	}
	printf("\nRead/Write Tape test is complete\n");
}

opcheck()
{
	register i;

	i = quickcheck();
	printf("\nThe Operation status is %x \n", (i == 0xdef)? 0: i);
}
