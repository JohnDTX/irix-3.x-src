/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#include <stdio.h>
#define ON 1
#define OFF 0
int	i;
int	range = OFF;                                                 /* flag used for printing a range of
                                                                          page numbers */

int	nowsubj = 0, nextsubj = 0, currsect = 0, nextsect = 0, currpage = 0, nextpage = 0;
main(argc, argv)
int	argc;
char	*argv[];
{
	if ((scanf("%d %d %d", &nowsubj, &currsect, &currpage)) != EOF) {
		if (nowsubj != 1) {
			for (i = 1; i < nowsubj; i++)             /* print 0 for any subject not found*/
				printf("0\n");                          /*in document */
		}
		printf("%d]", nowsubj);			         /* print current subject number */
	} else
	 {
		fprintf(stderr, "NO SUBJECTS FOUND:  INDEX EMPTY\n");
		fprintf(stderr, "Please check your formatter options.\n");
		exit(1);
	}
	if (*argv[1] == 'n')	         /* section-page numbering not used */
		procnosect();
	if (*argv[1] == 's')	         /* if section-page numbering is specified */ {
		printf("%d:", currsect);    /* print current section number */
		procsect();
	}
}


/* Given that pages are numbered sequentially, the following subroutine 
   collates the list of page numbers for each subject.  */
procnosect()
{
	while ((scanf("%d %d %d", &nextsubj, &nextsect, &nextpage)) != EOF) {
		if (nowsubj == nextsubj) {
			if (nextpage == currpage)
				;
			else if (nextpage == (currpage + 1)) {
				if (range == OFF) {
					printf("%d-", currpage);  /* print first page number in
					                                                                     range */
					range = ON;
				}
			} else
			 {
				printf("%d,", currpage);  /* print current page number */
				range = OFF;
			}
			currpage = nextpage;      /* update current page number */
		} else
		 {
			printf("%d\n", currpage);  /* print current page number */
			range = OFF;
			for (i = (nowsubj + 1); i < nextsubj; i++) /* print 0 for any subject
				                                                                         not found in document */

				printf("0\n");
			nowsubj = nextsubj;      /* update current subject number */
			currpage = nextpage;     /* update current page number */
			printf("%d]", nowsubj);   /* print current subject number */
		}
	}
	printf("%d\n", currpage);                 /* print current page number */
	exit(0);
}


/* The following subroutine collates the list of page numbers for each 
   subject when section-page numbering is used. */
procsect()
{
	while ((scanf("%d %d %d", &nextsubj, &nextsect, &nextpage)) != EOF) {
		if (nowsubj == nextsubj) {
			if (currsect == nextsect) {
				if (nextpage == currpage)
					;
				else if (nextpage == (currpage + 1)) {
					if (range == OFF) {
						printf("%d-", currpage);  /* print first page number in range */
						range = ON;
					}
				} else
				 {
					printf("%d,", currpage);  /* print current page number */
					range = OFF;
				}
				currpage = nextpage; /* update current page number */
			} else
			 {
				printf("%d; ", currpage);         /* print current page number */
				range = OFF;
				currsect = nextsect;       /* update current section number */
				currpage = nextpage;      /* update current page number */
				printf("%d:", currsect);   /* print current section number */
			}
		} else
		 {
			printf("%d\n", currpage);      /* print current page number */
			range = OFF;
			for (i = (nowsubj + 1); i < nextsubj; i++)/* print 0 for any subject not found
				                                                                        in document */
				printf("0\n");
			nowsubj = nextsubj;      /* update current subject number */
			currsect = nextsect;     /* update current section number */
			currpage = nextpage;     /* update current page number */
			printf("%d]%d:", nowsubj, currsect);       /* print current subject and section
			                                                                    numbers */
		}
	}
	printf("%d\n", currpage);                /* print current page number */
	exit(0);
}


