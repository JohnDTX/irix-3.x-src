/*	@(#)lddef.h	2.1	*/
#ifndef LDLIST

struct ldlist {
	LDFILE		ld_item;
	struct ldlist	*ld_next;
};

#define	LDLIST	struct ldlist
#define	LDLSZ	sizeof(LDLIST)

#endif
