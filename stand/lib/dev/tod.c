/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/tod.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:57 $
 */

#include	"sys/types.h"
#include	"cpureg.h"
#include	"tod.h"

#define	TOD_READ	0x0
#define	TOD_WRITE	0x1

static
todxfer( type, addr, buf, cnt )
int	type;
u_long	addr;
u_char	*buf;
int	cnt;
{
	u_char	val1,
		waddr;
	register u_char todregB;
	register int	tcnt = cnt;

	if ( ( addr & (u_long)CLK_DATA ) != (u_long)CLK_DATA )
		return (-1);

	if ( ( addr + cnt ) >
		(u_long)( CLK_DATA + sizeof(struct tod_dev) ) )
		return (-1);

	READFIELD( td_regB, todregB )
	/*
	** for posterity sake: the rtc just looks at the lower 6 bits
	** of the address, so the next instruction isn't really needed
	*/
	waddr = (u_char)( addr & 0x3f );
	if ( waddr <= TOD_CLKSZ ) {
		/* make sure you are not in update cycle or you
		 * will get garbage
		 */
		READFIELD( td_regB, todregB )
		val1 = todregB | RB_SET;
		WRITEFIELD( td_regB, val1 )
		do {
			READFIELD( td_regA, val1 );
		} while ( val1 & RA_UIP );
		WRITEFIELD( td_regB, todregB )
	}

	for ( ; tcnt; waddr++, tcnt-- ) {
		if ( type == TOD_READ ) {
			RDTODADR( waddr, *buf )
		} else {
			WRTTODADR( waddr, *buf )

			/*
			** update saved regB value, we just changed it
			*/
			if ( waddr == 0xb )
				todregB = *buf;
		}
		buf++;
	}
	WRITEFIELD( td_regB, todregB )
	return( cnt );
}

todread( addr, buf, cnt )
u_long	addr;
u_char	*buf;
int	cnt;
{
	return (todxfer( TOD_READ, addr, buf, cnt ));
}

todwrite( addr, buf, cnt )
u_long	addr;
u_char	*buf;
int	cnt;
{
	return (todxfer( TOD_WRITE, addr, buf, cnt ));
}


/* delays and alarms:				*/
static unsigned short times[] = { 1,2,4,8,16,31,63,125,250,500 };

/* delay for t milliseconds only works for less than 500 ms
** because you can't reset the divider chain without losing 1/2 second,
** We go through the buzz loop twice.  This will have the effect of delaying
** somewhere between t and 2t
** Also if you reset the divider chain, you get PF high 1/4 of a cycle later
*/
delay_ms(t)
{
	register u_char regc;
	register u_char tbase;
	register u_char todregB;
	register int i;

	if ( t > 500 )
		return(-1);

	for ( i=0; i<(sizeof(times)/sizeof(times[0]));i++ ) {
		if ( t <= times[i] )
			break;
	}
	/* make sure interrupts are off */
	READFIELD( td_regB, todregB )
	todregB &= ~(RB_PIE|RB_AIE|RB_UIE);
	WRITEFIELD( td_regB, todregB )

	tbase = i + RA_RS1KHZ;
	WRITEFIELD( td_regA, RA_DVIP2|tbase );
	READFIELD( td_regC, regc );		/* clear regc	*/
	i = 60000;				/* if hardware problem bound */
	do {
		READFIELD( td_regC, regc );
		if ( --i == 0 ) {		/* hardware hosed	*/
			return (-1);
		}
	}
	while  ( !(regc & RC_PF) );
	/* one more time	*/
	do {
		READFIELD( td_regC, regc );
	}
	while  ( !(regc & RC_PF) );

}

/* delay for t seconds (may be off by .5 sec) */
delay_sec(t)
register int t;
{
	t *= 2;
	while ( t-- )
		delay_ms(500);

	/* one for good luck */
	delay_ms(500);
}

sleep(sec)
{
	delay_sec(sec);
}

/*	set alarm for t second from now			*/
alarm_sec(t)
{
	struct tod_dev tod_dev;
	int resid;
	
	todxfer( TOD_READ, CLK_DATA, &tod_dev, TOD_CLKSZ+4 ); /* get the regs */
	resid = (int)tod_dev.td_sec + t + 1;
	tod_dev.td_secalrm = resid % 60;
	resid /= 60;
	resid += (int)tod_dev.td_min;
	tod_dev.td_minalrm = resid % 60;
	resid /= 60;
	resid += (int)tod_dev.td_hrs;
	if ( tod_dev.td_regB & RB_HR24 ) 
		tod_dev.td_hrsalrm = resid % 24;
	else
		tod_dev.td_hrsalrm = resid % 12;
	
	WRITEFIELD( td_secalrm, tod_dev.td_secalrm );
	WRITEFIELD( td_minalrm, tod_dev.td_minalrm );
	WRITEFIELD( td_hrsalrm, tod_dev.td_hrsalrm );
}

/*	set alarm for t milliseconds from now		*/
alarm_ms(t)
{
}

/* determine if alarm has gone off yet			*/
/* returns - 0	stay in bed
	     1  get up you lazy bum
	    -1  no alarm set
*/
alarm_rung(alarm)
{
	u_char regc;

	alarm = SEC_ALARM;
	switch (alarm) {

	case SEC_ALARM:
		READFIELD( td_regC, regc );
		return( (regc & RC_AF) != 0 );
		break;

	case MS_ALARM:
		break;

	default:
		return(-1);
		break;
	}
}


alarm_reset(alarm)
{
	switch (alarm) {

	case SEC_ALARM:
		break;

	case MS_ALARM:
		break;

	default:
		return(-1);
		break;
	}
}
