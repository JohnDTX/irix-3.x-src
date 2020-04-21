#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/setjmp.h"
#include "machine/cpureg.h"

#define	LPENBASE	(MBIO_VBASE+0x6000)	/* 16-bit quantities */


short	havelpen;	/* true if lpen is present	*/

/*
 * lpenprobe:
 */
lpenprobe()
{
	extern int *nofault;
	extern short beprint;
	int *saved_jb;
	jmp_buf jb;
	int	tmp;

	beprint = 0;
	saved_jb = nofault;
	if ( setjmp( jb ) == 0 )
	{
		nofault = jb;
		tmp = *(int *)LPENBASE;
#ifdef	lint
		tmp = tmp;
#endif
		havelpen++;
		printf( "lpen at mbio 0x6000\n" );
	} else
		printf( "lpen not installed\n" );
	nofault = saved_jb;
	beprint = 1;
}
