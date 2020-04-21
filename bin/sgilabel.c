/*
 * NAME
 *	sgilabel - read labels from sgi disks, or update a disk label
 * SYNOPSIS
 *	sgilabel [ partition | disk ] ...
 *	sgilabel [-b bootfs] [-n name] [-p fs:base,size] [-r rootfs]
 *		 [-s serial] [-S swapfs] disk
 * DESCRIPTION
 *	This a rewrite of sgilabel which allows modifications to more
 *	information in the disk label than the previous version.  Usage
 *	is more strictly checked: it either updates one disk or dumps
 *	zero or more disks or partitions.  A disk (partition) may be
 *	given as a device pathname (/dev/dv#f), a pathname suffix (dv#f),
 *	or a disk name and unit (dv#).
 *
 *	Using the -p option, one or more partitions may be created on
 *	a disk.  The partition parameters are:
 *		fs	a file system specification either of the form
 *			[a-h] or [0-8]
 *		base	the partition base in sectors
 *		size	the partition length in sectors
 *	The -b option allows the disk label bootfs field to be changed.
 *	The -r option changes the rootfs field, while -S changes the swapfs
 *	field.  Name (-n) and serial number (-s) options work as before.
 * BUGS
 *	This program depends upon current sgi disk device node naming
 * 	conventions (/dev/dv#f).
 * 
 * $Source: /d2/3.7/src/bin/RCS/sgilabel.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 14:51:04 $
 */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/dklabel.h>
#include <sys/dktype.h>

extern	int errno;

char	*progname;
void	usage();
void	pexit(/* char *fmt ... */);
void	perr(/* char *fmt ... */);

#define	DUMP		0	/* label i/o modes */
#define	UPDATE		2

#define	NULLFS		0xff	/* these should probably be in dklabel.h */
#define	LABELFS		7
#define	LASTFS		(NFS-1)
#define	LABELDEVFMT	"/dev/%sh"
#define	DEVNAMELEN	32

/*
 * Partition structure, an extension of struct disk_map.
 */
struct part {
	unsigned char	p_fs;	/* partition number [0-7) */
	struct disk_map	p_dmap;	/* base and size */
};
#define	p_base	p_dmap.d_base
#define	p_size	p_dmap.d_size

main(argc, argv)
	register int argc;
	register char **argv;
{
	int mode = DUMP;
	char *newname = NULL;
	struct part parts[NFS];
	struct part *partp = parts;
	unsigned char newbootfs = NULLFS;
	unsigned char newrootfs = NULLFS;
	unsigned char newswapfs = NULLFS;
	char *newserial = NULL;

	void getpart(/* char *arg, struct part *partp */);
	unsigned char getfs(/* char *arg */);
	void update(/* char *disk, unsigned char bootfs, char *name,
		       struct part *beginp, struct part *endp,
		       unsigned char rootfs, char *serial,
		       unsigned char swapfs */);
	void dump(/* char *name */); 

	if ((progname = strrchr(*argv, '/')) == NULL)
		progname = *argv;
	else
		progname++;

	/*
	 * Process options - currently any option means the mode is UPDATE,
	 * so we could read the label and operate on it in-line, but I wrote
	 * this so it could be easily extended for DUMP options.
	 */
	while (--argc > 0 && (*++argv)[0] == '-') {
		if (--argc == 0)
			usage();
		switch ((*argv)[1]) {
		  case 'b':
			newbootfs = getfs(*++argv);
			break;
		  case 'n':
			newname = *++argv;
			break;
		  case 'p':
			if (partp >= &parts[NFS])
				pexit("too many partitions");
			getpart(*++argv, partp++);
			break;
		  case 'r':
			newrootfs = getfs(*++argv);
			break;
		  case 's':
			newserial = *++argv;
			break;
		  case 'S':
			newswapfs = getfs(*++argv);
			break;
		  default:
			pexit("unknown option %s", *argv);
			break;
		}
		mode = UPDATE;
	}

	/*
	 * Either we are updating one disk label or we are dumping
	 * zero or more disk labels.
	 */
	if (mode == UPDATE) {
		if (argc == 0 || argc > 1)
			usage();	/* missing disk argument */
		update(*argv, newbootfs, newname, parts, partp, newrootfs,
		    newserial, newswapfs);
	} else {
		while (--argc >= 0)
			dump(*argv++);
	}
	exit(errno);
}

/*
 * Return through partp a new partition structure built from arg.  Error
 * exit if partition spec is malformed or if parameters are obviously bogus.
 */
