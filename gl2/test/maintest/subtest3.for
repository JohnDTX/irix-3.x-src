C     Test 3: POLF, POLFI, SPLF2, and SPLF2I
C	      sendl, sendls, sendss, and sendfs
C
      SUBROUTINE TEST3()
      REAL FPOLY2(2,7), FPOLY3(3,7)
      INTEGER*4 I, J, IPATRN(2,6), IPOLY2(2,7), IPOLY3(3,7)
      INTEGER*2 SSHADE(6)
      INTEGER NOP, NEXTDN
      CHARACTER*40 STRING*110, STR1, STR2, STR3
      LOGICAL PRESSD
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
      STRING = '    Subtest3:  POLF, POLFI, SPLF2, and SPLF2I'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,20)
      STRING = '    sendl, sendls, sendss, and sendfs'
      CALL CHARST(STRING, LEN(STRING))
C
      IF (.NOT. PRESSD()) THEN
        CALL CLRTXT()
	RETURN
      END IF
C
      DO 5 I=1,7
      IPOLY3(3,I) = 0
      FPOLY3(3,I) = 0.0
5     CONTINUE
C
      IPATRN(1,1) = 100
      IPATRN(2,1) = 100
      IPATRN(1,2) = 200
      IPATRN(2,2) = 160
      IPATRN(1,3) = 200
      IPATRN(2,3) = 280
      IPATRN(1,4) = 100
      IPATRN(2,4) = 340
      IPATRN(1,5) = 0
      IPATRN(2,5) = 280
      IPATRN(1,6) = 0
      IPATRN(2,6) = 160
C
C     Set up outline polygon
C
      IPOLY3(1,1) = IPATRN(1,1)
      IPOLY3(2,1) = IPATRN(2,1) - 2
      IPOLY3(1,2) = IPATRN(1,2) + 2
      IPOLY3(2,2) = IPATRN(2,2) - 2
      IPOLY3(1,3) = IPATRN(1,3) + 2
      IPOLY3(2,3) = IPATRN(2,3) + 2
      IPOLY3(1,4) = IPATRN(1,4)
      IPOLY3(2,4) = IPATRN(2,4) + 3
      IPOLY3(1,5) = IPATRN(1,5) - 2
      IPOLY3(2,5) = IPATRN(2,5) + 2
      IPOLY3(1,6) = IPATRN(1,6) - 2
      IPOLY3(2,6) = IPATRN(2,6) - 2
C
C     Make four white hexagonal outlines
C
      DO 10 J=1,6
      IPOLY3(1,J) = IPOLY3(1,J) + 45
      FPOLY3(1,J) = IPOLY3(1,J) + 245.0
      FPOLY3(2,J) = IPOLY3(2,J)
10    CONTINUE
      CALL COLOR(WHITE)
      CALL POLFI(6,IPOLY3)
      CALL POLF(6,FPOLY3)
      DO 20 J=1,6
      IPOLY3(1,J) = IPOLY3(1,J) + 490
      FPOLY3(1,J) = FPOLY3(1,J) + 490.0
20    CONTINUE
      CALL POLFI(6,IPOLY3)
      CALL POLF(6,FPOLY3)
C
C     Draw paterned hexagons, using all numbers of sides from 3 - 7
C
C     Black triangle
C
      IPOLY3(1,1) = 100
      IPOLY3(2,1) = 100
      IPOLY3(1,2) = 150
      IPOLY3(2,2) = 130
      IPOLY3(1,3) = 90
      IPOLY3(2,3) = 106
      DO 30 J=1,3
      IPOLY3(1,J) = IPOLY3(1,J) + 45
      FPOLY3(1,J) = IPOLY3(1,J) + 245.0
      FPOLY3(2,J) = IPOLY3(2,J)
30    CONTINUE
      CALL COLOR(BLACK)
      CALL POLFI(3,IPOLY3)
      CALL POLF(3,FPOLY3)
