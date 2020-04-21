/*
 * $Source: /d2/3.7/src/include/RCS/grp.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:11:31 $
 */

struct	group {	/* see getgrent(3) */
	char	*gr_name;
	char	*gr_passwd;
	int	gr_gid;
	char	**gr_mem;
};
