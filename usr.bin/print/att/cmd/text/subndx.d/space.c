/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#include	<stdio.h>
#include	"dstructs.h"

#define DEBUGs 0

#define BLOCKMAX 100

struct word *w_malloc ()

{
	static struct word *wblock;
	static int	wcurr = BLOCKMAX;
	unsigned	size;
	char	*malloc();

	if (DEBUGs)    
		printf ("w_alloc: called\n");
	 {
		if (wcurr == BLOCKMAX) {
			size = BLOCKMAX * (sizeof(struct word ));
			wblock = (struct word *) malloc (size);
			if (wblock == NULL)
				fprintf(stderr, "\nw_alloc: out of space");
			wcurr = 0;
		}
		if (DEBUGs)
			printf ("w_alloc: wcurr=%d\n", wcurr);
		wcurr++;
		return (wblock++);
	}
}


struct subj *s_malloc ()

{
	static struct subj *sblock;
	static int	scurr = BLOCKMAX;
	unsigned	size;
	char	*malloc();

	if (DEBUGs)    
		printf ("s_alloc: called\n");
	 {
		if (scurr == BLOCKMAX) {
			size = BLOCKMAX * (sizeof(struct subj ));
			sblock = (struct subj *) malloc (size);
			if (sblock == NULL)
				fprintf(stderr, "\ns_alloc: out of space");
			scurr = 0;
		}
		if (DEBUGs)
			printf ("s_alloc: scurr=%d\n", scurr);
		scurr++;
		return (sblock++);
	}
}


