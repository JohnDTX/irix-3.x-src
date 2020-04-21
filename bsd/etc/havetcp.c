/*
 * Determine if the system supports tcp.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/bsd/etc/RCS/havetcp.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 14:58:22 $
 */
#include "signal.h"

/* ICK - should have an include "tcp.h" that just does this stuff... */
#include "sys/types.h"
#include "sys/socket.h"
#include "net/af.h"
#include "netinet/in.h"

nottcp()
{
	exit(-1);
}

main()
{
	/*
	 * Try to create a socket, catching SIGSYS in case this is a
	 * really old kernel.
	 */
	signal(SIGSYS, nottcp);

	/*
	 * Try to get a socket.  This should always succeed on tcp systems.
	 * For non-tcp systems, it should always fail.
	 */
	if (socket(AF_INET, SOCK_STREAM, 0) < 0)
		exit(-1);
	exit(0);
}
