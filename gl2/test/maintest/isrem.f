      LOGICAL FUNCTION ISREM()
C
#ifdef REMOTE
      ISREM = .TRUE.
#else  LOCAL
      ISREM = .FALSE.
#endif LOCAL
      RETURN
      END
