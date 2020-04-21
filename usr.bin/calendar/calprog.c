char _Origin_[] = "System V";
char sccsid[] = "@(#)calprog.c	1.4";
/* $Source: /d2/3.7/src/usr.bin/calendar/RCS/calprog.c,v $ */
static	char	*Sccsid = "@(#)$Revision: 1.1 $";
/* $Date: 89/03/27 17:43:37 $ */

/*
 * /usr/lib/calprog produces an egrep -f file
 * that will select today's and tomorrow's
 * calendar entries, with special weekend provisions
 *
 * used by calendar command
 */

#include <stdio.h>
#include <time.h>

#define DAY (3600*24L)

char *month[] = {
	"[Jj]an",
	"[Ff]eb",
	"[Mm]ar",
	"[Aa]pr",
	"[Mm]ay",
	"[Jj]un",
	"[Jj]ul",
	"[Aa]ug",
	"[Ss]ep",
	"[Oo]ct",
	"[Nn]ov",
	"[Dd]ec"
};
char *wday[] = {
	"[Ss]un",
	"[Mm]on",
	"[Tt]ue",
	"[Ww]ed",
	"[Tt]hu",
	"[Ff]ri",
	"[Ss]at"
};
struct tm *localtime();

tprint(t,exact)
long t;
char *exact;
{
	register struct tm *tm;
	int yday;

	tm = localtime(&t);
	yday = tm->tm_yday + 1;
	printf("(^|[ (,;:])%s((%s[^ ]* *|0*%d[-/]|\\\\\\*[-/ ])0*%d|@%s[a-z]*([^/a-z]|/(%d/2|%d/3|%d/4)))([^0123456789]|$)\n",
	  exact, month[tm->tm_mon], tm->tm_mon + 1, tm->tm_mday,
	  wday[tm->tm_wday], yday%2, yday%3, yday%4);
}

main()
{
	long t;

	freopen("/dev/console","w",stderr);
	time(&t);
	checkhol();
	tprint(t,"=?");	/* Generate egrep pattern for today's date	      */
			/* Do tomorrow's date; if Sat, Sun, or Holiday repeat */
	while (tprint(t += DAY,""), ssh(localtime(&t)))
		;
}

/*	@(#)pnpsplit.c	1.3 of 6/22/82	*/
#define	NHOLIDAYS	50	/* max number of company holidays per year */

int	thisyear = 1984;	/* this is changed by holidays file */
int	holidays[NHOLIDAYS] = {	/* holidays file day-of-year table */
	-1
};
char	holfile[] = {
	"/usr/lib/acct/holidays\0"
};

/*
 *	Starting day after Christmas, complain if holidays not yet updated.
 *	This code is only executed once per program invocation.
 */
checkhol()
{
	register struct tm *tp;
	long t;

	holidays[0] = -1;
	if (inithol() == 0) {
		fprintf(stderr, "calendar: holidays table setup failed\n");
		thisyear = 0;
		holidays[0] = -1;
		return;
	}
	time(&t);
	tp = localtime(&t);
	tp->tm_year += 1900;
	if ((tp->tm_year == thisyear && tp->tm_yday > 359)
	  || tp->tm_year != thisyear)
		fprintf(stderr, "***UPDATE %s WITH NEW HOLIDAYS***\n", holfile);
	thisyear = 0;	/* checkhol() will not be called again */
}

/*
 * ssh returns 1 if Sat, Sun, or Holiday
 */
ssh(ltp)
register struct tm *ltp;
{
	register int i;

	if (ltp->tm_wday == 0 || ltp->tm_wday == 6)
		return 1;
	for (i = 0; holidays[i] >= 0; i++) {
		if (ltp->tm_yday > holidays[i])
			continue;
		else if (ltp->tm_yday == holidays[i])
			return 1;
		else if (ltp->tm_yday < holidays[i])
			break;	/* holidays is sorted */
	}
	return 0;
}

/*
 * inithol - read from an ascii file and initialize the "thisyear"
 * variable, the times that prime and non-prime start, and the
 * holidays array.
 */
inithol()
{
	FILE		*fopen(), *holptr;
	char		*fgets(), holbuf[128];
	register int	line = 0,
			holindx = 0,
			errflag = 0;
	void		sort();
	int		pstart, npstart;
	int		doy;	/* day of the year */

	if ((holptr=fopen(holfile, "r")) == NULL) {
		perror(holfile);
		fclose(holptr);
		return(0);
	}
	while (fgets(holbuf, sizeof(holbuf), holptr) != NULL) {
		if (holbuf[0] == '*')	/* Skip over comments */
			continue;
		else if (++line == 1) {	/* format: year p-start np-start */
			if (sscanf(holbuf, "%4d %4d %4d",
				&thisyear, &pstart, &npstart) != 3) {
				fprintf(stderr,
				  "%s: bad {yr ptime nptime} conversion\n",
					holfile);
				errflag++;
				break;
			}

			/* validate year */
			if (thisyear < 1970 || thisyear > 2000) {
				fprintf(stderr, "calendar: invalid year: %d\n",
					thisyear);
				errflag++;
				break;
			}


			continue;
		} else if (holindx >= NHOLIDAYS) {
			fprintf(stderr, "calendar: too many holidays, ");
			fprintf(stderr, "recompile with larger NHOLIDAYS\n");
			errflag++;
			break;
		}

		/* Fill up holidays array from holidays file */
		sscanf(holbuf, "%d	%*s %*s	%*[^\n]\n", &doy);
		if (doy < 1 || doy > 366) {
			fprintf(stderr,
				"calendar: invalid day of year %d\n", doy);
			errflag++;
			break;
		}
		holidays[holindx++] = (doy - 1);
	}
	fclose(holptr);
	if (!errflag && holindx < NHOLIDAYS) {
		sort(holidays, holindx - 1);
		holidays[holindx] = -1;
		return(1);
	} else
		return(0);
}

/*
 * sort - a version of the bubblesort algorithm from BAASE, Alg 2.1
 *
 * sorts the holidays array into nondecreasing order
 */
void
sort(array, nitems)
int	*array;		/* a pointer to the holidays array */
int	nitems;		/* the number of elements in the array */
{
	register int	index,	/* index going though holidays array */
			flag,	/* flag > 0 if more sorting is needed */
			k;

	flag = nitems;
	while (flag > 0) {
		k = flag - 1;
		flag = 0;
		for (index=0; index <= k; ++index) {
			if (array[index] > array[index+1]) {
				flag = array[index];	/* briefly use "flag"*/
				array[index] = array[index+1];
				array[index+1] = flag;
				flag = index;
			}
		}
	}

}
