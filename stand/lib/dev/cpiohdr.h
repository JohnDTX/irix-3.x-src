/*
**	cpiohdr - format of cpio file header
**	note that the cpioname is rounded to a short.  The data of the file
**	follows the header and is also rounded to a short
*/
struct cpiohdr {
	short  	cpiomagic;		/* Magic Number */
	short	cpiodev;		/* Device Minor number */
	ushort	cpioino;		/* Inode number */
	ushort	cpiomode;		/* Mode of the device */
	ushort	cpiouid;		/* User ID */
	ushort	cpiogid;		/* Group ID */
	short	cpiolink;		/* Number of links */
	short	cpiordev;		/* device ID */
	short	cpiomtime[2];		/* Last mod date */
	short	cpionamesize;		/* Size of the name field */
	ushort	cpiofilesize[2];	/* Size of the file */
	char	cpioname[256];		/* Name of the file */
};

#define CPIOMAGIC 070707	/* CPIO Magic number */
