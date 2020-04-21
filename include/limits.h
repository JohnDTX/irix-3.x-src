/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#define	ARG_MAX		10240		/* max length of arguments to exec */
					/* sys/param.h:NCARGS */

#define	CHAR_BIT	8		/* # of bits in a "char" */
					/* values.h:BITSPERBYTE */

#define	CHAR_MAX	127		/* max integer value of a "char" */
#define	CHAR_MIN	-128		/* min integer value of a "char" */

#ifdef PM2
#define	CHILD_MAX	50		/* max # of processes per user id */
					/* pmII/param.h:MAXUPRC */
#endif

#ifdef IP2
#define	CHILD_MAX	100		/* max # of processes per user id */
					/* ipII/param.h:MAXUPRC */
#endif

#define	CLK_TCK		100		/* # of clock ticks per second */

#define	DBL_DIG		16		/* digits of precision of a "double" */
					/* values.h:DSIGNIF */

#define	DBL_MAX		((long float) 1.79769313486231570e+307)
					/* max decimal value of a "double"*/
					/* values.h:MAXDOUBLE */

#define	DBL_MIN		((long float) 4.94065645841246544e-324 
					/* min decimal value of a "double"*/

#define	FCHR_MAX	1073741824	/* max size of a file in bytes */
					/* machine/param.h:CDLIMIT */

#define	FLT_DIG		7		/* digits of precision of a "float" */
					/* values.h:FSIGNIF */

#define	FLT_MAX		((float) 3.402823466385288598e+38)
					/* max decimal value of a "float" */
					/* values.h:MAXFLOAT */

#define	FLT_MIN		((float) 1.40129846432481707e-45)
					/* min decimal value of a "float" */

#define	HUGE_VAL	((float) 3.402823466385288598e+38)
					/* error value returned by Math lib*/
					/* math.h:HUGE */

#define	INT_MAX		2147483647	/* max decimal value of an "int" */
					/* values.h:MAXINT */

#define	INT_MIN		-2147483648	/* min decimal value of an "int" */
					/* values.h:HIBITI */

#define	LINK_MAX	1000		/* max # of links to a single file */
					/* sys/param.h:MAXLINK */

#define	LONG_MAX	2147483647	/* max decimal value of a "long" */
					/* values.h:MAXLONG */

#define	LONG_MIN	-2147483648	/* min decimal value of a "long" */
					/* values.h:HIBITL */

#define	NAME_MAX	14		/* max # of characters in a file name */
					/* sys/dir.h:DIRSIZ */

#define	OPEN_MAX	40		/* max # of open files per process */
					/* sys/param.h:NOFILE */

#define	PASS_MAX	8		/* max # of characters in a password */

#define	PATH_MAX	1024		/* max # of characters in a path name */
					/* sys/nami.h:MAXPATHLEN */

#define	PID_MAX		30000		/* max value for a process ID */
					/* sys/param.h:MAXPID */

#define	PIPE_BUF	10240		/* max # bytes atomic in write to pipe*/
					/* com/pipe_inode.h:PIPESIZE */

#define	PIPE_MAX	10240		/*max # bytes written to pipe in write*/
					/* com/pipe_inode.h:PIPESIZE */

#define	SHRT_MAX	32767		/* max decimal value of a "short" */
					/* values.h:MAXSHORT */

#define	SHRT_MIN	-32767		/* min decimal value of a "short" */
					/* values.h:HIBITS */

#define	STD_BLK		1024		/* # bytes in a physical I/O block */

#define	SYS_NMLN	9		/* # of chars in uname-ret'd strings */

#define	UID_MAX		60000		/* max value for a user or group ID */
					/* sys/param.h:MAXUID */

#define	USI_MAX		4294967296	/* max decimal value of an "unsigned" */

#define	WORD_BIT	32		/* # of bits in a "word" or "int" */

