/*
 * $Source: /d2/3.7/src/sys/ipII/RCS/tod.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:31:02 $
 */
#include	"../h/types.h"
#include	"../h/systm.h"
#include	"../ipII/cpureg.h"
#include	"../ipII/tod.h"

/*
** This module handles the I/O specific to the Motorola MC146818A
** CMOS real-time clock + RAM.
** Hardware constrains us to the 32.768kHz time base.
** I/O is very bitchy in that we must drive the lines ourself.
*/

/*
** Register A values (r/w except for UIP)
*/
#define	RA_UIP		0x80	/* update in progress		*/
#define	RA_DV4MHZ	0x00	/* 4.194304 MHz time base	*/
#define	RA_DV1MHZ	0x10	/* 1.048576 MHz time base	*/
#define	RA_DV32KHZ	0x20	/* 32.768 kHz time base		*/
#define	RA_RS64HZ	0x0a	/* 64 Hz periodic interrupt rate*/
				/*  for any time base		*/

/*
** Register B values (r/w)
*/
#define	RB_SET		0x80	/* disable/enable time updates	*/
				/*  (disabled if set)		*/
#define	RB_PIE		0x40	/* enable periodic interrupts	*/
#define	RB_AIE		0x20	/* enable alarm interrupts	*/
#define	RB_UIE		0x10	/* enable end of update interval*/
#define	RB_SQWE		0x08	/* enable square wave output	*/
#define	RB_DMBIN	0x04	/* data mode - binary		*/
#define	RB_DMBCD	0x00	/* data mode - bcd		*/
#define	RB_HR24		0x02	/* hour format - 24 hour	*/
#define	RB_HR12		0x00	/* hour format - 12 hour	*/
#define	RB_DSE		0x01	/* enable daylight savings mode	*/

/*
** Register C values (read only)
*/
#define	RC_IRQF		0x80	/* interrupt request:		*/
				/*  IRQF = PF*PIE + AF*AIE + UF*UIE	*/
#define	RC_PF		0x40	/* periodic interrupt flag	*/
#define	RC_AF		0x20	/* alarm interrupt flag		*/
#define	RC_UF		0x10	/* update-ended interrupt flag	*/

/*
** Register D values (read only)
*/
#define	RD_VRT		0x80	/* valid RAM and time bit	*/
				/*  set by reading register D	*/
/*
** defines for the device status
*/
#define	TD_TODRUN	0x01	/* set if time is running	*/
#define	TD_CLKRUN	0x02	/* set if clock is running	*/
#define	TD_VALID	0x04	/* set if device holds valid info	*/

/*
** defines for driving the addressing logic. (see the IP2 spec)
*/
#define	TOD_AS		0x01	/* address strobe		*/
#define	TOD_DS		0x02	/* data strobe			*/
#define	TOD_RDENAB	0x04	/* read enable			*/
#define	TOD_DRVCLK	0x08	/* drive clock to MC146818A	*/

/*
** misc defines
*/
#define	MEMBERADR(f)	((char)(&(((struct tod_dev *)0)->f))&0x3f)

#define	READFIELD(f,v)	{ u_short sr;\
			sr = splmax();\
			*CLK_DATA = MEMBERADR(f);\
			*CLK_CTL = TOD_RDENAB | TOD_DRVCLK | TOD_AS;\
			*CLK_CTL = TOD_RDENAB | TOD_DRVCLK;\
			*CLK_CTL = TOD_RDENAB | TOD_DS;\
			v = *CLK_DATA; *CLK_CTL = TOD_RDENAB; *CLK_CTL = 0;\
			splx(sr);\
			}
#define	WRITEFIELD(f,v) { u_short sr;\
			sr = splmax();\
			*CLK_DATA = MEMBERADR(f);\
			*CLK_CTL = TOD_DRVCLK | TOD_AS;\
			*CLK_CTL = TOD_DRVCLK;\
			*CLK_DATA = v;\
			*CLK_CTL = TOD_DRVCLK | TOD_DS;\
			*CLK_CTL = TOD_DRVCLK | TOD_DS;\
			*CLK_CTL = TOD_DRVCLK;\
			*CLK_CTL = 0;\
			splx(sr);\
			}

