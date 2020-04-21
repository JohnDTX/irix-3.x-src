#ifndef lint
/* @(#)umount.c	2.1 86/04/17 NFSSRC */ 
static char *sccsid = "@(#)umount.c 1.1 86/02/03 SMI"; /* from UCB 4.8 */
#endif

/*
 * umount
 */

#include <sys/param.h>
#include <sys/file.h>
#include <stdio.h>
#include <mntent.h>
#include <errno.h>
#include <sys/time.h>
#include <rpc/rpc.h>
#ifdef SVR3
#include <sys/fs/nfs.h>
#else
#include <nfs/nfs.h>
#endif
#include <rpcsvc/mount.h>
#include <sys/socket.h>
#include <netdb.h>
#ifdef sgi
# include <signal.h>
#endif

/*
 * This structure is used to build a list of mntent structures
 * in reverse order from /etc/mtab.
 */
struct mntlist {
	struct mntent *mntl_mnt;
	struct mntlist *mntl_next;
};

int	all = 0;
#if defined SVR3 || defined KILL
#define	KILLSLEEP	5
int	kill = 0;
#endif
int	verbose = 0;
int	host = 0;
int	type = 0;

char	*typestr;
char	*hoststr;

char	*xmalloc();
char	*index();
struct mntlist *mkmntlist();
struct mntent *mntdup();

int eachreply();

extern	int errno;

main(argc, argv)
	int argc;
	char **argv;
{
	char *options;

#ifdef sgi
	if (geteuid() != 0) {
		fprintf(stderr, "Must be root to use umount\n");
		exit(1);
	}
#endif

	argc--, argv++;
	sync();
	umask(0);
	while (argc && *argv[0] == '-') {
		options = &argv[0][1];
		while (*options) {
			switch (*options) {
			case 'a':
				all++;
				break;
			case 'h':
				all++;
				host++;
				hoststr = argv[1];
				argc--;
				argv++;
				break;
#ifdef KILLSLEEP
			case 'k':
				kill++;
				break;
#endif
			case 't':
				all++;
				type++;
				typestr = argv[1];
				argc--;
				argv++;
				break;
			case 'v':
				verbose++;
				break;
			default:
				fprintf(stderr, "umount: unknown option '%c'\n",
				    *options);
				usage();
			}
			options++;
		}
		argv++;
		argc--;
	}

	if (all && argc) {
		usage();
	}

	umountlist(argc, argv);
}

#ifdef sgi
unsigned char	interrupted = 0;

catch(sig)
	int sig;
{
	interrupted = 1;
	(void) signal(sig, catch);
}

#define	HANDLE(handler) { \
	(void) signal(SIGHUP, handler); \
	(void) signal(SIGINT, handler); \
	(void) signal(SIGTERM, handler); \
}
#endif

