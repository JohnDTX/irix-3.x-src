/*      mount.h     1.1     86/02/05     */
/* @(#)mount.h	2.1 86/04/14 NFSSRC */

/*
 * Copyright (c) 1984 Sun Microsystems, Inc.
 */

#define MOUNTPROG 100005
#define MOUNTPROC_MNT 1
#define MOUNTPROC_DUMP 2
#define MOUNTPROC_UMNT 3
#define MOUNTPROC_UMNTALL 4
#define MOUNTPROC_EXPORT 5
#define MOUNTPROC_EXPORTALL 6
#define MOUNTVERS_ORIG 1
#define MOUNTVERS 1

#ifndef svc_getcaller
#define svc_getcaller(x) (&(x)->xp_raddr)
#endif

bool_t xdr_path();
bool_t xdr_fhandle();
bool_t xdr_fhstatus();
bool_t xdr_mountlist();
bool_t xdr_exports();

struct mountlist {		/* what is mounted */
	char *ml_name;
	char *ml_path;
	struct mountlist *ml_nxt;
};

struct fhstatus {
	int fhs_status;
	fhandle_t fhs_fh;
};

/*
 * List of exported directories
 * An export entry with ex_groups
 * NULL indicates an entry which is exported to the world.
 */
struct exports {
	dev_t		  ex_dev;	/* dev of directory */
	char		 *ex_name;	/* name of directory */
	struct groups	 *ex_groups;	/* groups allowed to mount this entry */
	struct exports	 *ex_next;
	short		  ex_rootmap;	/* id to map root requests to */
	short		  ex_flags;	/* bits to mask off file mode */
};

struct groups {
	char		*g_name;
	struct groups	*g_next;
};
