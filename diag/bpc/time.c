/*
 *	Kurt Akeley
 *	5 September 1984
 *
 *	Pixel read/write time tests.
 */

#ifdef UC3
time () {
    printf ("  not implemented for UC3\n");
    }
#endif UC3

#ifdef UC4
#include "console.h"
#include "ucdev.h"
#include "uctest.h"

time (i)
int i;
{
    Save s;
    save (&s);
    switch (i) {
	case 0: {
	    register i;
	    register short *addr;
	    addr = UCCommandAddr (UC_DRAWPIXELAB);
	    LDXS (0);
	    LDXE (0x3ff);
	    LDYS (0);
	    LDYE (0x3ff);
	    printf ("Draw 1,000,000 16-bit pixels with autoincrement ... ");
	    for (i=100000; i>0; i--) {
		*addr = 0x555;
		*addr = 0x555;
		*addr = 0x555;
		*addr = 0x555;
		*addr = 0x555;
		*addr = 0x555;
		*addr = 0x555;
		*addr = 0x555;
		*addr = 0x555;
		*addr = 0x555;
		}
	    printf ("done\n");
	    }
	    break;
	case 1: {
	    register i;
	    register short *addr;
	    short j;
	    addr = UCCommandAddr (UC_READPIXELAB);
	    LDXS (0);
	    LDXE (0x3ff);
	    LDYS (0);
	    LDYE (0x3ff);
	    printf ("Read 1,000,000 16-bit pixels with autoincrement ... ");
	    for (i=100000; i>0; i--) {
		j = *addr;
		j = *addr;
		j = *addr;
		j = *addr;
		j = *addr;
		j = *addr;
		j = *addr;
		j = *addr;
		j = *addr;
		j = *addr;
		}
	    printf ("done\n");
	    }
	    break;
	case 2: {
	    register i;
	    register short *xaddr, *yaddr, *writeaddr;
	    short x, y, data;
	    x = 0;
	    y = 0;
	    data = 0;
	    writeaddr = UCCommandAddr (UC_DRAWPIXELAB_SETADR);
	    xaddr = UCBufferAddr (UC_XSB);
	    yaddr = UCBufferAddr (UC_YSB);
	    printf ("Draw 1,000,000 16-bit pixels at random locations ... ");
	    for (i=100000; i>0; i--) {
		*xaddr = x; *yaddr = y; *writeaddr = data;
		*xaddr = x; *yaddr = y; *writeaddr = data;
		*xaddr = x; *yaddr = y; *writeaddr = data;
		*xaddr = x; *yaddr = y; *writeaddr = data;
		*xaddr = x; *yaddr = y; *writeaddr = data;
		*xaddr = x; *yaddr = y; *writeaddr = data;
		*xaddr = x; *yaddr = y; *writeaddr = data;
		*xaddr = x; *yaddr = y; *writeaddr = data;
		*xaddr = x; *yaddr = y; *writeaddr = data;
		*xaddr = x; *yaddr = y; *writeaddr = data;
		}
	    printf ("done\n");
	    }
	    break;
	case 3: {
	    register i;
	    register short *xaddr, *yaddr, *readaddr;
	    short x, y, data;
	    x = 0;
	    y = 0;
	    readaddr = UCCommandAddr (UC_READPIXELAB_SETADR);
	    xaddr = UCBufferAddr (UC_XSB);
	    yaddr = UCBufferAddr (UC_YSB);
	    printf ("Read 1,000,000 16-bit pixels at random locations ... ");
	    for (i=100000; i>0; i--) {
		*xaddr = x; *yaddr = y; data = *readaddr;
		*xaddr = x; *yaddr = y; data = *readaddr;
		*xaddr = x; *yaddr = y; data = *readaddr;
		*xaddr = x; *yaddr = y; data = *readaddr;
		*xaddr = x; *yaddr = y; data = *readaddr;
		*xaddr = x; *yaddr = y; data = *readaddr;
		*xaddr = x; *yaddr = y; data = *readaddr;
		*xaddr = x; *yaddr = y; data = *readaddr;
		*xaddr = x; *yaddr = y; data = *readaddr;
		*xaddr = x; *yaddr = y; data = *readaddr;
		}
	    printf ("done\n");
	    }
	    break;
	}
    restore (&s);
    }
#endif UC4
