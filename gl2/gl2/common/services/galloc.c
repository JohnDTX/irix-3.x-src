/**************************************************************************
 *									  *
 * 		 Copyright (C) 1984, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/* HPM - Tue Apr 10 16:37:42 PST 1984 */
#include "globals.h"
#include "glerror.h"

char *gl_galloc(n)
int n;
{
    char **gl_temp;

    if((gl_temp = (char **)malloc(n+4))) {
	*gl_temp = (char *)gl_memory_list;
	gl_memory_list = gl_temp;
	return(((char *)(gl_temp)) + 4);
    } else {
	gl_outmem("gl_galloc");
	return 0;
    }
}

char *gl_grealloc(ptr, n)
char	*ptr;
int 	n;
{
    register char	**gl_temp;
    register char   **list_ptr;
    register int	head_of_list;

    /* get the address of the start of the block */
    gl_temp = (char **) (ptr - 4);

    /* now find this in the linked list hung off of gl_memory_list */
    if(gl_temp != gl_memory_list) {
	head_of_list = FALSE;
	list_ptr = gl_memory_list;
	while(gl_temp != (char **)*list_ptr)
	        list_ptr = (char **)*list_ptr;
    } else
	head_of_list = TRUE;
    if(!(gl_temp = (char **)realloc(gl_temp, n+4))) {
	gl_outmem("gl_grealloc");
	return(0);
    }
    if(head_of_list)
	gl_memory_list = gl_temp;
    else
	*list_ptr = (char *)gl_temp;
    return ((char *)gl_temp)+4;
}