/* converts year to number of days */
#define	DAYSPYR(x)	(((x) % 4 ) ? 365 : 366 )

/* holds number of days for each month */
int	dayspmon[ 12 ] =
{
	31,	28,	31,	30,
	31,	30,	31,	31,
	30,	31,	30,	31
};

struct tod_dev	tod_dev;	/* holds last read/set values for device */
char		todstat;	/* holds status of the device		 */

/*
** todclkstart
**  start the clock.
**  We just enable the periodic interrupts and *POOF* interrupts occur.
*/
todclkstart()
{
	tod_dev.td_regB |= RB_PIE;	/* enable periodic intrs	*/
	WRITEFIELD( td_regB, tod_dev.td_regB )	/* tell the chip	*/
	todstat |= TD_CLKRUN;		/* update internal state	*/
}

/*
** todclkstop
**  stop the clock.
**  Disable the periodic interrupts.
*/
todclkstop()
{
	tod_dev.td_regB &= ~RB_PIE;	/* disable periodic intrs	*/
	WRITEFIELD( td_regB, tod_dev.td_regB )	/* tell the chip	*/
	todstat &= ~TD_CLKRUN;		/* update internal state	*/
}

/*
** todclkset
**  Set the systems idea of the correct time.
**  If the chip has been determined to have valid time
**  then use its concept of the time.  Otherwise use the time
**  supplied to us.
**  NOTE: the chip was initialized previously by a call to todinit() in
**        mlsetup().
*/
todclkset()
{
	extern time_t	todgettim();
	static char first_time = 1;
	register time_t newtime;

	if ((todstat & (TD_VALID|TD_TODRUN)) == (TD_VALID|TD_TODRUN)) {

		/*
		 * Try to get time from chip.  We do run non-interruptible
		 * here to try to insure success when reading the clock
		 * registers.  However, a large dma transfer can cause the
		 * cpu to effectively be blocked from execution and cause
		 * the time values read from the clock registers to be
		 * inconsistent.
		 *
		 * To solve this problem, the following is done.  We read
		 * the time from the chip and compare it to the system
		 * hertz counter (time).  If the real time is more than
		 * a second out of phase with the hertz counter, then we
		 * don't believe it.  If the real time is the same or
		 * a second larger then we believe it, as the hertz clock
		 * can slide a bit.  Basically, we trust the hertz counter
		 * and distrust the reading of the real time clock.
		 */
		if (first_time) {
			register time_t time1;
			register int i, j;

			/*
			 * Believe the hardware clock the first time.
			 * Since we don't have a reference to compare the
			 * real time against, read it twice with a delay
			 * between the first and second samples.  If the
			 * time is very different, then keep trying.
			 */
			j = 0;
			for (;;) {
				time1 = todgettim();
				for (i = 100000; --i >= 0; )
					continue;	/* spin */
				newtime = todgettim();
				if (newtime >= time1 && time1+2 >= newtime)
					break;
				if (++j > 10) {		/* quit eventually */
					todstat &= ~TD_VALID;
					break;
				}
			}
			/*
			 * if the chip is in the past hour or the next 
			 * quarter, do not trust it.
			 */
			 if (newtime < time-60*60
			     || time+60*60*24*30*3 < newtime)
				todstat &= ~TD_VALID;

		} else {
			/*
			 * If hz clock ever drifts more than 2
			 * seconds off of the real time clock,
			 * this will never do anything.  Probably
			 * just as well anyway.
			 */
			newtime = todgettim();
			if (newtime < time-2 || newtime > time+2)
				todstat &= ~TD_VALID;
		}
	}

	if ((todstat & (TD_VALID|TD_TODRUN)) == (TD_VALID|TD_TODRUN)) {
		if (time-20 < newtime || time+20 > newtime) {
			time = newtime;
		} else {
			adjtime_sub((newtime-time)*1000000);
		}

	} else {
		todsettim(time);	/* fix chip if it was wrong */
	}
	first_time = 0;
}

