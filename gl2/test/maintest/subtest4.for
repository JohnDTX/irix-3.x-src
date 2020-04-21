C     Test 4:  ARCFS, ROTATE, SETBEL, and BLANKS
C	       sendb, senflb, sends, senfls, and sendo
C	(CIRCFI, TRANSL, PUSHMA, and POPMAT used, too)
C
      SUBROUTINE TEST4()
      CHARACTER*40 STRING*110, STR1, STR2, STR3
      LOGICAL PRESSD, LNOP, BELERR
      INTEGER NEXTDN, I, NOP
      INTEGER*2 X, Y, RADIUS
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C     Installation note:
C	Various Fortran compilers may require different styles of INCLUDEs.
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
      INCLUDE 'fgl.h'
C
      CALL CMOV2I(50,50)
      CALL CLRTXT()
      STRING = 'Indicate ''bad'' if you do not wish to do the following 
     1tests:'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,35)
      STRING = '    Subtest4:  ARCFS, ROTATE, SETBEL, and BLANKS'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,20)
      STRING = '    sendb, senflb, sends, senfls, and sendo'
      CALL CHARST(STRING, LEN(STRING))
C
      IF (.NOT. PRESSD()) THEN
        CALL CLRTXT()
	RETURN
      END IF
C
      CALL PUSHMA()
      CALL TRANSL(400.0,400.0,0.0)
      CALL COLOR(BLUE)
      CALL RECTF(0.0,0.0,200.0,200.0)
      CALL COLOR(WHITE)
      CALL CIRCFI(100,100,100)
      CALL COLOR(RED)
      X = 100
      Y = 100
      RADIUS = 100
      CALL ARCFS(X,Y,RADIUS,0,225)
      CALL TRANSL(100.0,100.0,0.0)
      X = 0
      Y = 0
      DO 100 I=1,3
      CALL ROTATE(450,'z')
      CALL ARCFS(X,Y,RADIUS,0,225)
100   CONTINUE
      DO 200 I=1,4
      CALL ROTATE(450,'Z')
      CALL ARCFS(X,Y,RADIUS,0,225)
200   CONTINUE
      CALL POPMAT()
C
      CALL CLRTXT()
      CALL CMOV2I(50,50)
      STRING = 'Look for a blue square with a red & white circle in it a
     1bove the hexagons.  The red & white spokes'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,35)
      STRING = 'should be the same width and length as each other, be ev
     1enly spaced, and just touch the sides of'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,20)
      STRING = 'the square.  As usual, indicate good or bad.'
      CALL CHARST(STRING, LEN(STRING))
C
      IF (PRESSD()) THEN
      STR1 = 'sends  OK'
      STR2 = 'senfls OK'
      STR3 = 'sendb  OK'
      ELSE
      STR1 = 'sends    ?'
      STR2 = 'senfls    ?'
      STR3 = 'sendb    ?'
      CALL CMOV2I(15,NEXTDN(0))
      STRING = 'ARCFS, ROTATE do funny things'
      CALL CHARST(STRING, LEN(STRING))
      END IF
      CALL TESTED(STR1,670)
      CALL TESTED(STR2,655)
      CALL TESTED(STR3,715)
C
      CALL CLRTXT()
      CALL CMOV2I(50,50)
      STRING = 'After you press to move on, you should hear a short beep
     1.'
      CALL CHARST(STRING, LEN(STRING))
      LNOP = PRESSD()
      CALL SETBEL(1)
      CALL RINGBE()
C
      CALL CLRTXT()
      CALL CMOV2I(50,50)
      STRING = 'After indicating good or bad, you should not hear a beep
     1.'
      CALL CHARST(STRING, LEN(STRING))
      IF (.NOT. PRESSD()) THEN
	BELERR = .TRUE.
      ELSE
	BELERR = .FALSE.
      END IF
      CALL SETBEL(0)
      CALL RINGBE()
C
      CALL CLRTXT()
      CALL CMOV2I(50,50)
      STRING = 'Indicate good if you did NOT just hear a beep; bad if yo
     1u did.'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,35)
      STRING = 'You should then hear a longer beep.'
      CALL CHARST(STRING, LEN(STRING))
      IF (.NOT. PRESSD()) BELERR = .TRUE.
      CALL SETBEL(2)
      CALL RINGBE()
C
      CALL CLRTXT()
      CALL CMOV2I(50,50)
      STRING = 'Did you just hear a longer beep?  Indicate good or bad.'
      CALL CHARST(STRING, LEN(STRING))
      IF (.NOT. PRESSD()) BELERR = .TRUE.
C
      IF (BELERR) THEN
        STR1 = 'senflb   ?'
        CALL CMOV2I(15,NEXTDN(0))
        STRING = 'SETBEL doesn''t'
        CALL CHARST(STRING, LEN(STRING))
      ELSE
        STR1 = 'senflb OK'
      END IF
      CALL TESTED(STR1,700)
      CALL CLRTXT()
C
      CALL CMOV2I(50,50)
      STRING = 'After you press to go on, the screen should go blank.'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,35)
      STRING = 'WARNING:  It will go REALLY blank, so note now and remem
     1ber what to do when it does.'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,20)
      STRING = 'Once the screen goes blank (or not), indicate good or ba
     1d.'
      CALL CHARST(STRING, LEN(STRING))
C
      LNOP = PRESSD()
      CALL BLANKS(.TRUE.)
      IF (PRESSD()) THEN
      STR1 = 'sendo  OK'
      ELSE
      STR1 = 'sendo    ?'
      CALL CMOV2I(15,NEXTDN(0))
      STRING = 'BLANKS won''t'
      CALL CHARST(STRING, LEN(STRING))
      END IF
      CALL BLANKS(.FALSE.)
      CALL TESTED(STR1,565)
      CALL CLRTXT()
C
      RETURN
      END
