#ifndef	ADDRSDEF
#define	ADDRSDEF
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
 * This file describes all the hardware address's used by the graphics
 * system (gl, kgl, etc).  These constants are hardware and kernel
 * dependent
 */

/*
 * XXX this file could be cleaned up quite a bit
 */
#ifdef iris
#ifndef PM2
#define PM2
#endif
#endif

#ifdef juniper
#ifndef IP2
#define IP2
#endif
#endif

#ifdef UNIX
#ifdef PM2
#include "pmII/cpureg.h"
#else
#include "ipII/cpureg.h"
#endif

#define	_MBIO_VBASE	MBIO_VBASE
#define	_SHMEM_VBASE	SHMEM_VBASE
#endif

/*
 * Dc constants
 */

#ifdef V  /* v kernel on pm2 */
#    define DCMBM(x)	    (0x30000 + (x))
#endif V

#ifdef UNIX
#    ifdef KERNEL
#        define DCMBM(x)	(_MBIO_VBASE + (x))
#    else
#	 ifdef PM2
#            define DCMBM(x)	(0xDF4000 + (x))
#	 endif
#	 ifdef IP2
#            define DCMBM(x)	(_MBIO_VBASE + (x))
#	 endif
#    endif KERNEL
#endif UNIX

#ifdef PROMALONE
#    define DCMBM(x)		(0xF70000 + (x))
#endif PROMALONE

/*
 * Gf2 constants
 */
#ifdef V		/* v kernel on pm2 */
#    define GFMBM(x)		(0x30000 + (x))
#    define gewait()	
#endif V

#ifdef UNIX		/* unix stuff */
#    ifdef KERNEL
#        define GFMBM(x)	(_MBIO_VBASE + (x))
#        define gewait()
#    else
#	 ifdef PM2
#            define GFMBM(x)	(0xDF4000 + (x))
#	 endif
#	 ifdef IP2
#            define GFMBM(x)	(_MBIO_VBASE + (x))
#	 endif
#    endif KERNEL
#endif UNIX

#ifdef PROMALONE
#    define GFMBM(x)	(0xF70000 + (x))
#    define gewait()
#endif PROMALONE

#ifdef GLUSER
#    define GFMBM(x)	(0xDF4000 + (x))
#    define gewait()
#endif GLUSER

#ifdef PM2
#    define GEPORT		*(unsigned short *)0xFD5000	/* write-only */
#    define GETOKEN		*(unsigned short *)0xFD4000	/* write-only */
#endif
#ifdef IP2
#    define GEPORT		*(unsigned short *)0x60001000	/* write-only */
#    define GETOKEN		*(unsigned short *)0x60000000	/* write-only */
#endif

/*
 * Uc4 constants
 */
#ifdef V		/* v kernel on pm2 */
#   define UCIOLOWADR	0x30000
#endif		/* unix on pm2 */

#ifdef UNIX
#   ifdef KERNEL
#       define UCIOLOWADR	(_MBIO_VBASE)
#   else
#  	ifdef PM2
#           define UCIOLOWADR	0xDF4000
#     	endif
#       ifdef IP2
#           define UCIOLOWADR	(_MBIO_VBASE)
#       endif
#   endif
#endif
#ifdef PROMALONE
#   define UCIOLOWADR	0xf70000
#endif
#define UCDEV	(0x3000)
#define UCMBM(x)	(UCIOLOWADR + UCDEV + (x))

#ifndef KERNEL
#   ifdef UNIX
#	ifdef PM2
#           define USERSHMEMPTR	0xDFC000
#	endif
#	ifdef IP2
#           define USERSHMEMPTR	0x1fffe000
#	endif
#   endif
#   ifdef V
#       define USERSHMEMPTR	0x2F000
#   endif
#   define gl_shmemptr	((struct shmem *)USERSHMEMPTR)
#endif

/*
 * Gunk for kgl
 */
#ifdef KERNEL
#    ifdef V
#	define USERTOKERNEL(x)	( (int)(x) )
#    else
#	ifdef PM2
#	    define USERTOKERNEL(x)	( ((int)(x))-0xdfc000+_SHMEM_VBASE )
#	endif
#	ifdef IP2
#	    define USERTOKERNEL(x)	( ((int)(x))-0x1fffe000+_SHMEM_VBASE )
#	endif
#    endif
#endif

#endif ADDRSDEF
