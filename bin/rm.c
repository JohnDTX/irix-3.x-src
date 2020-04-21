/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* #ident	"@(#)rm:rm.c	1.14" */
/*
**      rm [-fir] file ...
*/

#include        <stdio.h>
#include        <fcntl.h>
#include        <sys/types.h>
#include        <sys/stat.h>
#include        <dirent.h>
#include        <limits.h>

			/* HACK WARNING */
#define ino_t long			/* handle long i-numbers from stat */

#ifndef	S_IFLNK
#define	clstat	stat
#endif	S_IFLNK

#define ARGCNT		5		/* Number of arguments */
#define CHILD		0	 
#ifdef	S_IFLNK
#define	SYMLINK		((buffer.st_mode&S_IFMT) == S_IFLNK)
#endif	S_IFLNK
#define	DIRECTORY	((buffer.st_mode&S_IFMT) == S_IFDIR)
#define	FAIL		-1
#define MAXFORK		100		/* Maximum number of forking attempts */
#define MAXFILES        OPEN_MAX  - 2	/* Maximum number of open files */
#define	MAXLEN		DIRBUF-24  	/* Name length (1024) is limited */
				        /* stat(2).  DIRBUF (1048) is defined */
				        /* in dirent.h as the max path length */
#define	NAMESIZE	MAXNAMLEN + 1	/* "/" + (file name size) */
#define TRUE		1
#define	WRITE		02
int	errcode;
void	exit();
int	interactive, recursive, silent;		/* flags for command line options */
void	  free();
char	  *malloc();
void	  perror();
unsigned  sleep();
char	  *strcpy();

main(argc, argv)
int	argc;
char	*argv[];
{
	extern int	optind;
	int	errflg = 0;
	int	c;

	while ((c = getopt(argc, argv, "fri")) != EOF)
		switch (c) {
		case 'f':
			silent = TRUE;
			break;
		case 'i':
			interactive = TRUE;
			break;
		case 'r':
			recursive = TRUE;
			break;
		case '?':
			errflg = 1;
			break;
		}

	if (argc <= 1 || errflg) {
		fprintf(stderr, "usage: rm [-fir] file ...\n");
		exit(2);
	}
	
	argc -= optind;
	argv = &argv[optind];
	
	while (argc-- > 0) {
		rm (*argv);
		argv++;
	}

	exit(errcode ? 2 : 0);
	/* NOTREACHED */
}


rm (path)
char	*path;
{
	struct stat buffer;
	/* 
	 * CHECK FILE TO SEE IF IT EXISTS?
	 */
	if (clstat(path, &buffer) == FAIL) {
		if (!silent) {
			fprintf(stderr, "rm: %s non-existent\n", path);
			++errcode;
		}
		return;
	}
	
	/*
	 * IF IT IS A DIRECTORY, REMOVE DIRECTORY's CONTENTS.
	 */

	if (DIRECTORY)   {
		rmdirect (path, buffer.st_ino);
		return;
	}
	
	/*
	 * IF INTERACTIVE, ASK FOR ACKNOWLEDGEMENT.
	 */

	if (interactive) {
		printf("%s: ? ", path);
		if (!yes())
			return;
	} else if (!silent) {
		
		/* 
		 * IF NOT SILENT AND STDIN IS A TERMINAL,
		 * WHEN NO WRITE ACCESS, ASK FOR PERMISSION.
		 */

#ifdef S_IFLNK
		/*
		 * ACCESS IS NOT MEANINGFUL FOR SYMBOLIC LINKS
		 */
		if (!SYMLINK)
#endif S_IFLNK
		if (access(path, WRITE) == FAIL && isatty(fileno(stdin))) {
			printf("%s: %o mode ? ", path, buffer.st_mode & 0777);
			
			/* 
			 * IF PERMISION NOT GIVEN, SKIP FILE
			 */

			if (!yes())
				return;
		}
	}
	
	/* 
	 * IF FAILED TO UNLINK, INFORM USER IF INTERACTIVE OR
	 * NOT SILENT.
	 */

	if ((unlink(path) == FAIL) && (!silent || interactive)) {
		fprintf(stderr, "rm: %s not removed.\n", path);
		perror("");
		++errcode;
	}
}


