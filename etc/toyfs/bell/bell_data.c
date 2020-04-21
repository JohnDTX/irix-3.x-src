# include "toyfs.h"
# include "bell_toyfs.h"

struct toyparams bell_params =
{
    sizeof (struct bell_toyinode),
    6,
    sizeof (struct bell_toyfs),
    512,
    SUPERBOFF,
    BSHIFT,
    "BELL"
};
