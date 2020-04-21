/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#include <stdio.h>
#include "cnst.h"

/*given a string, omit checks to see if it is on its list of words and
  returns the integer value TRUE or FALSE*/

omit (word)

char	*word;

#define	 longest  8				/*length of longest omit word*/
{
	int	omt_tot = 11;			/*number of omit words*/
	static char	*omt[] = {  
		"all", 		/*list of omit words*/
		"both",
		"each",
		"etc.",
		"figure",
		"many",
		"section",
		"some",
		"table",
		"these",
		"this"
	};
	char	term[longest];
	int	i, found;
	int	ulcase();			/*changes string to lower case*/

	found = FALSE;
	if (strlen(word) <= longest - 1) {
		strcpy (term, word);
		ulcase (term);
		for (i = 0; i < omt_tot; i++)
			if (strcmp (term, omt[i]) == 0)
				found = TRUE;
	}
	return (found);
}


