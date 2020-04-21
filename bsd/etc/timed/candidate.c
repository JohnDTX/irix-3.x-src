/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)candidate.c	2.3 (Berkeley) 4/21/86";
#endif not lint

#include "globals.h"
#include <protocols/timed.h>

#define ELECTIONWAIT	3	/* seconds */

/*
 * `election' candidates a host as master: it is called by a slave 
 * which runs with the -M option set when its election timeout expires. 
 * Note the conservative approach: if a new timed comes up, or another
 * candidate sends an election request, the candidature is withdrawn.
 */

election(net)
struct netinfo *net;
{
	int ret;
	struct tsp *resp, msg, *readmsg();
	struct timeval wait;
	struct tsp *answer, *acksend();
	long casual();
	struct sockaddr_in server;
#if defined(sgi) || defined(sgi_vax)
	char loop_lim = 0;
/* This code can get totally confused if it gets slightly behind.  For
 *	example, if readmsg() has some QUIT messages waiting from the last
 *	round, we would send an ELECTION message, get the stale QUIT,
 *	and give up.  This results in network storms when several machines
 *	do it at once.
 */
	wait.tv_sec = 0;
	wait.tv_usec = 0;
	while (0 != readmsg(TSP_REFUSE, (char *)ANYADDR, &wait, net)) {
		if (trace)
			fprintf(fd, "election: discarded stale REFUSE\n");
	}
	while (0 != readmsg(TSP_QUIT, (char *)ANYADDR, &wait, net)) {
		if (trace)
			fprintf(fd, "election: discarded stale QUIT\n");
	}
again:
#endif

	syslog(LOG_INFO, "THIS MACHINE IS A CANDIDATE");
	if (trace) {
		fprintf(fd, "THIS MACHINE IS A CANDIDATE\n");
	}

	ret = MASTER;
	slvcount = 1;

	msg.tsp_type = TSP_ELECTION;
	msg.tsp_vers = TSPVERSION;
	(void)strcpy(msg.tsp_name, hostname);
	bytenetorder(&msg);
	if (sendto(sock, (char *)&msg, sizeof(struct tsp), 0, &net->dest_addr,
	    sizeof(struct sockaddr_in)) < 0) {
#if defined(sgi) || defined(sgi_vax)
		if (trace)
			fprintf(fd, "election: sendto errno %d\n", errno);
		return(SLAVE);
#else
		syslog(LOG_ERR, "sendto: %m");
		exit(1);
#endif
	}

	do {
		wait.tv_sec = ELECTIONWAIT;
		wait.tv_usec = 0;
		resp = readmsg(TSP_ANY, (char *)ANYADDR, &wait, net);
		if (resp != NULL) {
			switch (resp->tsp_type) {

			case TSP_ACCEPT:
				(void) addmach(resp->tsp_name, &from);
				break;

			case TSP_MASTERUP:
			case TSP_MASTERREQ:
				/*
				 * If a timedaemon is coming up at the same time,
				 * give up the candidature: it will be the master.
				 */
#if defined(sgi) || defined(sgi_vax)
				if (++loop_lim < 5
				    && !good_host(&from.sin_addr.s_addr)) {
					suppress(&from, resp->tsp_name, net);
					goto again;
				}
#endif
				ret = SLAVE;
				break;

			case TSP_QUIT:
			case TSP_REFUSE:
				/*
				 * Collision: change value of election timer 
				 * using exponential backoff.
				 * The value of timer will be recomputed (in slave.c)
				 * using the original interval when election will 
				 * be successfully completed.
				 */
				backoff *= 2;
				delay2 = casual((long)MINTOUT, 
							(long)(MAXTOUT * backoff));
#if defined(sgi) || defined(sgi_vax)
				if (trace)
					fprintf(fd,
						"election: delay2=%d\n",
						delay2);
#endif
				ret = SLAVE;
				break;

			case TSP_ELECTION:
				/* no master for another round */
				msg.tsp_type = TSP_REFUSE;
				(void)strcpy(msg.tsp_name, hostname);
				server = from;
				answer = acksend(&msg, &server, resp->tsp_name,
				    TSP_ACK, (struct netinfo *)NULL);
				if (answer == NULL) {
					syslog(LOG_ERR, "error in election");
				} else {
					(void) addmach(resp->tsp_name, &from);
				}
				break;

			case TSP_SLAVEUP:
				(void) addmach(resp->tsp_name, &from);
				break;

			case TSP_SETDATE:
			case TSP_SETDATEREQ:
				break;

			default:
				if (trace) {
					fprintf(fd, "candidate: ");
					print(resp, &from);
				}
				break;
			}
		} else {
			break;
		}
	} while (ret == MASTER);
	return(ret);
}
