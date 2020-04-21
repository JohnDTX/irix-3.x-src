/*
 * NAME
 *	tlink - clone a file tree via symbolic links
 * SYNOPSIS
 *	tlink [-chnvX] [-d name] [-x name] source target [path ...]
 * DESCRIPTION
 *	See tlink(1L).
 * AUTHOR
 *	Brendan Eich, 01/14/87
 *
 * $Source: /d2/3.7/src/swtools/RCS/tlink.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:26:05 $
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/nami.h>
#include <sys/stat.h>

/*
 * A pathname structure - MAXPATHLEN is the maximum path length _including_
 * the terminating 0.
 */
struct path {
	char	name[MAXPATHLEN];
	int	len;
};

/*
 * Call path() with a filename pname to initialize a path struct *pp.
 * Add a component to a path with push(); remove it with pop().
 *	char		*pname;
 *	struct path	*pp;
 *	int		len;
 */
void	path(/* pname, pp */);
void	push(/* pp, name, len */);
void	pop(/* pp, len */);

/*
 * A limit on the number of file descriptors to keep open during tree-linking.
 * Since we use depth-first search, the limit corresponds to the depth of the
 * tree at which we begin to close and re-open a directory's descriptor each
 * time we descend into one of its children.
 */
#define	DIR_FD_LIMIT	16
#define	SCARCE_FD_LIMIT	(3+DIR_FD_LIMIT)

/*
 * A list of regular expressions describing filenames to avoid.
 */
#define	MAXBADNAMES	15

char *badnames[] = {
	"^\\.{1,2}$", "^RCS$", "^.*,v$",
	NULL
};
char	*badexprs[MAXBADNAMES+1];

/*
 * With the -d option, we maintain a vector of regular expressions naming
 * directories to symbolically link into the target tree.
 */
#define	MAXDIRLINKS	8

char	*dirlinks[MAXDIRLINKS+1];

char	*progname;
int	(*linkfun)();		/* -h: make hard rather than symbolic links */
short	null_effect = 0;	/* -n: don't create anything */
short	verbose = 0;		/* -v: spit out created pathnames */

/* regular expression routines from libPW */
char	*regcmp(), *regex();

/* error handling utilities */
void	message(/* fmt, a1, a2 */);
int	perrorf(/* fmt, a1, a2 */);
void	sfail(/* fmt, a1, a2 */);
void	fail(/* fmt, a1, a2 */);

int
main(argc, argv)
	int argc;
	char **argv;
{
	register int nexpr, ndirs;
	struct path source, target;
	int clean_target_tree = 0;
	void treelink(/* frompath, topath, cleanflag */);
	int symlink(/* frompath, topath */);
	int link(/* frompath, topath */);

	/*
	 * Initialize.
	 */
	progname = *argv;
	linkfun = symlink;
	for (nexpr = 0; badnames[nexpr] != NULL; nexpr++) {
		badexprs[nexpr] = regcmp(badnames[nexpr], (char *) 0);
	}

	/*
	 * Process options.
	 */
	while (--argc > 0 && (*++argv)[0] == '-') {
		register char *ap;

		for (ap = *argv + 1; *ap != '\0'; ap++)
			switch (*ap) {
			  case 'c':
				clean_target_tree = 1;
				break;
			  case 'd':
				if (argc < 2)
					goto usage;
				--argc, argv++;
				if (ndirs >= MAXDIRLINKS) {
					message(
				"too many directories to link; -d %s ignored\n",
					    *argv);
					break;
				}
				dirlinks[ndirs++] = regcmp(*argv, (char *) 0);
				break;
			  case 'h':
				linkfun = link;
				break;
			  case 'n':
				null_effect = 1;
				break;
			  case 'v':
				verbose = 1;
				break;
			  case 'X':
				nexpr = 1;	/* always exclude . and .. */
				break;
			  case 'x':
				if (argc < 2)
					goto usage;
				--argc, argv++;
				if (nexpr >= MAXBADNAMES) {
					message(
				"too many names to exclude; -x %s ignored\n",
					    *argv);
					break;
				}
				badexprs[nexpr++] = regcmp(*argv, (char *) 0);
				break;
			  default:
				goto usage;
			}
	}
	if (argc < 2)
		goto usage;
	if (nexpr <= MAXBADNAMES)
		badexprs[nexpr] = NULL;

	/*
	 * Construct the path or restrictive sub-paths and call treelink().
	 */
	path(argv[0], &source);
	path(argv[1], &target);
	if (argc == 2)
		treelink(&source, &target, clean_target_tree);
	else {
		argc -= 2, argv += 2;
		while (--argc >= 0) {
			register int arglen = strlen(*argv);

			push(&source, *argv, arglen);
			push(&target, *argv, arglen);
			treelink(&source, &target, clean_target_tree);
			pop(&source, arglen);
			pop(&target, arglen);
			argv++;
		}
	}
	return 0;

usage:
	/*
	 * Complain about abusage.
	 */
	fprintf(stderr,
	    "usage: %s [-cnvX] [-d name] [-x name] source target [path ...]\n",
	    progname);
	return -1;
}

