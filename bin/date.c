/*
 *	date - with format capabilities
 */
char _Origin_[] = "System V";

/*	@(#)date.c	1.5	*/
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/bin/RCS/date.c,v 1.1 89/03/27 14:50:18 root Exp $";

#include	<sys/types.h>
#include	"utmp.h"
#include        <stdio.h>
#include	<time.h>

#define	MONTH	itoa(tim->tm_mon+1,cp,2)
#define	DAY	itoa(tim->tm_mday,cp,2)
#define	YEAR	itoa(tim->tm_year,cp,2)
#define	HOUR	itoa(tim->tm_hour,cp,2)
#define	MINUTE	itoa(tim->tm_min,cp,2)
#define	SECOND	itoa(tim->tm_sec,cp,2)
#define	JULIAN	itoa(tim->tm_yday+1,cp,3)
#define	WEEKDAY	itoa(tim->tm_wday,cp,1)
#define	MODHOUR	itoa(h,cp,2)
#define	dysize(A) (((A)%4)? 365: 366)

int dmsize[12]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

char	month[12][3] = {
	"Jan","Feb","Mar","Apr",
	"May","Jun","Jul","Aug",
	"Sep","Oct","Nov","Dec"
};

char	days[7][3] = {
	"Sun","Mon","Tue","Wed",
	"Thu","Fri","Sat"
};

char	*itoa();
char	*cbp;
long	timbuf;

struct	utmp	wtmp[2] = { {"","",OTIME_MSG,0,OLD_TIME,0,0,0},
			    {"","",NTIME_MSG,0,NEW_TIME,0,0,0} };

/* !=0 if we should not try to set the network time */
int nflag = 0;


main(argc, argv)
int	argc;
int	**argv;
{
	register char	*aptr, *cp, c;
	int	 h, hflag, i, tfailed, wf;
	long	tbuf, time(), lseek();
	struct	tm  *tim;
	char	 buf[200], *tzn;

	tfailed = 0;
	if(argc > 1) {
		cbp = (char *)argv[1];

		if(*cbp == '+')	{
			hflag = 0;
			for(cp=buf;cp< &buf[200];)
				*cp++ = '\0';
			tzset();
			(void) time(&tbuf);
			tim = localtime(&tbuf);
			tzn = tzname[tim->tm_isdst];
			aptr = (char *)argv[1];
			aptr++;
			cp = buf;
			while(c = *aptr++) {
			if(c == '%')
			switch(*aptr++) {
			case '%':
				*cp++ = '%';
				continue;
			case 'n':
				*cp++ = '\n';
				continue;
			case 't':
				*cp++ = '\t';
				continue;
			case 'm' :
				cp = MONTH;
				continue;
			case 'd':
				cp = DAY;
				continue;
			case 'y' :
				cp = YEAR;
				continue;
			case 'D':
				cp = MONTH;
				*cp++ = '/';
				cp = DAY;
				*cp++ = '/';
				cp = YEAR;
				continue;
			case 'H':
				cp = HOUR;
				continue;
			case 'M':
				cp = MINUTE;
				continue;
			case 'S':
				cp = SECOND;
				continue;
			case 'T':
				cp = HOUR;
				*cp++ = ':';
				cp = MINUTE;
				*cp++ = ':';
				cp = SECOND;
				continue;
			case 'j':
				cp = JULIAN;
				continue;
			case 'w':
				cp = WEEKDAY;
				continue;
			case 'r':
				if((h = tim->tm_hour) >= 12)
					hflag++;
				if((h %= 12) == 0)
					h = 12;
				cp = MODHOUR;
				*cp++ = ':';
				cp = MINUTE;
				*cp++ = ':';
				cp = SECOND;
				*cp++ = ' ';
				if(hflag)
					*cp++ = 'P';
				else *cp++ = 'A';
				*cp++ = 'M';
				continue;
			case 'h':
				for(i=0; i<3; i++)
					*cp++ = month[tim->tm_mon][i];
				continue;
			case 'a':
				for(i=0; i<3; i++)
					*cp++ = days[tim->tm_wday][i];
				continue;
			case 'z':
				if (tzn)
					for(i=0; i<3; i++)
						*cp++ = tolower(tzn[i]);
				continue;
			case 'Z':
				if (tzn)
					for(i=0; i<3; i++)
						*cp++ = tzn[i];
				continue;
			default:
				(void) fprintf(stderr, "date: bad format character - %c\n", *--aptr);
				exit(2);
			}	/* endsw */
			*cp++ = c;
			}	/* endwh */

			*cp = '\n';
			(void) write(1,buf,(cp - &buf[0]) + 1);
			exit(0);
		}

		if (!strcmp(cbp, "-n")) {
			nflag++;
			if (--argc <= 1) {
				usage();
				exit(2);
			}
			++argv;
			cbp = (char *)argv[1];
		}

		if(gtime()) {
			usage();
			exit(2);
		}

	/* convert to Greenwich time, on assumption of Standard time. */
		timbuf += timezone;

	/* Now fix up to local daylight time. */
		if (localtime(&timbuf)->tm_isdst)
			timbuf += -1*60*60;

		(void) time(&wtmp[0].ut_time);

		if ((nflag || !settime(timbuf)) && stime(&timbuf) < 0) {
			tfailed++;
			(void)fprintf(stderr,"date: no permission\n");
		} else {
			(void) time(&wtmp[1].ut_time);

/*	Attempt to write entries to the utmp file and to the wtmp file.	*/

			pututline(&wtmp[0]) ;
			pututline(&wtmp[1]) ;

			if ((wf = open(WTMP_FILE, 1)) >= 0) {
				(void) lseek(wf, 0L, 2);
				(void) write(wf, (char *)wtmp, sizeof(wtmp));
			}
		}
	}
	if (tfailed==0)
		(void) time(&timbuf);
	cbp = ctime(&timbuf);
	(void) write(1, cbp, 20);
	tzn = tzname[localtime(&timbuf)->tm_isdst];
	if (tzn)
		(void) write(1, tzn, 3);
	(void) write(1, cbp+19, 6);
	exit(tfailed?2:0);
}

