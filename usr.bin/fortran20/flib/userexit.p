(*****************************************************************************)
(*                                                                           *)
(*                            File: USEREXIT.TEXT                            *)
(*                                                                           *)
(*		The routine userexit is called before the error message is           *)
(*		printed or the files are closed prior to a fortran error exit.       *)
(*      This library module is the dummy entry for userexit if the user      *)
(*	    has not supplied his/her own.                                        *)
(*                                                                           *)
(*****************************************************************************)

{$%+} {$R-} {$I-}

unit %userexit;

interface

procedure userexit(var addr:longint; var n:integer);

implementation
procedure userexit{(var addr:longint; var n:integer)};
begin
end;
end. {userexit}

