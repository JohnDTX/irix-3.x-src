FUNCTION $t_logb(arg:REAL) : INTEGER;EXTERNAL;

FUNCTION $t_error (routine_name:SIX_CHAR_NAME; error:ERROR_TYPE) : REAL;
EXTERNAL;

FUNCTION $t_halfsz (arg:REAL) : REAL;EXTERNAL;

PROCEDURE $rep_error (routine_name:SIX_CHAR_NAME;  error:ERROR_TYPE);EXTERNAL;

PROCEDURE $t_pack ( value: REAL;  power_of_2: INTEGER;
                   VAR result: REAL;  var error: ERROR_TYPE );EXTERNAL;

PROCEDURE $t_unpack( value: REAL;
                    VAR mantissa:REAL;   VAR exponent: INTEGER);EXTERNAL;

FUNCTION $t_check ( VAR error: ERROR_TYPE; value: REAL): ERROR_TYPE;EXTERNAL;
