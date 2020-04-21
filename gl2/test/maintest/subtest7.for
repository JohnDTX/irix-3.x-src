C     Test 7:  dial and button box
C
C
      SUBROUTINE TEST7()
      CHARACTER*40 STRING*120, STR1, STR2, STR3
      LOGICAL PRESSD
      INTEGER NEXTDN, I
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
      STRING = '    Subtest7:  dial and button box'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,20)
      STRING = ' '
      CALL CHARST(STRING, LEN(STRING))
C
      IF (.NOT. PRESSD()) THEN
        CALL CLRTXT()
	RETURN
      END IF
C
      CALL CLRTXT()
      CALL CMOV2I(50,50)
      STRING = 'TEST7 not yet implemented'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,35)
      STRING = 'Press to move on.'
      CALL CHARST(STRING, LEN(STRING))
C
      IF (PRESSD()) THEN
      STR1 = 'test7'
      ELSE
      STR1 = 'test7 ?'
      CALL CMOV2I(15,NEXTDN(0))
      STRING = 'TEST7 not yet implemented'
      CALL CHARST(STRING, LEN(STRING))
      END IF
      CALL TESTED(STR1,430)
      CALL CLRTXT()
C
      RETURN
      END