C
C     Red septagon
C
      IPOLY3(1,1) = 90
      IPOLY3(2,1) = 106
      IPOLY3(1,2) = 150
      IPOLY3(2,2) = 130
      IPOLY3(1,3) = 200
      IPOLY3(2,3) = 160
      IPOLY3(1,4) = 200
      IPOLY3(2,4) = 280
      IPOLY3(1,5) = 100
      IPOLY3(2,5) = 340
      IPOLY3(1,6) = 0
      IPOLY3(2,6) = 280
      IPOLY3(1,7) = 0
      IPOLY3(2,7) = 160
      DO 40 J=1,7
      IPOLY3(1,J) = IPOLY3(1,J) + 45
      FPOLY3(1,J) = IPOLY3(1,J) + 245.0
      FPOLY3(2,J) = IPOLY3(2,J)
40    CONTINUE
      CALL COLOR(RED)
      CALL POLFI(7,IPOLY3)
      CALL POLF(7,FPOLY3)
C
C     Green septagon
C
      IPOLY3(1,1) = 70
      IPOLY3(2,1) = 118
      IPOLY3(1,2) = 175
      IPOLY3(2,2) = 160
      IPOLY3(1,3) = 200
      IPOLY3(2,3) = 220
      IPOLY3(1,4) = 200
      IPOLY3(2,4) = 280
      IPOLY3(1,5) = 100
      IPOLY3(2,5) = 340
      IPOLY3(1,6) = 0
      IPOLY3(2,6) = 280
      IPOLY3(1,7) = 0
      IPOLY3(2,7) = 160
      DO 50 J=1,7
      IPOLY3(1,J) = IPOLY3(1,J) + 45
      FPOLY3(1,J) = IPOLY3(1,J) + 245.0
      FPOLY3(2,J) = IPOLY3(2,J)
50    CONTINUE
      CALL COLOR(GREEN)
      CALL POLFI(7,IPOLY3)
      CALL POLF(7,FPOLY3)
C
C     Yellow hexagon
C
      IPOLY3(1,1) = 50
      IPOLY3(2,1) = 130
      IPOLY3(1,2) = 125
      IPOLY3(2,2) = 160
      IPOLY3(1,3) = 150
      IPOLY3(2,3) = 310
      IPOLY3(1,4) = 100
      IPOLY3(2,4) = 340
      IPOLY3(1,5) = 0
      IPOLY3(2,5) = 280
      IPOLY3(1,6) = 0
      IPOLY3(2,6) = 160
      DO 60 J=1,6
      IPOLY3(1,J) = IPOLY3(1,J) + 45
      FPOLY3(1,J) = IPOLY3(1,J) + 245.0
      FPOLY3(2,J) = IPOLY3(2,J)
60    CONTINUE
      CALL COLOR(YELLOW)
      CALL POLFI(6,IPOLY3)
      CALL POLF(6,FPOLY3)
C
C     Blue pentagon
C
      IPOLY3(1,1) = 30
      IPOLY3(2,1) = 142
      IPOLY3(1,2) = 75
      IPOLY3(2,2) = 160
      IPOLY3(1,3) = 50
      IPOLY3(2,3) = 310
      IPOLY3(1,4) = 0
      IPOLY3(2,4) = 280
      IPOLY3(1,5) = 0
      IPOLY3(2,5) = 160
      DO 70 J=1,5
      IPOLY3(1,J) = IPOLY3(1,J) + 45
      FPOLY3(1,J) = IPOLY3(1,J) + 245.0
      FPOLY3(2,J) = IPOLY3(2,J)
70    CONTINUE
      CALL COLOR(BLUE)
      CALL POLFI(5,IPOLY3)
      CALL POLF(5,FPOLY3)
C
C     Magenta quadralateral
C
      IPOLY3(1,1) = 10
      IPOLY3(2,1) = 154
      IPOLY3(1,2) = 25
      IPOLY3(2,2) = 160
      IPOLY3(1,3) = 0
      IPOLY3(2,3) = 220
      IPOLY3(1,4) = 0
      IPOLY3(2,4) = 160
      DO 80 J=1,4
      IPOLY3(1,J) = IPOLY3(1,J) + 45
      FPOLY3(1,J) = IPOLY3(1,J) + 245.0
      FPOLY3(2,J) = IPOLY3(2,J)
