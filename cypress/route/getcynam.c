/*
 * getcyhostbyname.c - modification of gethostbyname for cypress network
 *    so the match of a name to an /etc/hosts entry also makes sure that
 *    the entry is on the cypress network.
 *
 * author:  Gregory H. Smith
 *
 * date - 3-20-86
 *
 */

#include "defs.h"
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#ifndef BSD43
struct hostent *
getcyhostbyname(name)
	register char *name;
{
	register struct hostent *p;
	struct netent *pnetent;
	struct in_addr sin_cy1, sin_cy2;
	register char **cp;

	sethostent(0);
	
	if ((pnetent=getnetbyname(CYPRESSNET)) == NULL) {
	   printf("getcyhostbyname: fatal error getnetbyname(cypress).\n");
	   exit(1);
	}

	sin_cy1 = inet_makeaddr(pnetent->n_net, INADDR_ANY);

	while (p = gethostent()) {
	   if (strcmpfold(p->h_name, name) == 0) {
	      sin_cy2 = *((struct in_addr *) p->h_addr);
	      if (inet_netof(sin_cy1) == inet_netof(sin_cy2))
		 goto found;
	   }
	   
	   for (cp = p->h_aliases; *cp != 0; cp++)
	      if (strcmpfold(*cp, name) == 0) {
		 sin_cy2 = *((struct in_addr *) p->h_addr);
		 if (inet_netof(sin_cy1) == inet_netof(sin_cy2))
		    goto found;
	      }
	}
found:
	endhostent();
	return (p);
}
#endif BSD43
