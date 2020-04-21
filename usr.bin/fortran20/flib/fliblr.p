(*****************************************************************************)
(*                                                                           *)
(*                             File: FLIBLR.TEXT                             *)
(*                                                                           *)
(*           (C) Copyright 1982, 1985 Silicon Valley Software, Inc.          *)
(*                                                                           *)
(*                            All Rights Reserved.               10-May-85   *)
(*                                                                           *)
(*****************************************************************************)


{$%+} {$R-} {$I-}

unit %fliblr;

interface

uses {$u flibinit} %flibinit,
     {$u flibrec}  %flibrec,
     {$u fliblw}   %fliblw;

implementation

function %_dpwr10(n: integer): double; external;


{ Scanner for list directed Read.                     }
{ ListScan processes:                                 }
{  if FirstItem                                       }
{       blank* value  or                              }
{       nothing (if followed by a nonblank separator) }
{  else (not FirstItem)                               }
{       separator value  or                           }
{       separator (if followed by another separator)  }

function %ListScan: Boolean;
  const DIGITMAX = 30;
  var Done: Boolean; k: integer;
      digits: array[1..DIGITMAX] of 0..9;
      llistch: char; lrecbufp: integer; lint: integer;

  { Gets next nonblank character out of list of formatted records }
  
  procedure %listgetnonblank;
  begin {%listgetnonblank}
  while listch = ' ' do
    if recbufp <= recbuflen 
    then begin listch := chr(recbuf[recbufp]); recbufp := recbufp + 1; end
    else begin %nextrec; if EofFlag then listch := '/'; end;
  end; {%listgetnonblank}
  
  procedure %fractionpart;
    var Negative: Boolean; scale, i, exp: integer;
  begin {%fractionpart}
  scale := 0;
  if listch = '.'
  then begin
    %getch(listch);
    itemtoken := REALITEM; realval := 0;
    for i := 1 to k do
      realval := realval*10 + digits[i];
    while (listch >= '0') and (listch <= '9') do begin
      realval :=  realval*10 + ord(listch) - ord('0');
      scale := scale - 1; %getch(listch);
      end;
    end; { digits . digits }
  if (itemtoken = INTITEM) and (listch <> 'E') and (listch <> 'D')
                           and (listch <> 'e') and (listch <> 'd')
  then begin { calcintval }
    intval := 0;
    for i := 1 to k do
      intval := intval * 10 + digits[i];
    end;
  if (listch = 'E') or (listch = 'D') or (listch = 'e') or (listch = 'd')
  then begin
    if itemtoken = INTITEM
    then begin
      itemtoken := REALITEM; realval := 0;
      for i := 1 to k do
        realval := realval*10 + digits[i];
      end;
    %getch(listch); Negative := FALSE;
    if listch = '+' 
    then %getch(listch)
    else 
      if listch = '-'
      then begin %getch(listch); Negative := TRUE; end;
    if (listch < '0') and (listch > '9') then %error(87);
    exp := 0;
    while (listch >= '0') and (listch <= '9') do begin
      exp := exp * 10 + ord(listch) - ord('0'); %getch(listch);
      end;
    if Negative then exp := - exp;
    scale := scale + exp;
    end; { listch = 'E' }
  if itemtoken = REALITEM
  then 
    if scale < 0
    then realval := realval/%_dpwr10(-scale)
    else
      if scale > 0
      then realval := realval*%_dpwr10(scale);
  end; {%fractionpart}
  
  procedure %initialdigitnumeric;
  begin {%initialdigitnumeric}
  itemtoken := INTITEM; k := 0;
  repeat
    if k < DIGITMAX then k := k + 1 else %error(88); 
    digits[k] := ord(listch) - ord('0'); %getch(listch);
  until (listch < '0') or (listch > '9');
  %fractionpart;
  end; {%initialdigitnumeric}
  
  procedure %numeric;
    var NegFlag: Boolean;
  begin {%numeric}
  NegFlag := listch = '-';
  if (listch = '+') or (listch = '-') then %getch(listch);
  if (listch >= '0') and (listch <= '9')
  then %initialdigitnumeric
  else 
    if listch = '.'
    then begin k := 0; %fractionpart; end
    else %error(89);
  if NegFlag
  then
    if itemtoken = INTITEM then intval := - intval else realval := -realval;
  end; {%numeric}
  
  procedure %scanlogical;
  begin {%scanlogical}
  itemtoken := LOGITEM;
  if (listch = 'T') or (listch = 't') 
  then intval := 1 
  else begin
    intval := 0;
    if (listch <> 'F') and (listch <> 'f') then %error(90);
    end;
  repeat
    %getch(listch);
  until (listch = ' ') or (listch = ',') or (listch = '/');
  end; {%scanlogical}
  
  
