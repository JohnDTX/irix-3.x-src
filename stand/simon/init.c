/*
* $Source: /d2/3.7/src/stand/simon/RCS/init.c,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:20:47 $
*/

#include	"sys/types.h"
#include	"cpureg.h"
#include	"termio.h"
#include	"common.h"
#include	"tod.h"
#include	"iriskeybd.h"

extern char	*Version;
extern		_dugetchar(),
		_duputchar(),
		screenputc(),
		keygetchar();

extern int	Inprom;

/*
** this structure holds the initial values for the common area
** that resides in the beginning of the static RAM.
*/
struct commstruct	comminit = {
	0L,					/* mem array		*/
	0L,					/* megabyte count	*/
	(u_short *)SEG_MBMEM,			/* multibus mem address	*/
	(u_short *)( SEG_MBMEM + 0x100000 ),	/* multibus map address	*/
	(short)0,				/* mb start page	*/
	(u_char)0,				/* havefpa		*/
	(u_short)0,				/* dcr bits		*/
	(u_short)TTY_DUMB,			/* flags		*/
	_dugetchar,				/* getchar routine	*/
	_duputchar,				/* putchar routine	*/
	(u_short)ECHO,				/* tty flags		*/
};

/*
** init
**   initialize the common area at the beginning of our
**   private ram, size memory and clear it, determine tty type, probefpa
*/
init( flag )
int	flag;	/* indicates level of initialization	*/
{
	register unsigned	checksum = 0;
	register unsigned	*ptr;
	register u_char		*cptr;
	register u_int		i;
	struct swregbits	swregbits;

	/*
	** compute checksum of the common area.  If computed checksum
	** matches stored checksum we assume things are kosher and
	** we don't do any initializations
	*/
	ptr = (unsigned *)SRAM_BASE;
	for ( ; ptr < (unsigned *)&_commdat->c_chksum; ptr++ )
		checksum += *ptr;

	if ( Inprom && ( !flag || (_commdat->c_chksum != checksum) ) )
	{
		*(u_short *)&swregbits = *SWTCH_REG;

		/*
		** if we are a master, then we reset the multibus
		*/
		if ( ! ( swregbits.sw_mstrslv ) )
		{
			/*
			** force a multibus init and spin
			** spin time is - well it seems to work!
			*/
			*STATUS_REG |= ST_MBINIT;
			for ( i = 150000; --i; )
				;

			/*
			** turn off multibus init and spin
			*/
			*STATUS_REG &= ~ST_MBINIT;
			for ( i = 10000; --i; )
				;
		}
		bcopy( &comminit, _commdat, sizeof (struct commstruct) );
		cptr = (u_char *)&_commdat->c_powerflag;
		while ( cptr < (u_char *)&_commdat->c_argbuf[256] )
			*cptr++ = 0x00;
		msize();

		/*
		** set the multibus memory address only if a SLAVE processor
		** divide by 2 to update a (u_short *) crap.
		*/
		if ( swregbits.sw_mstrslv ) {
			_commdat->c_mbmemadr += 0x200000/2;
			_commdat->c_mbmapadr += 0x200000/2;
		}

		/* configure the stuff that is stored in TOD ram */
		todraminit();

		/*
		** everything is the common checksummed area is now
		** initialized - compute the checksum and store it
		*/
		checksum = 0;
		ptr = (unsigned *)SRAM_BASE;
		for ( ; ptr < (unsigned *)&_commdat->c_chksum; ptr++ )
			checksum += *ptr;
		_commdat->c_chksum = checksum;

	}

	/* initialize the two duarts	*/
	ttyconfig();

	/* check for the FPA */
	_commdat->c_havefpa = fpaprobe();
}


