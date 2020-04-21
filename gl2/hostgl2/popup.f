CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C      Installation note:
C    Various Fortran compilers may require different styles of INCLUDEs.
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
$     INCLUDE fgl.h
$     INCLUDE fdevice.h
C
      INTEGER I, J, MNAMES(7), COMAND, POPUP
      INTEGER LINE, POINTS, CIRCLE, RCTGL, RCTGLF, QUIT
      CHARACTER*20 MSTRGS(7)
      LINE = 1
      POINTS = 2
      CIRCLE = 3
      RCTGL = 4
      RCTGLF = 5
      QUIT = 6
C
C       Initialize main menu names and strings (names 1-6 are as per
C       the parameters above, but are easier to init. via the DO loop)
      DO 100 I=1,6
        MNAMES(I) = I
100   CONTINUE
      MNAMES(7) = 0
C
      MSTRGS(1) = 'Line'
      MSTRGS(2) = '100 points'
      MSTRGS(3) = 'Filled circle'
      MSTRGS(4) = 'Outlined rectangle'
      MSTRGS(5) = 'Filled rectangle'
      MSTRGS(6) = 'Quit'
      MSTRGS(7) = ' '
C
      CALL GINIT()
C       background only
      CALL MAPCOL(0,0,0,0)
C       drawing only
      CALL MAPCOL(1,0,255,0)
C       popup background
      CALL MAPCOL(2,0,0,0)
C       popup background over drawing
      CALL MAPCOL(3,0,0,0)
C       popup text only
      CALL MAPCOL(4,255,255,255)
C       popup text over drawing
      CALL MAPCOL(5,255,255,255)
C       popup highlight only
      CALL MAPCOL(6,100,100,100)
C       popup highlight over drawing
      CALL MAPCOL(7,100,100,100)
      CALL SETCUR(0,4,7)
      CALL QDEVIC(LEFTMO)
      CALL TIE(LEFTMO,MOUSEX,MOUSEY)
C
      CALL CURSOF()
      CALL COLOR(0)
      CALL CLEAR()
      CALL CURSON()
      CALL VIEWPO(150,850,50,750)
      CALL ORTHO2(-1.0,1.0,-1.0,1.0)
C
C       loop until quit selected
200   CONTINUE
        COMAND = POPUP(MNAMES,MSTRGS)
        CALL CURSOF()
        CALL COLOR(0)
        CALL CLEAR()
        CALL COLOR(1)
C
        IF (COMAND .EQ. LINE) THEN
          CALL MOVE2(-1.0,-1.0)
          CALL DRAW2(1.0,1.0)
        ELSE IF (COMAND .EQ. POINTS) THEN
          DO 400 I=1,10
            DO 300 J=1,10
              CALL PNT2(REAL(I)/20.0,REAL(J)/20.0)
300         CONTINUE
400       CONTINUE
        ELSE IF (COMAND .EQ. CIRCLE) THEN
          CALL CIRCF(0.0,0.0,0.5)
        ELSE IF (COMAND .EQ. RCTGL) THEN
          CALL RECT(-0.5,-0.5,0.5,0.5)
        ELSE IF (COMAND .EQ. RCTGLF) THEN
          CALL RECTF(-0.5,-0.5,0.5,0.5)
        ELSE IF (COMAND .EQ. QUIT) THEN
	  CALL GINIT()
	  CALL GEXIT()
          GO TO 500
        END IF
C
        CALL CURSON()
C       loop back until quit selected
      GO TO 200
C
500   STOP
      END
C
C
C
      INTEGER FUNCTION POPUP(NAMES,STRNGS)
      INTEGER NAMES(*)
      CHARACTER*20 STRNGS(*)
      INTEGER GETCOL, GETWRI, GETVAL, QTEST
      INTEGER I, MCOUNT, MTOP, MBOTOM, MLEFT, MRIGHT
      INTEGER LASTHL, HIGHLT
      INTEGER*2 DUMMY, X, Y
      INTEGER SVCOLR, SVMASK
      INTEGER*2 LLX, LLY, URX, URY
      INTEGER ILLX, ILLY, IURX, IURY
      LASTHL = -1
C
      CALL QREAD(DUMMY)
      CALL QREAD(X)
      CALL QREAD(Y)
C       save the state of everything
      SVCOLR = GETCOL()
      SVMASK = GETWRI()
      CALL GETVIE(LLX,URX,LLY,URY)
      ILLX = LLX
      IURX = URX
      ILLY = LLY
      IURY = URY
      CALL PUSHMA()
C       now set up to draw the menu
      CALL VIEWPO(0,1023,0,767)
      CALL ORTHO2(-0.5,1023.5,-0.5,767.5)
C       set MCOUNT equal to number of names in menu
      MCOUNT = 0
