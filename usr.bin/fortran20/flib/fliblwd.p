(*****************************************************************************)
(*                                                                           *)
(*                            File: FLIBLWD.TEXT                             *)
(*                                                                           *)
(*           (C) Copyright 1982, 1985 Silicon Valley Software, Inc.          *)
(*                                                                           *)
(*                            All Rights Reserved.               10-May-85   *)
(*                                                                           *)
(*****************************************************************************)


{$%+} {$R-} {$I-}

unit %fliblwd;

interface

uses {$u flibinit} %flibinit,
     {$u flibrec}  %flibrec,
     {$u fliblw}   %fliblw,
     {$u flibwd}   %flibwd;

implementation

{ Write a double, list directed }

procedure %_wrlr8(freal: real8);
  var lreal: real8;
begin {%_wrlr8}
lreal := abs(freal); 
if (1 <= lreal) and (lreal < 9.999999)
then begin 
  %needchars(17); edlet := 'F'; AorZFlag := FALSE;
  edd := 14; %putr8(freal); 
  end
else begin
  pval := 1; %needchars(22); edlet := 'E'; AorZFlag := FALSE;
  edd := 14; ede := 3; %putr8(freal); pval := 0;
  end;
end; {%_wrlr8}

procedure %_walr8(var frealarray: real8array; count: int4);
  var ctr: longint;
begin {%_walr8}
for ctr := 1 to count do
  %_wrlr8(frealarray[ctr]);
end; {%_walr8}

end. {%fliblwd}

