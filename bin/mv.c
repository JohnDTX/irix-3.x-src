/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mv:mv.c	1.22"
#ident	"$Header: /d2/3.7/src/bin/RCS/mv.c,v 1.1 89/03/27 14:50:46 root Exp $"
/*
 * Combined mv/cp/ln command:
 *	mv file1 file2
 *	mv dir1 dir2
 *	mv file1 ... filen dir1
 */



#include	<stdio.h>
#ifdef sgi
#include	<bsd/sys/param.h>
#include	<bsd/sys/dir.h>
#else sgi
#include	<sys/types.h>
#endif
#include	<sys/stat.h>
#include	<errno.h>

#define FTYPE(A)	(A.st_mode)
#define FMODE(A)	(A.st_mode)
#define	IDENTICAL(A,B)	(A.st_dev==B.st_dev && A.st_ino==B.st_ino)
#define ISBLK(A)	((A.st_mode & S_IFMT) == S_IFBLK)
#define ISCHR(A)	((A.st_mode & S_IFMT) == S_IFCHR)
#define ISDIR(A)	((A.st_mode & S_IFMT) == S_IFDIR)
#define ISFIFO(A)	((A.st_mode & S_IFMT) == S_IFIFO)
#define ISREG(A)	((A.st_mode & S_IFMT) == S_IFREG)
#ifdef sgi
#define ISLNK(A)	(((A).st_mode & S_IFMT) == S_IFLNK)
#endif

#define BLKSIZE	4096
#define	DOT	"."
#define	DELIM	'/'
#define EQ(x,y)	!strcmp(x,y)
#define	FALSE	0
#define MODEBITS 07777
#define TRUE 1

char	*malloc();
char	*dname();
char	*strrchr();
int	utime();
extern	int errno;
extern  char *optarg;
extern	int optind, opterr;
#ifndef sgi
struct stat s1, s2;
#endif
int cpy = FALSE;	
int mve = FALSE;	
int lnk = FALSE;	
#ifdef sgi
char symlnk = FALSE;
char iflag = FALSE;
char rflag = FALSE;
#endif sgi
char	*cmd;
int	silent = 0;

main(argc, argv)
register char *argv[];
{
	register int c, i, r, errflg = 0;
#ifdef sgi
	struct stat s1, s2;
#endif
	
	/*
	 * Determine command invoked (mv, cp, or ln) 
	 */
	
	if (cmd = strrchr(argv[0], '/'))
		++cmd;
	else
		cmd = argv[0];
	
	/*
	 * Set flags based on command.
	 */
	 
	  if (EQ(cmd, "mv"))
	  	mve = TRUE;
	  else if (EQ(cmd, "ln")) 
		   lnk = TRUE;
	  	else
#ifdef sgi
		    if (EQ(cmd, "cp"))
			cpy = TRUE;
		    else {
			fprintf(stderr, "%s: I don't know who I am\n",cmd);
			return (1);
		    }
#else			
		   cpy = TRUE;   /* default */
#endif
	
#ifdef sgi
	/*
	 * Check for options:
	 * 	cp [-ri] file1 [file2 ...] target
	 *	ln [-sif] file1 [file2 ...] target
	 *	mv [-if] file1 [file2 ...] target
	 *	mv [-if] dir1 target
	 */
	 
	while ((c = getopt(argc, argv, (cpy ? "ri" : ( lnk ? "sif" : "if"))))
		  != EOF) 
	switch(c) {
		case 'f':
			silent++;
			break;
		case 's':
			symlnk = TRUE;
			break;
		case 'i':
			iflag = TRUE;
			break;
		case 'r':
			rflag = TRUE;
			break;
		default:
			errflg++;
	}
#else sgi
	/*
	 * Check for options:
	 * 	cp file1 [file2 ...] target
	 *	ln [-f] file1 [file2 ...] target
	 *	mv [-f] file1 [file2 ...] target
	 *	mv [-f] dir1 target
	 */
	 
	if (cpy) {
		while ((c = getopt(argc, argv,"")) != EOF) 
			errflg++;
	} else {

		while ((c = getopt(argc, argv,"f")) != EOF) 
	 	switch(c) {
			case 'f':
				silent++;
				break;
			default:
				errflg++;
		}
	}
#endif sgi
	
	/*
	 * Check for sufficient arguments 
	 * or a usage error.
	 */

	argc -= optind;	 
	argv  = &argv[optind];
	
	if (argc < 2) {
		fprintf(stderr,"%s: Insufficient arguments (%d)\n",cmd,argc);
		usage();
	}
	
	if (errflg != 0)
		usage();
#ifndef sgi
	/*
	 * If is is a move command and the
	 * first argument is a directory
	 * then (mv dir1 dir2).
	 */
	 
	stat(argv[0], &s1);
	if (mve) 
		if (ISDIR(s1) && argc == 2) {
	        
			/*
	            	 * Call mv_dir to do actual directory move.
	            	 * NOTE: mv_dir must belong to root 
			 *       and have the set uid bit on.
	        	 */
	        
			execl( "/usr/lib/mv_dir", "mv", argv[0], argv[1], 0 );
	        	fprintf(stderr, "%s:  cannot exec() /usr/lib/mv_dir\n", cmd );
	        	exit(2);
		}
#endif
	/*
	 * If there is more than a source and target,
	 * the last argument (the target) must be a directory
	 * which really exists.
	 */
	 
	if (argc > 2) {
		if (stat(argv[argc-1], &s2) < 0) {
			fprintf(stderr, "%s: %s not found\n", cmd, argv[argc-1]);
			exit(2);
		}
		
		if (!ISDIR(s2)) {
#ifdef sgi
			if (!rflag) {
				fprintf(stderr,
					"%s: Target %s must be directory\n",
					cmd, argv[argc-1]);
				usage();
			}
#else
			fprintf(stderr,"%s: Target must be directory\n",cmd);
			usage();
#endif
		}
	}	
	
	/*
	 * Perform a multiple argument mv|cp|ln by
	 * multiple invocations of move().
	 */
	 
	r = 0;
	for (i=0; i<argc-1; i++)
		r += move(argv[i], argv[argc-1]);
	
	/* 
	 * Show errors by nonzero exit code.
	 */
	 
	 exit(r?2:0);
}

