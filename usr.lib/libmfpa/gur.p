(*UNIT util_real;*)
MODULE $util_real;

{ Copyright Richard E. James III, 1982 }

INCLUDE 'gdcl.h';

(*INTERFACE

USES {$U gdcl.obj} util_dcl;
*)

FUNCTION $t_logb(arg:REAL) : INTEGER;FORWARD;
FUNCTION $t_error (routine_name:SIX_CHAR_NAME; error:ERROR_TYPE) : REAL;
	FORWARD;
FUNCTION $t_halfsz (arg:REAL) : REAL;FORWARD;
PROCEDURE $rep_error (routine_name:SIX_CHAR_NAME;  error:ERROR_TYPE);FORWARD;
PROCEDURE $t_pack ( value: REAL;  power_of_2: INTEGER;
                   VAR result: REAL;  var error: ERROR_TYPE );FORWARD;
PROCEDURE $t_unpack( value: REAL;
                    VAR mantissa:REAL;   VAR exponent: INTEGER);FORWARD;
FUNCTION $t_check ( VAR error: ERROR_TYPE; value: REAL): ERROR_TYPE;FORWARD;

(*IMPLEMENTATION*)


PROCEDURE $pf_pack(VAR val: REAL;  VAR ur: UNP_REAL);   EXTERNAL;
PROCEDURE $pf_unpk(    val: REAL;  VAR ur: UNP_REAL);   EXTERNAL;

{------------------------------------------------------------unpack---}

PROCEDURE $t_unpack
  {value:REAL;                     { In: value to unpack }
  {VAR mantissa:REAL;              { Out: unpacked mantissa }
  {VAR exponent:INTEGER )          { Out: unpacked exponent }  ;

    {  Routine to unpack real numbers, returning a signed
       mantissa in the range (-2,-1] or [1,2), and an exponent
       in the range [-127,+127] for single.    }
VAR
   u:UNP_REAL;
BEGIN
   $pf_unpk(value, u);
   exponent := u.exp + 23;
   u.exp := -23;
   $pf_pack(mantissa, u);
END;

{------------------------------------------------------------logb-----}

FUNCTION $t_logb{(arg:REAL) : INTEGER};
VAR
   u:UNP_REAL;

    {  Routine to find the log (base 2) of a single precision number. }
BEGIN
   $pf_unpk(arg, u);
   $t_logb := u.exp + 23;
END;

{------------------------------------------------------------pack-----}

PROCEDURE $t_pack
  {value:REAL;               { In: value to multipy by power of 2 }
  {power_of_2:INTEGER;       { In: the power of 2 }
  {VAR result:REAL;          { Out: scaled value }
  {VAR error:ERROR_TYPE)     { Out: error flag }   ;

    {  Opposite of unpack.  }
VAR
   u:UNP_REAL;
BEGIN
   $pf_unpk(value, u);
   u.exp := u.exp + power_of_2;
   IF          u.exp < r_min_exp - 23 THEN BEGIN
      error := underflow ;
      result := 0;
   END ELSE IF u.exp > r_max_exp - 23 THEN BEGIN
      error := overflow;
      result :=(* $7F800000*) 2139095040;
   END ELSE      $pf_pack(result, u);
END;

{------------------------------------------------------------halfsz---}

FUNCTION $t_halfsz
{ (arg:REAL) }                   { In: value to round up }
{ : REAL     }  ;                { Out: rounded value }
VAR
   temp:REAL;
   u:UNP_REAL;
BEGIN
   $pf_unpk(arg, u);
   u.lower := (u.lower + 2048) DIV 4096;
   u.exp := u.exp + 12;
   $pf_pack(temp, u);
   $t_halfsz := temp;
END;

{------------------------------------------------------------rep_error}

PROCEDURE $rep_error {routine_name:SIX_CHAR_NAME;  error:ERROR_TYPE};
VAR
   i:INTEGER;
   error_msg:PACKED ARRAY [1..24] OF CHAR;
BEGIN
   error_msg := 'Error number x in yyyyyy';
   for i:=1 TO 6 DO  error_msg[18+i] := routine_name[i];
   error_msg[14] := chr( ord('0') + ord(error) );
   (*writeln(error_msg);*)   {??----change this----}
END;

{------------------------------------------------------------error----}

FUNCTION $t_error{(routine_name:SIX_CHAR_NAME; error:ERROR_TYPE) : REAL};
BEGIN
   $rep_error(routine_name, error);
   $t_error := 0.0;
END;

{------------------------------------------------------------check----}

FUNCTION $t_check  { (VAR error:ERROR_TYPE; value:REAL) : ERROR_TYPE };
VAR
   u:UNP_REAL;
BEGIN
   $pf_unpk(value, u);
   IF u.knd = 5 THEN  error:=not_a_number
                ELSE  error:=none;
   $t_check := error;
END;

(*END.*)
.



