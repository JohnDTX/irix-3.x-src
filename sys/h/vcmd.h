/*	vcmd.h	6.1	83/07/29	*/
# ifndef __VCMD__
# define __VCMD__

# ifdef KERNEL
# include "../h/ik_ioctl.h"
# else  KERNEL
# include <sys/ik_ioctl.h>
# endif KERNEL

#define	VPRINT		0100
#define	VPLOT		0200
#define	VPRINTPLOT	0400

#define	VGETSTATE	0
#define	VSETSTATE	1

#define VLF		01
#define VFF		02
#define VREOT		04
#define VEOT		04
# endif __VCMD_
