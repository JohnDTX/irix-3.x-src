/*
 * setmnt(1m) - initialize the mounted-filesystem table
 *
 * author: Brendan Eich
 *
 * $Source: /d2/3.7/src/etc/RCS/setmnt.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 15:38:41 $
 */
#include <ctype.h>
#include <stdio.h>
#include <mntent.h>
#include <sys/fstyp.h>
#include <sys/statfs.h>

char	line[2*(MNTMAXSTR+1)];
#ifdef SVR3
char	filesys[MNTMAXSTR] = "";
char	*fsname = filesys;
#else
char	filesys[MNTMAXSTR] = "/dev/";
char	*fsname = filesys + 5;
#endif
char	dir[MNTMAXSTR];
char	opts[MNTMAXSTR];

int
main(argc, argv)
	int argc;
	char **argv;
{
	register char *mtab;
	register FILE *mtabp;
	extern int errno;

	if (argc == 3 && strcmp(argv[1], "-f") == 0) {
		mtab = argv[2];
	} else {
		if (argc != 1) {
			fprintf(stderr, "usage: setmnt [-f file]\n");
			return -1;
		}
		mtab = MOUNTED;
	}

	if ((mtabp = setmntent(mtab, "w")) == NULL) {
		fprintf(stderr, "setmnt: cannot open ");
		perror(mtab);
		return errno;
	}
	while (gets(line) != NULL) {
		register short fstyp, nfstyp;
		struct statfs sfsb;
		char type[FSTYPSZ];
		struct mntent mnt;
		char *raw, *rawname();

		if (sscanf(line, "%s %s\n", fsname, dir) != 2) {
			fprintf(stderr, "setmnt: malformed input %s.\n",
			    line);
			continue;
		}
		/*
		 * Try to find the raw device name, to set the raw option.
		 */
		raw = rawname(fsname);
		if (raw == NULL) {
			strcpy(opts, MNTOPT_RW);
		} else {
			(void) sprintf(opts, "%s,%s=%s", MNTOPT_RW,
			    MNTOPT_RAW, raw);
		}

		/*
		 * Get filesystem type index and, from it, type name.
		 * Add an mtab entry for filesys.
		 */
		nfstyp = sysfs(GETNFSTYP);
		for (fstyp = 1; fstyp < nfstyp; fstyp++) {
			if (statfs(filesys, &sfsb, sizeof sfsb, fstyp) == 0)
				break;
		}
		if (fstyp != sfsb.f_fstyp
		    || sysfs(GETFSTYP, fstyp, type) < 0) {
			perror(filesys);
			continue;
		}
		mnt.mnt_fsname = filesys;
		mnt.mnt_dir = dir;
		mnt.mnt_type = type;
		mnt.mnt_opts = opts;
		if (addmntent(mtabp, &mnt)) {
			fprintf(stderr, "setmnt: can't add entry to ");
			perror(mtab);
			return errno;
		}
	}
	endmntent(mtabp);
	return 0;
}

#include <sys/types.h>
#include <sys/stat.h>

char *
rawname(fsname)
	char *fsname;	/* full path for SVR3, filename in /dev for SVR0 */
{
	struct stat stb;
#ifdef SVR3
	static char rawdev[MNTMAXSTR];

	if (sscanf(fsname, "/dev/dsk/%s", rawdev + 10) == 1)
		strncpy(rawdev, "/dev/rdsk/", 10);
	else if (sscanf(fsname, "/dev/%s", rawdev + 6) == 1)
		strncpy(rawdev, "/dev/r", 6);
	else
		rawdev[0] = '\0';
#else
	static char rawdev[MNTMAXSTR] = "/dev/r";

	strcpy(rawdev + 6, fsname);
#endif
	if (stat(rawdev, &stb) < 0 || (stb.st_mode & S_IFMT) != S_IFCHR)
		return NULL;
	return rawdev;
}