/*
** misc routines
*/

/*
** todinit
**  initialize the device
**  Sets the info into the status flag as to the current state of the device
**  and initializes the registers.
*/
todinit()
{
	READFIELD( td_btflg, tod_dev.td_btflg )
	READFIELD( td_pwrflg, tod_dev.td_pwrflg )
	READFIELD( td_wtimflg, tod_dev.td_wtimflg )

	if ( ( tod_dev.td_pwrflg & P_BADTIME ) != P_BADTIME )
		todstat = TD_VALID;

	/* 32.768Khz time base, 64hz clock */
	tod_dev.td_regA = RA_DV32KHZ | RA_RS64HZ;

	WRITEFIELD( td_regA, tod_dev.td_regA )

	/*
	** get current value in register B
	*/
	READFIELD( td_regB, tod_dev.td_regB )

	/*
	** update register B value:
	** data mode: binary, hour mode: 24 hour: enable square wave output
	** no interrupts and no daylight savings time.  If clock was not
	** running the (|=) keeps it from not running.
	*/
	tod_dev.td_regB |= RB_DMBIN | RB_HR24 | RB_SQWE;
	tod_dev.td_regB &= ~(RB_PIE | RB_AIE | RB_UIE | RB_DSE );

	WRITEFIELD( td_regB, tod_dev.td_regB )

	/* update device status */
	if ( ( tod_dev.td_regB & RB_SET ) != RB_SET )
		todstat |= TD_TODRUN;
}

/*
** todsettim
**   initialize the time of day clock.
**   It is set to GMT (doesn't know about daylight savings time)
*/
todsettim( curtime )
register time_t	curtime;
{
	int	day,
		hms,
		d1;

	/* turn off updating the tod */
	READFIELD( td_regB, tod_dev.td_regB )
	tod_dev.td_regB |= RB_SET;
	WRITEFIELD( td_regB, tod_dev.td_regB )

	/*
	** now calc the time as the tod clock needs it
	*/
	hms = curtime % 86400;
	day = curtime / 86400;

	/* calc and seconds, minutes, hours */
	tod_dev.td_sec = (char)( hms % 60 );
	d1 = hms / 60;
	tod_dev.td_min = (char)( d1 % 60 );
	tod_dev.td_hrs = (char)( d1 / 60 );

	/*
	** calc day of the week (we add 4 since Jan 1, 1970
	** was a thursday
	*/
	tod_dev.td_dow = (char)( ( ( day + 4 ) % 7 ) + 1 );

	/* now the year number */
	for ( d1 = 70; day >= DAYSPYR( d1 ); d1++ )
		day -= DAYSPYR( d1 );
	tod_dev.td_year = (char)d1;

	/* calc the # days for feb. */
	if ( DAYSPYR( d1 ) == 366 )
		dayspmon[ 1 ] = 29;

	/* calc month and day of month	*/
	for ( d1 = 0; day >= dayspmon[ d1 ]; d1++ )
		day -= dayspmon[ d1 ];

	dayspmon[ 1 ] = 28;	/* reset feb. # of days	*/
	tod_dev.td_month = (char)( d1 + 1 );
	tod_dev.td_dom = (char)( day + 1 );

	/* turn alarms off */
	tod_dev.td_secalrm = tod_dev.td_minalrm = tod_dev.td_hrsalrm = 0x00;

	WRITEFIELD( td_sec, tod_dev.td_sec )
	WRITEFIELD( td_secalrm, tod_dev.td_secalrm )
	WRITEFIELD( td_min, tod_dev.td_min )
	WRITEFIELD( td_minalrm, tod_dev.td_minalrm )
	WRITEFIELD( td_hrs, tod_dev.td_hrs )
	WRITEFIELD( td_hrsalrm, tod_dev.td_hrsalrm )
	WRITEFIELD( td_dow, tod_dev.td_dow )
	WRITEFIELD( td_dom, tod_dev.td_dom )
	WRITEFIELD( td_month, tod_dev.td_month )
	WRITEFIELD( td_year, tod_dev.td_year )

	tod_dev.td_regB &= ~RB_SET;
	WRITEFIELD( td_regB, tod_dev.td_regB )

	/*
	** update global status:
	**	turn off tod invalid flag. turn on tod is valid and running
	*/
	todstat |= ( TD_VALID | TD_TODRUN );

	/*
	** Update status of the tod in the static ram. Time is set
	*/
	tod_dev.td_pwrflg &= ~P_BADTIME;
	WRITEFIELD( td_pwrflg, tod_dev.td_pwrflg )
}

