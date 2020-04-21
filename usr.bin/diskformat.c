char _Origin_[] = "UniSoft Systems of Berkeley";

#include <stdio.h>
#include <sys/uioctl.h>
#include <sys/diskformat.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
 * diskformat [-size #] [-dens #] [-cyl f[-t]] [-head f[-t]] [-sec f[-t]] [-il #] device
 *	format a disk
 * parameters
 *	device - device to be formatted (must be raw device)
 *	-size # - specify sector size in bytes
 *	-dens # - specify density
 *	-cyl #[-#] - format cylinders f to t (default f)
 *		a specification like #- means to end
 *	-head #[-#] - format heads f to t (default f)
 *		a specification like #- means to end
 *	-sec #[-#] - format sectors f to t (default f)
 *		a specification like #- means to end
 *	-il # - interleave factor for the disk
 */

#define USAGE "usage: diskformat [-size #] [-dens #] [-cyl f[-t]] [-head f[-t]] [-sec f[-t]] [-il #] device"

main(argc, argv)
char **argv;
{
	int fp;
	char *device = 0;
	struct diskformat df;
	struct stat sbuf;
	char *p;

	df.d_secsize = DISKDEFAULT;
	df.d_dens = DISKDEFAULT;
	df.d_fcyl = DISKDEFAULT; df.d_lcyl = DISKDEFAULT;
	df.d_fhead = DISKDEFAULT; df.d_lhead = DISKDEFAULT;
	df.d_fsec = DISKDEFAULT; df.d_lsec = DISKDEFAULT;
	df.d_ileave = DISKDEFAULT;
	while (--argc) {
		argv++;
		if (**argv == '-') {
			if (argc < 2)
				perr("%s flag without a value", *argv);
			if (strcmp(*argv, "-dens") == 0) {
				if (df.d_dens != DISKDEFAULT)
					perr("multiple density specifications");
				df.d_dens = atoi(argv[1]);
			} else if (strcmp(*argv, "-size") == 0) {
				if (df.d_secsize != DISKDEFAULT)
					perr("multiple size specifications");
				df.d_secsize = atoi(argv[1]);
			} else if (strcmp(*argv, "-cyl") == 0) {
				if (df.d_fcyl != DISKDEFAULT)
					perr("multiple cyl specifications");
				df.d_fcyl = atoi(p = argv[1]);
				while (*p != 0 && *p != '-') p++;
				if (*p++ == '-') {
					if (*p)
						df.d_lcyl = atoi(p);
				} else
					df.d_lcyl = df.d_fcyl;
			} else if (strcmp(*argv, "-head") == 0) {
				if (df.d_fhead != DISKDEFAULT)
					perr("multiple head specifications");
				df.d_fhead = atoi(p = argv[1]);
				while (*p != 0 && *p != '-') p++;
				if (*p++ == '-') {
					if (*p)
						df.d_lhead = atoi(p);
				} else
					df.d_lhead = df.d_fhead;
			} else if (strcmp(*argv, "-sec") == 0) {
				if (df.d_fsec != DISKDEFAULT)
					perr("multiple sector specifications");
				df.d_fsec = atoi(p = argv[1]);
				while (*p != 0 && *p++ != '-') p++;
				if (*p++ == '-') {
					if (*p)
						df.d_lsec = atoi(p);
				} else
					df.d_lsec = df.d_fsec;
			} else if (strcmp(*argv, "-il") == 0) {
				if (df.d_ileave != DISKDEFAULT)
					perr(
					"multiple interleave specifications");
				df.d_ileave = atoi(argv[1]);
			}
			--argc;
			argv++;
		} else if (device)
			perr("multiple device specifications");
		else
			device = *argv;
	}
	if (device == 0)
		perr(USAGE);
	if (stat(device, &sbuf) < 0)
		perr("cannot stat %s", device);
	if ((sbuf.st_mode & S_IFMT) != S_IFCHR)
		perr("must specify RAW device");
	if ((fp = open(device, 2)) < 0)
		perr("cannot open %s", device);
	printf("About to format %s. Type return to continue:", device);
	while (getchar() != '\n');
	if (ioctl(fp, UIOCFORMAT, &df) < 0)
		perr("formatting failed on %s", device);
	exit(0);
}

perr(mes, par)
char *mes, *par;
{
	fprintf(stderr, mes, par);
	fprintf(stderr, "\n");
	exit(1);
}
