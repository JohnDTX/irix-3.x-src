/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/init.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:33 $
 */

#include	"types.h"
#include	"termio.h"
#include	"cpureg.h"
#include	"common.h"
#include	"iriskeybd.h"

char	*MBioVA     = (char *)SEG_MBIO;
char	*MBmemVA    = (char *)( ONEMEG * 32 );
char	*MBmallocVA = (char *)( ONEMEG * 32 );
long	MBmallocSZ  = ONEMEG/2;
long	MBmemphys;
long	MBiophys;
char	*MBmappedVA;		/* virtual address of a multibus mapped area */
char	*MBmappedphys;		/* multibus address of mapped area	*/
long	MBmappedstart = ONEMEG/2;

short	kbdstate;
char	kbdcntrlstate;
unsigned char	kbdtype;

extern _dugetchar(), _duputchar(), screenputc(),keygetchar();

_init()
{
	MBmemphys   = ( (long)_commdat->c_mbmemadr & ~SEG_MSK );
	MBiophys    = ( (long)_commdat->c_mbmapadr & ~SEG_MSK );
	MBmappedphys = (char *)(MBmemphys + MBmappedstart);

	ttyconfig();
	con_config();
}

ttyconfig()
{
	struct swregbits	swregbits;
	register int 	i;

	*(u_short *)&swregbits = *SWTCH_REG;

	/* let duarts settle but don't use delay_ms */
	for ( i=0; i < 50000; i++ )
		;

	/* initialize ports 0 and 1	*/
	_duinit( 0, CON_600, swregbits.sw_consspd );
	/* initialize port 2 and 3	*/
	_duinit( 2, swregbits.sw_consspd, swregbits.sw_consspd );

	/* let duarts settle but don't use delay_ms */
	for ( i=0; i < 50000; i++ )
		;

}


/*
** con_config
**   determine which tty we have (graphics or dumb) and do
**   all needed initialization for the port.
*/
con_config()
{
	unsigned char	c;
	register int	i;
	struct swregbits	swregbits;
	int foundkb;

	delay_ms(50);			/* let duarts settle */

	*(u_short *)&swregbits = *SWTCH_REG;

	/* XXX - set up the dc4 bits even if keyboard is hosed */
	_commdat->c_lflag = ECHO;
	if ( swregbits.sw_secdis )
		_commdat->c_flags |= DC_HIGH;

	/* now determine what console we have		*/

#define	buzzOut(x)	{_duputc((x),0); delay_ms(16);}
		/*
		 * Try to find the keyboard 3 times, allowing time for the
		 * keyboard to reset itself.  We give it three chances to
		 * avoid failing due to a glitch.
		 */
		foundkb = 0;
		kbdstate = 0;
		for (i = 0; (i < 3) && !foundkb; i++) {
			keyputchar(CONFIG_REQUEST);
			delay_ms(500);
			if ((c = _dugetc(0,2)) < 0)
				continue;
			switch (c) {
			  case CONFIG_BYTE_NEWKB:
				/* gobble second byte this keyboard sends */
				if ( ( c = _dugetc(0,8) ) < 0)
					continue;

				if ( c == CONFIG_KBSWTCH_ISO )
					kbdtype = KBD_4D60ISO;
				else
				if ( c == CONFIG_KBSWTCH_STD )
					kbdtype = KBD_4D60STD;
				else
					continue;
				/* walk leds through a pattern, and beep */
				buzzOut(NEWKB_SETDS4);
				buzzOut(NEWKB_SETDS5);
				buzzOut(NEWKB_SETDS6);
				buzzOut(NEWKB_SETDS7);
				buzzOut(NEWKB_OFFDS);

				/* enable auto-repeat and key-click */
				kbdcntrlstate = NEWKB_AUTOREPEAT;
				keyputchar(kbdcntrlstate
					 | NEWKB_SHORTBEEP
					 );
				foundkb = 1;
				break;
			  case CONFIG_BYTE_OLDKB:
				kbdtype = KBD_IRIS;
				/* us engineering folks like this keyboard... */
				kbdcntrlstate = 0;
				buzzOut(KBD_LEDCMD | KBD_LED0 | KBD_LED1);
				buzzOut(KBD_LEDCMD | KBD_LED0 | KBD_LED2);
				buzzOut(KBD_LEDCMD | KBD_LED0 | KBD_LED3);
				buzzOut(KBD_LEDCMD | KBD_LED0 | KBD_LED4);
				buzzOut(KBD_LEDCMD | KBD_LED0 | KBD_LED5);
				buzzOut(KBD_LEDCMD | KBD_LED0 | KBD_LED6);
				keyputchar(KBD_LEDCMD | KBD_LED0);
				keyputchar(kbdcntrlstate
					| KBD_BEEPCMD | KBD_SHORTBEEP
					);
				foundkb = 1;
				break;
			  default:
				delay_ms(1000);
				continue;
			}
		}

		if (!foundkb) {
			return (-1);
		}

		if ( ! ScreenInit() )
		{
			printf("init: Graphics Hardware failed\n");
		}
		else
		{			/* change getc and putc */
			if ( ! TermInit( -1 ) )
			{
				printf("init: Graphics Hardware failed\n");
			}
			else
			{
				_commdat->c_putc = screenputc;
				_commdat->c_getc = keygetchar;
			}
		}
}

/*
** bcopy
**   copy 'from' to 'to' for 'sz' bytes.
** RECODE in assembly for speed (whistle while you work!)
*/
bcopy( from, to, sz )
register char	*from,
		*to;
register int	sz;
{
	while ( sz-- )
		*to++ = *from++;
}
