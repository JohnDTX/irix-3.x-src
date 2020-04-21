/*
 * $Source: /d2/3.7/src/stand/cmd/sifex/RCS/conf.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:13:07 $
 */
#include	"stand.h"

int	_nullsys(),	_nullioctl();
int	_tcpstrategy(),	_tcpopen(),	_tcpclose(),	_tcpioctl();

struct devsw devsw[] = { /* name, flags, strategy, open, close, ioctl	*/
	{ "tcp",	F_NET,		_tcpstrategy,	_tcpopen,
	  _tcpclose,	_tcpioctl	},

	{ 0,		0,		0,		0,
	  0,		0		},
};

struct iob	iobuf[NFILES];
struct buf	bufs[NFILES];
struct inode	inodes[NFILES];
int		errno;
char		Debug;
char		motormouth;
