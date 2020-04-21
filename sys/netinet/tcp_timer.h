/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)tcp_timer.h	7.1 (Berkeley) 6/5/86
 */

/*
 * Definitions of the TCP timers.  These timers are counted
 * down PR_SLOWHZ times a second.
 */
#define	TCPT_NTIMERS	4

#define	TCPT_REXMT	0		/* retransmit */
#define	TCPT_PERSIST	1		/* retransmit persistance */
#define	TCPT_KEEP	2		/* keep alive */
#define	TCPT_2MSL	3		/* 2*msl quiet time timer */

/*
 * The TCPT_REXMT timer is used to force retransmissions.
 * The TCP has the TCPT_REXMT timer set whenever segments
 * have been sent for which ACKs are expected but not yet
 * received.  If an ACK is received which advances tp->snd_una,
 * then the retransmit timer is cleared (if there are no more
 * outstanding segments) or reset to the base value (if there
 * are more ACKs expected).  Whenever the retransmit timer goes off,
 * we retransmit one unacknowledged segment, and do a backoff
 * on the retransmit timer.
 *
 * The TCPT_PERSIST timer is used to keep window size information
 * flowing even if the window goes shut.  If all previous transmissions
 * have been acknowledged (so that there are no retransmissions in progress),
 * and the window is too small to bother sending anything, then we start
 * the TCPT_PERSIST timer.  When it expires, if the window is nonzero,
 * we go to transmit state.  Otherwise, at intervals send a single byte
 * into the peer's window to force him to update our window information.
 * We do this at most as often as TCPT_PERSMIN time intervals,
 * but no more frequently than the current estimate of round-trip
 * packet time.  The TCPT_PERSIST timer is cleared whenever we receive
 * a window update from the peer.
 *
 * The TCPT_KEEP timer is used to keep connections alive.  If an
 * connection is idle (no segments received) for TCPTV_KEEP amount of time,
 * but not yet established, then we drop the connection.  If the connection
 * is established, then we force the peer to send us a segment by sending:
 *	<SEQ=SND.UNA-1><ACK=RCV.NXT><CTL=ACK>
 * This segment is (deliberately) outside the window, and should elicit
 * an ack segment in response from the peer.  If, despite the TCPT_KEEP
 * initiated segments we cannot elicit a response from a peer in TCPT_MAXIDLE
 * amount of time, then we drop the connection.
 */

#define	TCP_TTL		30		/* time to live for TCP segs */
/*
 * Time constants.
 */
#define	TCPTV_MSL	( 15*PR_SLOWHZ)		/* max seg lifetime */
#ifdef sgi
#define	TCPTV_SRTTBASE	(0*SRTT_SCALE)		/* base roundtrip time;
						   if 0, no idea yet */
#define	TCPTV_SRTTDFLT	(3*PR_SLOWHZ*SRTT_SCALE) /* assumed RTT if no info */
#else
#define	TCPTV_SRTTBASE	0			/* base roundtrip time;
						   if 0, no idea yet */
#define	TCPTV_SRTTDFLT	(  3*PR_SLOWHZ)		/* assumed RTT if no info */
#endif

#define	TCPTV_KEEP	( 45*PR_SLOWHZ)		/* keep alive - 45 secs */
#define	TCPTV_PERSMIN	(  5*PR_SLOWHZ)		/* retransmit persistance */

#define	TCPTV_MAXIDLE	(  8*TCPTV_KEEP)	/* maximum allowable idle
						   time before drop conn */

#define	TCPTV_MIN	(  1*PR_SLOWHZ)		/* minimum allowable value */
#define	TCPTV_MAX	( 30*PR_SLOWHZ)		/* maximum allowable value */

#define	TCP_LINGERTIME	120			/* linger at most 2 minutes */

#define	TCP_MAXRXTSHIFT	12			/* maximum retransmits */

#ifdef	TCPTIMERS
char *tcptimers[] =
    { "REXMT", "PERSIST", "KEEP", "2MSL" };
#endif

/*
 * Retransmission smoothing constants.
 * Smoothed round trip time is updated by
 *    tp->t_srtt = (tcp_alpha * tp->t_srtt) + ((1 - tcp_alpha) * tp->t_rtt)
 * each time a new value of tp->t_rtt is available.  The initial
 * retransmit timeout is then based on
 *    tp->t_timer[TCPT_REXMT] = tcp_beta * tp->t_srtt;
 * limited, however to be at least TCPTV_MIN and at most TCPTV_MAX.
 */
#ifdef sgi
#define	SRTT_TYPE long
#define SRTT_SCALE 64			/* scaling to avoid floating point */
SRTT_TYPE tcp_alpha, tcp_beta;
#else
float	tcp_alpha, tcp_beta;
#endif

/*
 * Initial values of tcp_alpha and tcp_beta.
 * These are conservative: averaging over a long
 * period of time, and allowing for large individual deviations from
 * tp->t_srtt.
 */
#ifdef sgi
#define	TCP_ALPHA	(9*SRTT_SCALE/10)
#define	TCP_BETA	(20*SRTT_SCALE/10)
#else
#define	TCP_ALPHA	0.9
#define	TCP_BETA	2.0
#endif

/*
 * Force a time value to be in a certain range.
 */
#define	TCPT_RANGESET(tv, value, tvmin, tvmax) { \
	(tv) = (value); \
	if ((tv) < (tvmin)) \
		(tv) = (tvmin); \
	if ((tv) > (tvmax)) \
		(tv) = (tvmax); \
}
