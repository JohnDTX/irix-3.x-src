#include "../pmII/pte.h"

#include "../h/param.h"
#include "../vm/vm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/kprof.h"
#include "../pmII/cx.h"
#include "../pmII/reg.h"
#include "../pmII/cpureg.h"
#include "../pmII/frame.h"

main()
{
	register struct vmmeter *vm = (struct vmmeter *)0;
	register struct user *up = (struct user *)0;
	register struct cx *cx = (struct cx *)0;
	register struct frame *fp = (struct frame *)0;

	printf("#ifdef LOCORE\n");
	printf("#define\tV_TRAP %d\n", &vm->v_trap);
	printf("#define\tV_SYSCALL %d\n", &vm->v_syscall);
	printf("#define\tV_INTR %d\n", &vm->v_intr);
	printf("#define\tV_FAULTS %d\n", &vm->v_faults);
/* not needed under system V
	printf("#define\tPCB_SSWAP 0x%x\n", &up->u_pcb.pcb_sswap);
	printf("#define\tPCB_PC 0x%x\n", &up->u_pcb.pcb_pc);
*/
	printf("#define\tUSIZE %d\n", UPAGES * NBPG);
	printf("#define\tCX_USER 0x%x\n", &cx->cx_user);
	printf("#define\tNBPG 0x%x\n", NBPG);
	printf("#define\tFRAME_SP %d\n", &fp->regs[SP]);
	printf("#define\tFRAME_SR %d\n", &fp->sr);
	printf("#define\tFRAME_PC %d\n", &fp->pc);
	printf("#define\tFRAME_VECOFFSET %d\n", &fp->vecoffset);
	printf("#define\tFRAME_INTR %d\n", &fp->introutine);
/*
|
| UDOT_KCX is the offset in the pagemap of the user structure. The kernel
| mapping is designed such that the kernel always has its user page as
| the LAST pagemap entry in the pagemap.  This is done to keep the kernel
| out of the ``user usable'' portion of the pagemap.  Unfortunately, since
| the kernel uses 2mb of virtual space, we can only hide one meg of the
| kernel in the upper portion of the pagemap (virtual range 0xF00000-0xFFFFFF).
|
| VBR_CX0 is the offset in the pagemap for the interrupt vectors, while in
| context 0.  VBR_KCX is the offset in the pagemap for the interrupt
| vectors while in context KCX.
|
| The following constants KNOW that KCX is 0x20 and that KERN_VBASE
| is 0xC00000:
|	VBR_CX0, VBR_KCX, KV_KCX, KV_CX0
|
*/
	printf("#define\tUDOT_KCX 0x%x\n", btop(UDOT_VBASE ^ (KCX << 16)) * 2);
	printf("#define\tVBR_KCX 0x%x\n", btop(IVEC_VBASE ^ (KCX << 16)) * 2);
	printf("#define\tKV_KCX 0x%x\n", btop(KERN_VBASE ^ (KCX << 16)) * 2);
	printf("#define\tKV_CX0 0x%x\n", btop(KERN_VBASE ^ (0 << 16)) * 2);
	printf("#define\tVBR_CX0 0x%x\n", btop(IVEC_VBASE ^ (0 << 16)) * 2);

#ifdef	PROF
	printf("#define\tPCMASK 0x%x\n",
				(PCRANGE - 1) & ~((1<<PROFSHIFT) - 1));
#endif
	printf("#endif\n");

	exit(0);
}
