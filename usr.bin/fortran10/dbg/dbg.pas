(*****************************************************************************)
(*                                                                           *)
(*                              File: DBG.TEXT                               *)
(*                                                                           *)
(*           (C) Copyright 1983, 1985 Silicon Valley Software, Inc.          *)
(*                                                                           *)
(*                           All Rights Reserved.                30-Oct-85   *)
(*                                                                           *)
(*****************************************************************************)


{$R-}
{$I-}
{$%+}

program dbg;

label 999;

const DATE = '30-Oct-85';
      COPYRIGHT = '(C) Copyright 1983, 1985 Silicon Valley Software, Inc.';
      CHECKSUM = '\FD\ED\FD\ED\00\00\00\00\02\40DBG version 02.40\0A\00';
      VERSION = 'V2.4';
      ALLOWDEBUG = FALSE;
      MAXLEV = 8;
      NOPARENT = 4095;

      USCOREFLAG = TRUE;

      D0 = 0; D1 = 1; D2 =  2; D3 =  3; D4 =  4; D5 =  5; D6 =  6; D7 =  7;
      A0 = 8; A1 = 9; A2 = 10; A3 = 11; A4 = 12; A5 = 13; A6 = 14; A7 = 15;
      
      MALE = 0; FEMALE = 1;
      
      { Host Machine Description: }
        
        { M68000, M68020 }
          HOSTSEX = MALE;
        {}
        
        { N32000 
          HOSTSEX = FEMALE;
        {}
    
      MERLIN = 0; UNISOFT = 2; UNOS = 3; CROMIX = 4; ADVENTURE = 6;
      REGULUS = 7; XENIX = 9; GENIX = 10; XENIX_O = 11; CT_O = 12;
      SG_O = 13; SYS5_2 = 14;
      
      ENVIRONMENT = UNISOFT;
      
      { Merlin 
      TRAP7 = $4e47; (* trap 7 *)
      ARGCBASE = 0;
      OKHALT = 0;
      ERRHALT = -1;
      CINTMIN = -2147483648;
      CINTMAX =  2147483647;
      CINTSIZE = 4;
      {}
      
      { Unisoft , Regulus }
      (* Remember to set or clear SILGRAPH *)
      TRAP7 = $4e94; (* jsr (a4) *)
      ARGCBASE = 1;
      OKHALT = 0;
      ERRHALT = -1;
      CINTMIN = -2147483648;
      CINTMAX =  2147483647;
      CINTSIZE = 4;
      (*** Altos, QU68000, Regulus
      CINTMIN = -32768;
      CINTMAX =  32767;
      CINTSIZE = 2;
      ***)
      {}
      
      { Unos 
      TRAP7 = $4e94; (* jsr (a4) *)
      ARGCBASE = 1;
      OKHALT = 0;
      ERRHALT = -1;
      CINTMIN = -2147483648;
      CINTMAX =  2147483647;
      CINTSIZE = 4;
      {}
      
      { Cromix 
      TRAP7 = $4e94; (* jsr (a4) *)
      ARGCBASE = 0;
      OKHALT = 0;
      ERRHALT = -1;
      CINTMIN = -2147483648;
      CINTMAX =  2147483647;
      CINTSIZE = 4;
      {}
      
      { Adventure 
      TRAP7 = $4e94; (* jsr (a4) *)
      ARGCBASE = 0;
      OKHALT = 0;
      ERRHALT = -1;
      CINTMIN = -2147483648;
      CINTMAX =  2147483647;
      CINTSIZE = 4;
      {}
      
      { Xenix, Xenix_o, CT_o, SG_o 
      (* Check that entry point is read correctly in readaout for SG_O *)
      TRAP7 = $4e47;
      ARGCBASE = 1;
      OKHALT = 0;
      ERRHALT = -1;
      CINTMIN = -2147483648;
      CINTMAX =  2147483647;
      CINTSIZE = 4;
      {}
      
      { Sys5_2 
      (* Remember to set or clear ALTOSFLAG *)
      TRAP7 = $4e41;
      ARGCBASE = 1;
      OKHALT = 0;
      ERRHALT = -1;
      CINTMIN = -2147483648;
      CINTMAX =  2147483647;
      CINTSIZE = 4;
      {}
      
      { Genix 
      (* Remember to set or clear OLDDYNIX, DYNIXFLAG, OPUSFLAG and TEKFLAG *)
      TRAP7 = $f2; (* BPT *)
      ARGCBASE = 1;
      OKHALT = 0;
      ERRHALT = -1;
      CINTMIN = -2147483648;
      CINTMAX =  2147483647;
      CINTSIZE = 4;
      {}
      
      SILGRAPH = TRUE;   { works with UNISOFT }
      ALTOSFLAG = FALSE; { works with SYS5_2  }
      OLDDYNIX = FALSE;  { works with GENIX   }
      TEKFLAG = FALSE;   { works with GENIX   }
      OPUSFLAG = FALSE;  { works with GENIX   }
      DYNIXFLAG = FALSE; { works with GENIX   }
      
      MINUS = -1; ZERO = 0; PLUS = 1;

type alfa8 = packed array[1..8] of char;
     alfa31 = packed array[1..31] of char;
     langs = (PASCAL,FORTRAN,C,BASIC,UNKNOWN,FTNENTRY);
     string31 = string[31];
     string63 = string[63];
     byte = -128..127;
     pbyte = ^byte;
     plong = ^longint;
     setofregs = set of D0..A7;

     balancetype = MINUS..PLUS;

     pentryrec = ^entryrec;
     entryrec = packed record linkname,username: pbyte;
                              llink, rlink: pentryrec;
                              balance: balancetype;
                              linknlen,usernlen: 0..31;
                              language: langs;
                              level: 0..MAXLEV;
                              parent: 0..4095;
                              address: longint;
                              symbols: pbyte;
                              entryoff: integer;
                              exitoff: longint;
                              statements: pbyte;
                              case wherereg: byte of
                                0: (); { Nowhere }
                                1: (regmask: integer;
                                    regoffset: longint);
                       end;
     erecarr = array[0..4095] of entryrec;
     perecarr = ^erecarr;
     segrec = record name: alfa8;
                     size: longint;
                     address: longint;
              end;
     srecarr = array[0..127] of segrec;

     typeforms = (SCALAR,SUBRANGE,POINTERS,SETS,ARRAYS,RECORDS,FILES,
                  {TAGFIELD,VARIANT,}STRINGS,SCONST,FCHAR,FARRAY);
     ptyperec = ^typerec;
     typerec = packed record
                 PckdType: Boolean;
                 case form: typeforms of
                   SCALAR:   (maxconst: integer);
                   SUBRANGE: (rangeof: integer; min,max: longint);
                   POINTERS: (pointerto: integer);
                   SETS:     (setof: integer);
                   ARRAYS:   (bitsperel: 0..8; SignedEl: Boolean;
                              arrayof,indexedby: integer);
                   RECORDS:  (recsize: longint; fields: pbyte); 
                   FILES:    (fileof: integer);
                   STRINGS,
                   SCONST:   (stringlen: 0..255);
                   FCHAR:    (charlen: integer);
                   FARRAY:   (dims: 1..7; farrayof: integer;
                              bounds: array[1..7] of
                                        record flags: 0..3;
                                               lo,hi,sz: longint;
                                               end);
                 end;
     typearr = array[0..127] of ptyperec;
     
     states = (NONE,LOADED,RUNNING,STOPPED,FINISHED,CONTINUE,STEPPING,ERROR);
     
     a5rec = record fstdin: longint;
                    fstdout: longint;
                    ffargv: longint;
                    ffargc: integer;
                    fstderr: longint;
             end;
     
     breakrec = record oldinst: integer;
                       address: longint;
                       BSet: Boolean;
                       TSet: Boolean;
                       brcount: integer;
                       brcountinit: integer;
                end;
     
     complex = record rpart: real; ipart: real; end;
     
     valu = record case vkind: integer of
                     0: ();
                     1, { Boolean }
                     2, { Integer }
                     5: { Pointer } (ivalu: longint);
                     3: (dvalu: double);
                     4: (cvalu: char);
                     6: (svalu: string[255]);
                     7: (xvalu: complex);
                     8: (rvalu: real);
                     9: (stvalu: set of 0..2031);
            end;
     
     addr = record address: longint;
                   PackedFlag,SignedFlag: Boolean;
                   leftbit,numbits: 0..15;
            end;
     
     symbols = (IDENTSY,LINKSY,ICONSTSY,RCONSTSY,SCONSTSY,
                PERIODSY,MINUSSY,UPARROWSY,LBRACKSY,RBRACKSY,
                COMMASY,EQUALSY,ASSIGNSY,COLONSY,STARSY,DOLLARSY,
                LPARENSY,RPARENSY,
                ERRSY,EOLNSY);
     
     regarr = array[0..7] of longint;
     regparr = array[0..7] of plong;
     envrec = record dentryno: integer;
                     a,d: regparr;
                     fp,sb: plong; {32000 only}
              end;
     
     bytearray = array[0..255] of byte;
     pbytearray = ^bytearray;
     argvtype = array[1..101] of pbytearray;
     pargvtype = ^argvtype;
     statustype = array[0..3] of byte;
     cint = CINTMIN..CINTMAX; {Operating system 'C' integer type}
     
     {32000 data structures}
     nsreg = (NSR0, NSR1, NSR2, NSR3, NSR4, NSR5, NSR6, NSR7,
              NSF0, NSF1, NSF2, NSF3, NSF4, NSF5, NSF6, NSF7,
              NSPC, NSSP, NSFP, NSSB, NSMODPSR);
     pmodrec = ^modrec;
     modrec = record sb: longint;
                     address: longint;
                     size: longint;
                     entry: perecarr;
                     numentrys: integer;
                     modtable: integer;
                     next: pmodrec;
              end;

     pdatarec = ^datarec;
     datarec = record size: longint;
                      address: longint;
                      name: pbyte;
                      namelen: byte;
                      balance: balancetype;
                      llink, rlink: pdatarec;
                      inmodule: pmodrec; {32000 only}
               end;
     
     drecarr = array[0..4095] of datarec; {Use only for MERLIN}

var f: file; fbuf: array[0..511] of byte; fbufp: integer;
    seg: ^srecarr;
    numsegs: integer;
    lastmodule, {Set by getpentry and getsegoff}
    modules: pmodrec;
    entry: perecarr;
    numentrys: integer;
    entrytree: pentryrec;
    datatree: pdatarec;
    numdatas: integer;
    data: ^drecarr;
    database: longint;
    types: array[0..127] of ^typearr;
    procbase, procmax: integer;
    typebase, typemax: integer;
    ch: char;
    sfname,pfname,ifname,ofname: string63;
    
    lmemaddr: pbyte;
    lmemleft: integer;
    
    { Command Scanner Variables }
    
    commandfile: text;
    CommandFileOpen: Boolean;
    line: string[200];             { Next input line }
    chno: integer;
    token: symbols;
    sval: string63;
    ival: longint;
    rval: double;
    scansize: integer;
    
    { Debugee Program Values: }
    
    hicmark,locmark,            { Initial free code boundaries }
    hidmark,lodmark: longint;   { Initial free data boundaries }
    
    prog: record d: regarr; {Doesn't contain registers in all environments}
                 a: regarr; {Doesn't contain registers in all environments}
                 pc: longint;
                 sr: integer;
                 jt: longint; {Module pointer on 32000}
                 loadaddr: longint;
                 initsp: ^a5rec;
                 state: states;
           end;
    
    fin,fout: interactive;
    largc: integer;
    largv: pbyte;
    
    { Current Environment Description }
    
    envlevel: integer; { 0 = Unknown, >0 = Known language }
    envpc: longint; { PC of current environment }
    envdepth: integer; { No. of returns from PROG.PC }
    display: array[0..MAXLEV] of envrec;
    
    { Break-Point Variables }
    
    bpoint: array[1..100] of breakrec;
    
    { Variables used to Continue from a Break-Point }
    
    contadr1,contadr2,contbadr: longint;
    contins1,contins2,contbins: integer;
    FirstBreak,PrRecFlag: Boolean;
    contcount: longint;
    
    textareabase, dataareabase, bssareabase: longint;
    timedate: longint;
    lowestversion: integer;
    
    childpid: cint; {Set by fork}
    
{******************} DebugFlag: Boolean;
QuietStep: Boolean;

procedure %_trapon; external;
procedure %_trapof; external;
function %_getsp: longint; external;
procedure %_break; external; {Adventure and Cromix}
procedure %_run(progrec: longint; procedure trap(i: integer)); external;
function %_new4(n: longint): pbyte; external;
procedure %a_2_d(var a: alfa31; digits,exp: integer; var d: double); external;

{UNISOFT cexternals}
function _ptrace(data: cint; addr: longint; pid,request: cint): cint; cexternal;
function _fork: cint; cexternal; {Also used by UNOS}
function _execv(var fargv: argvtype; var path: bytearray): longint; cexternal;
procedure _wait(var status: statustype); cexternal;
procedure _system(shellarg: pbyte); cexternal; {Also used by UNOS}

function ptrace(data: cint; addr: longint; pid,request: cint): cint; cexternal;
function fork: cint; cexternal; {Also used by UNOS}
function execv(var fargv: argvtype; var path: bytearray): longint; cexternal;
procedure wait(var status: statustype); cexternal;
procedure system(shellarg: pbyte); cexternal; {Also used by UNOS}

{UNOS cexternals}
function _pget(var fint: integer; addr, space, pid: longint): longint; cexternal;
function _pput(var fint: integer; addr, space, pid: longint): longint; cexternal;
function _pdebug(pid: longint): longint; cexternal;
function _fexecv(var fargv: argvtype; 
                 fargc,ferr,fout,fin: longint;
                 var path: bytearray): longint; cexternal;
function _execvp(var fargv: argvtype; 
                 var path: bytearray): longint; cexternal;
function _cwait(var garb1, garb2: longint): longint; cexternal;
procedure _presume(pid: longint); cexternal;

  (*** move back into init later ***)
  procedure suffix(var fname: string63; suf: string31);
    var lowername: string63; i: integer; lch: char;
  begin
  lowername := fname;
  for i := 1 to length(lowername) do begin
    lch := lowername[i];
    if (lch >= 'A') and (lch <= 'Z')
    then lowername[i] := chr(ord(lch) + 32);
    end;
  if (length(lowername) <= length(suf)) or
     (pos(suf,lowername) <> length(lowername) - length(suf) + 1)
  then fname := concat(fname,suf);
  end; {suffix}
  
procedure initialize;
  var nextarg, i: integer; ch: char;
  
  procedure oops;
  begin
  if (ENVIRONMENT = ADVENTURE) or (ENVIRONMENT = CROMIX)
  then writeln('Usage: dbg [-ssymbols[.dbg]] prog [arg1 arg2 .. argn]')
  else writeln('Usage: dbg [-ssymbols[.dbg]] [-iinfile] ',
               '[-ooutfile] prog [arg1 arg2 .. argn]');
  halt(ERRHALT);
  end; {oops}
  
  function ProcessOption(argnum: integer): Boolean;
    var keych: char; lname: string63;
  begin {ProcessOption}
  ProcessOption := FALSE;
  if argnum > argc then oops;
  if argv[argnum]^[1] = '-'
  then begin
       if length(argv[argnum]^) <= 2 then oops;
       keych := argv[argnum]^[2];
       lname := copy(argv[argnum]^,3,length(argv[argnum]^) - 2);
       if (keych = 's') or (keych = 'S')
       then begin ProcessOption := TRUE; sfname := lname; end
       else if ENVIRONMENT = ADVENTURE
            then oops
            else if (keych = 'i') or (keych = 'I')
                 then begin ProcessOption := TRUE; ifname := lname; end
                 else if (keych = 'o') or (keych = 'O')
                      then begin ProcessOption := TRUE; ofname := lname; end
                      else oops;
       end;
  end; {ProcessOption}
  
begin {initialize}
if (ENVIRONMENT <> MERLIN) and (ENVIRONMENT <> ADVENTURE) and
   (ENVIRONMENT <> UNISOFT) and (ENVIRONMENT <> UNOS) and
   (ENVIRONMENT <> CROMIX) and (ENVIRONMENT <> XENIX) and
   (ENVIRONMENT <> GENIX) and (ENVIRONMENT <> XENIX_O) and
   (ENVIRONMENT <> CT_O) and (ENVIRONMENT <> SG_O) and
   (ENVIRONMENT <> SYS5_2)
then writeln('ENVIRONMENT set to unsupported operating system');
sfname := CHECKSUM;
sfname := ''; pfname := ''; ifname := ''; ofname := '';
nextarg := ARGCBASE;
while ProcessOption(nextarg + 1) do nextarg := nextarg + 1;
pfname := argv[nextarg+1]^;
if ENVIRONMENT = CROMIX then suffix(pfname,'.bin');
if sfname = ''
then if ENVIRONMENT = CROMIX
     then sfname := copy(pfname,1,length(pfname)-4)
     else sfname := pfname;
suffix(sfname,'.dbg');
fbufp := 32000; 
reset(f,sfname);
if ioresult <> 0
then begin writeln('*** Can''t open ''',sfname,''' ***'); halt(ERRHALT); end;
if (ENVIRONMENT = UNISOFT) or (ENVIRONMENT = UNOS) or
   (ENVIRONMENT = CROMIX) or (ENVIRONMENT = XENIX) or
   (ENVIRONMENT = GENIX) or (ENVIRONMENT = XENIX_O) or
   (ENVIRONMENT = CT_O) or (ENVIRONMENT = SG_O) or
   (ENVIRONMENT = SYS5_2)
then begin largv := @argv[nextarg+1]; largc := argc - nextarg; end
else begin {MERLIN or ADVENTURE}
     if argc > (nextarg + 1)
     then largv := @argv[nextarg+2]
     else largv := nil;
     largc := argc - nextarg - 1;
     end;
if ENVIRONMENT = GENIX
then writeln('DBG - NS32000 Symbolic Debugger ',VERSION,' ':9,DATE)
else if (ENVIRONMENT = CT_O) or (ENVIRONMENT = SG_O) or
        (ENVIRONMENT = SYS5_2)
     then writeln('DBG - MC68020 Symbolic Debugger ',VERSION,' ':9,DATE)
     else writeln('DBG - MC68000 Symbolic Debugger ',VERSION,' ':9,DATE);
writeln(COPYRIGHT);
writeln;
lmemleft := 0;
for i := 0 to 127 do types[i] := nil;
CommandFileOpen := FALSE; prog.state := NONE;
for i := 1 to 100 do begin
    bpoint[i].BSet := FALSE;
    bpoint[i].TSet := FALSE;
    end;
textareabase := 0; dataareabase := 0; bssareabase := 0;
modules := nil; numentrys := 0;
new(entrytree);
with entrytree^ do begin { header node; rlink is tree; llink is tree height }
     balance := ZERO; llink := nil; rlink := nil;
     end;
new(datatree);
with datatree^ do begin { header node; rlink is tree; llink is tree height }
     size := 0; address := 0; name := nil; namelen := 0;
     balance := ZERO; llink := nil; rlink := nil; inmodule := nil;
     end;
procbase := 0; procmax := 0; typebase := 0; typemax := 0;
lowestversion := 32767; PrRecFlag := FALSE;
if ALLOWDEBUG
then begin
     write('Debug? ');
     readln(ch); DebugFlag := (ch = 'y') or (ch = 'Y');
     end
else DebugFlag := FALSE;
end; {initialize}

{$S MISC}
{$I dbg.misc.1 }
{$I dbg.misc.2 }
{$S }
{$I dbg.read }
{$I dbg.load }
{$I dbg.run.1 }
{$I dbg.run.2 }

procedure finalize;
begin
end; {finalize}

{$S MISC}
procedure doit;
  var ExitFlag: Boolean; ch: char; i: integer;
begin
initialize;
if (ENVIRONMENT = ADVENTURE) or (ENVIRONMENT = UNISOFT) or
   (ENVIRONMENT = UNOS) or (ENVIRONMENT = CROMIX) or
   (ENVIRONMENT = XENIX)
then loadprog;
if (ENVIRONMENT = GENIX) or (ENVIRONMENT = XENIX_O) or
   (ENVIRONMENT = CT_O) or (ENVIRONMENT = SG_O) or
   (ENVIRONMENT = SYS5_2)
then begin
  ExitFlag := FALSE;
  while not ExitFlag do begin
    fbufp := 0;
    if blockread(f,fbuf,1) = 1
    then begin 
      readmap; readsymbols; readbpoints;
      typebase := typemax; procbase := procmax;
      end
    else ExitFlag := TRUE;
    end;
  close(f);
  end
else begin
  readmap; readsymbols; readbpoints; close(f);
  end;
if ALLOWDEBUG
then if DebugFlag
     then begin
          write('Show SEGMENT TABLE? '); 
          readln(ch);
          if (ch = 'y') or (ch = 'Y') then prsegs;
          write('Show ENTRY TABLE? ');
          readln(ch);
          if (ch = 'y') or (ch = 'Y') then prentrys;
          write('Show DATA AREA TABLE? ');
          readln(ch);
          if (ch = 'y') or (ch = 'Y') then prdatas(datatree^.rlink);
          write('Show TYPE TABLE? ');
          readln(ch);
          if (ch = 'y') or (ch = 'Y') then prtypes;
          end;
markmemory;
if (ENVIRONMENT = GENIX ) or (ENVIRONMENT = XENIX_O) or
   (ENVIRONMENT = CT_O) or (ENVIRONMENT = SG_O) or
   (ENVIRONMENT = SYS5_2)
then readaout;
if (ENVIRONMENT = MERLIN) or (ENVIRONMENT = GENIX) or
   (ENVIRONMENT = XENIX_O) or (ENVIRONMENT = CT_O) or
   (ENVIRONMENT = SG_O) or (ENVIRONMENT = SYS5_2)
then loadprog;
if (ENVIRONMENT = UNISOFT) or (ENVIRONMENT = XENIX) or
   (ENVIRONMENT = GENIX) or (ENVIRONMENT = XENIX_O) or
   (ENVIRONMENT = CT_O) or (ENVIRONMENT = SG_O) or
   (ENVIRONMENT = SYS5_2)
then repeat
           runit;
           writeln; write('Rerun program (Y/N) ? ');
           readln(ch); ExitFlag := (ch <> 'Y') and (ch <> 'y');
           if not ExitFlag
           then begin
                loadprog;
                for i := 1 to 100 do
                    with bpoint[i] do
                         if BSet or TSet
                         then if ENVIRONMENT = GENIX
                              then setub(address,TRAP7)
                              else setuw(address,TRAP7);
                end;
     until ExitFlag
else runit;
end; {doit}

begin {dbg}
doit;
999: finalize;
end. {dbg}