int
makedir(name, mode)
	char *name;
	int mode;
{
	if (!null_effect && mkdir(name, mode) < 0)
		return -1;
	if (verbose)
		printf("%s\n", name);
	return 0;
}

int
makelink(source, target)
	char *source, *target;
{
	if (!null_effect && (*linkfun)(source, target) < 0)
		return -1;
	if (verbose)
		printf("%s\n", target);
	return 0;
}

/*
 * Construct in *pp an absolute path to pname.
 */
void
path(pname, pp)
	register char *pname;
	register struct path *pp;
{
	char *getwd();	/* from libbsd */

	if (pname[0] == '/') {
		(void) strcpy(pp->name, pname);
		pp->len = strlen(pname);
	} else {
		if (getwd(pp->name) == NULL)
			sfail(pp->name);
		pp->len = strlen(pp->name);
		push(pp, pname, strlen(pname));
	}
}

void
push(pp, name, namelen)
	register struct path *pp;
	register char *name;
	register int namelen;
{
	if (pp->len + 1 + namelen >= MAXPATHLEN)
		fail("pathname %s/%s too long", pp->name, name);
	pp->name[pp->len++] = '/';
	(void) strncpy(&pp->name[pp->len], name, namelen);
	pp->len += namelen;
	pp->name[pp->len] = '\0';
}

void
pop(pp, namelen)
	register struct path *pp;
	register int namelen;
{
	pp->len -= namelen + 1;
	assert(pp->name[pp->len] == '/');
	pp->name[pp->len] = '\0';
}

/* XXX doesn't strip trailing ///s */
char *
tail(pp)
	register struct path *pp;
{
	register char *cp;

	for (cp = pp->name+pp->len-1; *cp != '/'; --cp) {
		if (cp == pp->name)
			return cp;	/* no slash in pp */
	}
	return cp + 1;
}

void
treelink(sp, tp, clean_target_tree)
	register struct path *sp, *tp;
	int clean_target_tree;
{
	register int perm;
	struct stat sb;
	register char *startdir;
	void treewalk(/* frompath, topath, nodefun */);
	int clean(/* to, from */), clone(/* from, to */);

	if (stat(sp->name, &sb) < 0 || (sb.st_mode & S_IFMT) != S_IFDIR)
		fail("source %s is not a directory", sp->name);
	perm = sb.st_mode & ~S_IFMT;
	if (stat(tp->name, &sb) < 0) {
		if (clean_target_tree)
			fail("target %s does not exist", tp->name);
		if (maketargetroot(tp->name, perm) < 0)
			sfail("cannot create target directory %s", tp->name);
	} else if ((sb.st_mode & S_IFMT) != S_IFDIR)
		fail("target %s is not a directory", tp->name);

	startdir = clean_target_tree ? tp->name : sp->name;
	if (chdir(startdir) < 0)
		sfail("cannot change directory to %s", startdir);
	if (clean_target_tree)
		treewalk(tp, sp, clean);
	else
		treewalk(sp, tp, clone);
}

