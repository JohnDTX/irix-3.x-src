/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)slave.c	2.16 (Berkeley) 6/5/86";
#endif not lint

#include "globals.h"
#include <protocols/timed.h>
#include <setjmp.h>

extern jmp_buf jmpenv;

extern u_short sequence;

slave()
{
	int length;
	int senddateack;
	long electiontime, refusetime, looktime;
	u_short seq;
#if defined(sgi) || defined(sgi_vax)
	struct in_addr cadr;
#endif
	char candidate[MAXHOSTNAMELEN];
	struct tsp *msg, to, *readmsg();
	struct sockaddr_in saveaddr, msaveaddr;
	struct timeval wait;
	struct timeval time, otime;
	struct tsp *answer, *acksend();
	int timeout();
	char *date();
	long casual();
	int bytenetorder();
	char olddate[32];
	struct sockaddr_in server;
	register struct netinfo *ntp;
	int ind;
	struct tsp resp;
	extern int Mflag;
	extern int justquit;
#ifdef MEASURE
	extern FILE *fp;
#endif
	if (slavenet) {
		resp.tsp_type = TSP_SLAVEUP;
		resp.tsp_vers = TSPVERSION;
		(void)strcpy(resp.tsp_name, hostname);
		bytenetorder(&resp);
		if (sendto(sock, (char *)&resp, sizeof(struct tsp), 0,
		    &slavenet->dest_addr, sizeof(struct sockaddr_in)) < 0) {
			syslog(LOG_ERR, "sendto: %m");
#if defined(sgi) || defined(sgi_vax)
			if (trace)
				fprintf(fd,"slave: sendto1 errno %d\n",
					errno);
#else
			exit(1);
#endif
		}
	}

	if (status & MASTER) {
#ifdef MEASURE
		if (fp == NULL) {
			fp = fopen("/usr/adm/timed.masterlog", "w");
#ifndef sgi
			setlinebuf(fp);
#endif
		}
#endif
		syslog(LOG_INFO, "THIS MACHINE IS A SUBMASTER");
		if (trace) {
			fprintf(fd, "THIS MACHINE IS A SUBMASTER\n");
		}
		for (ntp = nettab; ntp != NULL; ntp = ntp->next)
			if (ntp->status == MASTER)
				masterup(ntp);

	} else {
		syslog(LOG_INFO, "THIS MACHINE IS A SLAVE");
		if (trace) {
			fprintf(fd, "THIS MACHINE IS A SLAVE\n");
		}
	}

	seq = 0;
	senddateack = OFF;
	refusetime = 0;

	(void)gettimeofday(&time, (struct timezone *)0);
	electiontime = time.tv_sec + delay2;
	if (Mflag)
		if (justquit)
			looktime = time.tv_sec + delay2;
		else 
			looktime = 1;
	else
		looktime = 0;

loop:
	length = sizeof(struct sockaddr_in);
	(void)gettimeofday(&time, (struct timezone *)0);
	if (time.tv_sec > electiontime) {
		if (trace) 
			fprintf(fd, "election timer expired\n");
		longjmp(jmpenv, 1);
	}
	if (looktime && time.tv_sec > looktime) {
		if (trace) 
			fprintf(fd, "Looking for nets to master and loops\n");
		
		if (nignorednets > 0) {
			for (ntp = nettab; ntp != NULL; ntp = ntp->next) {
				if (ntp->status == IGNORE) {
					lookformaster(ntp);
					if (ntp->status == MASTER)
						masterup(ntp);
					else
						ntp->status = IGNORE;
				}
			}
			setstatus();
#ifdef MEASURE
			/*
			 * Check to see if we just became master
			 * (file not open)
			 */
			if (fp == NULL) {
				fp = fopen("/usr/adm/timed.masterlog", "w");
#ifndef sgi
				setlinebuf(fp);
#endif
			}
#endif
		}

		for (ntp = nettab; ntp != NULL; ntp = ntp->next) {
		    if (ntp->status == MASTER) {
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
						"slave: sendto2 errno %d\n",
						errno);
#else
				syslog(LOG_ERR, "sendto: %m");
				exit(1);
#endif
			}
		    }
		}
		(void)gettimeofday(&time, (struct timezone *)0);
		looktime = time.tv_sec + delay2;
	}
	wait.tv_sec = electiontime - time.tv_sec + 10;
	wait.tv_usec = 0;
	msg = readmsg(TSP_ANY, (char *)ANYADDR, &wait, (struct netinfo *)NULL);
	if (msg != NULL) {
		switch (msg->tsp_type) {
		case TSP_SETDATE:
#ifdef TESTING
		case TSP_TEST:
#endif
		case TSP_MSITE:
		case TSP_TRACEOFF:
		case TSP_TRACEON:
			break;
		case TSP_MASTERUP:
			if (fromnet == NULL) {
				if (trace) {
					fprintf(fd, "slave ignored: ");
					print(msg, &from);
				}
				goto loop;
			}
			break;
		default:
			if (fromnet == NULL || fromnet->status == IGNORE) {
				if (trace) {
					fprintf(fd, "slave ignored: ");
					print(msg, &from);
				}
				goto loop;
			}
			break;
		}

		switch (msg->tsp_type) {

		case TSP_ADJTIME:
			if (fromnet->status != SLAVE)
				break;
#if defined(sgi) || defined(sgi_vax)
			if (!good_host(&from.sin_addr.s_addr)) {
				syslog(LOG_NOTICE,
				       "attempted time adjustment by %s",
				       msg->tsp_name);
				suppress(&from, msg->tsp_name, fromnet);
				break;
			}
#endif
			(void)gettimeofday(&time, (struct timezone *)0);
			electiontime = time.tv_sec + delay2;
			if (seq != msg->tsp_seq) {
				seq = msg->tsp_seq;
				if ((status & SUBMASTER) == SUBMASTER) {
					synch((msg->tsp_time.tv_sec * 1000) + 
					    (msg->tsp_time.tv_usec / 1000));
				} else {
					adjclock(&(msg->tsp_time));
				}
			}
			break;
		case TSP_SETTIME:
			if (fromnet->status != SLAVE)
				break;
			if (seq == msg->tsp_seq)
				break;

			seq = msg->tsp_seq;

			(void)strcpy(olddate, date());
			(void)gettimeofday(&otime, (struct timezone *)0);
#if defined(sgi) || defined(sgi_vax)
			if (!good_host(&from.sin_addr.s_addr)) {
				syslog(LOG_NOTICE,
				       "attempted time setting by %s",
				       msg->tsp_name);
				suppress(&from, msg->tsp_name, fromnet);
				break;
			}
/* This fixes a bad protocol bug.  Imagine a system with several networks,
 * where there are a pair of redundant gateways between a pair of networks,
 * each running timed.  Assume that we start with a third machine mastering
 * one of the networks, and one of the gateways mastering the other.
 * Imagine that the third machine goes away and the non-master gateway
 * decides to replace it.  If things are timed just 'right,' we will have
 * each gateway mastering one network for a little while.  If a SETTIME
 * message gets into the network at that time, perhaps from the newly
 * masterful gateway as it was taking control, the SETTIME will loop
 * forever.  Each time a gateway receives it on its slave side, it will
 * call spreadtime to forward it on its mastered network.  We are now in
 * a permanent loop, since the SETTIME msgs will keep any clock
 * in the network from advancing.  Normally, the 'LOOP' stuff will detect
 * and correct the situation.  However, with the clocks stopped, the 
 * 'looktime' timer cannot expire.  While they are in this state, the
 * masters will try to saturate the network with SETTIME packets.
 */
			if ((status & SUBMASTER) == SUBMASTER
			    && looktime > 1)
				looktime--;
			if (otime.tv_sec - MAXADJ < msg->tsp_time.tv_sec
			    && msg->tsp_time.tv_sec < otime.tv_sec + MAXADJ) {
				if (trace)
					fprintf(fd,
					    "slave: ignoring SETTIME from %s",
						date());
			} else {
#ifdef sgi
				(void)stime(&msg->tsp_time.tv_sec);
#else
				(void)settimeofday(&msg->tsp_time,
						   (struct timezone *)0);
#endif
				syslog(LOG_NOTICE,
				       "date changed by %s from: %s",
					msg->tsp_name, olddate);
				if ((status & SUBMASTER) == SUBMASTER)
					spreadtime();
			}
#else
			(void)settimeofday(&msg->tsp_time,
				(struct timezone *)0);
			syslog(LOG_NOTICE, "date changed by %s from: %s",
				msg->tsp_name, olddate);
			if ((status & SUBMASTER) == SUBMASTER)
				spreadtime();
#endif
			(void)gettimeofday(&time, (struct timezone *)0);
			electiontime = time.tv_sec + delay2;

			if (senddateack == ON) {
				senddateack = OFF;
				msg->tsp_type = TSP_DATEACK;
				(void)strcpy(msg->tsp_name, hostname);
				bytenetorder(msg);
				length = sizeof(struct sockaddr_in);
				if (sendto(sock, (char *)msg, 
						sizeof(struct tsp), 0,
						&saveaddr, length) < 0) {
					syslog(LOG_ERR, "sendto: %m");
#if defined(sgi) || defined(sgi_vax)
					if (trace)
						fprintf(fd,
						    "slave: sendto3 errno %d\n",
							errno);
#else
					exit(1);
#endif
				}
			}
			break;
		case TSP_MASTERUP:
#if defined(sgi) || defined(sgi_vax)
			if (!good_host(&from.sin_addr.s_addr)) {
				suppress(&from, msg->tsp_name, fromnet);
				break;
			}
#endif
			if (slavenet && fromnet != slavenet)
				break;
			makeslave(fromnet);
			setstatus();
			msg->tsp_type = TSP_SLAVEUP;
			msg->tsp_vers = TSPVERSION;
			(void)strcpy(msg->tsp_name, hostname);
			bytenetorder(msg);
			answerdelay();
			length = sizeof(struct sockaddr_in);
			if (sendto(sock, (char *)msg, sizeof(struct tsp), 0, 
						&from, length) < 0) {
				syslog(LOG_ERR, "sendto: %m");
#if defined(sgi) || defined(sgi_vax)
				if (trace)
					fprintf(fd,
						"slave: sendto4 errno %d\n",
						errno);
#else
				exit(1);
#endif
			}
			backoff = 1;
			delay2 = casual((long)MINTOUT, (long)MAXTOUT);
#if defined(sgi) || defined(sgi_vax)
			if (trace)
				fprintf(fd, "slave: delay2=%d\n", delay2);
#endif
			(void)gettimeofday(&time, (struct timezone *)0);
			electiontime = time.tv_sec + delay2;
			refusetime = 0;
			break;
		case TSP_MASTERREQ:
			if (fromnet->status != SLAVE)
				break;
			(void)gettimeofday(&time, (struct timezone *)0);
			electiontime = time.tv_sec + delay2;
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
			msg->tsp_type = TSP_SETDATEREQ;
			msg->tsp_vers = TSPVERSION;
			(void)strcpy(msg->tsp_name, hostname);
			for (ntp = nettab; ntp != NULL; ntp = ntp->next) {
				if (ntp->status == SLAVE)
					break;
			}
			if (ntp == NULL)
				break;
			answer = acksend(msg, &ntp->dest_addr, (char *)ANYADDR,
			    TSP_DATEACK, ntp);
			if (answer != NULL) {
				msg->tsp_type = TSP_ACK;
				bytenetorder(msg);
				length = sizeof(struct sockaddr_in);
				if (sendto(sock, (char *)msg,
				    sizeof(struct tsp), 0, &saveaddr,
				    length) < 0) {
					syslog(LOG_ERR, "sendto: %m");
#if defined(sgi) || defined(sgi_vax)
					if (trace)
						fprintf(fd,
						    "slave: sendto5 errno %d\n",
							errno);
#else
					exit(1);
#endif
				}
				senddateack = ON;
			}
			break;
		case TSP_SETDATEREQ:
			saveaddr = from;
			if (status != SUBMASTER || fromnet->status != MASTER)
				break;
			for (ntp = nettab; ntp != NULL; ntp = ntp->next) {
				if (ntp->status == SLAVE)
					break;
			}
			ind = findhost(msg->tsp_name);
			if (ind < 0) {
			    syslog(LOG_WARNING,
				"DATEREQ from uncontrolled machine");
			    break;
			}
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
			syslog(LOG_DEBUG,
			    "forwarding date change request for %s",
			    msg->tsp_name);
			(void)strcpy(msg->tsp_name, hostname);
			answer = acksend(msg, &ntp->dest_addr, (char *)ANYADDR,
			    TSP_DATEACK, ntp);
			if (answer != NULL) {
				msg->tsp_type = TSP_DATEACK;
				bytenetorder(msg);
				length = sizeof(struct sockaddr_in);
				if (sendto(sock, (char *)msg,
				    sizeof(struct tsp), 0, &saveaddr,
				    length) < 0) {
					syslog(LOG_ERR, "sendto: %m");
#if defined(sgi) || defined(sgi_vax)
					if (trace)
						fprintf(fd,
						    "slave: sendto6 errno %d\n",
							errno);
#else
					exit(1);
#endif
				}
			}
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
		case TSP_SLAVEUP:
			if ((status & MASTER) && fromnet->status == MASTER) {
				ind = addmach(msg->tsp_name, &from);
				newslave(ind, msg->tsp_seq);
			}
			break;
		case TSP_ELECTION:
			if (fromnet->status == SLAVE) {
				(void)gettimeofday(&time, (struct timezone *)0);
				electiontime = time.tv_sec + delay2;
				seq = 0;            /* reset sequence number */
#if defined(sgi) || defined(sgi_vax)
				if (!good_host(&from.sin_addr.s_addr)) {
					syslog(LOG_NOTICE,
					       "suppress election of %s",
					       msg->tsp_name);
					msg->tsp_type = TSP_QUIT;
				} else if (cadr.s_addr != from.sin_addr.s_addr
					   && time.tv_sec < refusetime) {
/* if the candidate has to repeat itself, the old code would refuse it
 * the second time.  That would prevent elections.
 */
					msg->tsp_type = TSP_REFUSE;
				} else {
					cadr.s_addr = from.sin_addr.s_addr;
					msg->tsp_type = TSP_ACCEPT;
					refusetime = time.tv_sec + 30;
				}
#else
				if (time.tv_sec < refusetime)
					msg->tsp_type = TSP_REFUSE;
				else {
					msg->tsp_type = TSP_ACCEPT;
					refusetime = time.tv_sec + 30;
				}
#endif
				(void)strcpy(candidate, msg->tsp_name);
				(void)strcpy(msg->tsp_name, hostname);
				answerdelay();
				server = from;
				answer = acksend(msg, &server, candidate, TSP_ACK,
				    (struct netinfo *)NULL);
				if (answer == NULL)
					syslog(LOG_WARNING,
					   "no answer from master candidate\n");
			} else {	/* fromnet->status == MASTER */
				to.tsp_type = TSP_QUIT;
				(void)strcpy(to.tsp_name, hostname);
				server = from;
				answer = acksend(&to, &server, msg->tsp_name,
				    TSP_ACK, (struct netinfo *)NULL);
				if (answer == NULL) {
					syslog(LOG_WARNING,
					    "election error: no reply to QUIT");
				} else {
					(void) addmach(msg->tsp_name, &from);
				}
			}
			break;
                case TSP_CONFLICT:
			if (fromnet->status != MASTER)
				break;
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
                                        syslog(LOG_WARNING,
					    "conflict error: no reply to QUIT");
				} else {
                                        (void) addmach(answer->tsp_name, &from);
				}
                        }
                        masterup(fromnet);
                        break;
		case TSP_MSITE:
			if (!slavenet)
				break;
			msaveaddr = from;
			msg->tsp_type = TSP_MSITEREQ;
			msg->tsp_vers = TSPVERSION;
			(void)strcpy(msg->tsp_name, hostname);
			answer = acksend(msg, &slavenet->dest_addr,
					 (char *)ANYADDR, TSP_ACK, slavenet);
			if (answer != NULL) {
				msg->tsp_type = TSP_ACK;
				length = sizeof(struct sockaddr_in);
				bytenetorder(msg);
				if (sendto(sock, (char *)msg, 
						sizeof(struct tsp), 0,
						&msaveaddr, length) < 0) {
					syslog(LOG_ERR, "sendto: %m");
#if defined(sgi) || defined(sgi_vax)
					if (trace)
						fprintf(fd,
						    "slave: sendto7 errno %d\n",
							errno);
#else
					exit(1);
#endif
				}
			}
			break;
		case TSP_ACCEPT:
		case TSP_REFUSE:
			break;
		case TSP_RESOLVE:
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
#ifdef TESTING
		case TSP_TEST:
			electiontime = 0;
			break;
