CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C     Installation note:
C	Various Fortran compilers may require different styles of INCLUDEs.
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
$INCLUDE /usr/include/fdevice.h
$INCLUDE /usr/include/fgl.h
C
      CHARACTER*40 STRING*110, STR1
      INTEGER NEXTDN, I, NOP
      LOGICAL PRESSD, LNOP, BUTERR
C
C     Initialization
C
      NOP = NEXTDN(1)
      CALL GINIT()
      LNOP = SETSLO()
      CALL CURSOF()
      CALL COLOR(CYAN)
      CALL CLEAR()
      CALL QDEVIC(BUT100)
      CALL QDEVIC(BUT102)
C
      CALL COLOR(BLACK)
      CALL CMOV2I(50,750)
      STRING = 'This test tests the various remote Fortran communication
     1s routines shown below:'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(15,730)
      STRING = 'TESTED                NOT TESTED:'
      CALL CHARST(STRING, LEN(STRING))
      CALL COLOR(RED)
      CALL CMOV2I(75,730)
      STRING = '(when red)'
      CALL CHARST(STRING, LEN(STRING))
      CALL COLOR(BLACK)
      CALL CMOV2I(320,730)
      STRING = 'recB   <never used>'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(320,715)
      STRING = 'recLs  <never used>'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(550,730)
      STRING = 'sendos <never used>'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(550,715)
      STRING = 'recOs  <never used>'
      CALL CHARST(STRING, LEN(STRING))
C     CALL CMOV2I(760,730)
C     STRING = 'sendqs <defras()>'
C     CALL CHARST(STRING, LEN(STRING))
C     CALL CMOV2I(760,715)
C     STRING = 'recBs  <readRG()>'
C     CALL CHARST(STRING, LEN(STRING))
C     CALL CMOV2I(760,700)
C     STRING = 'recSs  <endpic(), etc>'
C     CALL CHARST(STRING, LEN(STRING))
C
      CALL CMOV2I(15,715)
      STRING = 'sendb'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(15,700)
      STRING = 'senflb'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(15,685)
      STRING = 'sendbs'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(15,670)
      STRING = 'sends'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(15,655)
      STRING = 'senfls'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(15,640)
      STRING = 'sendss'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(15,625)
      STRING = 'sendl'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(15,610)
      STRING = 'sendls'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(15,595)
      STRING = 'sendf'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(15,580)
      STRING = 'sendfs'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(15,565)
      STRING = 'sendo'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(15,550)
      STRING = 'sendqs'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(15,535)
      STRING = 'recBs'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(15,520)
      STRING = 'recS'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(15,505)
      STRING = 'recSs'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(15,490)
      STRING = 'recL'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(15,475)
      STRING = 'recF'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(15,460)
      STRING = 'recFs'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(15,445)
      STRING = 'recO'
      CALL CHARST(STRING, LEN(STRING))
C
C     Notify user about good/bad indications
C
      CALL CLRTXT()
      CALL CMOV2I(50,50)
      STRING = 'When the text here asks you to "indicate good or bad", p
     1ress the right mouse button'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,35)
      STRING = 'to indicate ''good'', or the left mouse button to indica
     1te ''bad'' or an error.'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,20)
      STRING = 'Press ''good''.'
      CALL CHARST(STRING, LEN(STRING))
      BUTERR = .FALSE.
      IF (.NOT. PRESSD()) BUTERR = .TRUE.
      CALL CLRTXT()
C
      CALL CMOV2I(50,50)
      STRING = 'To respond to the message "press to move on", press eith
     1er outside mouse button.'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,35)
      STRING = 'In any case, if a button press is not sensed, a long tim
     1eout will end the program.'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,20)
      STRING = 'Press to move on.'
      CALL CHARST(STRING, LEN(STRING))
      LNOP = PRESSD()
      CALL CLRTXT()
C
      CALL CMOV2I(50,50)
      STRING = 'Note:  each routine''s name is rewritten in red once it 
     1has been tested.  It is followed'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,35)
      STRING = 'by an ''OK'' if it tested good in at least one test, and
     1 by one ''?'' for each test in which'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,20)
      STRING = 'it was suspect.  The name of the offending test is then 
     1also listed above.  Press ''bad''.'
      CALL CHARST(STRING, LEN(STRING))
      IF (PRESSD()) BUTERR = .TRUE.
      IF (BUTERR) THEN
        STR1 = 'recL     ?'
        CALL CMOV2I(15,NEXTDN(0))
        STRING = 'QREAD gives error'
        CALL CHARST(STRING, LEN(STRING))
      ELSE
        STR1 = 'recL   OK'
      END IF
      CALL TESTED(STR1,490)
      CALL CLRTXT()
