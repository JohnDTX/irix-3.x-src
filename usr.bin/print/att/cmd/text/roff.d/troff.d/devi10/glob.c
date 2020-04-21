/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
 *
 * Definition and initialization of most of the global variables.
 *
 */


#include "gen.h"			/* general purpose definitions */
#include "init.h"			/* printer and system definitions */


char	**argv;				/* global so everyone can use them */
int	argc;

char	*prog_name = "";		/* really just for error messages */

int	x_stat = 0;			/* program exit status */
int	debug = OFF;			/* debug flag */
int	ignore = OFF;			/* what we do with FATAL errors */
long	lineno = 0;			/* really just for post-processor */

char	*fontdir = FONTDIR;		/* troff's binary font table directory */
char	*rastdir = ".";			/* raster files are found right here */

