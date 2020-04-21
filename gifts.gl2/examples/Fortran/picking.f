CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C      Installation note:
C    Various Fortran compilers may require different styles of INCLUDEs.
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
$     INCLUDE /usr/include/fgl.h
$     INCLUDE /usr/include/fdevice.h
C
      INTEGER*2 NAMBUF(50)
      INTEGER*2 VAL, I, J, K, L
      INTEGER TYPE, NUMPIC
C
      CALL GINIT()
C
      CALL QDEVIC(MOUSE3)
      CALL QDEVIC(MOUSE2)
C
      CALL MAKEOB(1)
        CALL COLOR(RED)
C         load the name "1" on the name stack
        CALL LOADNA(1)
        CALL RECTFI(20,20,100,100)
C         load the name "2", replacing "1"
        CALL LOADNA(2)
C         push the name "3", so the stack has "3 2"
        CALL PUSHNA(3)
        CALL CIRCI(50,500,50)
C         replace "3" with "4", so the stack has "4 2"
        CALL LOADNA(4)
        CALL CIRCI(50,530,60)
      CALL CLOSEO()
C
      CALL COLOR(BLACK)
      CALL CLEAR()
C       draw the object on the screen
      CALL CALLOB(1)
C
C       loop until the left mouse button is pushed
100   CONTINUE
        TYPE = QREAD(VAL)
C         try again if the event was a button release
        IF (VAL .EQ. 0) GO TO 100
C         if the left mouse button is pushed, the program exits
        IF (TYPE .EQ. MOUSE3) THEN
          CALL GEXIT()
          GO TO 400
        END IF
C         if the middle mouse button is pushed, tghe IRIS enters pick mode
        IF (TYPE .EQ. MOUSE2) THEN
          CALL PICK(NAMBUF,50)
C           restate the projection transformation for the object
          CALL ORTHO2(-0.5, XMAXSC+0.5, -0.5, YMAXSC+0.5)
C           call the object (no actual drawing takes place)
          CALL CALLOB(1)
C           print out the number of hits and a name-list for each hit
          NUMPIC = ENDPIC(NAMBUF)
          PRINT *, 'Hits: ', NUMPIC
          J = 1
          DO 300 I=1,NUMPIC
            K = NAMBUF(J)
            J = J + 1
            PRINT *,K
            DO 200 L=1,K
              PRINT *,'   ',NAMBUF(J)
              J = J + 1
200         CONTINUE
300       CONTINUE
        END IF
C       loop until the left mouse button is pushed
      GO TO 100
400   CONTINUE
      STOP
      END