1000  CONTINUE
        IF (NAMES(MCOUNT+1) .NE. 0) THEN
          MCOUNT = MCOUNT + 1
          GO TO 1000
      END IF
C
      MTOP = Y + MCOUNT * 8
      IF (MTOP .GT. 767) MTOP = 767
      MBOTOM = MTOP - MCOUNT * 16
C
      IF (MBOTOM .LT. 0) THEN
        MBOTOM = 0
        MTOP = MBOTOM + MCOUNT * 16
      END IF
C
      MLEFT = X - 100
      IF (MLEFT .LT. 0) MLEFT = 0
      MRIGHT = MLEFT + 200
C
      IF (MRIGHT .GT. 1023) THEN
        MRIGHT = 1023
        MLEFT = 823
      END IF
C
C       restrict to menu planes
      CALL WRITEM(6)
C       menu background
      CALL COLOR(2)
      CALL CURSOF()
      CALL RECTFI(MLEFT,MBOTOM,MRIGHT,MTOP)
C       menu text
      CALL COLOR(4)
      CALL RECTI(MLEFT,MBOTOM,MRIGHT,MTOP)
      DO 2000 I=1,MCOUNT
        CALL MOVE2I(MLEFT, MTOP - I*16)
        CALL DRAW2I(MRIGHT, MTOP - I*16)
        CALL CMOV2I(MLEFT + 10, MTOP - I*16 + 2)
        CALL CHARST(STRNGS(I),LEN(STRNGS(I)))
2000  CONTINUE
C
      CALL CURSON()
C       loop to highlight potential selection & accept selection
3000  CONTINUE
        X = GETVAL(266)
        Y = GETVAL(267)
C
C         if the cursor is inside the menu
        IF ((MLEFT.LT.X).AND.(X.LT.MRIGHT) .AND. (MBOTOM.LT.Y).AND.(Y.LT
     2.MTOP)) THEN
          HIGHLT = (MTOP - Y)/16
          CALL CURSOF()
C
          IF ((LASTHL.NE.-1) .AND. (LASTHL.NE.HIGHLT)) THEN
            CALL COLOR(2)
            CALL RECTFI(MLEFT+1, MTOP - LASTHL*16 - 15, MRIGHT-1, MTOP -
     2 LASTHL*16 -1)
            CALL COLOR(4)
            CALL CMOV2I(MLEFT + 10, MTOP - 14 - LASTHL*16)
            CALL CHARST(STRNGS(LASTHL+1),LEN(STRNGS(LASTHL+1)))
          END IF
C
          IF (LASTHL.NE.HIGHLT) THEN
            CALL COLOR(6)
            CALL RECTFI(MLEFT+1, MTOP - HIGHLT*16 - 15, MRIGHT-1, MTOP -
     2 HIGHLT*16 -1)
            CALL COLOR(4)
            CALL CMOV2I(MLEFT + 10, MTOP - 14 - HIGHLT*16)
            CALL CHARST(STRNGS(HIGHLT+1),LEN(STRNGS(HIGHLT+1)))
          END IF
C
          LASTHL = HIGHLT
          CALL CURSON()
C
C         if the cursor is outside the menu
        ELSE
          IF (LASTHL.NE.-1) THEN
            CALL CURSOF()
            CALL COLOR(2)
            CALL RECTFI(MLEFT+1, MTOP - LASTHL*16 - 15, MRIGHT-1, MTOP -
     2 LASTHL*16 -1)
            CALL COLOR(4)
            CALL CMOV2I(MLEFT + 10, MTOP - 14 - LASTHL*16)
            CALL CHARST(STRNGS(LASTHL+1),LEN(STRNGS(LASTHL+1)))
            CALL CURSON()
            LASTHL = -1
          END IF
        END IF
C
        IF (QTEST().NE.0) THEN
          CALL QREAD(DUMMY)
          CALL QREAD(X)
          CALL QREAD(Y)
          CALL COLOR(0)
          CALL CURSOF()
          CALL RECTFI(MLEFT,MBOTOM,MRIGHT,MTOP)
          CALL CURSON()
          IF ((MLEFT.LT.X).AND.(X.LT.MRIGHT) .AND. (MBOTOM.LT.Y).AND.(Y.
     2LT.MTOP)) THEN
            X = (MTOP - Y)/16 + 1
          ELSE
            X = 1
          END IF
C             now restore the state to what the user had
          CALL POPMAT()
          CALL COLOR(SVCOLR)
          CALL WRITEM(SVMASK)
          CALL VIEWPO(ILLX,IURX,ILLY,IURY)
          POPUP = NAMES(X)
          GO TO 4000
        END IF
C
C       loop back to highlight potential selection & accept selection
      GO TO 3000
C
C
4000  RETURN
      END
