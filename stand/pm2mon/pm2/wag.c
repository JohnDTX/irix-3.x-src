#include "pmII.h"
wag()
{
	STATUS_REG |= BINIT;
	msdelay(321);	/*150000*/
	STATUS_REG &= ~BINIT;
	msdelay(22);	/*10000*/
}
