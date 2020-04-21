cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c									 c
c 		 Copyright (C) 1983, Silicon Graphics, Inc.		 c
c									 c
c  These coded instructions, statements, and computer programs  contain  c
c  unpublished  proprietary  information of Silicon Graphics, Inc., and  c
c  are protected by Federal copyright law.  They  may  not be disclosed  c
c  to  third  parties  or copied or duplicated in any form, in whole or  c
c  in part, without the prior written consent of Silicon Graphics, Inc.  c
c									 c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

$	INCLUDE fgl.h

	integer*4 j

	call ginit
	call cursof
	call color(BLACK)
	call clear
	call color(BLUE)

c	move to center of the screen 
	call move2i(XMAXSC/2,YMAXSC/2)

	do 100 j=1,499,5
	    if (mod(j,2).ne.0) goto 200
		call rdr2i(j,0)
		call rdr2i(0,j)
		goto 100
200	    continue
		call rdr2i(-j,0)
		call rdr2i(0,-j)
100	continue

	call gexit
	stop
	end
