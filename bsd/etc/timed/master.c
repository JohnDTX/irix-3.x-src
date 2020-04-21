/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)master.c	2.14 (Berkeley) 6/5/86";
#endif not lint

#include "globals.h"
#include <protocols/timed.h>
#include <sys/file.h>
#include <setjmp.h>
#include <utmp.h>

extern int machup;
extern int measure_delta;
extern jmp_buf jmpenv;

extern u_short sequence;
#if defined(sgi) || defined(sgi_vax)
int dictate = 0;
#endif

#ifdef MEASURE
int header;
FILE *fp = NULL;
#endif

/*
 * The main function of `master' is to periodically compute the differences 
 * (deltas) between its clock and the clocks of the slaves, to compute the 
 * network average delta, and to send to the slaves the differences between 
 * their individual deltas and the network delta.
 * While waiting, it receives messages from the slaves (i.e. requests for
 * master's name, remote requests to set the network time, ...), and
 * takes the appropriate action.
 */

master()
{
	int ind;
	long pollingtime;
#if defined(sgi) || defined(sgi_vax)
	int polls = 0;
#endif
	struct timeval wait;
	struct timeval time;
	struct timeval otime;
	struct timezone tzone;
	struct tsp *msg, to;
	struct sockaddr_in saveaddr;
	int findhost();
	char *date();
	struct tsp *readmsg();
	struct tsp *answer, *acksend();
	char olddate[32];
	struct sockaddr_in server;
	register struct netinfo *ntp;

#ifdef MEASURE
	if (fp == NULL) {
		fp = fopen("/usr/adm/timed.masterlog", "w");
#ifndef sgi
		setlinebuf(fp);
#endif
	}
#endif

	syslog(LOG_INFO, "This machine is master");
	if (trace)
		fprintf(fd, "THIS MACHINE IS MASTER\n");

	for (ntp = nettab; ntp != NULL; ntp = ntp->next)
		if (ntp->status == MASTER)
			masterup(ntp);
	pollingtime = 0;

loop:
	(void)gettimeofday(&time, (struct timezone *)0);
#if defined(sgi) || defined(sgi_vax)
/* This code cause network storms if it gets too far behind.  (An obvious
 *	case of feedback and delay causing instability.)  Therefore, process
 *	all outstanding messages before spending the long time necessary
 *	to update all timers.  Clearly, this code had never been used in
 *	non-trivial networks.
 */
	wait.tv_sec = pollingtime - time.tv_sec;
	if (wait.tv_sec < 0)
		wait.tv_sec = 0;
	wait.tv_usec = 0;
	msg = readmsg(TSP_ANY, (char *)ANYADDR, &wait, (struct netinfo *)NULL);
	if (0 == msg)
#endif
	if (time.tv_sec >= pollingtime) {
#if defined(sgi) || defined(sgi_vax)
/* If a bogus master told us to quit, we can have decided to ignore a
 * network.  Therefore, periodically, try to take over everything.
 */
		polls = ((polls+1) & 3);
		if (!polls
		    && (setstatus(), nignorednets > 0)) {
			if (trace)
				fprintf(fd,"Looking for nets to re-master\n");
			for (ntp = nettab; ntp != NULL; ntp = ntp->next) {
				if (ntp->status == IGNORE) {
					lookformaster(ntp);
					if (ntp->status == MASTER) {
						masterup(ntp);
						polls++;
					}
				}
			}
			if (polls != 0) {
				if (trace)
					fprintf(fd,"Triumph.  ");
				setstatus();
				polls = 0;
			}
		}
#endif
		pollingtime = time.tv_sec + SAMPLEINTVL;
		synch(0L);

		for (ntp = nettab; ntp != NULL; ntp = ntp->next) {
			to.tsp_type = TSP_LOOP;
			to.tsp_vers = TSPVERSION;
			to.tsp_seq = sequence++;
			to.tsp_hopcnt = 10;
			(void)strcpy(to.tsp_name, hostname);
			bytenetorder(&to);
			if (sendto(sock, (char *)&to, sizeof(struct tsp), 0,
			    &ntp->dest_addr, sizeof(struct sockaddr_in)) < 0) {
#if defined(sgi) || defined(sgi_vax)
				if (trace)
					fprintf(fd,
						"master: sendto1 errno %d\n",
						errno);
#else
				syslog(LOG_ERR, "sendto: %m");
				exit(1);
#endif
			}
		}
	}

#if !defined(sgi) && !defined(sgi_vax)
	wait.tv_sec = pollingtime - time.tv_sec;
	wait.tv_usec = 0;
	msg = readmsg(TSP_ANY, (char *)ANYADDR, &wait, (struct netinfo *)NULL);
#endif
	if (msg != NULL) {
		switch (msg->tsp_type) {

		case TSP_MASTERREQ:
			break;
		case TSP_SLAVEUP:
			ind = addmach(msg->tsp_name, &from);
			newslave(ind, msg->tsp_seq);
			break;
		case TSP_SETDATE:
#if defined(sgi) || defined(sgi_vax)
			if (!good_host(&from.sin_addr.s_addr)) {
				(void)strcpy(olddate,
					     ctime(&msg->tsp_time.tv_sec));
				syslog(LOG_NOTICE,
				       "attempted date change by %s to: %s",
				       msg->tsp_name, olddate);
				spreadtime();
				break;
			}
#endif
			saveaddr = from;
			/*
			 * the following line is necessary due to syslog
			 * calling ctime() which clobbers the static buffer
			 */
			(void)strcpy(olddate, date());
			(void)gettimeofday(&time, &tzone);
			otime = time;
			time.tv_sec = msg->tsp_time.tv_sec;
			time.tv_usec = msg->tsp_time.tv_usec;
#ifdef sgi
			(void)stime(&time.tv_sec);
#else
			(void)settimeofday(&time, &tzone);
#endif
			syslog(LOG_NOTICE, "date changed from: %s", olddate);
			logwtmp(otime, time);
			msg->tsp_type = TSP_DATEACK;
			msg->tsp_vers = TSPVERSION;
			(void)strcpy(msg->tsp_name, hostname);
			bytenetorder(msg);
			if (sendto(sock, (char *)msg, sizeof(struct tsp), 0,
			    &saveaddr, sizeof(struct sockaddr_in)) < 0) {
				syslog(LOG_ERR, "sendto: %m");
#if defined(sgi) || defined(sgi_vax)
				if (trace)
					fprintf(fd,"master: sendto2 errno %d\n",
						errno);
#else
				exit(1);
#endif
			}
			spreadtime();
			pollingtime = 0;
			break;
		case TSP_SETDATEREQ:
			ind = findhost(msg->tsp_name);
			if (ind < 0) { 
			    syslog(LOG_WARNING,
				"DATEREQ from uncontrolled machine");
			    break;
			}
			if (hp[ind].seq !=  msg->tsp_seq) {
				hp[ind].seq = msg->tsp_seq;
#if defined(sgi) || defined(sgi_vax)
				if (!hp[ind].good) {
					(void)strcpy(olddate, 
					       ctime(&msg->tsp_time.tv_sec));
					syslog(LOG_NOTICE,
					   "attempted SETDATEREQ by %s to: %s",
					       msg->tsp_name, olddate);
					spreadtime();
					break;
				}
#endif
				/*
				 * the following line is necessary due to syslog
				 * calling ctime() which clobbers the static buffer
				 */
				(void)strcpy(olddate, date());
				(void)gettimeofday(&time, &tzone);
				otime = time;
				time.tv_sec = msg->tsp_time.tv_sec;
				time.tv_usec = msg->tsp_time.tv_usec;
#ifdef sgi
				(void)stime(&time.tv_sec);
#else
				(void)settimeofday(&time, &tzone);
#endif
				syslog(LOG_NOTICE,
				    "date changed by %s from: %s",
				    msg->tsp_name, olddate);
				logwtmp(otime, time);
				spreadtime();
				pollingtime = 0;
			}
			break;
		case TSP_MSITE:
		case TSP_MSITEREQ:
			break;
		case TSP_TRACEON:
			if (!(trace)) {
				fd = fopen(tracefile, "w");
#ifndef sgi
				setlinebuf(fd);
#endif
				fprintf(fd, "Tracing started on: %s\n\n", 
							date());
			}
			trace = ON;
			break;
		case TSP_TRACEOFF:
			if (trace) {
				fprintf(fd, "Tracing ended on: %s\n", date());
				(void)fclose(fd);
			}
#ifdef GPROF
			moncontrol(0);
			_mcleanup();
			moncontrol(1);
#endif
			trace = OFF;
			break;
		case TSP_ELECTION:
			to.tsp_type = TSP_QUIT;
			(void)strcpy(to.tsp_name, hostname);
			server = from;
			answer = acksend(&to, &server, msg->tsp_name, TSP_ACK,
			    (struct netinfo *)NULL);
			if (answer == NULL) {
				syslog(LOG_ERR, "election error");
			} else {
				(void) addmach(msg->tsp_name, &from);
			}
			pollingtime = 0;
			break;
		case TSP_CONFLICT:
			/*
			 * After a network partition, there can be 
			 * more than one master: the first slave to 
			 * come up will notify here the situation.
			 */

			(void)strcpy(to.tsp_name, hostname);

			if (fromnet == NULL)
				break;
#if defined(sgi) || defined(sgi_vax)
			/* someone had a lapse here.  The other master often
			 * gets into the same state, with boring results.
			 */
			for (ind = 0; ind < 3; ind++) {
#else
			for(;;) {
#endif
				to.tsp_type = TSP_RESOLVE;
				answer = acksend(&to, &fromnet->dest_addr,
				    (char *)ANYADDR, TSP_MASTERACK, fromnet);
				if (answer == NULL)
					break;
				to.tsp_type = TSP_QUIT;
				server = from;
				msg = acksend(&to, &server, answer->tsp_name,
				    TSP_ACK, (struct netinfo *)NULL);
				if (msg == NULL) {
					syslog(LOG_ERR, "error on sending QUIT");
				} else {
					(void) addmach(answer->tsp_name, &from);
				}
			}
			masterup(fromnet);
			pollingtime = 0;
			break;
		case TSP_RESOLVE:
			/*
			 * do not want to call synch() while waiting
			 * to be killed!
			 */
			(void)gettimeofday(&time, (struct timezone *)0);
			pollingtime = time.tv_sec + SAMPLEINTVL;
			break;
		case TSP_QUIT:
			/* become slave */
#ifdef MEASURE
			if (fp != NULL) {
				(void)fclose(fp);
				fp = NULL;
			}
#endif
			longjmp(jmpenv, 2);
			break;
		case TSP_LOOP:
			/*
			 * We should not have received this from a net
			 * we are master on.  There must be two masters
			 * in this case.
			 */
			to.tsp_type = TSP_QUIT;
			(void)strcpy(to.tsp_name, hostname);
			server = from;
			answer = acksend(&to, &server, msg->tsp_name, TSP_ACK,
				(struct netinfo *)NULL);
			if (answer == NULL) {
				syslog(LOG_WARNING,
				    "loop breakage: no reply to QUIT");
			} else {
				(void)addmach(msg->tsp_name, &from);
			}
		default:
			if (trace) {
				fprintf(fd, "garbage: ");
				print(msg, &from);
			}
			break;
#if defined(sgi) || defined(sgi_vax)
		case TSP_TEST:
			if (trace) {
				fprintf(fd,
		"\tnets = %d, masters = %d, slaves = %d, ignored = %d\n",
		nnets, nmasternets, nslavenets, nignorednets);
				setstatus();
			}
			break;
#endif
		}
	}
	goto loop;
}

/*
 * `synch' synchronizes all the slaves by calling measure, 
 * networkdelta and correct 
 */

synch(mydelta)
long mydelta;
{
	int i;
	int measure_status;
	long netdelta;
	struct timeval tack;
#ifdef MEASURE
#define MAXLINES	8
	static int lines = 1;
	struct timeval start, end;
#endif
	int measure();
	int correct();
	long networkdelta();
	char *date();

	if (slvcount > 1) {
#ifdef MEASURE
		(void)gettimeofday(&start, (struct timezone *)0);
		if (header == ON || --lines == 0) {
			fprintf(fp, "%s\n", date());
			for (i=0; i<slvcount; i++)
				fprintf(fp, "%.7s\t", hp[i].name);
			fprintf(fp, "\n");
			lines = MAXLINES;
			header = OFF;
		}
#endif
		machup = 1;
		hp[0].delta = 0;
		for(i=1; i<slvcount; i++) {
			tack.tv_sec = 0;
			tack.tv_usec = 500000;
			if ((measure_status = measure(&tack, &hp[i].addr)) <0) {
				syslog(LOG_ERR, "measure: %m");
#if defined(sgi) || defined(sgi_vax)
				if (trace)
					fprintf(fd,"master: measure failed\n");
				rmmach(i);
#else
				exit(1);
#endif
			}
			hp[i].delta = measure_delta;
			if (measure_status == GOOD)
				machup++;
		}
		if (status & SLAVE) {
			/* called by a submaster */
			if (trace)
				fprintf(fd, "submaster correct: %d ms.\n",
				    mydelta);
			correct(mydelta);	
		} else {
			if (machup > 1) {
#if defined(sgi) || defined(sgi_vax)
				if (!dictate) 
					netdelta = networkdelta();
				else {
					dictate--;
					netdelta = 0;
				}
#else
				netdelta = networkdelta();
#endif
				if (trace)
#if defined(sgi) || defined(sgi_vax)
					fprintf(fd, "master correct: %d ms.\n",
						netdelta);
#else
					fprintf(fd,
					    "master correct: %d ms.\n",
					    mydelta);
#endif
				correct(netdelta);
			}
		}
#ifdef MEASURE
		gettimeofday(&end, 0);
		end.tv_sec -= start.tv_sec;
		end.tv_usec -= start.tv_usec;
		if (end.tv_usec < 0) {
			end.tv_sec -= 1;
			end.tv_usec += 1000000;
		}
		fprintf(fp, "%d ms.\n", (end.tv_sec*1000+end.tv_usec/1000));
#endif
		for(i=1; i<slvcount; i++) {
			if (hp[i].delta == HOSTDOWN) {
				rmmach(i);
#ifdef MEASURE
				header = ON;
#endif
			}
		}
	} else {
		if (status & SLAVE) {
			correct(mydelta);
		}
	}
}

/*
 * 'spreadtime' sends the time to each slave after the master
 * has received the command to set the network time 
 */

spreadtime()
{
	int i;
	struct tsp to;
	struct tsp *answer, *acksend();

#if defined(sgi) || defined(sgi_vax)
/* Do not listen to the consensus after forcing the time.  This is because
 *	the consensus takes a while to reach the time we are dictating.
 */
	dictate = 2;
#endif
	for(i=1; i<slvcount; i++) {
		to.tsp_type = TSP_SETTIME;
		(void)strcpy(to.tsp_name, hostname);
		(void)gettimeofday(&to.tsp_time, (struct timezone *)0);
		answer = acksend(&to, &hp[i].addr, hp[i].name, TSP_ACK,
		    (struct netinfo *)NULL);
		if (answer == NULL) {
			syslog(LOG_WARNING,
			    "no reply to SETTIME from: %s", hp[i].name);
#if defined(sgi) || defined(sgi_vax)
			rmmach(i);
#endif
		}
	}
}

#if defined(sgi) || defined(sgi_vax)
findhost(name)
register char *name;
{
	register int i, ind;
	register struct host *hpp;
	register char c;

	c = *name;			/* do it faster */
	hpp = &hp[1];
	i = slvcount;
	while (--i > 0) {
		if (c == hpp->name[0] && !strcmp(name, hpp->name))
			return (slvcount-i);
		hpp++;
	}
	return (-1);
}
#else
findhost(name)
char *name;
{
	int i;
	int ind;

	ind = -1;
	for (i=1; i<slvcount; i++) {
		if (strcmp(name, hp[i].name) == 0) {
			ind = i;
			break;
		}
	}
	return(ind);
}
#endif

/*
 * 'addmach' adds a host to the list of controlled machines
 * if not already there 
 */

addmach(name, addr)
char *name;
struct sockaddr_in *addr;
{
	int ret;
	int findhost();

	ret = findhost(name);
	if (ret < 0) {
		hp[slvcount].addr = *addr;
		hp[slvcount].name = (char *)malloc(MAXHOSTNAMELEN);
		(void)strcpy(hp[slvcount].name, name);
		hp[slvcount].seq = 0;
		ret = slvcount;
		if (slvcount < NHOSTS)
			slvcount++;
		else {
			syslog(LOG_ERR, "no more slots in host table");
		}
	} else {
		/* need to clear sequence number anyhow */
		hp[ret].seq = 0;
	}
#if defined(sgi) || defined(sgi_vax)
	hp[ret].good = good_host(&addr->sin_addr.s_addr);
#endif
#ifdef MEASURE
	header = ON;
#endif
	return(ret);
}

/*
 * Remove all the machines from the host table that exist on the given
 * network.  This is called when a master transitions to a slave on a
 * given network.
 */

rmnetmachs(ntp)
	register struct netinfo *ntp;
{
	int i;

	if (trace)
		prthp();
	for (i = 1; i < slvcount; i++)
		if ((hp[i].addr.sin_addr.s_addr & ntp->mask) == ntp->net)
			rmmach(i--);
	if (trace)
		prthp();
}

/*
 * remove the machine with the given index in the host table.
 */
rmmach(ind)
	int ind;
{
	if (trace)
		fprintf(fd, "rmmach: %s\n", hp[ind].name);
	free(hp[ind].name);
	hp[ind] = hp[--slvcount];
}

prthp()
{
	int i;

	fprintf(fd, "host table:");
	for (i=1; i<slvcount; i++)
		fprintf(fd, " %s", hp[i].name);
	fprintf(fd, "\n");
}

masterup(net)
struct netinfo *net;
{
	struct timeval wait;
	struct tsp to, *msg, *readmsg();

	to.tsp_type = TSP_MASTERUP;
	to.tsp_vers = TSPVERSION;
	(void)strcpy(to.tsp_name, hostname);
	bytenetorder(&to);
	if (sendto(sock, (char *)&to, sizeof(struct tsp), 0, &net->dest_addr,
	    sizeof(struct sockaddr_in)) < 0) {
		syslog(LOG_ERR, "sendto: %m");
#if defined(sgi) || defined(sgi_vax)
		if (trace)
			fprintf(fd,"masterup: sendto errno %d\n", errno);
#else
		exit(1);
#endif
	}

#if defined(sgi) || defined(sgi_vax)
	wait.tv_sec = 1;		/* wait a total of 1 second */
	wait.tv_usec = 0;
	for (;;) {
		msg = readmsg(TSP_SLAVEUP, (char *)ANYADDR, &wait, net);
		if (msg != NULL) {
			(void) addmach(msg->tsp_name, &from);
		} else
			break;
		wait.tv_sec = 0;
		wait.tv_usec = 0;
	}
#else
	for (;;) {
		wait.tv_sec = 1;
		wait.tv_usec = 0;
		msg = readmsg(TSP_SLAVEUP, (char *)ANYADDR, &wait, net);
		if (msg != NULL) {
			(void) addmach(msg->tsp_name, &from);
		} else
			break;
	}
#endif
}

newslave(ind, seq)
u_short seq;
{
	struct tsp to;
	struct tsp *answer, *acksend();

	if (trace)
		prthp();
	if (seq == 0 || hp[ind].seq !=  seq) {
		hp[ind].seq = seq;
		to.tsp_type = TSP_SETTIME;
		(void)strcpy(to.tsp_name, hostname);
		/*
		 * give the upcoming slave the time
		 * to check its input queue before
		 * setting the time
		 */
#ifdef sgi
		sginap(HZ);
#else
		sleep(1);
#endif
		(void) gettimeofday(&to.tsp_time,
		    (struct timezone *)0);
		answer = acksend(&to, &hp[ind].addr,
		    hp[ind].name, TSP_ACK,
		    (struct netinfo *)NULL);
		if (answer == NULL) {
			syslog(LOG_WARNING,
			    "no reply to initial SETTIME from: %s",
			    hp[ind].name);
			rmmach(ind);
		}
	}
}

#ifdef sgi
struct utmp wtmp[2] = {
	{"","",OTIME_MSG,0,OLD_TIME,0,0,0},
	{"","",NTIME_MSG,0,NEW_TIME,0,0,0}
};
char *wtmpfile = WTMP_FILE;
#else
char *wtmpfile = "/usr/adm/wtmp";
struct utmp wtmp[2] = {
	{ "|", "", "", 0 },
	{ "{", "", "", 0 }
};
#endif
logwtmp(otime, ntime)
struct timeval otime, ntime;
{
	int f;

	wtmp[0].ut_time = otime.tv_sec + (otime.tv_usec + 500000) / 1000000;
	wtmp[1].ut_time = ntime.tv_sec + (ntime.tv_usec + 500000) / 1000000;
	if (wtmp[0].ut_time == wtmp[1].ut_time)
		return;
#ifdef sgi
	pututline(&wtmp[0]);
	pututline(&wtmp[1]);
	endutent();
#endif
	if ((f = open(wtmpfile, O_WRONLY|O_APPEND)) >= 0) {
		(void) write(f, (char *)wtmp, sizeof(wtmp));
		(void) close(f);
	}
}