/*
** ttyconfig
**   initialize the duarts
*/
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
	unsigned char	j;
	struct swregbits	swregbits;

	delay_ms(50);			/* let duarts settle */

	*(u_short *)&swregbits = *SWTCH_REG;

	/* XXX - set up the dc4 bits even if keyboard is hosed */
	_commdat->c_lflag = ECHO;
	if ( swregbits.sw_secdis )
		_commdat->c_flags |= DC_HIGH;

	/* now determine what console we have		*/

	keyputchar(ResetKeybd);		/* Send the reset value to Port A */
	delay_ms(500);			/* wait even longer for a response */

	if ( (j = _dugetc(0,2)) == 0xaa) /* if '0xaa' comes back it's the */
	{
		/* flash the kbd leds */
		for(j = 1; j < 0x80; j <<= 1) {
			keyputchar(j|1);
			delay_ms(16);
		}
		keyputchar(1);
		delay_ms(16);

		if (!ScreenInit()) {
			printf("init: Graphics Hardware failed\n");
		} else {			/* change getc and putc */
			if (!TermInit(-1)) {
				printf("init: Graphics Hardware failed\n");
			} else {
				beep();
				_commdat->c_putc = screenputc;
				_commdat->c_getc = keygetchar;
				_commdat->c_flags &= ~TTY_DUMB;
			}
		}
	}
}

/*
** msize
**   determine the amount of memory present.  We probe memory at
**   intervals of 1megabyte by mapping the first 4kb using the map
**   by using the user segment.	We start at the highest 1mb address
**   and work our way back to 0.  In each 4kb we test we also place the
**   expected megabyte of memory it is associated with.
**   If we can read back what we wrote we assume that the 1mb
**   of memory is present and we assume that the megabyte it is is the
**   one we placed in there.
**
**   This technique is used because memory reflects about 2mb in the 4mb
**   space each memory board takes up. This means that if a board is stuffed
**   with only 1mb, it would answer at the 2nd and 3rd megabyte of the
**   virtual board space also.
*/
msize()
{
	register unsigned short	*ptr;	/* for the probes	*/
	struct pte	pte;
	register int	i,	/* temp counters	*/
			j;

	/*
	** need to go out of boot mode so we can use the page map
	** also turn on quick timeouts.
	*/
	*STATUS_REG |= ST_SYSSEG_ | ST_EQKTIMO;

	/*
	** set text/data base and limit register to allow access
	** to only 1 page (user virtual 0).
	** we just choose to use the zeroth index in the page map.
	*/
	*TDBASE_REG = 0;
	*TDLMT_REG = 0;		/* no limit */

	*(u_long *)&pte = PTE_RWACC;	/* read/write access for all	*/

	for ( i = MAXMEMMB-1; i >= 0; i-- )
	{
		/*
		** program the pte we are going to use with the correct
		** physical page
		*/
		pte.pg_page = i * ONEMEGPG;
		setpte( 0, &pte );

		/*
		** slam in a few nice patterns
		*/
		ptr = (unsigned short *)0;
		*ptr++ = 0xdead;
		*ptr++ = -1;
		*ptr++ = 0xaaaa;
		*ptr++ = 0x5555;
		*ptr++ = i;

	}

	/*
	** check to see if they are really there
	*/
	for ( i = 0; i < MAXMEMMB; i++ )
	{
		pte.pg_page = i * ONEMEGPG;

		/*
		** yet another cute use of mfill
		** 0x1	increments the physical page number
		*/
		mfill( PTMAP_BASE, sizeof (long), *(u_long *)&pte,
		       0x1, ONEMEGPG );

		ptr = (unsigned short *)0;
		if ( *ptr++ != 0xdead )
			continue;
		if ( *ptr++ != -1 )
			continue;
		if ( *ptr++ != 0xaaaa )
			continue;
		if ( *ptr++ != 0x5555 )
			continue;
		if ( *ptr++ != i)	/* must match expected megabyte	*/
			continue;

		/*
		** update multibus starting page.  We keep track as
		** which 1megabyte chunk is the highest, in pages.  
		*/
		_commdat->c_mbspg = i << 8;	/* quick calculation for page */

		_commdat->c_memmb++;		/* update megabyte count  */
		_commdat->c_mem |= ( 1 << i );	/* update bit array	  */
		bzero( (char *)0, ONEMEG );
	}

	/* back to sys seg only	*/
	*STATUS_REG &= ~( ST_SYSSEG_| ST_EQKTIMO );
}