C
#ifdef REMOTE
      CALL CMOV2I(50,50)
      STRING = 'Indicate ''good'' if you wish to continue the tests in s
     1lowcom communications mode.'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,35)
      STRING = 'Indicate ''bad'' to switch the tests to fastcom (if
     1 the communications link'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,20)
      STRING = 'is capable of it).'
      CALL CHARST(STRING, LEN(STRING))
      IF (PRESSD()) LNOP = SETFAS()
      CALL CLRTXT()
#endif
C
      CALL TEST1()
      CALL TEST2()
      CALL TEST3()
      CALL TEST4()
      CALL TEST5()
      CALL TEST6()
      CALL TEST7()
C
      CALL CLRTXT()
      CALL CMOV2I(50,50)
      STRING = 'Press to return to interactive terminal.'
      CALL CHARST(STRING, LEN(STRING))
      LNOP = PRESSD()
      CALL CLRTXT()
      CALL GEXIT()
      STOP
      END
C
C
C     Return the y coordinate of the next text line down on the screen.
C
      INTEGER FUNCTION NEXTDN(INIT)
      INTEGER NEXT, INIT
      SAVE NEXT
      IF (INIT .EQ. 1) THEN
C     Initialize
      NEXT = 430
      ELSE
      NEXT = NEXT - 15
      END IF
      NEXTDN = NEXT
      RETURN
      END
C
C
C     Clear the text area at the bottom of the screen
C
      SUBROUTINE CLRTXT()
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C     Installation note:
C	Various Fortran compilers may require different styles of INCLUDEs.
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
$INCLUDE /usr/include/fgl.h
C
      CALL COLOR(CYAN)
      CALL RECTFI(40,12,985,70)
      CALL COLOR(BLACK)
      RETURN
      END
C
C
C     Rewrite the given routine in red, to indicate it has been tested.
C     The routine will be followed by an 'OK' if it tested good in one test.
C     If it is followed by one or more '?'s, it may be bad.
C
      SUBROUTINE TESTED(STRG,Y)
      INTEGER Y
      CHARACTER*(*) STRG
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C     Installation note:
C	Various Fortran compilers may require different styles of INCLUDEs.
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
$INCLUDE /usr/include/fgl.h
C
      CALL COLOR(RED)
      CALL CMOV2I(15,Y)
      CALL CHARST(STRG, LEN(STRG))
      CALL COLOR(BLACK)
      RETURN
      END
C      
C
C     Return once a mouse button has been released, then pressed
C     again.  Time out and exit if this doesn't happen.
C
      LOGICAL FUNCTION PRESSD()
      INTEGER I, DEVICE, NEXTDN
      INTEGER*2 DATA
      CHARACTER*40 STR1
      INTEGER LOOP
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C     Installation note:
C	Various Fortran compilers may require different styles of INCLUDEs.
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
$INCLUDE /usr/include/fdevice.h
$INCLUDE /usr/include/fgl.h
C
      CALL GFLUSH()
C     wait for event
      LOOP = LOOPSZ()
      DO 1000 I=1,LOOP
      IF (QTEST() .NE. 0) GO TO 1010
1000  CONTINUE
      STR1 = 'recL     ?'
      CALL TESTED(STR1,490)
      CALL CMOV2I(15,NEXTDN(0))
      STR1 = 'QDEVICE or QTEST unresponsive'
      CALL CHARST(STR1, LEN(STR1))
      CALL GEXIT()
      PRINT *, 'Mouse button not pressed or released.'
      STOP
1010  CONTINUE
C     loop for button pressed
      DEVICE = QREAD(DATA)
      IF (DATA .EQ. 0) GO TO 1010
C     true if left button
      PRESSD = .FALSE.
      IF (DEVICE .EQ. BUT100) PRESSD = .TRUE.
C
      RETURN
      END

CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C     Installation note:
C	The loop size should be adjusted to ~30 seconds.  Start with 2000
C	iterations for remote terminals; 2000000 for workstations.
C	See also the similar loops in subtest1.f.
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
      FUNCTION LOOPSZ()
      LOGICAL ISREM
      IF (ISREM()) THEN
      LOOPSZ = 2000
      ELSE
      LOOPSZ = 2000000
      END IF
      RETURN
      END
