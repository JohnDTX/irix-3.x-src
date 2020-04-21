# include "duart.h"
# define NBBY	8



# define CLOCKFREQ	3686	/* clock freq in khz */
# define BENCHBUZZ	512	/* "several" milliseconds, we hope */
# define CLOCKDIVIDER	16	/* cycles per tick */
# define MINBUZZ	400	/* min buzz per ms, safety feature */
# define MAXBUZZ	900	/* max buzz per ms, safety feature */
short millibuzz,decimillibuzz;

/*
 * calibuzz() --
 * calibrate 1ms buzz count.
 * called at spl7 very BEFORE any calls
 * to msdelay() or clockinit() !
 */
calibuzz()
{
    register duart *dp;
    register long buzzer;
    register unsigned short timercount,duartcount;

    dp = dad[TIMER];

    /* start timer on some big count */
    timercount = 0xFFFF;

    dp->d_sopbc = 0;
    dp->d_acr = 0xBB;
    dp->d_ctu = (unsigned char)(timercount>>NBBY);
    dp->d_ctl = (unsigned char)(timercount>>0);
    buzzer = dp->d_ccgo;

    /* buzz for awhile */
    for( buzzer = BENCHBUZZ; --buzzer >= 0; )
	;

    buzzer = dp->d_ccstp;

    /* find out how many duart ticks elapsed */
    duartcount = ((short)dp->d_ctu<<NBBY) | ((short)dp->d_ctl<<0);

    timercount -= duartcount;
    if( timercount > 0 )	/* just in case */
    {
	/*
	 * convert to nloops per millisecond,
	 * ie, nloops / nmilliseconds.
	 * 1ms ~ CLOCKFREQ/CLOCKDIVIDER duart ticks.
	 */
	millibuzz = (long)(BENCHBUZZ*CLOCKFREQ)
				/((int)timercount*CLOCKDIVIDER);
	decimillibuzz = millibuzz/10;
    }
}

/*
 * msdelay() --
 * msdelay approx n milliseconds.
 */
msdelay(n)
    register int n;
{
    extern short millibuzz;

    register long buzzer;
    register short buzzref;

    buzzref = millibuzz - 2;	/* subtract per-ms overhead */
    if( buzzref < MINBUZZ )
	buzzref = MINBUZZ;
    if( buzzref > MAXBUZZ )
	buzzref = MAXBUZZ;
    buzzer = buzzref - 7;	/* subtract per-call overhead */

    while( --n >= 0 )
    {
	while( --buzzer >= 0 )
	    ;
	buzzer = buzzref;
    }
}

decimsdelay(n)
{
    extern short millibuzz;

    register long buzzer;
    register short buzzref;

    buzzref = decimillibuzz;
    if( buzzref < MINBUZZ/10 )
	buzzref = MINBUZZ/10;
    if( buzzref > MAXBUZZ/10 )
	buzzref = MAXBUZZ/10;
    buzzer = buzzref - 7;	/* subtract per-call overhead */

    while( --n >= 0 )
    {
	while( --buzzer >= 0 )
	    ;
	buzzer = buzzref;
    }
}


# ifdef KLOOP
kloop(n)
    int n;
{
    register long buzzer;

    while( --n >= 0 )
	for( buzzer = 1000-3; --n >= 0; )
	    ;
}
# endif