void
getpart(arg, partp)
	register char *arg;
	register struct part *partp;
{
	register char *bp, *sp;

	if ((bp = strchr(arg, ':')) == NULL
	    || (sp = strchr(bp, ',')) == NULL)
		usage();
	*bp++ = '\0';
	*sp++ = '\0';
	partp->p_fs = getfs(arg);
	partp->p_base = atoi(bp);
	partp->p_size = atoi(sp);
	if (partp->p_base < 0 || partp->p_size < 0)
		pexit("illegal base,size for partition %s", arg);
}

/*
 * Return partition number, specified in arg as either [a-g] or [0-6].
 * Error exit if illegal fs number.
 */
unsigned char
getfs(arg)
	char *arg;
{
	register unsigned char fs = arg[0];

	if ('a' <= fs && fs <= 'a' + LASTFS)
		fs -= 'a';
	else if ('0' <= fs && fs <= '0' + LASTFS)
		fs -= '0';
	else
		pexit("illegal file system %s", arg);
	return fs;
}

/*
 * Getlab() complains and returns false if it can't read a label.
 * Otherwise it returns true with the label in *dlp and either NULLFS
 * or a partition number in *fsp depending upon whether or not name
 * denotes a partition or a disk (fsp == NULL means don't-care).
 *
 * Calling putlab() after getlab() writes the label pointed at by dlp
 * to the disk indicated by name.
 */
int	getlab(/* char *name, struct disk_label *dlp,
		  unsigned char *fsp */);
int	putlab(/* char *name, struct disk_label *dlp */);

/*
 * Read disk's label and update the specified members.
 * Take care to split boot/root meaning of d_rootfs member for old
 * labels (ones for which !d_rootnotboot).
 */
void
update(disk, bootfs, name, beginp, endp, rootfs, serial, swapfs)
	char *disk, *name, *serial;
	unsigned char bootfs, rootfs, swapfs;
	struct part *beginp, *endp;
{
	struct disk_label dl;
	register struct part *pp;

	if (! getlab(disk, &dl, (unsigned char *)NULL))
		return;
	if (!dl.d_rootnotboot) {
		if (rootfs == NULLFS)
			rootfs = dl.d_bootfs;
		dl.d_rootnotboot = 1;
	}
	if (bootfs != NULLFS)
		dl.d_bootfs = bootfs;
	if (name != NULL)
		(void) strcpy(dl.d_name, name);
	for (pp = beginp; pp < endp; pp++)
		dl.d_map[pp->p_fs] = pp->p_dmap;
	if (rootfs != NULLFS)
		dl.d_rootfs = rootfs;
	if (serial != NULL)
		(void) strcpy(dl.d_serial, serial);
	if (swapfs != NULLFS)
		dl.d_swapfs = swapfs;
	(void) putlab(disk, &dl);
}

/*
 * Either pretty-print a disk label or dump partition size depending
 * upon whether name denotes a disk or a partition.
 */
void
dump(name)
	char *name;
{
	struct disk_label dl;
	auto unsigned char sizeonly;
	void dumplab();

	if (! getlab(name, &dl, &sizeonly))
		return;
	if (sizeonly == NULLFS)
		dumplab(name, &dl);
	else if (sizeonly == LABELFS)	/* protect label */
		printf("0\t0\n");
	else {
		/* ASSERT (sizeonly < LASTFS) */
		printf("%d\t%d\n", dl.d_map[sizeonly].d_size,
		    dl.d_heads * dl.d_sectors);
	}
}

/*
 * Pretty-print a disk label.
 */
void
dumplab(name, dlp)
	char *name;
	register struct disk_label *dlp;
{
	register int spc, i, rootfs;
	char *prdtype(/* u_long type, struct dk_type dkt,
		         u_long ntypes */);

	rootfs = (dlp->d_rootnotboot) ? dlp->d_rootfs : dlp->d_bootfs;
	spc = dlp->d_heads * dlp->d_sectors;
	printf("%s: Name: %s, Serial: %s\n", name, dlp->d_name, dlp->d_serial);
	printf("     drive: %s, controller: %s\n",
	    prdtype(dlp->d_type, dk_dtypes, NTYPES),
	    prdtype(dlp->d_controller, dk_dcont, NCONTS));
	printf("     cylinders/heads/sectors(512 byte): %d/%d/%d\n",
	    dlp->d_cylinders, dlp->d_heads, dlp->d_sectors);
	printf("     alternate cylinder/# of alt cylinders: %d/%d\n",
	    dlp->d_altstart/spc, dlp->d_nalternates/spc);
	printf("     badtracks=%d, interleave=%d, trkskw=%d, cylskw=%d\n",
	    dlp->d_badspots, dlp->d_interleave, dlp->d_trackskew,
	    dlp->d_cylskew);
	printf("\tfs     base          size\n\t       sectors(cylinders)\n");
	for (i = 0; i < NFS; i++) {
		if (dlp->d_map[i].d_size)
			printf("\t%c: %6d(%4d%s), %6d(%4d%s)%s\n",
			    i + 'a',
			    dlp->d_map[i].d_base,
			    dlp->d_map[i].d_base / spc,
			    (dlp->d_map[i].d_base % spc) ? "+" : "",
			    dlp->d_map[i].d_size,
			    dlp->d_map[i].d_size / spc,
			    (dlp->d_map[i].d_size % spc) ? "+" : "",
			    (i == rootfs) ? " Root" :
			    (i == dlp->d_swapfs) ? " Swap" :
			    (i == dlp->d_bootfs) ? " Boot" : "");
	}
}

