/*
 * $Source: /d2/3.7/src/sys/sys/RCS/err.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:08 $
 */
#include "../h/param.h"
#include "../h/user.h"
#include "../h/file.h"
#include "../h/utsname.h"
#include "../h/elog.h"
#include "../h/erec.h"

static	short	logging;

erropen(dev,flg)
{
	if(logging) {
		u.u_error = EBUSY;
		return;
	}
	if((flg&FWRITE) || dev != 0) {
		u.u_error = ENXIO;
		return;
	}
	if(suser()) {
		logstart();
		logging++;
	}
}

/* ARGSUSED */
errclose(dev,flg)
{
	logging = 0;
}

/* ARGSUSED */
errread(dev)
{
	register struct errhdr *eup;
	register n;
	struct errhdr	*geterec();

	if(logging == 0)
		return;
	eup = geterec();
	n = MIN((unsigned)eup->e_len, u.u_count);
	if (copyout((caddr_t)eup, u.u_base, n))
		u.u_error = EFAULT;
	else
		u.u_count -= n;
	freeslot(eup);
}
