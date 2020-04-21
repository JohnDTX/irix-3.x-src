
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

C     graphics libary header file

C     maximum X and Y screen coordinates

      INTEGER*4 XMAXSC
      INTEGER*4 YMAXSC

C     various hardware/software limits

      INTEGER*4 ATTRIB
      INTEGER*4 VPSTAC
      INTEGER*4 MATRIX
      INTEGER*4 STARTT
      INTEGER*4 ENDTAG
    
C     names for colors in color map loaded by ginit()

      INTEGER*4 BLACK
      INTEGER*4 RED
      INTEGER*4 GREEN
      INTEGER*4 YELLOW
      INTEGER*4 BLUE
      INTEGER*4 MAGENT
      INTEGER*4 CYAN
      INTEGER*4 WHITE

C     function return values

      INTEGER*4	BLKQRE
      INTEGER*4	ENDFEE
      INTEGER*4	ENDPIC
      INTEGER*4	ENDSEL
      INTEGER*4	GENOBJ
      INTEGER*4	GENTAG
      INTEGER*4	GETBUF
      LOGICAL  	GETBUT
      LOGICAL  	GETCMM
      INTEGER*4	GETCOL
      LOGICAL  	GETDCM
      INTEGER*4	GETDIS
      INTEGER*4	GETFON
      INTEGER*4	GETHEI
      INTEGER*4	GETHIT
      LOGICAL  	GETLSB
      INTEGER*4	GETLSR
      INTEGER*4	GETLST
      INTEGER*4	GETLWI
      INTEGER*4	GETMAP
      INTEGER*4	GETMEM
      INTEGER*4	GETMON
      INTEGER*4	GETOPE
      INTEGER*4	GETPAT
      INTEGER*4	GETPLA
      LOGICAL  	GETRES
      INTEGER*4	GETVAL
      INTEGER*4	GETWRI
      LOGICAL  	GETZBU
      INTEGER*4 GVERSI
      LOGICAL  	ISOBJ
      LOGICAL  	ISTAG
      INTEGER*4	QREAD
      INTEGER*4	QTEST
      INTEGER*4	READPI
      INTEGER*4	READRG
      INTEGER*4	STRWID
      LOGICAL   SETFAS
      LOGICAL   SETSLO

C     maximum X and Y screen coordinates

      PARAMETER (XMAXSC = 1023)
      PARAMETER (YMAXSC = 767)

C     various hardware/software limits

      PARAMETER (ATTRIB = 10)
      PARAMETER (VPSTAC = 8)
      PARAMETER (MATRIX = 32)
      PARAMETER (STARTT = -2)
      PARAMETER (ENDTAG	= -3)

C     names for colors in color map loaded by ginit()

      PARAMETER (BLACK = 0)
      PARAMETER (RED   = 1)
      PARAMETER (GREEN = 2)
      PARAMETER (YELLOW= 3)
      PARAMETER (BLUE  = 4)
      PARAMETER (MAGENT= 5)
      PARAMETER (CYAN  = 6)
      PARAMETER (WHITE = 7)

