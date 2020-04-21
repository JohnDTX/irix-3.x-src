#ifndef lint
/* @(#)ypxdr.c	2.1 86/04/14 NFSSRC */
static  char sccsid[] = "@(#)ypxdr.c 1.1 86/02/03 Copyr 1985 Sun Micro";
#endif

/*
 * This contains xdr routines used by the yellowpages rpc interface.
 */

#define NULL 0
#include <rpc/rpc.h>
#include "yp_prot.h"
#include "ypclnt.h"

typedef struct xdr_discrim XDR_DISCRIM;
bool xdr_datum();
bool xdr_ypdomain_wrap_string();
bool xdr_ypmap_wrap_string();
bool xdr_ypreq_key();
bool xdr_ypreq_nokey();
bool xdr_ypresp_val();
bool xdr_ypresp_key_val();
bool xdr_ypbind_resp ();
bool xdr_yp_inaddr();
bool xdr_yp_binding();
bool xdr_ypmap_parms();
bool xdr_ypowner_wrap_string();
bool xdr_ypmaplist();
bool xdr_ypmaplist_wrap_string();
bool xdr_ypref();

extern char *malloc();

/*
 * Serializes/deserializes a dbm datum data structure.
 */
bool
xdr_datum(xdrs, pdatum)
	XDR * xdrs;
	datum * pdatum;

{
	return (xdr_bytes(xdrs, &(pdatum->dptr), &(pdatum->dsize),
	    YPMAXRECORD));
}


/*
 * Serializes/deserializes a domain name string.  This is a "wrapper" for
 * xdr_string which knows about the maximum domain name size.  
 */
bool
xdr_ypdomain_wrap_string(xdrs, ppstring)
	XDR * xdrs;
	char **ppstring;
{
	return (xdr_string(xdrs, ppstring, YPMAXDOMAIN) );
}

/*
 * Serializes/deserializes a map name string.  This is a "wrapper" for
 * xdr_string which knows about the maximum map name size.  
 */
bool
xdr_ypmap_wrap_string(xdrs, ppstring)
	XDR * xdrs;
	char **ppstring;
{
	return (xdr_string(xdrs, ppstring, YPMAXMAP) );
}

/*
 * Serializes/deserializes a ypreq_key structure.
 */
bool
xdr_ypreq_key(xdrs, ps)
	XDR *xdrs;
	struct ypreq_key *ps;

{
	return (xdr_ypdomain_wrap_string(xdrs, &ps->domain) &&
	    xdr_ypmap_wrap_string(xdrs, &ps->map) &&
	    xdr_datum(xdrs, &ps->keydat) );
}

/*
 * Serializes/deserializes a ypreq_nokey structure.
 */
bool
xdr_ypreq_nokey(xdrs, ps)
	XDR * xdrs;
	struct ypreq_nokey *ps;
{
	return (xdr_ypdomain_wrap_string(xdrs, &ps->domain) &&
	    xdr_ypmap_wrap_string(xdrs, &ps->map) );
}

/*
 * Serializes/deserializes a ypreq_xfr structure.
 */
bool
xdr_ypreq_xfr(xdrs, ps)
	XDR * xdrs;
	struct ypreq_xfr *ps;
{
	return (xdr_ypmap_parms(xdrs, &ps->map_parms) &&
	    xdr_u_long(xdrs, &ps->transid) &&
	    xdr_u_long(xdrs, &ps->proto) &&
	    xdr_u_short(xdrs, &ps->port) );
}

/*
 * Serializes/deserializes a ypresp_val structure.
 */

bool
xdr_ypresp_val(xdrs, ps)
	XDR * xdrs;
	struct ypresp_val *ps;
{
	return (xdr_u_long(xdrs, &ps->status) &&
	    xdr_datum(xdrs, &ps->valdat) );
}

/*
 * Serializes/deserializes a ypresp_key_val structure.
 */
bool
xdr_ypresp_key_val(xdrs, ps)
	XDR * xdrs;
	struct ypresp_key_val *ps;
{
	return (xdr_u_long(xdrs, &ps->status) &&
	    xdr_datum(xdrs, &ps->valdat) &&
	    xdr_datum(xdrs, &ps->keydat) );
}

/*
 * Serializes/deserializes a ypresp_master structure.
 */
bool
xdr_ypresp_master(xdrs, ps)
	XDR * xdrs;
	struct ypresp_master *ps;
{
	return (xdr_u_long(xdrs, &ps->status) &&
	     xdr_ypowner_wrap_string(xdrs, &ps->master) );
}

/*
 * Serializes/deserializes a ypresp_order structure.
 */
bool
xdr_ypresp_order(xdrs, ps)
	XDR * xdrs;
	struct ypresp_order *ps;
{
	return (xdr_u_long(xdrs, &ps->status) &&
	     xdr_u_long(xdrs, &ps->ordernum) );
}

/*
 * Serializes/deserializes a stream of struct ypresp_key_val's.  This is used
 * only by the client side of the transaction.
 */
bool
xdr_ypall(xdrs, callback)
	XDR * xdrs;
	struct ypall_callback *callback;
{
	bool more;
	struct ypresp_key_val kv;
	bool s;
	char keybuf[YPMAXRECORD];
	char valbuf[YPMAXRECORD];

	if (xdrs->x_op == XDR_ENCODE)
		return(FALSE);

	if (xdrs->x_op == XDR_FREE)
		return(TRUE);

	kv.keydat.dptr = keybuf;
	kv.valdat.dptr = valbuf;
	kv.keydat.dsize = YPMAXRECORD;
	kv.valdat.dsize = YPMAXRECORD;
	
