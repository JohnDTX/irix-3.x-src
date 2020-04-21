# include "toyfs.h"
# include "efs_toyfs.h"

struct toyparams efs_params =
{
    sizeof (struct efs_toyinode),
    BBSHIFT - EFS_INOPBBSHIFT,
    sizeof (struct efs_toyfs),
    BBSIZE,
    SUPERBOFF,
    BBSHIFT,
    "EFS"
};
