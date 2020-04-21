/*
** netcopy.c	- Copyright (C) Silicon Graphics - Mountain View, CA 94043
**		- Author: paulm (based heavily on sitape.c)
**		- Date: December 1986
**		- Any use, copy or alteration is strictly prohibited
**		- and gosh darn awfully -- morally inexcusable
**		- unless authorized by SGI.
**
** $Author: root $
** $State: Exp $
** $Source: /d2/3.7/src/stand/cmd/sifex/RCS/netcopy.c,v $
** $Revision: 1.1 $
** $Date: 89/03/27 17:13:18 $
*/
#include <sys/types.h>
#include "disk.h"
#include "streg.h"
#include <sys/dklabel.h>
#include "fex.h"
#include "errno.h"

#define	DEV_BSIZE	0x200	/* sector size 512 */
#define	DEV_BMASK	0x1ff	/* for fast modulo by DEV_BSIZE */
#define	DEV_BSHIFT	9	/* for fast divide by DEV_BSIZE */

/*
** Copy a partition image from a remote server to the local disk.
**
** Uses BOOTP to find the server and TFTP to perform the transfer.
*/
netcopy()
{
	register unsigned int count, lba;
	register int rdcount;
	register char *cp;
	register unsigned short *sp = (unsigned short *)BUF0;
	char server[64];
	char filename[64];
	char fullpath[128];
	int tfile;
	int ns, n = 0;
	int swapper = 0;
	int i, status, fd;
	int fs = 0;

	printf(" Remote File to Disk Copy\n");
	/*
	** Get server name and file name
	*/
gethost:
	printf("  Name of remote host? ");
	cp = getline();
	if(!*cp) {
		printf("\n  Host name is required\n");
		goto gethost;
	}
	strcpy(server, cp);
	printf("\n");
getfile:
	printf("  Name of remote file? ");
	cp = getline();
	if(!*cp) {
		printf("\n  File name is required\n");
		goto getfile;
	}
	strcpy(filename, cp);

	/*
	** Check for file names that look like tapes
	*/
	if (strncmp(filename, "/dev/", 5)) {
		/*
		** Doesn't start with /dev/, so probably isn't a tape
		*/
		tfile = 1;
		goto next;
	}

	/*
	** Rather than guess, let's just ask him
	*/
istape:
	printf("\n  Is the remote file a tape (y/n)? ");
	cp = getline();
	if(*cp == 'n' || *cp == 'N') {
		tfile = 1;
		goto next;
	}
	if(*cp != 'y' && *cp != 'Y') {
		printf(" Please enter 'y' for yes, 'n' for no");
		goto istape;
	}
	tfile = 2;
	printf("\n  Tape file (%d)? ", tfile);
	if((i = getnum(10, 1, 20)) != -1)
		tfile = i;
next:
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
		while(((ns + i) * 512) < BUFSIZE)
			ns += i;
	}
	printf("\n  Copying %d blks in chunks of %d blks\n  From remote file %s:%s\n  To si%d%c\n",
		count, ns, server, filename, dunit, fs+'a');
	printf("  Type 'go<return>' to begin...");
	cp = getline();
	printf("\n");
	if(!uleq(cp, "go")) {
		printf("  ABORTED\n");
		return;
	}

	/*
	** Format pathname as expected by standalone io library
	*/
	sprintf(fullpath, "tcp.%s:%s", server, filename);

	if ((fd = open(fullpath, 0)) < 0) {
		printf("  %s: ", fullpath);
		perror("can't open");
		return;
	}

	/*
	** Position tape if required
	*/
	if (--tfile > 0)
		printf("  Skipping %d tape file(s)...\n", tfile);
	while (tfile > 0) {
		do {
			rdcount = ns << DEV_BSHIFT;	/* byte count */
			rdcount = read(fd, (char *)(BUF0), rdcount);
		} while (rdcount > 0);
		close(fd);
		if ((fd = open(fullpath, 0)) < 0) {
			printf("  %s: ", fullpath);
			perror("can't reopen");
			return;
		}
		tfile--;
	}

	printf("\n  *** Copy started....\n");
	while (count) {
		if(ns > count) ns = count;
		rdcount = ns << DEV_BSHIFT;	/* byte count */
		if ((status = read(fd, (char *)(BUF0), rdcount)) != rdcount) {
			if (status < 0) {
				printf("\n  %s: ", fullpath);
				perror("read failed");
				return;
			}
			if (status & DEV_BMASK) {
				printf("\n  %s: read returns %d bytes (not multiple of %d)\n",
					fullpath, status, DEV_BSIZE);
				return;
			}
			ns = status >> DEV_BSHIFT;
			count = ns;	/* EOF now */
		}
		if (wdata(ns, lba, (char *)(BUF0))) {
			explode(lba);
			printf("\n  Write to si%d%c Failed at %d/%d/%d\n",
				dunit, fs+'a', dcyl, dhd, dsec);
			return;
		}
		n++;
		if (swapper) {
	    		if (n)
				printf("%3d ", n);
		} else {
	    		if (n && (n%5) == 0)
				printf("%3d ", n);
			else
				printf(". ");
		}
		count -= ns;
		lba += ns;
	}
	printf("\n");
	if (verbose) {
		explode(lba);
		printf("\n  Copy ended at %d/%d/%d\n", dcyl, dhd, dsec);
	}
	close(fd);
	printf("\n  Remote File to Disk Copy complete\n");
}
