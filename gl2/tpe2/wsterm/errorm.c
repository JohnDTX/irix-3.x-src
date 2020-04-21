/**************************************************************************
 *									  *
 * 		 Copyright (C) 1985, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/*
 * errorm.c - Prints an error message to stderr prefixed with the
 * program name.  Calls _doprnt directly, so errorm() can be given arguments
 * a la fprintf().
 *
 * Usage: errorm(type,s,arg1,arg2,arg3,...);
 *
 * where:
 *	type 	= generally the error type
 *	s    	= fprintf() format string; errorm() appends a '\n\r' (sorry)
 *		  No printing is done if s is NULL.
 *	arg{n}  = remaining arguments to fprintf()
 *
 * Type must be one of:
 *	'i'	  init    ``s'' is now the string which errorm('u') prints,
 *			  the first word of which errorm() takes to be the
 *			  program name.  Three additional arguments, all
 *			  pointers to functions returning ints, must be
 *			  given: If arg1 is not NULL, the 
 *			  funciton it points to (the "extra" function)
 *			  will be called inbetween printing the
 *			  program name and the error message.  If this 
 *			  function is perror(), it is handled specially:
 *			  it is called after printing the error message
 *			  instead of before, and it is only called if
 *			  the type is a capital letter and errno
 *			  (see intro(2)) is non-zero (it then clears errno).
 *			  Another use for this function is to indicate
 *			  line numbers.  If arg2 is not NULL, 
 *			  then errorm() calls of type 'd' are only printed 
 *			  if what this function (the "debug" function)
 *			  is non-zero.  If arg3 is not NULL, the function 
 *			  it points to (the "cleanup" function) will
 *			  be called before exiting.  errorm() returns,
 *			  if it returns at all, the value returned by 
 *			  the extra function, or if arg1 is NULL, the
 *			  value returned by the debug function, or if
 *			  arg2 is NULL or errorm() wasn't called with 
 *			  a type of 'd', zero.  If errorm() exits,
 *			  the exit status is the value returned by the
 *			  cleanup function, or as above if arg3 is NULL,
 *			  except that the exit status is always forced
 *			  to be non-zero.
 *	'f','F'   fatal	  causes exit after printing message; the exit
 *			  status is that returned by the cleanup function,
 *			  but will always be non-zero.  
 *	'a','A'   abort	  as 'f', except exits after dumping core
 *	'w','W'   warning returns to caller
 *	'u'	  usage	  equivalent to type 'f' with ``s'' set to that
 *			  which was passed when errorm() was initialized
 *	'd'	  debug	  equivalent to type 'w' except that if the function
 *			  given to errorm() when it was initialized returns
 *			  zero, no message is printed.
 *
 * Argument rules:
 *   type 
 *   'i'	5 arguments must be given
 *   'u'	1 argument only
 *  others	at least 2 arguments
 *
 * HISTORY
 * Mon Sep 12 12:23:45 1983  Charles (Herb) Kuta  (kuta at Olympus)
 *  - Completely revised.
 */

#include <stdio.h>
#include <errno.h>

static char *usage = NULL, *progname = NULL;
static (*extra)() = NULL, (*debug)() = NULL, (*cleanup)() = NULL;
static int result;
static int extraIsPerror = 0;

extern int perror();
extern int xstat;

/*VARARGS1*/
int 
errorm(type,s,arg1,arg2,args)
char type; 
char *s;
{	
    /* no automatic variables !! */
    result = 0;
    if (type == 'i') {
	/* automatics OK here since not calling _doprnt() */
	register int len;
	register char *cp;

	extra = (int (*)())arg1;
	debug = (int (*)())arg2;
	cleanup = (int (*)())args;

        if (extraIsPerror = (extra == perror))
	    errno = 0;
	if (usage != NULL)
	    free(usage);
	if (progname != NULL)
	    free(progname);
	if (s == NULL) {
	    usage = NULL;
	    progname = NULL;
	}
	else {
	    usage = (char *) malloc(strlen(s)+1);
	    for (cp = s; *cp && *cp != ' '; cp++)
		;
	    len = cp - s + 1;
	    progname = (char *) malloc(len);
	    if (usage == NULL || progname == NULL)
		errorm('f',"out of storage in errorm()");
	    strcpy(usage,s);
	    strncpy(progname,s,len);
	    progname[len-1] = '\0';
	}
    	return result;
    } 
    else if (type == 'd' 
    		&& (debug == NULL 
			|| ( debug != NULL && !(result = (*debug)()) )))
	/* just return */ ;
    else {
	fflush(stdout);	  /* maintain order in case stderr is dup of stdout */
	if (type == 'u') {
	    if (usage)
		fprintf(stderr,"Usage: %s\n\r",usage);
	}
	else if (s && *s) {
	    int needNL = 0;

	    if (progname != NULL) {
		fprintf(stderr,"%s: ",progname);
		needNL++;
	    }
	    if (!extraIsPerror && extra != NULL) {
		result = (*extra)();
		needNL++;	/* this is most likely correct */
	    }
	    if (s && *s) {
	    	_doprnt(s,&arg1,stderr);
		needNL++;
	    }
	    if (extraIsPerror && errno && type >= 'A' && type <= 'Z') {
	    	if (needNL)
		    fprintf(stderr,": ");
		perror("");
		fprintf(stderr, "\r");
		errno = 0;
		needNL = 0;
	    }
	    if (needNL)
		fprintf(stderr,"\n\r");
	}
	fflush(stderr);		/* just in case buffered */
	switch (type) {
	    case 'd':
	    case 'w':	
	    case 'W':	
		break;
	    case 'a':
	    case 'A':
	    case 'u':
	    case 'f':	
	    case 'F':	
		xstat = 1;
		if (cleanup != NULL)
		    result = (*cleanup)(0);
		fflush(stdout);		/* just in case */
		fflush(stderr);
		if (type == 'a')
		    abort();
		else {
		    if (!result)	/* force a non-zero exit status */
		    	result = 1;
	    	    exit(result);
		}
	    default:	
	    	errorm('w',"errorm: unknown error type: %c",type);
		break;
	}
    }
    return result;
}
