static char ID[] = "@(#)reset.c	1.1";

#include "trek.h"
reset()
{
	longjmp(env,1);
}
