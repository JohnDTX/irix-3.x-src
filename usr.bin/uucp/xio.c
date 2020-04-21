/* @(#)xio.c	1.4 */
#include <sys/param.h>
#include <signal.h>
#ifndef RT
#include <sys/types.h>
#endif
#ifndef RT
#include <sys/sysmacros.h>
#endif
#include <sys/buf.h>
#include <setjmp.h>
#include "uucp.h"

#define XBUFSIZ 512
time_t time();
static jmp_buf Failbuf;

/*
 * x.25 protocol
 */
static xalarm() {longjmp(Failbuf);}
static int (*xsig)();

/*
 * trun on protocol timer
 */
xturnon()
{
	xsig=signal(SIGALRM, xalarm);
	return(0);
}
xturnoff()
{
	signal(SIGALRM, xsig);
	return(0);
}

/*
 * write message across x.25 link
 *	type	-> message type
 *	str	-> message body (ascii string)
 *	fn	-> x.25 file descriptor
 * return
 *	0
 */
xwrmsg(type, str, fn)
register char *str;
int fn;
char type;
{
	register char *s;
	char bufr[XBUFSIZ];

	bufr[0] = type;
	s = &bufr[1];
	while (*str)
		*s++ = *str++;
	*s = '\0';
	if (*(--s) == '\n')
		*s = '\0';
	write(fn, bufr, (unsigned) strlen(bufr) + 1);
	return(0);
}

/*
 * read message from x.25 link
 *	str	-> message buffer
 *	fn	-> x.25 file descriptor
 * return
 *	FAIL	-> send timed out
 *	0	-> ok message in str
 */
xrdmsg(str, fn)
register char *str;
{

	register unsigned len;

	if(setjmp(Failbuf))
		return(FAIL);

	for (;;) {
		alarm(60);
		if( (len = read(fn, str, XBUFSIZ)) == 0)
			continue;
		str += len;
		if (*(str - 1) == '\0')
			break;
	}
	alarm(0);
	return(0);
}

/*
 * read data from file fp1 and write
 * on x.25 link
 *	fp1	-> file descriptor
 *	fn	-> x.25 descriptor
 * returns:
 *	FAIL	->failure in x.25 link
 *	0	-> ok
 */
xwrdata(fp1, fn)
register FILE *fp1;
{
	register int len, ret;
	long bytes;
	char bufr[XBUFSIZ];
	char text[XBUFSIZ];
	time_t t1, t2;

	bytes = 0L;
	time(&t1);
	nstat.t_sxf = times(&nstat.t_txfs);
	while ((len = fread(bufr, sizeof (char), XBUFSIZ, fp1)) > 0) {
		bytes += len;
		ret = write(fn, bufr, (unsigned) len);
		if (ret != len) {
			return(FAIL);
		}
		if (len != XBUFSIZ)
			break;
	}
	ret = write(fn, bufr, (unsigned) 0);
	time(&t2);
	nstat.t_exf = times(&nstat.t_txfe);
	sprintf(text, "-> %ld / %ld  sec", bytes, t2 - t1);
	DEBUG(1, "%s\n", text);
	syslog(text);
	sysacct(bytes, t2 - t1);
	return(0);
}

/*
 * read data from x.25 link and
 * write into file
 *	fp2	-> file descriptor
 *	fn	-> x.25 descriptor
 * returns:
 *	0	-> ok
 *	FAIL	-> failure on x.25 link
 */
xrddata(fn, fp2)
register FILE *fp2;
{
	register int len;
	long bytes;
	char text[XBUFSIZ];
	char bufr[XBUFSIZ];
	time_t t1, t2;

	bytes = 0L;
	time(&t1);
	nstat.t_sxf = times(&nstat.t_txfs);
	for (;;) {
		len = xrdblk(bufr, XBUFSIZ, fn);
		if (len < 0) {
			return(FAIL);
		}
		bytes += len;
		fwrite(bufr, sizeof (char), len, fp2);
		if (len < XBUFSIZ)
			break;
	}
	time(&t2);
	nstat.t_exf = times(&nstat.t_txfe);
	sprintf(text, "<- %ld / %ld secs", bytes, t2 - t1);
	DEBUG(1, "%s\n", text);
	syslog(text);
	sysacct(bytes, t2 - t1);
	return(0);
}

/*
 * read blank from x.25 link
 * reads are timed
 *	blk	-> address of buffer
 *	len	-> size to read
 *	fn	-> x.25 descriptor
 * returns:
 *	FAIL	-> link error timeout on link
 *	i	-> # of bytes read
 */
xrdblk(blk, len,  fn)
register char *blk;
{
	register int i, ret;

	if(setjmp(Failbuf))
		return(FAIL);

	for (i = 0; i < len; i += ret) {
		alarm(60);
		if ((ret = read(fn, blk, (unsigned) len - i)) < 0) {
			alarm(0);
			return(FAIL);
		}
		blk += ret;
		if (ret == 0)
			break;
	}
	alarm(0);
	return(i);
}
