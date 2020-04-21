/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*#ident	"@(#)kern-port:sys/nami.h	10.3"*/

#ifndef MAXPATHLEN
# define MAXPATHLEN	1024
#endif

struct pathname {
	char	*pn_buf;
	char	*pn_component;		/* current component pointer */
	char	*pn_nextchar;		/* next char to process */
	ushort	pn_complen;		/* current component length */
	ushort	pn_maxsize;		/* total bytes ever used in buf */
};

#define	SYSPATH		fsstring
#define	USERPATH	fustring
int	fsstring(), fustring();

enum pnfollow { DONTFOLLOW=0, FOLLOWLINK=1 };

#ifdef KERNEL
/*
 * Macros and globals for "." and ".." testing.
 */
#define	ISDOT(cp, len) \
	((len) == 1 && (cp)[0] == '.')
#define	ISDOTDOT(cp, len) \
	((len) == 2 && (cp)[0] == '.' && (cp)[1] == '.')
/*
 * Fast tests for "." and ".." where the string to test is zero-padded to
 * a long boundary.
 */
#define	FAST_ISDOT(cp) \
	(*(long *)(cp) == *(long *)dot)
#define	FAST_ISDOTDOT(cp) \
	(*(long *)(cp) == *(long *)dotdot)

extern char dot[], dotdot[];

/*
 * Pathname macro operations.  A pathname is represented by a null-termintaed
 * string which may be broken into null-terminated components.
 */
#define	pn_peekchar(pn)		(*(pn)->pn_nextchar)
#define	pn_endofpath(pn)	(pn_peekchar(pn) == '\0')

#define	PN_SKIPSLASH(pn) { \
	register char *cp = (pn)->pn_nextchar; \
	while (*cp == '/') \
		cp++; \
	(pn)->pn_nextchar = cp; \
}

/*
 * Isolate the next component in pn by skipping over and terminating it.
 * As with old AT&T and SGI pathname handling code, this algorithm permits
 * such solecisms as "/bin/tar/" and "/etc/passwd/".
 */
#define	PN_GETCOMPONENT(pn, cp) { \
	cp = (pn)->pn_nextchar; \
	(pn)->pn_component = cp; \
	while (*cp != '\0' && *cp != '/') \
		cp++; \
	(pn)->pn_complen = cp - (pn)->pn_component; \
	while (*cp == '/') \
		*cp++ = '\0'; \
	(pn)->pn_nextchar = cp; \
	cp = (pn)->pn_component; \
}

/*
 * Initializer and symbolic link expander for the pathname object.
 */
int
pathname(/* char *namep,		// pointer to name bytes
	    int (*fetch)(),		// SYSPATH or USERPATH
	    struct pathname *pn		// result parameter
	  */);

int
pn_putlink(/* struct pathname *pn,	// pathname containing symlink
	      struct inode *symlinkip	// symlink inode to read
	    */);

#define	MAXLINKDEPTH	8

#endif	/* KERNEL */

/*
 * Structure used by system calls to pass parameters
 * to the file system independent namei and attribute
 * functions.
 */
struct argnamei {	/* namei's flag argument */
	ushort	cmd;	/* command type (see below) */
	short	rcode;	/* a scratch for return codes (see below) */
	long	mode;	/* mode for creat and chmod */
	ushort	ftype;	/* file type */
	ushort	uid;	/* uid for chown */
	ushort	gid;	/* gid for chown */
	dev_t	idev;	/* dev for creat */

	caddr_t	name;	/* symbolic link or rename source filename */
	ushort	namlen;	/* length of symbolic link contents at name */
	ushort	namseg;	/* address space of name */

	struct inode *dp;	/* rename source file's directory */
	struct inode *ip;	/* rename/link source inode, unlocked */
};

/*
 * Possible values for argnamei.cmd field 
 */
#define	NI_DEL		0x1	/* unlink this file */
#define	NI_CREAT	0x2	/* create */
#define	NI_XCREAT	0x3	/* Exclusive create, error if */
				/* the file exists */
