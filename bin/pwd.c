/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* #ident	"@(#)pwd:pwd.c	1.11" */

/*
**	Print working (current) directory
*/

#include	<stdio.h>
#include	<sys/param.h>
#include	<sys/signal.h>
#include	<sys/types.h>
#include	<sys/sysmacros.h>
#include	<sys/stat.h>
#include	<dirent.h>

struct	stat	d, dd;
struct	dirent	*dir;

char	dot[]	 = ".";
char	dotdot[] = "..";
char	name[MAXNAMLEN];

DIR	*file;
int	off = -1;

main()
{
	for(;;) {
		if(stat(dot, &d) < 0) {
			fprintf(stderr, "pwd: cannot stat .!\n");
			exit(2);
		}
		if ((file = opendir(dotdot)) == NULL) {
			fprintf(stderr,"pwd: cannot open ..\n");
			exit(2);
		}
		if(fstat(file->dd_fd, &dd) < 0) {
			fprintf(stderr, "pwd: cannot stat ..!\n");
			exit(2);
		}
		if(chdir(dotdot) < 0) {
			fprintf(stderr, "pwd: cannot chdir to ..\n");
			exit(2);
		}
		if(d.st_dev == dd.st_dev) {
			if(d.st_ino == dd.st_ino)
				prname();
			do
				if ((dir = readdir(file)) == NULL) {
					fprintf(stderr, "pwd: read error in ..\n");
					exit(2);
				}
			while (dir->d_ino != d.st_ino);
		}
		else do {
				if((dir = readdir(file)) == NULL) {
					fprintf(stderr, "pwd: read error in ..\n");
					exit(2);
				}
				stat(dir->d_name, &dd);
		} while(dd.st_ino != d.st_ino || dd.st_dev != d.st_dev);
		(void)closedir(file);
		cat();
	}
}

prname()
{
	write(1, "/", 1);
	if (off<0)
		off = 0;
	name[off] = '\n';
	write(1, name, off+1);
	exit(0);
}

cat()
{
	register i, j;

	i = -1;
	while (dir->d_name[++i] != 0) ;
	if ((off+i+2) > MAXNAMLEN - 1)
		prname();
	for(j=off+1; j>=0; --j)
		name[j+i+1] = name[j];
	off=i+off+1;
	name[i] = '/';
	for(--i; i>=0; --i)
		name[i] = dir->d_name[i];
}
