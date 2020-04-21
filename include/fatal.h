/*
 * $Source: /d2/3.7/src/include/RCS/fatal.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:11:22 $
 */

extern	int	Fflags;
extern	char	*Ffile;
extern	int	Fvalue;
extern	int	(*Ffunc)();
extern	int	Fjmp[13];

# define FTLMSG		0100000
# define FTLCLN		 040000
# define FTLFUNC	 020000
# define FTLACT		    077
# define FTLJMP		     02
# define FTLEXIT	     01
# define FTLRET		      0

# define FSAVE(val)	SAVE(Fflags,old_Fflags); Fflags = val;
# define FRSTR()	RSTR(Fflags,old_Fflags);
