/*
 * $Source: /d2/3.7/src/sys/h/RCS/utsname.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:30:20 $
 */
struct utsname {
	char	sysname[9];
	char	nodename[9];
	char	release[9];
	char	version[9];
	char	machine[9];
};

#ifdef	KERNEL
extern	struct utsname utsname;
#endif
