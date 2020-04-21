/*
** Routines to deal with the FPA
** $Source: /d2/3.7/src/sys/ipII/RCS/fpa.c,v $
** $Revision: 1.1 $
** $Date: 89/03/27 17:30:44 $
*/

#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/setjmp.h"
#include "../h/systm.h"
#include "../ipII/cpureg.h"
#include "../ipII/fpa.h"

extern int	*nofault;
extern short	beprint;

short	havefpa;

#define	FPAOPT_REG	0x13
#define	FPAERR_REG	0xff
#define	FPAMSK_REG	0xff

/*
** fpprobe
**   check for existence of the fpa board
**   This must be done by the proms because FPA timeouts will starve refresh.
**   See the code in machdep.c that is run right after booting
*/
fpprobe()
{
	int	*savnofault;
	jmp_buf	jb;

	/*
	** this mess is because the fpa does not always come
	** up from a reset in a clean state.
	** If the fpa is present (currently a switch
	** setting which the PROMS translate) we touch a couple
	** registers to init the board to flush any posted errors.
	*/
	if ( havefpa )
	{
		printf( "fpa installed\n" );
		beprint = 0;			/* no printing in trap() */
		savnofault = nofault;
		if ( setjmp( jb ) == 0 )
		{
			nofault = jb;
			fpaopcodeB( FPA_EREG, FPA_FILLER, FPA_FILLER ) = FPAERR_REG;
			fpaopcodeB( FPA_MREG, FPA_FILLER, FPA_FILLER ) = FPAMSK_REG;
		}
		nofault = savnofault;
		beprint = 1;			/* printing in trap() */
	}
	else
		printf( "fpa not installed\n" );
}

/*
** fpinit
**   give the fpa board an initial state for a user.
**   Currently:
**	Option Reg	- 13 (Fast Mode, Rnd to zero, overlap mode)
**	Error Reg	- ff (clear any posted errors)
**	Error Mask Reg	- ff (report no errors)
*/
fpinit()
{
	int	*savnofault;
	jmp_buf	jb;
	short	sr;

	if (!havefpa)
		return;
	savnofault = nofault;

	/*
	** it is possible that while initing the sucker, you could
	** get a bus error?  Probably will only happen the very first
	** time, since after that the user state is saved first.
	*/
	sr = spl6();
	if ( setjmp( jb ) == 0 )
	{
		nofault = jb;
		beprint = 0;
		fpaopcodeB( FPA_OREG, FPA_FILLER, FPA_FILLER ) = FPAOPT_REG;
	}
	else
		fpaopcodeB( FPA_OREG, FPA_FILLER, FPA_FILLER ) = FPAOPT_REG;
	nofault = savnofault;
	beprint = 1;
	splx( sr );
	fpaopcodeB( FPA_EREG, FPA_FILLER, FPA_FILLER ) = FPAERR_REG;
	fpaopcodeB( FPA_MREG, FPA_FILLER, FPA_FILLER ) = FPAMSK_REG;
}

/*
** fpsave
**   save the state of the fpa board.
**   According to the manual:
**	1. save error register. (possible bus error could occur)
**	2. reset error register.
**	3. save error mask register.
**	4. save option register.
**	5. save condition register.
**	6. save registers.
*/
fpsave()
{
	extern short	beprint;
	register short	i;
	int		*savnofault;
	jmp_buf		jb;
	short		sr;

	if (!havefpa)
		return;
	savnofault = nofault;

	/*
	** the first access to the fpa could cause a bus error.
	** We setup to handle it, in order not to hide a buserror
	** from elsewhere we mask out the clock intrs for a bit.
	*/
	if ( setjmp( jb ) == 0 )
	{
		sr = spl6();
		nofault = jb;
		beprint = 0;
		u.u_pcb.pcb_fps.f_fper = fpaopcodeB( FPA_EREG, FPA_FILLER, FPA_FILLER );
	}
	else
		u.u_pcb.pcb_fps.f_fper = fpaopcodeB( FPA_EREG, FPA_FILLER, FPA_FILLER );
	beprint = 1;
	nofault = savnofault;
	splx( sr );

	fpaopcodeB( FPA_EREG, FPA_FILLER, FPA_FILLER ) = FPAERR_REG;
	u.u_pcb.pcb_fps.f_fpmr = fpaopcodeB( FPA_MREG, FPA_FILLER, FPA_FILLER );
	u.u_pcb.pcb_fps.f_fpor = fpaopcodeB( FPA_OREG, FPA_FILLER, FPA_FILLER );
	u.u_pcb.pcb_fps.f_fpcr = fpaopcodeB( FPA_CREG, FPA_FILLER, FPA_FILLER );

	/*
	** to save the registers, we must save 64bits worth.
	*/
	for ( i = 0; i < 16; i++ )
		u.u_pcb.pcb_fps.f_fpregs[ i ][ 0 ] = fpaopcodeL( FPA_DBLEHI, i );
	for ( i = 0; i < 16; i++ )
		u.u_pcb.pcb_fps.f_fpregs[ i ][ 1 ] = fpaopcodeL( FPA_DBLELO, i );
}	

/*
** fprestore
**   restore the context from the users u_pcb.pcb_fps
**   Reverse order from the save actions.
*/
fprestore()
{
	register short	i;

	if (!havefpa)
		return;

	/*
	** to restore the registers, we must restore 64bits worth.
	*/
	for ( i = 0; i < 16; i++ )
		 fpaopcodeL( FPA_DBLEHI, i ) = u.u_pcb.pcb_fps.f_fpregs[ i ][ 0 ];
	for ( i = 0; i < 16; i++ )
		 fpaopcodeL( FPA_DBLELO, i ) = u.u_pcb.pcb_fps.f_fpregs[ i ][ 1 ];

	fpaopcodeB( FPA_CREG, FPA_FILLER, FPA_FILLER ) = u.u_pcb.pcb_fps.f_fpcr;
	fpaopcodeB( FPA_OREG, FPA_FILLER, FPA_FILLER ) = u.u_pcb.pcb_fps.f_fpor;
	fpaopcodeB( FPA_MREG, FPA_FILLER, FPA_FILLER ) = u.u_pcb.pcb_fps.f_fpmr;
	fpaopcodeB( FPA_EREG, FPA_FILLER, FPA_FILLER ) = u.u_pcb.pcb_fps.f_fper;
}

/*
** fpreset
**   this routine is used to reset the error mask register to ignore all
**   errors.  Used by the trap code.
*/
fpreset()
{
	if (!havefpa)
		return;
	fpaopcodeB( FPA_MREG, FPA_FILLER, FPA_FILLER ) = FPAMSK_REG;
}
