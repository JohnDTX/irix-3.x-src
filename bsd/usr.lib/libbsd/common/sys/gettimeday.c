/* compatibility function to make System V time(2) look like the 4.2
 *	function.
 *
 *	This file does not include something for settimeofday().  You should
 *	surely not have to port BSD code that sets the date.
 */

#include <time.h>			/* System V */
#include <sys/types.h>
#include <sys/times.h>			/* more System V for times(2) */
#include <sys/time.h>			/* BSD */

int					/* return 0 or -1 */
gettimeofday(tp,tzp)
struct timeval *tp;
struct timezone *tzp;
{
	/*
	 * In the SUN kernel the 'tzp' argument can be 0, in which case
	 * the timezone is not returned.  This feature isn't documented
	 * in the 4.2 manual from Berkeley.
	 */
	if (tzp) {
		tzset();
		tzp->tz_minuteswest = timezone;
		tzp->tz_dsttime = daylight;
	}

	return (tp!=0 ? BSD_getime(tp) : 0);
}
