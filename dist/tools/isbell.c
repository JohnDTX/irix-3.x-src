/*
 * NAME
 *	isbell - test whether a filesystem is Bell
 * SYNOPSIS
 *	isbell device
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/filsys.h>

#define	perr(msg)	write(2, msg, sizeof msg)

main(argc, argv)
	int argc;
	char *argv[];
{
	register int fs;
	struct filsys sb;
	extern int errno;

	if (argc <= 1)
		return -1;
	if ((fs = open(argv[1], 0)) < 0) {
		perr("isbell: can't open filesystem device\n");
		return errno;
	}
	if (lseek(fs, SUPERBOFF, 0) < 0) {
		perr("isbell: can't seek to superblock\n");
		return errno;
	}
	if (read(fs, &sb, sizeof sb) != sizeof sb) {
		perr("isbell: can't read superblock\n");
		return errno;
	}
	return sb.s_magic != FsMAGIC;	/* 0 if bell, 1 otherwise */
}