gtime()
{
	register int i;
	register int y, t;
	int d, h, m, s;
	long nt;

	tzset();

	t = gpair();
	if(t<1 || t>12)
		return(1);
	d = gpair();
	if(d<1 || d>31)
		return(1);
	h = gpair();
	if(h == 24) {
		h = 0;
		d++;
	}
	m = gpair();
	if(m<0 || m>59)
		return(1);
	if (*cbp == '.') {
		cbp++;
		s = gpair();
		if(s<0 || s>59)
			return(1);
	} else
		s = 0;
	y = gpair();
	if (y<0) {
		(void) time(&nt);
		y = localtime(&nt)->tm_year;
	}
	if (*cbp == 'p')
		h += 12;
	if (h<0 || h>23)
		return(1);
	timbuf = 0;
	y += 1900;
	for(i=1970; i<y; i++)
		timbuf += dysize(i);
	/* Leap year */
	if (dysize(y)==366 && t >= 3)
		timbuf += 1;
	while(--t)
		timbuf += dmsize[t-1];
	timbuf += (d-1);
	timbuf *= 24;
	timbuf += h;
	timbuf *= 60;
	timbuf += m;
	timbuf *= 60;
	timbuf += s;
	return(0);
}


gpair()
{
	register int c, d;
	register char *cp;

	cp = cbp;
	if(*cp == 0)
		return(-1);
	c = (*cp++ - '0') * 10;
	if (c<0 || c>=100)
		return(-1);
	if(*cp == 0)
		return(-1);
	if ((d = *cp++ - '0') < 0 || d > 9)
		return(-1);
	cbp = cp;
	return (c+d);
}

char *
itoa(i,ptr,dig)
register  int	i;
register  int	dig;
register  char	*ptr;
{
	switch(dig)	{
		case 3:
			*ptr++ = i/100 + '0';
			i = i - i / 100 * 100;
		case 2:
			*ptr++ = i / 10 + '0';
		case 1:	
			*ptr++ = i % 10 + '0';
	}
	return(ptr);
}
usage()
{
	fprintf(stderr, "usage: date [[-n] mmddhhmm[.ss][yy]]\n");
}


#include <sys/time.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define TSPTYPES
#include <protocols/timed.h>

