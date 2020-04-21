(*****************************************************************************)
(*                                                                           *)
(*                             File: FLIBWD.TEXT                             *)
(*                                                                           *)
(*           (C) Copyright 1981, 1985 Silicon Valley Software, Inc.          *)
(*                                                                           *)
(*                            All Rights Reserved.               10-May-85   *)
(*                                                                           *)
(*****************************************************************************)


{$%+} {$R-} {$I-} {$M-}

unit %flibwd;

interface

uses {$u flibinit} %flibinit,
     {$u flibrec}  %flibrec;

procedure %putr8(freal: real8);

implementation

function %_dpwr10(n: integer): double; external;
function %_dsgnbt(val: double): Boolean; external;

procedure %putr8{*freal: real8*};
  const NUMDECDIGITS = 16; NUMEXPDIGITS = 8; EXPGUESS = 32;
  var NegFlag, ZeroFlag, AllStars: Boolean; normval: real8; 
      eddctr, lcol, i, leftw, leftb, exp, truncval,digcols, prntcols: integer;
      buf: array[1..NUMDECDIGITS] of char; addexp: integer;
      expbuf: packed array[1..NUMEXPDIGITS] of char;
      savededlet: char; savededd, savededw, savedpval: integer;
  
  procedure %prcol8;
  begin {%prcol8}
  lcol := lcol + 1;
  if lcol <= NUMDECDIGITS then %putch(buf[lcol]) else %putch('0');
  end; {%prcol8}

begin {%putr8}

{ Check for + or - infinity and NAN }

