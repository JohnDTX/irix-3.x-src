/*
 * $Source: /d2/3.7/src/sys/sys/RCS/lock.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:23 $
 */
#include "../h/param.h"
#include "../h/proc.h"
#include "../h/user.h"
#include "../h/text.h"
#include "../h/lock.h"

lock()
{
	struct a {
		long oper;
	};
	register struct user *up = &u;
	register struct proc *p = up->u_procp;

	if (!suser())
		return;

	switch ((int)(((struct a *)up->u_ap)->oper)) {
	case TXTLOCK:
		if (up->u_lock & (PROCLOCK|TXTLOCK))
			goto bad;
		up->u_lock |= TXTLOCK;
		p->p_flag |= STLOCK;
		break;
	case PROCLOCK:
		if (up->u_lock & (PROCLOCK|TXTLOCK|DATLOCK))
			goto bad;
		up->u_lock |= PROCLOCK|TXTLOCK|DATLOCK;
		p->p_flag |= STLOCK|SDLOCK;
		break;
	case DATLOCK:
		if (up->u_lock & (PROCLOCK|DATLOCK))
			goto bad;
		up->u_lock |= DATLOCK;
		p->p_flag |= SDLOCK;
		break;
	case UNLOCK:
		if ((up->u_lock & (PROCLOCK|TXTLOCK|DATLOCK)) == 0)
			goto bad;
		up->u_lock = 0;
		p->p_flag &= ~(STLOCK|SDLOCK);
		break;

	default:
bad:
		up->u_error = EINVAL;
		break;
	}
}
