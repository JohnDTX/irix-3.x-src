/* manage data structures for blink: peter 8/21/84 */

#include "../h/param.h"
#include "../gl1/grioctl.h"

#define MAXBLINKERS 20

short blinkcount = 0;		/* how many blinkers going now */
struct grblinkcolor blinktab[MAXBLINKERS];

struct grblinkcolor *
blink_alloc(index)
    register int index;
{
    register short count, bcount = blinkcount;
    register struct grblinkcolor *pbc = blinktab;

    for(count = 0; count < bcount; pbc++, count++) {	/* all ready there */
	if( pbc->gr_index == index)
	    return(pbc);
    }

    if(count >= MAXBLINKERS)				/* full already */
	return((struct grblinkcolor *) 0);

							/* new slot needed */
    pbc = &blinktab[blinkcount];
    blinkcount++;
    return(pbc);
}

int
del_blink(index)
    register int index;
{
    register short count, bcount = blinkcount;
    register struct grblinkcolor *pbc = blinktab;

    for(count = 0; count < bcount; pbc++, count++) {	/* find it */
	if( pbc->gr_index == index)
	    break;
    }
    if(count >= bcount || count >= MAXBLINKERS)		/* not there */
	return(-1);
							/* back to original */
    mapcolor(index,pbc->gr_red0,pbc->gr_green0,pbc->gr_blue0);
    if(count < bcount)			/* need to compact */
	*pbc = blinktab[--bcount];
    blinkcount = bcount;
    return(0);
}

do_blink()
{
    register short count, bcount = blinkcount;
    register struct grblinkcolor *pbc = blinktab;

    for(count = 0; count < bcount; pbc++, count++) {
	if(!--(pbc->gr_bticks)) {
	    if(pbc->gr_currently = !pbc->gr_currently)	/* flip/flop */
		mapcolor(pbc->gr_index,
			 pbc->gr_red1,pbc->gr_green1,pbc->gr_blue1);
	    else
		mapcolor(pbc->gr_index,
			 pbc->gr_red0,pbc->gr_green0,pbc->gr_blue0);
	    pbc->gr_bticks = pbc->gr_brate;		/* reset */
	}
    }
}

#ifdef DEBUG_BLINK_STAND_ALONE
main()
{
    register struct grblinkcolor *pbc = blinktab;
    register int i;

    for(i=0;i< MAXBLINKERS + 2; i++) {
	pbc = blink_alloc(i);
	if(pbc) {
	    pbc->gr_index = i;
	    pbc->gr_red0 = i;
	}
    }
    blinkstat();
    del_blink(2);
    blinkstat();
    pbc = blink_alloc(7);
    if(pbc) {
	pbc->gr_index = 7;
	pbc->gr_red0 = 3;
    }
    blinkstat();
    for(i=0;i< MAXBLINKERS + 2; i++) {
	printf("\t%d:0x%08x",i,del_blink(i));
	blinkstat();
    }
    blinkstat();
}

mapcolor(i,r,g,b)
{
printf("\nmapcolor %d %d",i,r);
}

blinkstat()
{
    register int i;

    printf("\n%d ok\n",blinkcount);
    for(i=0;i<MAXBLINKERS;i++)
	printf("\t%d:%d",blinktab[i].gr_index,blinktab[i].gr_red0);
    printf("\n");
}
#endif DEBUG_BLINK_STAND_ALONE