#endif
		case TSP_MSITEREQ:
			if (status & MASTER)
				break;
			if (trace) {
				fprintf(fd, "garbage: ");
				print(msg, &from);
			}
			break;

		case TSP_LOOP:
			/* looking for loops of masters */
			if ( !(status & MASTER))
				break;
			if (fromnet->status == SLAVE) {
			    if ( !strcmp(msg->tsp_name, hostname)) {
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
					(char *)ANYADDR, TSP_MASTERACK,
					fromnet);
				    if (answer == NULL)
					    break;
				    to.tsp_type = TSP_QUIT;
				    (void)strcpy(to.tsp_name, hostname);
				    server = from;
				    answer = acksend(&to, &server,
					answer->tsp_name, TSP_ACK,
					(struct netinfo *)NULL);
				    if (answer == NULL) {
					syslog(LOG_ERR, "loop kill error");
				    } else {
					electiontime = 0;
				    }
				  }
			    } else {
				if (msg->tsp_hopcnt-- <= 0)
				    break;
				bytenetorder(msg);
				ntp = nettab;
				for (; ntp != NULL; ntp = ntp->next)
				    if (ntp->status == MASTER)
					if (sendto(sock, (char *)msg, 
					    sizeof(struct tsp), 0,
					    &ntp->dest_addr, length) < 0) {
						syslog(LOG_ERR, "sendto: %m");
#if defined(sgi) || defined(sgi_vax)
					    if (trace)
						    fprintf(fd,
						"slave: sendto8 errno %d\n",
							    errno);
#else
						exit(1);
#endif
					}
			    }
			} else {
			    /*
			     * We should not have received this from a net
			     * we are master on.  There must be two masters
			     * in this case.
			     */
			    if (fromnet->my_addr.s_addr == from.sin_addr.s_addr)
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
				    (char *)ANYADDR, TSP_MASTERACK,
				    fromnet);
				if (answer == NULL)
					break;
				to.tsp_type = TSP_QUIT;
				(void)strcpy(to.tsp_name, hostname);
				server = from;
				answer = acksend(&to, &server, answer->tsp_name,
				    TSP_ACK, (struct netinfo *)NULL);
				if (answer == NULL) {
					syslog(LOG_ERR, "loop kill error2");
				} else {
					(void)addmach(msg->tsp_name, &from);
				}
			    }
			}
			break;
		default:
			if (trace) {
				fprintf(fd, "garbage: ");
				print(msg, &from);
			}
			break;
		}
	}
	goto loop;
}

/*
 * Used before answering a broadcast message to avoid network
 * contention and likely collisions.
 */
answerdelay()
{
#if defined(sgi) || defined(sgi_vax)
/* This is wrong.  If all machines answer at once, then there might be
 * lots of ethernet collisions and so forth.  However, by delaying,
 * we can make the protocol machine go crazy.  It is so fragile (wrong?)
 * that the slow-down caused by tracing can make it get farther and farther
 * behind and that makes it go totally crazy.
 */
#else
	struct timeval timeout;

	timeout.tv_sec = 0;
	timeout.tv_usec = delay1;

	(void)select(0, (fd_set *)NULL, (fd_set *)NULL, (fd_set *)NULL,
	    &timeout);
	return;
#endif
}
