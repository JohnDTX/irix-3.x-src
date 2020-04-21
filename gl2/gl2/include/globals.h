#ifndef GLOBALSDEF
#define GLOBALSDEF
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
#include	"gl.h"
#include	"gltypes.h"
#include	"shmem.h"

#define gl_wstatep	(&(gl_shmemptr->ws))

/* Per application state information: */

extern Fontchar gl_defont_offsets[MAXFONTNC];
extern short	gl_temporary_object[300];
extern short    gl_replacemode;
extern objhdr 	*gl_saveopenobjhdr;
extern short 	*gl_savecurrentpos;
extern short 	*gl_savecurrentend;
extern long 	gl_saveopenobj;
extern short 	gl_savereplacemode;
extern short	gl_issaved;
extern short	*gl_currentpos;	/* points to next legal spot in curr. chunk */
extern short	*gl_currentend;	/* points to one past end of current chunk */
extern objhdr	*gl_openobjhdr;
extern Object	gl_openobj;

extern short		gl_cursornum;
extern short 		gl_picksizex, gl_picksizey;
extern short		gl_fbmode;	/* if in feed back */
extern short		gl_picking;	/* if picking */
extern short		gl_pickselect;	/* if selecting or picking */
extern short		*gl_pickbuf;		/* is this right tom? */
extern long		gl_GEbuflen;
extern Matrix 		gl_pickmat;

extern char		*gl_interperrcmd, *gl_interperr;
extern short		gl_interperrlevel, gl_ininterp;
extern hashrec		*gl_hashtable[HASHTABLESIZE];
extern long		gl_objchunksize;
extern short		gl_objsizefrozen;
extern short		gl_hintinited;

/* Global gl state variables: */

extern short		gl_hitbits;
extern short		gl_ginited;
extern char		**gl_memory_list;
extern long		gl_currentshade;
extern int		(*gl_checkspace)(); 
extern int		(*gl_closeobj)(); 
extern int		(*gl_initobj)(); 
extern int		gl_personal_buffer_mode;

#endif GLOBALSDEF
