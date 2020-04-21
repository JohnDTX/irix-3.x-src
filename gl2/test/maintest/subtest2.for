C     Test 2:  GETGPO, RECTF, READRG, and DEFRAS
C	       recF, sendf, recBs, and sendqs
C
      SUBROUTINE TEST2()
      CHARACTER*40 STR1, STRING*110, DEFSTR*2
      CHARACTER*25 RVALS, GVALS, BVALS, CMPARY
      LOGICAL PRESSD, LNOP, SQSOK, BSOK
      INTEGER NEXTDN, I, NOVALS, N, NOP
      INTEGER*2 SCHARS(12), SRASTR(15), IARRAY(6)
      REAL X, Y, Z, W, SARRAY(2,4)
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C     Installation note:
C	Various Fortran compilers may require different styles of INCLUDEs.
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
      INCLUDE 'fgl.h'
C
C
      CALL CMOV2I(50,50)
      CALL CLRTXT()
      STRING = 'Indicate ''bad'' if you do not wish to do the following 
     1tests:'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,35)
      STRING = '    Subtest2:  GETGPO, RECTF, DEFRAS, and READRG'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,20)
      STRING = '    recF, sendf, recBs, and sendqs'
      CALL CHARST(STRING, LEN(STRING))
C
      IF (.NOT. PRESSD()) THEN
        CALL CLRTXT()
	RETURN
      END IF
C
      CALL COLOR(RED)
      CALL RECTF(31.0,3.0,994.0,79.0)
      CALL COLOR(GREEN)
      CALL RECTF(34.0,6.0,991.0,76.0)
      CALL COLOR(BLUE)
      CALL RECTF(37.0,9.0,988.0,73.0)
C
C	Note: GETGPO is broken in the Vkernel terminal, and crashes it.
C	      Works fine on the UNIX terminal
C
      CALL GETGPO(X,Y,Z,W)
      IF ((INT(X*1000.0).EQ.-926).AND.(INT(Y*1000.0).EQ.-975)
     1        .AND.(Z.EQ.0.0).AND.(W.EQ.1.0)) THEN
	STR1 = 'recF   OK'
      ELSE
	STR1 = 'recF     ?'
	CALL CMOV2I(15,NEXTDN(0))
	STRING = 'GETGPO gets wrong g''pos'
	CALL CHARST(STRING, LEN(STRING))
      END IF
      CALL TESTED(STR1,475)
C
      CALL CLRTXT()
      CALL CMOV2I(50,50)
      STRING = 'This text area should now have an even RGB border around
     1 it.'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,35)
      STRING = 'Indicate good or bad.'
      CALL CHARST(STRING, LEN(STRING))
C
      IF (PRESSD()) THEN
      STR1 = 'sendf  OK'
      ELSE
      STR1 = 'sendf    ?'
      CALL CMOV2I(15,NEXTDN(0))
      STRING = 'RECTF in trouble'
      CALL CHARST(STRING, LEN(STRING))
      END IF
      CALL TESTED(STR1,595)
      CALL CLRTXT()
C
      SARRAY(1,1) = 256.0
      SARRAY(2,1) = 100.0
      SARRAY(1,2) = 269.0
      SARRAY(2,2) = 100.0
      SARRAY(1,3) = 269.0
      SARRAY(2,3) = 120.0
      SARRAY(1,4) = 256.0
      SARRAY(2,4) = 120.0
      IARRAY(1) = 0
      IARRAY(2) = 7
      IARRAY(3) = 7
      IARRAY(4) = 0
C
      DO 100 I=1,25
	CMPARY(I:I) = CHAR(6)
100   CONTINUE
      CMPARY(6:6) = CHAR(0)
      CMPARY(7:7) = CHAR(1)
      CMPARY(8:8) = CHAR(1)
      CMPARY(9:9) = CHAR(2)
      CMPARY(10:10) = CHAR(2)
      CMPARY(11:11) = CHAR(3)
      CMPARY(12:12) = CHAR(3)
      CMPARY(13:13) = CHAR(4)
      CMPARY(14:14) = CHAR(4)
      CMPARY(15:15) = CHAR(5)
      CMPARY(16:16) = CHAR(5)
      CMPARY(17:17) = CHAR(6)
      CMPARY(18:18) = CHAR(6)
      CMPARY(19:19) = CHAR(7)
