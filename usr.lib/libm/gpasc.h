PROCEDURE $pi_reduce
  (arg:REAL;
   VAR result:REAL;
   VAR multiple:INTEGER;
   VAR error:ERROR_TYPE);EXTERNAL;

PROCEDURE $e_reduce
  (arg:REAL;
   VAR p:REAL;
   VAR q:REAL;
   VAR multiple:INTEGER;
   VAR error:ERROR_TYPE);EXTERNAL;

PROCEDURE $t_atan(VAR result:REAL; arg:REAL; VAR error:ERROR_TYPE);EXTERNAL;

PROCEDURE $t_exp(VAR result:REAL; arg:REAL; VAR error:ERROR_TYPE);EXTERNAL;

PROCEDURE $t_log(VAR result:REAL; arg:REAL; VAR error:ERROR_TYPE);EXTERNAL;

PROCEDURE $t_sqrt
  (VAR y:REAL;
   x:REAL;
   VAR error:ERROR_TYPE);EXTERNAL;

FUNCTION $sin ( arg: REAL ) : REAL;EXTERNAL;

FUNCTION $cos ( arg: REAL ) : REAL;EXTERNAL;

FUNCTION $atan ( arg: REAL ) : REAL;EXTERNAL;

FUNCTION $exp ( arg: REAL ) : REAL;EXTERNAL;

FUNCTION $log ( arg: REAL ) : REAL;EXTERNAL;

FUNCTION $sqrt ( arg: REAL ) : REAL;EXTERNAL;

