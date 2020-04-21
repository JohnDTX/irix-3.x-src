/*
 * $Source: /d2/3.7/src/sys/ipII/RCS/genassym.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:30:48 $
 */
#include "../ipII/pte.h"

#include "../h/param.h"
#include "../vm/vm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/kprof.h"
#include "../ipII/reg.h"
#include "../ipII/cpureg.h"
#include "../ipII/frame.h"
#include "../ipII/cx.h"

main()
{
	register struct vmmeter *vm = (struct vmmeter *)0;
	register struct user *up = (struct user *)0;
	register struct frame *fp = (struct frame *)0;
	register struct cx *cx = (struct cx *)0;

	printf("#ifdef LOCORE\n");
	printf("#define\tV_TRAP %d\n", &vm->v_trap);
	printf("#define\tV_SYSCALL %d\n", &vm->v_syscall);
	printf("#define\tV_INTR %d\n", &vm->v_intr);
	printf("#define\tV_FAULTS %d\n", &vm->v_faults);
	printf("#define\tUSIZE %d\n", UPAGES * NBPG);
	printf("#define\tUDOT_OFF 0x%x\n", btop( ( UDOT_VBASE & ~SEG_MSK ) * sizeof (struct pte ) ) );
	printf("#define\tNBPG 0x%x\n", NBPG);
	printf("#define\tFRAME_SP %d\n", &fp->fr_regs[SP]);
	printf("#define\tFRAME_SR %d\n", &fp->fr_sr);
	printf("#define\tFRAME_PC %d\n", &fp->fr_pc);
	printf("#define\tFRAME_VECOFFSET %d\n", &fp->fr_vecoff);
	printf("#define\tFRAME_INTR %d\n", &fp->fr_intrhandler);
	printf("#define\tPGSHIFT %d\n", PGSHIFT );
	printf("#define\tCX_TDUSER %d\n", &cx->cx_tduser );
	printf("#define\tCX_TDSIZE %d\n", &cx->cx_tdsize );
	printf("#define\tCX_SUSER %d\n", &cx->cx_suser );
	printf("#define\tCX_SSIZE %d\n", &cx->cx_ssize );
	printf("#define\tPPCXLOG2 %d\n", PPCXLOG2 );
	printf("#define\tPTEPCX %d\n", PTEPCX );
#ifdef	PROF
	printf("#define\tPROFSHIFT %d\n", PROFSHIFT );
	printf("#define\tPCMASK 0x%x\n",
				(PCRANGE - 1) & ~((1<<PROFSHIFT) - 1));
#endif

	printf("#endif\n");

	exit(0);
}