int
maketargetroot(name, mode)
	char *name;
	int mode;
{
	register char *lastslash;

	lastslash = strrchr(name, '/');
	if (lastslash != NULL) {
		struct stat sb;

		*lastslash = '\0';
		if (stat(name, &sb) < 0) {
			if (maketargetroot(name, mode) < 0)
				return -1;
		}
		*lastslash = '/';
	}
	return makedir(name, mode);
}

/*
 * Walk a tree rooted at the path in *fp, maintaining a parallel pathname in
 * path *tp.  Precondition is that we have cd'd into fp's root.
 *
 * TODO:
 *	- collapse symbolic links in the source tree
 *	- handle hard links to directories
 */
void
treewalk(fp, tp, pf)
	register struct path *fp, *tp;
	int (*pf)();
{
	register DIR *dirp;
	register long offset;

	dirp = NULL;
	offset = 0;
	for (;;) {
		register struct dirent *dp;
		register int namelen;

		if (dirp == NULL) {
			if ((dirp = opendir(".")) == NULL) {
				sfail("cannot open current directory %s/..",
				    fp->name);
			}
			if (offset != 0) {
				seekdir(dirp, (long)offset);
#ifdef DIROFFBUG
				/*
				 * Since IRIS V.3 seekdir takes you to the
				 * beginning of the last entry that you read,
				 * you must re-read it to get past it.
				 */
				(void) readdir(dirp);
#endif
			}
		}

		if ((dp = readdir(dirp)) == NULL)
			break;
		if (match(dp->d_name, badexprs))
			continue;
		namelen = strlen(dp->d_name);
		push(fp, dp->d_name, namelen);
		push(tp, dp->d_name, namelen);

		if ((*pf)(fp, tp)) {
			if (chdir(dp->d_name) < 0) {
				sfail("cannot change directory to %s",
				    fp->name);
			}
			if (dirp->dd_fd >= SCARCE_FD_LIMIT) {
				offset = telldir(dirp);
				closedir(dirp);
				dirp = NULL;
			}
			treewalk(fp, tp, pf);
			if (chdir("..") < 0) {
				sfail("cannot change directory from %s to ..",
				    fp->name, dp->d_name);
			}
		}

		pop(fp, namelen);
		pop(tp, namelen);
	}
	assert(dirp != NULL);
	closedir(dirp);
}

/*
 * Return true if name matches one of the patterns in the vector rep.
 */
int
match(name, rep)
	register char *name;
	register char **rep;
{
	while (*rep != NULL) {
		if (regex(*rep, name) != NULL)
			return 1;
		rep++;
	}
	return 0;
}

/*
 * Given a file (full path tp->name, relative path tail(tp)) in the target
 * tree, check whether its counterpart sp->name exists.  If not then remove
 * tail(tp).  Being a treewalk() node processing function, clean() must
 * return 1 if tp names a searchable directory, 0 otherwise.
 */
int
clean(tp, sp)
	struct path *tp, *sp;
{
	register char *target = tail(tp);
	register char *source = sp->name;
	register int cleaned;
	struct stat tsb, ssb;

	if (lstat(target, &tsb) < 0) {
		perrorf("cannot get attributes for %s", tp->name);
		return 0;
	}

	/*
	 * If the source exists, check whether both target and source are
	 * directories, returning 1 if so.  If the target is a link check
	 * whether it names the source.
	 */
	if (stat(source, &ssb) == 0) {
		if ((tsb.st_mode & S_IFMT) == S_IFDIR) {
			if ((ssb.st_mode & S_IFMT) == S_IFDIR)
				return 1;
		} else if ((tsb.st_mode & S_IFMT) == S_IFLNK) {
			struct stat sb;	/* link source attributes */

			if (((ssb.st_mode & S_IFMT) != S_IFDIR
			    || match(tp->name, dirlinks))
			    && stat(target, &sb) == 0
			    && sb.st_dev == ssb.st_dev
			    && sb.st_ino == ssb.st_ino) {
				return 0;
			}
		}
	}

