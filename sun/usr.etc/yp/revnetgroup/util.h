/*  @(#)util.h 1.1 86/02/05 (C) 1985 Sun Microsystems, Inc. */
/* @(#)util.h	2.1 86/04/16 NFSSRC */ 


#define EOS '\0'

#ifndef NULL 
#	define NULL ((char *) 0)
#endif


#define MALLOC(object_type) ((object_type *) malloc(sizeof(object_type)))

#ifdef sgi
#define FREE(ptr)	(0!=(ptr) ? free((char *) ptr) : 0)
#else
#define FREE(ptr)	free((char *) ptr) 
#endif

#ifndef sgi
#define ALLOCA(object_type) ((object_type *) alloca(sizeof(object_type)))
#endif

#define STRCPY(dst,src) \
	(dst = malloc((unsigned)strlen(src)+1), (void) strcpy(dst,src))

#define STRNCPY(dst,src,num) \
	(dst = malloc((unsigned)(num) + 1),\
	(void)strncpy(dst,src,num),(dst)[num] = EOS) 

extern char *malloc();
extern char *alloca();

char *getline();
void fatal();


