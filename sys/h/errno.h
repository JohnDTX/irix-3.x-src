#ifndef	__ERRNO__

#define	__ERRNO__

/*
 * Error codes
 */

#define	EPERM	1	/* Not super-user			*/
#define	ENOENT	2	/* No such file or directory		*/
#define	ESRCH	3	/* No such process			*/
#define	EINTR	4	/* interrupted system call		*/
#define	EIO	5	/* I/O error				*/
#define	ENXIO	6	/* No such device or address		*/
#define	E2BIG	7	/* Arg list too long			*/
#define	ENOEXEC	8	/* Exec format error			*/
#define	EBADF	9	/* Bad file number			*/
#define	ECHILD	10	/* No children				*/
#define	EAGAIN	11	/* No more processes			*/
#define	ENOMEM	12	/* Not enough core			*/
#define	EACCES	13	/* Permission denied			*/
#define	EFAULT	14	/* Bad address				*/
#define	ENOTBLK	15	/* Block device required		*/
#define	EBUSY	16	/* Mount device busy			*/
#define	EEXIST	17	/* File exists				*/
#define	EXDEV	18	/* Cross-device link			*/
#define	ENODEV	19	/* No such device			*/
#define	ENOTDIR	20	/* Not a directory			*/
#define	EISDIR	21	/* Is a directory			*/
#define	EINVAL	22	/* Invalid argument			*/
#define	ENFILE	23	/* File table overflow			*/
#define	EMFILE	24	/* Too many open files			*/
#define	ENOTTY	25	/* Not a typewriter			*/
#define	ETXTBSY	26	/* Text file busy			*/
#define	EFBIG	27	/* File too large			*/
#define	ENOSPC	28	/* No space left on device		*/
#define	ESPIPE	29	/* Illegal seek				*/
#define	EROFS	30	/* Read only file system		*/
#define	EMLINK	31	/* Too many links			*/
#define	EPIPE	32	/* Broken pipe				*/

/* math library */
#define	EDOM	33	/* Math arg out of domain of func	*/
#define	ERANGE	34	/* Math result not representable	*/

/*
 * networking error numbers (tcp/ip)
 */

/* non-blocking and interrupt i/o */
#define	EWOULDBLOCK	35		/* Operation would block */
#define	EINPROGRESS	36		/* Operation now in progress */
#define	EALREADY	37		/* Operation already in progress */

/* argument errors */
#define	ENOTSOCK	38		/* Socket operation on non-socket */
#define	EDESTADDRREQ	39		/* Destination address required */
#define	EMSGSIZE	40		/* Message too long */
#define	EPROTOTYPE	41		/* Protocol wrong type for socket */
#define	ENOPROTOOPT	42		/* Protocol not available */
#define	EPROTONOSUPPORT	43		/* Protocol not supported */
#define	ESOCKTNOSUPPORT	44		/* Socket type not supported */
#define	EOPNOTSUPP	45		/* Operation not supported on socket */
#define	EPFNOSUPPORT	46		/* Protocol family not supported */
#define	EAFNOSUPPORT	47		/* Address family not supported by protocol family */
#define	EADDRINUSE	48		/* Address already in use */
#define	EADDRNOTAVAIL	49		/* Can't assign requested address */

/* operational errors */
#define	ENETDOWN	50		/* Network is down */
#define	ENETUNREACH	51		/* Network is unreachable */
#define	ENETRESET	52		/* Network dropped connection on reset */
#define	ECONNABORTED	53		/* Software caused connection abort */
#define	ECONNRESET	54		/* Connection reset by peer */
#define	ENOBUFS		55		/* No buffer space available */
#define	EISCONN		56		/* Socket is already connected */
#define	ENOTCONN	57		/* Socket is not connected */
#define	ESHUTDOWN	58		/* Can't send after socket shutdown */
#define	ETOOMANYREFS	59		/* Too many references: can't splice */
#define	ETIMEDOUT	60		/* Connection timed out */
#define	ECONNREFUSED	61		/* Connection refused */

/* random errors */
#define	ELOOP		62		/* Too many levels of symbolic links */
#define	ENAMETOOLONG	63		/* File name too long */
#define	EHOSTDOWN	64		/* Host is down */
#define EHOSTUNREACH	65		/* No route to host */

/* 66 and 69-71 are NFS-related; 67 and 68 are for V.0 ipc */
#define ENOTEMPTY       66              /* Directory not empty */
#define	ENOMSG		67		/* No message of desired type */
#define	EIDRM		68		/* Identifier removed */
#define EDQUOT          69              /* Disc quota exceeded */
#define ESTALE          70              /* Stale NFS file handle */
#define	EREMOTE		71		/* multihop NFS - XXX fix clash */

/* sgi errors - 76 thru 79 are rfu */
#define	EINODETABLEFULL	72		/* inode table is full */
#define	ENOFPA		73		/* no fpa to run program */
#define	EGRBUSY		74		/* graphics already in use */
#define	ENOGR		75		/* no graphics hardware present */
#define	ETEXTABLEFULL	76		/* text table is full */

/* 5.2.1 locking errors */
#define	EDEADLK	80	/* Deadlock condition.			*/
#define	ENOLCK	81	/* No record locks available.		*/

/* stream problems */
#define ENOSTR	82	/* Device not a stream			*/
#define ENODATA	83	/* no data (for no delay io)		*/
#define ETIME	84	/* timer expired			*/
#define ENOSR	85	/* out of streams resources		*/
#define EBADMSG 86	/* trying to read unreadable message	*/


#endif	__ERRNO__
