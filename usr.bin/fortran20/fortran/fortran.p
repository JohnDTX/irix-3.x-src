(*****************************************************************************)
(*                                                                           *)
(*                             Program FORTRAN77                             *)
(*                                                                           *)
(*           (C) Copyright 1981, 1985 Silicon Valley Software, Inc.          *)
(*                                                                           *)
(*                            All Rights Reserved.               30-Oct-85   *)
(*                                                                           *)
(*****************************************************************************)


{$R-}
{$I-}
{$%+}

program fortran77;

label 999;

const DATE = '30-Oct-85';
      COPYRIGHT = '(C) Copyright 1981, 1985 Silicon Valley Software, Inc.';
      CHECKSUM = '\FD\ED\FD\ED\00\00\00\00\02\40FORTRAN version 02.40\0A\00';
      MAXFILES = 5;
      MAXLBUFF = 2048;
      MAXIDLEN = 31;
      MAXCOMIDLEN = 29;  { Should be: 6 <= MAXCOMIDLEN <= MAXIDLEN - 2 }
      MAXCHAR = 4096;
	MAXCONTLINES = 100;
      
      MALE = 0; FEMALE = 1;
      
      M68000 = 0; N32000 = 1; M68020 = 2;
      
      { Target Machine Description: }
        
        { M68000 }
#ifndef FOR_68020
          VERSION = 'V2.4'; VERSNO = 2; VERSSUBNO = 4;
          TITLE = 'MC68000 FORTRAN 77 Compiler';
          CHIP = M68000; TARGSEX = MALE; ALLIGN32 = FALSE;
          DATAREGS = 5; ADDRREGS = 3; FLOTREGS = 0; GENPREGS = 0;
          MAXLOCAL = 30000;
#endif
        {}
        
        { M68020 }
#ifdef FOR_68020
          VERSION = 'V2.4'; VERSNO = 2; VERSSUBNO = 4;
          TITLE = 'MC68020 FORTRAN 77 Compiler';
          CHIP = M68020; TARGSEX = MALE; ALLIGN32 = TRUE;
          DATAREGS = 5; ADDRREGS = 3; FLOTREGS = 0; GENPREGS = 0;
          MAXLOCAL = 30000;
