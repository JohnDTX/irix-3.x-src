/*
 * $Source: /d2/3.7/src/lib/libc/common/gen/RCS/errlst.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:14:06 $
 */

#ifndef	__ERRNO__

#define	__ERRNO__

static	char unused[] = "Unused";

/*LINTLIBRARY*/
char	*sys_errlist[] = {
	"Error 0",				/*  0 - */
	"Not owner",				/*  1 - EPERM */
	"No such file or directory",		/*  2 - ENOENT */
	"No such process",			/*  3 - ESRCH */
	"Interrupted system call",		/*  4 - EINTR */
	"I/O error",				/*  5 - EIO */
	"No such device or address",		/*  6 - ENXIO */
	"Arg list too long",			/*  7 - E2BIG */
	"Exec format error",			/*  8 - ENOEXC */
	"Bad file number",			/*  9 - EBADF */
	"No child processes",			/* 10 - ECHILD */
	"No more processes",			/* 11 - EAGAIN */
	"Not enough space",			/* 12 - ENOMEM */
	"Permission denied",			/* 13 - EACCES */
	"Bad address",				/* 14 - EFAULT */
	"Block device required",		/* 15 - ENOTBLK */
	"Device busy",				/* 16 - EBUSY */
	"File exists",				/* 17 - EEXIST */
	"Cross-device link",			/* 18 - EXDEV */
	"No such device",			/* 19 - ENODEV */
	"Not a directory",			/* 20 - ENOTDIR */
	"Is a directory",			/* 21 - EISDIR */
	"Invalid argument",			/* 22 - EINVAL */
	"File table overflow",			/* 23 - ENFILE */
	"Too many open files",			/* 24 - EMFILE */
	"Not a typewriter",			/* 25 - ENOTTY */
	"Text file busy",			/* 26 - ETXTBSY */
	"File too large",			/* 27 - EFBIG */
	"No space left on device",		/* 28 - ENOSPC */
	"Illegal seek",				/* 29 - ESPIPE */
	"Read-only file system",		/* 30 - EROFS */
	"Too many links",			/* 31 - EMLINK */
	"Broken pipe",				/* 32 - EPIPE */

    /* math library */
	"Argument out of domain",		/* 33 - EDOM */
	"Result too large",			/* 34 - ERANGE */

    /* non-blocking and interrupt i/o */
	"Operation would block",		/* 35 - EWOULDBLOCK */
	"Operation now in progress",		/* 36 - EINPROGRESS */
	"Operation already in progress",	/* 37 - EALREADY */

    /* argument errors */
	"Socket operation on non-socket",	/* 38 - ENOTSOCK */
	"Destination address required",		/* 39 - EDESTADDRREQ */
	"Message too long",			/* 40 - EMSGSIZE */
	"Protocol wrong type for socket",	/* 41 - EPROTOTYPE */
	"Protocol not available",		/* 42 - ENOPROTOOPT */
	"Protocol not supported",		/* 43 - EPROTONOSUPPORT */
	"Socket type not supported",		/* 44 - ESOCKTNOSUPPORT */
	"Operation not supported on socket",	/* 45 - EOPNOTSUPP */
	"Protocol family not supported",	/* 46 - EPFNOSUPPORT */
	"Address family not supported by protocol family",
						/* 47 - EAFNOSUPPORT */
	"Address already in use",		/* 48 - EADDRINUSE */
	"Can't assign requested address",	/* 49 - EADDRNOTAVAIL */

    /* operational errors */
	"Network is down",			/* 50 - ENETDOWN */
	"Network is unreachable",		/* 51 - ENETUNREACH */
	"Network dropped connection on reset",	/* 52 - ENETRESET */
	"Software caused connection abort",	/* 53 - ECONNABORTED */
	"Connection reset by peer",		/* 54 - ECONNRESET */
	"No buffer space available",		/* 55 - ENOBUFS */
	"Socket is already connected",		/* 56 - EISCONN */
	"Socket is not connected",		/* 57 - ENOTCONN */
	"Can't send after socket shutdown",	/* 58 - ESHUTDOWN */
	"Too many references: can't splice",	/* 59 - ETOOMANYREFS */
	"Connection timed out",			/* 60 - ETIMEDOUT */
	"Connection refused",			/* 61 - EREFUSED */
	"Too many levels of symbolic links",	/* 62 - ELOOP */
	"File name too long",			/* 63 - ENAMETOOLONG */
	"Host is down",				/* 64 - EHOSTDOWN */
	"Host is unreachable",			/* 65 - EHOSTUNREACH */

    /* an nfs-related error */
	"Directory not empty",			/* 66 - ENOTEMPTY */

    /* 5.0 ipc */
	"No message of desired type",		/* 67 - ENOMSG */
	"Identifier removed",			/* 68 - EIDRM */

    /* more nfs-related errors */
	"Disc quota exceeded",			/* 69 - EDQUOT */
	"Stale NFS file handle",		/* 70 - ESTALE */
	"Too many levels of remote in path",	/* 71 - EREMOTE */

    /* sgi errors - 76 thru 79 are rfu */
	"Inode table is full",			/* 72 - EINODETABLEFULL */
	"No floating point hardware",		/* 73 - ENOFPA */
	"Graphics already in use",		/* 74 - EGRBUSY */
	"No graphics hardware present",		/* 75 - ENOGR */
	"Text table is full",			/* 76 - ETEXTABLEFULL */
	unused,					/* 77 */
	unused,					/* 78 */
	unused,					/* 79 */

    /* 5.2.1 locking errors */
	"Deadlock condition",			/* 80 - EDEADLK */
	"No record locks available",		/* 81 - ENOLCK */

    /* stream problems */
	"Not a stream device",			/* 82 - ENOSTR */
	"No data available",			/* 83 - ENODATA */
	"Timer expired",			/* 84 - ETIME */
	"Out of stream resources",		/* 85 - ENOSTR */
	"Not a data message",			/* 86 - EBADMSG */
};
int	sys_nerr = { sizeof(sys_errlist)/sizeof(sys_errlist[0]) };

#endif	__ERRNO__