80    CONTINUE
      CALL COLOR(MAGENT)
      CALL POLFI(4,IPOLY3)
      CALL POLF(4,FPOLY3)
C
C     Draw shaded hexagons
C
      DO 90 J=1,6
      IPOLY2(1,J) = IPATRN(1,J) + 535
      FPOLY2(1,J) = IPATRN(1,J) + 780.0
      IPOLY2(2,J) = IPATRN(2,J)
      FPOLY2(2,J) = IPATRN(2,J)
      SSHADE(J) = J - 1
90    CONTINUE
      CALL SPLF2I(6,IPOLY2,SSHADE)
      CALL SPLF2(6,FPOLY2,SSHADE)
C
C     Question user about the polygons
C
C     polfi
C
      CALL CLRTXT()
      CALL CMOV2I(50,50)
      STRING = 'You should now see 4 multi-colored regular hexagons with
     1 narrow white borders.  They should all look'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,35)
      STRING = 'approximately the same, except that the edges between th
     1e colors in the two right ones will be'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,20)
      STRING = 'curved and rough.  Does the left hexagon look O.K?  Indi
     1cate good or bad.'
      CALL CHARST(STRING, LEN(STRING))
C
      IF (PRESSD()) THEN
      STR1 = 'sendl  OK'
      STR2 = 'sendls OK'
      ELSE
      STR1 = 'sendl     ?'
      STR2 = 'sendls   ?'
      CALL CMOV2I(15,NEXTDN(0))
      STRING = 'POLFI problems'
      CALL CHARST(STRING, LEN(STRING))
      END IF
      CALL TESTED(STR1,625)
      CALL TESTED(STR2,610)
      CALL CLRTXT()
C
C     polf
C
      CALL CMOV2I(50,50)
      STRING = 'How about the second one from the left?'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,35)
      STRING = 'Indicate good or bad.'
      CALL CHARST(STRING, LEN(STRING))
C
      IF (PRESSD()) THEN
      STR1 = 'sendl  OK'
      STR2 = 'sendfs OK'
      ELSE
      STR1 = 'sendl      ?'
      STR2 = 'sendfs   ?'
      CALL CMOV2I(15,NEXTDN(0))
      STRING = 'POLF incorrect'
      CALL CHARST(STRING, LEN(STRING))
      END IF
      CALL TESTED(STR1,625)
      CALL TESTED(STR2,580)
      CALL CLRTXT()
C
C     splf2i
C
      CALL CMOV2I(50,50)
      STRING = 'The third one from the left?'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,35)
      STRING = 'Indicate good or bad.'
      CALL CHARST(STRING, LEN(STRING))
C
      IF (PRESSD()) THEN
      STR1 = 'sendl  OK'
      STR2 = 'sendls OK'
      STR3 = 'sendss OK'
      ELSE
      STR1 = 'sendl       ?'
      STR2 = 'sendls    ?'
      STR3 = 'sendss   ?'
      CALL CMOV2I(15,NEXTDN(0))
      STRING = 'SPLF2I no good'
      CALL CHARST(STRING, LEN(STRING))
      END IF
      CALL TESTED(STR1,625)
      CALL TESTED(STR2,610)
      CALL TESTED(STR3,640)
      CALL CLRTXT()
C
C     splf2
C
      CALL CMOV2I(50,50)
      STRING = 'And the last (rightmost) one?'
      CALL CHARST(STRING, LEN(STRING))
      CALL CMOV2I(50,35)
      STRING = 'Indicate good or bad.'
      CALL CHARST(STRING, LEN(STRING))
C
      IF (PRESSD()) THEN
      STR1 = 'sendl  OK'
      STR2 = 'sendfs OK'
      STR3 = 'sendss OK'
      ELSE
      STR1 = 'sendl        ?'
      STR2 = 'sendfs    ?'
      STR3 = 'sendss    ?'
      CALL CMOV2I(15,NEXTDN(0))
      STRING = 'SPLF2 does things wrong'
      CALL CHARST(STRING, LEN(STRING))
      END IF
      CALL TESTED(STR1,625)
      CALL TESTED(STR2,580)
      CALL TESTED(STR3,640)
      CALL CLRTXT()
C
      RETURN
      END
