#ifndef lint
/* @(#)rpc.mountd.c	2.1 86/04/17 NFSSRC */ 
static char sccsid[] = "@(#)rpc.mountd.c 1.1 86/02/05 Copyr 1985 Sun Micro";
#endif

/*
 * Copyright (c) 1985 Sun Microsystems, Inc.
 */

/* NFS server */

#include <sys/param.h>
#ifndef sgi
#include <ufs/fs.h>
#endif
#include <rpc/rpc.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>
#include <stdio.h>
#include <signal.h>
#ifndef sgi
#include <sys/wait.h>
#endif
#include <sys/ioctl.h>
#include <sys/errno.h>
#ifdef SVR3
#include <sys/sysmacros.h>
#include <sys/fs/com_inode.h>	/* XXX root i-number */
#define	ROOTINO	COM_ROOTINO
#include <sys/fs/nfs.h>
#else
#include <nfs/nfs.h>
#endif
#include <rpcsvc/mount.h>
#include <netdb.h>
#if defined sgi && !defined DEBUG
# include <syslog.h>
# define perror(string)	syslog(LOG_DEBUG, "%s: %m", string)
# define fprintf	syslog
# undef stdout
# define stdout		LOG_DEBUG
#endif

#define	EXPORTS	"/etc/exports"
#define RMTAB	"/etc/rmtab"
#define	MAXLINE	2048

extern int errno;

int mnt();
char *exmalloc();
int catch();
struct groups  *newgroup();
struct exports *newexport();

static struct mountlist *mountlist;
char myname[256];
char mydomain[256];
struct sockaddr_in myaddr;
char *exportfile = EXPORTS;
struct exports *exports = NULL;
int nfs_portmon = 0;
int ipaddr_check = 0;

main(argc, argv)
char	*argv[];
{
	struct sockaddr_in addr;
	int len = sizeof(struct sockaddr_in);
	SVCXPRT *transp;

#ifdef DEBUG
	{
		int s;
		struct sockaddr_in addr;
		int len = sizeof(struct sockaddr_in);

		if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
			perror("inet: socket");
			return - 1;
		}
		if (bind(s, &addr, sizeof(addr)) < 0) {
			perror("bind");
			return - 1;
		}
		if (getsockname(s, &addr, &len) != 0) {
			perror("inet: getsockname");
			(void)close(s);
			return - 1;
		}
		pmap_unset(MOUNTPROG, MOUNTVERS);
		pmap_set(MOUNTPROG, MOUNTVERS, IPPROTO_UDP,
		    ntohs(addr.sin_port));
		if (dup2(s, 0) < 0) {
			perror("dup2");
			exit(1);
		}
	}
#else
#ifdef sgi
	openlog("mountd", LOG_PID|LOG_NOWAIT, LOG_DAEMON);
#endif
#endif	

	if (getsockname(0, &addr, &len) != 0) {
		perror("rstat: getsockname");
		exit(1);
	}
	if ((transp = svcudp_create(0)) == NULL) {
		fprintf(stdout, "couldn't create udp transport\n");
		exit(1);
	}
	if (!svc_register(transp, MOUNTPROG, MOUNTVERS, mnt, 0)) {
		fprintf(stdout, "couldn't register MOUNTPROG");
		exit(1);
	}

	/*
	 * Initalize the world
	 */
	gethostname(myname, sizeof(myname));
	getdomainname(mydomain, sizeof(mydomain));
	get_myaddress(&myaddr);
	readfromfile();
	set_exports();

	/*
	 * Start serving
	 */
	svc_run();
	fprintf(stdout, "Error: svc_run shouldn't have returned\n");
	abort();
}

/*
 * Server procedure switch routine
 */
