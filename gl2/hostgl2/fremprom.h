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

      integer*4 PESC
      integer*4 PSLOWC
      integer*4 PFASTC
      integer*4 PSTATU
      integer*4 PDOWNL

c codes for dliris and prom communication

      PARAMETER ( PESC =	16 )
      PARAMETER ( PSLOWC =	0 )
      PARAMETER ( PFASTC =	1 )
      PARAMETER ( PSTATU =	2 )
      PARAMETER ( PDOWNL =	3 )