/*
** todgettim()
**   get the current time from the chip.
**   Its concept of time must be convert to the systems concept: # of seconds
**   elapsed from the epoch.
*/
time_t
todgettim()
{
	short		sr;
	register time_t	age;
	register char	val1;
	char		year,
			month,
			dom,
			hrs,
			mins,
			secs;
	int		i,
			ydays;

	/*
	** if we checked the UIP bit we would have only 244 usec
	** in which to read info.  We have no guarantee that an
	** update did not occur between any of the reads.  We shut
	** off all interrupts and wait for a clear UIP.  In this manner
	** no interrupts can come along and steal any of our 244 usec.
	*/
	sr = splmax();
	for ( ;; )
	{
		READFIELD( td_regA, val1 )
		if ( ! ( val1 & RA_UIP ) )
			break;
	}
	READFIELD( td_year, year )
	READFIELD( td_month, month )
	READFIELD( td_dom, dom )
	READFIELD( td_hrs, hrs )
	READFIELD( td_min, mins )
	READFIELD( td_sec, secs )

	splx( sr );	/* finished reading */

	/*
	** now we must convert the info to seconds from the epoch
	*/

	/* seconds before this year (excludes febuary 29's)	*/
	age = ( (time_t)year - 70 ) * 31536000;

	/* now add in seconds for feburary 29's */
	age += (((time_t)year - 69 ) / 4 ) * 86400;

	if ( DAYSPYR( (int)year ) == 366 )
		dayspmon[ 1 ] = 29;	/* revise # days for feb. */

	for ( ydays = 0, i = 0; i < (int)month - 1; i++ )
		ydays += dayspmon[ i ];

	dayspmon[ 1 ] = 28;	/* reset #days for feb. */

	/* now seconds for days in this year */
	age += (time_t)ydays * 86400;

	/* now add in seconds for days this month */
	age += (time_t)( dom - 1 ) * 86400;

	/* and finally add in hours, minutes and seconds */
	age += (time_t)hrs * 3600;
	age += (time_t)mins * 60;
	age += (time_t)secs;

	return ( age );
}

/*
** todintrclr
**   clears the interrupt latch from the chip (for clock not tod).
*/
todintrclr()
{
	READFIELD( td_regC, tod_dev.td_regC );
	if ( !(lbolt & 63 ) ) 
		*STATUS_REG ^= ST_LEDB0;
}

/*
** todsetflg( flag, val )
** int	flag,			which flag to operate on
**	val			the value to give the flag
**
**   Set the given value into the given flag.
**   The flag resides in the 50bytes of static RAM in the chip.
*/
char
todsetflg( flag, val )
int	flag,
	val;
{
	char	val1;

	switch ( flag )
	{
	   case TD_BTFLG:
		READFIELD( td_btflg, val1 )
		WRITEFIELD( td_btflg, val )
		break;

	   case TD_PWRFLG:
		READFIELD( td_pwrflg, val1 )
		WRITEFIELD( td_pwrflg, val )
		break;

	   case TD_WTIMFLG:
		READFIELD( td_wtimflg, val1 )
		WRITEFIELD( td_wtimflg, val )
		break;

	}
	return ( val1 );
}
