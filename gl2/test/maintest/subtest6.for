C     Test 6:  PICK and ENDPIC
C	       recSs
C
      SUBROUTINE TEST6()
      CHARACTER*40 STRING*120, STR1, STR2, STR3
      LOGICAL PRESSD, PIKERR, LNOP
      INTEGER NEXTDN, I, J, PKOBJ
      INTEGER*2 X, Y, RADIUS, OBJARY(15)
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C     Installation note:
C	Various Fortran compilers may require different styles of INCLUDEs.
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
      INCLUDE 'fgl.h'
C
      CALL CLRTXT()
      CALL CMOV2I(50,50)
      STRING = 'Indicate ''bad'' if you do not wish to do the following 
     1tests:'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,35)
      STRING = '    Subtest6:  PICK and ENDPIC'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,20)
      STRING = '    recSs'
      CALL CHARST(STRING, LEN(STRING))
C
      IF (.NOT. PRESSD()) THEN
        CALL CLRTXT()
	RETURN
      END IF
C
      PIKERR = .FALSE.
      CALL SETCUR(0,WHITE,255)
      CALL CURSON()
      CALL MAKEOB(97)
	CALL PUSHNA(97)
	CALL COLOR(RED)
	CALL CIRCI(825,450,50)
	CALL CIRCI(825,450,75)
	CALL POPNAM()
      CALL CLOSEO()
C
      CALL MAKEOB(98)
	CALL PUSHNA(98)
	CALL COLOR(BLUE)
	CALL CIRCI(900,525,75)
	CALL CALLOB(97)
	CALL POPNAM()
      CALL CLOSEO()
C
      CALL MAKEOB(99)
	CALL PUSHNA(99)
	CALL COLOR(GREEN)
	CALL CIRCFI(750,525,100)
	CALL POPNAM()
      CALL CLOSEO()
C
      CALL MAKEOB(100)
	CALL LOADNA(100)
	CALL CALLOB(99)
	CALL CALLOB(98)
	CALL POPNAM()
      CALL CLOSEO()
      CALL CALLOB(100)
C
      CALL CLRTXT()
      CALL CMOV2I(50,50)
      STRING = 'At the middle right of the screen, you should see three 
     1colors of circles: red, blue, and filled green.'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,35)
      STRING = 'To pick a circle, move the cursor to point at it.  You m
     1ust point at a spot that is the color of the'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,20)
      STRING = 'circle you are picking, not at the background color with
     1in an outlined circle.  Press to move on.'
      CALL CHARST(STRING, LEN(STRING))
      LNOP = PRESSD()
C
      CALL CLRTXT()
      CALL CMOV2I(50,50)
      STRING = 'Position the cursor and press the right mouse button to 
     1pick one or more of the colored circles.'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,35)
      STRING = 'You will get a readout indicating which circle(s) you pi
     1cked.  Do this several times, picking'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,20)
      STRING = 'various circles.'
      CALL CHARST(STRING, LEN(STRING))
C
100   CONTINUE
      IF (PRESSD()) THEN
        DO 200 I=1,15
        OBJARY(I) = -999
200     CONTINUE
	CALL PUSHMA()
C	CALL PUSHVI()
C	CALL VIEWPO(0.0, 1023.0, 0.0, 767.0)
        CALL PICK(OBJARY,15)
	CALL ORTHO2(0.0, 1023.0, 0.0, 767.0)
        CALL CALLOB(100)
        PKOBJ = ENDPIC(OBJARY)
C	CALL POPVIE()
	CALL POPMAT()
	CALL CLRTXT()
	CALL CMOV2I(50,50)
	STRING = 'Each circle you picked should be listed below once:'
        CALL CHARST(STRING, LEN(STRING))
	CALL CMOV2I(50,20)
	STRING = 'Press the right mouse button to pick again; the left o
     1ne to end the test.'
        CALL CHARST(STRING, LEN(STRING))
	J = 1
	STR1 = ' '
	STR2 = ' '
	STR3 = ' '
