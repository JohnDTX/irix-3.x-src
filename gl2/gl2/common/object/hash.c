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

/*
 * This file contains all of the routines that manage the hash table of
 * objects and tags.
 */
#include "globals.h"

#define hash(obj, tag)	(obj+tag)&(HASHTABLESIZE-1)

gl_freeallobjects()
{
    register hashrec *hashptr, *hashptr1;
    register i;

    for(i = 0; i < HASHTABLESIZE; i++) {
	hashptr = gl_hashtable[i];
	while(hashptr) {
	    if( (hashptr->tag==-1) && ((objhdr *)(hashptr->item))->valid ) {
		delobj(hashptr->obj);
		free(hashptr->item);
	    }
	    hashptr = hashptr->link;
	}
    }
    /* Now clear out all of the hash table headers. */
    for(i = 0; i < HASHTABLESIZE; i++) {
	hashptr = gl_hashtable[i];
	while(hashptr) {
	    hashptr1 = hashptr->link;
	    free(hashptr);
	    hashptr = hashptr1;
	}
	gl_hashtable[i] = 0;
    }
}

/*
 * gl_addhash adds an entry to the hash table.  gl_addhash assumes that the
 * entry is not there already. 0 is returned if the addition is
 * unsuccessful.
 */
gl_addhash(ptr, obj, tag)
short	*ptr;
Object	obj;
Tag	tag;
{
    register hashrec *hashptr;
    register h;

    if(!(hashptr = (hashrec *)malloc(sizeof(hashrec))))
	return 0;
    h = hash(obj, tag);
    hashptr->item = ptr;
    hashptr->link = gl_hashtable[h];
    hashptr->obj = obj;
    hashptr->tag = tag;
    gl_hashtable[h] = hashptr;
    return 1;	/* successful addition */
}

/*
 * gl_removehash removes the hash table entry. 0 is returned if
 * unsuccessful.
 */
gl_removehash(obj, tag)
register Object obj;
register Tag tag;
{
    register hashrec	*hashptr, *prevptr;
    register long	h;

    h = hash(obj, tag);
    if(!(hashptr = gl_hashtable[h]))
	return 0;
    if((hashptr->obj == obj ) && (hashptr->tag == tag)) {
	gl_hashtable[h] = hashptr->link;
	free((char *)hashptr);
	return 1;
    }
    prevptr = hashptr;
    hashptr = hashptr->link;
    while(hashptr) {
	if((hashptr->obj == obj) && (hashptr->tag == tag)) {
	    prevptr->link = hashptr->link;
	    free((char *)hashptr);
	    return 1;
	}
	prevptr = hashptr;
	hashptr = hashptr->link;
    }
    return 0;
}

/*
 * gl_findhash returns 0 if unsuccessful, or the item if successful. 
 */
short *gl_findhash(obj, tag)
register Object	obj;
register Tag	tag;
{
    register hashrec	*hashptr;

    hashptr = gl_hashtable[hash(obj, tag)];
    while(hashptr) {
	if((hashptr->obj == obj) && (hashptr->tag == tag))
	    return hashptr->item;
	hashptr = hashptr->link;
    }
    return 0;
}

/*
 * gl_replacehash returns 0 if unsuccessful, or the item if successful.
 * Its purpose is to change the item at which an object-tag pair
 * points.
 */
gl_replacehash(newptr, obj, tag)
short *newptr;
register Object	obj;
register Tag	tag;
{
    register hashrec	*hashptr;

    hashptr = gl_hashtable[hash(obj, tag)];
    while(hashptr) {
	if((hashptr->obj == obj) && (hashptr->tag == tag)) {
	    hashptr->item = newptr;
	    return 1;
	}
	hashptr = hashptr->link;
    }
    return 0;
}
