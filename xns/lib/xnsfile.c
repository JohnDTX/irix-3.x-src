/*
 * Find an available special file.
 */
#include <sys/ioctl.h>
#include <xns/Xnsioctl.h>

#ifndef BASENAME
#define	NETDEV	"/dev/tty"
#endif

char _xnsttyslot[32];		/* slot name (as in /etc/utmp) */
char _xnsfilename[32];		/* full path name of special file */
char xnsmsg[128];		/* external error messages */

/*
 * Return pathname of special file, given its "number".
 * Leave copy of pathname in _xnsfilename.
 * Leave slot name in _xnsttyslot.
 */
char *
xnspathname(num)
register num;
{

#ifdef SYSTEMV
	sprintf(_xnsfilename, "%sn%d", NETDEV, num);
	sprintf(_xnsttyslot, "%sn%d", "tty", num);
#else
	sprintf(_xnsfilename, "%s%c%x", NETDEV, 'a'+(num>>4), num&0xf);
	sprintf(_xnsttyslot, "%s%c%x", "tty", 'a'+(num>>4), num&0xf);
#endif
	return(_xnsfilename);
}

extern	int errno;

xnsfile()
{
	register f, net, trip;
	char *name;
	short num;

	f = open(name=xnspathname(0), 0);
	if (f<0) {
		sprintf(xnsmsg, "Can't open %s: check permissions\n", name);
		return(f);
	}


	/*
	 * make 4 attempts.
	 */
	for(trip=0; trip<4; trip++) {
		ioctl(f, NXAVAIL, &num);
		if (num<0) {
			close(f);
			sprintf(xnsmsg, "All network files busy\n");
			return(num);
		}

		net = open(xnspathname(num), 2);
		if (net>=0) {
			ioctl(net, NXIOTEST, &num);
			if (num>1) {
				close(net);
				continue;
			}
			close(f);
			return(net);
		}
	}
	close(f);
	if (net<0)
		sprintf(xnsmsg, "All network files busy\n");
	return(net);
}


xnserror(s)
char *s;
{
	if (*s)
		write(2, s, strlen(s));
	if (xnsmsg[0])
		write(2, xnsmsg, strlen(xnsmsg));
}
