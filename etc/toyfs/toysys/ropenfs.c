# include "toyfs.h"

extern USR U;

# ifdef EFS
extern FS *efs_mount();
# endif EFS

# ifdef BELL
extern FS *bell_mount();
# endif BELL

# ifdef BKY
extern FS *bky_mount();
# endif BKY

extern USR U;

FS *
toy_ropenfs(name)
    char *name;
{
    register FS *sp;

# ifdef EFS
    if( (sp = efs_mount(name)) != 0 )
	return sp;
# endif EFS

    U.u_errmsg = 0;
# ifdef BELL
    if( (sp = bell_mount(name)) != 0 )
	return sp;
# endif BELL

    U.u_errmsg = 0;
# ifdef BKY
    if( (sp = bky_mount(name)) != 0 )
	return sp;
# endif BKY

    return 0;
}
