/*
 *	 		Quirk clock device functions 
 *
 *		      Paul Haeberli and David J. Brown
 *				June 20, 1983
 */
#include "duart.h"		/* PM2 uart definitions */

#include "Qdevices.h"		/* Quirk defines */
#include "Qglobals.h"


# define NBBY		8
#define	TIMERCOUNT	1843	/* number of ticks per 1 ms */
#define DCD_MASK OP5BIT

int alarmcount;
long Q10ticks;
char clock1ms, clock10ms, clock100ms, clock1s;


/*
** 	clockinit - set up a 1ms timer interval
**
*/
clockinit()
{
    register char tmp;
    register duart *dp;

    clock1ms = 10;
    clock10ms = 10;
    clock100ms = 10;
    
    /* Initialize the timer for 1 ms ticks */
    /* set clock to run as a timer using duart clock frequency */
    dp = dad[TIMER];

    /* this "should" be done dp->d_sopbc = 0; */

    dp->d_acr = ACR|0x60;

    /* set the number of ticks per millisecond */
    dp->d_ctu = (unsigned char)(TIMERCOUNT>>NBBY);
    dp->d_ctl = (unsigned char)(TIMERCOUNT>>0);

    /* start counting (touch the magic place to start it up) !! */
    tmp = dp->d_ccgo;
}



/*
**	alarm - set the alarm to go off sometime
**
*/
alarm( clicks )
int clicks;
{
    alarmcount = clicks;
    MAKENOTREADY(ALARM);
}

/*
**	lbolt - this is the clock interrupt 
**
*/

char last_dcd_state;	/* keeps the last dcd state of the console port */

lbolt()
{
    register char tmp;


    if( alarmcount != 0 )
    if( --alarmcount == 0)
	MAKEREADY(ALARM);

    MAKEREADY(CLOCK1MS);
    if( --(clock1ms) == 0 )
    {
	clock1ms = 10;
	SCHED(clk10ms);
    }

    tmp = dad[TIMER]->d_ccstp;		/* clear the timer interrupt */
}

/*
**	clk10ms - 10 millisecond click 
**
*/
clk10ms()
{
    Q10ticks++;
    MAKEREADY(CLOCK10MS);
    if( --clock10ms == 0 )
    {
	clock10ms = 10;
	SCHED(clk100ms);
    }
}

/*
**	clk100ms - 100 millisecond click
**
*/
clk100ms()
{
    MAKEREADY(CLOCK100MS);
    if( --clock100ms == 0 )
    {
	clock100ms = 10;
	SCHED(clk1s);
    }
}

/*
**	clk1s - 1 second click
**
*/
clk1s()
{
    Qtime++;
    MAKEREADY(CLOCK1S);
/*    putc('.',LOCAL);*/
}
