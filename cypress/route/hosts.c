/* 
 * hosts.c - Routines to find a hosts network number given its name.
 * 
 * Author:	Greg Smith
 * 		Dept. of Computer Sciences
 * 		Purdue University
 * 
 * Date:	Tue Jul 16 1985
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>	
#include <netdb.h>
#include <stdio.h>
#include <ctype.h>
#include "defs.h"
/*
 * GetNetNumber - invoked with a cypress node name as a parameter to
 * to obtain its Internet address.  It first checks to see if the cache of
 * hosts exists, if so it is searched.  If the name is not found or if the
 * cache is empty a call to getcyhostbyname is made.  If the entry is not
 * found an error exit is taken.
 * 
 */

char *GetNetNumber(sbNodeName)

char *sbNodeName;

{
    struct hostent *pHostent;
    struct hostent *getcyhostbyname();
    char   *inet_ntoa ();
    char   *MakeString ();
    char   *sbMsg[50];
    char   *sbInternetAddr;
    int    ind;
    extern struct Connect   ConnectTable[];
    extern struct host HostCache[];
    extern int  cHostCache;

    /* see if the cache exists */
    if (cHostCache > 0 && ((ind = SearchHostCache(sbNodeName)) >= 0))
        return(MakeString(HostCache[ind].sbNetNumber));
    else {
#ifdef BSD43
	if ((pHostent = gethostbyname (sbNodeName)) != NULL) {
	    struct netent *pnetent = getnetbyname(CYPRESSNET);
	    char **psbifs;	/* network interface numbers */
#ifdef sgi
	    register struct in_addr *padr;
#endif
	    if (pnetent == NULL) {
		sprintf(sbMsg, "net %s not in network database.\n", CYPRESSNET);
		return(NULL);
	    }
	    for (psbifs = pHostent->h_addr_list; *psbifs; psbifs++) {
#ifdef sgi
		padr = (struct in_addr *)*psbifs;
		if (inet_netof(*padr) == pnetent->n_net) break;
		printf("Comparing ");
		printaddr(inet_netof(*padr), 4, AF_INET);
#else
		if (inet_netof(*(struct in_addr *)*psbifs) == pnetent->n_net) break;
		printf("Comparing ");
		printaddr(inet_netof(*(struct in_addr *)*psbifs), 4, AF_INET);
#endif
		printf(" with ");
		fflush(stdout);
		printaddr(&(pnetent->n_net), 4, pnetent->n_addrtype);
	    }
	    if (!(*psbifs)) {
		sprintf (sbMsg, "host %s found in host tables, but none with net number 0x%x", sbNodeName, pnetent->n_net);
		
		FatalError (sbMsg);
		return (NULL);
	    }
#ifdef sgi
	    sbInternetAddr = inet_ntoa (*padr);
#else
	    sbInternetAddr = inet_ntoa (*((struct in_addr  *) *psbifs));
#endif
	    return (MakeString (sbInternetAddr));
	}
#else
	if ((pHostent = getcyhostbyname (sbNodeName)) != NULL) {
	    sbInternetAddr = inet_ntoa (*((struct in_addr  *) pHostent -> h_addr));
	    return (MakeString (sbInternetAddr));
	}
#endif	
	else {
	   sprintf (sbMsg, "host %s not found in host tables", sbNodeName);
	   FatalError (sbMsg);
	   return (NULL);
	}
     }
 }

/*
 * ============================================================
 * printaddr - print out the address 
 * ============================================================
 */
printaddr(sb, len, addrtype)
char *sb;
int len, addrtype;
{
    switch(addrtype) {
    case AF_INET: printf("\t%d.%d.%d.%d\n", sb[0] & 0xff, sb[1]& 0xff,
			sb[2]&0xff, sb[3]& 0xff);
		  break;
    default: printf("Unknown address family %d\n", addrtype);
    }
    return;
	
}