begin {%ListScan}
if itemcnt > 1
then itemcnt := itemcnt - 1
else begin
  %listgetnonblank;
  
  { If FirstItem, than an initial nonblank value separator is a null item. }
  { If not FirstItem then a nonblank value separator is either a comma,    }
  { which is to be skipped or a slash, which may as well be processed as   }
  { a null item.                                                           }
  
  if (not FirstItem) and (listch = ',') 
  then begin %getch(listch); %listgetnonblank; end;
  
  { Should now have first character of value or a value separator in listch. }
  { See if there is a repeat factor.                                         }
  
  itemcnt := 1; { Unless otherwise discovered }
  if (listch >= '0') and (listch <= '9')
  then begin { Possible repeat factor, possible data value }
    { Prepare to backup if not repeat factor }
    lrecbufp := recbufp; llistch := listch;
    lint := 0;
    repeat
      lint := (10 * lint - ord('0')) + ord(listch);
      %getch(listch);
    until (listch < '0') or (listch > '9');
    if listch = '*'
    then begin { Repeat factor }
      itemcnt := lint; %getch(listch); 
      if itemcnt <= 0 then %error(91);
      end
    else begin { Backup } listch := llistch; recbufp := lrecbufp; end;
    end;
  
  { Itemcnt is now set, process the actual value or separator }
  
  if (listch = '+') or (listch = '-')
  then %numeric
  else
    if (listch >= '0') and (listch <= '9')
    then %initialdigitnumeric
    else
      if listch = '.'
      then begin { Either numeric value or logical value }
        %getch(listch);
        if (listch >= '0') and (listch <= '9')
        then begin { Numeric value }
          recbufp := recbufp - 1; listch := '.'; k := 0; %fractionpart; 
          end
        else %scanlogical;
        end
      else
        if (listch = ',') or (listch = ' ')
        then itemtoken := NULLITEM
        else
          if listch = '/'
          then begin itemcnt := 32000; itemtoken := NULLITEM; end
          else
            if listch = ''''
            then begin { Character item }
              charlen := 0; Done := FALSE;
              repeat
                while (recbufp > recbuflen) and (not EofFlag) do
                  %nextrec;
                listch := chr(recbuf[recbufp]); recbufp := recbufp + 1; 
                while (listch <> '''') and (not EofFlag) do begin
                  charlen := charlen + 1; charval[charlen] := listch;
                  while (recbufp > recbuflen) and (not EofFlag) do
                    %nextrec;
                  listch := chr(recbuf[recbufp]); recbufp := recbufp + 1; 
                  end;
                %getch(listch);      { Found first ', is there another }
                if listch = '''' 
                then begin 
                  charlen := charlen + 1; charval[charlen] := listch; 
                  end
                else Done := TRUE;
              until Done;
              itemtoken := CHARITEM;
              end
            else 
              if listch = '('
              then begin { Complex item }
                %getch(listch); %listgetnonblank; %numeric;
                if itemtoken = INTITEM
                then complexval.realpart := intval
                else complexval.realpart := realval;
                %listgetnonblank;
                if listch = ',' then %getch(listch) else %error(92);
                %listgetnonblank; %numeric;
                if itemtoken = INTITEM
                then complexval.imagpart := intval
                else complexval.imagpart := realval;
                %listgetnonblank;
                if listch = ')' then %getch(listch) else %error(93);
                itemtoken := COMPLEXITEM;
                end
              else %scanlogical;
  end;
FirstItem := FALSE; %ListScan := itemtoken <> NULLITEM;
end; {%ListScan}


{ Initialize external read statement (sequential access) }

procedure %_ixlrd(unitnum: longint);
begin {%_ixlrd}
Reading := TRUE; EofFlag := FALSE; %ixl(unitnum); 
if curunit <> nil 
then 
  if curunit^.Seqen then %forceeofiflastwritten;
%nextrec;
FirstItem := TRUE; itemcnt := 1; listch := ' '; { Harmless look ahead }
end; {%_ixlrd}


{ Initialize external read statement (sequential access), UNIT=* }

procedure %_ixlrdd;
begin {%_ixlrdd}
%_ixlrd(0);
end; {%_ixlrdd}


{ Terminate read statement }

procedure %_tlrd(fiostat: plongint; errexit, endexit: pcodearray);
begin {%_tlrd}
if curunit <> nil
then begin
  if curunit^.Seqen and EofFlag then curunit^.PastEndFile := TRUE;
  curunit^.lastop := READOP;
  end;
if EofFlag and (endexit[1] = nil) then %error(-601);
%termiostmt(fiostat,errexit);
if EofFlag and (endexit[1] <> nil) and (errornumber = 0)
then begin
  if fiostat <> nil then fiostat^ := -1;
  if ENVIRONMENT = GENIX
  then endexit[-1] := endexit[1]
  else endexit[0] := endexit[1];
  end;
end; {%_tlrd}


{ Read an integer }

procedure %_rdli4(var fint: int4);
begin {%_rdli4}
if %ListScan
then
  if itemtoken = INTITEM then fint := intval else %error(101);
end; {%_rdli4}

procedure %_rdli2(var fint: int2);
begin {%_rdli2}
if %ListScan
then
  if itemtoken = INTITEM then fint := intval else %error(101);
end; {%_rdli2}

procedure %_rdli1(var fint: int1);
begin {%_rdli1}
if %ListScan
then
  if itemtoken = INTITEM then fint := intval else %error(101);
end; {%_rdli1}


{ Read a double }

procedure %_rdlr8(var freal: real8);
begin {%_rdlr8}
if %ListScan
then
  if itemtoken = INTITEM 
  then freal := intval { Free float }
  else
    if itemtoken = REALITEM
    then freal := realval 
    else %error(102);
end; {%_rdlr8}

{ Read a real }

procedure %_rdlr4(var freal: real4);
begin {%_rdlr4}
if %ListScan
then
  if itemtoken = INTITEM 
  then freal := intval
  else
    if itemtoken = REALITEM
    then freal := realval
    else %error(102);
end; {%_rdlr4}


{ Read a logical }

procedure %_rdll4(var flog: int4);
begin {%_rdll4}
if %ListScan
then
  if itemtoken = LOGITEM then flog := intval else %error(103);
end; {%_rdll4}

procedure %_rdll2(var flog: int2);
begin {%_rdll2}
if %ListScan
then
  if itemtoken = LOGITEM then flog := intval else %error(103);
end; {%_rdll2}

procedure %_rdll1(var flog: int1);
begin {%_rdll1}
if %ListScan
then
  if itemtoken = LOGITEM then flog := intval else %error(103);
end; {%_rdll1}


{ Read a complex }

procedure %_rdlc8(var fcomplex: complex);
begin {%_rdlc8}
if %ListScan
then
  if itemtoken = COMPLEXITEM then fcomplex := complexval else %error(104);
end; {%_rdlc8}


{ Read a character variable }

procedure %_rdlch(fpac: ppac; paclen: longint);
  var i, chtomove: integer;
begin {%_rdlch}
if %ListScan
then 
  if itemtoken = CHARITEM
  then begin
    if paclen <= charlen then chtomove := paclen else chtomove := charlen;
    for i := 1 to chtomove do 
      fpac^[i] := charval[i];
    for i := chtomove + 1 to paclen do
      fpac^[i] := ' ';
    end
  else %error(105);
end; {%_rdlch}


procedure %_rali4(var fintarray: int4array; count: int4);
  var ctr: longint;
begin {%_rali4}
for ctr := 1 to count do
  %_rdli4(fintarray[ctr]);
end; {%_rali4}

procedure %_rali2(var fintarray: int2array; count: int4);
  var ctr: longint;
begin {%_rali2}
for ctr := 1 to count do
  %_rdli2(fintarray[ctr]);
end; {%_rali2}

procedure %_rali1(var fintarray: int1array; count: int4);
  var ctr: longint;
begin {%_rali1}
for ctr := 1 to count do
  %_rdli1(fintarray[ctr]);
end; {%_rali1}

procedure %_ralr8(var frealarray: real8array; count: int4);
  var ctr: longint;
begin {%_ralr8}
for ctr := 1 to count do
  %_rdlr8(frealarray[ctr]);
end; {%_ralr8}

procedure %_ralr4(var frealarray: real4array; count: int4);
  var ctr: longint;
begin {%_ralr4}
for ctr := 1 to count do
  %_rdlr4(frealarray[ctr]);
end; {%_ralr4}

procedure %_rall4(var flogarray: int4array; count: int4);
  var ctr: longint;
begin {%_rall4}
for ctr := 1 to count do
  %_rdll4(flogarray[ctr]);
end; {%_rall4}

procedure %_rall2(var flogarray: int2array; count: int4);
  var ctr: longint;
begin {%_rall2}
for ctr := 1 to count do
  %_rdll2(flogarray[ctr]);
end; {%_rall2}

procedure %_rall1(var flogarray: int1array; count: int4);
  var ctr: longint;
begin {%_rall1}
for ctr := 1 to count do
  %_rdll1(flogarray[ctr]);
end; {%_rall1}

procedure %_ralc8(var fcomplexarray: complexarray; count: int4);
  var ctr: longint;
begin {%_ralc8}
for ctr := 1 to count do
  %_rdlc8(fcomplexarray[ctr]);
end; {%_ralc8}

procedure %_ralch(fpac: pbyte; paclen: integer; count: int4);
  var i,chtomove: integer; ctr: longint; p: ppac;
begin {%_ralch}
for ctr := 0 to count - 1 do
  if %ListScan
  then 
    if itemtoken = CHARITEM
    then begin
      p := pointer(ord(fpac) + ctr*paclen);
      if paclen <= charlen then chtomove := paclen else chtomove := charlen;
      for i := 1 to chtomove do 
        p^[i] := charval[i];
      for i := chtomove + 1 to paclen do
        p^[i] := ' ';
      end
    else %error(105);
end; {%_ralch}

end. {%fliblr}

                                                                                                                                                                                                                                                                                                            