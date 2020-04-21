/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * $Source: /d2/3.7/src/sys/streams/RCS/poll.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:34:44 $
 */

/*
 * Structure of file descriptor/event pairs supplied in
 * the poll arrays.
 */
struct pollfd {
	int fd;				/* file desc to poll */
	short events;			/* events of interest on fd */
	short revents;			/* events that occurred on fd */
};

/*
 * Testable select events 
 */
#define POLLIN		01		/* fd is readable */
#define POLLPRI		02		/* priority info at fd */
#define	POLLOUT		04		/* fd is writeable (won't block) */

/*
 * Non-testable poll events (may not be specified in events field,
 * but may be returned in revents field).
 */
#define POLLERR		010		/* fd has error condition */
#define POLLHUP		020		/* fd has been hung up on */
#define POLLNVAL	040		/* invalid pollfd entry */

/*
 * Number of pollfd entries to read in at a time in poll.
 * The larger the value the better the performance, up to the
 * maximum number of open files allowed.  Large numbers will
 * use excessive amounts of kernel stack space.
 */
#define NPOLLFILE	20

