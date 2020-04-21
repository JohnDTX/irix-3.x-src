#include <sys/types.h>
#include <utmp.h>
#include <errno.h>

#define USIZE	(sizeof (struct utmp))
#define	EQ(a,b)	(strcmp(a,b)==0)
extern errno;

static int f = -1;
static int ttys = -1;



mkutmp(name, line, host)
char *name, *line, *host;
{
#ifdef SYSTEMV
struct utmp u[200];
register struct utmp *up;
int slot, cc;

	if (f<0) {
		f = open("/etc/utmp", 2);
		if (f<0) {
			if (errno == ENOENT)
				f = creat("/etc/utmp", 0644);
			if (f<0)
				return(f);
		}
	}

	lseek(f, (long)0, 0);
	cc = read(f, u, sizeof u);

	for (up=u; cc >= USIZE; up++,cc -= USIZE) {
		if (EQ(up->ut_line, line)) {
			goto found;
		}
	}
	strncpy(up->ut_line, line, sizeof (up->ut_line));

found:
	slot = up - u;
	strncpy(up->ut_name, name, sizeof (up->ut_name));
	time(&up->ut_time);

	if (EQ(name, ""))
		up->ut_type = EMPTY;
	else
		up->ut_type = USER_PROCESS;
	up->ut_pid = getpid();
	up->ut_id[0] = up->ut_line[3];
	up->ut_id[1] = up->ut_line[4];
	up->ut_id[2] = up->ut_line[5];
	up->ut_id[3] = up->ut_line[6];

	lseek(f, (long)slot*USIZE, 0);
	write(f, up, USIZE);
	return(slot);
}
#endif
#ifdef BSD42
struct utmp utmp;
register t, w;

	time(&utmp.ut_time);
	if (f < 0) {
		f = open("/etc/utmp", 2);
		if (f < 0)
			return(0);
	}
	t = ttyslot(line);
	if (t > 0) {
		lseek(f, (long)(t*(USIZE)), 0);
		strncpy(utmp.ut_line, line, sizeof (utmp.ut_line));
		strncpy(utmp.ut_name, name, sizeof (utmp.ut_name));
		strncpy(utmp.ut_host, host, sizeof (utmp.ut_host));
		write(f, (char *)&utmp, USIZE);
	}
	if (t > 0 && (w = open("/usr/adm/wtmp", 1)) >= 0) {
		lseek(w, 0L, 2);
		write(w, (char *)&utmp, USIZE);
		close(w);
	}
	return(t);
}
#endif


rmutmp(slot)
{
struct utmp u;


	if (f<0) {
		f = open("/etc/utmp", 2);
		if (f<0) {
			if (errno == ENOENT)
				f = creat("/etc/utmp", 0644);
			if (f<0)
				return;
		}
	}

	lseek(f, (long)slot*USIZE, 0);
	read(f, &u, USIZE);
	u.ut_name[0] = 0;
#ifdef SYSTEMV
	u.ut_type = EMPTY;
#endif
	lseek(f, (long)slot*USIZE, 0);
	write(f, &u, USIZE);
}



endutmp()
{
	close(f);
	f = -1;
}

#ifdef BSD42
static char ttysbuf[8192];
ttyslot(ttyn)
char *ttyn;
{
register char *p, *q;
register slot;
static cc;

	if (ttys<0) {
		ttys = open("/etc/ttys", 0);
		cc = read(ttys, ttysbuf, sizeof ttysbuf);
		close(ttys);
		if (cc<4) {
			ttys = -1;
			return(0);
		}
	}

	for(slot=1,p=ttysbuf; p< &ttysbuf[cc]; ) {
		p += 2;
		q = p;
		while (*q && *q!='\n') {
			if (q >= &ttysbuf[cc])
				return(0);
			q++;
		}
		*q = 0;
		if (EQ(p, ttyn))
			return(slot);
		p = q + 1;
		slot++;
	}
	return(0);
}
#endif