rmdirect(path, inode)
char	*path;
ino_t inode;
{
	char	*newpath;
	int	status, id;
	int	namelen;
	DIR	*name;
	struct dirent *direct;
	/*
	 * IF RECURSIVE, PREPARE TO READ DIRECTORY.
	 */

	if (!recursive) {
		/*
		 * IF NOT RECURSIVE, TRYING TO REMOVE DIRECTORIES IS
		 * AN ERROR. 
		 */

		fprintf(stderr, "rm: %s directory\n", path);
		++errcode;
		return;
	}

	/*
	 * IF INTERACTIVE AND FILE ISN'T IN THE PATH OF CURRENT
	 * WORKING DIRECTORY, ASK FOR ACKNOWLEDGEMENT.
	 */
	
	if (interactive && !mypath(inode)) {
		printf("directory %s: ? ", path);
	
		/*
		 * IF NO ACKNOWLEDGEMENT, SKIP DIRECTORY.
		 */

		if (!yes())
			return;
	}
	
	/*
	 * OPEN DIRECTORY FOR READING 
	 */

	if ((name = opendir(path)) == NULL) {
		fprintf(stderr, "rm: cannot open %s\n", path);
		perror("");
		exit(2);
	}
	
	/* 
	 * READ EVERY DIRECTORY ENTRY.
	 */

	while ((direct = readdir(name)) != NULL) {
		
		/*
		 * IGNORE '.' and '..' DIRECTORY ENTRIES.
 		 */
		
		if ( !strcmp(direct->d_name, ".") 
		  || !strcmp(direct->d_name, ".."))
			continue;
		
		/*
		 * FILE EXISTS, TRY TO REMOVE IT.
		 */
		namelen = strlen(direct->d_name) + 1;
		newpath = malloc ((strlen (path) + namelen));

		if (newpath == NULL) {
			fprintf(stderr,"rm: Insufficient memory.\n");
			perror("");
			exit(1);
		}
		
		/*
		 * LIMIT CHILD PATH LENGTH TO THAT SUPPORTED
		 * SO THAT A CLEAR ERROR MESSAGE IS PROVIDED.
		 * LENGTH OF CHILD = LENGTH OF PARENT + '/' + DIR NAME + NULL 
		 */

		if (strlen(path) + namelen + 1 >= MAXLEN) {
			fprintf(stderr,"rm: Path too long (%d/%d).\n",
					    strlen(path)+namelen+1, MAXLEN);
			exit(2);
		}

		sprintf(newpath, "%s/%s", path, direct->d_name);
 
		/* 
		 * IF A SPARE FILE DESCRIPTOR IS AVAILABLE,
		 * JUST CALL RM SUBROUTINE WITH FILE NAME;
		 * OTHERWISE CLOSE THE LATEST FILE TO REUSE
		 * FOR CHILD AND REOPEN IT WHEN CHILD REMOVED.
		 */

		if (name->dd_fd >= MAXFILES) {
			closedir (name);
			rm (newpath);
			if ((name = opendir(path)) == NULL) {
				fprintf(stderr, "rm: cannot open %s\n", path);
				perror("");
				exit(2);
			}
		} else {
			rm (newpath);
		}
 
		free(newpath);
	}

	/*
	 * CLOSE THE DIRECTORY WE JUST FINISHED READING.
	 */

	closedir(name);
	
	/* 
	 * IF DIRECTORY WE JUST REMOVED THE CONTENTS OF 
	 * IS IN THE PATH OF THE CURRENT WORKING DIRECTORY,
	 * SKIP THE DIRECTORY.
	 */
		
	if (mypath(inode)) {
		return;
	}
	
	/*
	 * IF INTERACTIVE, ASK FOR ACKNOWLEDGEMENT.
	 */

	if (interactive) {
		printf("%s: ? ", path);
		
		/*
		 * IF NO ACKNOWLEDGEMENT, SKIP DIRECTORY.
		 */

		if (!yes())
			return;
	}

	/*
	 * USE 'rmdir' SYSTEM CALL TO REMOVE THE DIRECTORY ITSELF.
	 * IF CALL FAILS, INFORM USER IF INTERACTIVE OR NOT SILENT.
	 */
	if (rmdir(path) != 0 && (!silent || interactive)) {
		fprintf(stderr, "rm: %s not removed.\n", path);
		perror("");
		++errcode;
	}
	return;
}

yes()
{
	int	i, b;

	i = b = getchar();
	while (b != '\n' && b != '\0' && b != EOF)
		b = getchar();
	return(i == 'y');
}


mypath(inode)
ino_t	inode;
{
	struct stat buffer;
	ino_t	child = 0;
	char	*path;
	int	i, j;
	for (i = 1; ; i++) {
		
		/*
		 * STARTING WITH DOT, WALK TOWARD ROOT,
		 * LOOKING AT EACH INODE ALONG THE WAY.
		 */

		path = malloc((3 * i));

		if (path == NULL) {
			fprintf(stderr,"rm: Insufficient memory.\n");
			perror("");
			exit(1);
		}

		strcpy(path, ".");
		for (j = 1; j < i; j++) 
			if (j == 1)
				strcpy(path, "..");
			else
				sprintf(path, "%s/.2s", path, "..");
		stat(path, &buffer);
		/*
		 * IF INODE OF THE CURRENT WORKING DIRECTORY's PATH
		 * MATCHES GIVEN INODE, THEN FILE IS IN PATH AND 
		 * MYPATH IS TRUE.
		 */

		if (buffer.st_ino == inode)
			return(1);
		
		/*
		 * IF AT ROOT, INODE WAS NOT FOUND IN PATH OF THE
		 * CURRENT WORKING DIRECTORY.
		 */

		if (buffer.st_ino == child) 
			return(0);
	
		/* 
		 * SAVE CURRENT INODE, AND GET READY TO PROCESS PARENT.
		 */

		child = buffer.st_ino;
		free(path);
	}
}