mnt(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
#if defined sgi && !defined DEBUG
#define	return	exit(0)	/* XXX inetd */
#endif

	switch(rqstp->rq_proc) {
		case NULLPROC:
			if (!svc_sendreply(transp, xdr_void, 0)) {
			    	fprintf(stdout,
				     "couldn't reply to rpc call\n");
				abort();
			}
			return;
		case MOUNTPROC_MNT:
#ifdef DEBUG
			fprintf(stdout, "about to do a mount\n");
#endif
			if (imposter(rqstp,transp)) {
				svcerr_weakauth(transp);
				return;
			}
			set_exports();
			mount(rqstp, transp);
			return;
		case MOUNTPROC_DUMP:
#ifdef DEBUG
			fprintf(stdout, "about to do a dump\n");
#endif
			if (!svc_sendreply(transp,xdr_mountlist,&mountlist)) {
			    	fprintf(stdout,
				    "couldn't reply to rpc call\n");
				abort();
			}
			return;
		case MOUNTPROC_UMNT:
#ifdef DEBUG
			fprintf(stdout, "about to do an unmount\n");
#endif
			if (imposter(rqstp,transp)) {
				svcerr_weakauth(transp);
				return;
			}
			umount(rqstp, transp);
			return;
		case MOUNTPROC_UMNTALL:
#ifdef DEBUG
			fprintf(stdout, "about to do an unmountall\n");
#endif
			if (imposter(rqstp,transp)) {
				svcerr_weakauth(transp);
				return;
			}
			umountall(rqstp, transp);
			return;
		case MOUNTPROC_EXPORT:
		case MOUNTPROC_EXPORTALL:
#ifdef DEBUG
			fprintf(stdout, "about to do a export\n");
#endif
			set_exports();
			export(rqstp, transp);
			return;
		default:
			svcerr_noproc(transp);
			return;
	}
#if defined sgi && !defined DEBUG
#undef return
#endif
}

imposter(rqstp,transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	struct in_addr claim;
	struct sockaddr_in actual;
	struct hostent *hp;
	char *machine;
	

	if (rqstp->rq_cred.oa_flavor != AUTH_UNIX) {
		svcerr_weakauth(transp);
		return(1);
	}
	actual = *svc_getcaller(transp);
	if (nfs_portmon) {
		if (ntohs(actual.sin_port) >= IPPORT_RESERVED) {
			return(1);
		}
	}
	if (ipaddr_check) {
		/* 
		 * check for imposters (unfortunately, this may not
		 * work if the request comes from a gateway)
	 	 * The 'verifier' is the source IP address.
		 */
		machine = ((struct authunix_parms *)
			rqstp->rq_clntcred)->aup_machname;
		hp = gethostbyname(machine);
		if (hp == NULL) { 
			svcerr_auth(transp,AUTH_BADCRED);
			return(1);
		}
		bcopy(hp->h_addr,&claim,sizeof(struct in_addr));
		if (actual.sin_addr.s_addr != claim.s_addr) {;
			svcerr_weakauth(transp,AUTH_REJECTEDVERF);
			return(1);
		}
	}
	return(0);
}

/*
 * Check mount requests, add to mounted list if ok
 */
mount(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	fhandle_t fh;
	struct fhstatus fhs;
	char *path, *machine;
	int fd;
	struct mountlist *ml;
	struct stat statbuf;
	struct exports *ex;
	struct groups *gl;

	path = NULL;
	if (!svc_getargs(transp, xdr_path, &path)) {
		svcerr_decode(transp);
		abort();
	}
#ifdef DEBUG
	fprintf(stdout, "path is %s\n", path);
#endif
	machine =
	  ((struct authunix_parms *)rqstp->rq_clntcred)->aup_machname;
	if ((fd = open(path, O_RDONLY, 0)) < 0) {
		fhs.fhs_status = errno;
		perror("mountd: open");
		goto fail;
	}
	if (getfh(fd, &fh) < 0) {
		fhs.fhs_status = errno;
		perror("mountd: getfh");
		close(fd);
		goto fail;
	}
	else
		fhs.fhs_status = 0;
	if (fstat(fd, &statbuf) < 0) {
		fhs.fhs_status = errno;
		perror("mountd: stat");
		close(fd);
		goto fail;
	}
	close(fd);
	for(ex = exports; ex != NULL; ex = ex->ex_next) {
#ifdef DEBUG
		fprintf(stdout, "checking %s %o for %o\n", ex->ex_name, ex->ex_dev, statbuf.st_dev);
#endif
		if (ex->ex_dev != statbuf.st_dev)
			continue;
		if (ex->ex_groups == NULL) {
			goto hit;
		}
		for (gl = ex->ex_groups; gl != NULL; gl = gl->g_next) {
#ifdef DEBUG
			fprintf(stdout, "checking %s for %s\n", gl->g_name, machine);
#endif
			if (innetgr(gl->g_name, machine, NULL, mydomain))
				goto hit;
			if (strcmp(gl->g_name, machine) == 0) {
				goto hit;
			}
		}
	}
	fhs.fhs_status = EACCES;
	goto fail;
  hit:
	fhs.fhs_fh = fh;
	for (ml = mountlist; ml != NULL; ml = ml->ml_nxt) {
		if (strcmp(ml->ml_path, path) == 0 &&
		    strcmp(ml->ml_name, machine) == 0)
			break;
	}
	if (ml == NULL) {
		ml = (struct mountlist *)exmalloc(sizeof(struct mountlist));
		ml->ml_path = (char *)exmalloc(strlen(path) + 1);
		strcpy(ml->ml_path, path);
		ml->ml_name = (char *)exmalloc(strlen(machine) + 1);
		strcpy(ml->ml_name, machine);
		ml->ml_nxt = mountlist;
		mountlist = ml;
	}
fail:
	if (!svc_sendreply(transp, xdr_fhstatus, &fhs)) {
	    	fprintf(stdout, "couldn't reply to rpc call\n");
		abort();
	}
	dumptofile();
	svc_freeargs(transp, xdr_path, &path);
}

