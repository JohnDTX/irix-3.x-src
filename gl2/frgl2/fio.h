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
c This file and io.f are the two most likely files that will have to
c be edited in order to make the IRIS software work on your machine
c if you are not running VAX 4.2BSD UNIX.
c
c     common constants
c
      INTEGER*4   BYTSH1, BYTSH2, BYTSH3, BYTSH4
      INTEGER*4   DEC   , HIGHP , IBLANK, IBM   , LBYTGR 
      INTEGER*4   LELMT1, LELMT2, LELMT3, LELMT4
      INTEGER*4   LTRUE1, LTRUE2, LTRUE3, LTRUE4
      INTEGER*4   OUTBSZ, SELMT1, SELMT2
c
c OUTBSZ defines the size of the output buffer to the IRIS. There are
c various tradeoffs to be made to make this value correct for your
c installation.  For example, if you are doing highly interactive
c work with the IRIS, and need a real-time response, then you
c will most likely want to make the following value pretty low. If
c you are more interested in doing output to the IRIS, and don't
c really require much real-time response, then you may want to
c make this a pretty big number.
c     The point is that you will probably want to play with this
c a bit to see what is right for you.
c
      PARAMETER   ( OUTBSZ = 150 )
c
c Various commands to the IRIS are different sizes; the following
c defines the largest command size in bytes. You will not want to
c change this parameter.
c
      PARAMETER   ( LBYTGR = 50 )
c
c High water mark to check and see if the buffer is getting full
c
      PARAMETER   ( HIGHP  = OUTBSZ - LBYTGR + 1 )
c
c IBLANK is the ascii value of the blank characer. This is used in
c constructing data to send to the IRIS. No matter what the character
c set is on your machine, you will not want to change this parameter.
c
      PARAMETER   ( IBLANK = 32 )
c
c The following is used to define the byte order of the host machine.
c Definition of host:  only one should be true
c      0 = false, 1 = true
c
c     PARAMETER   ( BRANDX = 1)
      PARAMETER   ( DEC = 1 )
      PARAMETER   ( IBM = 0 )
c
c Define the byte order for integer*4's
c LELMTn = long word element number n
c
      PARAMETER   ( LELMT1 = 4*DEC + 1*IBM )
      PARAMETER   ( LELMT2 = 3*DEC + 2*IBM )
      PARAMETER   ( LELMT3 = 2*DEC + 3*IBM )
      PARAMETER   ( LELMT4 = 1*DEC + 4*IBM )
c
c Define the byte order for integer*2's
c SELMTn = short word element number n
c
      PARAMETER   ( SELMT1 = 2*DEC + 1*IBM )
      PARAMETER   ( SELMT2 = 1*DEC + 2*IBM )
c
c Define the starting bit numbers for each of the bytes 
c
      PARAMETER   ( BYTSH1 = 24*DEC +  0*IBM )
      PARAMETER   ( BYTSH2 = 16*DEC +  8*IBM )
      PARAMETER   ( BYTSH3 =  8*DEC + 16*IBM )
      PARAMETER   ( BYTSH4 =  0*DEC + 24*IBM )
c
c Define the value for TRUE in each byte of a longword
c
      PARAMETER   ( LTRUE1 = 16777216*DEC + 1*IBM       )
      PARAMETER   ( LTRUE2 = 65536*DEC    + 256*IBM     )
      PARAMETER   ( LTRUE3 = 256*DEC      + 65536*IBM   )
      PARAMETER   ( LTRUE4 = 1*DEC        + 16777216*IBM)
c
c     common variables
c
      INTEGER*4   NTINIT, FASTMO, BUFPTR
      CHARACTER*1 OUTBUF
c
c     common definitions
c
c You will not have to include these definitions in your user code,
c as they are used by lib.f and io.f, and already have these
c references 'included'. But do watch out in that if your system
c limits the number of commons available, then you must include
c these in your count.
c
      COMMON / SGINTS / NTINIT, FASTMO, BUFPTR
      COMMON / SGCHRS / OUTBUF(OUTBSZ)
C
C                          NTINIT is a switch to determine NET state
C                                 0 = NET not inited
C                                 1 = NET is inited
C
C                          FASTMO is a switch to determine Command Mode
C                                 0 = Slow mode ( initial mode )
C                                 1 = Fast mode
C
C                          BUFPTR is an index into OUTBUF for next character
C                                 1 <= BUFPTR <= OUTBSZ
C
C                          OUTBUF is the IRIS output buffer
C
C     End common sgraph
C
      SAVE
