(*****************************************************************************)
(*                                                                           *)
(*                              File: CODE.TEXT                              *)
(*                                                                           *)
(*           (C) Copyright 1980, 1985 Silicon Valley Software, Inc.          *)
(*                                                                           *)
(*                            All rights reserved.               30-Oct-85   *)
(*                                                                           *)
(*****************************************************************************)


{$I-}
{$R-}

program code;

label 999;

const DATE = '30-Oct-85';
      VERSION = 'V2.4';
      TITLE = 'MC68000 Code Generator';
      COPYRIGHT = '(C) Copyright 1980, 1985 Silicon Valley Software, Inc.';
      CHECKSUM = '\FD\ED\FD\ED\00\00\00\00\02\40Code version 02.40\0A\00';
      D0 = 0; D1 =  1; D2 =  2; D3 =  3; D4 =  4; D5 =  5; D6 =  6; D7 =  7;
      A0 = 8; A1 =  9; A2 = 10; A3 = 11; A4 = 12; A5 = 13; A6 = 14; SP = 15;
      BYTE = 1; WORD = 2; LONG = 3; QUAD = 4;
      MAXCBUFF = 2047; MAXBLOCKS = 127; MAXBIGBLOCKS = 255;
      MAXUNITS = 256; MAXWITHLEV = 15;
      TRACEJUMPS = FALSE;
      STATISTICS = FALSE;
      SUMMARY = FALSE;

      SKY_FLAG = TRUE; SKYADDRESS = $1040; SKY_LONG = FALSE;
      MAX_FLAG = FALSE; MAXADDRESS = $ff00f4; MAX_LONG = FALSE;
      HIT_FLAG = FALSE; HITADDRESS = $000f00; HIT_LONG = FALSE;
      
      PASCAL = 0; FORTRAN = 1; BASIC = 2; C = 3;

      MERLIN = 0; IDRIS = 1; UNISOFT = 2; UNOS = 3; CROMIX = 4; ADVENTURE = 6;
      REGULUS = 7; CPM = 8;
      
      { Choose one: }
      
      { A:   SHORTCALLS = TRUE; AMAX = A3;  {}
      { B: } SHORTCALLS = FALSE; AMAX = A4; {}
      
      { Merlin, Cromix, Adventure 
        ARGCBASE = 0; ERRHALT = -1;
      {}
      
      { Idris 
        ARGCBASE = 1; ERRHALT = 0;
      {}
      
      { Unisoft, Regulus, CPM, UNOS }
        ARGCBASE = 1; ERRHALT = -1;
      {}
      
      { M68000 instruction values: }
      
      _BRA = $60; _BSR = $61; _BHI = $62; _BLS = $63;
      _BCC = $64; _BCS = $65; _BNE = $66; _BEQ = $67;
      _BVC = $68; _BVS = $69; _BPL = $6A; _BMI = $6B;
      _BGE = $6C; _BLT = $6D; _BGT = $6E; _BLE = $6F;
      
      _EXT_W = $4880;   _EXT_L = $48C0;
      _NEG_L = $4480;
      _OR_B = $8000;    _OR_W = $8040;    _OR_L = $8080;
      _ANDI_B = $0200;  _ANDI_W = $0240;  _ANDI_L = $0280;
      _ORI_B = $0000;   _ORI_W = $0040;   _ORI_L = $0080;
      _LSL_B = $E108;   _LSL_W = $E148;   _LSL_L = $E188;
      _LSR_B = $E008;   _LSR_W = $E048;   _LSR_L = $E088;
      _ASR_B = $E000;   _ASR_W = $E040;   _ASR_L = $E080;
      
      { SKY Instruction Constants: }
      
      S_FADD = $01;   S_FSUB = $07;   S_FMUL = $0B;
      
      { Maximizer Instruction Constants: }
      
      M_RESET = $80;  M_I2F = $81;    M_FNINT = $82;  M_FTRUNC = $83;
      M_FSQRT = $84;  M_FEXP = $85;   M_FLN = $86;    M_FLOG10 = $87;
      M_FSIN = $88;   M_FCOS = $89;   M_FTAN = $8A;  {M_FASIN = $8B;--
     -M_FACOS = $8C;} M_FATAN = $8D;  M_FRAC = $8E;
      M_FADD1 = $90;  M_FSUB1 = $91;  M_FMUL2 = $92;  M_FDIV2 = $93;
      M_F1SUB = $94;  M_F1DIV = $95;  M_FSQR = $96;   M_FCMP0 = $97;
      M_DADDVA = $98; M_DSUBVA = $99; M_DMULVA = $9A; M_DDIVVA = $9B;
      M_DRSBVA = $9C; M_DRDVVA = $9D;{M_FRSUB = $9E;--M_FRDIV = $9F;}
      M_FADD = $A0;   M_FSUB = $A1;   M_FMUL = $A2;   M_FDIV = $A3;
      M_FMOD = $A4;   M_FCMP = $A5;   M_IMUL4 = $A6;  M_IDIV4 = $A7;
      M_IMOD4 = $A8;  M_UDIV4 = $A9;  M_UMOD4 = $AA;
      M_DADDAA = $AC; M_DSUBAA = $AD; M_DMULAA = $AE; M_DDIVAA = $AF;
      M_D2F = $B0;    M_DNINT = $B1;  M_DTRUNC = $B2; M_F2D = $B3;
      M_I2D = $B4;   {M_DSQRT = $B5;--M_DEXP = $B6;---M_DLN = $B7;----
     -M_DLOG10 = $B8;-M_DSIN = $B9;---M_DCOS = $BA;---M_DTAN = $BB;---
     -M_DASIN = $BC;--M_DACOS = $BD;--M_DATAN = $BE;--M_DFRAC = $BF;}
      M_DADD1 = $C0;  M_DSUB1 = $C1;  M_DMUL2 = $C2;  M_DDIV2 = $C3;
      M_D1SUB = $C4;  M_D1DIV = $C5;  M_DSQR = $C6;   M_DCMP0 = $C7;
      M_DADDAV = $CC; M_DSUBAV = $CD; M_DMULAV = $CE; M_DDIVAV = $CF;
      M_DADD = $D0;   M_DSUB = $D1;   M_DMUL = $D2;   M_DDIV = $D3;
      M_DMOD = $D4;   M_DCMP = $D5;  {M_DRSUB = $D6;--M_DRDIV = $D7;}
      
      { Hitachi Floating Point Instruction Constants: }
      
      H_FADD = $01;   H_FSUB = $02;   H_FMUL = $03;   H_FCMP = $05;
      H_FTRUNCW = $06;H_FTRUNC = $07; H_W2F = $08;    H_I2F = $09;
      
      
     (**************************************************)
     (*                                                *)
     (*     Register useage:                           *)
     (*                                                *)
     (*     D0 - D7:  Used to compute expressions.     *)
     (*     A0 - A3:  Used to compute addresses.       *)
     (*     A4:       Points to procedure jump table.  *)
     (*     A5:       Global Base Register.            *)
     (*     A6:       Local Base Register.             *)
     (*     SP:       Top Of Stack pointer.            *)
     (*                                                *)
     (**************************************************)
     
type register = D0..SP;
     addrrange = -8388608..8388607;
     cattrkind = (EXPR,ADDR,INDX,CNST,COND,VARB,FDAT,BOOL,STCK,BITZ);
     condition = (T,F,HI,LS,HS,LO,NE,EQ,VC,VS,PL,MI,GE,LT,GT,LE);
     constkind = (SCALCNST,REALCNST,STRCNST,PAOCCNST,SETCNST);
     
     alfa4 = packed array[1..4] of char;
     alfa8 = packed array[1..8] of char;
	 { added by jim t at sgi for comparing pointers to addresses }
	 pidstring = ^idstring;
     idstring = string[31];
     pstrings = ^strings;
     strings = record case integer of
                        0: (s3: string[3]);
                        1: (s7: string[7]);
                        2: (s11: string[11]);
                        3: (s15: string[15]);
                        4: (s19: string[19]);
                        5: (s23: string[23]);
                        6: (s27: string[27]);
                        7: (s: idstring);
               end;
     linkid = record linkno: integer; n4: alfa4; tail: pstrings; end;
     
     pstrcrec = ^strcrec;
     strcrec = record strval: alfa8;
                      next: pstrcrec;
               end;
     
     pintlist = ^intlist;
     intlist = record int: integer;
                      next: pintlist;
               end;
     
     preflist = ^reflist;
     reflist = record refblk,refoff: integer;
                      next: preflist;
               end;
     
     valu = record case cstkind: constkind of
                     SCALCNST: (case Boolean of
                                  FALSE: (ivalu: array[0..3] of integer);
                                  TRUE:  (lvalu: longint; lvalu2: longint));
                     STRCNST,
                     PAOCCNST: (len: integer; strvalu: pstrcrec);
                     SETCNST:  (setbytes: integer; setvalu: pintlist;
                                FrontAddress: Boolean);
            end;
     
     plitcref = ^litcref;
     litcref = record litclab: integer;
                      litvalsz: LONG..QUAD;
                      litval,litval2: longint;
                      next: plitcref;
               end;
     pbigcref = ^bigcref;
     bigcref = record bigclab: integer;
                      bigval: valu;
                      next: pbigcref;
               end;
     pfmtrec = ^fmtrec;
     fmtrec = record fmt: valu;
                     fmtno,fmtilabno: integer;
                     next: pfmtrec;
              end;
     
     pregrec = ^regrec;
     
     cattr = record
               case ckind: cattrkind of
                    EXPR,
                    BOOL,
                    BITZ: (exreg: pregrec);
                    ADDR: (aoffset: integer; adreg: pregrec);
                    VARB: (voffset: integer; vlev: integer);
                    FDAT: (doffset: addrrange; flev: integer);
                    INDX: (inoffset: integer; inxareg: pregrec;
                           inxrreg: pregrec; LongIndex: Boolean);
                    CNST: (cvalu: valu);
                    COND: (cc: condition);
                    STCK: ();
               end;
     
     regrec = record count: integer;
                     InReg: Boolean;
                     regno: register;
                     InMem: Boolean;
                     memcattr: cattr;
              end;
     
     ptmplist = ^tmplist;
     tmplist = record tmpcattr: cattr;
                      addr,size: longint;
                      next: ptmplist;
               end;
     
     bkinds = (BLNK,BMEM,BREG,BADR);
     flags = (ABSI,JMPS,JMPL,NOPI,CASJ,PREL);
     
     pprocref = ^procref;
     procref = record procname: linkid;
                      reflist: preflist;
                      next: pprocref;
                      pflev: integer;
               end;
     approcref = array[0..31] of pprocref;
     
     plabarr = ^labarr;
     labarr = array[0..255] of integer;
     
     puserlabel = ^userlabel;
     userlabel = record userno,intno: integer;
                        linkerno: integer; {-1 = not global }
                        next: puserlabel;
                 end;
     pblkoff = ^blkoff;
     blkoff = record blk,off: integer; next: pblkoff; end;
     pcommonrec = ^commonrec;
     commonrec = record left,right: pcommonrec;
                        hashno,commonno: integer;
                        datasize: addrrange;
                        comname: linkid;
                        reflist: pblkoff;
                 end;
     pentryrec = ^entryrec;
     entryrec = record next: pentryrec;
                       entname: linkid;
                       entryblk,entryoff: integer;
                       dbgenblk,dbgenoff,dbgexblk,dbgexoff: integer;
                end;
     
     ptrans = ^transrec;
     transrec = record next: ptrans;
                       freeword: integer;
                       b: array[0..999] of integer;
                end;
     
     ptransblk = ^transblk;
     transblk = record next: ptransblk;
                       tsize: longint;
                       tblock,tbyte: integer;
                end;
     
	 { linenumber and filename added by jim t at sgi for dbx }
     addrrec = record blk,off,linenumber: integer; filename: pidstring; end;
     pstlocrec = ^stlocrec;
     stlocrec = record next: pstlocrec;
                       loc: array[0..99] of addrrec;
                end;
     
     bite = -128..127;
     pcaserec = ^caserec;
     caserec = record next: pcaserec;
                      cval: longint;
                      clab: integer;
               end;
     pcaselst = ^caselst;
     caselst = record next: pcaselst;
                      thistable: pcaserec;
                      entrysz: 0..4; TabFarAway,AnyHoles: Boolean;
                      numentrys,tablabno,elselabno: integer;
                      cjmpaddr,ctabaddr: longint;
               end;
     tags = (TAG_END,TAG_BR8,TAG_BR4,TAG_BR2,TAG_BR0,TAG_PC2,TAG_LC,
             TAG_LOAD,TAG_SAVE,TAG_A0,TAG_RBYT,TAG_CJMP,TAG_CTAB,TAG_LAB,
             TAG_WITH);
     pblk = ^block;
     block = record blkaddr: longint; blksize: integer;
                    DeadCode: Boolean;
                    case blktag: tags of
                      TAG_BR8,
                      TAG_BR4,
                      TAG_BR2,
                      TAG_BR0,
                      TAG_PC2:  (brop: bite; refblk: integer);
                      TAG_LC,
                      TAG_LOAD,
                      TAG_SAVE,
                      TAG_A0,
                      TAG_LAB,
                      TAG_END:  ();
                      TAG_RBYT: (roff: integer);
                      TAG_CJMP,
                      TAG_CTAB: (creg: register; cvallist: pcaselst);
                      TAG_WITH: (wreg: register; SaveWith: Boolean;
                                 withoff: integer);
             end;
     pblkrec = ^blkrec;
     blkrec = array[0..MAXBLOCKS] of block;
     pcbuf = ^cbuf;
     cbuf = record buff: array[0..MAXCBUFF] of integer;
                   next,last: pcbuf;
            end;
     rvclass = (UNKNRV,CNSTRV,ADDRRV,VALURV,INDRRV,INDXRV);
     regvalrec = record rvsize: 0..4; {BYTE, WORD, LONG, QUAD}
                        case rvkind: rvclass of
                          UNKNRV: ();
                          CNSTRV: (ival: longint);
                          ADDRRV,
                          VALURV,
                          INDRRV,
                          INDXRV: (rvlev: integer; {1..N: level, <=0: FDATA }
                                   rvoffset: longint;
                                   rvReadOnly: Boolean; { Val in memory }
                                                        { never changes }
                                   rvinxcst: integer); { Only used by INDXRV }
                 end;
     
var level: integer;
    quickforget : boolean;
    lc,extralc,ftnstaticlc: addrrange;
    gcattr,SPcattr: cattr;
    proclist: pprocref;
    bigclist: pbigcref;
    litclist: plitcref;
    gcaselist: pcaselst;
    templist: ptmplist;
    reg: array[register] of
            record r: pregrec;
                   t: integer;
                   Locked,Perm,Used: Boolean;
                   v: regvalrec;
            end;
    timer: integer;
    ccodereg: register;     { D0-D7 valid; SP invalid }
    bases: array[0..31] of
             record case bwhere: bkinds of
                      BLNK: ();
                      BMEM: (boffset: integer);
                      BREG: (bregno: integer);
                      BADR: (baoffset,balev: integer);
                    end;
    pc: longint;
    codeword: integer;
    cbuflist,thiscbuf: pcbuf;
    blktable: array[0..MAXBIGBLOCKS] of pblkrec;
    nextblk: integer;
    blkpc: longint;
    heapmark: ^Boolean;
    op: integer;
    
    infile: file;
    inbuff: array[0..511] of -128..127;
    inblock,inbyte: integer;
    InFOpen,SaveIFile: Boolean;
    
    outfile: file;
    outbuff: array[0..255] of integer;
    outblock,outword: integer;
    OutFOpen: Boolean;
    
    errors: integer;
    HalfFlag: Boolean;
    halfvalue: integer;
    
    withlevel: integer;
    withcattr: array[1..MAXWITHLEV] of
                    record cat: cattr;
                           wkind: (WCSTAD,WINREG,WINMEM);
                           wblockno: integer;
                    end;
    
    neglabs,poslabs: array[0..127] of plabarr;
    ulabellist: puserlabel;
    
    nextlabel: integer;
    
    userprocs: array[0..31] of ^approcref;
    
    FPHardware,GlobalLabels,LongNames: Boolean;
    ZeroStack, SWDivinHW: Boolean;
    gotocattr: cattr;
    
    csize: longint;
    
    shortjumps: array[1..5] of integer;
    
    UnitFlag,GlobalInfo: Boolean;
    utextaddr,utextsize: integer;
    lastmodule: longint;                 { Disk address of last module  }
    
    ftndtree: pcommonrec;
    entrylist: pentryrec;
    BeginMess: Boolean;
    username: idstring;
	{ added by jim t at sgi for dbx }
	curfilename: pidstring;
    segname: alfa8;
    entryid: linkid;
    entrylab: integer;
    
    language: integer;
    fmtlist: pfmtrec;
    A0SetFlag: Boolean;
    altrtnic: integer;
    translist,lasttrans: ptrans;
    tblocks,lasttblock: ptransblk;
    crtnlab: integer;
    
    sizebits: array[BYTE..LONG] of integer;  { Maps BYTE..LONG to ins.field }
    csizes: array[0..4] of BYTE..QUAD;
    tname: array[0..25] of alfa8;            { Call names for trans ops }
    TArgKind: array[0..25] of Boolean;       { TRUE = Ref parameter }
    QuietFlag: Boolean;
    targetos: integer;
    popmask,pushmask,regsavsz,lastreg: integer;
    
    stloclist,thisstloc: pstlocrec;
    numstmts: integer;
    DebugInfo: Boolean;
    
    LastPop: Boolean;           { True if last instruction   }
    popbytes: integer;          { was: "ADD #popbytes,SP"    }
    LastPushQ: Boolean;         { was: "MOVEM.L D0/D1,-(sp)" }
    
    { Statistical information: }
    
    totalblocks,totalilabels,xtralblks,deadbytes: longint;
    totalconstsinregs,totalvarsinregs,totalfvarsinregs: longint;
    totalwithsnotsaved,totaladdrsinregs: integer;
    totalindxsinregs: integer;
    
procedure initialize;
  var fname,lname: string[63];
      ch: char;
      i: integer;
  
  procedure initnames;
    var i: integer;
  begin
  tname[$00] := '%_SIN   '; tname[$01] := '%_PDSIN ';
  tname[$02] := '%_COS   '; tname[$03] := '%_PDCOS ';
  tname[$04] := '%_LN    '; tname[$05] := '%_PDLN  ';
  tname[$06] := '%_EXP   '; tname[$07] := '%_PDEXP ';
  tname[$08] := '%_ATAN  '; tname[$09] := '%_PDATN ';
  tname[$0A] := '%_SQRT  '; tname[$0B] := '%_PDSQT ';
  tname[$0C] := '%_FTAN  '; tname[$0D] := '%_DTAN  ';
  tname[$0E] := '%_FASIN '; tname[$0F] := '%_DASN  ';
  tname[$10] := '%_FACOS '; tname[$11] := '%_DACS  ';
  tname[$12] := '%_FLOG10'; tname[$13] := '%_DL10  ';
  tname[$14] := '%_FSINH '; tname[$15] := '%_DSNH  ';
  tname[$16] := '%_FCOSH '; tname[$17] := '%_DCSH  ';
  tname[$18] := '%_FTANH '; tname[$19] := '%_DTNH  ';
  for i := 0 to 11 do TArgKind[i] := FALSE;
  for i := 12 to 25 do TArgKind[i] := TRUE;
  end; {initnames}
  
begin {initialize}
writeln(TITLE,'  ',VERSION,' ':17,DATE);
writeln(COPYRIGHT);
writeln;

if ord(SKY_FLAG) + ord(MAX_FLAG) + ord(HIT_FLAG) > 1
then begin
     writeln('*** Multiple Floating Point Environments! Recompile! ***');
     halt(-1);
     end;
fname := CHECKSUM;
errors := 0; csize := 0;

{ a cludge to fix SCR 3613 : j = j + gg(j) :dar}
quickforget := FALSE;

InFOpen := FALSE; OutFOpen := FALSE;
if STATISTICS or SUMMARY
then begin
     for i := 1 to 5 do shortjumps[i] := 0;
     totalblocks := 0; totalilabels := 0; xtralblks := 0;
     deadbytes := 0; totalconstsinregs := 0;
     totalvarsinregs := 0; totalfvarsinregs := 0;
     totalwithsnotsaved := 0; totaladdrsinregs := 0;
     totalindxsinregs := 0;
     end;

if argc > ARGCBASE
then begin
     SaveIFile := FALSE;
     reset(infile,argv[ARGCBASE + 1]^);
     if ioresult <> 0
     then begin
          errors := errors + 1;
          writeln('*** Unable to open input file ***');
          goto 999;
          end;
     InFOpen := TRUE;
     if argc > ARGCBASE + 1
     then fname := argv[ARGCBASE + 2]^
     else begin
          lname := argv[ARGCBASE + 1]^;
          fname := concat(copy(lname,1,length(lname) - 2),'.obj');
          end;
     rewrite(outfile,fname);
     if ioresult <> 0
     then begin
          errors := errors + 1;
          writeln('*** Unable to open output file ***');
          goto 999;
          end;
     OutFOpen := TRUE;
     end
else begin
     SaveIFile := TRUE;
     write('Input file [.I] - ');
     readln(lname);
     if length(lname) = 0 then lname := 'X';
     reset(infile,lname);
     if ioresult <> 0
     then begin
          fname := concat(lname,'.I');
          reset(infile,fname);
          if ioresult <> 0
          then begin
               errors := errors + 1;
               writeln('*** Unable to open input file ***');
               goto 999;
               end;
          end;
     InFOpen := TRUE;
     write('Output file [',lname,'][.obj] - ');
     readln(fname);
     if length(fname) = 0 then fname := lname;
     fname := concat(fname,'.obj');
     rewrite(outfile,fname);
     if ioresult <> 0
     then begin
          errors := errors + 1;
          writeln('*** Unable to open output file ***');
          goto 999;
          end;
     OutFOpen := TRUE;
     end;

DebugInfo := FALSE;
outblock := 0; outword := 0;
inblock := -1; inbyte := 32767;
HalfFlag := FALSE;
mark(heapmark);
lastmodule := 0;
csizes[0] := BYTE; csizes[1] := WORD; csizes[2] := LONG;
csizes[3] := LONG; csizes[4] := QUAD;
sizebits[BYTE] := 0; sizebits[WORD] := 64; sizebits[LONG] := 128;
initnames;
end; {initialize}

(****** Debug routines *******)

procedure prreg(freg: register);
  var ch: char; lregno: integer;
begin
if freg = SP
then write('SP')
else begin
     if freg <= D7
     then begin ch := 'D'; lregno := freg; end
     else begin ch := 'A'; lregno := freg - 8; end;
     write(ch,lregno);
     end;
end; {prreg}

procedure prregrec(fregrec: pregrec);
begin
with fregrec^ do begin
     write('CNT=',count);
     if InReg then write(' InReg      ') else write(' Not InReg (');
     prreg(regno);
     if InReg
     then if reg[regno].Perm
          then write('* ')
          else write('  ')
     else write(') ');
     if InMem
     then with memcattr do begin
               write(' InMem, kind=');
               if memcattr.ckind = VARB
               then write('VARB, vlev=',vlev,', voffset=',voffset)
               else if memcattr.ckind = FDAT
               then write('FDAT, flev=',flev,', doffset=',doffset)
               else write('????');
               end
     else write(' Not InMem');
     end;
end; {prregrec}

procedure prregs;
  var rr,firstreg,lastreg: register; Skipped: Boolean;
begin
Skipped := FALSE;
for rr := D0 to SP do begin
    with reg[rr] do begin
         if t <> 0
         then begin
              if Skipped
              then begin
                   prreg(firstreg); write(' .. '); prreg(lastreg);
                   writeln(' unused.');
                   end;
              prreg(rr); write(': ');
              write('T=',t:3);
              if Locked then write(',   Locked, ') else write(', Unlocked, ');
              prregrec(r); writeln;
              Skipped := FALSE;
              end
         else begin
              if not Skipped then firstreg := rr;
              lastreg := rr; Skipped := TRUE;
              end;
         end;
    end;
if Skipped
then begin
     prreg(firstreg); write(' .. '); prreg(lastreg);
     writeln(' unused.');
     end;
writeln;
end; {prregs}

procedure prcattr(var fcattr: cattr);
begin
with fcattr do begin
     write('CKIND=');
     case ckind of
       EXPR: begin write('EXPR, exreg='); prregrec(exreg); end;
       BOOL: begin write('BOOL, exreg='); prregrec(exreg); end;
       BITZ: begin write('BITZ, exreg='); prregrec(exreg); end;
       ADDR: begin write('ADDR, aoffset=',aoffset,', adreg=');
                   prregrec(adreg);
             end;
       VARB: write('VARB, voffset=',voffset,', vlev=',vlev);
       FDAT: write('FDAT, doffset=',doffset,', flev=',flev);
       INDX: write('INDX, ...');
       CNST: write('CNST, ...');
       COND: write('COND, cc=',ord(cc));
       STCK: write('STCK');
     otherwise: write('????');
     end; {case}
     end;
writeln;
end; {prcattr}

(****** End of DEBUG ******)

{$I h.code.1 }
{$I h.code.2 }
{$I h.code.3 }
{$I h.code.max }
{$S SEG1}
{$I h.code.4 }
{$I h.code.5 }
{$I h.code.6 }
{$S SEG2}
{$I h.code.7 }
{$I m.code.8 }
{$I h.code.9 }

procedure unitheader;
  var i,junk,flags: integer; unitname: idstring;
begin
UnitFlag := TRUE;
junk := nextbyte;
utextaddr := nextword div 2;
junk := nextbyte;
junk := nextbyte;
utextsize := nextword div 2;
junk := nextbyte;
flags := nextword;
GlobalInfo := odd(flags);
nexts(unitname);

{ Unit Block: }

out($92); out3(28); outs8(unitname);
out4(28); out4(0); out3(utextsize*2); out(0); out4(0);

op := nextbyte;
end; {unitheader}

procedure c_header;
  var i: integer; c_name: idstring;
begin
GlobalInfo := TRUE;
nexts(c_name);

{ Unit Block: }

out($92); out3(28); outs8(c_name);
out4(28); out4(0); out4(0); out4(0);

op := nextbyte;
end; {c_header}

procedure unittail;
  var i,fromblock: integer;
begin
if outword > 0 then flushout;
if blockread(outfile,outbuff,1,0) <> 1
then error(401);
outbuff[8] := outblock div 128;
outbuff[9] := outblock*512;
if blockwrite(outfile,outbuff,1,0) <> 1
then error(408);
fromblock := utextaddr;
for i := 1 to utextsize do begin
    if blockread(infile,outbuff,1,fromblock) <> 1
    then error(409);
    if blockwrite(outfile,outbuff,1,outblock) <> 1
    then error(400);
    fromblock := fromblock + 1;
    outblock := outblock + 1;
    end;
end; {unittail}

procedure markglobinfo;
  var i,fromblock: integer;
begin
if outword > 0 then flushout;
if blockread(outfile,outbuff,1,0) <> 1
then error(401);
{ Patch in address of initialization module }
outbuff[12] := lastmodule div 65536;
outbuff[13] := lastmodule;
if blockwrite(outfile,outbuff,1,0) <> 1
then error(408);
end; {markglobinfo}

procedure checkversion;
  var language,version,subvers,vlo,vhi,k: integer; name: string[7];
begin
language := nextbyte; version := nextbyte; subvers := nextbyte;
case language of
  PASCAL:  begin 
           vlo := 204; vhi := 204; name := 'Pascal';
           end;
  FORTRAN: begin
           vlo := 204; vhi := 204; name := 'FORTRAN';
           end;
  C:       begin
           vlo := 203; vhi := 204; name := '"C"';
           end;
otherwise: begin 
           writeln('\07*** Input file is not an .I file ***');
           errors := errors + 1; goto 999;
           end;
end; {case}
k := version*100 + subvers;
if (k < vlo) or (k > vhi)
then begin
     writeln('This code generator requires ',name,' front end version ',
             vlo div 100,'.',vlo mod 100,' through ',vhi div 100,'.',
             vhi mod 100,', not version ',version,'.',subvers,'!');
     errors := errors + 1; goto 999;
     end;
op := nextbyte;
end; {checkversion}

procedure middleize;
begin
UnitFlag := FALSE; GlobalInfo := FALSE; op := nextbyte;
if op = $fb
then checkversion
else begin
     writeln('This code generator is for language versons 2.0 and later!');
     errors := errors + 1; goto 999;
     end;
if op = $f9 {C}
then c_header
else if op = 244 {UNIT}
     then unitheader
     else if op <> $f0
          then begin
               writeln('\07*** Input file is not an .I file ***');
               errors := errors + 1;
               goto 999;
               end;
while op = $f0 {MODULE} do begin
      lastmodule := (ord4(outblock)*256 + outword)*2;
      genproc;
      op := nextbyte;
      end;
if op <> $ff {ENDICODE} then error(2001);
out2(0);
if UnitFlag then unittail;
if GlobalInfo then markglobinfo;
end; {middleize}

{$S SEG1}
procedure loadem;
begin
middleize;
end; {loadem}

{$S}
procedure finalize;
  var AnyWayFlag: Boolean; ch: char; total,i: integer;
begin
AnyWayFlag := FALSE;
if OutFOpen
then begin
     if errors <> 0
     then begin
          writeln('*** There were ',errors,' errors ***');
          write('Do you want to save the code file anyway (Y/N) - ');
          readln(ch);
          AnyWayFlag := (ch = 'Y') or (ch = 'y');
          end;
     if (errors = 0) or AnyWayFlag
     then begin
          if outword > 0
          then flushout;
          close(outfile,LOCK);
          end
     else close(outfile,PURGE);
     end;
if InFOpen
then if SaveIFile
     then close(infile,LOCK)
     else close(infile,PURGE);
if STATISTICS or SUMMARY
then begin
     write('Jumps:'); for i := 1 to 5 do write(' ',shortjumps[i]); writeln;
     writeln('TBlocks: ',totalblocks,', TLabels: ',totalilabels,
             ', XtraLBlocks: ',xtralblks);
     writeln('DeadBytes: ',deadbytes);
     writeln('ConstsInRegs: ',totalconstsinregs,', VarsInRegs: ',
             totalvarsinregs,', FVarsInRegs: ',totalfvarsinregs);
     writeln('WithsNotSaved: ',totalwithsnotsaved,
             ', AddrsInRegs: ',totaladdrsinregs,
             ', IndxsInRegs: ',totalindxsinregs);
     end;
if (STATISTICS or SUMMARY) or not QuietFlag
then writeln('Total code size = ',csize);
if errors > 0 then halt(ERRHALT);
end; {finalize}

begin {code}
initialize;
loadem;
999: finalize;
end. {code}

