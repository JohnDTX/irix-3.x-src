# ifndef _DPRINTF_

# define _DPRINTF_

# ifdef DEBUG
extern char DEBUG;
# define dprintf(x)	(DEBUG?printf x:0)
# define ifdebug(x)	(DEBUG?x:0)
# else  DEBUG
# define dprintf(x)
# define ifdebug(x)
# endif DEBUG

# endif _DPRINTF_
