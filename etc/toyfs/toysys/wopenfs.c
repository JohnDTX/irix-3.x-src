# include "toyfs.h"

extern USR U;

# ifdef EFS
extern FS *efs_wmount();
# endif EFS

# ifdef BELL
extern FS *bell_wmount();
# endif BELL

# ifdef BKY
extern FS *bky_wmount();
# endif BKY

extern USR U;

FS *
toy_wopenfs(name)
    char *name;
{
    register FS *sp;

# ifdef EFS
    if( (sp = efs_wmount(name)) != 0 )
	return sp;
# endif EFS

    U.u_errmsg = 0;
# ifdef BELL
    if( (sp = bell_wmount(name)) != 0 )
	return sp;
# endif BELL

    U.u_errmsg = 0;
# ifdef BKY
    if( (sp = bky_wmount(name)) != 0 )
	return sp;
# endif BKY

    return 0;
}
