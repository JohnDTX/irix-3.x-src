#
# $Source: /d2/3.7/src/stand/lib/RCS/Make.h,v $
# $Revision: 1.1 $
# $Date: 89/03/27 17:14:10 $
#
# The following are general definitions
#

include ${ROOT}/usr/include/make/commondefs

LCDEFS  = -DDEBUG -DTESTING  -DSTAND
LCOPTS	= -O 
LCINCS	= -I../../include -I$(ROOT)/usr/include
I	= $(ROOT)/usr/include
INCL	= ../../include

#
# Rules
#
.c.o:
	$(CCF) -c $<

