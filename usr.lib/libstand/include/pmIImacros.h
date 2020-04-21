# ifndef _PM2MACROS_

# define _PM2MACROS_

#define	atop(x) ((x)>>12)
#define ptoa(x) ((x)<<12)

/*
# define bzero(t,n)	clear(t,n)
 */
# define bcopy(s,t,n)	blt(t,s,n)
# define busfix(x)	((int)(x)^01)

# define PROMSTATIC

# define MAXBSIZE	(2*DBLOCK)
# define DBLOCK		(1<<DBSHIFT)
# define DBSHIFT	9

# endif  _PM2MACROS_
