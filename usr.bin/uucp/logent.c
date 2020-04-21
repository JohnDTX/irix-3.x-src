/* @(#)logent.c	1.3 */
#include "uucp.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

time_t time();

char Tmplog[MAXFULLNAME] = "";
FILE *Lp = NULL;

FILE	*Lf = NULL;
FILE	*Sf = NULL;

/*
 * Make log entry
 *	text	-> ptr to text string
 *	status	-> ptr to status string
 * Returns:
 *	none
 */
logent(text, status)
register char *text, *status;
{
	register struct tm *tp;
	extern struct tm *localtime();
	time_t clock;
	struct stat s1;
	int	i,j;
	int	mask;

	if (nstat.t_pid == 0)
		nstat.t_pid = getpid();
	if(Lf == NULL){
		mask = umask(0);
		if((Lf = fopen(LOGFILE, "a")) == NULL){
			umask(mask);
			return;
		}
		umask(mask);
	}
	time(&clock);
	tp = localtime(&clock);
/*
	for(i=0,j=0;i<20;i++){
		if(fstat(i, &s1) == -1)
			continue;
		j++;
	}
*/
	fprintf(Lf, "%s!%s ", Rmtname, User);
	fprintf(Lf, "(%d/%d-%d:%2.2d:%2.2d) (%.1c,%d,%d) ", tp->tm_mon + 1,
	  tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec, Pchar, nstat.t_pid,
	  seqn);
	fprintf(Lf, "%s (%s)\n", status, text);
	fflush(Lf);
	return;
}

/*
 * Make entry for a conversation (uucico only)
 *	text	-> pointer to message string
 * Returns:
 *	none
 */
syslog(text)
register char *text;
{
	register struct tm *tp;
	extern struct tm *localtime();
	time_t clock;
	long	td,th,tm,ts;
	int	mask;

	if(Sf == NULL){
		mask = umask(0);
		if((Sf = fopen(SYSLOG, "a")) == NULL){
			umask(mask);
			return;
		}
		umask(mask);
	}
	if(nstat.t_pid == 0)
		nstat.t_pid = getpid();
	time(&clock);
	tp = localtime(&clock);
	fprintf(Sf, "%s!%s %s ", Rmtname, User, Role == SLAVE?"S":"M");
	fprintf(Sf, "(%d/%d-%d:%2.2d:%2.2d) ", tp->tm_mon + 1,
		tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);
	fprintf(Sf, "(%.1c,%d,%d) ",
		Pchar, nstat.t_pid,  seqn);
	td = clock - nstat.t_qtime;
	ts = td%60;
	td /= 60;
	tm = td%60;
	td /= 60;
	th = td;
	fprintf(Sf, "(%ld:%ld:%ld) ", th, tm, ts);
	fprintf(Sf, " %s\n [%ld:%ld:%ld:%ld:%ld:%ld:%ld:%ld] [%s,%ld,%d,%ld,%d]\n", 
		text,
		(nstat.t_exf - nstat.t_beg)/60,
		(nstat.t_scall - nstat.t_beg)/60,
		(nstat.t_ecall - nstat.t_scall)/60,
		(nstat.t_sftp - nstat.t_ecall)/60,
		(nstat.t_sxf - nstat.t_sftp)/60,
		(nstat.t_exf - nstat.t_sxf)/60,
		(nstat.t_txfe.tms_stime + nstat.t_txfe.tms_cstime 
		 - nstat.t_txfs.tms_stime - nstat.t_txfs.tms_cstime) ,
		(nstat.t_txfe.tms_utime + nstat.t_txfe.tms_cutime 
		 - nstat.t_txfs.tms_utime - nstat.t_txfs.tms_cutime),
		&dc[0], nstat.t_tacu, nstat.t_ndial, nstat.t_tlog, nstat.t_nlogs);
	nstat.t_sxf = nstat.t_exf;
	nstat.t_txfs = nstat.t_txfe;
	fflush(Sf);
	return;
}

/*
 * Close log files before a fork
 */
closelog()
{

	if(Sf){
		fflush(Sf);
		Sf = NULL;
	}
	if(Lf){
		fflush(Lf);
		Lf = NULL;
	}
}
logcls()
{
}

