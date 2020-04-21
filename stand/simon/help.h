/*
* $Source: /d2/3.7/src/stand/simon/RCS/help.h,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:20:46 $
*/

/*
** the struct the holds the help messages for usage strings as well as
** general help.  Actual info is defined in help.c.
*/
struct helps
{
	char	*h_msg;
};

extern struct helps	helps[];
extern struct helps	shelps[];