C
C     Whole test is bogus.  Must put in RGB mode in order to use readRGB.
C		CSK 3/3/86
C
      CALL SPLF2(4,SARRAY,IARRAY)
      CALL CMOV2I(251,110)
      N = 25
ccccc	Next line commented out because of remote terminal error.
C      NOVALS = READRG(N, RVALS, GVALS, BVALS)
 
ccccc	These next lower case lines are used to test this test.
c      print *, 'rvals gvals bvals ', novals, 'values returned'
c      if ((novals .lt. 1) .or. (novals .gt. n)) novals = n
c      do 111 i=1,novals
c	print *, ichar(rvals(i:i)),' ',ichar(gvals(i:i)),' ',ichar(bvals
c     1(i:i))
111   continue
c      stop
C
      if (novals .ne. n/3) go to 300
C     IF (NOVALS .NE. N) GO TO 300
      DO 200 I=1,NOVALS
	if (rvals(i:i) .ne. cmpary((i*3)-2:(i*3)-2)) go to 300
	if (gvals(i:i) .ne. cmpary((i*3)-1:(i*3)-1)) go to 300
	if (bvals(i:i) .ne. cmpary(i*3:i*3)) go to 300
C       IF (RVALS(I:I) .NE. CMPARY(I:I)) GO TO 300
200   CONTINUE
C	if O.K.
      STR1 = 'recBs  OK'
      call cmov2i(15,nextdn(0))
      string = 'READRG: test is incomplete & incorrect'
      call charst(string, len(string))
      GO TO 400
300   CONTINUE
      STR1 = 'recBs    ?'
      CALL CMOV2I(15,NEXTDN(0))
      STRING = 'READRG gets incorrect values'
ccccc	Next line added because of remote terminal error.
      STRING = 'READRG test commented out -- test is bogus since not 
     1 in RGB mode'
      CALL CHARST(STRING, LEN(STRING))
400   CONTINUE
      CALL TESTED(STR1,535)
      CALL CLRTXT()
C
C       no character 0
      SCHARS(1) = 0
      SCHARS(2) = 0
      SCHARS(3) = 0
      SCHARS(4) = 0
C       character 1 should be correct: w=9, h=15, xoff = -4, yoff = -7
      SCHARS(5) = 0
      SCHARS(6) = (9 * 256) + 15
      SCHARS(7) = (-4 * 256) - 7 + 256
      SCHARS(8) = 12
C       character 2 should have w & h, xoff & yoff reversed
      SCHARS(9) = 0
      SCHARS(10) = (15 * 256) + 9
      SCHARS(11) = (-7 * 256) - 4 + 256
      SCHARS(12) = 12
C
      SRASTR(1) = 65408
      SRASTR(2) = 32896
      SRASTR(3) = 48768
      SRASTR(4) = 41600
      SRASTR(5) = 43648
      SRASTR(6) = 43648
      SRASTR(7) = 43648
      SRASTR(8) = 43648
      SRASTR(9) = 43648
      SRASTR(10) = 43648
      SRASTR(11) = 47744
      SRASTR(12) = 33408
      SRASTR(13) = 65152
      SRASTR(14) = 128
      SRASTR(15) = 65408
C
      CALL COLOR(RED)
      CALL RECTI(14,91,26,109)
      CALL RECTI(54,91,66,109)
      CALL RECTI(94,91,106,109)
