(*****************************************************************************)
(*                                                                           *)
(*                            File: FLIBINIT.TEXT                            *)
(*                                                                           *)
(*           (C) Copyright 1981, 1985 Silicon Valley Software, Inc.          *)
(*                                                                           *)
(*                            All Rights Reserved.               30-Oct-85   *)
(*                                                                           *)
(*****************************************************************************)


{$%+} {$R-} {$I-} {$M-}

unit %flibinit;

interface

const DATE = '30-Oct-85'; VERSION = 'V2.4';
      COPYRIGHT = '(C) Copyright 1981, 1985 Silicon Valley Software, Inc.';
      RIGHTS    = '    All rights reserved.';
      
      MAXFILENAMELEN = 50;
      MAXRECLEN = 514;
      CHARVALMAX = 256;
      FBLKSIZE = 512;
      SEEK_ABSOLUTE = 0; SEEK_RELATIVE = 1;
      
      UNDERSCORE = TRUE;
      
      MERLIN = 0; IDRIS = 1; UNISOFT = 2; UNOS = 3; CROMIX = 4;
      ADVENTURE = 6; REGULUS = 7; CPM = 8; ELITE = 9; GENIX = 10;
      TEK = 11;
      
      ENVIRONMENT = UNISOFT;
      
      { Merlin 
      ERRPROMPT = '<space> to continue, <ESC> to exit - ';
      EOLCH = '\0D';
      OKHALT = 0;
      ERRHALT = -1;
      EMESSFILE = '!ftnrterrs.text';
      EMESS2FILE = 'ftnrterrs.text';
      {}
      
      { Idris 
      (*$S %_F77RTS *)
      ERRPROMPT = '<return> to continue, <delete> to exit - ';
      EOLCH = '\0A';
      OKHALT = 1;
      ERRHALT = 0;
      EMESSFILE = '/lib/ftnrterrs';
      EMESS2FILE = '/usr/lib/ftnrterrs';
      {}
      
      { Unisoft, Regulus, Genix, Tek }
      (*$S %_F77RTS *)
      ERRPROMPT = '<return> to continue, <delete> to exit - ';
      EOLCH = '\0A';
      OKHALT = 0;
      ERRHALT = -1;
      EMESSFILE = '/lib/ftnrterrs';
      EMESS2FILE = '/usr/lib/ftnrterrs';
      {}
      
      { Unos 
      (*$S %_F77RTS *)
      ERRPROMPT = '<return> to continue, <ctrl-C> to exit - ';
      EOLCH = '\0A';
      OKHALT = 0;
      ERRHALT = -1;
      EMESSFILE = '/lib/ftnrterrs';
      EMESS2FILE = '/usr/lib/ftnrterrs';
      {}
      
      { Cromix 
      (*$S %_F77RTS *)
      ERRPROMPT = '<return> to continue, <ctrl-C> to exit - ';
      EOLCH = '\0A';
      OKHALT = 0;
      ERRHALT = -1;
      EMESSFILE = '/lib/ftnrterrs';
      EMESS2FILE = '/usr/lib/ftnrterrs';
      {}
      
      { Adventure 
      (*$S %_F77RTS *)
      ERRPROMPT = '<return> to continue, <ESC-return> to exit - ';
      EOLCH = '\0D';
      OKHALT = 0;
      ERRHALT = -1;
      EMESSFILE = 'jjj:ftnrterr.src';
      EMESS2FILE = 'ftnrterr.src';
      {}
    
      { CPM 
      (*$S %_F77RTS *)
      ERRPROMPT = '<return> to continue, <ctrl-C> to exit - ';
      EOLCH = '\0A';
      OKHALT = 0;
      ERRHALT = -1;
      EMESSFILE = 'a:ftnrterr.src';
      EMESS2FILE = 'b:ftnrterr.src';
      {}
      
      { Elite 
      ERRPROMPT = '<return> to continue, <eof> to exit - ';
      EOLCH = '\0D';
      OKHALT = 0;
      ERRHALT = -1;
      EMESSFILE = 'ftnrterrs#text';
      EMESS2FILE = 'ftnrterrs#text/0';
      {}
      
      ESCBYTE = 27; EORBYTE = 10; FEORBYTE = 16;
      
type byte = -128..127;
     pac = packed array[1..32000] of char;
     ppac = ^pac;
     
     int1 = byte; int2 = integer; int4 = longint;
     pbyte = ^byte;
     
     aint12 = array[1..2] of int1; { Two bytes   }
     aint14 = array[1..4] of int1; { Four bytes  }
     aint18 = array[1..8] of int1; { Eight bytes }
     
     real4 = real; real8 = double;
     
     complex = record realpart: real; imagpart: real; end;
     
     many = 1..100000;
     int1array = array[many] of int1;
     int2array = array[many] of int2;
     int4array = array[many] of int4;
     real4array = array[many] of real4;
     real8array = array[many] of real8;
     complexarray = array[many] of complex;
     
     pint1array = ^int1array;
     plongint = ^longint;
     
     filenamestring = string[MAXFILENAMELEN];
     
     string16 = string[16];
     
     ppcodearray = ^pcodearray;
     pcodearray = array[1..1] of ppcodearray;
     
     ptext = ^text;
     filetype = file;
     
     { Merlin data structures }
     
     filekind = (UNTYPEDFILE,XDSKFILE,CODEFILE,TEXTFILE,INFOFILE,
                 DATAFILE,GRAFFILE,FOTOFILE,SECURDIR);
     
     merfib = record
                fwindow: ^int1array;
                FEOLN: Boolean;
                FEOF: Boolean;
                FText: Boolean;
                fstate: (FTVALID,FIEMPTY,FIVALID,FTEMPTY);
                frecsize: integer;
                case FIsOpen: Boolean of
                  TRUE: (FIsBlocked: Boolean;
                         funit: integer;
                         fvid: string[7];
                         frepeatcount,
                         fnextblock,
                         fmaxblock: integer;
                         FModified: Boolean;
                         fheader: packed record
                                    firstblock: integer;
                                    nextblock: integer;
                                    MarkBit: Boolean;
                                    filler: 0..2047;
                                    case fkind: filekind of
                                      SECURDIR,
                                      UNTYPEDFILE: ();
                                      XDSKFILE,CODEFILE,TEXTFILE,INFOFILE,
                                      DATAFILE,GRAFFILE,FOTOFILE:
                                        (filler2: string[15];
                                         dlastbyte: integer; {Bytes in last block}
                                         filler3: integer);
                                  end;
                         case FSoftBuf: Boolean  of
                           TRUE: (fnextbyte,fmaxbyte: integer;
                                  FBufChanged: Boolean;
                                  fbuffer: array[0..511] of byte;
                                  fuparrow: integer));
              end;
     
     { Idris data structures }
     
     bstates = (BEMPTY,BRDING,BWRING);
     
     idfib = record
               fwindow: pint1array;                            { 0 }
               FEOLN: Boolean;                                 { 4 }
               FEOF: Boolean;                                  { 5 }
               FTEXT: Boolean;                                 { 6 }
               fstate: (IDFTVAL, IDFIEMPTY, IDFIVAL, IDFTEMP); { 7 }
               frecsize: integer;                              { 8 }
               fnextblock: integer;                            { 10 }
               fd: longint;                                    { 12 }
               buffed: Boolean;                                { 16 }
               bstate: bstates;                                { 17 }
               bnxtby: pint1array;                             { 18 }
               blstby: pint1array;                             { 22 }
               filepos: longint; {for Adventure}               { 26 }
               filesiz: longint; {for CPM}                     { 30 }
             end;
     
     { Cromix data structures }
     
     crofib = record
                fwindow: pint1array;                            { 0 }
                FEOLN: Boolean;                                 { 4 }
                FEOF: Boolean;                                  { 5 }
                FTEXT: Boolean;                                 { 6 }
                fstate: (CRFTVAL, CRFIEMPTY, CRFIVAL, CRFTEMP); { 7 }
                frecsize: integer;                              { 8 }
                fnextblock: integer;                            { 10 }
                fd: integer;                                    { 12 }
                fibmark: integer; {  Magic number }             { 14 }
                FOpen: Boolean;                                 { 16 }
                fhole: byte;                                    { 17 }
                Buffed: Boolean;                                { 18 }
                bstate: bstates;                                { 19 }
                bnxtby: pint1array;                             { 20 }
                blstby: pint1array;                             { 24 }
                buffer: array[0..511] of byte;                  { 28 }
              end;
     
     { Elite data structures }
     
     eltfib = record
                fwindow: pint1array;                            { 0 }
                FEOLN: Boolean;                                 { 4 }
                FEOF: Boolean;                                  { 5 }
                FTEXT: Boolean;                                 { 6 }
                fstate: (ELFTVAL, ELFIEMPTY, ELFIVAL, ELFTEMP); { 7 }
                frecsize: integer;                              { 8 }
                fnextblock: longint;                            { 10 }
                fd: longint;                                    { 14 }
                Buffed: Boolean;                                { 18 }
                bstate: bstates;                                { 19 }
                bnxtby: pint1array;                             { 20 }
                blstby: pint1array;                             { 24 }
                filesyspos: longint;                            { 28 }
                highwater: longint;  { CP/M only }              { 32 }
                osname: packed array[1..64] of char;            { 36 }
                buffer: array[0..511] of byte;                  { 100 }
              end;
     
     punitrec = ^unitrec;
     
     unitrec = record
       nextunit:    punitrec;
       unitid:      longint;
       fname:       filenamestring;
       reclen:      longint;  { defined if not Seqen }
       Scratch:     Boolean;  { status = SCRATCH in open statement }
       Seqen:       Boolean;
       {Only UNFORMATTED if Seqen, PRINTER is FORMATTED, ordering significant}
       Formted:     (PRINTER,FORMATTED,UNFORMATTED,BINARY);
       BlankNull:   Boolean;
       PastEndFile: Boolean;  { is the file positioned beyond the endfile record }
       lastop:      (READOP,WRITEOP);
       case integer of
         0: (osfile: ^filetype);
         1: (osfib: ^merfib);
         2: (idosfib: ^idfib);
         3: (ostextfile: ptext);
         4: (crosfib: ^crofib);
         5: (eltpfib: ^eltfib);
     end;
     
     listitems = (INTITEM, REALITEM, CHARITEM, LOGITEM, COMPLEXITEM, NULLITEM);
     charvaltype = packed array[1..CHARVALMAX] of char;
     
     str64 = string[64]; pstring = ^ str64; { For stop and pause }
     
var curunit,                            { Unit for current I/O action         }
    units,                              { List of connected units             }
    consoleunit,                        { Reading from is special             }
    stderrunit,                         { Don't close under MERLIN            }
    freeunits: punitrec;                { Free list of closed units           }
    
    errornumber: integer;               { 0 is no error detected              }
    tempfilectr: integer;               { used to generate temp file names    }
    
    Reading: Boolean;                   { Reading or writing                  }
    col: integer;                       { Used for counting columns on input  }
    reclenctr: longint;                 { Used to count bytes in direct I/O   }
    EofFlag: Boolean;                   { End of file encountered on reading  }
    
    fmttok: char;                       { Last format token scanned           }
    fmtintval: integer;                 { Last integer scanned out of format  }
    
    { Describing a format, and the state of scanning it is at }
    
    curfmt: ppac;                       { Pointer to current format           }
    fmtp: integer;                      { curfmt^[fmtp] is next char to read  }
    gpcount: array[1..11] of integer;   { Reps left in fmt group at a level   }
    gpbegin: array[1..11] of integer;   { curfmt^[gpbegin[i]] is '(^' of gp   }
    gplevel: 1..11;                     { Currently processing this gp level  }
    lastgppos: integer;                 { Backup position for end of fmt      }
    lastgprep: integer;                 { Reps for backup group               }
    lastgplevel: 1..2;                  { Level after backup                  }
    OKToExit: Boolean;                  { Is there another item in I/O list   }
    InhibitFinalCR: Boolean;            { Allow partial line prompting to *   }
    FieldFound: Boolean;                { Has a repeatable ed been found      }
    Gp2FieldFound: Boolean;             { Was a field found in last lev 2 gp  }
    
    { Edit descriptor - Describes next repeatable format }
    
    edcnt: integer;                     { More times to use ed                }
    edlet: char;                        { Key letter for ed                   }
    edw, edd, ede, edm: integer;        { Integer parameters to ed, see 13.2  }
    AorZFlag: Boolean;                  { edlet has value 'A' or 'Z'          }
    EdePresent: Boolean;                { Is final Ee present for edlet = 'E' }
    EdwPresent: Boolean;                { Is w field present in a format      }
    BZFlag: Boolean;                    { BZ or BN currently in force         }
    pval: integer;                      { Scale factor value                  }
    PrintOptionalPlus: Boolean;         { Controlled by S, SP, and SS format  }
    
    { Record I/O is being done to }
    
    recbuf: array[1..4096] of int1;
    recbufp: integer;                   { recbuf[recbufp] is next I/O char    }
    lastwritten: integer;               { position in recbuf of last write    }
    maxlastwritten: integer;            { max position in recbuf written      }
    recbuflen: integer;                 { number of chars in recbuf for Rding }
    
    
    { Variables for list directed I/O }
    
    itemcnt: integer;                   { Repeat count for current list item  }
    itemtoken: listitems;               { What kind of list item was scanned  }
    listch: char;                       { Look ahead char for list input      }
    FirstItem: Boolean;                 { ListScan needs to know if fst call  }
    NeedBlank: Boolean;                 { Is a blank required before next wrl }
    
    intval: longint;                    { Value of INTITEM or LOGITEM         }
    realval: real8;                     { Value of REALITEM                   }
    charval: charvaltype;               { Value of CHARITEM                   }
    charlen: integer;                   { Length of value of CHARITEM         }
    complexval: complex;                { Value of COMPLEXITEM                }
    
    { Variables for internal I/O }
    
    InternalIO: Boolean;                { So that nextrec knows if internal   }
    intfile: pint1array;                { Pointer to internal file            }
    intreclen: integer;                 { Record length of internal file      }
    intbufp: longint;                   { intfile[intbufp] is next ch for I/O }
    
    ranseed: longint;                   { Random number seed                  }
    lastran: real;                      { Last random number returned         }
	%called_userexit : Boolean;
    
procedure %error(errnum: integer);
function %findunit(unitnum: longint): punitrec;
procedure %pageinfbuffer(relblknum: integer);
procedure %pageoutfbuffer;
procedure %_ferror(addr: longint; n: integer);
procedure %updatehighwatermark;
procedure %forceeofiflastwritten;
procedure %sysseek(targetposition: longint);
procedure %lclose(unitnum: longint; fstatus: ppac; fstatuslen: integer);
procedure %makelname(var lname: filenamestring; name: ppac; namelen: integer);
procedure %_rtsfin;
function %rtsmatch(fpac: ppac; fpaclen: integer; fst: string16): Boolean;
procedure %disposeunit(funit: punitrec);

implementation


procedure %_prloc(addr: longint); external;
procedure _mktemp(var fname: filenamestring); cexternal;
procedure mktemp(var fname: filenamestring); cexternal;
procedure %_fseek(fib: ptext; targetpos,sense: longint); external;

(* SGI_TRUNCATE *)
procedure %_trunc(fib: ptext); external;

(* a user error routine is called before error exits
   to enable users to put such things as databases in 
   a consistent state *)
procedure userexit(var addr: longint; var n:integer); external;


procedure %error{*errnum: integer*};
begin {%error}
if errornumber = 0 then errornumber := errnum + 600;
end; {%error}

procedure %pageoutfbuffer;
begin {%pageoutfbuffer}
if ENVIRONMENT = MERLIN
then
  if curunit <> nil
  then
    with curunit^ do begin
      if blockwrite(osfile^,osfib^.fbuffer,1,osfib^.fnextblock - 1) <> 1 
      then %error(48);
      osfib^.FModified := TRUE; osfib^.FBufChanged := FALSE;
      end;
end; {%pageoutfbuffer}

procedure %pageinfbuffer{*relblknum: integer*};
begin {%pageinfbuffer}
if ENVIRONMENT = MERLIN
then
  if curunit <> nil
  then
    with curunit^ do begin
      if relblknum >= osfib^.fmaxblock
      then begin { Block over the previous high water mark for this file }
        osfib^.fbuffer[0] := 0; osfib^.fbuffer[1] := 0; 
        moveleft(osfib^.fbuffer[0],osfib^.fbuffer[2],FBLKSIZE-2);
        osfib^.fmaxblock := relblknum + 1;{ New highest block touched in file }
        if Formted > FORMATTED 
        then osfib^.fheader.dlastbyte := 0; { Won't stay 0 }
        end
      else { Read in file block to initialize buffer }
        if blockread(osfile^,osfib^.fbuffer,1,relblknum) <> 1 then %error(49);
      osfib^.fnextbyte := 0; osfib^.fnextblock := relblknum + 1; 
      osfib^.FBufChanged := FALSE;
      end;
end; {%pageinfbuffer}

procedure %sysclose(KeepIt: Boolean; funit: punitrec);
begin {%sysclose}
with funit^ do begin { Never nil }
  if ENVIRONMENT = MERLIN
  then begin
    if osfib^.FIsBlocked and osfib^.FBufChanged then %pageoutfbuffer;
    if KeepIt then close(osfile^,LOCK) else close(osfile^);
    end
  else begin
    if ENVIRONMENT = CPM
    then
      if Formted > FORMATTED 
      then idosfib^.ftext := FALSE; {Inhibit ^Z appending}
    if KeepIt then close(ostextfile^,LOCK) else close(ostextfile^,PURGE);
    end;
  if ioresult <> 0 then %error(70);
  end;
end; {%sysclose}

procedure %forceeofiflastwritten;
  var i: integer;
begin {%forceeofiflastwritten}
if ENVIRONMENT = MERLIN
then begin
  if curunit <> nil
  then begin
    with curunit^ do begin
      if osfib^.FIsBlocked and (lastop = WRITEOP)
      then begin
        for i := osfib^.fnextbyte to fblksize do
          osfib^.fbuffer[i] := 0;
        if Formted > FORMATTED 
        then osfib^.fheader.dlastbyte := osfib^.fnextbyte;
        %pageoutfbuffer;
        if odd(osfib^.fnextblock) and (Formted <= FORMATTED)
        then begin
          osfib^.fnextblock := osfib^.fnextblock + 1;
          osfib^.fbuffer[0] := 0; osfib^.fbuffer[1] := 0; 
          moveleft(osfib^.fbuffer[0],osfib^.fbuffer[2],FBLKSIZE-2);
          %pageoutfbuffer;
          end;
        osfib^.fmaxblock := osfib^.fnextblock;
        end;
	  end;
	end;
end else
	(* SGI_TRUNCATE *)
	if (curunit <> nil) then
      with curunit^ do 
		if (Seqen) and (lastop = WRITEOP) then
	      %_trunc(ostextfile);
end; {%forceeofiflastwritten}

procedure %updatehighwatermark;
begin {%updatehighwatermark}
if ENVIRONMENT = MERLIN
then
  if curunit <> nil
  then
    with curunit^ do begin
      if (Formted > FORMATTED) and 
         (osfib^.fnextblock = osfib^.fmaxblock) and 
         (osfib^.fnextbyte > osfib^.fheader.dlastbyte)
      then osfib^.fheader.dlastbyte := osfib^.fnextbyte;
      end;
end; {%updatehighwatermark}

procedure %sysseek{*targetposition: longint*};
  var targblk, targnxtbyte: integer;
begin {%sysseek}
if ENVIRONMENT = MERLIN
then begin
  targblk := targetposition div FBLKSIZE;
  targnxtbyte := targetposition mod FBLKSIZE;
  if curunit <> nil
  then
    with curunit^ do begin
      if osfib^.fheader.fkind = TEXTFILE then targblk := targblk + 2;
      if osfib^.fnextblock <> (targblk + 1)
      then begin
        if osfib^.FBufChanged then %pageoutfbuffer;
        %pageinfbuffer(targblk);
        end;
      osfib^.fnextbyte := targnxtbyte;
      end;
  end
else begin {ENVIRONMENT <> MERLIN}
  if curunit <> nil
  then begin
    %_fseek(curunit^.ostextfile,targetposition,SEEK_ABSOLUTE);
    if ioresult <> 0 then %error(49);
    end;
  end;
end; {%sysseek}

function %rtsmatch{*fpac: ppac; fpaclen: integer; fst: string16): Boolean*};
  var i,cnt: integer; lch: char; MatchFlag: Boolean;
begin {%rtsmatch}
i := 1; cnt := length(fst); MatchFlag := TRUE;
while (i <= cnt) and MatchFlag do begin
  if i <= fpaclen then lch := fpac^[i] else lch := '*';
  if (lch >= 'a') and (lch <= 'z') then lch := chr(ord(lch) - 32);
  MatchFlag := fst[i] = lch;
  i := i + 1;
  end;
%rtsmatch := MatchFlag;
end; {%rtsmatch}

function %findunit{*unitnum: longint): punitrec*};
  var lunit: punitrec; NotFound: Boolean;
begin {%findunit}
lunit := units; NotFound := TRUE; 
while (lunit <> nil) and NotFound do
  if lunit^.unitid = unitnum 
  then NotFound := FALSE 
  else lunit := lunit^.nextunit;
%findunit := lunit;
end; {%findunit}

procedure %disposeunit{*funit: punitrec*};
  var lunit: punitrec;
begin {%disposeunit}
if funit = units
then units := funit^.nextunit { %_rtsfin depends on this }
else begin
  lunit := units;
  while lunit <> nil do
    with lunit^ do begin
      if nextunit = funit then nextunit := funit^.nextunit;
      lunit := nextunit;
      end;
  end;
{ Don't reclaim consoleunit or stderrunit even if closed }
{ since they do not have reuseable pointers to fibs.     }
if (funit <> consoleunit) and (funit <> stderrunit)
then begin funit^.nextunit := freeunits; freeunits := funit; end;
end; {%disposeunit}


{ Procedure %lclose does the work of %_rtscl but does not use errexit or  }
{ fiostat.  This allows it to be called internally in the I/O system.     }

procedure %lclose{*unitnum: longint; fstatus: ppac; fstatuslen: integer*};
  var KeepIt: Boolean;
begin {%lclose}
curunit := %findunit(unitnum);
if curunit <> nil 
then 
  with curunit^ do begin
    if Seqen 
    then %forceeofiflastwritten
    else begin
      %updatehighwatermark;
      if ENVIRONMENT = MERLIN
      then
        if odd(osfib^.fmaxblock) and (Formted <= FORMATTED)
        then begin
          if osfib^.FIsBlocked and osfib^.FBufChanged then %pageoutfbuffer;
          %pageinfbuffer(osfib^.fmaxblock); { Clears fbuffer }
          %pageoutfbuffer;
          end;
      end;
    KeepIt := not Scratch;
    if fstatus <> nil
    then 
      if %rtsmatch(fstatus,fstatuslen,'KEEP')
      then 
        if Scratch then %error(71) else KeepIt := TRUE
      else 
        if %rtsmatch(fstatus,fstatuslen,'DELETE')
        then KeepIt := FALSE
        else %error(72);
    if ENVIRONMENT = ADVENTURE
    then begin
      if ostextfile <> @stderr then %sysclose(KeepIt,curunit);
      end
    else %sysclose(KeepIt,curunit);
    %disposeunit(curunit);
    end;
end; {%lclose}


{ Create lname, normalized form of fname and fnamelen }

procedure %makelname{*var lname: filenamestring; name: ppac; namelen: integer*};
  var i: integer;
begin {%makelname}
if namelen > MAXFILENAMELEN then namelen := MAXFILENAMELEN;
if name = nil
then begin
  if (ENVIRONMENT = UNISOFT) or (ENVIRONMENT = GENIX) or (ENVIRONMENT = TEK)
  then begin
    lname := 'TMPnnXXXXXX'; lname[12] := chr(0);
    if UNDERSCORE then _mktemp(lname) else mktemp(lname);
    end
  else
    if ENVIRONMENT = ADVENTURE 
    then lname := 'TMP  .TMP' 
    else lname := 'TMP  ';
  lname[4] := chr(((tempfilectr div 10) mod 10) + ord('0'));
  lname[5] := chr((tempfilectr mod 10) + ord('0'));
  tempfilectr := tempfilectr + 1;
  end
else begin
  lname[0] := chr(0);
  for i := 1 to namelen do begin
    lname[i] := name^[i];
    if name^[i] <> ' ' then lname[0] := chr(i);
    end;
  end;
end; {%makelname}

procedure %f850401;
  type pac6 = packed array[1..6] of char;
  var lname2: ppac; lname6: ^pac6; lname: filenamestring;
      s: string[80];
begin {%f850401}
%called_userexit := FALSE;
{ Preconnect unit 0 to output for writing }
new(lname6); lname6^ := 'output'; moveleft(lname6,lname2,4);
%makelname(lname,lname2,6);
freeunits := nil; tempfilectr := 0; errornumber := 0;
new(consoleunit);
with consoleunit^ do begin
  unitid := 0; nextunit := nil; fname := lname; Formted := FORMATTED;
  reclen := 0; Scratch := FALSE; Seqen := TRUE; BlankNull := TRUE;
  PastEndFile := FALSE; lastop := READOP;
  ostextfile := @output;
  end;
units := consoleunit;

{ Preconnect unit 1 to stderr for writing }
new(lname6); lname6^ := 'stderr'; moveleft(lname6,lname2,4);
%makelname(lname,lname2,6);
new(stderrunit);
with stderrunit^ do begin
  unitid := 1; nextunit := units; fname := lname; Formted := FORMATTED;
  reclen := 0; Scratch := FALSE; Seqen := TRUE; BlankNull := TRUE;
  PastEndFile := FALSE; lastop := READOP;
  ostextfile := @stderr;
  end;
units := stderrunit;
s := COPYRIGHT; s := RIGHTS;
ranseed := $137f8044; lastran := 0;
end; {%f850401}

procedure %_rtsfin;
begin {%_rtsfin}
while units <> nil do 
  if (units = consoleunit) or (units = stderrunit)
  then units := units^.nextunit
  else %lclose(units^.unitid,nil,0); { Side effect: units := units^.nextunit }
end; {%_rtsfin}

procedure %_ferror{*addr: longint; n: integer*};
  var i: integer; errmess: text; s: string[80];
begin {%_ferror}

(* call user error routine to put the databases in a 
   consistent state *)
if (%called_userexit = FALSE) 
then begin
	%called_userexit := TRUE;
	userexit(addr,n);
end;


writeln('FORTRAN run time error number: ',n:1);
reset(errmess,EMESSFILE);
if ioresult <> 0 then reset(errmess,EMESS2FILE);
if ioresult = 0
then begin
  repeat
    read(errmess,i);
    if i = n
    then begin
      readln(errmess,s); writeln(s);
      end
    else readln(errmess);
  until i > n;
  close(errmess);
  end;
if addr > 0 then %_prloc(addr);
%_rtsfin; halt(ERRHALT);
end; {%_ferror}

end. {%flibinit}

