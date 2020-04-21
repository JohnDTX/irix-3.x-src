/*
 * $Source: /d2/3.7/src/include/RCS/pwd.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:11:46 $
 */

struct passwd {
	char	*pw_name;
	char	*pw_passwd;
	int	pw_uid;
	int	pw_gid;
	char	*pw_age;
	char	*pw_comment;
	char	*pw_gecos;
	char	*pw_dir;
	char	*pw_shell;
};

struct comment {
	char	*c_dept;
	char	*c_name;
	char	*c_acct;
	char	*c_bin;
};

struct passwd	*getpwent(/* void */);
struct passwd	*getpwuid(/* int uid */);
struct passwd	*getpwnam(/* char *name */);
void		setpwent(/* void */);
void		endpwent(/* void */);
