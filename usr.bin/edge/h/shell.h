#ifndef	SH_STOPPED
/*
 * Interface to shell manager.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/h/RCS/shell.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:05 $
 */
#include "tf.h"
#include "te.h"

typedef	struct {
	termulator *sh_te;		/* termulator to use */
	short	sh_flags;		/* random state flags */
	int	sh_childpid;		/* pid of child process */
	int	sh_ptynum;		/* pty being used */
	int	sh_masterfd;		/* controlling pty fd */
	int	sh_slavefd;		/* slave pty fd */
	char	*sh_master;		/* name of master tty */
	char	*sh_slave;		/* name of slave tty */
} shellframe;

/* sh_flags */
#define	SH_STOPPED	0x0001		/* shell is ^S'd */
#define	SH_NEEDUPDATE	0x0002		/* frame needs updating */
#define	SH_TIMER	0x0004		/* timer pending */
#define	SH_LOGIN	0x0008		/* shell is logged in */

/* interface */
extern	int	shnew();		/* create a new shell */
extern	void	shfree();		/* destroy a shell */
extern	void	shevent();		/* handle shell event */

#endif	/* SH_STOPPED */
