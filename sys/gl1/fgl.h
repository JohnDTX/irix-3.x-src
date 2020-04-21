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
c
c	include file "fgl.h"
c
c	this file contains the common set of definitions necessary for use of
c	the graphics library from FORTRAN.  It assumes the program has
c	been linked with the block data routine which initializes the common
c	block for the constants below.
c

      integer*4 XMAXSC, YMAXSC, RETRAC
      integer*2 BLACK, RED, GREEN, YELLOW, BLUE, 
     1   MAGENT, CYAN, WHITE

	  COMMON /GL/ XMAXSC, YMAXSC, RETRAC,
     1  BLACK, RED, GREEN, YELLOW, BLUE, MAGENT, CYAN, WHITE

c
c function declarations
c

      logical getbut, getlsb, getres, isobj, istag
c
      integer*2 gversi, getbuf, getcmm, getcol, getdis, 
     1  getfon, gethei, gethit,
     1  getlst, getlwi, getmap, getobj, getpla, gettex, getval, getwri,
     1  qread, qtest, readpi, readRG
c
      integer*4 strwid, clipli, clippo, endpic, endsel, genobj, gentag
c
	  character keyboa
