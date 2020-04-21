#include <signal.h>
#include <setjmp.h>
#include <sys/ioctl.h>
#include <xns/Xnsioctl.h>

static jmp_buf env;
static sigflag;

timeout()
{
	longjmp(env, -1);
}
hangup()
{
	if (sigflag) {
		longjmp(env, -1);
	} else {
		signal(SIGHUP, hangup);
	}
}

setvec()
{
#ifdef BSD4.2
	vec.sv_handler = timeout;
	sigvec(SIGALRM, &vec, 0);
	vec.sv_handler = hangup;
	sigvec(SIGHUP, &vec, 0);
#endif
	signal(SIGALRM, timeout);
	signal(SIGHUP, hangup);
	signal(SIGINT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
}

tread(fd, buf, cc, secs)
char *buf;
register cc;
{
	alarm(secs);
	sigflag++;
	if (setjmp(env)<0) {
		cc = -1;
	} else {
		cc = read(fd, buf, cc);
		alarm(0);
	}
	sigflag = 0;
	return(cc);
}


twrite(fd, buf, cc, secs)
char *buf;
register cc;
{
	alarm(secs);
	sigflag++;
	if (setjmp(env)<0) {
		cc = -1;
	} else {
		cc = write(fd, buf, cc);
		alarm(0);
	}
	sigflag = 0;
	return(cc);
}



tsend(fd, buf, cc, secs, dtype)
char *buf;
register cc;
{
struct xnsio io;

	alarm(secs);
	sigflag++;
	if (setjmp(env)<0) {
		cc = -1;
	} else {
		io.addr = buf;
		io.count = cc;
		io.dtype = dtype;
		ioctl(fd, NXWRITE, &io);
		cc = io.count;
		alarm(0);
	}
	sigflag = 0;
	return(cc);
}
