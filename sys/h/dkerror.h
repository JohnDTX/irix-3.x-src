/*
 * Generic disk error handling
 *
 * $Source: /d2/3.7/src/sys/h/RCS/dkerror.h,v $
 * $Date: 89/03/27 17:29:19 $
 * $Revision: 1.1 $
 */

struct	dkerror {
	u_char	dke_errornum;
	char	*dke_msg;
};

/* this function searchs the error list for the given error */
char	*dkerror();
