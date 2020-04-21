#ifndef	__TIME_H__
#define	__TIME_H__

/*
 * $Source: /d2/3.7/src/include/RCS/time.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:12:08 $
 */

struct	tm {	/* see ctime(3) */
	int	tm_sec;
	int	tm_min;
	int	tm_hour;
	int	tm_mday;
	int	tm_mon;
	int	tm_year;
	int	tm_wday;
	int	tm_yday;
	int	tm_isdst;
};
extern struct tm *gmtime(), *localtime();
extern char *ctime(), *asctime();
extern void tzset();
extern long timezone;
extern int daylight;
extern char *tzname[];

#endif	/* __TIME_H__ */