	for (;;) {
		if (! xdr_bool(xdrs, &more) )
			return (FALSE);
			
		if (! more)
			return (TRUE);

		s = xdr_ypresp_key_val(xdrs, &kv);
		
		if (s) {
			s = (*callback->foreach)(kv.status, kv.keydat.dptr,
			    kv.keydat.dsize, kv.valdat.dptr, kv.valdat.dsize,
			    callback->data);
			
			if (s)
				return (TRUE);
		} else {
			return (FALSE);
		}
	}
}

/*
 * This is like xdr_ypmap_wrap_string except that it serializes/deserializes
 * an array, instead of a pointer, so xdr_reference can work on the structure
 * containing the char array itself.
 */
bool
xdr_ypmaplist_wrap_string(xdrs, pstring)
	XDR * xdrs;
	char *pstring;
{
	char *s;

	s = pstring;
	return (xdr_string(xdrs, &s, YPMAXMAP) );
}

/*
 * Serializes/deserializes a ypmaplist.
 */
bool
xdr_ypmaplist(xdrs, lst)
	XDR *xdrs;
	struct ypmaplist **lst;
{
	bool more_elements;
	int freeing = (xdrs->x_op == XDR_FREE);
	struct ypmaplist **next;

	while (TRUE) {
		more_elements = (*lst != (struct ypmaplist *) NULL);
		
		if (! xdr_bool(xdrs, &more_elements))
			return (FALSE);
			
		if (! more_elements)
			return (TRUE);  /* All done */
			
		if (freeing)
			next = &((*lst)->ypml_next);

		if (! xdr_reference(xdrs, lst, (u_int) sizeof(struct ypmaplist),
		    xdr_ypmaplist_wrap_string))
			return (FALSE);
			
		lst = (freeing) ? next : &((*lst)->ypml_next);
	}
}

/*
 * Serializes/deserializes a ypresp_maplist.
 */
bool
xdr_ypresp_maplist(xdrs, ps)
	XDR * xdrs;
	struct ypresp_maplist *ps;

{
	return (xdr_u_long(xdrs, &ps->status) &&
	   xdr_ypmaplist(xdrs, &ps->list) );
}

/*
 * Serializes/deserializes an in_addr struct.
 * 
 * Note:  There is a data coupling between the "definition" of a struct
 * in_addr implicit in this xdr routine, and the true data definition in
 * <netinet/in.h>.  
 * (Not for sgi there ain't!)
 */
bool
xdr_yp_inaddr(xdrs, ps)
	XDR * xdrs;
	struct in_addr *ps;

{
#ifdef sgi
	return (xdr_opaque(xdrs, (caddr_t) ps, sizeof *ps));
#else
	return (xdr_u_long(xdrs, &ps->s_addr));
#endif
}

/*
 * Serializes/deserializes a ypbind_binding struct.
 */
bool
xdr_yp_binding(xdrs, ps)
	XDR * xdrs;
	struct ypbind_binding *ps;

{
#ifdef sgi
	return (xdr_yp_inaddr(xdrs, &ps->ypbind_binding_addr)
	    && xdr_opaque(xdrs, &ps->ypbind_binding_port,
		sizeof ps->ypbind_binding_port));
#else
	return (xdr_yp_inaddr(xdrs, &ps->ypbind_binding_addr) &&
            xdr_u_short(xdrs, &ps->ypbind_binding_port));
#endif
}

/*
 * xdr discriminant/xdr_routine vector for yp binder responses
 */
XDR_DISCRIM ypbind_resp_arms[] = {
	{(int) YPBIND_SUCC_VAL, (xdrproc_t) xdr_yp_binding},
	{(int) YPBIND_FAIL_VAL, (xdrproc_t) xdr_u_long},
	{__dontcare__, (xdrproc_t) NULL}
};

/*
 * Serializes/deserializes a ypbind_resp structure.
 */
bool
xdr_ypbind_resp(xdrs, ps)
	XDR * xdrs;
	struct ypbind_resp *ps;

{
	return (xdr_union(xdrs, &ps->ypbind_status, &ps->ypbind_respbody,
	    ypbind_resp_arms, NULL) );
}

/*
 * Serializes/deserializes a peer server's node name
 */
bool
xdr_ypowner_wrap_string(xdrs, ppstring)
	XDR * xdrs;
	char **ppstring;

{
	return (xdr_string(xdrs, ppstring, YPMAXPEER) );
}

/*
 * Serializes/deserializes a ypmap_parms structure.
 */
bool
xdr_ypmap_parms(xdrs, ps)
	XDR *xdrs;
	struct ypmap_parms *ps;

{
	return (xdr_ypdomain_wrap_string(xdrs, &ps->domain) &&
	    xdr_ypmap_wrap_string(xdrs, &ps->map) &&
	    xdr_u_long(xdrs, &ps->ordernum) &&
	    xdr_ypowner_wrap_string(xdrs, &ps->owner) );
}

/*
 * Serializes/deserializes a ypbind_setdom structure.
 */
bool
xdr_ypbind_setdom(xdrs, ps)
	XDR *xdrs;
	struct ypbind_setdom *ps;
{
	char *domain = ps->ypsetdom_domain;
	
	return (xdr_ypdomain_wrap_string(xdrs, &domain) &&
	    xdr_yp_binding(xdrs, &ps->ypsetdom_binding) &&
	    xdr_u_short(xdrs, &ps->ypsetdom_vers));
}

/*
 * Serializes/deserializes a yppushresp_xfr structure.
 */
bool
xdr_yppushresp_xfr(xdrs, ps)
	XDR *xdrs;
	struct yppushresp_xfr *ps;
{
	return (xdr_u_long(xdrs, &ps->transid) &&
	    xdr_u_long(xdrs, &ps->status));
}


