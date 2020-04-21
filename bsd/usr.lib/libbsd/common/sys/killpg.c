/*
 * killpg emulation
 *
 * $Source: /d2/3.7/src/bsd/usr.lib/libbsd/common/sys/RCS/killpg.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 15:01:37 $
 */

int
killpg(pid, sig)
	int pid, sig;
{
	return kill(-pid, sig);
}
