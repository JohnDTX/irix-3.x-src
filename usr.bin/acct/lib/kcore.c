/* kcore -- return number of K bytes given number of clicks */

#include <sys/param.h>

kcore(clicks)
{
	return (ctob(clicks)>>10);
}
