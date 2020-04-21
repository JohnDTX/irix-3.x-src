#ifndef lint
/* @(#)yp_match.c	2.1 86/04/14 NFSSRC */
static  char sccsid[] = "@(#)yp_match.c 1.1 86/02/03 Copyr 1985 Sun Micro";
#endif

#define NULL 0
#include <sys/time.h>
#include <rpc/rpc.h>
#include "yp_prot.h"
#include "ypv1_prot.h"
#include "ypclnt.h"

extern struct timeval _ypserv_timeout;
extern int _yp_dobind();
extern unsigned int _ypsleeptime;
extern char *malloc();
static int v2domatch(), v1domatch();

/*
 * Requests the yp server associated with a given domain to attempt to match
 * the passed key datum in the named map, and to return the associated value
 * datum. This part does parameter checking, and implements the "infinite"
 * (until success) sleep loop.
 */
int
yp_match (domain, map, key, keylen, val, vallen)
	char *domain;
	char *map;
	char *key;
	int  keylen;
	char **val;		/* returns value array */
	int  *vallen;		/* returns bytes in val */
{
	int domlen;
	int maplen;
	int reason;
	struct dom_binding *pdomb;
	int (*dofun)();

	if ( (map == NULL) || (domain == NULL) ) {
		return(YPERR_BADARGS);
	}
	
	domlen = strlen(domain);
	maplen = strlen(map);
	
	if ( (domlen == 0) || (domlen > YPMAXDOMAIN) ||
	    (maplen == 0) || (maplen > YPMAXMAP) ||
	    (key == NULL) || (keylen == 0) ) {
		return(YPERR_BADARGS);
	}

	for (;;) {
		
		if (reason = _yp_dobind(domain, &pdomb) ) {
			return(reason);
		}

		dofun = (pdomb->dom_vers == YPVERS) ? v2domatch : v1domatch;

		reason = (*dofun)(domain, map, key, keylen, pdomb,
		    _ypserv_timeout, val, vallen);

		if (reason == YPERR_RPC) {
			yp_unbind(domain);
			(void) sleep(_ypsleeptime);
		} else {
			break;
		}
	}
	
	return(reason);

}

/*
 * This talks v2 protocol to ypserv
 */
static int
v2domatch (domain, map, key, keylen, pdomb, timeout, val, vallen)
	char *domain;
	char *map;
	char *key;
	int  keylen;
	struct dom_binding *pdomb;
	struct timeval timeout;
	char **val;		/* return: value array */
	int  *vallen;		/* return: bytes in val */
{
	struct ypreq_key req;
	struct ypresp_val resp;
	unsigned int retval = 0;

	req.domain = domain;
	req.map = map;
	req.keydat.dptr = key;
	req.keydat.dsize = keylen;
	
	resp.valdat.dptr = NULL;
	resp.valdat.dsize = 0;

	/*
	 * Do the match request.  If the rpc call failed, return with status
	 * from this point.
	 */
	
	if(clnt_call(pdomb->dom_client,
	    YPPROC_MATCH, xdr_ypreq_key, &req, xdr_ypresp_val, &resp,
	    timeout) != RPC_SUCCESS) {
		return(YPERR_RPC);
	}

	/* See if the request succeeded */
	
	if (resp.status != YP_TRUE) {
		retval = ypprot_err((unsigned) resp.status);
	}

	/* Get some memory which the user can get rid of as he likes */

	if (!retval && ((*val = malloc((unsigned)
	    resp.valdat.dsize + 2)) == NULL)) {
		retval = YPERR_RESRC;
	}

	/* Copy the returned value byte string into the new memory */

	if (!retval) {
		*vallen = resp.valdat.dsize;
		bcopy(resp.valdat.dptr, *val, resp.valdat.dsize);
		(*val)[resp.valdat.dsize] = '\n';
		(*val)[resp.valdat.dsize + 1] = '\0';
	}

	CLNT_FREERES(pdomb->dom_client, xdr_ypresp_val, &resp);
	return(retval);

}

/*
 * This talks v1 protocol to ypserv
 */
static int
v1domatch (domain, map, key, keylen, pdomb, timeout, val, vallen)
	char *domain;
	char *map;
	char *key;
	int  keylen;
	struct dom_binding *pdomb;
	struct timeval timeout;
	char **val;		/* return: value array */
	int  *vallen;		/* return: bytes in val */
{
	struct yprequest req;
	struct ypresponse resp;
	unsigned int retval = 0;

	req.yp_reqtype = YPMATCH_REQTYPE;
	req.ypmatch_req_domain = domain;
	req.ypmatch_req_map = map;
	req.ypmatch_req_keyptr = key;
	req.ypmatch_req_keysize = keylen;
	
	resp.ypmatch_resp_valptr = NULL;
	resp.ypmatch_resp_valsize = 0;

	/*
	 * Do the match request.  If the rpc call failed, return with status
	 * from this point.
	 */
	
	if(clnt_call(pdomb->dom_client,
	    YPOLDPROC_MATCH, _xdr_yprequest, &req, _xdr_ypresponse, &resp,
	    timeout) != RPC_SUCCESS) {
		return(YPERR_RPC);
	}

	/* See if the request succeeded */
	
	if (resp.ypmatch_resp_status != YP_TRUE) {
		retval = ypprot_err((unsigned) resp.ypmatch_resp_status);
	}

	/* Get some memory which the user can get rid of as he likes */

	if (!retval && ((*val = malloc((unsigned)
	    resp.ypmatch_resp_valsize + 2)) == NULL)) {
		retval = YPERR_RESRC;
	}

	/* Copy the returned value byte string into the new memory */

	if (!retval) {
		*vallen = resp.ypmatch_resp_valsize;
		bcopy(resp.ypmatch_resp_valptr, *val,
		    resp.ypmatch_resp_valsize);
		(*val)[resp.ypmatch_resp_valsize] = '\n';
		(*val)[resp.ypmatch_resp_valsize + 1] = '\0';
	}

	CLNT_FREERES(pdomb->dom_client, _xdr_ypresponse, &resp);
	return(retval);

}