#define WAITACK		2	/* seconds */
#define WAITDATEACK	5	/* seconds */

extern	int errno;
/*
 * Set the date in the machines controlled by timedaemons
 * by communicating the new date to the local timedaemon. 
 * If the timedaemon is in the master state, it performs the
 * correction on all slaves.  If it is in the slave state, it
 * notifies the master that a correction is needed.
 * Returns 1 on success, 0 on failure.
 */
settime(tv)
time_t tv;
{
	int s, length, port, timed_ack, found, err;
	long waittime;
	fd_set ready;
	char hostname[MAXHOSTNAMELEN+1];
	struct timeval tout;
	struct servent *sp;
	struct tsp msg;
	struct sockaddr_in sin, dest, from;

	sp = getservbyname("timed", "udp");
	if (sp == 0) {
		fprintf(stderr, "udp/timed: unknown service\n");
		return (0);
	}	
	dest.sin_port = sp->s_port;
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = htonl((u_long)INADDR_ANY);
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		if (errno != EPROTONOSUPPORT)
			perror("date: socket");
		goto bad;
	}
	bzero((char *)&sin, sizeof (sin));
	sin.sin_family = AF_INET;
	for (port = IPPORT_RESERVED - 1; port > IPPORT_RESERVED / 2; port--) {
		sin.sin_port = htons((u_short)port);
		if (bind(s, (struct sockaddr *)&sin, sizeof (sin)) >= 0)
			break;
		if (errno != EADDRINUSE) {
			if (errno != EADDRNOTAVAIL)
				perror("date: bind");
			goto bad;
		}
	}
	if (port == IPPORT_RESERVED / 2) {
		fprintf(stderr, "date: All ports in use\n");
		goto bad;
	}
	msg.tsp_type = TSP_SETDATE;
	msg.tsp_vers = TSPVERSION;
	(void) gethostname(hostname, sizeof (hostname));
	(void) strncpy(msg.tsp_name, hostname, sizeof (hostname));
	msg.tsp_seq = htons((u_short)0);
	msg.tsp_time.tv_sec = htonl((u_long)tv);
	msg.tsp_time.tv_usec = htonl(0);
	length = sizeof (struct sockaddr_in);
	if (connect(s, &dest, length) < 0) {
		perror("date: connect");
		goto bad;
	}
	if (send(s, (char *)&msg, sizeof (struct tsp), 0) < 0) {
		if (errno != ECONNREFUSED)
			perror("date: send");
		goto bad;
	}
	timed_ack = -1;
	waittime = WAITACK;
loop:
	tout.tv_sec = waittime;
	tout.tv_usec = 0;
	FD_ZERO(&ready);
	FD_SET(s, &ready);
	found = select(FD_SETSIZE, &ready, (fd_set *)0, (fd_set *)0, &tout);
	length = sizeof(err);
	if (getsockopt(s, SOL_SOCKET, SO_ERROR, (char *)&err, &length) == 0
	    && err) {
		errno = err;
		if (errno != ECONNREFUSED)
			perror("date: send (delayed error)");
		goto bad;
	}
	if (found > 0 && FD_ISSET(s, &ready)) {
		length = sizeof (struct sockaddr_in);
		if (recvfrom(s, (char *)&msg, sizeof (struct tsp), 0, &from,
		    &length) < 0) {
			if (errno != ECONNREFUSED)
				perror("date: recvfrom");
			goto bad;
		}
		msg.tsp_seq = ntohs(msg.tsp_seq);
		msg.tsp_time.tv_sec = ntohl(msg.tsp_time.tv_sec);
		msg.tsp_time.tv_usec = ntohl(msg.tsp_time.tv_usec);
		switch (msg.tsp_type) {

		case TSP_ACK:
			timed_ack = TSP_ACK;
			waittime = WAITDATEACK;
			goto loop;

		case TSP_DATEACK:
			(void)close(s);
			return (1);

		default:
			fprintf(stderr,
			    "date: Wrong ack received from timed: %s\n", 
			    tsptype[msg.tsp_type]);
			timed_ack = -1;
			break;
		}
	}
	if (timed_ack == -1)
		fprintf(stderr,
		    "date: Can't reach time daemon, time set locally.\n");
bad:
	(void)close(s);
	return (0);
}

