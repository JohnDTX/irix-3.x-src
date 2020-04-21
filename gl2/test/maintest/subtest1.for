C     Test 1:  GETBUT, CMOV2I, CHARST, and STRWID
C	       sendl, sendbs, recL, and recO
C
      SUBROUTINE TEST1()
C	integer function test1()
      CHARACTER*40 STRING*110, STR1, STR2, STR3, STR4
      INTEGER WIDTH, STRLEN, NEXTDN, DATA, DEVICE, I, LOOP
      LOGICAL PRESSD, WIDERR
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C     Installation note:
C	Various Fortran compilers may require different styles of INCLUDEs.
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
$INCLUDE /usr/include/fgl.h
$INCLUDE /usr/include/fdevice.h
C
      CALL CMOV2I(50,50)
      CALL CLRTXT()
      STRING = 'Indicate ''bad'' if you do not wish to do the following 
     1tests:'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,35)
      STRING = '    Subtest1:  GETBUT, CMOV2I, CHARST, and STRWID'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,20)
      STRING = '    sendl, sendbs, recL, and recO'
      CALL CHARST(STRING, LEN(STRING))
C
      IF (.NOT. PRESSD()) THEN
        CALL CLRTXT()
	RETURN
      END IF
C
      CALL CLRTXT()
      CALL CMOV2I(50,50)
      STRING = 'You should be able to read this text, and the table of t
     1ested and untested'
      STRLEN = LEN('You should be able to read this text, and the table 
     1of tested and untested')
      CALL CHARST(STRING, STRLEN)
      WIDTH = STRWID(STRING, STRLEN)
      IF (WIDTH .NE. (STRLEN * 9)) THEN
	WIDERR = .TRUE.
      ELSE
	WIDERR = .FALSE.
      END IF
      CALL CMOV2I(50,35)
      STRING = 'communications routines at the top of the screen should 
     1look good.'
      STRLEN = LEN('communications routines at the top of the screen sho
     1uld look good.')
      CALL CHARST(STRING, STRLEN)
      WIDTH = STRWID(STRING, STRLEN)
      IF (WIDTH .NE. (STRLEN * 9)) WIDERR = .TRUE.
      CALL CMOV2I(50,20)
      STRING = 'Indicate good or bad.'
      STRLEN = LEN('Indicate good or bad.')
      CALL CHARST(STRING, STRLEN)
      WIDTH = STRWID(STRING, STRLEN)
      IF (WIDTH .NE. (STRLEN * 9)) WIDERR = .TRUE.
C
      IF (PRESSD()) THEN
        STR1 = 'sendl  OK'
        STR2 = 'sendbs OK'
        IF (WIDERR) THEN
          STR3 = 'recL      ?'
          CALL CMOV2I(15,NEXTDN(0))
          STRING = 'STRWID returned an incorrect value.'
          CALL CHARST(STRING, LEN(STRING))
        ELSE
          STR3 = 'recL   OK'
        END IF
      ELSE
        STR1 = 'sendl    ?'
        STR2 = 'sendbs   ?'
        IF (WIDERR) THEN
          STR3 = 'recL      ?'
          CALL CMOV2I(15,NEXTDN(0))
          STRING = 'STRWID returned an incorrect value.'
          CALL CHARST(STRING, LEN(STRING))
        ELSE
          STR3 = 'recL   OK'
        END IF
        CALL CMOV2I(15,NEXTDN(0))
        STRING = 'CMOV2I, CHARST problems.'
        CALL CHARST(STRING, LEN(STRING))
      END IF
      STR4 = 'recO   OK'
      CALL TESTED(STR1,625)
      CALL TESTED(STR2,685)
      CALL TESTED(STR3,490)
      CALL TESTED(STR4,445)
      CALL CLRTXT()
C
      CALL CMOV2I(100,50)
      STRING = 'Test@@@@@'
      CALL CHARST(STRING, 4)
      CALL CMOV2I(200,50)
      STRING = 'Testi@@@@@'
      CALL CHARST(STRING, 5)
      CALL CMOV2I(300,50)
      STRING = 'Testin@@@@@'
      CALL CHARST(STRING, 6)
      CALL CMOV2I(400,50)
      STRING = 'Testing@@@@@'
      CALL CHARST(STRING, 7)
      CALL CMOV2I(50,35)
      STRING = 'The line immediately above should have no @ signs.'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,20)
      STRING = 'Indicate good or bad.'
      CALL CHARST(STRING, LEN(STRING))
C
      IF (PRESSD()) THEN
        STR2 = 'sendbs OK'
      ELSE
        STR2 = 'sendbs    ?'
        CALL CMOV2I(15,NEXTDN(0))
        STRING = 'CHARST strings not terminated reliably'
        CALL CHARST(STRING, LEN(STRING))
      END IF
      CALL TESTED(STR2,685)
      CALL CLRTXT()
C
      CALL CMOV2I(50,50)
      STRING = 'Wait a moment, then press ''good'' and hold down for a f
     1ew seconds to verify'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,35)
      STRING = 'the GETBUT function.'
      CALL CHARST(STRING, LEN(STRING))
C
C     wait for button released
      LOOP = LOOPSZ()
      DO 1000 I=1,LOOP
      IF (.NOT. GETBUT(BUT100)) GO TO 1010
1000  CONTINUE
C     button never released
      STR1 = 'senfls   ?'
      STR2 = 'recO     ?'
      CALL CMOV2I(15,NEXTDN(0))
      STRING = 'GETBUT stuck .TRUE.'
      CALL CHARST(STRING, LEN(STRING))
      GO TO 1040
      I = 0
1010  CONTINUE
C     loop for button pressed
      DEVICE = QREAD(DATA)
      IF (DEVICE .NE. BUT100) GO TO 1010
      IF (DATA .EQ. 0) GO TO 1010
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C     Installation note:
C	The loop size should be adjusted to ~3 seconds for contact bounce.
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
      LOOP = LOOPSZ() / 10
      DO 1020 I=1,LOOP
      IF (GETBUT(BUT100)) GO TO 1030
1020  CONTINUE
C     failed test
      STR1 = 'senfls   ?'
      STR2 = 'recO     ?'
      CALL CMOV2I(15,NEXTDN(0))
      STRING = 'GETBUT stuck .FALSE.'
      CALL CHARST(STRING, LEN(STRING))
1030  CONTINUE
C     passed test
      STR1 = 'senfls OK'
      STR2 = 'recO   OK'
      GO TO 1040
1040  CONTINUE
      CALL TESTED(STR1,655)
      CALL TESTED(STR2,445)
1050  CONTINUE
C     clear the device queue
      IF (QTEST() .EQ. 0) GO TO 1060
      DEVICE = QREAD(DATA)
      GO TO 1050
1060  CONTINUE
C
      RETURN
      END
