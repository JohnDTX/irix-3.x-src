/*
 * Netisr simulator for non vaxen
 */
#ifdef SVR3
#include "../tcp-param.h"
#include "sys/sbd.h"
#include "sys/param.h"
#include "sys/mbuf.h"
#else
#include "../h/param.h"
#include "../h/mbuf.h"
#include "cypress.h"
#endif
#include "netisr.h"

#ifndef SVR3
char	netisrflag;

setsoftnet()
{
	int s;

	s = splmax();
	if (netisrflag == 0)
		netisrflag = 1;
	splx(s);
}
#endif

#ifdef SVR3
netintr()
#else
service_net()
#endif
{
	int s;

#ifdef SVR3
	acksoftnet();
#endif
	if (netisr) {
		s = splnet();
#ifdef	IMP
		if (netisr & SCHEDBIT(NETISR_IMP)) {
			netisr &= ~NETISR_IMP;
			impintr();
		}
#endif
#ifdef	INET
		if (netisr & SCHEDBIT(NETISR_IP)) {
			netisr &= ~NETISR_IP;
			ip_intr();
		}
#endif
#ifdef	NS
		if (netisr & SCHEDBIT(NETISR_NS)) {
			netisr &= ~NETISR_NS;
			nsintr();
		}
#endif
		if (netisr & SCHEDBIT(NETISR_RAW)) {
			netisr &= ~NETISR_RAW;
			rawintr();
		}
		splx(s);
	}
}
