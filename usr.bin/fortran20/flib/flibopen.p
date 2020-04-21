(*****************************************************************************)
(*                                                                           *)
(*                            File: FLIBOPEN.TEXT                            *)
(*                                                                           *)
(*           (C) Copyright 1982, 1985 Silicon Valley Software, Inc.          *)
(*                                                                           *)
(*                            All Rights Reserved.               22-Aug-85   *)
(*                                                                           *)
(*****************************************************************************)


{$%+} {$R-} {$I-}

unit %flibopen;

interface

uses {$U flibinit} %flibinit,
     {$U flibrec}  %flibrec;

implementation

procedure %_rtscl(unitnum: longint;
                  fiostat: plongint;
                  errexit: pcodearray;
                  fstatus: ppac; fstatuslen: longint);
begin {%_rtscl}
errornumber := 0; 
%lclose(unitnum,fstatus,fstatuslen);
%termiostmt(fiostat,errexit);
end; {%_rtscl}


procedure %_rtsop(unitnum: longint;
                  fiostat: plongint;
                  errexit: pcodearray;
                  name: ppac; namelen: longint;
                  fstatus: ppac; fstatuslen: longint;
                  faccess: ppac; faccesslen: longint;
                  fform: ppac; fformlen: longint;
                  frecl: longint; FReclPresent: integer;
                  fblank: ppac; fblanklen: longint;
                  fbuffed: ppac; fbuffedlen: longint;
                  fbinary: integer);
  var lunit: punitrec; UnitIsNew, OpenOld, Buffered: Boolean; 
      lname: filenamestring;
  
  function %newunit: punitrec;
    var llunit: punitrec;
  begin {%newunit}
  if freeunits = nil
  then begin new(llunit); new(llunit^.ostextfile); end
  else begin llunit := freeunits; freeunits := freeunits^.nextunit; end;
  with llunit^ do begin
    unitid := unitnum; nextunit := units; fname := lname; Formted := FORMATTED;
    reclen := 0; Scratch := FALSE; Seqen := TRUE; BlankNull := TRUE;
    PastEndFile := FALSE; lastop := READOP;
    end;
  units := llunit; %newunit := llunit;
  end; {%newunit}

  procedure %sysopen(OpenOld: Boolean; funit: punitrec);
  begin {%sysopen}
  with funit^ do begin { Never nil }
    if ENVIRONMENT = MERLIN
    then
      if OpenOld
      then reset(osfile^,fname)
      else rewrite(osfile^,fname)
    else {ENVIRONMENT <> MERLIN}
      if Buffered
      then
        if OpenOld
        then reset(ostextfile^,fname,buffered)
        else rewrite(ostextfile^,fname,buffered)
      else
        if OpenOld
        then reset(ostextfile^,fname,unbuffered)
        else rewrite(ostextfile^,fname,unbuffered);
    if ioresult <> 0
    then
      if OpenOld
      then begin
        if fstatus = nil
        then begin %sysopen(FALSE,funit); exit(%sysopen); end
        else
          if %rtsmatch(fstatus,fstatuslen,'UNKNOWN')
          then begin %sysopen(FALSE,funit); exit(%sysopen); end
          else %error(69);
        end
      else %error(69);
    if ENVIRONMENT = MERLIN
    then
      if osfib^.FIsBlocked
      then begin
        if osfib^.fheader.fkind = TEXTFILE then osfib^.fnextblock := 2;
        osfib^.fnextbyte := FBLKSIZE; osfib^.FBufChanged := FALSE;
        end
      else
        if not Seqen then %error(54);
    end;
  end; {%sysopen}
  
begin {%_rtsop}
%makelname(lname,name,namelen);

{ Tie up to proper lunit, or allocate a new one if necessary }

UnitIsNew := FALSE; { Need to remember if freshly allocated }
Buffered := TRUE; { Default }
lunit := %findunit(unitnum); { does the specified unit exist? }
if lunit = nil
then begin lunit := %newunit; UnitIsNew := TRUE; end
else { lunit <> nil, unit already exists with the same unitnum }
  if name <> nil
  then
    if lname <> lunit^.fname
    then begin 
      %lclose(unitnum,nil,0); lunit := %newunit; UnitIsNew := TRUE; 
      end;
    { Else names match in which case lunit is the punitrec to be "reopened" }
  { Else name = nil in which case lunit is the punitrec to be "reopened" }

{ Set up lunit^ }

errornumber := 0;
with lunit^ do begin
  Scratch := (name = nil) and (fstatus = nil);
  OpenOld := not Scratch;
  if UnitIsNew
  then begin
    if fbuffed <> nil
    then
      if %rtsmatch(fbuffed,fbuffedlen,'UNBUFFERED')
      then Buffered := FALSE
      else
        if not %rtsmatch(fbuffed,fbuffedlen,'BUFFERED')
        then %error(72);
    if fstatus <> nil
    then
      if %rtsmatch(fstatus,fstatuslen,'OLD') or 
         %rtsmatch(fstatus,fstatuslen,'UNKNOWN')
      then begin
        if name = nil then %error(73);
        end
      else
        if %rtsmatch(fstatus,fstatuslen,'NEW')
        then begin
          OpenOld := FALSE;
          if name = nil then %error(73);
          end
        else
          if %rtsmatch(fstatus,fstatuslen,'SCRATCH')
          then begin
            Scratch := TRUE; OpenOld := FALSE;
            if name <> nil then %error(74);
            end
          else %error(72);
    if faccess <> nil
    then
      if %rtsmatch(faccess,faccesslen,'DIRECT')
      then Seqen := FALSE
      else
        if not %rtsmatch(faccess,faccesslen,'SEQUENTIAL')
        then %error(72);
    if fform = nil
    then 
      if Seqen
      then Formted := FORMATTED
      else Formted := BINARY
    else
      if %rtsmatch(fform,fformlen,'UNFORMATTED')
      then 
        if (fbinary = 0) and Seqen
        then Formted := UNFORMATTED
        else Formted := BINARY
      else
        if %rtsmatch(fform,fformlen,'BINARY')
        then Formted := BINARY
        else
          if %rtsmatch(fform,fformlen,'PRINTER')
          then Formted := PRINTER
          else
            if not %rtsmatch(fform,fformlen,'FORMATTED')
            then %error(72);
    if FReclPresent <> 0
    then begin
      if Seqen then %error(75);
      if frecl <= 0 then %error(76);
      reclen := frecl;
      end
    else 
      if not Seqen then %error(77);
    end; { UnitIsNew }
  if fblank <> nil
  then begin
    if Formted > FORMATTED then %error(78);
    if %rtsmatch(fblank,fblanklen,'ZERO')
    then BlankNull := FALSE
    else
      if %rtsmatch(fblank,fblanklen,'NULL')
      then BlankNull := TRUE
      else %error(72);
    end;
  end;
if UnitIsNew and (errornumber = 0) then %sysopen(OpenOld,lunit);
if UnitIsNew and (errornumber <> 0) then %disposeunit(lunit);
%termiostmt(fiostat,errexit);
end; {%_rtsop}

end. {%flibopen}

