/*
 * $Source: /d2/3.7/src/sys/sys/RCS/tty_sys.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:46 $
 */
#include "../h/param.h"
#include "../h/inode.h"
#include "../h/user.h"
#include "../h/conf.h"
#include "../h/proc.h"

/*
 * indirect driver for controlling tty.
 */

#define STR_TTY() (cdevsw[major(u.u_ttyd)].d_str)
#define USE_CDEV(op) (*cdevsw[major(u.u_ttyd)].op)

/* ARGSUSED */
syopen(dev, flag)
dev_t dev;
int flag;
{
	if (sycheck()) {
		if (STR_TTY()) {
			if (!u.u_ttyip) {
				u.u_error = ENXIO;
				return;
			}
			stropen(u.u_ttyip, flag);
		} else {
			USE_CDEV(d_open)(u.u_ttyd, flag);
		}
	}
}

/* ARGSUSED */
syread(dev)
dev_t dev;
{
	if (sycheck()) {
		if (STR_TTY())
			strread(u.u_ttyip);
		else
			USE_CDEV(d_read)(u.u_ttyd);
	}
}

/* ARGSUSED */
sywrite(dev)
dev_t dev;
{
	if (sycheck()) {
		if (STR_TTY())
			strwrite(u.u_ttyip);
		else
			USE_CDEV(d_write)(u.u_ttyd);
	}
}

/* ARGSUSED */
syioctl(dev, cmd, arg, mode)
dev_t dev;
int cmd, arg;
int mode;
{
	if (sycheck()) {
		if (STR_TTY())
			strioctl(u.u_ttyip, cmd, arg, mode);
		else
			USE_CDEV(d_ioctl)(u.u_ttyd, cmd, arg, mode);
	}
}

sycheck()
{
	if (u.u_ttyp == NULL) {
		u.u_error = ENXIO;
		return(0);
	}
	if (*u.u_ttyp != u.u_procp->p_pgrp) {
		u.u_error = EIO;
		return(0);
	}

	return(1);			/* all is well */
}
