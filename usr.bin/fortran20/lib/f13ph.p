{ File: f13.text }
{ Date: 24-Jul-85 }


{ Replaces: GDCL, GUR, GPASC, GFTN and IUPI in PASLIB and FTNLIB }


{ Copyright Richard E. James III, 1981 }
{ Copyright 1981, 1982, 1983, 1984, 1985 Silicon Valley Software, Inc. }

{$R-} {$I-} {$M-} {$%+}

unit %f13;

interface

const MALE = 0;   { MC68000 }
      FEMALE = 1; { NS32000 }
      
      SEX = MALE;
      
      R_MIN_EXP = -127;
      R_MAX_EXP = 127;
      D_MIN_EXP = -1023;
      D_MAX_EXP = 1023;
 {??ck min/max exp}

type types = 1..5;
     error_type = (NONE,UNDERFLOW,OVERFLOW,DOMAIN_ERROR,
                   NOT_A_NUMBER,INFEASIBLE);
     unp_real = record upper: longint;
                       lower: longint;
                       exp: integer;
                       sgn: -128..127;
                       knd: types;
                end;
     alfa6 = packed array[1..6] of char;
     complex = record re: real; im: real; end;

function %gennan4(i: longint): real;

implementation


{-----------------------------------------------------------gennan4---}
function %gennan4{i: longint): real};
  var l: longint; a: real;
begin
l := $7f800000 or i; moveleft(l,a,4); %gennan4 := a;
end; {%gennan4}

procedure %_cerr(var result: complex; var routine_name: alfa6;
                 len: longint; var error: longint);
begin 
result.re := %gennan4(error); result.im := result.re;
end; {%_cerr} 

{  Routine to test for illegal numbers.
Return an error code as function result & 1st parameter.  }
  
function %c_cck(var error: longint; var value: complex): longint;
begin
if IsNan(value.re) or IsNan(value.im)
then begin error := ord(NOT_A_NUMBER); %c_cck := 0; {Fortran .FALSE.} end
else begin error := ord(NONE); %c_cck := 1; {Fortran .TRUE.} end;
end; {%c_cck}

end. {%f13}