/*
 * Remove an entry from mounted list
 */
umount(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	char *path, *machine;
	struct mountlist *ml, *oldml;

	path = NULL;
	if (!svc_getargs(transp, xdr_path, &path)) {
		svcerr_decode(transp);
		abort();
	}
	if (rqstp->rq_cred.oa_flavor == AUTH_UNIX) {
		machine =
		  ((struct authunix_parms *)rqstp->rq_clntcred)->aup_machname;
	}
	else
		return;
#ifdef DEBUG
	fprintf(stdout, "name %s path %s\n", machine, path);
#endif
	oldml = mountlist;
	for (ml = mountlist; ml != NULL;
	    oldml = ml, ml = ml->ml_nxt) {
		if (strcmp(ml->ml_path, path) == 0 &&
		    strcmp(ml->ml_name, machine) == 0) {
			if (ml == mountlist)
				mountlist = ml->ml_nxt;
			else
				oldml->ml_nxt = ml->ml_nxt;
#ifdef DEBUG
	fprintf(stdout, "freeing %s\n", path);
#endif
			free(ml->ml_path);
			free(ml->ml_name);
			free(ml);
			break;
		    }
	}
	if (!svc_sendreply(transp,xdr_void, NULL)) {
	    	fprintf(stdout, "couldn't reply to rpc call\n");
		abort();
	}
	dumptofile();
	svc_freeargs(transp, xdr_path, &path);
}

/*
 * Remove all entries for one machine from mounted list
 */
umountall(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	char *machine;
	struct mountlist *ml, *oldml;

	if (!svc_getargs(transp, xdr_void, NULL)) {
		svcerr_decode(transp);
		return;
	}
	/*
	 * We assume that this call is asynchronous and made via the 
	 * portmapper callit routine.  Therefore return control immediately.
	 * The error causes the portmapper to remain silent, as apposed to
	 * every machine on the net blasting the requester with a response.
	 */
	svcerr_systemerr(transp);
	if (rqstp->rq_cred.oa_flavor == AUTH_UNIX) {
		machine =
		   ((struct authunix_parms *)rqstp->rq_clntcred)->aup_machname;
	}
	else
		return;
	oldml = mountlist;
	for (ml = mountlist; ml != NULL; ml = ml->ml_nxt) {
		if (strcmp(ml->ml_name, machine) == 0) {
#ifdef DEBUG
			fprintf(stdout, "got a hit\n");
#endif
			if (ml == mountlist) {
				mountlist = ml->ml_nxt;
				oldml = mountlist;
			}
			else
				oldml->ml_nxt = ml->ml_nxt;
			free(ml->ml_path);
			free(ml->ml_name);
			free(ml);
		}
		else
			oldml = ml;
	}
	dumptofile();
	svc_freeargs(transp, xdr_void, NULL);
}

/*
 * send current export list
 */
export(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	struct exports *ex;

	if (!svc_getargs(transp, xdr_void, NULL)) {
		svcerr_decode(transp);
		abort();
	}

	ex = exports;
	if (!svc_sendreply(transp, xdr_exports, &ex)) {
	    	fprintf(stdout, "couldn't reply to rpc call\n");
		abort();
	}
}

/*
 * Save current mount state info so we
 * can attempt to recover in case of a crash.
 */
