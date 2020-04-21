# include "IrisConf.h"
# include "m68vectors.h"

#define ADDRLOC		EVEC_RESETPC
#define PROBELOC	EVEC_RESETSSP


slaveboot()
{
	*EVEC_RESETSSP = 0;
	*EVEC_RESETPC = 0;

	if (VERBOSE(switches))
		printf("\n\
SLAVE processor selected; polling address...");

	while (!(*PROBELOC))
		;

	/* someone poked us..... */
	spl7();
	ResetDuarts();

	(*(int (*(*))())ADDRLOC)();

	warmboot();
}
