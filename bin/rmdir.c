static char *sccsid = "@(#)rmdir.c	4.7 (Berkeley) 12/19/82";
/*
 * Remove directory
 */
#include <stdio.h>

main(argc,argv)
	int argc;
	char **argv;
{
	int errors = 0;

	if (argc < 2) {
		fprintf(stderr, "rmdir: arg count\n");
		exit(1);
	}
	while (--argc)
		if (rmdir(*++argv) < 0) {
			message(*argv);
			errors++;
		}
	exit(errors != 0);
}

#include <errno.h>

extern int sys_nerr, errno;
extern char *sys_errlist[];

/*
|| rmdir failed for 'name' ... print an appropriate error message
*/
message(name)
char *name;
{
	register char *msg;
	char buf[64];

	switch (errno) {
		case EEXIST:	msg = "Directory not empty";
			break;
		case ENOTDIR:	msg = "Path component not a directory";
			break;
		case ENOENT:	msg = "Directory does not exist";
			break;
		case EACCES:	msg = "Search or write permission needed";
			break;
		case EBUSY:	msg = "Directory is a mount point or in use";
			break;
		case EROFS:	msg = "Read-only file system";
			break;
		case EIO:	msg = "I/O error accessing file system";
			break;
		case EINVAL:	msg = "Can't remove current directory or ..";
			break;
		case EFAULT:
		default:
			if (errno < sys_nerr)
				msg = sys_errlist[errno];
			else {
				msg = buf;
				sprintf(msg, "Unknown error %d", errno);
			}
			break;
	}
	fprintf(stderr, "rmdir: %s: %s\n", name, msg);
}
