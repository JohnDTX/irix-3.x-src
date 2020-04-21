# include "pmII.h"
# undef  DEBUG do_debug
# include "dprintf.h"
	extern char do_debug;
# define MAXPUSH 1


PROMSTATIC	short magicused;
PROMSTATIC	short pushedpages[MAXPUSH],pushedprots[MAXPUSH];

int
mapmagic()
{
    register short virtmagic;

    if( magicused >= MAXPUSH )
	panic("MAGIC OVERFLOW");

    virtmagic = VIRT_LAST_MAGIC_PAGE-1-magicused;
    getmap(virtmagic,&pushedpages[magicused],&pushedprots[magicused]);

# ifdef DEBUG
    if( do_debug&04 )
	dprintf((" mapmagic %d [$%x] was [%x %x]"
		,magicused,virtmagic
		,pushedpages[magicused],pushedprots[magicused]));
# endif DEBUG

    magicused++;
    return virtmagic;
}

unmapmagic()
{
    register short virtmagic;

    magicused--;

    if( magicused < 0 )
	panic("MAGIC UNDERFLOW");

    virtmagic = VIRT_LAST_MAGIC_PAGE-1-magicused;
    mapin(virtmagic,pushedpages[magicused],pushedprots[magicused],1);

# ifdef DEBUG
    if(do_debug&04)
	dprintf((" unmapmagic %d [$%x] was [$%x $%x]"
		,magicused,virtmagic
		,pushedpages[magicused],pushedprots[magicused]));
# endif DEBUG
}