if IsNum(freal)
then begin
  
  { Compute sign, 1.0 <= normalized value < 10.0, and exponent + 1 of value }
  { This is the exponent of the digits if printed in .ddddd form.           }
  
  if freal < 0
  then begin NegFlag := TRUE; freal := -freal; end
  else NegFlag := FALSE;
  
  savededlet := edlet; 
  if edlet = 'G'
  then begin { Simulate another edit enviroment }
    savededd := edd; savededw := edw; savedpval := pval;
    if (freal < 1D-1) or (freal >= %_dpwr10(edd))
    then edlet := 'E'
    else begin
      edlet := 'F'; pval := 0; { The scale factor "has no effect" }
      if EdePresent then edw := edw - ede - 2 else edw := edw - 4;
      end;
    end;
  
  exp := 0; normval := freal;
  if normval <> 0
  then begin
    if normval >= 10
    then begin
      i := EXPGUESS; { Must be a power of 2 }
      repeat
        if normval >= %_dpwr10(i)
        then begin
          exp := exp + i; normval := freal/%_dpwr10(exp);
          end
        else i := i div 2;
      until i = 0;
      end
    else
      if normval < 1
      then begin
        if normval < 1d-307
        then begin freal := freal*1d32; addexp := -32; end
        else addexp := 0;
        i := EXPGUESS; { Must be a power of 2 }
        repeat
          if normval < 1/%_dpwr10(i - 1)
          then begin
            exp := exp - i; normval := freal*%_dpwr10(-exp);
            end
          else i := i div 2;
        until i = 0;
        exp := exp + addexp;
        end;
    exp := exp + 1; { exp + pval is num of printing digits left of . for 'F' }
    end;
  
  { Time to set edd for 'G' editing }
  
  if (edlet = 'F') and (savededlet = 'G') then edd := edd - exp;
  
  { Round by adding 1/2 to digit just beyond printing decimal places }
  
  if edlet = 'F'
  then prntcols := exp + pval + edd { Number of printing digits for F format }
  else { (edlet = 'E') or (edlet = 'D') }
    if pval > 0
    then prntcols := edd + 1
    else 
      if pval = 0
      then prntcols := edd
      else
        if pval > -edd  { Otherwise error condition }
        then prntcols := edd + pval;
  
  { Note: prntcols = 0 before rounding may be prntcols = 1 after rounding }
  
  if prntcols >= 0  { May not print at all for F format with large -exp }
  then
    if prntcols >= NUMDECDIGITS { Round as little as possible }
    then normval := normval + 5/%_dpwr10(NUMDECDIGITS)
    else normval := normval + 5/%_dpwr10(prntcols);
  
  { Fix normalization if rounding messed it up }
  
  if normval > 10
  then begin
    exp := exp + 1; normval := normval / 10;
    if edlet = 'F' 
    then begin
      prntcols := prntcols + 1;
      if savededlet = 'G' then edd := edd - 1;
      end;
    end;
  
  { Fill in the characters of buf }
  
  ZeroFlag := TRUE;
  for i := 1 to NUMDECDIGITS do begin
    truncval := trunc(normval);
    if truncval > 9 then truncval := 9;
    if (truncval <> 0) and (i <= prntcols) then ZeroFlag := FALSE;
    buf[i] := chr(truncval + ord('0')); 
    normval := (normval - truncval)*10;
    end;
  if ZeroFlag then NegFlag := FALSE;
  
  if edlet = 'F' 
  then begin
    exp := exp + pval;  { External value different from internal value }
    if ZeroFlag and (exp > 0) then exp := 0;
    leftw := edw - edd - 1;
    if leftw < 0 
    then leftb := -1 {%error(42)}
    else begin
      if exp <= 0 then leftb := leftw else leftb := leftw - exp;
      if NegFlag or PrintOptionalPlus then leftb := leftb - 1;
      if (edd = 0) and (exp <= 0) then leftb := leftb - 1;
      end;
    if leftb < 0
    then %putstars { Too big to fit into field width }
    else begin
      %putblanks(leftb); 
      if NegFlag 
      then %putch('-')
      else
        if PrintOptionalPlus then %putch('+');
      lcol := 0;
      if (edd = 0) and (exp <= 0)
      then %putch('0')
      else
        while exp > 0 do begin
          %prcol8; exp := exp - 1;
          end;
      %putch('.'); eddctr := edd;
      while (exp < 0) and (eddctr > 0) do begin
        %putch('0'); exp := exp + 1; eddctr := eddctr - 1;
        end;
      for i := 1 to eddctr do
        %prcol8;
      end;
    end
  else
    if (edlet = 'E') or (edlet = 'D')
    then begin
      AllStars := FALSE;
      if (pval <= (- edd)) or (pval > (edd + 1))
      then AllStars := TRUE; {%error(43)}
      if ZeroFlag 
      then exp := 0
      else exp := exp - pval; { Set printable exp }
      
      { Determine if exponent fits into field and calculate the number of }
      { digit columns, those to be used for [-][0].ddddd                  }
      
      if EdePresent 
      then begin
        if abs(exp) >= pwroften(ede) then AllStars := TRUE;
        digcols := edw - ede - 2;
        end
      else begin
        if abs(exp) >= 1000 then AllStars := TRUE;
        digcols := edw - 4;
        end;
      
      { if pval >= 1 then need edd + 2 places for digits else need edd + 1 }
      
      prntcols := edd + 1 + ord(pval>=1);
      if NegFlag or PrintOptionalPlus then prntcols := prntcols + 1;
      if digcols < prntcols then AllStars := TRUE;
      if AllStars
      then %putstars
      else begin
        %putblanks(digcols - prntcols); 
        if NegFlag 
        then %putch('-')
        else
          if PrintOptionalPlus then %putch('+');
        lcol := 0;
        for i := 1 to pval do  { Digits to the left of decimal point }
          if ZeroFlag and (i <> pval) then %putch(' ') else %prcol8;
        %putch('.');
        for i := -1 downto pval do  { Zeros to the right of decimal point }
          %putch('0');
        
        { Cases in which edd + 1 digits print, except if all left of point }
        
        if (pval >= 1) and (pval < (edd + 1)) then %prcol8;
        
        { Significant digits after decimal point }
        
        for i := 1 to edd - abs(pval) do
          %prcol8;
        
        { Print exponent part }
        
        if EdePresent 
        then %putch(edlet)
        else
          if abs(exp) < 100
          then begin %putch(edlet); ede := 2; end { OK to use ede as temp since }
          else ede := 3;                         { not EdePresent.             }
        if exp < 0 then begin exp := - exp; %putch('-'); end else %putch('+');
        lcol := NUMEXPDIGITS;
        repeat
          expbuf[lcol] := chr((exp mod 10) + ord('0'));
          exp := exp div 10; lcol := lcol - 1;
        until exp = 0;
        for i := 1 to ede - (NUMEXPDIGITS - lcol) do
          %putch('0');
        for i := lcol + 1 to NUMEXPDIGITS do
          %putch(expbuf[i]);
        end;
      end
    else %error(44);
  if savededlet = 'G'
  then begin { print trailing blanks and restore true edit descriptor }
    if edlet = 'F' 
    then
      if EdePresent then %putblanks(ede + 2) else %putblanks(4);
    edlet := savededlet; edd := savededd; edw := savededw; pval := savedpval;
    end;
  end { IsNum }
else
  if IsNan(freal)
  then %fillfield('?')
  else
    if %_dsgnbt(freal)
    then %fillfield('-')
    else %fillfield('+');
end; {%putr8}

end. {%flibwd}

                                                                                                                    