umountlist(argc, argv)
	int argc;
	char *argv[];
{
	int i, pid;
	int didit;
	struct mntent *mnt;
	struct mntlist *mntl, *mntcur;
	struct mntlist *mntrev = NULL;
	int tmpfd;
	char *colon;
	FILE *tmpmnt;
	char *tmpname = "/etc/umountXXXXXX";

#ifdef sgi
	HANDLE(catch);
#endif
	mktemp(tmpname);
	if ((tmpfd = open(tmpname, O_RDWR|O_CREAT|O_TRUNC, 0644)) < 0) {
		perror(tmpname);
		exit(1);
	}
	close(tmpfd);
	tmpmnt = setmntent(tmpname, "w");
	if (tmpmnt == NULL) {
		perror(tmpname);
		exit(1);
	}
	if (all) {
		if (!host &&
		    (!type || (type && strcmp(typestr, MNTTYPE_NFS) == 0))) {
			pid = fork();
			if (pid < 0)
				perror("umount: fork");
			if (pid == 0) {
				endmntent(tmpmnt);
				clnt_broadcast(MOUNTPROG,
				    MOUNTVERS, MOUNTPROC_UMNTALL,
				    xdr_void, NULL, xdr_void, NULL, eachreply);
				exit(0);
			}
		}
	}
	/*
	 * get a last first list of mounted stuff, reverse list and
	 * null out entries that get unmounted.
	 */
	for (mntl = mkmntlist(MOUNTED); mntl != NULL;
	    mntcur = mntl, mntl = mntl->mntl_next,
	    mntcur->mntl_next = mntrev, mntrev = mntcur) {
		mnt = mntl->mntl_mnt;
		if (strcmp(mnt->mnt_dir, "/") == 0) {
			continue;
		}
		if (strcmp(mnt->mnt_type, MNTTYPE_IGNORE) == 0) {
			continue;
		}
		if (all) {
			if (type && strcmp(typestr, mnt->mnt_type)) {
				continue;
			}
			if (host) {
				if (strcmp(MNTTYPE_NFS, mnt->mnt_type)) {
					continue;
				}
				colon = index(mnt->mnt_fsname, ':');
				if (colon) {
					*colon = '\0';
					if (strcmp(hoststr, mnt->mnt_fsname)) {
						*colon = ':';
						continue;
					}
					*colon = ':';
				} else {
					continue;
				}
			}
			if (umountmnt(mnt)) {
				mntl->mntl_mnt = NULL;
			}
			continue;
		}

		for (i=0; i<argc; i++) {
			if ((strcmp(mnt->mnt_dir, argv[i]) == 0) ||
			    (strcmp(mnt->mnt_fsname, argv[i]) == 0) ) {
				if (umountmnt(mnt)) {
					mntl->mntl_mnt = NULL;
				}
				*argv[i] = '\0';
				break;
			}
		}
	}

	for (i=0; i<argc; i++) {
		if (*argv[i]) {
			struct mntent tmpmnt;

			tmpmnt.mnt_fsname = NULL;
			tmpmnt.mnt_dir = argv[i];
#ifdef sgi
			/* XXX type doesn't matter */
			tmpmnt.mnt_type = MNTTYPE_EFS;
			(void) umountmnt(&tmpmnt);
#else
			tmpmnt.mnt_type = MNTTYPE_42;
			umountmnt(&tmpmnt);
#endif
		}
	}

	/*
	 * Build new temporary mtab by walking mnt list
	 */
	for (; mntcur != NULL; mntcur = mntcur->mntl_next) {
		if (mntcur->mntl_mnt) {
			addmntent(tmpmnt, mntcur->mntl_mnt);
		}
	}
	endmntent(tmpmnt);

	/*
	 * Move tmp mtab to real mtab
	 */
	if (rename(tmpname, MOUNTED) < 0) {
		perror(MOUNTED);
		exit(1);
	}
}

#ifdef sgi
#include <setjmp.h>
jmp_buf	context;

jump(sig)
	int sig;
{
	catch(sig);
	longjmp(context, 1);
}
#endif

umountmnt(mnt)
	struct mntent *mnt;
{
#ifdef sgi
	if (interrupted)
		return 0;
#ifdef KILLSLEEP
	if (kill) {
		char cmd[MNTMAXSTR];

		if (verbose) {
			sprintf(cmd, "/etc/fuser -k %s", mnt->mnt_fsname);
		} else {
			sprintf(cmd, "2>&1 /etc/fuser -k %s > /dev/null",
				mnt->mnt_fsname);
		}
		(void) system(cmd);
		sleep(KILLSLEEP);
	}
#endif
	if (umount(mnt->mnt_dir) < 0) {
#else
	if (unmount(mnt->mnt_dir) < 0) {
#endif
		if (errno != EINVAL) {
			perror(mnt->mnt_dir);
			return(0);
		}
		fprintf(stderr, "%s not mounted\n", mnt->mnt_dir);
		return(1);
	} else {
		if (strcmp(mnt->mnt_type, MNTTYPE_NFS) == 0) {
#ifdef sgi
			if (!setjmp(context)) {
				HANDLE(jump);
				(void) rpctoserver(mnt);
				HANDLE(catch);
			}
#else
			rpctoserver(mnt);
#endif
		}
		if (verbose) {
			fprintf(stderr, "%s: Unmounted\n", mnt->mnt_dir);
		}
		return(1);
	}
}

usage()
{
#ifdef KILLSLEEP
	fprintf(stderr, "usage: umount -a[kv] [-t <type>] [-h <host>]\n");
	fprintf(stderr, "       umount [-kv] <path> | <dev> ...\n");
#else
	fprintf(stderr, "usage: umount -a[v] [-t <type>] [-h <host>]\n");
	fprintf(stderr, "       umount [-v] <path> | <dev> ...\n");
#endif
	exit(1);
}

rpctoserver(mnt)
	struct mntent *mnt;
{
	char *p;
	struct sockaddr_in sin;
	struct hostent *hp;
	int s;
	struct timeval timeout;
	CLIENT *client;
	enum clnt_stat rpc_stat;
		
