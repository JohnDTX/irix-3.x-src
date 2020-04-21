# include	"pwd.h"
# include	"sys/types.h"
# include	"macros.h"

SCCSID(@(#)logname.c	5.2);

char	*logname()
{
	struct passwd *getpwuid();
	struct passwd *log_name;
	int uid;

	uid = getuid();
	log_name = getpwuid(uid);
	endpwent();
	if (! log_name)
		return(0);
	else
		return(log_name->pw_name);
}
