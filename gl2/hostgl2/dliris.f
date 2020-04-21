      PROGRAM DLIRIS
c     Author: RSJ
c     Date:   11/14/83
c     modification history:
c
c     Initials	Date	Purpose
c     --------	----	-------
c     RSJ       4/3/84  Add some miscellaneous documentation
c     RSJ	8/22/84 Get rid of the dependency on the graphics
c                       library, update to the 'c' version of dliris
c     CSK       6/21/85 Brought up to date with dliris.c
c
c     This program is used to download the iris terminal software
c     over a serial line, using this program and the fortran 77
c     library provided by Silicon Graphics Inc.
c
c     WARNING - Don't use this unless you have to, as the download
c     process is painfully long. It's faster to use a floppy boot.
c     If any of the folks with a Unix system are eyeballing this,
c     use the c version instead.
c
c     assumptions:
c           PESC and TESC have the same value in order for gcmd to work
c
c           all input and output is done in 1024 byte chunks
c
      INCLUDE 'fremprom.h'
c     INCLUDE 'irisboot.h'
      integer IBMAGH, IBMAGI, IBLAST
      PARAMETER ( IBMAGH = 263 )
      PARAMETER ( IBMAGI = 449 )
      PARAMETER ( IBLAST = 455 )
      integer bufptr
      character*1024 outbuf
      common /SGINTS/ bufptr
      common /SGCHRS/ outbuf
c
c     local variables
c
c     integer lshift  THIS IS AN INTRINSIC FUNCTION ON UNIX
      integer rdbuf, uichar, inttmp
      integer*2 nbytes
      character*512 buffer

c
c     do the first block of bytes - note that we always do
c     1024 byte transfers
c
c     The file that is read for input is logical unit 6, which
c     typically doesn't need to be opened for input
c
      nbytes = rdbuf( buffer )
      if ( nbytes .eq. 0 ) goto 900
      inttmp = uichar( buffer(4:4) ) 
      inttmp = inttmp + lshift( uichar( buffer(3:3)),  8 )
      inttmp = inttmp + lshift( uichar( buffer(2:2)), 16 )
      inttmp = inttmp + lshift( uichar( buffer(1:1)), 24 )
      if ( inttmp .ne. IBMAGI .and. inttmp .ne. IBMAGH) goto 910
      call gcmd( PDOWNL ) 
c
c     Download this guy!
c
200   call sendby( buffer, nbytes )
      if ( nbytes .lt. 1024 ) goto 500
      nbytes = rdbuf( buffer )
      if ( nbytes .ne. 0 ) goto 200
c
c     Now we're finished ( one way or another )
c
500   call flushg
      goto 2000
c
c     error entries
c
900   write ( 6, 1400 ) 
1400  format( ' dliris: input file is empty' )
      goto 2000
910   write ( 6, 1500 )inttmp 
1500  format(  ' dliris: bad magic number on boot file',I10 )
      goto 2000
2000  continue
      end
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine gcmd( intval )
c     The following are intrinsic functions
c     integer char, and, rshift
      character tmpchr
      integer intval, inttmp
      integer IBLANK
      include 'fremprom.h'
      PARAMETER ( IBLANK = 32 )

      tmpchr = char(PESC)
      call putgch( tmpchr )
c
c     just use the bottom 6 bits
c
      inttmp = and( intval, 63 ) + IBLANK
      tmpchr = char( inttmp )
      call putgch( tmpchr )

c
c     and now the next set of 6 bits
c
      inttmp = rshift( intval, 6 )
      inttmp = and( inttmp, 63 ) + IBLANK
      tmpchr = char( inttmp )
      call putgch( tmpchr )

      return
      end
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      integer function rdbuf( buffer )
      character*(*) buffer
c     character char THIS IS AN INTRINSIC FUNTION IN UNIX
      integer index, nxtchr, rdchar
      integer EOF
      PARAMETER ( EOF = -1 )
c
c     Make sure this is set to the same size as in the routine above
c
c
      do 100 index = 1, 1024
      nxtchr       = rdchar()
      if ( nxtchr .eq. EOF ) goto 200
      buffer( index:index ) = char( nxtchr )
      rdbuf = index
100   continue
c
c     our read is done; either we have read all requested bytes or
c     we have hit end of file
c
200   return
      end
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine sendby( buffer, nbytes )
      character*(*) buffer
      integer   nshort, index, intequ, uichar, k
      integer*2 nbytes, checks, ashort
      equivalence ( ashort, intequ )
c
c     Tell the IRIS how many bytes are coming
c
      call sends(nbytes)
c
c     Figure out how many shorts that is
c
      nshort = (nbytes+1)/2
c
c     Set our starting checksum to zero
c
      checks = 0
      do 50 index = 1, nshort
      k = index * 2
      intequ = lshift( uichar( buffer( k:k ) ), 8 )
      k = k - 1
      intequ = intequ + uichar( buffer( k:k ))
      checks = checks + ashort
      call sends(ashort)
50    continue
      call sends(checks)
      return
      end
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine sends( value )
      integer*2 value
c     Intrinsic functions
c     integer and, rshift, char
      character tmpchr
      integer inttmp
      integer IBLANK
      PARAMETER ( IBLANK = 32 )

      inttmp = and( value, 63 ) + IBLANK
      tmpchr = char( inttmp )
      call putgch( tmpchr )

      inttmp = rshift( value, 6 )
      inttmp = and( inttmp, 63 ) + IBLANK
      tmpchr = char( inttmp )
      call putgch( tmpchr )

      inttmp = rshift( value, 12 )
      inttmp = and( inttmp, 63 ) + IBLANK
      tmpchr = char( inttmp )
      call putgch( tmpchr )

      return
      end
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine putgch( onechr )
      character*1 onechr
      character*1024 outbuf
      integer bufptr
      common /SGINTS/ bufptr
      common /SGCHRS/ outbuf

      outbuf( bufptr:bufptr ) = onechr
      bufptr = bufptr + 1
      if ( bufptr .gt. 1024 )call flushg
      return
      end
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine flushg
      character outbuf
      integer bufptr, k
      common /SGINTS/ bufptr
      common /SGCHRS/ outbuf( 1024 )

      if( bufptr .eq. 1 ) goto 100
      k = bufptr -1
      call wrtbuf( outbuf, k )
      bufptr = 1
100   return
      end
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      integer function uichar( chrval )
c     integer ichar THIS IS AN INTRINSIC FUNCTION UNDER UNIX
      character chrval

      uichar = and( ichar(chrval), 255 )
      return
      end
