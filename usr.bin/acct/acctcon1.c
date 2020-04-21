char _Version_[] = "(C) Copyright 1983 UniSoft Corp. Version V.1.0";
char _Origin_[] = "System V";
/*	@(#)acctcon1.c	1.4 of 6/22/82	*/
/*
 *	acctcon1 [-p] [-t] [-l file] [-o file] <wtmp-file >ctmp-file
 *	-p	print input only, no processing
 *	-t	test mode: use latest time found in input, rather than
 *		current time when computing times of lines still on
 *		(only way to get repeatable data from old files)
 *	-l file	causes output of line usage summary
 *	-o file	causes first/last/reboots report to be written to file
 *	reads input (normally /etc/wtmp), produces
 *	list of sessions, sorted by ending time in ctmp.h/ascii format
 *	TSIZE is max # distinct ttys
 */

#include <sys/types.h>
#include "acctdef.h"
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <utmp.h>
#include "ctmp.h"


int	tsize	= -1;	/* used slots in tbuf table */
struct  utmp	wb;	/* record structure read into */
struct	ctmp	cb;	/* record structure written out of */

struct tbuf {
	char	tline[LSZ];	/* /dev/* */
	char	tname[NSZ];	/* user name */
	time_t	ttime;		/* start time */
	dev_t	tdev;		/* device */
	int	tlsess;		/* # complete sessions */
	int	tlon;		/* # times on (ut_type of 7) */
	int	tloff;		/* # times off (ut_type != 7) */
	long	ttotal;		/* total time used on this line */
} tbuf[TSIZE];

#define NSYS	20
int	nsys;
struct sys {
	char	sname[LSZ];	/* reasons for ACCOUNTING records */
	char	snum;		/* number of times encountered */
} sy[NSYS];

time_t	datetime;	/* old time if date changed, otherwise 0 */
time_t	firstime;
time_t	lastime;
int	ndates;		/* number of times date changed */
int	exitcode;
char	*report	= NULL;
char	*replin = NULL;
int	printonly;
int	tflag;

char	*ctime();
long	ftell();
uid_t	namtouid();
dev_t	lintodev();

main(argc, argv) 
char **argv;
{

	while (--argc > 0 && **++argv == '-')
		switch(*++*argv) {
		case 'l':
			if (--argc > 0)
				replin = *++argv;
			continue;
		case 'o':
			if (--argc > 0)
				report = *++argv;
			continue;
		case 'p':
			printonly++;
			continue;
		case 't':
			tflag++;
			continue;
		}

	if (printonly) {
		while (wread()) {
			if (valid()) {
				printf("%.12s\t%.8s\t%lu",
					wb.ut_line,
					wb.ut_name,
					wb.ut_time);
				printf("\t%s", ctime(&wb.ut_time));
			} else
				fixup(stdout);
			
		}
		exit(exitcode);
	}

	while (wread()) {
		if (firstime == 0)
			firstime = wb.ut_time;
		if (valid())
			loop();
		else
			fixup(stderr);
	}
	wb.ut_name[0] = '\0';
	strcpy(wb.ut_line, "acctcon1");
	wb.ut_type = ACCOUNTING;
	if (tflag)
		wb.ut_time = lastime;
	else
		time(&wb.ut_time);
	loop();
	if (report != NULL)
		printrep();
	if (replin != NULL)
		printlin();
	exit(exitcode);
}

wread()
{
	return( fread(&wb, sizeof(wb), 1, stdin) == 1 );
	
}

/*
 * valid: check input wtmp record, return 1 if looks OK
 */
valid()
{
	register i, c;

	for (i = 0; i < NSZ; i++) {
		c = wb.ut_name[i];
		if (isalnum(c) || c == '$' || c == ' ')
			continue;
		else if (c == '\0')
			break;
		else
			return(0);
	}

	if((wb.ut_type >= EMPTY) && (wb.ut_type <= UTMAXTYPE))
		return(1);

	return(0);
}

/*
 *	fixup assumes that V6 wtmp (16 bytes long) is mixed in with
 *	V7 records (20 bytes each)
 *
 *	Starting with Release 5.0 of UNIX, this routine will no
 *	longer reset the read pointer.  This has a snowball effect
 *	On the following records until the offset corrects itself.
 *	If a message is printed from here, it should be regarded as
 *	a bad record and not as a V6 record.
 */
fixup(stream)
register FILE *stream;
{
	fprintf(stream, "bad wtmp: offset %lu.\n", ftell(stdin)-sizeof(wb));
	fprintf(stream, "bad record is:  %.12s\t%.8s\t%lu",
		wb.ut_line,
		wb.ut_name,
		wb.ut_time);
	fprintf( stream, "\t%s", ctime(&wb.ut_time));
#ifdef	V6
	fseek(stdin, (long)-4, 1);
#endif
	exitcode = 1;
}

loop()
{
	register timediff;
	register struct tbuf *tp;

	if( wb.ut_line[0] == '\0' )	/* It's an init admin process */
		return;			/* no connect accounting data here */
	switch(wb.ut_type) {
	case OLD_TIME:
		datetime = wb.ut_time;
		return;
	case NEW_TIME:
		if(datetime == 0)
			return;
		timediff = wb.ut_time - datetime;
		for (tp = tbuf; tp <= &tbuf[tsize]; tp++)
			tp->ttime += timediff;
		datetime = 0;
		ndates++;
		return;
	case BOOT_TIME:
		upall();
	case ACCOUNTING:
	case RUN_LVL:
		lastime = wb.ut_time;
		bootshut();
		return;
	case USER_PROCESS:
	case LOGIN_PROCESS:
	case INIT_PROCESS:
		update(&tbuf[iline()]);
		return;
	case DEAD_PROCESS:
	case EMPTY:
		return;
	default:
		fprintf(stderr, "acctcon1: invalid type %d for %s %s %s",
			wb.ut_type,
			wb.ut_name,
			wb.ut_line,
			ctime(&wb.ut_time));
	}
}