dumptofile()
{
	static char *t1 = "/etc/zzXXXXXX";
	static char *t2 = "/etc/zzXXXXXX";
	FILE *fp;
	struct mountlist *ml;
	char *mktemp();
	int mf;
	
	strcpy(t2, t1);
	t2 = mktemp(t2);
	if ((mf = creat(t2, 0644)) < 0)
		perror("mountd: creat");
	if ((fp = fdopen(mf, "w")) == NULL)
		fprintf(stdout, "mountd: fdopen");
#ifdef sgi
	for (ml = mountlist; ml != NULL; ml = ml->ml_nxt) {
# undef	fprintf
		fprintf(fp, "%s:%s\n", ml->ml_name, ml->ml_path);
# define fprintf syslog
	}
	if (rename(t2, RMTAB) < 0) {
		perror("mountd: rename");
		(void) unlink(t2);
	}
#else
	for (ml = mountlist; ml != NULL; ml = ml->ml_nxt)
		fprintf(fp, "%s:%s\n", ml->ml_name, ml->ml_path);
	if (rename(t2, RMTAB) < 0)
		perror("mountd: link");
#endif
	fclose(fp);
}

/*
 * Restore saved mount state
 */
readfromfile()
{
	FILE *fp;
	struct mountlist *ml;
	char name[BUFSIZ];
	char *path, *index(), *rindex();
	
	fp = fopen(RMTAB, "r");
	if (fp == NULL)
		return;
	while (1) {
		if (fgets(name, sizeof(name), fp) == NULL)
			break;
		path = rindex(name, '\n');
		if (path == NULL)
			break;
		*path = 0;
		path = index(name, ':');
		if (path == NULL)
			break;
		*path++ = NULL;
		ml = (struct mountlist *) exmalloc(sizeof(struct mountlist));
		ml->ml_path = (char *)exmalloc(strlen(path) + 1);
		strcpy(ml->ml_path, path);
		ml->ml_name = (char *)exmalloc(strlen(name) + 1);
		strcpy(ml->ml_name, name);
		ml->ml_nxt = mountlist;
		mountlist = ml;
	}
	fclose(fp);
}

struct groups *
newgroup(name, next)
	char *name;
	struct groups *next;
{
	struct groups *new;
	char *newname;

	new = (struct groups *)exmalloc(sizeof(*new));
	newname = (char *)exmalloc(strlen(name) + 1);
	strcpy(newname, name);

	new->g_name = newname;
	new->g_next = next;
	return (new);
}

struct exports *
newex(name, dev, groups, next)
	char *name;
	dev_t dev;
	struct groups *groups;
	struct exports *next;
{
	struct exports *new;
	char *newname;

	new = (struct exports *)exmalloc(sizeof(*new));
	newname = (char *)exmalloc(strlen(name) + 1);
	strcpy(newname, name);

	new->ex_name = newname;
	new->ex_dev = dev;
	new->ex_groups = groups;
	new->ex_next = next;
	return (new);
}


struct stat exportstat;
int exportdone = 0;
/*
 * Parse exports file
 * If this is the first call or the file exportfile (set in main) has
 * changed exportfile is opened and parsed to create an exports list.
 * file should look like:
 * ^dir names*
 *   or
 * #anything
 * where: dir is the name of a mount point for a local file system
 *        names is a netgroup or host name or a list of white separated names
 *        A '#' anywhere in the line marks a comment to the end of that line
 * NOTE: a non-white character in column 1 indicates a new export specification.
 */
