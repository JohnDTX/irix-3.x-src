/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/trap.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:57 $
 */

#include	"sys/types.h"
#include	"cpureg.h"
#include	"common.h"

char	*trps[] = {
	"Reset",
	"",
	"Bus Error",
	"Address Error",
	"Illegal Insruction",
	"Zero Divide",
	"CHK/CHK2",
	"cpTRAPcc/TRAPcc/TRAPV",
	"Priviledge Instruction",
	"Trace",
	"Line 1010 Emulator",
	"Line 1111 Emulator",
	"Reserved",
	"Coproc Violation",
	"Format Error",
	"Uninit Intr"
};

struct frame
{
	short	fr_sr;
	long	fr_pc;
	u_short	fr_vecoff;
	short	fr_misc[1];
};

trap( savessp, frame )
long		savessp;
struct frame	frame;
{
	register unsigned int	vec,
				vecoff;
	int delay;

	vecoff = frame.fr_vecoff;
	vec = ( vecoff & 0x0fff ) >> 2;

	if ( _commdat->c_nofault )
		longjmp( _commdat->c_nofault, vec );

	printf( "\nFault Information (vector offset: %04x):\n",
		vecoff & 0xffff );

	trpprt( vec );

	vecoff >>= 12;	/* calc the vector format	*/
	if ( vecoff == 0xa )
	{
		/* short format bus error */
		printf( "ssw: %04x dcfa: %08x dob: %08x\n", frame.fr_misc[ 1 ],
			*(long *)&( frame.fr_misc[ 4 ] ),
			*(long *)&( frame.fr_misc[ 8 ] )
		      );
	}
	else
	if ( vecoff == 0xb )
	{
		/* long format bus error */
		printf( "ssw: %04x dcfa: %08x dib: %08x dob: %08x\n",
			frame.fr_misc[ 1 ],
			*(long *)&( frame.fr_misc[ 4 ] ),
			*(long *)&( frame.fr_misc[ 18 ] ),
			*(long *)&( frame.fr_misc[ 16 ] )
		      );
	}

	printf( "\nProcessor Registers (ssp: %08x):\n", savessp );
	printf( "   pc: %08x  sr: %04x\n", frame.fr_pc,
		frame.fr_sr & 0xffff );

	printf( "\nBoard Registers:\n" );
	printf( "   text base/limit (%04x/%04x)\n", *TDBASE_REG & 0xffff,
		*TDLMT_REG & 0xffff );
	printf( "   stk  base/limit (%04x/%04x)\n", *STKBASE_REG & 0xffff,
		*STKLMT_REG & 0xffff );
	printf( "   status: %04x parctl: %02x mbp: %02x\n",
		*STATUS_REG & 0xffff, *PARCTL_REG & 0xffff );

	/* Don't return right away */
	for ( delay=1; delay< 1000000; delay++ ) 
		;
}

trpprt( vec )
int	vec;
{
	printf( "Exception: " );

	if ( vec <= 15 )
		printf( "%s", trps[ vec ] );
	if ( vec >= 16 && vec <= 23 )
		printf( "Reserved" );
	if ( vec == 24 )
		printf( "Spurious Intr" );
	if ( vec >= 25 && vec <= 31 )
		printf( "Level %d Intr Auto Vector", ( vec % 24 ) );
	if ( vec >= 32 && vec <= 47 )
		printf( "Trap %d", ( vec % 32 ) );
	if ( vec >= 48 && vec <= 63 )
		printf( "Reserved" );
	if ( vec >= 64 && vec <= 255 )
	{
		if ( ( vec >= 65 && vec < 84 ) || ( vec > 84 && vec <= 86 ) )
			printf( "Special %02x", vec );
		else
			printf( "User Defined %02x", vec );
	}
	if ( vec > 255 )
		printf( "Unknown Exception" );

	printf( " (Vector #%d)\n", vec );
}
