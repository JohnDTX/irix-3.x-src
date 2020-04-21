(*UNIT util_dcl;*)

(* include file gdcl.h *)

{ Copyright 1981, Richard E. James III. }
{$%+}

(*INTERFACE*)

CONST
      r_min_exp = -127;
      r_max_exp = 127;
      d_min_exp = -1023;
      d_max_exp = 1023;
 {??ck min/max exp}

TYPE 
     LONGINT = INTEGER;		(* for upas (GB) *)
     TYPES = 1..5;
     ERROR_TYPE=(none, underflow, overflow, domain_error, 
                 not_a_number, infeasible); 
     SNGL = RECORD
       CASE BOOLEAN OF
         TRUE: (REEL: REAL);
         FALSE: (INT: LONGINT)
     END;
     FAKE_REAL = LONGINT;
     DBLE = ARRAY [1..2] OF LONGINT;
     COMPLEX = RECORD
       RE: REAL;
       IM: REAL
     END;
     UNP_REAL = PACKED RECORD		(* PACKED for upas *)
       UPPER: LONGINT;
       LOWER: LONGINT;
       EXP: -32767..32767;			(* must be 16-bit entity *)
       SGN: -128..127;				(* must be 8-bit entity *)
       KND: 0..255				(* must be 8-bit entity *)
     END;
     SIX_CHAR_NAME = PACKED ARRAY[1..6] OF CHAR;

(*IMPLEMENTATION*)

(*END.*)


