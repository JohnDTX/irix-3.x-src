(******************************************************************************)
(*                                                                            *)
(*                                                                            *)
(*                               PROGRAM ULINKER                              *)
(*                                                                            *)
(*                                                                            *)
(*                         (C) Copyright 1982, 1985                           *)
(*                                                                            *)
(*                            R. Steven Glanville                             *)
(*                                                                            *)
(*                               Jeffrey Barth                                *)
(*                                                                            *)
(*                       Silicon Valley Software, Inc.                        *)
(*                                                                            *)
(*                            All Rights Reserved.                            *)
(*                                                                            *)
(*                                                                30-Oct-85   *)
(*                                                                            *)
(******************************************************************************)


{$G+}
{$I-}
{$R-}

program ulinker;

label 999;

const MAXOUTBYTE = 4095;
      DBGTABLESIZE = 28;
      DATE = '30-Oct-85';
      CTITLE = 'MC68000 Unos Object Code Formatter  ';
      UTITLE = 'MC68000 Unix Object Code Formatter  ';
      ITITLE = 'MC68000 Idris Object Code Formatter ';
      ATITLE = 'MC68000 Adventure Object Code Formatter ';
      RTITLE = 'MC68000 Object Code Formatter       ';
      MTITLE = 'MC68000 CPM Object Code Formatter   ';
      VERSION = 'V2.4';
      
      (***
      MERLIN = 0;     Don't do C Constructs
      IDRIS = 1; 
      UNISOFT = 2;    Done
      UNOS = 3;       Done
      CROMIX = 4;     Don't do C Constructs
      ALTOS = 5;  
      ADVENTURE = 6;  Done
      QU68000 = 7; 
      REGULUS = 8;
      CPM = 9;
      CT = 10;        Done
      LANDL = 11;     Done
      SANTACRUZ = 12; Done
      NCR = 13;       Done
      ARETE = 14;     Done
      BERKELEY = 15;  Done
      NBI = 16        Done
      PLEXUS = 17;    Done
      MULTI = 18;     Done
      ***)
      
      MERLIN = 0; IDRIS = 1; UNISOFT = 2; UNOS = 3; CROMIX = 4;
      ALTOS = 5;  ADVENTURE = 6; QU68000 = 7; REGULUS = 8; CPM = 9;
      CT = 10; LANDL = 11; SANTACRUZ = 12; NCR = 13; ARETE = 14;
      BERKELEY = 15; NBI = 16; PLEXUS = 17; MULTI = 18;
      SINGLEWORD = 0; DOUBLEWORD = 1;
      
      ENVIRONMENT = BERKELEY;
      UNDERSCORE = TRUE;  { Does not work with all environments }
                          { Works with UNISOFT, PLEXUS }
      
      { ENVIRONMENT <> UNISOFT, UNISOFT, XENIX, VALID }
      ALIGNMENT = SINGLEWORD;
      {}
      
      { CADLINC 
      ALIGNMENT = DOUBLEWORD;
      {}
      
      
type string8 = string[8];
     string50 = string[50];
     alfa8 = packed array[1..8] of char;
     alfa4 = packed array[1..4] of char;
     alfa2 = packed array[1..2] of char;
     sizes = (BYTE,WORD,LONG);
     addrrange = -8388608..8388607;
     bite = -128..127;
     
     pdiskaddr = ^diskaddr;
     diskaddr = record next: pdiskaddr;
                       block,byte: integer;
                end;
     
     pfilerec = ^filerec;
     filerec = record name: string50;
                      lpbase: integer;
                      case LibFlag: Boolean of
                        TRUE: (modreflist: pdiskaddr);
               end;
     
     pintlist = ^intlist;
     intlist = record intval: integer;
                      nextint: pintlist;
               end;
     
     pmodlist = ^modulelist;
     modulelist = record modinfile: pfilerec;
                         modblock,modbyte: integer;
                         nextmodule: pmodlist;
                  end;
     
     psegment = ^segmentrec;
     segmentrec = record segname: alfa8; segnum: bite;
                         llink,rlink,nextseg: psegment;
                         codesize: addrrange;
                         jtbaseloc: integer;
                         modlist: pmodlist;
                         segreloclist: pintlist;
                         segbaseaddr: longint;
                  end;
     
     preflist = ^reflist;
     reflist = record refloc: integer;
                      refseg: bite;
                      ShortRef: Boolean;
                      nextref: preflist;
               end;
     
     psymbol = ^symbol;
     symbol = record linkname,username: alfa8;
                     llink,rlink: psymbol;
                     sreflist: preflist;
                     Visible: Boolean;
                     jtloc: integer;
                     case Defined: Boolean of
                         FALSE: (isymtabentry: integer);
                         TRUE:  (loc: integer;
                                 seg: psegment);
              end;
     
     ppatchlist = ^patchlist;
     patchlist = record patchloc: integer;
                        patchval: integer;
                        nextpatch: ppatchlist;
                 end;
     
     pfpatchlist = ^fpatchlist;
     fpatchlist = record patchloc: integer; (* This is segment relative *)
                         patchval: longint;
                         nextpatch: pfpatchlist;
                 end;
     
     { Unit names }
     pcommon = ^common;
     common = record name: alfa8;
                     nextcommon: pcommon;
              end;
     
     pdatainit = ^datainit;
     datainit = record datamodule: pmodlist;
                       dataloc: longint;
                       datanext: pdatainit;
                end;
                
     { FORTRAN data areas }
     pfcommon = ^fcommon;
     fcommon = record name: alfa8;
                      llink,rlink: pfcommon;
                      reflist: preflist;
                      initlist,lastinit: pdatainit;
                      reladdr: longint;
                      TextResident: Boolean;
                      AtAbsolute: Boolean;
                      case Defined: Boolean of
                           TRUE: (size: addrrange;
                                  next: pfcommon);
              end;
     
     pfdreloc = ^fdreloc;
     fdreloc = record dname,rname: alfa8;
                      daddr: longint;
                      next: pfdreloc;
               end;
     linkid = record linkno: integer; nam: string8; end;

var infile: file;
    inword,inblock,nextinblock,lastinword: integer;
    inbuff: array[0..2047] of integer;
    listfile: text;
    ListOpen,HalfFlag: Boolean;
    LastByte: integer;
    fname,ofname: string50;
    EofFlag: Boolean;
    
    reloclist,lastreloc: pintlist;
    
    largc: integer;
    thisfile: pfilerec;
    baseloc: addrrange;
    startloc: integer;
    startflag: Boolean;
    lprocbase,lprocmax: integer;
    
    nametree: psymbol;          { Entry point tree }
    commonlist: pcommon;        { List of Unit Names }
    ftndatatree,                { Tree of FTN Data Areas }
    ftndatalist: pfcommon;      { List of FTN Data Areas }
    fdreloclist: pfdreloc;      { Head of FTN Data relocation list }
    fdfreloclist: pfdreloc;     { Head of FTN Data Function relocation list }
    CConstructs: Boolean;       { Are there any 87 or 8D blocks }
    DataInText: Boolean;        { Is any data area text resident }
    InRelocation: Boolean;      { Is walkfcommon looking for relocation info }
    
    headpatchlist,headjtpatchlist,headrelpatchlist: ppatchlist;
    ftextpatchlist, fdatapatchlist, fbsspatchlist: pfpatchlist;
    
    ComFlag: Boolean;
    commandfile: text;
    
    DebugFlag: Boolean;
    undefrefs: longint;      (* Number of undefined external references *)
    defrefs: longint;        (* Number of defined externals *)
    undefcharcount: longint; (* Number of chars in undefined ext. refs. *)
    stringpos: longint;      (* Used for ALTOS to keep track of string table *)
    undefftn: integer;
    errors: integer;
    
    segtree,thisseg,seglist,startseg,overflowsegs: psegment;
    nextsegnum: integer;
    
    globalsize: addrrange;
    OK: Boolean;
    
    AlfaFlag,EscapeFlag: Boolean;
    
    ioutfile: file;
    ioutbuffer: array[0..4095] of bite;
    ioutbyte,ioutblock: integer;
    IOutOpen: Boolean;
    
    symfile: file;
    symbuffer: packed array[0..1023] of bite;
    symbyte,symblock: integer;
    SymOpen: Boolean;
    
    entrycodesize: integer;
    totalcodesize,executablecodesize: longint;
    bsssize, datasize: longint;
    TextRounded: Boolean;
    totalbytesrelocationinfo,
    totalbytesdatarelocationinfo: longint;
    
    hex: array[0..240] of integer;
    rellinelen: integer;
    argcbase: integer;
    
    vartypelist,                { List of Variable/Type defs $A0 }
    lastvartype,
    bpointlist,                 { List of Statement Offset defs $A1 }
    lastbpoint: pmodlist;
    
function lookupfcommon(var ftree: pfcommon; 
                           fname: alfa8): pfcommon; forward;

procedure warning(s: string50);
begin
writeln(chr(7),'*** Warning - ',s,' ***');
end; {warning}

procedure error(s: string50);
begin
writeln(chr(7),'*** Error - ',s,' ***');
errors := errors + 1;
end; {error}

procedure fatal_error(s: string50);
begin
writeln(chr(7),'*** Fatal Error - ',s,' ***');
errors := errors + 1;
goto 999;
end; {fatal_error}

procedure prompt(message: string50; var s: string50);
  var ch: char; lfcommon: pfcommon; atlocs: string50; base,i: integer;
      Fail: Boolean;
      
  function getfcommonname: pfcommon;
    var i: integer; lalfa8: alfa8;
  begin {getfcommonname}
  for i := 1 to 8 do
    if length(s) >= (i + 2)
    then 
      if (s[i+2] >= 'a') and (s[i+2] <= 'z')
      then lalfa8[i] := chr(ord(s[i+2]) - 32)
      else lalfa8[i] := s[i+2]
    else lalfa8[i] := ' ';
  getfcommonname := lookupfcommon(ftndatatree,lalfa8); 
  end; {getfcommonname}
  
begin {prompt}
write(message);
if ComFlag
then begin
  write(' : ');
  if eof(commandfile)
  then begin
    writeln('*** Eof ***'); close(commandfile);
    ComFlag := FALSE; prompt(message,s);
    end
  else begin readln(commandfile,s); writeln(s); end;
  end
else begin
  write(' - '); readln(s);
  if length(s) > 0
  then 
    if s[1] = '<'
    then begin
      delete(s,1,1); reset(commandfile,s);
      if ioresult = 0
      then ComFlag := TRUE
      else begin
        s := concat(s,'.TEXT'); reset(commandfile,s);
        if ioresult = 0
        then ComFlag := TRUE
        else warning('Unable to open file');
        end;
      prompt(message,s);
      end
    else 
      if s[1] = chr(27) { ESCAPE }
      then begin EscapeFlag := TRUE; goto 999; end;
  end;
if length(s) > 0
then 
  if s[1] in ['+','-','?']
  then begin
    if length(s) >= 2 then ch := s[2] else ch := ' ';
    case s[1] of
      '?': begin
           writeln('Options are:');
           writeln;
           writeln('Option Value Description:');
           writeln('------ ----- ------------');
           if AlfaFlag then ch := '+' else ch := '-';
           writeln('+A -A   ''',ch,'''  to get symbol list');
           writeln;
           writeln('+Ssymbolfilename or -Ssymbolfilename');
           writeln('       to get debugger information file');
           writeln;
           end;
      '+','-':
           if ch in ['A','a','D','d','t','T','F','f','S','s']
           then 
             case ch of
               'A','a': AlfaFlag := s[1] = '+';
               'D','d': DebugFlag := s[1] = '+';
               'S','s': 
                 if length(s) > 2
                 then 
                   if SymOpen
                   then warning('Multiple symbol files')
                   else 
                     if (ENVIRONMENT = IDRIS) or
                        (ENVIRONMENT = CPM)
                     then warning('Debugger not implemented under this O.S.')
                     else begin
                       s := copy(s,3,length(s) - 2);
                       rewrite(symfile,s);
                       if ioresult <> 0 
                       then fatal_error('Can''t open symbol file');
                       SymOpen := TRUE; symbyte := 0; symblock := 0;
                       end
                 else warning('Illegal option');
               'T','t': 
                 begin
                 lfcommon := getfcommonname;
                 lfcommon^.TextResident := TRUE; DataInText := TRUE;
                 if lfcommon^.AtAbsolute
                 then fatal_error('Must not be both text and absolute');
                 end;
               'F','f': 
                 if ENVIRONMENT = UNISOFT
                 then begin
                   lfcommon := getfcommonname;
                   lfcommon^.AtAbsolute := TRUE;
                   if lfcommon^.TextResident
                   then fatal_error('Must not be both text and absolute');
                   repeat
                     Fail := FALSE;
                     prompt('to be located at',atlocs);
                     base := 10; lfcommon^.reladdr := 0;
                     for i := 1 to length(atlocs) do
                       if atlocs[i] = '$'
                       then base := 16
                       else
                         if (atlocs[i] >= '0') and (atlocs[i] <= '9')
                         then lfcommon^.reladdr := 
                                base*lfcommon^.reladdr + ord(atlocs[i]) - ord('0')
                         else
                           if (atlocs[i] >= 'a') and 
                              (atlocs[i] <= 'f') and
                              (base = 16)
                           then lfcommon^.reladdr := 
                                  base*lfcommon^.reladdr + 
                                    ord(atlocs[i]) - ord('a') + 10 
                           else
                             if (atlocs[i] >= 'A') and 
                                (atlocs[i] <= 'F') and
                                (base = 16)
                             then lfcommon^.reladdr := 
                                    base*lfcommon^.reladdr + 
                                      ord(atlocs[i]) - ord('A') + 10 
                             else begin
                               if not Fail then warning('Badly formed location');
                               Fail := TRUE;
                               end;
                   until not Fail;
                   end
                 else fatal_error('Absolute address data areas not supported');
             end
           else warning('Illegal option');
    end; {case}
    prompt(message,s);
    end;
end; {prompt}

procedure uppercase(var s: string50);
  var i: integer; ch: char;
begin
for i := 1 to length(s) do
    begin
    ch := s[i];
    if (ch >= 'a') and (ch <= 'z')
    then s[i] := chr(ord(ch) - 32);
    end;
end; {uppercase}

function GetArg(var fname: string50): Boolean;
  var lname: string50; l: integer;
begin {GetArg}
if argc > largc
then begin
  largc := largc + 1;
  l := length(argv[largc]^);
  if l > 50 then fatal_error('Too many character in file name');
  moveleft(argv[largc]^,fname,l+1); GetArg := TRUE;
  if l > 2
  then begin
    lname := copy(argv[largc]^,1,2); {Grab initial two characters}
    if ((lname[1] = '+') or (lname[1] = '-')) and
       ((lname[2] = 's') or (lname[2] = 'S'))
    then
      if SymOpen
      then fatal_error('Multiple symbol files')
      else begin
        if (ENVIRONMENT = IDRIS) or
           (ENVIRONMENT = CPM)
        then warning('Debugger not implemented under this O.S.')
        else begin
          lname := copy(argv[largc]^,3,l - 2);
          rewrite(symfile,lname);
          if ioresult <> 0 
          then fatal_error('Can''t open symbol file');
          SymOpen := TRUE; symbyte := 0; symblock := 0;
          end;
        GetArg := GetArg(fname);
        end;
    end;
  end
else GetArg := FALSE;
end; {GetArg}

procedure initialize;
  var i: integer;
begin
if ENVIRONMENT = ADVENTURE then argcbase := 0 else argcbase := 1;
if ENVIRONMENT = IDRIS
then writeln(ITITLE,VERSION,'     ',DATE)
else 
  if (ENVIRONMENT = UNISOFT) or (ENVIRONMENT = ALTOS) or
     (ENVIRONMENT = QU68000) or (ENVIRONMENT = CT) or
     (ENVIRONMENT = LANDL) or (ENVIRONMENT = SANTACRUZ) or
     (ENVIRONMENT = NCR) or (ENVIRONMENT = ARETE) or
     (ENVIRONMENT = BERKELEY) or (ENVIRONMENT = NBI) or
     (ENVIRONMENT = PLEXUS) or (ENVIRONMENT = MULTI)
  then writeln(UTITLE,VERSION,'     ',DATE)
  else
    if ENVIRONMENT = UNOS
    then writeln(CTITLE,VERSION,'     ',DATE)
    else
      if ENVIRONMENT = ADVENTURE
      then writeln(ATITLE,VERSION,' ',DATE)
      else
        if ENVIRONMENT = REGULUS
        then writeln(RTITLE,VERSION,'     ',DATE)
        else
          if ENVIRONMENT = CPM
          then writeln(MTITLE,VERSION,'     ',DATE);
writeln('(C) Copyright 1982, 1985 Silicon Valley Software, Inc.');
for i := 0 to 9 do begin
  hex[i] := i + ord('0');
  hex[16*i] := i + ord('0');
  end;
for i := 10 to 15 do begin
  hex[i] := i - 10 + ord('A');
  hex[16*i] := i - 10 + ord('A');
  end;
rellinelen := 0;
ComFlag := FALSE; DebugFlag := FALSE; EscapeFlag := FALSE;
ListOpen := FALSE; IOutOpen := FALSE; SymOpen := FALSE;
undefrefs := 0; defrefs := 0;
undefftn := 0; lprocbase := 0; StartFlag := FALSE;
if ENVIRONMENT = ADVENTURE 
then entrycodesize := 24 
else entrycodesize := 18;
new(nametree);
with nametree^ do begin
     linkname := '$START  '; username := '$START  ';
     llink := nil; rlink := nil; sreflist := nil;
     Visible := TRUE; Defined := FALSE;
     end;
ioutbyte := 0; ioutblock := 0; reloclist := nil; errors := 0;
commonlist := nil; ftndatatree := nil; ftndatalist := nil;
fdreloclist := nil; fdfreloclist := nil; CConstructs := FALSE;
vartypelist := nil; bpointlist := nil;
DataInText := FALSE; InRelocation := FALSE;
segtree := nil; thisseg := nil; seglist := nil; overflowsegs := nil;
nextsegnum := -128;
largc := argcbase;
if GetArg(fname)
then begin {Command line mode}
  AlfaFlag := (fname <> '-l') and (fname <> '-L');
  if AlfaFlag
  then begin
    rewrite(listfile,fname,buffered);
    if ioresult <> 0 then fatal_error('Can''t open list file');
    ListOpen := TRUE;
    end;
  if GetArg(ofname)
  then begin
    rewrite(ioutfile,ofname);
    if ioresult <> 0 then fatal_error('Can''t open output file');
    IOutOpen := TRUE;
    end
  else fatal_error('Incorrectly formed command line');
  end
else begin {Prompting mode}
  prompt('Listing file',fname);
  AlfaFlag := length(fname) <> 0;
  if AlfaFlag
  then begin
    if ENVIRONMENT = ADVENTURE then uppercase(fname);
    if DebugFlag
    then rewrite(listfile,fname,unbuffered)
    else rewrite(listfile,fname,buffered);
    if ioresult <> 0 then fatal_error('Can''t open list file');
    ListOpen := TRUE;
    end;
  if ENVIRONMENT = CPM
  then begin
    repeat
      prompt('Output file [.O]',ofname);
    until length(ofname) > 0;
    uppercase(ofname);
    if (length(ofname) <= 2) or (pos('.O',ofname) <> length(ofname) - 1)
    then ofname := concat(ofname,'.O');
    end
  else
    if ENVIRONMENT = ADVENTURE
    then begin
      repeat
        prompt('Output file [.REL]',ofname);
      until length(ofname) > 0;
      uppercase(ofname);
      if (length(ofname) <= 4) or (pos('.REL',ofname) <> length(ofname) - 3)
      then ofname := concat(ofname,'.REL');
      end
    else
      if ENVIRONMENT = UNOS
      then begin
        repeat
          prompt('Output file [.j]',ofname);
        until length(ofname) > 0;
        if (length(ofname) <= 2) or (pos('.j',ofname) <> length(ofname) - 1)
        then ofname := concat(ofname,'.j');
        end
      else begin {ENVIRONMENT <> CPM, UNOS, ADVENTURE}
        repeat
          prompt('Output file [.o]',ofname);
        until length(ofname) > 0;
        if (length(ofname) <= 2) or (pos('.o',ofname) <> length(ofname) - 1)
        then ofname := concat(ofname,'.o');
        end;
  rewrite(ioutfile,ofname);
  if ioresult <> 0 then fatal_error('Can''t open output file');
  IOutOpen := TRUE;
  end;
end; {initialize}

{$I u.misc}
{$I u.phase1}
{$I u.phase2.1}
{$I u.phase2.2}
{$I u.phase2.3}
{$I u.phase2.4}

procedure sortbyloc(fsymbol: psymbol);
 var lsymbol: psymbol;
begin
if fsymbol <> nil
then with fsymbol^ do
     begin
     sortbyloc(llink); llink := nil;
     sortbyloc(rlink); rlink := nil;
     if Defined
     then if nametree = nil
          then nametree := fsymbol
          else begin
               lsymbol := nametree;
               while lsymbol <> nil do
                     if loc < lsymbol^.loc
                     then if lsymbol^.rlink = nil
                          then begin
                               lsymbol^.rlink := fsymbol;
                               lsymbol := nil;
                               end
                          else lsymbol := lsymbol^.rlink
                     else if lsymbol^.llink = nil
                          then begin
                               lsymbol^.llink := fsymbol;
                               lsymbol := nil;
                               end
                          else lsymbol := lsymbol^.llink;
               end;
     end;
end; {sortbyloc}

procedure printftndata(fftndata: pfcommon);
begin
if fftndata <> nil
then with fftndata^ do
     begin
     printftndata(rlink);
     write(listfile,name,' at ');
     if Defined
     then begin
          printhex(listfile,reladdr,6);
          if AtAbsolute
          then write(listfile,' Absolute location,  size ') 
          else
            if TextResident
            then write(listfile,' Text area relative, size ') 
            else
              if initlist <> nil
              then write(listfile,' Data area relative, size ') 
              else write(listfile,' BSS  area relative, size ');
          printhex(listfile,size,6);
          end
     else write(listfile,'*******');
     writeln(listfile);
     printftndata(llink);
     end;
end; {printftndata}

procedure psegsymbols(fsymbol: psymbol; fseg: psegment);
begin
if fsymbol <> NIL
then with fsymbol^ do
     begin
     psegsymbols(rlink,fseg);
     if fseg = seg
     then begin
          write(listfile,linkname,' - ',username,'    ');
          if Defined
          then begin
               printhex(listfile,seg^.segbaseaddr + loc,6);
               if (seg^.segname <> '        ') and
                  (seg^.segname <> 'OVERFLOW') 
               then write(listfile,',',seg^.segname);
               end
          else write(listfile,'******');
          writeln(listfile);
          end;
     psegsymbols(llink,fseg);
     end;
end; {psegsymbols}

procedure finalize;
  var lseg: psegment; i: integer; lsymbol: psymbol; totalcsize: longint;
begin
if (ListOpen and AlfaFlag) or SymOpen
then begin
  lsymbol := nametree;
  nametree := nil;
  sortbyloc(lsymbol);
  end;
if ListOpen and AlfaFlag
then begin
  if ENVIRONMENT = IDRIS
  then writeln(listfile,ITITLE,' ':2,DATE)
  else 
    if (ENVIRONMENT = UNISOFT) or (ENVIRONMENT = ALTOS) or
       (ENVIRONMENT = QU68000) or (ENVIRONMENT = CT) or
       (ENVIRONMENT = LANDL) or (ENVIRONMENT = SANTACRUZ) or
       (ENVIRONMENT = NCR) or (ENVIRONMENT = ARETE) or
       (ENVIRONMENT = BERKELEY) or (ENVIRONMENT = NBI) or
       (ENVIRONMENT = PLEXUS) or (ENVIRONMENT = MULTI)
    then writeln(listfile,UTITLE,' ':2,DATE)
    else
      if ENVIRONMENT = UNOS
      then writeln(listfile,CTITLE,' ':2,DATE)
      else
        if ENVIRONMENT = ADVENTURE
        then writeln(listfile,ATITLE,' ':2,DATE)
        else
          if ENVIRONMENT = REGULUS
          then writeln(listfile,RTITLE,' ':2,DATE)
          else
            if ENVIRONMENT = CPM
            then writeln(listfile,MTITLE,' ':2,DATE);
  writeln(listfile);
  writeln(listfile,'File: ',ofname);
  
  lseg := seglist;
  while lseg <> nil do begin
        writeln(listfile);
        writeln(listfile,'Memory map for segment ''',lseg^.segname,'''');
        writeln(listfile);
        psegsymbols(nametree,lseg);
        lseg := lseg^.nextseg;
        writeln(listfile);
        end;
  
  if ftndatatree <> nil
  then begin
       writeln(listfile);
       writeln(listfile,'Static Data Areas:');
       writeln(listfile);
       printftndata(ftndatatree);
       end;
  writeln(listfile);
  writeln(listfile,'No:  Segment:    Size:');
  i := 0; totalcsize := 0; lseg := seglist;
  while lseg <> nil do
        with lseg^ do begin
             write(listfile,i:2,'. ''',segname,'''  ');
             printhex(listfile,codesize,6);
             writeln(listfile);
             totalcsize := totalcsize + codesize;
             lseg := nextseg;
             i := i + 1;
             end;
  writeln(listfile);
  if StartFlag
  then begin
       if startseg <> nil
       then begin
         write(listfile,'Start Loc = ');
         printhex(listfile,startseg^.segbaseaddr+startloc,6);
         writeln(listfile);
         end;
       end
  else writeln(listfile,'*** No Starting Location');
  write(listfile,'Code Size = ');
  printhex(listfile,totalcsize,6);
  writeln(listfile);
  write(listfile,'Global Size = ');
  printhex(listfile,globalsize,6);
  writeln(listfile);
  close(listfile,LOCK);
  end;
if not StartFlag
then writeln('*** No Starting Location');
if SymOpen and (errors = 0) then outsymfile;
(* Removed for unix
if undefrefs > 0
then writeln(undefrefs,' undefined external references.');
*)
if undefftn > 0
then writeln(undefftn,' undefined static data areas.');
if errors > 0
then writeln(errors,' errors.');
writeln;
if IOutOpen
then if (errors > 0) or EscapeFlag
     then close(ioutfile,PURGE)
     else close(ioutfile,LOCK);
if SymOpen
then if errors > 0
     then close(symfile,PURGE)
     else begin flushsymfile; close(symfile,LOCK); end;
if ENVIRONMENT = IDRIS
then halt(ord(errors = 0))
else
  if (ENVIRONMENT = UNISOFT) or (ENVIRONMENT = ALTOS) or 
     (ENVIRONMENT = ADVENTURE) or (ENVIRONMENT = REGULUS) or
     (ENVIRONMENT = CPM) or (ENVIRONMENT = QU68000) or
     (ENVIRONMENT = CT) or (ENVIRONMENT = LANDL) or
     (ENVIRONMENT = SANTACRUZ) or (ENVIRONMENT = NCR) or
     (ENVIRONMENT = ARETE) or (ENVIRONMENT = BERKELEY) or 
     (ENVIRONMENT = NBI) or (ENVIRONMENT = PLEXUS) or
     (ENVIRONMENT = MULTI)
  then halt(ord(errors <> 0))
  else
    if ENVIRONMENT = UNOS
    then 
      if errors = 0
      then halt(undefrefs)
      else halt(-1);
end; {finalize}

begin {ulinker}
initialize;
if GetArg(fname)
then
  repeat
    reset(infile,fname);
    OK := ioresult = 0;
    if OK
    then begin
         new(thisfile);
         thisfile^.name := fname; thisfile^.lpbase := lprocbase;
         nextinblock := 0; lastinword := -1; inword := 0;
         lprocmax := lprocbase;
         phase1(thisfile^);
         lprocbase := lprocmax;
         close(infile);
         end
    else warning(concat('Can''t open input file ',fname));
  until not GetArg(fname)
else begin
  if (ENVIRONMENT = ADVENTURE) or (ENVIRONMENT = CPM)
  then prompt('Input file [.OBJ]',fname)
  else prompt('Input file [.obj]',fname);
  while length(fname) > 0 do
        begin
        if (ENVIRONMENT = ADVENTURE) or (ENVIRONMENT = CPM)
        then begin
          uppercase(fname);
          if (length(fname) <= 4) or (pos('.OBJ',fname) <> length(fname) - 3)
          then fname := concat(fname,'.OBJ');
          end
        else begin
          if (length(fname) <= 4) or (pos('.obj',fname) <> length(fname) - 3)
          then fname := concat(fname,'.obj');
          end;
        reset(infile,fname);
        OK := ioresult = 0;
        if OK
        then begin
             new(thisfile);
             thisfile^.name := fname; thisfile^.lpbase := lprocbase;
             nextinblock := 0; lastinword := -1; inword := 0;
             lprocmax := lprocbase;
             phase1(thisfile^);
             lprocbase := lprocmax;
             close(infile);
             end
        else warning('Can''t open input file');
        if (ENVIRONMENT = ADVENTURE) or (ENVIRONMENT = CPM)
        then prompt('Input file [.OBJ]',fname)
        else prompt('Input file [.obj]',fname);
        end;
  end;
phase2;
999: finalize;
end. {ulinker}

