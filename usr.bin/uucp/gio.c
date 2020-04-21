/* @(#)gio.c	1.3 */
#define USER 1
#include "pk.p"
#include <sys/param.h>
#include <signal.h>
#ifndef RT
#include <sys/types.h>
#endif
#ifndef RT
#include <sys/sysmacros.h>
#endif
#include "pk.h"
#include <sys/buf.h>
#include <setjmp.h>
#include "uucp.h"

time_t time();

jmp_buf Failbuf;

struct pack *Pk;

pkfail()
{
	longjmp(Failbuf, 1);
}

gturnon()
{
	int ret;
	struct pack *pkopen();
	if (setjmp(Failbuf))
		return(FAIL);
	if (Pkdrvon) {
		ret = pkon(Ofn, PACKSIZE);
		DEBUG(4, "pkon - %d ", ret);
		DEBUG(4, "Ofn - %d\n", Ofn);
		if (ret <= 0)
			return(FAIL);
	}
	else {
		if (Debug > 4)
			pkdebug = 1;
		Pk = pkopen(Ifn, Ofn);
		if ((int) Pk == NULL)
			return(FAIL);
	}
	return(0);
}


gturnoff()
{
	if(setjmp(Failbuf))
		return(FAIL);
	if (Pkdrvon)
		pkoff(Ofn);
	else
		pkclose(Pk);
	return(0);
}


gwrmsg(type, str, fn)
char type, *str;
{
	char bufr[BUFSIZ], *s;
	int len, i;

	if(setjmp(Failbuf))
		return(FAIL);
	bufr[0] = type;
	s = &bufr[1];
	while (*str)
		*s++ = *str++;
	*s = '\0';
	if (*(--s) == '\n')
		*s = '\0';
	len = strlen(bufr) + 1;
	if ((i = len % PACKSIZE)) {
		len = len + PACKSIZE - i;
		bufr[len - 1] = '\0';
	}
	gwrblk(bufr, len, fn);
	return(0);
}


grdmsg(str, fn)
char *str;
{
	unsigned len;

	if(setjmp(Failbuf))
		return(FAIL);
	for (;;) {
		if (Pkdrvon)
			len = read(fn, str, PACKSIZE);
		else
			len = pkread(Pk, str, PACKSIZE);
		if (len == 0)
			continue;
		str += len;
		if (*(str - 1) == '\0')
			break;
	}
	return(0);
}


gwrdata(fp1, fn)
FILE *fp1;
{
	char bufr[BUFSIZ];
	int len;
	int ret;
	time_t t1, t2;
	long bytes;
	char text[BUFSIZ];

	if(setjmp(Failbuf))
		return(FAIL);
	bytes = 0L;
	time(&t1);
	nstat.t_sxf = times(&nstat.t_txfs);
	while ((len = fread(bufr, sizeof (char), BUFSIZ, fp1)) > 0) {
		bytes += len;
		ret = gwrblk(bufr, len, fn);
		if (ret != len) {
			return(FAIL);
		}
		if (len != BUFSIZ)
			break;
	}
	ret = gwrblk(bufr, 0, fn);
	time(&t2);
	nstat.t_exf = times(&nstat.t_txfe);
	sprintf(text, "-> %ld / %ld secs", bytes, t2 - t1);
	DEBUG(1, "%s\n", text);
	syslog(text);
	sysacct(bytes, t2 - t1);
	return(0);
}


grddata(fn, fp2)
FILE *fp2;
{
	int len;
	char bufr[BUFSIZ];
	time_t t1, t2;
	long bytes;
	char text[BUFSIZ];

	if(setjmp(Failbuf))
		return(FAIL);
	bytes = 0L;
	time(&t1);
	nstat.t_sxf = times(&nstat.t_txfs);
	for (;;) {
		len = grdblk(bufr, BUFSIZ, fn);
		if (len < 0) {
			return(FAIL);
		}
		bytes += len;
		fwrite(bufr, sizeof (char), len, fp2);
		if (len < BUFSIZ)
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


grdblk(blk, len,  fn)
char *blk;
{
	int i, ret;

	for (i = 0; i < len; i += ret) {
		if (Pkdrvon)
			ret = read(fn, blk, (unsigned) len - i);
		else
			ret = pkread(Pk, blk, len - i);
		if (ret < 0)
			return(FAIL);
		blk += ret;
		if (ret == 0)
			return(i);
	}
	return(i);
}


gwrblk(blk, len, fn)
char *blk;
{
	int ret;

	if (Pkdrvon)
		ret = write(fn, blk, (unsigned) len);
	else
		ret = pkwrite(Pk, blk, len);
	return(ret);
}
