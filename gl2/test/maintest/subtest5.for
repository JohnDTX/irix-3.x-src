C     Test 5:  DEFCUR, SETCUR, GETCUR, LOADMA, and GETMAT
C	       senfls, sendss, sendfs, recO, recS, and recFs
C
      SUBROUTINE TEST5()
      CHARACTER*40 STRING*110, STR1, STR2, STR3
      LOGICAL PRESSD, B, LNOP
      INTEGER NEXTDN, I, J
      INTEGER*2 RADIUS, INDEX, COL, WTM, BITMAP(16)
      REAL MATIN(4,4), MATOUT(4,4)
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
      STRING = '    Subtest5:  DEFCUR, SETCUR, GETCUR, LOADMA, and GETMA
     1T'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,20)
      STRING = '    senfls, sendss, sendfs, recO, recS, and recFs'
      CALL CHARST(STRING, LEN(STRING))
C
      IF (.NOT. PRESSD()) THEN
        CALL CLRTXT()
	RETURN
      END IF
C
      BITMAP(16) = 57344
      BITMAP(15) = 39936
      BITMAP(14) = 33664
      BITMAP(13) = 16640
      BITMAP(12) = 16896
      BITMAP(11) = 16640
      BITMAP(10) = 18560
      BITMAP(9) = 13376
      BITMAP(8) = 8736
      BITMAP(7) = 272
      BITMAP(6) = 136
      BITMAP(5) = 68
      BITMAP(4) = 34
      BITMAP(3) = 17
      BITMAP(2) = 9
      BITMAP(1) = 7
C
      CALL CURSON()
      CALL CLRTXT()
      CALL CMOV2I(50,50)
      STRING = 'A cursor -- a red or white arrow which follows the mouse
     1 -- has appeared on the screen.'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,35)
      STRING = 'It may disappear or change color as it is moved over var
     1ious elements on the screen.'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,20)
      STRING = 'Once you are familiar with the cursor, press to change i
     1t into a green outlined arrow.'
      CALL CHARST(STRING, LEN(STRING))
      LNOP = PRESSD()
C
      CALL DEFCUR(5,BITMAP)
      CALL SETCUR(5,GREEN,255)
      CALL CURSOF()
      CALL GETCUR(INDEX, COL, WTM, B)
      IF (B) THEN
C     recO bad
        CALL CURSON()
        CALL GETCUR(INDEX, COL, WTM, B)
        STRING = 'GETCUR boolean always .TRUE.'
        GO TO 100
      END IF
      CALL CURSON()
      CALL GETCUR(INDEX, COL, WTM, B)
      IF (.NOT. B) THEN
C     recO bad
        STRING = 'GETCUR boolean always .FALSE.'
        GO TO 100
      END IF
C     if recO OK
      STR1 = 'recO   OK'
      GO TO 200
100   CONTINUE
      STR1 = 'recO   OK ?'
      CALL CMOV2I(15,NEXTDN(0))
      CALL CHARST(STRING, LEN(STRING))
200   CONTINUE
      CALL TESTED(STR1,445)
C
      CALL CMOV2I(50,50)
      CALL CLRTXT()
      STRING = 'The cursor should now have changed from a red or white a
     1rrow into a green outlined arrow.'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,35)
      STRING = 'The green arrow should be hollow, clean, smooth, and poi
     1nting left and up as before.'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,20)
      STRING = 'Move it over a red background to see it best.  Indicate 
     1if it is good or bad.'
      CALL CHARST(STRING, LEN(STRING))
C
      IF (PRESSD()) THEN
        STR1 = 'senfls OK'
        STR2 = 'sendss OK'
        IF ((INDEX.EQ.5) .AND. (COL.EQ.GREEN) .AND. (WTM.EQ.255)) THEN
          STR3 = 'recS   OK'
        ELSE
          STR3 = 'recS     ?'
          CALL CMOV2I(15,NEXTDN(0))
          STRING = 'GETCUR not OK (DEFCUR & SETCUR OK)'
          CALL CHARST(STRING, LEN(STRING))
        END IF
      ELSE
        STR1 = 'senfls     ?'
        STR2 = 'sendss     ?'
        IF ((INDEX.EQ.5) .AND. (COL.EQ.GREEN) .AND. (WTM.EQ.255)) THEN
          STR3 = 'recS   OK'
        ELSE
          STR3 = 'recS     ?'
          CALL CMOV2I(15,NEXTDN(0))
          STRING = 'CUR routines not working'
          CALL CHARST(STRING, LEN(STRING))
        END IF
      END IF
      CALL TESTED(STR1,655)
      CALL TESTED(STR2,640)
      CALL TESTED(STR3,520)
      CALL CLRTXT()
C
      DO 300 I=1,4
      DO 300 J=1,4
      MATIN(I,J) = I + (10 * J) - 11
300   CONTINUE
      CALL PUSHMA()
      CALL LOADMA(MATIN)
      CALL GETMAT(MATOUT)
      CALL POPMAT()
      DO 400 I=1,4
      DO 400 J=1,4
      IF (MATIN(I,J) .NE. MATOUT(I,J)) GO TO 500
400   CONTINUE
      STR1 = 'sendfs OK'
      STR2 = 'recFs  OK'
      GO TO 600
500   CONTINUE
      STR1 = 'sendfs     ?'
      STR2 = 'recFs    ?'
      CALL CMOV2I(15,NEXTDN(0))
      STRING = 'MATIN/OUT not 100%'
      CALL CHARST(STRING, LEN(STRING))
600   CONTINUE
      CALL TESTED(STR1,580)
      CALL TESTED(STR2,460)
C
      RETURN
      END
