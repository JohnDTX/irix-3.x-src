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
}


aerr(fn, aa, irsr, pc)
long fn, aa, irsr, pc;
{
	printf("\nAddress Error near PC %x, Address %x\n", pc, aa);
}

ierr(ps, pc)
long ps, pc;
{
	printf("\nIllegal Instruction near PC %x\n", pc);
}