move(source, target)
char *source, *target;
{
	register last, c, i;
	char	*buf = (char *)NULL;
	int from, to, ct, oflg;
	char fbuf[BLKSIZE];
	
	struct	utimbuf	{
		time_t	actime;
		time_t	modtime;
		};
#ifdef sgi
	struct utimbuf times;
	struct stat s1, s2;
#else
	struct utimbuf *times;
#endif

	/* 
	 * While source or target have trailing 
	 * DELIM (/), remove them (unless only "/")
	 */

	while (((last = strlen(source)) > 1)
	    &&  (source[last-1] == DELIM))
		 source[last-1]=NULL;
	
	while (((last = strlen(target)) > 1)
	    &&  (target[last-1] == DELIM))
		 target[last-1]=NULL;
	
	/*
	 * Make sure source file exists.
	 */
#ifdef sgi
		/* You can move or link a dangling symbolic link. */
		/* You can also symbolically link to a non-existent file. */
	if ((cpy ? stat(source, &s1) : lstat(source, &s1)) < 0 && !symlnk) {
#else sgi
	if (stat(source, &s1) < 0) {
#endif sgi
		fprintf(stderr, "%s: cannot access %s\n", cmd, source);
		return(1);
	}

	/* 
         * Make sure source file is not a directory,
	 * we don't move() directories...
	 */
#ifdef sgi
		/* Quit unless making symbolic link, cp -r, or renaming */
	if (ISDIR(s1)
	    && (!symlnk && !mve && !rflag)) {
#else sgi
	if (ISDIR(s1)) {
#endif sgi
		fprintf(stderr, "%s : <%s> directory\n", cmd, source);
		return(1);
	}
	
	/*
	 * If it is a move command and we don't have write access 
	 * to the source's parent then goto s_unlink for the error
	 * message.
	 */

	if ((mve)
	  && accs_parent(source, 2) == -1)
		goto s_unlink;

	/*
	 * If stat fails, then the target doesn't exist,
	 * we will create a new target with default file type of regular.
 	 */	

#ifdef sgi
	FTYPE(s2) = (rflag ? FTYPE(s1) : S_IFREG);
#else
	FTYPE(s2) = S_IFREG;
#endif

#ifdef sgi
	if (stat(target, &s2) >= 0
		 || (!cpy && lstat(target, &s2) >= 0)) {
#else
	if (stat(target, &s2) >= 0) {
#endif
		
		/*
		 * If target is a directory,
		 * make complete name of new file
		 * within that directory.
		 */

		if (ISDIR(s2)) {
			if ((buf = malloc(strlen(target) + strlen(dname(source)) + 4)) == NULL) {
				fprintf(stderr,"%s: Insufficient memory to %s %s\n ",cmd,cmd,source);
				exit(3);
			}
			sprintf(buf, "%s/%s", target, dname(source));
			target = buf;
		}
		
		/*
		 * If filenames for the source and target are 
		 * the same and the inodes are the same, it is
		 * an error.
		 */

#ifdef sgi
		if (stat(target, &s2) >= 0
		    || (!cpy && lstat(target, &s2) >= 0)) {
#else
		if (stat(target, &s2) >= 0) {
#endif
			if (IDENTICAL(s1,s2)) {
				fprintf(stderr, "%s: %s and %s are identical\n", cmd, source, target);
				if (buf != NULL)
					free(buf);
				return(1);
			}
			
#ifdef sgi
			/* if not silent, get confirmation from user */
			if (!silent && iflag && (!ISDIR(s2) || !cpy)) {
				fprintf(stderr, "%s: overwrite %s? ",
					cmd, target);
				i = c = getchar();
				while (c != '\n' && c != EOF)
					c = getchar();
				if (i != 'y') {
					if (buf != NULL) free(buf);
					return(1);
				}
			}
#endif sgi
			/*
			 * Because copy doesn't unlink target,
			 * treat it separately.
			 */
			
			if(cpy)
				goto skip;
			
			/* 
			 * Prevent super-user from overwriting a directory
			 * structure with file of same name.
			 */
			 
			 if (mve && ISDIR(s2)) {
				fprintf(stderr, "%s: Cannot overwrite directory %s\n", cmd, target);
				if (buf != NULL)
					free(buf);
				return(1);
			}
			
			/*
			 * If the user does not have access to
			 * the target, ask him----if it is not
			 * silent and user invoked command 
			 * interactively.
			 */
			
			if (access(target, 2) < 0 
			 && isatty(fileno(stdin))
#ifdef sgi
			 && !iflag
#endif sgi
			 && !silent) {
				fprintf(stderr, "%s: %s: %o mode? ", cmd, target,
					FMODE(s2) & MODEBITS);
			
				/* Get response from user. Based on
				 * first character, make decision.
				 * Discard rest of line.
				 */
				
				i = c = getchar();
				while (c != '\n' && c != EOF)
					c = getchar();
				if (i != 'y') {
					if (buf != NULL)
						free(buf);
					return(1);
				}
			}
			
			/*
			 * Attempt to unlink the target.
			 */
#ifdef sgi			 
			 /* in the mv case, rename call will remove target. */
			 if (!mve && unlink(target) < 0) {
#else
			 if (unlink(target) < 0) {
#endif sgi
				fprintf(stderr, "%s: cannot unlink %s\n", cmd, target);
				if (buf != NULL)
					free(buf);
				return(1);
			}
		}
	}
skip:
	/* 
	 * Either the target doesn't exist,
	 * or this is a copy command ...
	 */
	 
#ifdef sgi
	if (ISDIR(s1) && rflag) {		/* do recursive copy */
	    int result = rcopy(source, target);
	    if (buf != NULL) free(buf);
	    return(result);
	}

	if (lnk) {
	    register int result = 0;
	    if (symlnk) {
		if (symlink(source, target) < 0) {
		    fprintf(stderr, "%s: %s ", cmd, source);
		    perror(target);
		    result = 1;
		}
	    } else if (link(source,target) < 0) {
		if (errno == EXDEV) {
			fprintf(stderr, "%s: %s %s: different file systems\n",
				cmd, source,target);
		} else {
			fprintf(stderr, "%s: %s ", cmd, source);
	                perror(target);
		}
		result = 1;
	    }
	    if (buf != NULL) free(buf);
	    return(result);
	}

	if (mve) {
            /* can't rename directories across devices, for example 
	     * but we want to fall through and do the copy for a 
	     * simple file
	     */
	    if (rename(source, target) < 0) {
		if (errno == EXDEV) {
		    if (!ISREG(s1) && !ISLNK(s1)) {
			fprintf(stderr, "%s: %s %s: different file systems\n",
				cmd, source,target);
			if (buf != NULL) free(buf);
			return (1);
		    /*
		     * if source is a symbolic link, to the 'copy' now.
		     */
		    } else if (ISLNK(s1)) {
			char lbuf[MAXPATHLEN+1];
			register int len;
			register int result = 1;
			len = readlink(source,lbuf,sizeof(lbuf)-1);
			if (len >= sizeof(lbuf)) {
			    lbuf[sizeof(lbuf)-1] = '\0';
			    fprintf(stderr,
				    "%s: symbolic link %s -> %s too long\n",
				    cmd, source, lbuf);
			} else if (len < 0) {
			    fprintf(stderr, "%s: ", cmd);
			    perror(source);
			} else if (unlink(target) < 0) {
			    fprintf(stderr, "%s: ", cmd);
			    perror(target);
			} else if (lbuf[len] = '\0',
				   symlink(lbuf, target) < 0) {
			    fprintf(stderr, "%s: %s ", cmd, source);
			    perror(target);
			} else {
			    result = 0;
			}
			if (buf != NULL) free(buf);
			return (result);
		    }
		} else {
		    fprintf(stderr, "%s: %s ", cmd, source);
		    perror(target);
		    if (buf != NULL) free(buf);
		    return (1);
		}
	    } else {
		if (buf != NULL)
			free(buf);
		return(0);
	    }
	}

#else
	if (cpy || link(source, target) < 0) {

		/*
		 * If link failed, and command was 'ln'
		 * send out appropriate error message.
		 */
		 
		if (lnk) {
			if(errno == EXDEV)
				fprintf(stderr, "%s: different file system\n", cmd);
			else
				fprintf(stderr, "%s: no permission for %s\n", cmd, target);

			if (buf != NULL)
				free(buf);
			return(1);
		}
#endif
		
		/* 
		 * Attempt to open source for copy.
		 */
		 
		if((from = open(source, 0)) < 0) {
			fprintf(stderr, "%s: cannot open %s\n", cmd, source);
			if (buf != NULL)
				free(buf);
			return (1);
		}
		
#ifdef sgi
		/* 
		 * If we are copying to complete a cross-device mv, and the
		 * target is a symbolic link, then remove the link before
		 * trying to copy the file.
		 */
		if (!cpy) {
			struct stat ls;
			if (lstat(target, &ls) >=0 && ISLNK(ls)) {
				if (unlink(target) < 0) {
					fprintf(stderr, "%s: ", cmd);
					perror(target);
					if (buf != NULL) free(buf);
					return (1);
				}
			}
		}
#endif
		/* 
		 * Save a flag to indicate target existed.
		 */
		
		oflg = access(target, 0) == -1;
		
		/* 
		 * Attempt to create a target.
		 */
		
		if((to = creat (target, 0666)) < 0) {
			fprintf(stderr, "%s: cannot create %s\n", cmd, target);
			if (buf != NULL)
				free(buf);
			return (1);
		}
		
		/*
		 * Block copy from source to target.
		 */
		 
		/*
		 * If we created a target,
		 * set its permissions to the source
		 * before any copying so that any partially copied
		 * file will have the source's permissions (at most)
		 * or umask permissions whichever is the most restrictive.
		 */
		 
		if (oflg)
			chmod(target, FMODE(s1));
		
		while((ct = read(from, fbuf, BLKSIZE)) != 0)
			if(ct < 0 || write(to, fbuf, ct) != ct) {
				fprintf(stderr, "%s: bad copy to %s\n", cmd, target);
				/*
				 * If target is a regular file,
				 * unlink the bad file.
				 */
				 
				if (ISREG(s2))
					unlink(target);
				if (buf != NULL)
					free(buf);
				return (1);
			}
		
		/*
		 * If it was a move, leave times alone.
		 */
		if (mve) {
#ifdef sgi   /* if you malloc() times, you should check the results */
			times.actime = s1.st_atime;
			times.modtime = s1.st_mtime;
			utime(target, &times);
#else
			times = (struct utimbuf *) malloc((unsigned) sizeof(struct utimbuf) + 2);
			times->actime = s1.st_atime;
			times->modtime = s1.st_mtime;
			utime(target, times);
			free(times);
#endif
		}
		close(from), close(to);
		
#ifndef sgi
	}
	
	/* 
	 * If it is a copy or a link,
	 * we don't have to remove the source.
	 */
	 
	if (!mve) {
		if (buf != NULL)
			free(buf);
		return (0);
	}
#endif
	
	/* 
	 * Attempt to unlink the source.
	 */
#ifdef sgi
	/* unlink source iff rename failed and we decided to copy */
	if (!cpy && unlink(source) < 0) {
		/*
		 * If we can't unlink the source, assume we lack permission.
		 * Remove the target, as we may have copied it erroneously:
		 *  (1)	from an NFS filesystem, running as root.  In this case
		 *	the call to accs_parent(source) succeeds based on the
		 *	client credentials, but unlink(source) has just failed
		 *	because the server uid for client root is usually -2.
		 *  (2) because after the accs_parent(source) succeeded, we
		 *	lost a race to someone who removed write permission
		 *	from the source directory.
		 */
		if (stat(source, &s1) == 0)	
			(void) unlink(target);
#else
	if (unlink(source) < 0) {
#endif

s_unlink:
		fprintf(stderr, "%s: cannot unlink %s\n", cmd, source);
		if (buf != NULL)
			free(buf);
		return(1);
	}
	if (buf != NULL)
		free(buf);
	return(0);
}


accs_parent(name, amode)
register char *name;
register int amode;
{
	register c;
	register char *p, *q;
	char *buf;

	/*
	 * Allocate a buffer for parent.
	 */
	
	if ((buf = malloc(strlen(name) + 2)) == NULL) {
		fprintf(stderr,"%s: Insufficient memory space.\n",cmd);
		exit(3);
	}
	p = q = buf;
	
	/* 
	 * Copy name into buf and set 'q' to point to the last
	 * delimiter within name.
	 */
	 
	while (c = *p++ = *name++)
		if (c == DELIM)
			q = p-1;
	
	/*
	 * If the name had no '\' or was "\" then leave it alone,
	 * otherwise null the name at the last delimiter.
	 */
	 
	if (q == buf && *q == DELIM)
		q++;
	*q = NULL;
	
	/*
	 * Find the access of the parent.
	 * If no parent specified, use dot.
	 */
	 
	c = access(buf[0] ? buf : DOT,amode);
	free(buf);
	
	/* 
	 * Return access for parent.
	 */
	
	return(c);
}

#ifdef sgi
/* recursive copy */
int
rcopy(from, to)
char *from, *to;
{
	register DIR *fold;
	register struct direct *dp;
	int errs = 0;
	struct stat t_sbuf, f_sbuf;
	int made_dir = 0;
	char fromname[MAXPATHLEN+1];

	fold = opendir(from);
	if (fold == 0) {
		fprintf(stderr, "cannot open %s\n", from);
		return (1);
	}

	if (stat(to, &t_sbuf) < 0) {
		made_dir = 1;
		FMODE(f_sbuf) = 0;
		if (0 > stat(from, &f_sbuf))
			perror(from);
		if (mkdir(to, FMODE(f_sbuf)|0700) < 0) {
			fprintf(stderr, "cp: ");
			perror(to);
			return(1);
		}
	} else if (!ISDIR(t_sbuf)) {
		usage();
	}

	for (;;) {
		dp = readdir(fold);
		if (dp == 0) {
			closedir(fold);
			if (made_dir) {
				if (0 > chmod(to, FMODE(f_sbuf)))
					perror(to);
			}
			return (errs);
		}
		if (dp->d_ino == 0
		    || !strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;

		if (strlen(from) + 1 + dp->d_namlen >= sizeof(fromname)-1) {
			fprintf(stderr, "cp: %s/%s: Name too long.\n",
				from, dp->d_name);
			errs++;
			continue;
		}
		(void) sprintf(fromname, "%s/%s", from, dp->d_name);

		errs += move(fromname, to);
	}
}
#endif sgi

char *
dname(name)
register char *name;
{
	register char *p;

	/* 
	 * Return just the file name given the complete path.
	 * Like basename(1).
	 */
	 
	p = name;
	
	/*
	 * While there are characters left,
	 * set name to start after last
	 * delimiter.
	 */
	 
	while (*p)
		if (*p++ == DELIM && *p)
			name = p;
	return name;
}

usage()
{
#ifdef sgi
	/*
	 * Display usage message.
	 */

	if (cpy) {
		fprintf(stderr, "Usage: cp [-ir] f1 f2\n");
		fprintf(stderr, "       cp [-ir] f1 ... fn d1\n"); 
	} else if (mve) {
		fprintf(stderr, "Usage: mv [-fi] f1 f2\n");
		fprintf(stderr, "       mv [-fi] f1 ... fn d1\n"); 
		fprintf(stderr, "       mv [-fi] d1 d2\n");
	} else {
		fprintf(stderr, "Usage: ln [-fis] f1 f2\n");
		fprintf(stderr, "       ln [-fis] f1 ... fn d1\n"); 
	}
#else
	register char *opt;
	
	/*
	 * Display usage message.
	 */
	 
	opt = cpy ? "" : " [-f]";
	fprintf(stderr, "Usage: %s%s f1 f2\n       %s%s f1 ... fn d1\n", 
		cmd, opt, cmd, opt);
	if(mve)
		fprintf(stderr, "       mv [-f] d1 d2\n");
#endif
	exit(2);
}
