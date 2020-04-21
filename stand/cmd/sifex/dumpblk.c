#include "types.h"
#include <sys/dklabel.h>
#include "disk.h"
#include "streg.h"
#include "fex.h"

/*
 * nod:
 *	- dump a file, just like the kernel does
 */
dumpblk()
{
	long lba;
	char *cp;

	printf("\nDump block on disk\n");
	do {
		printf("\nWhich block for Reading (cyl/hd/sec)? ");
		cp = getline();
		if(*cp == 0) {
			printf(" Quit\n");
			return;
		}
		else
			lba = bparse(cp, 1);
	} while(lba == -1);
	printf("\n");
	rdata(1, lba, (char *)BUF0);
	dumpmemory((char *)BUF0, 512);
}

dumpmemory(address, count)
	char *address;
	u_short count;
{
	register int i;
	register u_short *sp;
	register int offset;
	char buf[18];

	sp = (u_short *)address;
	offset = 0;
	buf[16] = 0;
	while (offset < count) {
		bcopy(sp, buf, 16);
		printf("%06x: %04x %04x %04x %04x %04x %04x %04x %04x ",
			      sp, *sp,
			      *(sp + 1), *(sp + 2),
			      *(sp + 3), *(sp + 4),
			      *(sp + 5), *(sp + 6),
			      *(sp + 7));
		for (i = 0; i < 16; i++)
			if ((buf[i] < ' ') || (buf[i] >= 0x7f))
				buf[i] = '.';
		printf("%s\n", buf);
		offset += 16;
		sp += 8;
	}
}
