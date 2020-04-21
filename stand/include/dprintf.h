/*
* $Source: /d2/3.7/src/stand/include/RCS/dprintf.h,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:13:39 $
*/
# ifndef _DPRINTF_

# define _DPRINTF_

# ifdef DEBUG
extern char Debug;
# define dprintf(x)	(Debug?printf x :0)
# define Dprintf(n,x)	(Debug>n?printf x :0)
# else  DEBUG
# define dprintf(x)
# define Dprintf(n,x)
# endif DEBUG

# endif _DPRINTF_
