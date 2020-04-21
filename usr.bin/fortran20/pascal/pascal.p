(*****************************************************************************)
(*                                                                           *)
(*                             File: PASCAL.TEXT                             *)
(*                                                                           *)
(*           (C) Copyright 1980, 1985 Silicon Valley Software, Inc.          *)
(*                                                                           *)
(*                            All rights reserved.               30-Oct-85   *)
(*                                                                           *)
(*****************************************************************************)

{$R-}
{$I-}
{$%+}

program pascal;

label  999;
  
const
  
  DATE = '30-Oct-85';
  COPYRIGHT = '(C) Copyright 1980, 1985 Silicon Valley Software, Inc.';
  CHECKSUM = '\FD\ED\FD\ED\00\00\00\00\02\40Pascal version 02.40\0A\00';
  MAXDISPLAY = 16;
  MAXLEVEL = 8;
  MAXUNITS = 256;
  NUMRW = 42;
  MAXFILES = 5;
  MAXLBUFF = 1024;
  FIBSIZE = 120;
  MAXIDLEN = 31;
  
  MALE = 0; FEMALE = 1;
  
  M68000 = 0; N32000 = 1; M68020 = 2;
  
  { Target Machine Description: }
    
    { M68000 
      VERSION = 'V2.4'; VERSNO = 2; VERSSUBNO = 4;
      CODEGEN = 'H3.CODE';
      TITLE = 'MC68000 Pascal Compiler';
      CHIP = M68000; TARGSEX = MALE; ALLIGN32 = FALSE;
      DATAREGS = 5; ADDRREGS = 2; FLOTREGS = 0; GENPREGS = 0;
    {}
    
    { M68020 }
      VERSION = 'V2.4'; VERSNO = 2; VERSSUBNO = 4;
      CODEGEN = 'J.CODE';
      TITLE = 'MC68020 Pascal Compiler';
      CHIP = M68020; TARGSEX = MALE; ALLIGN32 = TRUE;
      DATAREGS = 5; ADDRREGS = 3; FLOTREGS = 0; GENPREGS = 0;
    {}
    
    { N32000 
      VERSION = 'V2.4'; VERSNO = 2; VERSSUBNO = 4;
      CODEGEN = 'N3.CODE';
      TITLE = 'NS32000 Pascal Compiler';
      CHIP = N32000; TARGSEX = FEMALE; ALLIGN32 = FALSE;
      DATAREGS = 0; ADDRREGS = 0; FLOTREGS = 4; GENPREGS = 4;
    {}
    
  { Host Machine Description: }
    
    { M68000, M68020 }
      HOSTSEX = MALE;
    {}
    
    { N32000 
      HOSTSEX = FEMALE;
    {}
    
  { Operating System Codes: }
  
  MERLIN = 0; IDRIS = 1; UNISOFT = 2; UNOS = 3; CROMIX = 4; ADVENTURE = 6;
  REGULUS = 7; CPM = 8; ELITE = 9; GENIX = 10; TEK = 11;
  
  { Target Operating System Description }
    
    TARGETOS = UNISOFT;
    
    { Merlin, Unisoft , Regulus, Unos, Cromix, Adventure, CPM, Elite }
      OKHALT = 0; CALLCXP = FALSE; LONGNAMES = TRUE; GLOBSTATIC = FALSE;
    {}
    
    { Idris 
      OKHALT = 1; CALLCXP = FALSE; LONGNAMES = FALSE; GLOBSTATIC = FALSE;
    {}
    
    { Genix 
      OKHALT = 0; CALLCXP = TRUE; LONGNAMES = TRUE; GLOBSTATIC = FALSE;
    {}
    
    { Tek 
      OKHALT = 0; CALLCXP = FALSE; LONGNAMES = TRUE; GLOBSTATIC = TRUE;
    {}
    
  { Host Operating System Description: }
    
    HOSTOS = UNISOFT;
    
    { Merlin 
      ERRPROMPT = '<space> to continue, <ESC> to exit - ';
      SRCSUFFIX = '.text';
      ARGCBASE = 0; ERRHALT = -1;
      EMESSFILE = '!pascterrs.text';
      EMESS2FILE = 'pascterrs.text';
    {}
    
    { Idris 
      ERRPROMPT = '<return> to continue, <delete> to exit - ';
      SRCSUFFIX = '.pas';
      ARGCBASE = 1; ERRHALT = 0;
      EMESSFILE = '/lib/pascterrs';
      EMESS2FILE = '/usr/lib/pascterrs';
    {}
    
    { Unisoft, Regulus, Genix, Tek }
      ERRPROMPT = '<return> to continue, <delete> to exit - ';
      SRCSUFFIX = '.pas';
      ARGCBASE = 1; ERRHALT = -1;
      EMESSFILE = '/lib/pascterrs';
      EMESS2FILE = '/usr/lib/pascterrs';
    {}
    
    { Unos 
      ERRPROMPT = '<return> to continue, <ctrl-C> to exit - ';
      SRCSUFFIX = '.pas';
      ARGCBASE = 1; ERRHALT = -1;
      EMESSFILE = '/lib/pascterrs';
      EMESS2FILE = '/usr/lib/pascterrs';
    {}
    
    { Cromix 
      ERRPROMPT = '<return> to continue, <ctrl-C> to exit - ';
      SRCSUFFIX = '.pas';
      ARGCBASE = 0; ERRHALT = -1;
      EMESSFILE = '/lib/pascterrs';
      EMESS2FILE = '/usr/lib/pascterrs';
    {}
    
    { Adventure 
      ERRPROMPT = '<return> to continue, <ESC><return> to exit - ';
      SRCSUFFIX = '.pas';
      ARGCBASE = 0; ERRHALT = -1;
      EMESSFILE = 'jjj:pascterr.src';
      EMESS2FILE = 'pascterr.src';
    {}
    
    { CPM 
      ERRPROMPT = '<return> to continue, <ctrl-C> to exit - ';
      SRCSUFFIX = '.pas';
      ARGCBASE = 1; ERRHALT = -1;
      EMESSFILE = 'a:pascterr.src';
      EMESS2FILE = 'b:pascterr.src';
    {}
    
    { Elite 
      ERRPROMPT = '<return> to continue, <eof> to exit - ';
      SRCSUFFIX = '#text';
      ARGCBASE = 1; ERRHALT = -1;
      EMESSFILE = 'pascterrs';
      EMESS2FILE = '';
    {}
  
type
  
  byte = -128..127;
  
  symbol = (IDENTSY,   ICONSTSY,  RCONSTSY,  DCONSTSY,  DOSY,
            OFSY,      TOSY,      FORSY,     ASSIGNSY,  NILSY,
            IFSY,      THENSY,    ELSESY,    DOWNTOSY,  BEGINSY,
            ENDSY,     WITHSY,    GOTOSY,    CONSTSY,   VARSY,
            TYPESY,    ARRAYSY,   RECORDSY,  SETSY,     FILESY,
            FUNCTSY,   PROCSY,    LABELSY,   PACKEDSY,  PROGRAMSY,
            STRINGSY,  CASESY,    WHILESY,   UNTILSY,   REPEATSY,
            CCONSTSY,  SCONSTSY,  LPARENSY,  RPARENSY,  LBRACKSY,
            RBRACKSY,  PERIODSY,  COMMASY,   SEMISY,    COLONSY,
            UPARROWSY, OTHERSY,
                   
                   { UCSD Symbols }
            
            UNITSY,    USESSY,    INTERSY,   IMPLESY,   MODULESY,
            
                   { Unary Operators }
            
            ATSIGNSY,  NOTSY,
                   
                   { Multops }
            
            STARSY,    SLASHSY,   DIVSY,     MODSY,     ANDSY,
                   
                   { Addops }
            
            PLUSSY,    MINUSSY,   ORSY,
                   
                   { Relops }
            
            LTSY,      GTSY,      LESY,      GESY,      EQSY,
            NESY,      INSY,
               
               { Error and End-of-File symbols }
            
            ERRSY,     EOFSY,
               
               { Character classes without corresponding symbols.  }
               { Note that the ordering of LETCL..DIGITCL should   }
               { not be altered and that these character classes   }
               { must appear at the end of symbol.                 }
            
            DOLLARSY,  LBRACECL,  BLANKCL,   LETCL,     DIGITCL);
  
  setofsys = set of symbol;
  
  alfa4 = packed array[1..4] of char;
  alfa8 = packed array[1..8] of char;
  alfa31 = packed array[1..31] of char;
  alfa81 = packed array[1..81] of char;
  string7 = string[7];
  string31 = string[31];
  string63 = string[63];
  pstring31 = ^string31;
  idstring = string[MAXIDLEN];
  identtype = record case Boolean of
                       FALSE: (a8: alfa8; s: idstring);
                       TRUE:  (a4: alfa4);
              end;
  ppaoc = ^paoc;
  paoc = record case integer of
                  0: (a: packed array[1..MAXIDLEN] of char);
                  1: (a4: alfa4);
                  2: (a8: alfa8);
                  3: (a12: packed array[1..12] of char);
                  4: (a16: packed array[1..16] of char);
                  5: (a20: packed array[1..20] of char);
                  6: (a24: packed array[1..24] of char);
          end;
  identrec = record case integer of
                      0: (all: alfa8);
                      1: (a: alfa4; b: alfa4);
                      2: (dummy: alfa4; tail: ppaoc; len: integer);
             end;
  
  strvaltype = ^strvalrec;
  
  strvalrec = record
    strpart: alfa8;
    next:    strvaltype;
  end;
  
  levrange = -MAXUNITS..MAXLEVEL;
  disprange = 0..MAXDISPLAY;
  
  { Symbol table types }
  
  typeforms = (SCALAR,SUBRANGE,POINTERS,SETS,ARRAYS,RECORDS,FILES,
               TAGFIELD,VARIANT,STRINGS,SCONST);
  
  declkind = (STANDARD,DECLARED);
  
  pintlist = ^intlist;
  intlist = record intval: integer; nextint: pintlist; end;
  
  plnglist = ^lnglist;
  lnglist = record lngval: longint; nextlng: plnglist; end;
  
  pt = ^t; pn = ^n;
  
  t = packed record
        typeno: integer;
        bytes: longint; bits: 0..7;
        Marked: Boolean;
        Ftype: Boolean;
        case form: typeforms of
          SCALAR:   (case scalkind: declkind of
                       DECLARED: (maxconst: pn));       { Reversed list }
          SUBRANGE: (rangeof: pt; min,max: longint);
          POINTERS: (pointerto: pt);
          SETS:     (setof: pt);
          ARRAYS:   (arrayof,indexedby: pt;
                     Pckdarr,BigIndex,BigElement,BigArray: Boolean;
                       case BitPacked: Boolean of
                         TRUE: (SignedEl: Boolean; bitsperel: 1..8));
          RECORDS:  (PckdRec: Boolean; fields: pn; 
                     fstfield: pn; varpart: pt);
          TAGFIELD: (tagname: pn; variants: pt);
          VARIANT:  (varfldlst: pn; nextvar,subvar: pt; varvalus: pintlist);
          FILES:    (PckdFile: Boolean; fileof: pt);
          STRINGS,
          SCONST:   (stringlen: 0..255);
        end;

  nodekind = (IDENTNODE,UNNODE,BINNODE,TRINODE,INDEXNODE,CSTNODE);
  
  nameclass = (CONSTS,TYPES,VARS,FIELD,PROC,FUNC,UNITS);
  
  accesskind = (DRCT,INDRCT);
  
  pfkind = (DECL,EXTDECL,FORWDECL,FORMAL);
  
  plabrec = ^labrec;
  labrec = record
             nextlabel: plabrec;
             labelno: integer;   { User label no. }
             ilabelno: integer;  { Internal label no. }
             globrefno: integer; { -1 = No Global Refs }
             Defined: Boolean;
           end;
  
  setvaltype = ^setvalrec;
  setvalrec = record nextset: setvaltype;
                setval: set of 0..31;
                end;
  
  valu = record case integer of
                  1: (ivalu: longint);
                  2: (dvalu: double);
                  3: (svalu: strvaltype; svalulen: integer);
                  4: (setvalu: setvaltype; maxsetel: integer);
                end;
  
  n = packed record
        case node: nodekind of
          IDENTNODE: (RefBelow,RefParam,InReg: Boolean; refcount: integer;
                      name: identrec; llink,rlink,next: pn; idtype: pt;
                       case class: nameclass of
                         CONSTS: (valueof: valu);
                         VARS:   (vkind: accesskind;
                                  vlev: levrange; voff: longint);
                         FIELD:  (foff: longint;
                                 { case } PckdField: Boolean;
                                   { TRUE: } bitoff: 0..15);
                         PROC,
                         FUNC:   (case pfdeclkind: declkind of
                                    STANDARD:
                                       (key: 1..999; iname: identrec);
                                    DECLARED:
                                       (pflev: levrange; pfarglist: pn;
                                        case pfdecl: pfkind of
                                          DECL,EXTDECL,FORWDECL:
                                            (CCall: Boolean; lcbits: 0..255;
                                             parmbytes,rtnno,floc: integer;
                                             lc: longint);
                                          FORMAL:
                                            (pfoff: longint)));
                         UNITS:  (ulev: levrange; ulc: longint));
          UNNODE:    (unop: 0..255; unsubop: integer; unarg: pn);
          BINNODE:   (binop: 0..255; binsubop: integer; rightarg: pn;
                       case integer of
                         0: (leftarg: pn);
                         1: (leftpt: pt));
          TRINODE:   (triop: 0..255; tript: pt; tri1,tri2: pn);
          INDEXNODE: (indexop: 9..11; InxCkBnd: Boolean; inxexpsz: 2..4;
                      inxexpr: pn; inxsz: longint;
                       case Boolean of
                         FALSE: (inxaddr: pn);
                         TRUE:  (inxbound: longint));
          CSTNODE:   (csttype: pt; cstvalu: valu);
        end;
  
  stmttypes = (BEGINST,ASSIGNST,IFST,WHILEST,REPST,WITHST,FORTOST,
               FORDOWNST,CALLST,GOTOST,CASEST,CSTMTST,LABEDST,REGST,
               FUNRESST,EPOINTST,XPOINTST);
  
  pstmt = ^stmt;
  
  stmt = packed record nextstmt: pstmt;
           stmtno: integer;
           case stmtop: stmttypes of
             BEGINST:  (subst: pstmt);
             ASSIGNST: (assop: 0..63; assbop2: 0..63; asssubop: integer;
                        assvar,assexpr: pn);
             FORTOST,
             FORDOWNST:(forvar: pn; forinit,forlimit: pn; forst: pstmt;
                        forsize: integer);
             IFST:     (ifexpr: pn; thenst,elsest: pstmt);
             WITHST:   (withvar: pn; withbody: pstmt);
             REPST,
             WHILEST:  (condexpr: pn; loopstmt: pstmt);
             CALLST:   (procpn: pn; parglist: pn);
             GOTOST:   (gotolab: plabrec; lablev: levrange);
             CASEST:   (caseexpr: pn; cstmtlist: pstmt; otherstmt: pstmt;
                        cexpsize: integer);
             CSTMTST:  (casevals: plnglist; thiscase: pstmt);
             LABEDST:  (stlab: plabrec; labstmt: pstmt);
             REGST:    (regno,regop: byte; DefReg,LoadReg,LdAddr: Boolean;
                        regexpr: pn);
             FUNRESST: (funop: integer; funval: pn);
             EPOINTST,
             XPOINTST: ();
           end;
  
  where = (BLK,REC,PARAMS);
  pboolean = ^Boolean;
  nclassset = set of nameclass;
  
  attr = record 
           asize: integer;
           typtr: pt;
           treeptr: pn;
         end;
  
  regrec = record v: pn; count,blev: integer; DoLoad,DoLdAddr: Boolean; end;
  
var
  
  token: symbol;
  intval: longint;
  doubleval: double;
  strval: strvaltype;
  ch: char;
  
  chclass: array[0..127] of symbol;
  rwsymbol: array[1..NUMRW] of symbol;
  
  blockbegsys,statbegsys,typedels,constbegsys,
  typebegsys,simptypebegsys,selectsys,facbegsys: setofsys;
  
  currentproc: pn;
  initheap,heapmark: pboolean;
  procstmt: pstmt;
  
  ufname: string63;
  
  rwnames: array[1..NUMRW] of alfa8;
  lrwnames: array[0..8] of integer;
  
  IOFlag,RangeFlag,PrintErrors,SwapFlag,DebugFlag,CCallFlag,
  PromptFlag,QuietFlag,MemCheck,FltCheck,FPHardware,NoRMW,RefValues: Boolean;
  SWDivinHW: boolean;
  
  ucstptr,utypptr,uvarptr,ufldptr,uprcptr,ufctptr: pn;
  outputptr,inputptr,stderrptr: pn;
  argvpn: pn;
  
  SourceOpen: Boolean;
  sourcefile: file;
  sfname: string63;
  EOLSource: Boolean;
  linestate: (INLINE,SAWCR,SAWLF);
  
  topoffilestack: integer;
  filestack: array[1..MAXFILES] of
    record
      lastrelblkread: integer;
      nextchtoread:   integer;
      lineinfile:     longint;
      fname:          string63;
    end;
  curlineinfile: longint;
  curfile: string63;
  
  inbufp: integer;
  inbuf: array[0..1025] of byte;
  
  numunits: integer;
  InterFlag: Boolean;
  firstublock,firstubyte,lastublock,lastubyte: integer;
  intfname: string63;
  
  listfile: text;
  ListFlag,ListOpen: Boolean;
  
  ident: identtype;
  lcbitmap: integer;
  errors: integer;
  linenumber: longint;
  stmtnumb: integer;
  frststno: integer;
  CountAhead: Boolean;
  curline,prevline: alfa81;
  pcurline, pprevline: integer;
  errfile: text;
  ErrFileOpen: Boolean;
  
  intptr,sintptr,lintptr,realptr,doubleptr,charptr,boolptr,textptr,interptr,
  nilptr,str0ptr,str1ptr,fileptr: pt;
  
  forwplist: pn;
  
  level: levrange;
  disx,top: disprange;
  display: array[disprange] of
             packed record nametree: pn;
               case occur: where of
                    BLK:    (labels,exitlabel: plabrec;
                             rootlink,bigvars: pn;
                             ftypeno,ltypeno: integer);
                    REC:    (RecPckd: Boolean; withcount,withreg: integer);
                  { PARAMS: (); }
               end;
  acount: array[0..16] of integer; { Must be >= number of registers }
  nextwreg: integer;
  uplevref,uplevloc: array[2..MAXLEVEL] of integer;
  a: array[0..ADDRREGS] of regrec;
  d: array[0..DATAREGS] of regrec;
  f: array[0..FLOTREGS] of regrec;
  r: array[0..GENPREGS] of regrec;
  
  gattr: attr;
  
  CodeOpened: Boolean;
  icodefile: file;
  cfname,ifname: string63;
  buff: array[0..511] of byte;
  lbuff: array[0..MAXLBUFF] of byte;
  byteno,blockno,lbyteno: integer;
  locprocno: integer;
  InUnit,CodeFlag,Using,OptFlag: Boolean;
  unitlist: pn;
  
  segname: alfa8;
  
  LeftHandSide: Boolean;
  
  nexttypeno: integer;
  
procedure scan; forward;

procedure fillinbuf; forward;

procedure constant(fsys: setofsys; var fvalu: valu; var fpt: pt); forward;

procedure error(errnum: integer); forward;

procedure %a_2_d(var a: alfa31; digits,exp: integer; var d: double); external;

procedure bumpstno; forward;



{$S }      {$I p.misc.0 }

{$S OPT }  {$I p.opt }

{$S DUMP } {$I p.dump.1 }
           {$I p.dump.2 }

{$S INIT } {$I p.init }

{$S DECL } {$I p.decl.1 }
           {$I p.decl.2 }
           {$I p.decl.3 }

{$S }      {$I p.body.1 }
{$S BODY } {$I p.body.2 }
           {$I p.body.3 }
           {$I p.body.4 }
           {$I p.body.5 }

{$S }      {$I p.misc.1 }
           {$I p.misc.2 }

