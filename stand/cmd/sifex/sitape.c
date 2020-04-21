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
** $Source: /d2/3.7/src/stand/cmd/sifex/RCS/sitape.c,v $
** $Revision: 1.1 $
** $Date: 89/03/27 17:13:22 $
*/
#include <sys/types.h>
#include "disk.h"
#include "streg.h"
#include <sys/dklabel.h>
#include "fex.h"
#include "tapereg.h"

extern int	tape_flags;
extern int	tape_status0;
extern int	tape_status1;
extern int	tape_status2;
extern int	tape_status3;
extern int	tape_status4;
extern int	tape_status5;

/*
** The new tape commands
*/
tapex()
{
	register unsigned int count, lba;
	register char *cp;
	register unsigned short *sp = (unsigned short *)BUF0;
	int ns, n = 0;
	int tfile = 2;
	int swapper = 0;
	int i, status;
	int fs = 0;

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
		while(((ns + i) * 512) < (BUFSIZE*2))
			ns += i;
	}
	printf("\n");
	printf("  Copying %d blks in %d chunks from tape file %d to si%d%c\n",
		count, ns, tfile, dunit, fs+'a');
	printf("  Type 'go<return>' to begin...");
	cp = getline();
	printf("\n");
	if(!uleq(cp, "go")) {
		printf("  ABORTED\n");
		return;
	}
	printf("  Started\n");
	if ((tape_flags & TAPE_INITED) == 0)
		if (tapeinit()) {
errexit:
			tapestatus();
			printf("  EXIT -- rewinding....\n");
			rewind();
			return;
		}
	if (tapestatus())
		goto errexit;
	if ((tape_status1 & BOT) == 0) {
		printf("  rewinding....\n");
		if (rewind())
			goto errexit;
	}
	if (tfile != 1) {
		printf("  spacing forward %d files....\n", tfile-1);
		if (spacef(tfile-1))
			goto errexit;
	}
	printf("\n  *** Copy started....\n");
	while (count) {
		if(ns > count) ns = count;
		n++;
		if (fs == drivep->label.d_swapfs) {
	    		if (n)
				printf("%3d ", n);
		} else {
	    		if (n && (n%5) == 0)
				printf("%3d ", n);
			else
				printf(". ");
		}
		if (status = rtdata(ns, (char *)(BUF0))) {
			if (status == -1) {
				printf("Read from Tape Failed at lba %d\n",
					lba);
				goto errexit;
			}
			printf("\n  Found filemark -- ");
			if (status)
				ns -= status;
			printf("  Write out additional %d blocks\n", ns);
		}
		if (wdata(ns, lba, (char *)(BUF0))) {
			explode(lba);
			printf("\n  Write to si%d%c Failed at %d/%d/%d\n",
				dunit, fs+'a', dcyl, dhd, dsec);
			goto errexit;
		}
		count -= ns;
		lba += ns;
		if (tape_flags & FILE_MARK_FOUND) {
			tape_flags &= ~(FILE_MARK_FOUND);
			break;
		}
	}
	printf("\n");
	if (verbose) {
		explode(lba);
		printf("  Copy ended at %d/%d/%d\n", dcyl, dhd, dsec);
	}
	printf("  rewinding....\n");
	if (rewind())
		goto errexit;
	printf("\n  Tape to Disk Copy complete\n");
}
