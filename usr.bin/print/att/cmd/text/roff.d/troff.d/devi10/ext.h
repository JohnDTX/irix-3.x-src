/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
 *
 * External definitions for the variables defined in glob.c. I've also
 * included a few commonly used function declarations just for convenience.
 *
 */


extern char	**argv;			/* global so everyone can use them */
extern int	argc;

extern char	*prog_name;		/* really just for error messages */

extern int	x_stat;			/* program exit status */
extern int	debug;			/* debug flag */
extern int	ignore;			/* what we do with FATAL errors */
extern long	lineno;			/* really just for post-processor */

extern char	*fontdir;		/* troff's binary font table directory */
extern char	*rastdir;		/* Imagen raster table directory */


extern char	*malloc();
extern char	*rastalloc();
extern char	*cuserid();