	if ((p = index(mnt->mnt_fsname, ':')) == NULL)
#ifdef sgi
		return(1);
#else
		return;
#endif
	*p++ = 0;
	if ((hp = gethostbyname(mnt->mnt_fsname)) == NULL) {
		fprintf(stderr, "%s not in hosts database\n", mnt->mnt_fsname);
#ifdef sgi
		return(0);
#else
		return(1);
#endif
	}
	bzero(&sin, sizeof(sin));
	bcopy(hp->h_addr, (char *) & sin.sin_addr, hp->h_length);
	sin.sin_family = AF_INET;
	s = RPC_ANYSOCK;
	timeout.tv_usec = 0;
	timeout.tv_sec = 10;
	if ((client = clntudp_create(&sin, MOUNTPROG, MOUNTVERS,
	    timeout, &s)) == NULL) {
		clnt_pcreateerror("Warning: umount:");
#ifdef sgi
		return(0);
#else
		return(1);
#endif
	}
	if (! bindresvport(s)) {
		fprintf(stderr,"Warning: umount: cannot do local bind.\n");
	}
	client->cl_auth = authunix_create_default();
	timeout.tv_usec = 0;
	timeout.tv_sec = 25;
	rpc_stat = clnt_call(client, MOUNTPROC_UMNT, xdr_path, &p,
	    xdr_void, NULL, timeout);
	if (rpc_stat != RPC_SUCCESS) {
		clnt_perror(client, "Warning: umount:");
#ifdef sgi
		return(0);
#else
		return(1);
#endif
	}
#ifdef sgi
	return(1);
#endif
}

eachreply(resultsp, addrp)
	char *resultsp;
	struct sockaddr_in *addrp;
{
	int done = 1;

	return (done);
}

char *
xmalloc(size)
	int size;
{
	char *ret;
	
	if ((ret = (char *)malloc(size)) == NULL) {
		fprintf(stderr, "umount: ran out of memory!\n");
		exit(1);
	}
	return (ret);
}

struct mntent *
mntdup(mnt)
	struct mntent *mnt;
{
	struct mntent *new;

	new = (struct mntent *)xmalloc(sizeof(*new));

	new->mnt_fsname = (char *)xmalloc(strlen(mnt->mnt_fsname) + 1);
	strcpy(new->mnt_fsname, mnt->mnt_fsname);

	new->mnt_dir = (char *)xmalloc(strlen(mnt->mnt_dir) + 1);
	strcpy(new->mnt_dir, mnt->mnt_dir);

	new->mnt_type = (char *)xmalloc(strlen(mnt->mnt_type) + 1);
	strcpy(new->mnt_type, mnt->mnt_type);

	new->mnt_opts = (char *)xmalloc(strlen(mnt->mnt_opts) + 1);
	strcpy(new->mnt_opts, mnt->mnt_opts);

	new->mnt_freq = mnt->mnt_freq;
	new->mnt_passno = mnt->mnt_passno;

	return (new);
}

struct mntlist *
mkmntlist(file)
	char *file;
{
	FILE *mounted;
	struct mntlist *mntl;
	struct mntlist *mntst = NULL;
	struct mntent *mnt;

	mounted = setmntent(MOUNTED, "r");
	if (mounted == NULL) {
		perror(MOUNTED);
		exit(1);
	}
	while ((mnt = getmntent(mounted)) != NULL) {
		mntl = (struct mntlist *)xmalloc(sizeof(*mntl));
		mntl->mntl_mnt = mntdup(mnt);
		mntl->mntl_next = mntst;
		mntst = mntl;
	}
	return(mntst);
}

bindresvport(sd)
	int sd;
{
	u_short port;
	struct sockaddr_in sin;
	int err = -1;

#	define MAX_PRIV (IPPORT_RESERVED-1)
#	define MIN_PRIV (IPPORT_RESERVED/2)

	get_myaddress(&sin);
	sin.sin_family = AF_INET;
	for (port = MAX_PRIV; err && port >= MIN_PRIV; port--) {
		sin.sin_port = htons(port);
		err = bind(sd,&sin,sizeof(sin));
	}
	return (err == 0);
}
