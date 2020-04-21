/* 
 * cache.c - routines to manipulate the cache of cypress hosts and networks.
 * 
 * Author:      Greg Smith
 *              Dept. of Computer Sciences
 *              Purdue University
 * 
 * Date:        Sat Aug 10 1985
 */

#include <sys/types.h>
#include <sys/stat.h>
#include "defs.h"
#include <stdio.h>

UpdateCache()

{
    extern struct Connect ConnectTable[];
    extern int cConnectCurr;
    FILE *fd;
    int i;

    /* go through the connection table writing host names */
    /* and their corresponding internet addresses         */
    if ((fd = fopen(HOSTCACHE, "w")) != NULL) {

        for (i = 0; i < cConnectCurr; i++)
	     fprintf(fd, "%s %s\n", ConnectTable[i].sbNodeName, ConnectTable[i].sbNetNumber);

        fclose(fd);
    }
    else FatalError("Unable to update host cache");
 }

/*
 * BuildCache - parses the host cache and builds an in memory structure
 * sorted by host name.
 * 
 */

BuildCache()

{
	extern struct host HostCache[]; /* array of cypress host names and  */
					/* their internet addresses         */
	FILE *fdHostCache;              /* file of cypress host names and   */
					/* their internet addresses         */
	extern int cHostCache;          /* number of entries in host cache  */
	char str1[50];                  /* buffer to read name into         */
	char str2[50];                  /* buffer to read network addr into */
	extern char *MakeString();      /* copy a string                    */
	struct stat hosts;		/* pointer to file status structure */
					/* for /etc/hosts		    */
	struct stat hostcache;		/* pointer to same for host cache   */

	/* check the modification date of /etc/hosts against */
	/* that of the cache, if later don't create the cache*/
	if ((stat("/etc/hosts", &hosts) == 0) && (stat(HOSTCACHE, &hostcache) == 0))
	    if (hosts.st_mtime < hostcache.st_mtime) 
		/* build the cache of host names if possible */
		if ((fdHostCache = fopen(HOSTCACHE, "r")) != NULL) {
                    while (fscanf(fdHostCache, "%s %s\n", str1, str2) != EOF) {
			HostCache[cHostCache].sbNodeName = MakeString(str1);
			HostCache[cHostCache++].sbNetNumber = MakeString(str2);
		    }

		    fclose(fdHostCache);
		}
	    else cHostCache = 0;
}

/* 
 * SearchHostCache(sbNdName) -- Searches the cache of cypress hosts and
 * for a name and returns its index in the table if found or -1 otherwise.
 * 
 */

SearchHostCache(sbNdName)

char *sbNdName;

{
    int     low,
            high,
            mid,
            cond;
    extern struct host HostCache[];
    extern int cHostCache;

    low = 0;
    high = cHostCache - 1;

    while (low <= high) {
	mid = (low + high) / 2;
	if ((cond = strcmp (sbNdName, HostCache[mid].sbNodeName)) < 0)
	    high = mid - 1;
	else
	    if (cond > 0)
		low = mid + 1;
	    else
		return (mid);
    }
    return (-1);
}
