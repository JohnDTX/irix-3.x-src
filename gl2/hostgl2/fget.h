
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
C
C Still to be checked:
C     Are the bits in the correct order for gethit?
C
C     definitions for returned values of get routines

      INTEGER*4 NOBUFF
      INTEGER*4 BCKBUF
      INTEGER*4 FRNTBU
      INTEGER*4 BOTHBU

C     values returned by getcmm

      LOGICAL   CMAPMU
      LOGICAL   CMAPON

C     values returned by getdis

      INTEGER*4 DMRGB
      INTEGER*4 DMSING
      INTEGER*4 DMDOUB

C     values returned by getmon

      INTEGER*4 HZ30
      INTEGER*4 HZ60
      INTEGER*4 NTSC
      INTEGER*4 PAL
      INTEGER*4 HZ50
      INTEGER*4 MONA
      INTEGER*4 MONB
      INTEGER*4 MONC
      INTEGER*4 MOND
      INTEGER*4 MONSPE

C     individual hit bits returned by gethit

      INTEGER*4 FARPLA
      INTEGER*4 NEARPL
      INTEGER*4 TOPPLA
      INTEGER*4 BOTTOM
      INTEGER*4 RIGHTP
      INTEGER*4 LEFTPL

C     values returned by getbuf

      PARAMETER ( NOBUFF = 0 )
      PARAMETER ( BCKBUF = 1 )
      PARAMETER ( FRNTBU = 2 )
      PARAMETER ( BOTHBU = 3 )

C     values returned by getcmm

      PARAMETER ( CMAPMU = .FALSE. )
      PARAMETER ( CMAPON = .TRUE.  )

C    values returned by getdis

      PARAMETER ( DMRGB  = 0 )
      PARAMETER ( DMSING = 1 )
      PARAMETER ( DMDOUB = 2 )

C     values returned by getmon

      PARAMETER ( HZ30   = 0 )
      PARAMETER ( HZ60   = 1 )
      PARAMETER ( NTSC   = 2 )
      PARAMETER ( PAL    = 2 )
      PARAMETER ( HZ50   = 3 )
      PARAMETER ( MONA   = 5 )
      PARAMETER ( MONB   = 6 )
      PARAMETER ( MONC   = 7 )
      PARAMETER ( MOND   = 8 )
      PARAMETER ( MONSPE = 32)

C     individual hit bits returned by gethit

      PARAMETER ( FARPLA = 1 )
      PARAMETER ( NEARPL = 2 )
      PARAMETER ( TOPPLA = 4 )
      PARAMETER ( BOTTOM = 8 )
      PARAMETER ( RIGHTP = 16 )
      PARAMETER ( LEFTPL = 32 )