#endif
        {}
        
        { N32000 
          VERSION = 'V2.4'; VERSNO = 2; VERSSUBNO = 4;
          TITLE = 'NS32000 FORTRAN 77 Compiler';
          CHIP = N32000; TARGSEX = FEMALE; ALLIGN32 = FALSE;
          DATAREGS = 0; ADDRREGS = 0; FLOTREGS = 4; GENPREGS = 4;
          MAXLOCAL = 16000000;
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
        
        { Merlin, Unisoft, Regulus, Unos, Cromix, Adventure, CPM, Elite }
          OKHALT = 0; CALLCXP = FALSE; LONGNAMES = TRUE;
        {}
        
        { Idris 
          OKHALT = 1; CALLCXP = FALSE; LONGNAMES = FALSE;
        {}
        
        { Genix 
          OKHALT = 0; CALLCXP = TRUE; LONGNAMES = TRUE;
        {}
        
        { Tek 
          OKHALT = 0; CALLCXP = FALSE; LONGNAMES = TRUE;
        {}
        
      { Host Operating System Description: }
      
      HOSTOS = UNISOFT;
      
      { Merlin 
        ERRPROMPT = '<space> to continue, <ESC> to exit - ';
        SRCSUFFIX = '.text';
        ARGCBASE = 0; EOLCH = '\0D'; ERRHALT = -1;
        EMESSFILE = '!ftncterrs.text';
        EMESS2FILE = 'ftncterrs.text';
      {}
      
      { Idris 
        ERRPROMPT = '<return> to continue, <delete> to exit - ';
        SRCSUFFIX = '.for';
        ARGCBASE = 1; EOLCH = '\0A'; ERRHALT = 0;
        EMESSFILE = '/lib/ftncterrs';
        EMESS2FILE = '/usr/lib/ftncterrs';
      {}
      
      { Unisoft, Regulus, Genix, Tek }
        ERRPROMPT = '<return> to continue, <delete> to exit - ';
        SRCSUFFIX = '.for';
        ARGCBASE = 1; EOLCH = '\0A'; ERRHALT = -1;
        EMESSFILE = '/lib/ftncterrs';
        EMESS2FILE = '/usr/lib/ftncterrs';
      {}
      
      { Unos 
        ERRPROMPT = '<return> to continue, <ctrl-C> to exit - ';
        SRCSUFFIX = '.for';
        ARGCBASE = 1; EOLCH = '\0A'; ERRHALT = -1;
        EMESSFILE = '/lib/ftncterrs';
        EMESS2FILE = '/usr/lib/ftncterrs';
      {}
      
      { Cromix 
        ERRPROMPT = '<return> to continue, <ctrl-C> to exit - ';
        SRCSUFFIX = '.for';
        ARGCBASE = 0; EOLCH = '\0A'; ERRHALT = -1;
        EMESSFILE = '/lib/ftncterrs';
        EMESS2FILE = '/usr/lib/ftncterrs';
      {}
      
      { Adventure 
        ERRPROMPT = '<return> to continue, <ESC><return> to exit - ';
        SRCSUFFIX = '.for';
        ARGCBASE = 0; EOLCH = '\0D'; ERRHALT = -1;
        EMESSFILE = 'jjj:ftncterr.src';
        EMESS2FILE = 'ftncterr.src';
      {}
      
      { CPM 
        ERRPROMPT = '<return> to continue, <ctrl-C> to exit - ';
        SRCSUFFIX = '.for';
        ARGCBASE = 1; EOLCH = '\0A'; ERRHALT = -1;
        EMESSFILE = 'a:ftncterr.src';
        EMESS2FILE = 'b:ftncterr.src';
      {}
      
      { Elite 
        ERRPROMPT = '<return> to continue, <eof> to exit - ';
        SRCSUFFIX = '#text';
        ARGCBASE = 1; EOLCH = '\0D'; ERRHALT = -1;
        EMESSFILE = 'ftncterrs';
        EMESS2FILE = '';
      {}
      

type types = (INTEGR, REEL, CHARACTER, LOGICAL, COMPLEX);
     
     alfa2 = packed array[1..2] of char;
     alfa4 = packed array[1..4] of char;
     alfa6 = packed array[1..6] of char;          { Labels }
     alfa8 = packed array[1..8] of char;
     alfa31 = packed array[1..31] of char;
     alfa = packed array[1..10] of char;          { Char consts and FORMATs }
     idstring = string[MAXIDLEN];
     string16 = string[16];
     string31 = string[31];
     string32 = string[32];
     string63 = string[63];
     
     symbol = (IDENTSY,  ICONSTSY, RCONSTSY, LCONSTSY, CCONSTSY,
               DCONSTSY, LPARENSY, RPARENSY, PERIODSY, COMMASY,
               ASSIGNSY, DOLLARSY, COLONSY,  CONCATSY,
               EXPSY,    STARSY,   SLASHSY,  PLUSSY,   MINUSSY,
               LTSY,     GTSY,     LESY,     GESY,     EQSY,
               NESY,     EQVSY,    NEQVSY,
               NOTSY,    ANDSY,    ORSY,
               EOLNSY,   ERRSY,    LETCL,    DIGITCL);
     setofsys = set of symbol;
               
     chvaltype = ^chvalrec;               { Char consts and FORMAT val }
     
     chvalrec = record
       chpart: alfa;
       next:   chvaltype;
     end;
     
     byte = -128..127;
     
     states = (INITSTATE, HEADSTATE, IMPSTATE, SPECSTATE,
               STMTFUNSTATE, EXECSTATE, ENDSTATE);
     
     levelclass = (COMMONVAR,SAVEDVAR,LOCALVAR,STFUNVAR);
     addrrange = -8388608..8388607;
     blocktype = (UNKWNBLOCK, PROGBLOCK, SUBBLOCK, FUNBLOCK, BLOCKBLOCK);
     
     { Valid types are:       }
     {   INTEGER*1, *2, *4    }
     {   REAL*4, *8 = DOUBLE  }
     {   LOGICAL*1, *2, *4    }
     {   COMPLEX*8            }
     {   CHARACTER*1 to *255  }
     {   CHARACTER*0 = *(*)   }
                          
     ftntype = record typ: types; siz: integer; end;
   
     { Labels and Control Structures }
     
     pstmt = ^stmt;
     
     ctrlblks = (DOBLK, IFBLK, ELSEIFBLK, ELSEBLK);
     
     pctrlstkrec = ^ctrlstkrec;
     
     ctrlstkrec = packed record
       nextblk:    pctrlstkrec;
       ctrlblknum: integer;
       case cbkind: ctrlblks of
         DOBLK:     (finallab: alfa6;
                     doblkstmt: pstmt);
         IFBLK,
         ELSEIFBLK: (blkstmt: pstmt);
       { ELSEBLK:   (); }
     end;
     
     plocofref = ^locofref;
     
     locofref = record
       refctrlstk:     pctrlstkrec;             { Control stk at GOTO     }
       nextref:        plocofref;
     end;
     
     labkinds = (FORMATLAB, EXECLAB, UNKNOWNLAB);
     
     plabelrec = ^labelrec;
     
     labelrec = packed record
       labelval:  alfa6;
       nextlabel: plabelrec;
       case labkind: labkinds of
         FORMATLAB: (FmtDefined: Boolean;
                     fmtstring:  chvaltype;
                     fmtlen:     integer;
                     fmtcstno:   integer);
         UNKNOWNLAB,
         EXECLAB: (LabIsArg: Boolean;
                   ilabno: integer;
                   case Defined: Boolean of
                     TRUE:  (inblk:  integer);    { Control block label is in }
                     FALSE: (labreflist: plocofref))
     end;
     
     plabellist = ^labellist;
     labellist = record next: plabellist;
                    labrec: plabelrec;
                 end;
     
     { Symbol Table Structures: }
     
     idkind = (UNKN, VARS, FUNCT, SUBR, EXTERN, INTRIN, PROG, COMMON,
               EQUIVALENCE, PARAMETER, BLOCKD, IMPDO, LABL);
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
     boundkind = (CONSTBOUND, ADJUSTBOUND, ASSUMEDBOUND);
     pbound = ^bound; pn = ^n;
     brec = record case bkind: boundkind of
                     CONSTBOUND: (i: addrrange);
                     ADJUSTBOUND: (adjboff: addrrange;
                                   adjexpr: pn);
            end;
     bound = record next: pbound;
                    lo,hi: brec;
                    BigIndex,BigElSize: Boolean;
                    esize,sizoff: addrrange;
             end;
     datatype = (NODATA, CHARDATA, NUMDATA);
     pcomref = ^comref;
     comref = record next: pcomref;
                     loc: addrrange;
              end;
     pnlist = ^nlist;
     nlist = record next: pnlist;
                    nrec: pn;
             end;
     valu = record case integer of
                   1: (ival: longint);
                   2: (rval: real);
                   3: (clen: integer; cval: chvaltype);
                   4: (creal: real; cimag: real);
                   5: (dval: double);
            end;
     nodekind = (IDENTNODE,UNNODE,BINNODE,TRINODE,INDEXNODE,CSTNODE);
     n = record
           case node: nodekind of
             IDENTNODE:
                (ExpType: Boolean;
		 isparam: Boolean;
                 SaveFlag: Boolean;
                 Indirect: Boolean;
                 name: identrec;
                 llink,rlink,next: pn;
                 nref: pcomref;
                 idtype: ftntype;
                 refcount: integer;
                 InReg,AddrNeeded: Boolean;
                 case nkind: idkind of
                   UNKN:   (unbounds: pbound);
                   VARS:   (nlev: levelclass;
                            incommon: pn;
                            addr: addrrange;
                            size: addrrange;
                            bounds: pbound);
                   FUNCT,
                   SUBR,
                   EXTERN,
                   PROG,
                   BLOCKD: (NDefined,
                            RtnLocal: Boolean;
                            rtnlev: 1..3;         { 1=MAIN,2=PROC,3=STFUN }
                            rtnno: integer;
                            parambytes: integer;
                            numaltret: integer;   { # of alt returns }
                            arglist: pnlist);
                   INTRIN: (key,ifunno: integer);
                   COMMON: (comellist: pn;        { Head of list of vars }
                            comsize: addrrange;   { Bytes in this COMMON }
                            commonno: integer;    { Used by DUMP }
                            cdatakind: datatype);
                   EQUIVALENCE:
                           (eqdatakind: datatype;
                            eqellist: pn;         { Variables in eq class }
                            eqsize: addrrange;    { Size of equiv. class }
                            EvenBias: Boolean;
                            AddrFlag: Boolean;    { Final addrs assigned? }
                            eqkind: levelclass;   { LOCALVAR or SAVEDVAR }
                            eqaddr: addrrange);   { Base address for block }
                   PARAMETER: (pvalu: valu);
                   IMPDO:  (implev: integer);
                   LABL:   (labp: plabelrec));
             UNNODE:
                (unop: 0..255; unsubop: longint; unarg: pn);
             BINNODE:
                (binop: 0..255; binsubop: integer; leftarg,rightarg: pn);
             TRINODE:
                (triop: 0..255; tri1,tri2: pn;
                   case Boolean of
                     FALSE: (tript: pbound);
                     TRUE:  (tri3: pn));
             INDEXNODE:
                (indexop: 9..11; InxCkBnd: Boolean; inxexpsz: 2..4;
                 inxexpr: pn;
                 case integer {indexop} of
                   9,10: (inxaddr: pn);
                     11: (inxbnds: pbound));
             CSTNODE:
                (csttype: ftntype; cstvalu: valu);
           end;
     
     { Expressions }
     
     attr = record atype: ftntype;
                   treeptr: pn;
            end;
     
     { Data statement structures }
     
     pdatarec = ^datarec;
     datarec = record drnext: pdatarec;
                      drtype: ftntype;
                      drnumb: integer;
                      drvalu: valu;
               end;
     vkinds = (DVARB,DLOOP);
     pvarbrec = ^varbrec;
     varbrec = record vrnext: pvarbrec;
                      case vkind: vkinds of
                        DVARB: (vrvarb: pn;
                                vrindex: pnlist;
                                case SubString: Boolean of
                                  TRUE: (losub,hisub: integer));
                        DLOOP: (loloop,hiloop,steploop: pn;
                                looplist: pvarbrec);
               end;
     pdatael = ^datael;
     datael = record next: pdatael;
                   data: pdatarec;
                   varb: pvarbrec;
            end;
     
     { Equivalence data structures }
   
     pe = ^e; pequivel = ^equivel;
     e = record
           next: pe;
           equivgroup: pequivel;
         end;
     equivel = packed record
                 varname: pn;
                 reladdr: addrrange;
                 dimens: 0..7;
                 inx: array[1..7] of addrrange;
                 next: pequivel;
                 case SubStrFlag: Boolean of
                   TRUE: (lo,hi: integer);
               end;
     
     { Executable statement data structures }
     
     typecodes = (INT1,INT2,INT4,REEL4,REEL8);
     listtags = (IOLEXPTAG,IOLARRTAG,IOLIMPDOTAG);
     
     piolistel = ^iolistel;
     iolistel = record
                nextiolistel: piolistel;
                case listtag: listtags of
                  IOLEXPTAG:   (iolexp: attr);
                  IOLARRTAG:   (iolarr: attr;
                                iolarrcount: pn);
                  IOLIMPDOTAG: (impdoiolist: piolistel;
                                impdotypecode: typecodes;
                                impdovar,impdoinit,impdoterm, impdoinc: pn;
                                impdotemp: addrrange);
                end;
     
     padjarrsz = ^adjarrsz;
     adjarrsz = record next: padjarrsz; arr: pn; addr: longint; end;
     
     fmtknds = (FMTUNFORMATTEDKIND,FMTLABKIND,FMTCHKIND,
                FMTASVARKIND,FMTARRAYKIND,FMTLISTKIND);
     
     stmttypes = (ASMTST,ASSGNST,GOTOST,CGOTOST,AGOTOST,AIFST,IFST,
                  DOST,STOPST,PAUSEST,CALLST,RETRNST,LABEDST,
                  INQREST,CLOSEST,OPENST,REWDST,ENDFST,BACKST,
                  READST,WRITEST,PRINTST,STFUNST,ENTRYST,CONTST);
     
     stmt = record 
              nextstmt: pstmt;
              { linenumber added for dbx by jim t SGI }
              linenumber: integer;
		filename : idstring;
              case stmtop: stmttypes of
                ASMTST:  (asmtop: 0..255; asmtop2: 2..4; asmtvar,asmtexpr: pn);
                ASSGNST: (assignlab: plabelrec; assignvar: pn);
                GOTOST:  (gotolab: plabelrec);
                CGOTOST: (cgotolabs: plabellist; cgotocnt,cgotosiz: integer;
                          cgotoexp: pn);
                AGOTOST: (agotovar: pn;
                          agotolabs: plabellist; agotocnt: integer);
                AIFST:   (aifexpr: pn; aiftypecode: typecodes;
                          neglab,zerolab,poslab: plabelrec);
                IFST:    (ifexpr: pn; thenst,elsest: pstmt);
                DOST:    (dotypecode: typecodes; dotemp: addrrange;
                          dovar, doinit, doterm, doinc: pn; dobody: pstmt);
                STOPST,
                PAUSEST: (sparg: chvaltype; sparglen: integer);
                CALLST:  (procpn,parglist: pn);
                RETRNST: (altret: pn);
                LABEDST: (stlab: plabelrec); {nextstmt points to stmt labeled}
                INQREST: (iqunit: pn; iqfile: pn;
                          iqiostat, iqexist, iqopened, iqnumber, 
                          iqnamed, iqrecl, iqnextrec: pn; 
                          iqerr: plabelrec;
                          iqname, iqaccess, iqsequential, iqdirect, iqform, 
                          iqformatted, iqunformatted, iqblank: pn);
                CLOSEST: (clunit, cliostat: pn; clerr: plabelrec;
                          clstatus: pn);
                OPENST:  (opunit, opiostat, oprecl, opbuffed: pn;
                          opfile, opstatus, opaccess, opform, opblank: pn;
                          operr: plabelrec; OpBinary: Boolean);
                REWDST,
                ENDFST,
                BACKST:  (rebunit, rebiostat: pn; reberr: plabelrec);
                READST,
                WRITEST,
                PRINTST: (* rwunit int4 or ch depending on InternalFlag *)
                         (InternalFlag: Boolean; rwunit: pn;
                          rwrec, rwiostat: pn;
                          rwerr, rwend: plabelrec;
                          rwiolist: piolistel;
                          case fmtkind: fmtknds of
                            FMTLABKIND:   (fmtlab: plabelrec);
                            FMTCHKIND:    (fmtchval: pn);
                            FMTASVARKIND: (fmtvar: pn);
                            FMTARRAYKIND: (fmtarrayvar: pn));
                STFUNST: (funstmt: pstmt; funname: pn);
                ENTRYST: (entryname: pn; enum: integer);
                CONTST:  ();
            end;
     
     regrec = record v: pn; count: integer; end;
     
var gattr: attr;
    
    { fscan interface }
    
    token:   symbol;                    { Symbol scanned             }
    ident:   idstring;                  { IDENTSY scanned            }
    intval:  longint;                   { ICONSTSY or LCONSTSY value }
    realval: real;                      { RCONSTSY scanned value     }
    dblval:  double;                    { DCONSTSY scanned value     }
    chval:   chvaltype;                 { CCONSTSY scanned value     }
    chlen:   integer;                   { Numbr of chars in CCONSTSY }
    
    { Information about statement currently being processed }
    
    errors,warnings: integer;           { Count errors and warnings  }
    AnyErrors,
    ErrorEnable: Boolean;               { ISASSIGN can shut off errs }
    
    listfile:   text;                   { Where to put listing       }
    ListOpen,                           { Is list file open          }
    ListFlag,                           { Is listing currently on    }
    EOFSource,                          { EOF(sourcefile)            }
    StmtbufValid,                       { GETSTMT succeeded          }
    ErrFileOpen,                        { Is error file open         }
    SourceOpen,                         { Is Source file open        }
    CodeOpen: Boolean;                  { Is code file open          }
    icodefile,
    sourcefile: file;                   { FORTRAN input file         }
    errfile: text;
    sfname,                             { Source file name           }
    ifname,                             { i-code file name           }
    cfname: string63;                   { obj-code file name         }
    
    labeledby:   alfa6;                 { Its label, blanks on right }
    LabeledFlag: Boolean;               { Is it labeled              }
    
    chclass: array[0..127] of symbol;   { Maps chars to char classes }
    
    linenumber: longint;                { Normally, number of LALINE }
	extralines:	longint;	{ number of lines #included }
	gotorigfile: boolean;		{ initialized originalfile }
	cppfilename: idstring;		{ File name from cpp #line for dbx,
					  by jim t at SGI }
	ininclfile: boolean;
	incline: longint;
	cpplinenum:	longint;
	originalfile: idstring;		{ the source file name as opposed
					  to #included files }
	lastcppfilename: idstring;	{ So we only dump it 
					when it changes }
    fstlinenum: longint;                { Numb of FST LN in STMTBUF  }
    
    { Information about current statement }
    
    stmtbufp:   integer;             { Points to next character in STMTBUF  }
    stmtbuf: array[1..1386] of byte; { 20 lines and an LALINE     }
    lalinep:    integer;             { Points to start of where LALINE goes }
    
    lineends: array[1..MAXCONTLINES] of integer;         { Pos of LINEENDS in STMTBUF }
    linetail: array[1..MAXCONTLINES] of alfa8;           { Tails of lines in STMTBUF  }
    linefile,                                  { File level for STMTBUF     }
    lineline: array[1..MAXCONTLINES] of longint;         { Line in file for STMTBUF   }
  
    { Information about look ahead line }
    
    LALineDefined: Boolean;        { Only exists if LA is initial line      }
    lalabeledby:   alfa6;          { What is label of lookahead line        }
    LALabFlag:     Boolean;        { Is look ahead line labeled             }
    lalinelen:     integer;        { Number of characters in LALINE         }
    latail:        alfa8;          { Tail of lookahead line, for errors     }
    lalineno:      longint;        { Line in file for lookahead line        }
    lafile:        integer;        { File level for lookahead line          }
    thisline,
    prevline: array[1..120] of byte;{ Line buffers for making clean listing }
    prevlineno:    integer;
    IsPrevLine,                    { True if prevline holds unprinted line  }
    PrevLineCom:   Boolean;        { True if above line is a comment line   }
    
    topoffilestack: 0..MAXFILES;               { Stack top pointer          }
    filestack: array[1..MAXFILES] of           { Stack of INCLUDE files     }
      record
        lastrelblkread: integer;               { Double block position      }
        nextchtoread:   integer;               { Character offset within DB }
        lineinfile:     longint;               { File relative line number  }
        fname:          string63;              { Source file name           }
      end;
    
    inbufp: integer;                           { INBUF[INBUFP] is NEXTCH    }
    inbuf: array[0..1025] of byte;             { Read source file into this }
    curlineinfile: longint;                    { Used for error messages    }
    curfile: string63;                         { Current source file name   }
    errname: idstring;                         { Printed in proc error      }
    
    ZeroStack,
    SWDivinHW,
    Swapped,
    QuietFlag,
    PromptFlag,
    XrefFlag,
    F66DoFlag,
    CharEqu,
    ArgCheck,
    FreeForm,
    DsRComment,
    FPHardware,
    FPHardwarediv,
    DebugFlag,
    OnlyAnsi,
    CCallFlag,
	{ jt - added flag to force all float constnts to double precision }
    DoubPrecConst,
    NoRMW,

    { gb - added flag to tell when we are in a data statement }
    HexOctConstsLegal,

    { dar - added flags to give error on recursion when stack }
    {       space has been exceeded			      }
    RecurseErr,
    OverflowStack,

    Binary,SystemBinary: Boolean;
    stmtnumb: integer;
    nexttypeno: integer;
    defisize,deflsize: integer;         { Default INTEGER and LOGICAL size }
    state: states;
    
    minspace: longint;
    initheap,heapmark: ^Boolean;
    implicit: array['A'..'Z'] of
                    record imptype: ftntype;
                           Defd: Boolean;
                    end;
    { dar - added save of do variable to give warning when changing it }
    dovarsav:pn;

    lastlabel: plabelrec;
    ctrlstk: pctrlstkrec;
    lastctrlblk: integer;       { Last ctrl block entered }
    
    InsideLogIf,AfterDoLab: Boolean;
    oldstmtbufp: integer;
    oldnextaddr: addrrange;
    
    icodeblock,icodebyte: integer;
    icodebuff: array[0..511] of byte;
    lbyteno: integer;
    lbuff: array[0..MAXLBUFF] of byte;
    blockkind: blocktype;
    MainProcFlag: Boolean;
    procname: idstring;
    
    level: levelclass;          { Level of current routine: LOCALVAR/STFUNVAR }
    bcommonsize: addrrange;     { Max size of blank common in words }
    commonlist,                 { List of all commons local to current proc }
    ncomlist: pn;               { Global list of all named commons }
    impdotree,                  { Symbol tree for implide DO's in DATA stmts }
    intrintree,                 { Symbol table for INTRINSIC names }
    globaltree,                 { Symbol table for global names }
    localtree,                  { Symbol table for local names }
    stfuntree: pn;              { Symbol table for Statement Funct Params }
    stfunlist: pstmt;           { List of statement functions }
    equivlist: pe;              { List of source equiv relationships }
    datalist: pdatael;          { List of source data statements }
    nextaddr,                   { Next free byte of STACK storage, counts dn }
    nextsaddr: addrrange;       { Next free byte of SAVED storage, counts up }
    lovarsize,                  { Used by assvaraddr in assigning variable }
    hivarsize: longint;         {   addresses. Assigns iff LO <= size <= HI }
    newglobals: pnlist;         { List of new global symbols, in order }
    proctree: pstmt;            { Root of tree for current routine }
    segname: alfa8;             { Name of current segment }
    GSaveFlag: Boolean;         { Global SAVE flag }
{ OSaveFlag set by +s command line option turns SAVE on for all functions
{ and procedures. For SGI by jim t }
    OSaveFlag: Boolean;		{ Command line Option SAVE flag }
    nextilabno: integer;        { Internal label number generator }
    entrycount,                 { Number of ENTRY statements }
    entrynoloc: integer;        { Offset of entry point indicator }
    extprocno: integer;         { Next local ($000) linker number }
    maxfsize: integer;          { Max function/entry result size }
    maxaltreturn: integer;      { Max number of * formals }
    altrettable: integer;       { Location of alternate return table }
    numcharcalls,               { Number of calls to CHAR }
    chartmploc: integer;        { Location of CHAR temps }
    adjszlst: padjarrsz;        { List of adjustable arrays in IO lists }
    
    exprkind: (NORMEXPR,DATAEXPR,ADJUSTEXPR);
    
    firststmt,newstmt,laststmt: pstmt; {communication for execstmts}
    IOGExitFlag,
    IOGOptionFound,
    IOGFirstTime: Boolean;      { Global flags for I/O stmts }
    PassLabels: Boolean;        { TRUE if labels passed as parameters }
    realtype,int4type,doubletype,cmplxtype,int2type,log1type: ftntype;
    
    intname: array[1..116] of identrec;
    
    a: array[0..ADDRREGS] of regrec;
    d: array[0..DATAREGS] of regrec;
    f: array[0..FLOTREGS] of regrec;
    r: array[0..GENPREGS] of regrec;
    
    {***} TraceCounts: Boolean;
    
procedure varsizes(fpn: pn); forward;

procedure newn(var fpn: pn; fkind: idkind; fname: idstring); forward;

function lookupname(fnametree: pn; fname: idstring): pn; forward;

function lookupid(fnametree: pn; fid: identrec): pn; forward;

procedure entername(fpn: pn; var tree: pn); forward;

function declarevar(var fnametree: pn; fname: idstring): pn; forward;

procedure unkntovars(fpn: pn); forward;

procedure declunkn(fpn: pn); forward;

procedure makeglobal(fpn: pn); forward;

procedure needstate(minstate,maxstate: states); forward;

function findlabel(flabel: alfa6; flabkind: labkinds): plabelrec; forward;

procedure formatstmt; forward;

procedure xref(fpn: pn); forward;

procedure dumpntree(fpn: pn); forward;

function IsVariable(fpn: pn): Boolean; forward;

function IsAddress(fpn: pn): Boolean; forward;

procedure out(a: integer); forward;

procedure out2(a: integer); forward;

procedure out3(fval: addrrange); forward;

procedure out4(fval: longint); forward;

procedure outname6(fname: alfa6); forward;

procedure outname8(fname: alfa8); forward;

procedure error(errnum: integer); forward;

procedure warning(errnum: integer); forward;

procedure notansi; forward;

procedure getstmt; forward;

function getchar: char; forward;

function getnonblank: char; forward;

function Match(fst: string16): Boolean; forward;

function IsHollerith: Boolean; forward;

procedure fscanhollerith; forward;

procedure fscan; forward;

procedure getlabel(var flabel: alfa6); forward;

function IsAssign: Boolean; forward;

procedure fillinbuf; forward;

procedure cstexpression; forward;

function Numeric(ftyp: types): Boolean; forward;

procedure cmakentype(var fattr: attr; totype: ftntype); forward;

procedure cmakesnum(var fattr: attr; totype: ftntype); forward;

procedure cmakeslen(var fattr: attr; tosize: integer); forward;

procedure fold(var fexpr: pn); forward;

procedure fcall(fpn: pn; callkind: idkind); forward;

procedure expression; forward;
 
procedure variable(fpn: pn); forward;

function newbinnode(fbinop: integer; fleftarg,frightarg: pn): pn; forward;

function newunnode(funop: integer; funarg: pn): pn; forward;

procedure makeisize(var fattr: attr; tosize: integer); forward;

procedure maxlsize(var aattr,battr: attr); forward;

procedure minsize2(var fattr: attr); forward;

procedure makentype(var fattr: attr; totype: ftntype); forward;

procedure maxnumtypes(var aattr,battr: attr); forward;

procedure entrystmt; forward;

procedure skip(fsys: setofsys); forward;

procedure ckstrparm(var fattr: attr); forward;

function IsSubStr: Boolean; forward;

function jumptolabel(flabel: alfa6): plabelrec; forward;
    
procedure checkgotovalid(ctrlstk: pctrlstkrec; labinblk: integer); forward;
    
procedure datacode(fdatalist: pdatarec; fvarblist: pvarbrec;
                   DumpFlag: Boolean); forward;

procedure flushlbuff; forward;

procedure lout(fval: integer); forward;

procedure lout2(fval: integer); forward;

procedure needaddr(fpn: pn); forward;

procedure printprevline; forward;

function onadjlst(fpn: pn): padjarrsz; forward;

procedure %a_2_d(var a: alfa31; digits,exp: integer; var d: double); external;
function %i_up_i(a,b: longint): longint; external;
function %_up_i(base: real; pow: longint): real; external;
function %_dupi(var db: double; var ie: longint): double; external;
function %_cupi(var c: double; var n: longint): double; external; {--> complex}

{$S DUMP} {$I f.dump.1 }
          {$I f.dump.2 }
{$S INIT} {$I f.init }
{$S SPEC} {$I f.spec }
{$S EXEC} {$I f.exec.1 }
          {$I f.exec.2 }
          {$I f.exec.3 }
          {$I f.exec.4 }
{$S MAIN} {$I f.call }
          {$I f.expr }
          {$I f.misc }
          {$I f.opt }
          {$I f.state }
{$S     } {$I f.symb }
          {$I f.scan.1 }
          {$I f.scan.2 }
          {$I f.assign }


procedure finalize;
  var largv: array[1..2] of ^string63; coderesult: integer;
  
  procedure finalmessage(var f: text);
  begin
  write(f,errors,' errors.  ');
  if warnings > 0 then write(f,warnings,' warnings.  ');
  writeln(f,linenumber,' lines.  File ',sfname);
  if (HOSTOS = MERLIN) or
     (HOSTOS = IDRIS) or
     (HOSTOS = ADVENTURE) or
     (HOSTOS = CPM)
  then writeln(f,'Smallest available space: ',minspace,' bytes.');
  end; {finalmessage}
  
begin {finalize}
if ListOpen and ListFlag
then dumpntree(globaltree)
else
  if not QuietFlag then writeln;
out(255{ENDICODE});
coderesult := 0;
if CodeOpen
then begin
     if (icodebyte > 0) and (errors = 0)
     then if blockwrite(icodefile,icodebuff,1,icodeblock) <> 1
          then error(400);
     if errors = 0
     then begin
          close(icodefile,LOCK);
          if HOSTOS = MERLIN
          then begin
               release(initheap);
               largv[1] := @ifname; largv[2] := @cfname;
               if CHIP = M68020
               then begin
                    coderesult := call('J.CODE',input,output,largv,2);
                    if coderesult <> 0
                    then writeln('*** Can''t call J.CODE ***');
                    end
               else if CHIP = M68000
                    then begin
                         coderesult := call('H3.CODE',input,output,largv,2);
                         if coderesult <> 0
                         then writeln('*** Can''t call H3.CODE ***');
                         end
                    else begin
                         coderesult := call('N3.CODE',input,output,largv,2);
                         if coderesult <> 0
                         then writeln('*** Can''t call N3.CODE ***');
                         end;
               end;
          end
     else close(icodefile,PURGE);
     end;
if ListOpen
then begin
     writeln(listfile);
     finalmessage(listfile);
     page(listfile);
     close(listfile,LOCK);
     end;
if ErrFileOpen
then begin
     writeln(errfile);
     finalmessage(errfile);
     close(errfile,LOCK);
     end;
if not QuietFlag then writeln;
finalmessage(output);
if HOSTOS = ELITE
then if SourceOpen then close(sourcefile);
if (errors > 0) or (coderesult <> 0) then halt(ERRHALT);
end; {finalize}

{$S MAIN}
procedure mainloop;
begin
while StmtbufValid do begin 
  needstate(HEADSTATE,HEADSTATE);
  declstmts;
  execstmts;
  needstate(ENDSTATE,ENDSTATE);
  state := INITSTATE;
  getstmt;
  end;
end; {mainloop}

{$S EXEC}
procedure loadexec;
begin
mainloop;
end; {loadexec}

{$S SPEC}
procedure loadspec;
begin
loadexec;
end; {loadexec}

{$S DUMP}
procedure loaddump;
begin
loadspec;
end; {loadexec}


begin {fortran77}
initialize;
loaddump;
if state <> INITSTATE then error(91);
999: finalize;
end. {fortran77}
