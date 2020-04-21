/*
 *	loadav -
 *		Display three bars indicating the load average of the 
 *		system.  This program must be owned by root and have 
 *		the set uid bit set to work correctly.
 *
 * 		% make loadav
 * 		% su
 *		# chown root loadav
 *		# chmod u+s loadav
 *
 *				Paul Haeberli - 1984
 *
 */
#include "gl.h"
#include "device.h"
#include "port.h"
#include <sys/types.h>
#include <nlist.h>
#include <stdio.h>

struct	nlist nl[] = {
	{ "_avenrun" },
#define	X_AVENRUN	0
	{ "" },
};

#define fxtod(i)	(((double)vec[i])/1024.0)

int kmem;
long vec[3];
int makeframe();
float cur1, cur2, cur3;
float new1, new2, new3;

/*
 * Get 1, 5, and 15 minute load averages.
 * (Found by looking in kernel for avenrun).
 */
main()
{
    register int i, j;

    if ((kmem = open("/dev/kmem", 0)) < 0) {
	fprintf(stderr, "loadav needs special handling . . \n\n");
	fprintf(stderr, "Do the following commands:\n");
	fprintf(stderr, "    %% su\n");
	fprintf(stderr, "    # chown root loadav\n");
	fprintf(stderr, "    # chmod u+s loadav\n");
	fprintf(stderr, "    # exit\n");
	fprintf(stderr, "    %%\n");
	exit(1);
    }
    nlist("/vmunix", nl);
    if (nl[0].n_type==0) {
	    fprintf(stderr, "No namelist\n");
	    exit(1);
    }
    prefsize(200,70);
    winopen("loadav");
    wintitle("loadav");
    makeframe();
    while (1) {
	drawlav();
	sleep(2);
	redraw(makeframe);
    }
}

drawlav()
{
    lseek(kmem, (long)nl[X_AVENRUN].n_value, 0);
    read(kmem, vec, 3*sizeof(float));
    color(GREY(15));
    clear();
    color(RED);
    rectf(0.0,1.0,fxtod(0),3.0);
    color(GREEN);
    rectf(0.0,4.0,fxtod(1),6.0);
    color(BLUE);
    rectf(0.0,7.0,fxtod(2),9.0);
}


makeframe()
{
    reshapeviewport();
    ortho2(-0.5,6.5,0.0,10.0);
}
