/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/conf.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:20 $
 */
#include	"stand.h"

int	_nullsys(),	_nullioctl();
int	_xnsstrategy(),	_xnsopen(),	_xnsclose();
int	_tcpstrategy(),	_tcpopen(),	_tcpclose(),	_tcpioctl();
int	hdopen();
int	mdstrategy(),	mdopen(),	mdclose(),	mdioctl();
int	stdstrategy(),	stdopen(),	stdclose(),	stdioctl();
int	ipstrategy(),	ipopen(),	ipclose(),	ipioctl();
int	ctopen();
int	mtstrategy(),	mtopen(),	mtclose(),	mtioctl();
int	ststrategy(),	stopen(),	stclose(),	stioctl();
int	fdopen();
int	mfstrategy(),	mfopen(),	mfclose(),	mfioctl();
int	stfstrategy(),	stfopen(),	stfclose(),	stfioctl();
int	romstrategy(),	romopen(),	romclose();

struct devsw devsw[] = { /* name, flags, strategy, open, close, ioctl	*/
	{ "xns",	F_NET,		_xnsstrategy,	_xnsopen,
	  _xnsclose,	_nullsys	},
	{ "tcp",	F_NET,		_tcpstrategy,	_tcpopen,
	  _tcpclose,	_tcpioctl	},
	{ "hd",		F_DISK,		_nullsys,	hdopen,
	  _nullsys,	_nullsys	},
	{ "ip",		F_DISK,		ipstrategy,	ipopen,
	  ipclose,	ipioctl	},
	{ "sd",		F_DISK,		stdstrategy,	stdopen,
	  stdclose,	stdioctl	},
	{ "si",		F_DISK,		stdstrategy,	stdopen,
	  stdclose,	stdioctl	},
	{ "md",		F_DISK,		mdstrategy,	mdopen,
	  mdclose,	mdioctl	},
	{ "ct",		F_TAPE,		_nullsys,	ctopen,
	  mtclose,	mtioctl	},
	{ "mt",		F_TAPE,		mtstrategy,	mtopen,
	  mtclose,	mtioctl	},
	{ "mq",		F_TAPE,		mtstrategy,	mtopen,
	  mtclose,	mtioctl	},
	{ "st",		F_TAPE,		ststrategy,	stopen,
	  stclose,	stioctl	},
	{ "sq",		F_TAPE,		ststrategy,	stopen,
	  stclose,	stioctl	},
	{ "fd",		F_DISK,		_nullsys,	fdopen,
	  _nullsys,	_nullsys	},
	{ "sf",		F_DISK,		stfstrategy,	stfopen,
	  stdclose,	stdioctl	},
	{ "mf",		F_DISK,		mfstrategy,	mfopen,
	  mfclose,	mfioctl	},
	{ "rom",	F_PROM,		romstrategy,	romopen,
	  romclose,	_nullsys	},

	{ 0,		0,		0,		0,
	  0,		0		},
};

struct iob	iobuf[NFILES];
struct buf	bufs[NFILES];
struct inode	inodes[NFILES];
int		errno;
char		Debug;
char		motormouth;
