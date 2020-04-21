char _Origin_[] = "UniSoft Systems of Berkeley";

#include <stdio.h>
#include <sys/uioctl.h>
#include <sys/disktune.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
 * disktune [-srt #] [-hlt #] [-hut #] device
 *	tune the floppy disk settling time parameters
 * parameters
 *	-srt # - seek motor stepping rate time in ms
 *	-hlt # - head loading time in ms
 *	-hut # - head unload time in ms
 * if no parameters are given, read present ones
 * if parameters are missing, do not change them
 */

#define USAGE	"disktune [-srt #] [-hlt #] [hut #] device"

main(argc, argv)
char **argv;
{
	int fp;
	struct disktune dt;
	struct stat sbuf;
	char *device = 0;
	int rw;

	dt.dt_srt = DISKDEFAULT;
	dt.dt_hlt = DISKDEFAULT;
	dt.dt_hut = DISKDEFAULT;
	while (--argc) {
		argv++;
		if (**argv == '-') {
			if (argc == 0)
				perr("no value with %s parameter", *argv);
			if (strcmp(*argv, "-srt") == 0)
				dt.dt_srt = atoi(argv[1]);
			else if (strcmp(*argv, "-hlt") == 0)
				dt.dt_hlt = atoi(argv[1]);
			else if (strcmp(*argv, "-hut") == 0)
				dt.dt_hut = atoi(argv[1]);
			else
				perr("invalid key %s", *argv);
			--argc;
			argv++;
		} else if (device)
			perr("multiple device specifications");
		else
			device = *argv;
	}
	if (device == 0)
		perr(USAGE);
	rw = (dt.dt_srt==DISKDEFAULT && dt.dt_hlt==DISKDEFAULT &&
	      dt.dt_hut==DISKDEFAULT)?0:2;
	if (stat(device, &sbuf) < 0)
		perr("cannot stat %s", device);
	if ((sbuf.st_mode & S_IFMT) != S_IFCHR)
		perr("must specify RAW device");
	if ((fp = open(device, rw)) < 0)
		perr("cannot open %s", device);
	if (rw)
		if (ioctl(fp, UIOCSETDT, &dt) < 0)
			perr("set parameters failed on %s", device);
	if (ioctl(fp, UIOCGETDT, &dt) < 0)
		perr("get parameters failed on %s", device);
	printf("step rate time = %d\n", dt.dt_srt);
	printf("head load time = %d\n", dt.dt_hlt);
	printf("head unload time = %d\n", dt.dt_hut);
	exit(0);
}

perr(mes, par)
char *mes, *par;
{
	fprintf(stderr, mes, par);
	fprintf(stderr, "\n");
	exit(1);
}
