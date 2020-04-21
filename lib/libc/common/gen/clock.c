/*	@(#)clock.c	1.1	*/
/*LINTLIBRARY*/

#include <sys/types.h>
#include <sys/times.h>
#ifdef m68000
#define HZ	60
#else
#include <sys/param.h>	/* for HZ (clock frequency in Hz) */
#endif
#define TIMES(B)	(B.tms_utime+B.tms_stime+B.tms_cutime+B.tms_cstime)

extern long times();
static long first = 0L;

long
clock()
{
	struct tms buffer;

	if (times(&buffer) != -1L && first == 0L)
		first = TIMES(buffer);
	return ((TIMES(buffer) - first) * (1000000L/HZ));
}
