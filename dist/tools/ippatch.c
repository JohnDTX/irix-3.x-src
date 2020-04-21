/*
 * NAME
 *	ippatch
 * SYNOPSIS
 *	ippatch [-k kernel] partition base size
 */
#include <stdio.h>
#include <nlist.h>
#include <sys/types.h>
#include <sys/iobuf.h>
#include <sys/dklabel.h>
#define KERNEL
#include <multibus/iphreg.h>

struct nlist nl[] = {
#define	ipsoftc_nl	nl[0]
	{ "_ipsoftc", },
	{ "", }
};

main(argc, argv)
	int argc;
	char **argv;
{
	char *progname;
	char *kernel = "/vmunix";
	register int patchfs;
	struct disk_map dmap;
	register long dmapoff;
	register int kmem;
	extern int errno;

	/*
	 * Process arguments
	 */
	progname = *argv;
	while (--argc > 0 && (*++argv)[0] == '-')
		switch (argv[0][1]) {
		case 'k':
			if (--argc == 0)
				goto usage;
			kernel = *++argv;
			break;
		default:
			fprintf(stderr, "%s: unknown option %s\n",
			    progname, *argv);
			goto usage;
		}
	if (argc != 3)
		goto usage;
	patchfs = argv[0][0];
	if ('a' <= patchfs && patchfs < 'a' + NFS)
		patchfs -= 'a';
	else {
		fprintf(stderr, "%s: bad partition %s; must be [a-h]\n",
		    progname, *argv);
		return 1;
	}
	dmap.d_base =atoi(argv[1]);
	dmap.d_size =atoi(argv[2]);

	/*
	 * Patch kernel
	 */
	nlist(kernel, nl);
	if (ipsoftc_nl.n_value == 0) {
		fprintf(stderr, "%s: can't get namelist for %s\n",
		    progname, kernel);
		return 1;
	}
	dmapoff = ipsoftc_nl.n_value
	    + (int) &((struct softc *) 0)->sc_disk[0].sc_fs[patchfs].d_base;
	if ((kmem = open("/dev/kmem", 1)) < 0
	    || lseek(kmem, dmapoff, 0) < 0
	    || write(kmem, (char *) &dmap, sizeof dmap) < 0) {
		perror("/dev/kmem");
		return errno;
	}
	(void) close(kmem);
	return 0;

usage:
	fprintf(stderr, "usage: %s [-k kernel] partition base size\n",
	    progname);
	return 1;
}