/*
 * There should be an #ifdef KERNEL ... #endif around prdtype's declaration
 * in <sys/dktype.h>.
 */
char *
prdtype(type, dkt, ntypes)
	register u_long type, ntypes;
	register struct dk_type *dkt;
{
	static char buf[40];

	while (ntypes-- != 0) {
		if (dkt->d_type == type)
			return (dkt->d_name);
		dkt++;
	}
	sprintf(buf, "unknown type (%d)", type);
	return (buf);
}

/*
 * Disk label i/o functions.
 *
 * These routines are not general - putlab() cannot be called until
 * after getlab(), and moreover puts its argument label to the device
 * named in the last getlab().
 *
 * Openlab() converts name into a device node name, opens the device,
 * and returns a file descriptor directly, the label device name in
 * labeldev, and a partition number if specified by name in fsp.
 */
int	labelfd = -1;
int	openlab(/* char *name, char *labeldev, unsigned char *fsp */);

int
getlab(name, dlp, fsp)
	register char *name;
	register struct disk_label *dlp;
	unsigned char *fsp;
{
	register int nb;
	char labeldev[DEVNAMELEN];

	if (labelfd != -1)
		(void) close(labelfd);
	if ((labelfd = openlab(name, labeldev, fsp)) < 0)
		perr("can't open %s", labeldev);
	else if ((nb = read(labelfd, (char *) dlp, sizeof *dlp)) < 0)
		perr("can't read label from %s", labeldev);
	else if (nb != sizeof *dlp)
		perr("short label on %s", labeldev);
	else if (dlp->d_magic != D_MAGIC)
		perr("drive %s not initialized\n", name);
	else
		return 1;
	return 0;
}

int
putlab(name, dlp)
	register char *name;
	register struct disk_label *dlp;
{
	off_t lseek();

	/* ASSERT (labelfd >= 0) */
	if (lseek(labelfd, (off_t) 0, 0) != (off_t) 0)
		perr("can't seek to label on %s", name);
	else if (write(labelfd, (char *) dlp, sizeof *dlp) != sizeof *dlp)
		perr("can't write label on %s", name);
	else {
		(void) close(labelfd);
		return 1;
	}
	return 0;
}

/*
 * Open the label partition device indicated by name.  Translate thus:
 *	name		labeldev	*fsp
 *	[/dev/]dv#[f]	/dev/dv#h	[f - 'a']
 * Return NULLFS through fsp if no file system was specified.
 */
int
openlab(name, labeldev, fsp)
	register char *name, *labeldev;
	unsigned char *fsp;
{
	register char *cp;
	register unsigned char fs = NULLFS;

	if ((cp = strchr(name, ':')) != NULL)
		*cp = 0;
	/* enhanced to allow /dev/dv#f arguments as well as dv#f... */
	if (strncmp(name, "/dev/", 5) == 0)
		name += 5;
	if (strlen(name) == 4) {
		fs = name[3] - 'a';
		if (LASTFS < fs) {
			perr("illegal file system %s", name);
			fs = NULLFS;
		}
		name[3] = '\0';
	}
	if (fsp != NULL)
		*fsp = fs;
	(void) sprintf(labeldev, LABELDEVFMT, name);
	return open(labeldev, 2);
}

/*
 * Usage and miscellaneous error handlers.
 */
void
usage()
{
	fprintf(stderr,
	    "usage:\t%s [ partition | disk ] ...\n",
	    progname);
	fprintf(stderr,
	    "or\t%s [-b bootfs] [-n name] [-p fs:base,size] [-r rootfs]\n",
	    progname);
	fprintf(stderr,
	    "\t\t [-s serial] [-S swapfs] disk\n");
	exit(-1);
}

void
pexit(fmt, a1)
	char *fmt, *a1;
{
	perr(fmt, a1);
	exit(errno == 0 ? -1 : errno);
}

void
perr(fmt, a1)
	char *fmt, *a1;
{
	extern char *sys_errlist[];

	fprintf(stderr, "%s: ", progname);
	fprintf(stderr, fmt, a1);
	if (errno != 0)
		fprintf(stderr, ": %s", sys_errlist[errno]);
	fprintf(stderr, ".\n");
}