C
C       This code draws crosshairs if desired
C     CALL MOVE2I(0,100)
C     CALL DRAW2I(120,100)
C     CALL MOVE2I(20,80)
C     CALL DRAW2I(20,120)
C     CALL MOVE2I(60,80)
C     CALL DRAW2I(60,120)
C     CALL MOVE2I(100,80)
C     CALL DRAW2I(100,120)
C
      CALL COLOR(0)
      DO 500 I=0,12,12
	CALL MOVE2I(56+I,107)
	CALL DRAW2I(64+I,107)
	CALL DRAW2I(64+I,93)
	CALL DRAW2I(56+I,93)
	CALL DRAW2I(56+I,105)
	CALL DRAW2I(62+I,105)
	CALL DRAW2I(62+I,95)
	CALL DRAW2I(58+I,95)
	CALL DRAW2I(58+I,103)
	CALL DRAW2I(60+I,103)
	CALL DRAW2I(60+I,97)
500   CONTINUE

      CALL DEFRAS(5, 16, 3, SCHARS, 15, SRASTR)
      CALL FONT(5)
      CALL CMOV2I(20, 100)
      DEFSTR(1:1) = CHAR(1)
      DEFSTR(2:2) = CHAR(1)
      CALL CHARST(DEFSTR,2)
      CALL CMOV2I(100, 100)
      DEFSTR(1:1) = CHAR(2)
      DEFSTR(2:2) = CHAR(2)
      CALL CHARST(DEFSTR,2)
      CALL FONT(0)
C
      CALL CLRTXT()
      CALL CMOV2I(50,50)
      STRING = 'There should now be three red rectangles in the lower le
     1ft corner of the screen.  The middle one'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,35)
      STRING = 'has a rectangular spiral inside it, and another just to 
     1the right.  If the left red rectangle has'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,20)
      STRING = 'exactly the same pattern in and next to it, press ''good
     1''.  Otherwise, press ''bad''.'
      CALL CHARST(STRING, LEN(STRING))
C
      SQSOK = .TRUE.
      IF (PRESSD()) THEN
        STR1 = 'sendqs OK'
	BSOK = .TRUE.
      ELSE
        CALL CLRTXT()
        CALL CMOV2I(50,50)
        STRING = 'If the right rectangle has exactly the same pattern in
     1 and next to it as the middle one,'
        CALL CHARST(STRING, LEN(STRING))
        CALL CMOV2I(50,35)
        STRING = 'press ''good''; otherwise, press ''bad''.'
        CALL CHARST(STRING, LEN(STRING))
C
        IF (PRESSD()) THEN
          STR1 = 'sendqs   ?'
	  BSOK = .FALSE.
        ELSE
	  SQSOK = .FALSE.
          STR1 = 'sendqs   ?'
	  CALL CMOV2I(15,NEXTDN(0))
          STRING = 'DEFRAS doesn''t work'
	  CALL CHARST(STRING, LEN(STRING))
        END IF
      END IF
      CALL TESTED(STR1,550)
C
      IF (SQSOK) THEN
        CALL CLRTXT()
        CALL CMOV2I(50,50)
	IF (BSOK) THEN
	  STRING = 'Your DEFRAS is OK.  The DEFRAS CHAR array should con
     1tain these 4 INTEGER*2 values:'
	  CALL CHARST(STRING, LEN(STRING))
          CALL CMOV2I(50,35)
          STRING = 'offset;   h + (w * 256);   yoff + (xoff * 256);   wi
     1dth'
          CALL CHARST(STRING, LEN(STRING))
          CALL CMOV2I(50,20)
          STRING = 'If h or yoff is negative, add 1 to w or xoff to comp
     1ensate for sign extension.  Press to move on.'
          CALL CHARST(STRING, LEN(STRING))
	ELSE
	  STRING = 'DEFRAS works, except that w & h, xoff & yoff are rev
     1ersed in the Fontchar structure.'
	  CALL CHARST(STRING, LEN(STRING))
          CALL CMOV2I(50,35)
          STRING = 'Press to move on.'
          CALL CHARST(STRING, LEN(STRING))
	END IF
      LNOP = PRESSD()
      END IF
C
      CALL CLRTXT()
      RETURN
      END
