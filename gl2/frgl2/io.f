c PLEASE READ THE FOLLOWING INFORMATION
c
c 	This is the generic "brand-x" release of the FORTRAN Remote Graphics
c Library for the IRIS terminal modified to run with the f77 compiler
c supplied with VAX 4.2BSD UNIX.  IT IS (almost) GUARANTEED NOT TO RUN ON
c YOUR SYSTEM IF YOU ARE NOT RUNNING 4.2 UNIX.  You must make changes (at
c least) in this file and in the file fio.h to get the IRIS up and
c running on your system.  The routines that will most likely cause you
c problems are found in the front of this file, and are bounded by the
c strings 'c*******'.  You may need to also change something in the file
c lib.f, but that is much more unlikely.
c 
c 	The three files: fio.h, io.f, and lib.f make up the Remote Graphics
c Library.  In this 4.2BSD UNIX implementation, there is another file:
c fsys.c.  It contains several primitive routines written in C which
c could not be coded in FORTRAN.  You will most likely want to make an
c object module library with these files for your users. There are also
c some include files, which have the filetype ".h", that you will want to
c make available to your users to include with their applications
c programs.
c 
c 	These routines were written with several different compilers in
c mind; namely the FORTRAN-77 type compiler for VMS and UNIX, and the
c FORTRAN g compiler for the IBM world. In attempting to somewhat satisfy
c all worlds, the code may not be the prettiest in alot of places.
c
c
c    Variable type description:
c Integer    is an integer*4 ( 4 bytes);
c Logical    is a  4 byte quantity;
c Real       is a  4 byte quantity;
c
c
c BRIEF DESCRIPTION OF THE PHILOSOPHY BEHIND THE I/O LIBRARY
c
c           HOST COMPUTER
c
c +-------------------------------------+
c |	user's application program	|
c +-------------------------------------+
c |		lib.f			| set of auto-generated stubs 
c +-------------------------------------+
c |		io.f			| routines that really do i/o
c |            fsys.c (4.2 BSD only)    |
c +-------------------------------------+
c                 ^
c                 |
c                 |                       serial line
c                 |
c                 |
c                 v
c           IRIS TERMINAL
c        +--------+----------+
c        | Graphics software |
c        +-------------------+
c
c
c	The user's application program calls routines found in lib.f,
c which in turn call routines in io.f.  (Since the routines in io.f
c have to do alot of bit manipulation, real-time considerations would
c be better served by using something in place of Fortran. But you
c would most likely do this after getting the Fortran working.)
c	Commands that are to be passed to the IRIS are buffered. When
c the buffer gets full; or when flushg,gflush,gexit,greset,gfinis,ginit
c are called; and occasionally when recos,recbs,recfs,recls,recss are
c called, the buffer is emptied. All tokens are passed as longwords
c (32 bits), so that if a character string, for example, is not an even
c multiple of four, extra bytes are tacked onto the end when it is put
c into the buffer. These extra bytes are ignored when the IRIS receives
c them, as a byte count is also passed.
c	There are two transmission modes: fastmode and slowmode.
c Fastmode sends 8 bit quantities ( high byte to low byte ), while
c slowmode sends 6 bit quantities starting with the low byte. Note
c that slowmode also adds the ascii value of "blank" to the 6 bit
c entity, so as to make it a printable ( and hence traceable )
c character. See the routine i4trns for details.
c       The slowmode is used for serial line connections, which is
c most likely the one you are using. The fastmode is for ethernet,
c or somesuch bit-blasting device. For the slow mode, we send
c 8 bits at a time (no parity); but since all characters are printable,
c you should be able to send everything in 7 bit mode with no problem.
c********************************************************************
c DESCRIPTION
c
c     The IRIS is passed character strings in ascii. If your machine
c uses something else, then you will want a translation table to go
c from your character bit formations to the standard ascii. The
c following is an illustration, and is not valid for any machine.
c Note also that you will want to include the common ebc2as in your
c user code.
c If your machine uses ascii, you will not want to use this.
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c     purpose:  contains block data to initialize ebcdic --> ascii
c               translation table.
c
c     assumptions:
c               user has specified named common /ebc2as/ ibcdic(256) in his
c               code to receive this block data. 
c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c     block data
c     integer ibcdic
c     common /ebc2as/ ibcdic(256)
c
c     data ibcdic(  1) /  0/
c     data ibcdic(  2) /  1/
c     data ibcdic(  3) /  2/
c     data ibcdic(  4) /  3/
c     data ibcdic(  5) /  0/
c     data ibcdic(  6) /  9/
c     data ibcdic(  7) /  0/
c     data ibcdic(  8) /127/
c     data ibcdic(  9) /  0/
c     data ibcdic( 10) /  0/
c     data ibcdic( 11) /  0/
c     data ibcdic( 12) /  1/
c     data ibcdic( 13) / 12/
c     data ibcdic( 14) / 13/
c     data ibcdic( 15) / 14/
c     data ibcdic( 16) / 15/
c     data ibcdic( 17) / 16/
c     data ibcdic( 18) / 17/
c     data ibcdic( 19) / 18/
c     data ibcdic( 20) /  0/
c     data ibcdic( 21) /  0/
c     data ibcdic( 22) / 10/
c     data ibcdic( 23) /  8/
c     data ibcdic( 24) /  0/
c     data ibcdic( 25) / 24/
c     data ibcdic( 26) / 25/
c     data ibcdic( 27) /  0/
c     data ibcdic( 28) /  0/
c     data ibcdic( 29) / 28/
c     data ibcdic( 30) / 29/
c     data ibcdic( 31) / 30/
c     data ibcdic( 32) / 31/
c     data ibcdic( 33) /  0/
c     data ibcdic( 34) /  0/
c     data ibcdic( 35) /  0/
c     data ibcdic( 36) /  0/
c     data ibcdic( 37) /  0/
c     data ibcdic( 38) / 10/
c     data ibcdic( 39) / 23/
c     data ibcdic( 40) / 27/
c     data ibcdic( 41) /  0/
c     data ibcdic( 42) /  0/
c     data ibcdic( 43) /  0/
c     data ibcdic( 44) /  0/
c     data ibcdic( 45) /  0/
c     data ibcdic( 46) /  5/
c     data ibcdic( 47) /  6/
c     data ibcdic( 48) /  7/
c     data ibcdic( 49) /  0/
c     data ibcdic( 50) /  0/
c     data ibcdic( 51) / 22/
c     data ibcdic( 52) /  0/
c     data ibcdic( 53) /  0/
c     data ibcdic( 54) /  0/
c     data ibcdic( 55) /  0/
c     data ibcdic( 56) /  4/
c     data ibcdic( 57) /  0/
c     data ibcdic( 58) /  0/
c     data ibcdic( 59) /  0/
c     data ibcdic( 60) /  0/
c     data ibcdic( 61) / 20/
c     data ibcdic( 62) / 21/
c     data ibcdic( 63) /  0/
c     data ibcdic( 64) / 26/
c     data ibcdic( 65) / 32/
c     data ibcdic( 66) /  0/
c     data ibcdic( 67) /  0/
c     data ibcdic( 68) /  0/
c     data ibcdic( 69) /  0/
c     data ibcdic( 70) /  0/
c     data ibcdic( 71) /  0/
c     data ibcdic( 72) /  0/
c     data ibcdic( 73) /  0/
c     data ibcdic( 74) /  0/
c     data ibcdic( 75) /  0/
c     data ibcdic( 76) / 46/
c     data ibcdic( 77) / 60/
c     data ibcdic( 78) / 40/
c     data ibcdic( 79) / 43/
c     data ibcdic( 80) /124/
c     data ibcdic( 81) / 38/
c     data ibcdic( 82) /  0/
c     data ibcdic( 83) /  0/
c     data ibcdic( 84) /  0/
c     data ibcdic( 85) /  0/
c     data ibcdic( 86) /  0/
c     data ibcdic( 87) /  0/
c     data ibcdic( 88) /  0/
c     data ibcdic( 89) /  0/
c     data ibcdic( 90) /  0/
c     data ibcdic( 91) / 33/
c     data ibcdic( 92) / 36/
c     data ibcdic( 93) / 42/
c     data ibcdic( 94) / 41/
c     data ibcdic( 95) / 59/
c     data ibcdic( 96) / 94/
c     data ibcdic( 97) / 45/
c     data ibcdic( 98) / 47/
c     data ibcdic( 99) /  0/
c     data ibcdic(100) /  0/
c     data ibcdic(101) /  0/
c     data ibcdic(102) /  0/
c     data ibcdic(103) /  0/
c     data ibcdic(104) /  0/
c     data ibcdic(105) /  0/
c     data ibcdic(106) /  0/
c     data ibcdic(107) /124/
c     data ibcdic(108) / 44/
c     data ibcdic(109) / 37/
c     data ibcdic(110) / 95/
c     data ibcdic(111) / 62/
c     data ibcdic(112) / 63/
c     data ibcdic(113) /  0/
c     data ibcdic(114) /  0/
c     data ibcdic(115) /  0/
c     data ibcdic(116) /  0/
c     data ibcdic(117) /  0/
c     data ibcdic(118) /  0/
c     data ibcdic(119) /  0/
c     data ibcdic(120) /  0/
c     data ibcdic(121) /  0/
c     data ibcdic(122) / 96/
c     data ibcdic(123) / 58/
c     data ibcdic(124) / 35/
c     data ibcdic(125) / 64/
c     data ibcdic(126) / 39/
c     data ibcdic(127) / 61/
c     data ibcdic(128) / 34/
c     data ibcdic(129) /  0/
c     data ibcdic(130) / 97/
c     data ibcdic(131) / 98/
c     data ibcdic(132) / 99/
c     data ibcdic(133) /100/
c     data ibcdic(134) /101/
c     data ibcdic(135) /102/
c     data ibcdic(136) /103/
c     data ibcdic(137) /104/
c     data ibcdic(138) /105/
c     data ibcdic(139) /  0/
c     data ibcdic(140) /  0/
c     data ibcdic(141) /  0/
c     data ibcdic(142) /  0/
c     data ibcdic(143) /  0/
c     data ibcdic(144) /  0/
c     data ibcdic(145) /  0/
c     data ibcdic(146) /106/
c     data ibcdic(147) /107/
c     data ibcdic(148) /108/
c     data ibcdic(149) /109/
c     data ibcdic(150) /110/
c     data ibcdic(151) /111/
c     data ibcdic(152) /112/
c     data ibcdic(153) /113/
c     data ibcdic(154) /114/
c     data ibcdic(155) /  0/
c     data ibcdic(156) /  0/
c     data ibcdic(157) /  0/
c     data ibcdic(158) /  0/
c     data ibcdic(159) /  0/
c     data ibcdic(160) /  0/
c     data ibcdic(161) /  0/
c     data ibcdic(162) /126/
c     data ibcdic(163) /115/
c     data ibcdic(164) /116/
c     data ibcdic(165) /117/
c     data ibcdic(166) /118/
c     data ibcdic(167) /119/
c     data ibcdic(168) /120/
c     data ibcdic(169) /121/
c     data ibcdic(170) /122/
c     data ibcdic(171) /  0/
c     data ibcdic(172) /  0/
c     data ibcdic(173) /  0/
c     data ibcdic(174) /  0/
c     data ibcdic(175) /  0/
c     data ibcdic(176) /  0/
c     data ibcdic(177) /  0/
c     data ibcdic(178) /  0/
c     data ibcdic(179) /  0/
c     data ibcdic(180) /  0/
c     data ibcdic(181) /  0/
c     data ibcdic(182) /  0/
c     data ibcdic(183) /  0/
c     data ibcdic(184) /  0/
c     data ibcdic(185) /  0/
c     data ibcdic(186) /  0/
c     data ibcdic(187) /  0/
c     data ibcdic(188) /  0/
c     data ibcdic(189) /  0/
c     data ibcdic(190) /  0/
c     data ibcdic(191) /  0/
c     data ibcdic(192) /  0/
c     data ibcdic(193) /123/
c     data ibcdic(194) / 65/
c     data ibcdic(195) / 66/
c     data ibcdic(196) / 67/
c     data ibcdic(197) / 68/
c     data ibcdic(198) / 69/
c     data ibcdic(199) / 70/
c     data ibcdic(200) / 71/
c     data ibcdic(201) / 72/
c     data ibcdic(202) / 73/
c     data ibcdic(203) /  0/
c     data ibcdic(204) /  0/
c     data ibcdic(205) /  0/
c     data ibcdic(206) /  0/
c     data ibcdic(207) /  0/
c     data ibcdic(208) /  0/
c     data ibcdic(209) /125/
c     data ibcdic(210) / 74/
c     data ibcdic(211) / 75/
c     data ibcdic(212) / 76/
c     data ibcdic(213) / 77/
c     data ibcdic(214) / 78/
c     data ibcdic(215) / 79/
c     data ibcdic(216) / 80/
c     data ibcdic(217) / 81/
c     data ibcdic(218) / 82/
c     data ibcdic(219) /  0/
c     data ibcdic(220) /  0/
c     data ibcdic(221) /  0/
c     data ibcdic(222) /  0/
c     data ibcdic(223) /  0/
c     data ibcdic(224) /  0/
c     data ibcdic(225) / 92/
c     data ibcdic(226) /  0/
c     data ibcdic(227) / 83/
c     data ibcdic(228) / 84/
c     data ibcdic(229) / 85/
c     data ibcdic(230) / 86/
c     data ibcdic(231) / 87/
c     data ibcdic(232) / 88/
c     data ibcdic(233) / 89/
c     data ibcdic(234) / 90/
c     data ibcdic(235) /  0/
c     data ibcdic(236) /  0/
c     data ibcdic(237) /  0/
c     data ibcdic(238) /  0/
c     data ibcdic(239) /  0/
c     data ibcdic(240) /  0/
c     data ibcdic(241) / 48/
c     data ibcdic(242) / 49/
c     data ibcdic(243) / 50/
c     data ibcdic(244) / 51/
c     data ibcdic(245) / 52/
c     data ibcdic(246) / 53/
c     data ibcdic(247) / 54/
c     data ibcdic(248) / 55/
c     data ibcdic(249) / 56/
c     data ibcdic(250) / 57/
c     data ibcdic(251) /124/
c     data ibcdic(252) /  0/
c     data ibcdic(253) /  0/
c     data ibcdic(254) /  0/
c     data ibcdic(255) /  0/
c     data ibcdic(256) /  0/
c
c    the above ibcdic def's are equivalent to the following byte values
c    You will probably want to make a jump table to index into the above
c    values.
c    0/  0,  1,  2,  3,  0,  9,  0,127,  0,  0,  0, 11, 12, 13, 14, 15,
c    1  16, 17, 18,  0,  0, 10,  8,  0, 24, 25,  0,  0, 28, 29, 30, 31,
c    2   0,  0,  0,  0,  0, 10, 23, 27,  0,  0,  0,  0,  0,  5,  6,  7,
c    3   0,  0, 22,  0,  0,  0,  0,  4,  0,  0,  0,  0, 20, 21,  0, 26,
c    4  32,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 46, 60, 40, 43,124,
c    5  38,  0,  0,  0,  0,  0,  0,  0,  0,  0, 33, 36, 42, 41, 59,126,
c    6  45, 47,  0,  0,  0,  0,  0,  0,  0,  0,124, 44, 37, 95, 62, 63,
c    7   0,  0,  0,  0,  0,  0,  0,  0,  0, 96, 58, 35, 64, 39, 61, 34,
c    8   0, 97, 98, 99,100,101,102,103,104,105,  0,  0,  0,  0,  0,  0,
c    9   0,106,107,108,109,110,111,112,113,114,  0,  0,  0,  0,  0,  0,
c    a   0,126,115,116,117,118,119,120,121,122,  0,  0,  0,  0,  0,  0,
c    b   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
c    c 123, 65, 66, 67, 68, 69, 70, 71, 72, 73,  0,  0,  0,  0,  0,  0,
c    d 125, 74, 75, 76, 77, 78, 79, 80, 81, 82,  0,  0,  0,  0,  0,  0,
c    e  92,  0, 83, 84, 85, 86, 87, 88, 89, 90,  0,  0,  0,  0,  0,  0,
c    f  48, 49, 50, 51, 52, 53, 54, 55, 56, 57,124,  0,  0,  0,  0,  0/
c
c    ebcdic translation table, with ascii values given above
c
c    0 nul,soh,stx,etx, pf, ht, lc,del, ge,rlb,smm, vt, ff, cr, so, si,
c    1 dle,dc1,dc2, tm,res, nl, bs, il,can, em, cc,cu1,ifs,igs,irs,ius,
c    2  ds,sos, fs,   ,byp, lf,etb,esc,   ,   , sm,cu2,   ,enq,ack,bel,
c    3    ,   ,syn,   , pn, rs, uc,eot,   ,   ,   ,cu3,dc4,nak,   ,sub,
c    4  sp,   ,   ,   ,   ,   ,   ,   ,   ,   ,cnt,  .,  <,  (,  +,  |,
c    5   &,   ,   ,   ,   ,   ,   ,   ,   ,   ,  !,  $,  *,  ),  ;,  ~,
c    6   -,  /,   ,   ,   ,   ,   ,   ,   ,   ,  |,  ,,  %,  _,  >,  ?,
c    7    ,   ,   ,   ,   ,   ,   ,   ,   ,  `,  :,  #,  @,  ',  =,  ",
c    8    ,  a,  b,  c,  d,  e,  f,  g,  h,  i,   ,   ,   ,   ,   ,   ,
c    9    ,  j,  k,  l,  m,  n,  o,  p,  q,  r,   ,   ,   ,   ,   ,   ,
c    a    ,  ~,  s,  t,  u,  v,  w,  x,  y,  z,   ,   ,   ,   ,   ,   ,
c    b    ,   ,   ,   ,   ,   ,   ,   ,   ,   ,   ,   ,   ,   ,   ,   ,
c    c   {,  a,  b,  c,  d,  e,  f,  g,  h,  i,   ,   ,hok,   ,   ,   ,
c    d   },  j,  k,  l,  m,  n,  o,  p,  q,  r,   ,   ,   ,   ,   ,   ,
c    e   \,   ,  s,  t,  u,  v,  w,  x,  y,  z,   ,   ,chr,   ,   ,   ,
c    f   0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  |,   ,   ,   ,   , eo
c
c     end
c DESCRIPTION
c
c      rdchar is a function that returns the value of the character
c read in from the IRIS. The following is a routine used on VMS
c to read in one character. It will certainly not work for you, but
c is left here as an example.
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                     c
c	rdchar - read in a character                                  c
c                                                                     c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c     integer function rdchar()
c     Implicit Integer*4 ( A-Z )
c     Character*1 OneChar
c     Byte OneByte
c     Integer Eot, Uichar
c     Data Eot /3/
c     Data StartFlag /0/
c
c     External IO$_TTYREADALL
c     External IO$M_TIMED, IO$M_NOECHO
c
c Constants
c
c     Data Event_Flag /0/
c     Data BufSiz /1/
c     FRead =%Loc(IO$_TTYREADALL)+%Loc(IO$M_TIMED)+%Loc(IO$M_NOECHO)
c
c Assign channel to terminal device (TT:)
c
c     If (StartFlag .eq. 0) Then
c        Icode=SYS$ASSIGN('SYS$INPUT', Channel,,)
c        StartFlag = 1
c     End If
c
c Read One Character
c
c     Icode=SYS$QIOW( %Val(Event_Flag), %Val(Channel), %Val(FRead),
c    *		,,,OneByte, %Val(BufSiz),0,0,,)
c     OneChar = Char(OneByte)
c     RdChar = Uichar( OneChar )
c     If (Ichar(OneChar) .eq. Eot) STOP
c
c     Return
c     End
c
c DESCRIPTION
c
c     wrtbuf ships out the contents of the output buffer to the
c IRIS.	This routine writes a buffer in non-formatted binary; note
c that the following routine is provided merely as an example, and
c will most definitely not work on your machine.
c
c     Subroutine WrtBuf( Buff, Nchar )
c
c     Implicit Integer*4 ( A-Z )
c     Character*1 Buff(Nchar), Onechr
c
c     External IO$_WRITEVBLK, IO$M_NOFORMAT
c
c Constants
c
c     Data Event_Flag /0/
c     Data StartFlag /0/
c     VWRITE = %Loc(IO$_WRITEVBLK)+%Loc(IO$M_NOFORMAT)
c
c Assign Channel to terminal device (TT:)
c
c     If (StartFlag .eq. 0) Then
c        Icode = SYS$ASSIGN('SYS$OUTPUT',Channel,,)
c        StartFlag = 1
c     End If
c
c Write the buffer
c
c     If (Nchar .gt. 0) Then
c      ICode = SYS$QIOW( %Val(Event_Flag), %Val(Channel), %Val(VWRITE),
c    *        , , ,%Ref(Buff), %Val(Nchar),0,%Val(0),,)
c     End If
c
c     Return
c     End
c
c DESCRIPTION
c
c    Echoon and echoff turn echoing on and off, respectively, for
c the output device ( the IRIS ). These routines are called by the
c lib.f routines ( see description above ).
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                   c
c     echo - dummy echo routines                                    c
c                                                                   c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c     subroutine echoon
c     return
c     end
c
c     subroutine echoff
c     return
c     end
c
c DESCRIPTION - see below
c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                    c
c     and - return the logical 'and' of two integer*4 variables      c
c                                                                    c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c      integer function and( arg1, arg2 )
c
c      Implemented on BSD 4.2 as an implicit function
c          (see bit in section 3f)
c       You will want to use some language that does bit manipulation
c
c      return
c      end
c
c DESCRIPTION
c
c	or is the logical 'or' operation of two integer*4 variables
c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                    c
c       or                                                           c
c                                                                    c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c      integer function or( arg1, arg2 )
c
c      Implemented on BSD 4.2 as an implicit function
c          (see bit in section 3f)
c       You will want to use some language that does bit manipulation
c
c      return
c      end
c
c DESCRIPTION
c
c	lshift is a logical left shift with no end around carry
c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c    lshift - shift the word 'word' left nbits.
c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c       integer function lshift( word, nbits )
c       integer*4 word
c       integer*4 nbits
c 
c       Implimented on BSD 4.2 as an implicit function
c            ( see bit in section 3f )
c       You will want to use some language that does bit manipulation
c
c       return
c       end
c
c DESCRIPTION
c
c	rshift is an arithmatic right shift with sign extension
c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c    rshift - shift the word 'word' right ( with sign extension )
c             nbits.
c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c       integer function rshift( word, nbits )
c       integer*4 word
c       integer*4 nbits
c 
c       Implimented on BSD 4.2 as an implicit function
c            ( see bit in section 3f )
c       You will want to use some language that does bit manipulation
c
c       return
c       end
c
c DESCRIPTION
c
c    f2ieee is a function that converts the users floating point
c numbers to IEEE format ( this is what the IRIS uses ). These numbers
c occupy 4 bytes each. The following is a sample routines that we use
c to convert DEC floating point values to IEEE values.
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                       c
c    f2ieee - convert dec floating point to ieee floating point         c
c                                                                       c
c									c
c    returns zero on underflow instead of denormalized ieee.		c
c    ( note the use of the intrinsic function 'and' )
c									c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      integer function f2ieee( fltval )
      real        fltval, fltequ
      integer*2   in2equ(2), in2tmp
      integer*4   intequ, inttmp
      equivalence ( intequ, fltequ, in2equ(1) )
c
      fltequ = fltval
      inttmp = intequ
c
c     Assume we have underflow
c
      f2ieee = 0
      if ( and( inttmp, 32640 ) .le. 256 ) goto 100
      intequ = intequ - 256
      in2tmp    = in2equ(1)
      in2equ(1) = in2equ(2)
      in2equ(2) = in2tmp
      f2ieee    = intequ
100   return
      end
c
c DESCRIPTION
c
c     ieee2d does just the reverse of d2ieee; it takes an ieee
c formatted floating point value, and converts it the host machine's
c format. The following is an example of converting ieee reals to
c DEC formatted reals.
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                       c
c    ieee2f - converts ieee floating point to dec floating point        c
c                                                                       c
c
c    returns zero for denormalized ieee and
c            decmax on overflow ( including ieee infinity )
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      real function ieee2f( n )
      integer*2   in2equ(2), in2tmp
      integer     inttmp, intequ, n
      real        fltequ
      equivalence ( intequ, fltequ, in2equ(1) )
c
      intequ = n
      in2tmp    = in2equ(1)
      in2equ(1) = in2equ(2)
      in2equ(2) = in2tmp
c
c assume we're gonna have underflow
c
      inttmp = and( intequ, 32640 )
      if ( inttmp .eq. 0 ) goto 200
      if ( inttmp .lt. 32512 ) goto 100
c
c         inttmp < 0  returns x'ffffffff'
c
c
c         inttmp >= 0 returns x'ffff7ffff'
c
      inttmp = -1
      if ( intequ .ge. 0 ) inttmp = -32679
      goto 200
100   inttmp = intequ + 256
200   intequ = inttmp
      ieee2f = fltequ
      return
      end
c
c DESCRIPTION
c
c     i4trns is the routine that takes a 4 byte integer and converts
c it into "chunks" to send to the IRIS.
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                   c
c     i4trns( intval, i, j )                                        c
c                                                                   c
c                                                                   c
c     function: given an integer*4 variable 'intval', take 'j'      c
c               bits starting at 'i' (where bit 0 = lsb),           c
c               add the ascii value for blank, and return it as a   c
c               printable ascii character.                          c
c                                                                   c
c               However, if j is 8 bits, we are doing raw io, and   c
c               the ascii value for blank is not added              c
c                                                                   c
c     where:
c        intval = integer variable to work on
c        i = initial bit position to begin creating a byte value
c                                (not ebcdic) value. 0 <= i <= 30
c        j      = number of bits to use. j = { 2, 4, 6, or 8}
c
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      character*1 function i4trns( intval, i, j)
      include 'fio.h'
      integer     intval, i, j, k

      k = rshift( intval, i )
c
c Set up our bit field
c
      GO TO ( 10, 20, 30, 40 ) J/2
10    k = and( k,3 ) + iblank
      goto 100
20    k = and( k,15 ) + iblank
      goto 100
30    k = and( k, 63) + iblank
      goto 100
40    k = and( k, 255)
100   i4trns = char( k )
400   return
      end
c
c DESCRIPTION
c
c     TTYRES merely restores the terminal to it's original state,
c so that instead of having the special characteristics needed for
c the IRIS, it's used as a regular VDT.
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                    c
c  ttyres - restore the terminal                                     c
c                                                                    c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c     subroutine ttyres
c     return
c     end
c
c DESCRIPTION
c
c  cleane - Put the user's IRIS back into regular VDT mode, and then
c give a successful return code to his(her) cli (shell).
c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                    c
c  cleane - make a clean getaway back to the user cli (shell)        c
c                                                                    c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c     subroutine cleane
c     integer status, succes
c     data succes /0/
c
c     call ttyres
c     call exit(status=succes)
c     return
c     end
c*********************************************************************
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                    c
c     endpri - End of a primitive routine                            c
c                                                                    c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine endpri
      return
      end
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                    c
c     flushg - flush the graphics stream                             c
c                                                                    c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine flushg
      include 'fio.h'
      integer     k
c
      if (bufptr .eq. 1) goto 100
      k = bufptr - 1
      call wrtbuf(outbuf, k)
      bufptr = 1
100   return
      end
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                   c
c     gcmd - send graphics escape and the specified command code    c
c                                                                   c
c                                                                   c
c     assumptions:                                                  c
c        1) this is a 2's complement machine                        c
c        2) long  = 4 bytes                                         c
c        3) short = 2 bytes                                         c
c                                                                   c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine gcmd( intval )
      include 'fio.h'
      include 'frpc.h'
      integer intval, FLTIME
c
c     flush time checks to see if we might be into the
c     end of our output buffer
c
      parameter ( FLTIME = OUTBSZ - LBYTGR + 1 )
c
c     make sure we're initialized 
c
      if (ntinit .eq. 0) call netini
      if ( bufptr .gt. FLTIME ) call flushg
      call putgch( char(TESC) )
c
c     put out bits<0:5>, <6:11>
c
      call putg( intval, 12, 6)
      return
      end
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                   c
c    getg - get twidth/cwidth [+1] characters from the iris         c
c                                                                   c
c
c     twidth = total bit width of the incoming entity
c     cwidth = bit width of a character
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      integer function getg( twidth, cwidth)
      include 'fio.h'
      integer     twidth, cwidth, inttmp, l, rdchar
c
      inttmp = 0
      do 700 l = 1, twidth, cwidth
      inttmp = or( inttmp , lshift( (rdchar() - iblank ), l-1 ))
700   continue
      getg = inttmp
      return
      end
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                    c
c     gexit  - equivalent to flushg                                  c
c                                                                    c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine gexit
      include 'fio.h'
c
      if ( ntinit .eq. 0 ) call netini
      call xgexit
      call flushg
      call ttyres
      return
      end
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                    c
c     gfinis  - equivalent to flushg                                 c
c                                                                    c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine gfinis
      include 'fio.h'
c
      if ( ntinit .eq. 0 ) call netini
      call flushg
      return
      end
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                    c
c     gflush - equivalent to flushg                                  c
c                                                                    c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine gflush
      include 'fio.h'
c
      if ( ntinit .eq. 0 ) call netini
      call flushg
      return
      end
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                    c
c     ginit - MUST BE THE FIRST GRAPHICS COMMAND ISSUED              c
c                                                                    c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine ginit
      include 'fio.h'
c
      if ( ntinit .eq. 0 ) call netini
      call xginit
      call flushg
      return
      end
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                    c
c     greset - takes special action on the terminal                  c
c                                                                    c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine greset
      include 'fio.h'
c
      if ( ntinit .eq. 0 ) call netini
      call xgrese
      call flushg
      return
      end
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                    c
c     netini - initialize the net                                    c
c                                                                    c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine netini
      include 'fio.h'
      integer fast, nettst, i
c
c     show the world that we are now init'ed
c
      ntinit = 1
c
c     initialize the buffer pointer to point to the first entry
c
      bufptr = 1
c
      fast = nettst()
c
c Save our current terminal status
c
      call termsv
c
c     initialize command mode
c
      do 300 i = 1, lbytgr
      call putgch( char(0) )
      if ( bufptr .gt. highp ) call flushg
300   continue
c
      if ( fast .eq. 0 ) goto 100
c
c We have ethernet at our command
c
      call setfas
      goto 200
c
c No ethernet available
c
100   call setslo
200   continue
      return
      end
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                   c
c     putg - send a bunch of stuff to iris                          c
c                                                                   c
c
c     inputs:
c           intval	input value to be converted to characters
c           twidth   	bit width of intval
c           cwidth      bit width of graphics characters to be sent
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine putg( intval, twidth, cwidth)
      include 'fio.h'
      integer     intval, twidth, cwidth, k, iwidth
      character*1   i4trns
      logical     exflag
c
      iwidth = cwidth
c
c     set exit flag to false initially
c
      exflag = .false.
      do 510 k = 0, twidth, iwidth
      if (( twidth - k ) .gt. iwidth) goto 500
      exflag = .true.
      iwidth = twidth - k
500   outbuf( bufptr ) =  i4trns( intval, k, iwidth)
      bufptr = bufptr + 1
      if ( exflag ) return
510   continue
      end
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                    c
c     putgch - put char to graphics output buffer                    c
c                                                                    c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine putgch( chrval )
      include 'fio.h'
      character*1    chrval
c
      outbuf( bufptr ) = chrval
      bufptr = bufptr + 1
      return
      end
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                   c
c    recb - receive one byte from the iris                          c
c                                                                   c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      character*1 function recb()
      include 'frpc.h'
      integer     getg, rdchar
c
200   if ( rdchar() .ne. RESC ) goto 200
      recb   = char( getg( 12, 6 ) )
      return
      end
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                   c
c     recbs - receive an array of bytes                             c
c                                                                   c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine recbs ( strng )
      include 'fio.h'
      include 'frpc.h'
      character*(*) strng
      integer       getg, recl, intval, k, l, nlongs, nvals, rdchar
      integer       inttmp
      character*1   i4trns, chrvls(4)
c
      nvals    = recl()
      if ( nvals .gt. len( strng ) )write ( 6, 1100 )
      nlongs   = ( nvals +  3 )/ 4
      if ( rdchar() .eq. RESC ) goto 100
      write ( 6, 1000 )
      goto 700
c
100   do 600 l = 1, nlongs
      k = l - 1
c
c     grab 4 bytes worth of data
c     this section should be machine independent
c
      intval = getg ( 32, 6 )
      chrvls(1) = i4trns( intval,  0, 8)
      chrvls(2) = i4trns( intval,  8, 8)
      chrvls(3) = i4trns( intval, 16, 8)
      chrvls(4) = i4trns( intval, 24, 8)
c
c     Every eighth time send a sync character to the IRIS
c
      if ( and( k, 7 ) .ne. 0 ) goto 110
      inttmp = rdchar()
      call putgch ( char(AESC) )
      call flushg
110   continue
      k = 4*l - 3
c
c     we can always assign the first byte
c
      if ( l .lt. nlongs ) goto 560
      goto ( 590, 580, 570, 560 ) nvals + 1 - k
560   strng(k+3:k+3) = chrvls(lelmt4)
570   strng(k+2:k+2) = chrvls(lelmt3)
580   strng(k+1:k+1) = chrvls(lelmt2)
590   strng(k:k)     = chrvls(lelmt1)
c
600   continue
      inttmp = rdchar()
700   return
1000  format (' recbs: error in array transport')
1100  format (' recbs: overflow of string array')
      end
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                   c
c     reccr - get a graphics character, and throw it away           c
c                                                                   c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine reccr()
      integer     inttmp, rdchar
c
      inttmp =  rdchar()
      return
      end
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                    c
c     recf - receive a float                                         c
c                                                                    c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      real function recf()
      include 'fio.h'
      integer     recl
      real        ieee2f
c
      recf  = ieee2f( recl() )
      return
      end
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                    c
c     recfs - receive a an array of floats                           c
c                                                                    c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine recfs( rvals )
      include 'frpc.h'
      real        rvals( * ), ieee2f
      integer     recl, getg, nlongs, i, k, rdchar, inttmp
c
      nlongs = recl()
      if ( rdchar() .eq. RESC ) goto 100
      write ( 6, 1000 )
      goto 700
100   continue
      do 600 i = 1, nlongs
      k = i - 1
      rvals( i ) = ieee2f( getg( 32, 6 ) )
c
c     Every eighth time through, starting with the first, send
c     a sync character to the IRIS.
c
      if ( and( k, 7 ) .ne. 0 ) goto 600
      inttmp = rdchar()
      call putgch( char(AESC) )
      call flushg
600   continue
      inttmp = rdchar()
700   return
1000  format (' recfs: error in array transport')
      end
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                   c
c     recl - receive a long word from the iris                      c
c                                                                   c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      integer function recl()
      include 'frpc.h'
      integer     getg, rdchar
c
725   if ( rdchar() .ne. RESC ) goto 725
      recl = getg( 32, 6)
      return
      end
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                   c
c     recls - receive an array of longs                             c
c                                                                   c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine recls( ivals )
      include 'frpc.h'
      integer     ivals( * ), getg, recl, k, l, nlongs, inttmp, rdchar
c
      nlongs = recl()
      if ( rdchar() .eq. RESC ) goto 100
      write ( 6, 1000 )
      goto 700
100   continue
      do 600 l = 1, nlongs
      k = l - 1
      ivals( l ) = getg( 32, 6)
      if ( and( k, 7 ) .ne. 0 ) goto 600
      inttmp = rdchar()
      call putgch( char(AESC) )
      call flushg
600   continue
      inttmp = rdchar()
700   return
1000  format (' recls: error in array transport')
      end
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                     c
c     reco - receive a boolean                                        c
c                                                                     c
c
c     we have to translate bytes from the iris to logical longwords
c     for the host
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc 
      logical function reco()
      integer recl, i
c
      i = recl()
      reco = .true.
      if ( i .eq. 0 ) reco = .false.
      return
      end
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                   c
c     recos - receive an array of logical bytes                     c
c                                                                   c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c     This has no prayer of working.  Logicals are now 4 bytes      c
c     long.							    c
c			Rocky and Tom  7/9/85			    c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine recos ( lvals )
      include 'fio.h'
      include 'frpc.h'
      logical       lvals ( * )
      integer       getg, recl, uichar, intval, k, l, nlongs, nvals
      integer       rdchar, inttmp
      character*1   i4trns, chvals(4)
c
      nvals    = recl()
      nlongs   = ( nvals +  3 )/ 4
      do 600 l = 1, nlongs
      k = l - 1
      if ( rdchar() .eq. RESC ) goto 100
      write ( 6, 1000 )
      goto 700
c
c     jam the 6 bytes received from the iris into intval, and then
c     cull them out into the chvals array
c
100   intval    = getg ( 32, 6 )
      chvals(1) = i4trns( intval,  0, 8)
      chvals(2) = i4trns( intval,  8, 8)
      chvals(3) = i4trns( intval, 16, 8)
      chvals(4) = i4trns( intval, 24, 8)
      if ( and( k, 7 ) .ne. 0 ) goto 110
      inttmp = rdchar()
      call putgch ( char(AESC) )
      call flushg
110   continue
      k = 4*l - 3
c
c     we can always assign the first logical entity
c     (we are attempting to be careful of overstepping
c      array boundaries here)
c
      if ( l .lt. nlongs ) goto 560
      goto ( 590, 580, 570, 560 ) nvals + 1 - k
560   lvals( k+3 ) = .true.
      if( uichar( chvals( lelmt4 )) .eq. 0 ) lvals( k+3 ) = .false.
570   lvals( k+2 ) = .true.
      if( uichar( chvals( lelmt3 )) .eq. 0 ) lvals( k+2 ) = .false.
580   lvals( k+1 ) = .true.
      if( uichar( chvals( lelmt2 )) .eq. 0 ) lvals( k+1 ) = .false.
590   lvals( k ) = .true.
      if( uichar( chvals( lelmt1 )) .eq. 0 ) lvals( k ) = .false.
c
600   continue
      inttmp = rdchar()
700   return
1000  format (' recOs: error in array transport')
      end
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                   c
c     recs - receive a short                                        c
c                                                                   c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      integer*2 function recs()
      include 'frpc.h'
      integer     getg, rdchar
c
630   if ( rdchar() .ne. RESC ) goto 630
      recs = getg( 16, 6)
      return
      end
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                   c
c     recss - receive an array of shorts                            c
c                                                                   c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine recss( ivals )
      include 'fio.h'
      include 'frpc.h'
      integer      nvals, getg, recl, intequ, k, l, nlongs, rdchar
      integer      inttmp
      integer*2    ivals( * ), in2equ(2)
      logical      even
      equivalence  ( intequ, in2equ(1) )
c
      nvals = recl()
      even  = .true.
      if ( and( nvals, 1 ) .ne. 0 ) even = .false.
      nlongs = ( nvals + 1 ) / 2
      if ( rdchar() .eq. RESC ) goto 100
      write ( 6, 1000 )
      goto 700
100   continue
      do 640 l = 1, nlongs
      k = l - 1
      intequ = getg( 32, 6)
      if ( and( k, 7 ) .ne. 0 ) goto 110
      inttmp = rdchar()
      call putgch( char(AESC) )
      call flushg
110   continue
      ivals( 2*l-1 ) = in2equ(selmt1)
      if ( (l .ne. nvals ) .or. (even) ) ivals( 2*l ) = in2equ(selmt2)
640   continue
      inttmp = rdchar()
700   return
1000  format (' recss: error in array transport')
      end
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                    c
c     senflb - send a byte to the iris                               c
c              ( first convert the argument from a long to a byte )  c
c                                                                    c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine senflb(intval)
      integer   intval
      character chrval
      chrval = char( intval )
      call sendb( chrval )
      return
      end
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                    c
c     sendb - send a byte to the iris                                c
c                                                                    c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine sendb(chrval)
      include 'fio.h'
      character*1 chrval
      integer     uichar
c
      if ( fastmo .eq. 1 ) goto 100
c
c     put bits <0:5>, <6:7> out to the buffer if in slow mode
c     ( note that we don't care about possible sign extension here)
c
      call putg( uichar( chrval), 8, 6)
      goto 200
c
c     put bits <0:8> out to the buffer if in fast mode
c
100   call putgch( chrval )
200   if ( bufptr .gt. highp ) call flushg
      return
      end
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                    c
c     sendbs  - send an array of bytes to the graphics unit          c
c                                                                    c
c                                                                    c
c     send an array of bytes to the iris                             c
c                                                                    c
c     input:	nvals - number of bytes to send to iris; note that   c
c                       these bytes are not character strings        c
c     		bytvls - array of bytes                              c
c                                                                    c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine sendbs( bytvls, nvals )
      include 'fio.h'
      include 'frpc.h'
      integer     nvals, intval, k, l, m, nlongs, uichar
      character*(*) bytvls
c
c     define how many longwords we are about to send.
c     note that nlongs is used as a signed integer. if
c     huge transfers are ever desired in the future,
c     this section may need some work
c
      nlongs = (nvals + 3 ) / 4
c
c     tell the iris how many bytes he's going to get
c
      call sendl(nvals)
c
c     we need to keep track of how many bytes we've done to avoid
c     overstepping the boundary of the bytvals array
c
      do 60 l = 1, nlongs
      m = l - 1
      k = 4*l - 3
c
c     we can always assign the first byte
c
c     Tho the following code must be plain as day, the intent
c     is to take the bytes from the bytval array, and plug them
c     into the corresponding byte positions in intval. Whether
c     they go in from hi to lo or vice versa depends on our host
c
      if ( l .lt. nlongs ) goto 10
      intval = 0
      goto ( 40, 30, 20, 10 ) nvals + 1 - k
10    intval = lshift( uichar( bytvls(k+3:k+3) ), BYTSH4 )
20    intval = or( intval , lshift( uichar( bytvls(k+2:k+2)),BYTSH3))
30    intval = or( intval , lshift( uichar( bytvls(k+1:k+1)),BYTSH2))
40    intval = or( intval , lshift( uichar( bytvls(k:k    )),BYTSH1))
50    if ( and( m, 7 ) .eq. 0 ) call putgch( char(AESC) )
      call sendl( intval )
60    continue
      return
      end
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                   c
c     sendc - send a string                                         c
c                                                                   c
c                                                                   c
c     assumptions:                                                  c
c         the user has done his own ebcdic - ascii                  c
c     conversion for his input character string.                    c
c     we could do it if we overlaid his string, but we only         c
c     have a 1-1 correspondence from ebcdic -> ascii, and           c
c     not the other direction.                                      c
c                                                                   c
c                                                                   c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine sendc( strng )
      include 'fio.h'
      include 'frpc.h'
      character*(*) strng
      integer     uichar, inttmp, intval, k, l, m, nlongs, nvals
c
c     define how many longwords we are about to send.
c     note that nlongs is used as a signed integer. if
c     huge transfers are ever desired in the future,
c     this section may need some work
c
      nvals  = len(strng) + 1
      nlongs = (nvals + 3 ) / 4
c
c     tell the iris how many bytes he's going to get
c
      call sendl(nvals)
c
c     we need to keep track of how many bytes we've done to avoid
c     overstepping the boundary of the bytvals array
c
      do 200 l = 1, nlongs
      m = l - 1
      k = 4*l - 3
c
c     we can always assign the first byte
c
c     Tho the following code must be plain as day, the intent
c     is to take the bytes from the bytval array, and plug them
c     into the corresponding byte positions in intval. Whether
c     they go in from hi to lo or vice versa depends on our host
c
c
c     if this is the last time thru, we need to do some special
c     stuff
c
      if ( l .lt. nlongs ) goto 110
      intval = 0
      goto ( 150, 140, 130, 120 ), nvals + 1 - k
110   intval = lshift( uichar( strng(k+3:k+3)), BYTSH4 )
120   inttmp = lshift( uichar( strng(k+2:k+2)), BYTSH3 )
      intval = or( intval , inttmp )
130   inttmp = lshift( uichar( strng(k+1:k+1)), BYTSH2 )
      intval = or( intval , inttmp )
140   inttmp = lshift( uichar( strng(k:k)),     BYTSH1 )
      intval = or( intval , inttmp )
150   if( and( m, 7 ) .eq. 0 ) call putgch( char(AESC) )
      call sendl( intval )
200   continue
      return
      end
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                   c
c     sendf - send a float 6+6+6+6+6+2                              c
c                                                                   c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine sendf( fltval )
      include 'fio.h'
      real        fltval
      integer     f2ieee
c
      call sendl( f2ieee(fltval) )
      return
      end
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                   c
c     sendfs - send an array of floats                              c
c                                                                   c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine sendfs( rvals, nvals)
      include 'frpc.h'
      integer   nvals, k, l
      real      rvals( * )
c
      call sendl( nvals*4 )
      do 475 l = 1, nvals
      k = l - 1
c
c     Send sync character every eighth time, starting with the first
c
      if ( and( k, 7 ) .eq. 0 ) call putgch( char(AESC) )
      call sendf( rvals(l) )
475   continue
      return
      end
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                   c
c     sendl - send a long                                           c
c                                                                   c
c
c     receive 4 bytes, and send them out as 6+6+...+6+remainder
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine sendl(intval)
      include 'fio.h'
      integer    intval
      character*1  i4trns
c
c     slow mode -
c     put bits <0:5>,<6:11>,<12:17>,<18:23>,<24:29>,<30:31>
c              out to the buffer
c
c     fast mode -
c     put bits <0:7>,<8:15>,<16:23>, and <24:31> out to buffer
c
      if ( fastmo .ne. 0 ) goto 100
      call putg( intval, 32, 6)
      goto 200
100   call putgch( i4trns( intval, BYTSH1, 8 ))
      call putgch( i4trns( intval, BYTSH2, 8 ))
      call putgch( i4trns( intval, BYTSH3, 8 ))
      call putgch( i4trns( intval, BYTSH4, 8 ))
200   if ( bufptr .gt. highp ) call flushg
      return
      end
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                   c
c     sendls - send an array of longs                               c
c                                                                   c
c
c     input:	nvals - number of longs to send to iris
c     		ivals - array of longs
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine sendls( ivals, nvals )
      include 'frpc.h'
      integer    nvals, ivals( * ), k, l
c
c     tell the iris how many longs he's going to get
c
      call sendl( nvals*4 )
c
c     we need to keep track of how many bytes we've done to avoid
c     overstepping the boundary of the ivals array
c
      do 300 l = 1, nvals
      k = l - 1
      if ( and( k, 7 ) .eq. 0 ) call putgch( char(AESC) )
      call sendl( ivals(l) )
300   continue
      return
      end
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                   c
c     sendo - send a logical                                        c
c                                                                   c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine sendo( lval )
      logical lval
      character*1 chrval
c
c     Assume our input value lval is false
c
      chrval = char(0)
      if (lval .eq. .true.) chrval = char(1)
      call sendb( chrval )
      return
      end
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                    c
c     sendos  - send an array of logical bytes to the graphics unit  c
c                                                                    c
c
c     send an array of logical values to the iris
c
c     input:	nvals - number of bytes to send to iris
c                       note that these bytes are not character strings
c                       but logical values
c               lvals   array of logical values
c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine sendos( lvals, nvals )
      include 'frpc.h'
      include 'fio.h'
      integer     nvals, inttmp, k, l, m, nlongs
      logical     lvals( nvals )
c
c     define how many longwords we are about to send.
c     note that nlongs is used as a signed integer. if
c     huge transfers are ever desired in the future,
c     this section may need some work
c
      nlongs = ( nvals + 3 ) / 4
c
c     tell the iris how many bytes he's going to get
c
      call sendl(nvals)
c
c     we need to keep track of how many bytes we've done to avoid
c     overstepping the boundary of the bytvals array
c
      do 60 l = 1, nlongs
      m = l - 1
      k = 4*l - 3
c
c     1) we can always assign the first byte
c     2) watch out for array boundaries
c     3) assume all input values are false to begin with
c
      inttmp = 0
      if ( l .lt. nlongs ) goto 10
      goto ( 40, 30, 20, 10 ) nvals + 1 - k
10    if ( lvals( k+3 )) inttmp = LTRUE4
20    if ( lvals( k+2 )) inttmp = or( inttmp , LTRUE3 )
30    if ( lvals( k+1 )) inttmp = or( inttmp , LTRUE2 )
40    if ( lvals( k  ) ) inttmp = or( inttmp , LTRUE1 )
50    if ( and( m, 7 ) .eq. 0 ) call putgch( char(AESC) )
      call sendl( inttmp )
60    continue
      return
      end
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                    c
c     sendqs  - send an array of Fontchars to the graphics unit      c
c                                                                    c
c
c     send an array of Fontchars to the iris
c
c     input:	nvals - number of Fontchars to send to iris
c                       note that these are each 4 integer*2's
c     		qvals - array of integer*2's
c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine sendqs( qvals, nvals )
      integer*2 qvals( * )
      integer nvals
c
      call sendss( qvals, nvals * 4 )
      return
      end
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                   c
c     senfls - send a short                                         c
c                                                                   c
c                                                                   c
c     send 16 bits in lsword, and send them out as 6+6+4            c
c     ( first convert the longword to a short )                     c
c                                                                   c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine senfls(intval)
      integer   intval
      integer*2 in2val
      in2val = intval
      call sends( in2val )
      return
      end
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                   c
c     sends - send a short                                          c
c                                                                   c
c                                                                   c
c     receive 16 bits in lsword, and send them out as 6+6+4         c
c                                                                   c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine sends(intval)
      include 'fio.h'
      integer*2  intval
      integer    inttmp
      character*1 i4trns
c
c     put bits <0:5>, <6:11>, <12:15> out to the buffer
c
      inttmp = intval
      if ( fastmo .ne. 0 ) goto 100
c
c     we're to use slow mode
c
      call putg( inttmp, 16, 6)
      goto 200
100   continue
c
c     we're to use fast mode
c
      call putgch( i4trns( inttmp, 8, 8 ))
      call putgch( i4trns( inttmp, 0, 8 ))
200   if ( bufptr .gt. highp ) call flushg
      return
      end
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                   c
c     sendss - send an array of shorts                              c
c                                                                   c
c
c     input:	nvals - number of shorts
c                       to send to iris
c     		ivals - array of integer*2.
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine sendss( ivals, nvals )
      include 'fio.h'
      include 'frpc.h'
      integer    nvals, k, l, nlongs, intequ
      integer*2  ivals( * ), in2equ(2)
      logical     even
      equivalence ( intequ, in2equ(1) )
c
c     define how many longwords we are about to send
c     note that nlongs is used as a signed integer. if
c     huge transfers are ever desired in the future,
c     this section may need some work
c
      nlongs = (nvals + 1)/2
      even = .true.
      if ( nvals .ne. ((nvals/2)*2) ) even = .false.
c
c     tell the iris how many bytes he's going to get
c
      call sendl( nvals*2 )
c
c     we need to keep track of how many shorts we've done to avoid
c     overstepping the boundary of the ivals array
c
      do 450 l = 1, nlongs
      k = l+7
      in2equ(selmt1) = ivals( 2*l-1 )
      if ( (l .ne. nlongs) .or. (even) ) in2equ(selmt2) = ivals( 2*l )
c
c     Send a Sync character every eighth time, starting with the first
c
      if ( k .eq. ((k/8)*8) )call putgch( char(AESC) )
      call sendl( intequ )
450   continue
      return
      end
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                    c
c     setfas - go into fast mode                                     c
c                                                                    c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      logical function setfas()
      include 'fio.h'
      logical xsetfa

      if ( ntinit .eq. 0 ) call netini
      if (xsetfa()) goto 100
      setfas = .FALSE.
      return
100   continue
      fastmo = 1
      setfas = .TRUE.
      return
      end
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                    c
c     setslo - go into slow mode                                     c
c                                                                    c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      logical function setslo()
      include 'fio.h'
      logical xsetsl
c
      if( ntinit .eq. 0 ) call netini
      if (xsetsl()) goto 100
      setslo = .FALSE.
      return
100   continue
      fastmo = 0
      setslo = .TRUE.
      return
      end
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                     c
c     uichar - return an unsigned integer value from a byte           c
c     (unfortunately, ichar sign extends)                             c
c                                                                     c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      integer function uichar( chrval )
      include 'fio.h'
      character*1 chrval
c
      uichar = and( ichar( chrval ), 255 )
      return
      end