#define	NI_LINK		0x4	/* make a link */
#define	NI_MKDIR	0x5	/* mkdir */
#define	NI_RMDIR	0x6	/* rmdir */
#define	NI_MKNOD	0x7	/* mknod */

#define	NI_SYMLINK	0x8	/* make a symbolic link */
#define	NI_RENAME	0x9	/* rename a file */

/* pseudo-command used as argnamei pointer value */
#define	NI_LOOKUP	0

/* Requests to fs_setattr */
#define	NI_CHOWN	0x1	/* change owner */
#define	NI_CHMOD	0x2	/* change mode (permissions and 
				/* ISUID, ISGID, ISVTX) */

/* Codes to fs_notify */
#define	NI_OPEN		0x1	/* open - some fstyps may want */
				/* to know when a file is opened */
#define	NI_CLOSE	0x2	/* close */
#define	NI_CHDIR	0x3	/* let fstyp know that a cd */
				/* is happening (e.g., some fstyp */
				/* may need to know so that */
				/* directory cache can be */
				/* flushed - if it has one) */

/*
 * Return Codes for argnamei.rcode
 */
#define	FSN_FOUND	0x1	/* The file was found by namei (it exists) */
#define	FSN_NOTFOUND	0x2	/* The file was not found by namei  */
				/* (i.e., it does not exist */

/*
 * Return Codes for file sytem dependent namei's
 */
#define	NI_PASS		0	/* Error free FS specific namei */
#define	NI_FAIL		1	/* Error encountered in FS specific namei */
#define	NI_RESTART	2	/* The fs dependent code overwrote */
				/* the buffer and the namei must */
				/* begin again. Used mostly for */
				/* symbolic links. */
#define	NI_DONE		3	/* The fs dependent operation is */
				/* complete. There is no need to do */
				/* any further pathnme processing. */
				/* This is equivalent to NI_PASS for */
				/* those commands that require some */
				/* action from the fs dependent code */
#define NI_NULL 	4	/* Tell fs independent to return NULL to */
				/* the calling procedure without doing /*
				/* an iput */

/*
 * Data that is passed from the file system independent namei to the
 * file system dependent namei.
 */
struct nx {
	struct	inode *dp;	/* directory inode in, sometimes child out */
	struct	inode *ip;	/* child inode for NI_LOOKUP pseudo-command */
	caddr_t	comp;		/* pointer to beginning of current component */
	ushort	len;		/* component length */
	struct pathname *pn;	/* pointer to the pathname buffer */
	enum pnfollow follow;	/* whether to follow a terminal symlink */
	long	flags;		/* Flag field */
};

#define	NX_ISROOT	0x1	/* Inode is root of fs */

/* Values for fsinfo[].fs_notify. If an fstyp wishes to be */
/* notified of an action the appropriate flag should be set */
/* in fsinfo[].fs_notify and fstypsw[].fs_notify should point */
/* to the desired fs dependent function */
#define	NO_CHDIR	0x1	/* chdir */
#define	NO_CHROOT	0x2	/* chroot */
#define	NO_SEEK		0x4	/* seek */

struct argnotify {
	long 	cmd;	/* command - see above */
	long	data1;	/* Allow caller to pass two pieces of data. */
	long 	data2;	/* These should be caste appropriately in */
			/* the fs_notify routine. They are declared */
			/* as longs here. However, they should be */
			/* large enough to hold the largest machine */
			/* specific data types (e.g., if a pointer is */
			/* larger than a long then these should be */
			/* int *). */
};

#ifdef KERNEL
/*
 * Namei() is the traditional inode lookup-by-pathname function.  The others
 * are pathname object operators used by namei(), rename(), and symbolic link
 * expansion code.
 */
struct inode *
namei(/* int (*fetch)(),		// SYSPATH or USERPATH
	 struct argnamei *action,	// NI_LOOKUP or (struct argnamei *)
	 enum pnfollow follow		// whether to follow tail symlinks
       */);

int	
pn_lookup(/* struct pathname *pn,
	     struct argnamei *action,
	     enum pnfollow follow,
	     struct inode **dpp,	// result inodes if non-null
	     struct inode **ipp
	   */);

#endif	/* KERNEL */