set_exports()
{
	int bol;	/* begining of line */
	int eof;	/* end of file */
	int opt;	/* beginning of option */
	struct exports *ex;
	char ch;
	char *str;
	char *l;
	char line[MAXLINE];	/* current line */
	struct stat statb;
	FILE *fp;
#ifdef sgi
	auto int flags, rootid;
	register int done_exportfs = 0;
#endif

	if (exportdone++) {
		if (stat(exportfile, &statb) < 0) {
			fprintf(stdout, "mountd: stat failed ");
			perror(exportfile);
			freeex(exports);
			exports = NULL;
			return;
		}
		if (exportstat.st_mtime == statb.st_mtime) {
			return;
		}
	}
	exportstat = statb;

	if ((fp = fopen(exportfile, "r")) == NULL) {
		perror(exportfile);
		freeex(exports);
		exports = NULL;
		return;
	}

	freeex(exports);
	eof = 0;
	ex = NULL;
	l = line;
	*l = '\0';
	while (!eof) {
		switch (*l) {
		case '\0':
		case '\n':
#ifdef sgi
			/*
			 * Process the last entry's export options.
			 */
			if (ex != NULL) {
				if (!done_exportfs
				    && exportfs(ex->ex_name,rootid,flags) < 0) {
					fprintf(stdout,
					    "mountd: bad export for ");
					perror(ex->ex_name);
				}
				done_exportfs = 1;
			}
			flags = 0;	/* a normal, r/w export */
			rootid = -2;	/* nobody */
#endif
			/*
			 * End of line, read next line and set state vars
			 */
			if (fgets(line, MAXLINE, fp) == NULL) {
				eof = 1;
			} else {
				bol = 1;
				opt = 0;
				l = line;
			}
			break;
		case ' ':
		case '\t':
			/*
			 * White space, skip to first non-white
			 */
			while (*l == ' ' || *l == '	') {
				l++;
			}
			bol = 0;
			break;
		case '#':
			/*
			 * Comment, skip to end of line.
			 */
			*l = '\0';
			break;
		case '-':
			/*
			 * option of the form: -option=value
			 */
			if (bol) {
				fprintf(stdout,
				    "mountd: can't parse '%s'\n", l);
				*l = '\0';
				break;
			}
			opt = 1;
			l++;
			break;
		default:
			/*
			 * normal character: if col one get dir else name or opt
			 */
			str = l;
			while (*l != ' ' && *l != '\t' &&
			     *l != '\0' && *l != '\n') {
				l++;
			}
			ch = *l;
			*l = '\0';
			if (bol) {
				if (stat(str, &statb) < 0) {
					fprintf(stdout, "mountd: can't stat ");
					perror(str);
					break;
				}
				if (statb.st_ino != ROOTINO ||
				    (major(statb.st_dev) == 0377)) {
					fprintf(stdout,
					    "mountd: %s bad file system root\n",
					    str);
					break;
				} else {
					ex = newex(str, statb.st_dev, NULL, ex);
#ifdef sgi
					done_exportfs = 0;
#endif
				}
			} else {
				if (opt) {
					opt = 0;
#ifdef sgi
					setopt(str, ex, &flags, &rootid);
#else
					setopt(str, ex);
#endif
				} else {
					ex->ex_groups =
					    newgroup(str, ex->ex_groups);
				}
			}
			*l = ch;
			bol = 0;
			break;
		}
	}
	fclose(fp);
	exports = ex;
	return;
}

#ifdef sgi
#include <ctype.h>
#include <pwd.h>
#include <sys/mount.h>

setopt(str, ex, flags, rootid)
	char *str;
	struct exports *ex;
	int *flags, *rootid;
{
	register char *cp, *nextopt;
	char *strchr();

	do {
		nextopt = strchr(str, ',');
		if (nextopt != NULL) {
			*nextopt++ = '\0';
		}
		cp = strchr(str, '=');
		if (cp != NULL) {
			*cp++ = '\0';
		}
		if (!strcmp(str, "ro")) {
			*flags |= EX_RDONLY;
		} else if (!strcmp(str, "nohide")) {
			*flags |= EX_SUBMOUNT;
		} else if (!strcmp(str, "rootid")) {
			if (isdigit(*cp)) {
				*rootid = atoi(cp);
			} else {
				struct passwd *pw = getpwnam(cp);
				if (pw != NULL)
					*rootid = pw->pw_uid;
			}
		}
	} while ((str = nextopt) != NULL);
}
#else
setopt(str, ex)
	char *str;
	struct exports *ex;
{
}
#endif


freeex(ex)
	struct exports *ex;
{
	struct groups *groups, *tmpgroups;
	struct exports *tmpex;

	while (ex) {
		groups = ex->ex_groups;
		while (groups) {
			tmpgroups = groups->g_next;
			free(groups);
			groups = tmpgroups;
		}
		tmpex = ex->ex_next;
		free(ex);
		ex = tmpex;
	}
}

char *
exmalloc(size)
	int size;
{
	char *ret;

	if ((ret = (char *)malloc(size)) == 0) {
		fprintf(stdout, "Out of memory\n");
		exit(1);
	}
	return (ret);
}

catch()
{
}
