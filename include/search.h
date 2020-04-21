/*
 * $Source: /d2/3.7/src/include/RCS/search.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:11:52 $
 */

/* HSEARCH(3C) */
typedef struct entry { char *key, *data; } ENTRY;
typedef enum { FIND, ENTER } ACTION;

/* TSEARCH(3C) */
typedef enum { preorder, postorder, endorder, leaf } VISIT;
