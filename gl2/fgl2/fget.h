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

      integer*4 NOBUFF
      integer*4 BCKBUF
      integer*4 FRNTBU
      integer*4 BOTHBU
      integer*4 CMAPMU
      integer*4 CMAPON
      integer*4 DMRGB
      integer*4 DMSING
      integer*4 DMDOUB
      integer*4 HZ30
      integer*4 HZ60
      integer*4 NTSC
      integer*4 PAL
      integer*4 HZ50
      integer*4 MONA
      integer*4 MONB
      integer*4 MONC
      integer*4 MOND
      integer*4 MONSPE
      integer*4 FARPLA
      integer*4 NEARPL
      integer*4 TOPPLA
      integer*4 BOTTOM
      integer*4 RIGHTP
      integer*4 LEFTPL

c include file containing definitions for returned values of get* routines


c values returned by getbuffer()

      PARAMETER ( NOBUFF =	0 )
      PARAMETER ( BCKBUF =	1 )
      PARAMETER ( FRNTBU =	2 )
      PARAMETER ( BOTHBU =	3 )

c values returned by getcmmode()

      PARAMETER ( CMAPMU =	0 )
      PARAMETER ( CMAPON =	1 )

c values returned by getdisplaymode()

      PARAMETER ( DMRGB =	0 )
      PARAMETER ( DMSING =	1 )
      PARAMETER ( DMDOUB =	2 )

c values returned by getmonitor()

      PARAMETER ( HZ30	=	0 )
      PARAMETER ( HZ60	=	1 )
      PARAMETER ( NTSC	=	2 )
      PARAMETER ( PAL	=	2 )
      PARAMETER ( HZ50	=	3 )
      PARAMETER ( MONA	=	5 )
      PARAMETER ( MONB	=	6 )
      PARAMETER ( MONC	=	7 )
      PARAMETER ( MOND	=	8 )
      PARAMETER ( MONSPE =	32 )

c individual hit bits returned by gethitcode()

      PARAMETER ( FARPLA =	1 )
      PARAMETER ( NEARPL =	2 )
      PARAMETER ( TOPPLA =	4 )
      PARAMETER ( BOTTOM =	8 )
      PARAMETER ( RIGHTP =	16 )
      PARAMETER ( LEFTPL =	32 )