C
	IF (PKOBJ .EQ. 0) THEN
	  STR1 = 'no circle'
	ELSE IF (PKOBJ .EQ. 1) THEN
	  CALL WCHONE(J, OBJARY, STR1, PIKERR)
	ELSE IF (PKOBJ .EQ. 2) THEN
	  CALL WCHONE(J, OBJARY, STR1, PIKERR)
	  CALL WCHONE(J, OBJARY, STR2, PIKERR)
	ELSE IF (PKOBJ .EQ. 3) THEN
	  CALL WCHONE(J, OBJARY, STR1, PIKERR)
	  CALL WCHONE(J, OBJARY, STR2, PIKERR)
	  CALL WCHONE(J, OBJARY, STR3, PIKERR)
	ELSE
	  PIKERR = .TRUE.
	  STR1 = '# of objects picked is < 0 or > 3!'
	  STR2 = 'recL       ?'
	  CALL TESTED(STR2,490)
	  CALL CMOV2I(15,NEXTDN(0))
          CALL CHARST(STR1, LEN(STR1))
	END IF
C
        CALL CMOV2I(50,35)
        CALL CHARST(STR1, LEN(STR1))
        CALL CMOV2I(350,35)
        CALL CHARST(STR2, LEN(STR2))
        CALL CMOV2I(650,35)
        CALL CHARST(STR3, LEN(STR3))
	GO TO 100
      END IF
C
      CALL CLRTXT()
      CALL CMOV2I(50,50)
      STRING = 'Indicate ''good'' if every pick worked correctly.'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,35)
      STRING = 'Indicate ''bad'' if one or more picks failed other than 
     1because of sloppy pointing.'
      CALL CHARST(STRING, LEN(STRING))
C
      IF (.NOT. PRESSD()) PIKERR = .TRUE.
      IF (PIKERR) THEN
	STR1 = 'recSs    ?'
        CALL CMOV2I(15,NEXTDN(0))
	STRING = 'bad ENDPIC results'
        CALL CHARST(STRING, LEN(STRING))
      ELSE
	STR1 = 'recSs  OK'
      END IF
      CALL TESTED(STR1,505)
C
C	The following commented out code was for test routine development.
C	  CALL GEXIT()
C	  PRINT *, PKOBJ, OBJARY(1), OBJARY(2), OBJARY(3), OBJARY(4), O
C    1BJARY(5), OBJARY(6), OBJARY(7), OBJARY(8), OBJARY(9), OBJARY(10), 
C    2OBJARY(11), OBJARY(12), OBJARY(13), OBJARY(14), OBJARY(15) 
C	  STOP
C
      CALL CLRTXT()
C
      RETURN
      END
C
C
      SUBROUTINE WCHONE(J, OBJARY, STRING, PIKERR)
      INTEGER I, J
      INTEGER*2 OBJARY(*)
      CHARACTER*(*) STRING
      LOGICAL PIKERR
C
      IF (OBJARY(J+1) .NE. 100) THEN
	STRING = 'something incomprehensible'
	PIKERR = .TRUE.
	RETURN
      END IF
      IF (OBJARY(J) .EQ. 2) THEN
	J = J + 3
	IF (OBJARY(J-1) .EQ. 99) THEN
	  STRING = 'the green circle'
	ELSE IF (OBJARY(J-1) .EQ. 98) THEN
	  STRING = 'the blue circle'
	ELSE
	  STRING = 'something incomprehensible'
	  PIKERR = .TRUE.
	END IF
      ELSE IF (OBJARY(J) .EQ. 3) THEN
	J = J + 4
	IF (OBJARY(J-2) .EQ. 98) THEN
	  IF (OBJARY(J-1) .EQ. 97) THEN
	    STRING = 'a red circle'
	  ELSE
	    STRING = 'something incomprehensible'
	    PIKERR = .TRUE.
	  END IF
	ELSE
	  STRING = 'something incomprehensible'
	  PIKERR = .TRUE.
        END IF
      ELSE
	STRING = 'something incomprehensible'
	PIKERR = .TRUE.
      END IF
C
      RETURN
      END
