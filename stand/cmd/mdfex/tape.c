/*
** tape.c	- Copyright (C) Silicon Graphics - Mountain View, CA 94043
**		- Author: chase
**		- Date: April 1985
**		- Any use, copy or alteration is strictly prohibited
**		- and gosh darn awfully -- morally inexcusable
**		- unless authorized by SGI.
**
** $Author: root $
** $State: Exp $
** $Source: /d2/3.7/src/stand/cmd/mdfex/RCS/tape.c,v $
** $Revision: 1.1 $
** $Date: 89/03/27 17:12:58 $
*/
#include <sys/types.h>
#include "disk.h"
#include "dsdreg.h"
#include <sys/dklabel.h>
#include "fex.h"

#ifdef TAPEDEBUG
char extra = 1;
#endif


/*
** The new tape commands
*/
tapex()
{
	int tfile = 2;
	int swapper = 0;
	register i;
	int fs = 0;
	register unsigned int count, lba;
	register char *cp;
	register unsigned short *sp = (unsigned short *)BUF0;
	int ns, n = 0;

	printf(" Tape to Disk Copy\n");
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
		count = 6000;		/* NEW SWAP STUFF */
		ns = 500;
	} else {
		swapper = 0;
		count = drivep->label.d_map[fs].d_size;
		i = ns = drivep->label.d_heads * drivep->label.d_sectors;
		while(((ns + i) * 512) < (BIGBUFSIZE))
			ns += i;
	}
	printf("\n");
	printf("  Copying %d blks in %d chunks from tape file %d to md%d%c\n",
		count, ns, tfile, dunit, fs+'a');
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
	printf("\n  Copy started....\n");
	while (count) {
		if(ns > count) ns = count;
		n++;
		if (fs == drivep->label.d_swapfs) {
	    		if (n)
				printf("%3d ",n);
		} else {
	    	/*	if (n) */
	    		if (n && (n%5) == 0)
				printf("%3d ",n);
		}
		if(rtdata(ns, (char *)(BUF0))) {
			printf("Read from Tape Failed at lba %d\n", lba);
			return;
		}
		if(wdata(ns, lba, (char *)(BUF0))) {
			explode(lba);
			printf("Write to md%d%c Failed at %d/%d/%d\n",
				dunit, fs+'a', dcyl, dhd, dsec);
			return;
		}
		count -= ns;
		lba += ns;
	}
	printf("\n");
#ifdef NOTDEF
	explode(lba);
	printf(" Copy ended at %d/%d/%d\n", dcyl, dhd, dsec);
#endif NOTDEF
	sd_flags = 0;	/* Force ctlr reset */
	tapestatus();	/* Force tape reset */
	rewind();	/* Now, rewind the tape */
	printf("\n  Tape to Disk Copy complete\n");
}

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
		case 'S':
			printf("Init/Read the Status of the Tape\n");
			tapestatus();
			printstatus();
			break;
		}
	}
}

tapehelp()
{
	printf("Tape commands:\n");
	printf("  e..rase	- Erase Tape:%d\n", tunit);
	printf("  h..elp	- Help, this message\n");
	printf("  q..uit	- quit\n");
	printf("  r..ewind	- Rewind Tape:%d\n", tunit);
	printf("  s..tatus      - Read Status with NO RE-INIT of Controller\n");
	printf("  t..ension     - Retension Tape:%d\n", tunit);
	printf("  Q..uick	- Check the Operation Status\n");
	printf("  R..ead Write	- Read Write Tape:%d Test\n", tunit);
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
			printf("Taperw() Failure on rewind()\n");
			return;
		}
	}
	config();
	while(loop--) {
		if(cmd == 0) {	/* READ */
			if(rtdata(ns, (char *)(BUF0))) {
				printf("Error on reading the Tape\n");
				tdpp();
				return;
			}
		} else {	/* WRITE */
			/*
			 * Read from the Disk
			 */
			if(rdata(ns, lba, (char *)(BUF0))) {
				printf("Error on reading the Disk\n");
				tdpp();
				return;
			}
			/*
			 * Write to the Tape
			 */
			if(wtdata(ns, (char *)(BUF0))) {
				printf("Error on writing to the Tape\n");
				tdpp();
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
	printf("\n  Operation status=0x%x\n", cib->i_opstat);
}
