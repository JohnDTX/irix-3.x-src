/*
* $Source: /d2/3.7/src/stand/simon/RCS/ls.c,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:20:53 $
*/
#include "sys/types.h"
#include "sys/stat.h"
#include "sys/dir.h"

#define BUFSIZE 512
#define MAXCOL	4		/* print 4 names to a line */
#define TABLEN	8

char lsname[50];		/* XXX ugly */

void
listfile(file)
char *file;
{
	register char *cp;
	register struct stat *sp;
	register struct direct *dp;
	register int cnt;
	register int fd;
	register int col;
	int tapeflag;
	char name[DIRSIZ+1];
	char *pre, *ext, *path;
	char *malloc();

	/* create a full load file specification to pass to open	*/
	splitspec(file, &pre, &ext, &path);
	if ( path == 0 || *path == '\0' )	/* XXX - fix splitspec */
		path = "/";			/* default path		*/

	setdflts(&pre, &ext, &path);

	/* XXX hack for cpio tapes */
	if ( (strcmp(pre,"ct") == 0) || (strcmp(pre,"mt") == 0) ||
				(strcmp(pre,"st") == 0) ) {
		tapeflag = 1;
		path = 0;
	} else {
		tapeflag = 0;
	}

	strcpy(lsname,pre);
	strcat(lsname,".");
	strcat(lsname,ext);
	strcat(lsname,":");
	if ( path != 0 )
		strcat(lsname,path);

	if ( (fd = open(lsname,0)) < 0 ) {
		printf("Can't ls ");
		perror( lsname );
		return;
	}

	if ( (cp = malloc(BUFSIZE)) <= 0 ) {
		perror("malloc failed");
		close(fd);
		return;
	}

	sp = (struct stat *)cp;
	if ( fstat(fd, sp) < 0 ) {
		perror("stat failed");
		goto error;
	}

	switch ( sp->st_mode & S_IFMT ) {

	case S_IFREG:
		printf("%s\n",lsname);
		break;

	case S_IFDIR:
		col = 0;
		while ( (cnt=read(fd,cp,BUFSIZE)) > 0 ) {
			dp = (struct direct *)cp;
			while ( cnt ) {
				if ( dp->d_ino != 0 ) {
					strncpy(name,dp->d_name,DIRSIZ);
					name[DIRSIZ] = '\0';
					printf("%s",name);
					if ( (col < MAXCOL)
						&& (strlen(name) >= TABLEN) )
						printf("\t");
					else
						printf("\t\t");
					col++;
				}
				if ( col >= MAXCOL ) {
					col = 0;
					printf("\n");
				}
				dp++;
				cnt -= sizeof(struct direct);
			}
		}
		if ( col != 0 )
			printf("\n");
		break;

	case S_IFCHR:
		if ( tapeflag ) {
			cpiolist(fd,path);
			break;
		} else {
			printf("can only ls character device that is a tape\n");
		}
		break;

	default:
		printf("can only ls disk or tape devices\n");
		printf("bad mode for %s (0x%x)\n",lsname,sp->st_mode);
	}

error:
	/* be a good boy and clean up after yourself	*/
	free(cp);
	close(fd);
}
