# include "pmII.h"


berr(fn, aa, irsr, pc)
long fn, aa, irsr, pc;
{
	printf("\nBus Error near PC %x, Address %x: ", pc, aa);
	if((EXCEPTION_REG & FAULT_PRESENT) == 0)
		printf("Not Present ");
	if((EXCEPTION_REG & FAULT_MAP) == 0)
		printf("Map Fault ");
	if((EXCEPTION_REG & FAULT_TIMEOUT) == 0)
		printf("Timeout ");
	if((EXCEPTION_REG & FAULT_PARITY) == 0)
		printf("Parity Fault ");
	printf("\n");
	delayed_warmboot();
}


aerr(fn, aa, irsr, pc)
long fn, aa, irsr, pc;
{
	printf("\nAddress Error near PC %x, Address %x\n", pc, aa);
	delayed_warmboot();
}

ierr(ps, pc)
long ps, pc;
{
	printf("\nIllegal Instruction near PC %x\n", pc);
	delayed_warmboot();
}

delayed_warmboot()
{
    delayed_reboot();
    warmboot();
}

/* Added by Kipp/Chase */
struct fault_frame {
	short	sr;
	long	pc;
	short	vecoffset;
};

cfault(frame)
	struct fault_frame frame;
{
	printf("Fault: sr=%x pc=%x vecoffset=%x\n",
		       frame.sr, frame.pc, frame.vecoffset);
	halt();
}