	/*
	 * Attempt to remove the target, which may be a non-empty directory.
	 */
	if (null_effect) {
		cleaned = 1;	/* so we can be verbose */
	} else if ((tsb.st_mode & S_IFMT) == S_IFDIR) {
		char cmd[sizeof "rm -rf " + MAXNAMLEN];

		(void) sprintf(cmd, "rm -rf %s", target);
		cleaned = (system(cmd) == 0);
		if (!cleaned)
			message("cannot remove %s\n", target);
	} else {
		cleaned = (unlink(target) == 0);
		if (!cleaned)
			perrorf("cannot unlink %s", target);
	}
	if (cleaned && verbose)
		printf("%s\n", tp->name);
	return 0;
}

/* 
 * If the source is a directory, clone it in the target tree.  Otherwise make
 * a symbolic link named target pointing at source.  Return 1 if source is a
 * directory and should be searched, 0 otherwise.
 */
int
clone(sp, tp)
	struct path *sp, *tp;
{
	register char *source = tail(sp);
	register char *target = tp->name;
	struct stat ssb, tsb;
	int stalepurge(/* target */);

	if (stat(source, &ssb) < 0 || lstat(source, &ssb) < 0) {
		perrorf("cannot get attributes for %s", sp->name);
		return 0;
	}

	/*
	 * If the target exists, check whether it's a stale link.  If it is a
	 * fresh link, return 0 to prevent searching deeper in the source tree.
	 * Otherwise stalepurge() has removed the target.  If the target is a
	 * directory, check whether the source is also.
	 */
	if (lstat(target, &tsb) == 0) {
		if ((tsb.st_mode & S_IFMT) == S_IFLNK) {
			if (!stalepurge(target))
				return 0;
		} else {
			if ((tsb.st_mode & S_IFMT) == S_IFDIR) {
				if ((ssb.st_mode & S_IFMT) == S_IFDIR)
					return 1;
				if (verbose)
					perrnof(EISDIR, target);
			} else if ((ssb.st_mode & S_IFMT) == S_IFDIR) {
				if (verbose)
					perrnof(ENOTDIR, target);
			}
			return 0;
		}
	}

	/*
	 * Either the target did not exist upon entry, or it was a stale link
	 * and we removed it.  Either way, if the source is a directory and if
	 * tp is not one of the directories to be linked, make an empty twin
	 * in the target tree and search the source directory.  Otherwise make
	 * a symbolic link.
	 */
	if ((ssb.st_mode & S_IFMT) == S_IFDIR && !match(target, dirlinks)) {
		if (makedir(target, ssb.st_mode & ~S_IFMT) < 0)
			sfail("cannot make directory %s", target);
		return 1;
	}
	if (makelink(sp->name, target) < 0)
		sfail("cannot symbolically link %s to %s", target, source);
	return 0;
}

/*
 * Return 1 if target, a symbolic link, names a nonexistent file and is
 * successfully removed; return 0 otherwise.
 */
int
stalepurge(target)
	char *target;
{
	struct stat sb;

	if (stat(target, &sb) == 0)
		return 0;
	if (!null_effect && unlink(target) < 0) {
		perrorf("cannot unlink stale link %s", target);
		return 0;
	}
	if (verbose)
		perrnof(ENOENT, target);
	return 1;
}

/*
 * Formatted message output, with progname.
 */
void
message(fmt, a1, a2)
	char *fmt;
	long a1, a2;
{
	fprintf(stderr, "%s: ", progname);
	fprintf(stderr, fmt, a1, a2);
}

/*
 * Formatted perror, with progname.  NB: potentially clobbers errno.
 */
int
perrorf(fmt, a1, a2)
	char *fmt;
	long a1, a2;
{
	extern int errno;

	return perrnof(errno, fmt, a1, a2);
}

int
perrnof(error, fmt, a1, a2)
	int error;
	char *fmt;
	long a1, a2;
{
	extern int sys_nerr;
	extern char *sys_errlist[];

	message(fmt, a1, a2);
	if (0 < error && error <= sys_nerr)
		fprintf(stderr, ": %s", sys_errlist[error]);
	fputc('\n', stderr);
	return error;
}

/*
 * System call failure.
 */
void
sfail(fmt, a1, a2)
	char *fmt;
	long a1, a2;
{
	exit(perrorf(fmt, a1, a2));
}

void
fail(fmt, a1, a2)
	char *fmt;
	long a1, a2;
{
	exit(perrnof(0, fmt, a1, a2));
}
