/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#include <stdio.h>
#define TRUE  1
#define FALSE 0
#define LINELENGTH 256                                           /* max length in chars of each subject */
#define SEP "] "                                                 /* separator string used by strtok */
#define SEP1 "]"                                                 /* separator string used by strtok */
main()
{
	char	str[LINELENGTH];
	char	*p, *strtok();
	char	*args[3];
	int	i, missing = FALSE;
	while (gets(str) != NULL) {
		if ((p = strtok(str, SEP)) != NULL)                /* read past subject number in file */
			;
		for (i = 0; i < 3; i ++) {
			if ((args[i] = strtok(0, SEP1)) == NULL)   /* read until end of string */ {

				missing = TRUE;
				break;
			}
		}
		if (missing)                              /* skip any subjects not found in document */
			missing = FALSE;
		else
			printf("%s ... %s\n", args[0], args[2]);   /* print subject, corresponding page numbers */
	}
	exit(0);
}


