/* Get Sendmail Alias from the Yellow Pages if possible.
 */

#ifndef lint
static  char rcsid[] = "$Header: /d2/3.7/src/bsd/usr.lib/sendmail/src/RCS/aliasfetch.c,v 1.1 89/03/27 15:02:44 root Exp $";
#endif

#include <stdio.h>
#include <dbm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

static char ypdomain[256];
static char yellowup;			/* are yellow pages up? */
static char checkyellow = 1;		/* see if YP are up */

static datum yp_ok = {"+", 2};		/* present in DB to enable YP */


/* try to use the Yellow Pages, and then fall back on the old fashioned
 *	new-database
 */
aliasfetch(lhs,rhs)
register datum *lhs;
register datum *rhs;
{
	*rhs = fetch(*lhs);		/* use YP only if no local value */
	if (rhs->dptr != NULL
	    || (lhs->dsize == 1		/* and it is not the special value */
		&& *lhs->dptr == '@'))
		return;

	if (checkyellow) {
		checkyellow = 0;

		getdomainname(ypdomain, sizeof(ypdomain));
		*rhs = fetch(yp_ok);
		yellowup = (rhs->dptr != NULL && rhs->dsize == yp_ok.dsize
			    && !strncmp(rhs->dptr, yp_ok.dptr, yp_ok.dsize)
			    && !yp_bind(ypdomain));
	}

	if (!yellowup			/* and if YP are healthy */
	    || yp_match(ypdomain, "mail.aliases", lhs->dptr, lhs->dsize,
		        &rhs->dptr, &rhs->dsize)) {
		rhs->dptr = NULL;
		rhs->dsize = 0;
	}
}


/* install the special YP aliases
 */
ypalias_init(af)
register FILE *af;
{
	datum key, content;
	char local_host[255];
	struct stat sb;
	char ans[11];

	key.dptr = "YP_LAST_MODIFIED";
	key.dsize = strlen(key.dptr);
	fstat(fileno(af), &sb);
	sprintf(ans, "%010d", sb.st_mtime);
	content.dptr = ans;
	content.dsize = strlen(ans);
	store(key,content);

	key.dptr = "YP_MASTER_NAME";
	key.dsize = strlen(key.dptr);
	gethostname(local_host, sizeof(local_host) - 1);
	content.dptr = local_host;
	content.dsize= strlen(local_host);
	store(key,content);

	checkyellow = 1;		/* see if we should use YP */
}
