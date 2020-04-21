(*****************************************************************************)
(*                                                                           *)
(*                            File: FLIBLWR.TEXT                             *)
(*                                                                           *)
(*           (C) Copyright 1982, 1985 Silicon Valley Software, Inc.          *)
(*                                                                           *)
(*                            All Rights Reserved.               10-May-85   *)
(*                                                                           *)
(*****************************************************************************)


{$%+} {$R-} {$I-}

unit %fliblwr;

interface

uses {$u flibinit} %flibinit,
     {$u flibrec}  %flibrec,
     {$u fliblw}   %fliblw,
     {$u flibwr}   %flibwr;

implementation

{ Write a real, list directed }

procedure %_wrlr4(freal: real4);
  var lreal: real4;
begin {%_wrlr4}
lreal := abs(freal); 
if (1 <= lreal) and (lreal <= 9.99999)
then begin 
  %needchars(9); edlet := 'F'; AorZFlag := FALSE;
  edd := 6; %putr4(freal); 
  end
else begin
  pval := 1; %needchars(13); edlet := 'E'; AorZFlag := FALSE;
  edd := 6; ede := 2; %putr4(freal); pval := 0;
  end;
end; {%_wrlr4}

{ Write a complex, list directed }

procedure %_wrlc8(fimag,freal: real4);
begin {%_wrlc8}
%needchars(29); edw := 13; %putch('('); 
pval := 1; edlet := 'E'; AorZFlag := FALSE; edd := 6; ede := 2;
%putr4(freal); %putch(','); %putr4(fimag); %putch(')'); pval := 0;
end; {%_wrlc8}

{ Write a real array, list directed }

procedure %_walr4(var frealarray: real4array; count: int4);
  var ctr: longint;
begin {%_walr4}
for ctr := 1 to count do
  %_wrlr4(frealarray[ctr]);
end; {%_walr4}

{ Write a complex array, list directed }

procedure %_walc8(var fcomplexarray: complexarray; count: int4);
  var ctr: longint;
begin {%_walc8}
for ctr := 1 to count do
  %_wrlc8(fcomplexarray[ctr].imagpart,fcomplexarray[ctr].realpart);
end; {%_walc8}

end. {%fliblwr}