/*
 * bootshut: record reboot (or shutdown)
 * bump count, looking up wb.ut_line in sy table
 */
bootshut()
{
	register i;

	for (i = 0; i < nsys && !EQN(wb.ut_line, sy[i].sname); i++)
		;
	if (i >= nsys) {
		if (++nsys > NSYS) {
			fprintf(stderr,
				"acctcon1: recompile with larger NSYS\n");
			nsys = NSYS;
			return;
		}
		CPYN(sy[i].sname, wb.ut_line);
	}
	sy[i].snum++;
}

/*
 * iline: look up/enter current line name in tbuf, return index
 * (used to avoid system dependencies on naming)
 */
iline()
{
	register i;

	for (i = 0; i <= tsize; i++)
		if (EQN(wb.ut_line, tbuf[i].tline))
			return(i);
	if (++tsize >= TSIZE) {
		fprintf(stderr, "acctcon1: RECOMPILE WITH LARGER TSIZE\n");
		exit(2);
	}

	CPYN(tbuf[tsize].tline, wb.ut_line);
	tbuf[tsize].tdev = lintodev(wb.ut_line);
	return(tsize);
}

upall()
{
	register struct tbuf *tp;

	wb.ut_type = INIT_PROCESS;	/* fudge a logoff for reboot record */
	for (tp = tbuf; tp <= &tbuf[tsize]; tp++)
		update(tp);
}

/*
 * update tbuf with new time, write ctmp record for end of session
 */
update(tp)
struct tbuf *tp;
{
	long	told,	/* last time for tbuf record */
		tnew;	/* time of this record */
			/* Difference is connect time */

	told = tp->ttime;
	tnew = wb.ut_time;
	if (told > tnew) {
		fprintf(stderr, "acctcon1: bad times: old: %s", ctime(&told));
		fprintf(stderr, "new: %s", ctime(&tnew));
		exitcode = 1;
		return;
	}
	tp->ttime = tnew;
	switch(wb.ut_type) {
	case USER_PROCESS:
		tp->tlsess++;
		if(tp->tname[0] != '\0') { /* Someone logged in without */
					   /* logging off. Put out record. */
			cb.ct_tty = tp->tdev;
			CPYN(cb.ct_name, tp->tname);
			cb.ct_uid = namtouid(cb.ct_name);
			cb.ct_start = told;
			pnpsplit(cb.ct_start, tnew-told, cb.ct_con);
			prctmp(&cb);
			tp->ttotal += tnew-told;
		}
		else	/* Someone just logged in */
			tp->tlon++;
		CPYN(tp->tname, wb.ut_name);
		break;
	case INIT_PROCESS:
	case LOGIN_PROCESS:
		tp->tloff++;
		if(tp->tname[0] != '\0') { /* Someone logged off */
			/* Set up and print ctmp record */
			cb.ct_tty = tp->tdev;
			CPYN(cb.ct_name, tp->tname);
			cb.ct_uid = namtouid(cb.ct_name);
			pnpsplit(cb.ct_start, tnew-told, cb.ct_con);
			cb.ct_start = told;
			prctmp(&cb);
			tp->ttotal += tnew-told;
			tp->tname[0] = '\0';
		}
	}
}

printrep()
{
	register i;

	freopen(report, "w", stdout);
	printf("from %s", ctime(&firstime));
	printf("to   %s", ctime(&lastime));
	if (ndates)
		printf("%d\tdate change%c\n",ndates,(ndates>1 ? 's' : '\0'));
	for (i = 0; i < nsys; i++)
		printf("%d\t%.12s\n", sy[i].snum, sy[i].sname);
}

/*
 *	print summary of line usage
 *	accuracy only guaranteed for wtmp file started fresh
 */
printlin()
{
	register struct tbuf *tp;
	double timet, timei;
	double ttime;
	int tsess, ton, toff;

	freopen(replin, "w", stdout);
	ttime = 0.0;
	tsess = ton = toff = 0;
	timet = MINS(lastime-firstime);
	printf("TOTAL DURATION IS %.0f MINUTES\n", timet);
	printf("LINE\tMINUTES\tPERCENT\t# SESS\t# ON\t# OFF\n");
	for (tp = tbuf; tp <= &tbuf[tsize]; tp++) {
		timei = MINS(tp->ttotal);
		ttime += timei;
		tsess += tp->tlsess;
		ton += tp->tlon;
		toff += tp->tloff;
		printf("%.8s\t%.0f\t%.0f\t%d\t%d\t%d\n",
			tp->tline,
			timei,
			(timet > 0.)? 100*timei/timet : 0.,
			tp->tlsess,
			tp->tlon,
			tp->tloff);
	}
	printf("TOTALS\t%.0f\t--\t%d\t%d\t%d\n", ttime, tsess, ton, toff);
}

prctmp(t)
register struct ctmp *t;
{

	printf("%u\t%u\t%.8s\t%lu\t%lu\t%lu",
		t->ct_tty,
		t->ct_uid,
		t->ct_name,
		t->ct_con[0],
		t->ct_con[1],
		t->ct_start);
	printf("\t%s", ctime(&t->ct_start));
}
