#ifndef lint
static char Notice[] = "Copyright (c) 1985 Adobe Systems Incorporated";
static char *RCSID="$Header: /d2/3.7/src/usr.bin/print/trscript/src/RCS/ps630.c,v 1.1 89/03/27 18:20:43 root Exp $";
#endif
/* ps630.c
 *
 * Copyright (c) 1985 Adobe Systems Incorporated
 *
 * Diablo 630 to PostScript translator
 *
 * Diablo is a Xerox Company
 * PostScript is a trademark of Adobe Systems Incorporated
 * NOTICE:  All information contained herein or attendant hereto is, and
 * remains, the property of Adobe Systems, Inc.  Many of the intellectual
 * and technical concepts contained herein are proprietary to Adobe Systems,
 * Inc. and may be covered by U.S. and Foreign Patents or Patents Pending or
 * are protected as trade secrets.  Any dissemination of this information or
 * reproduction of this material are strictly forbidden unless prior written
 * permission is obtained from Adobe Systems, Inc.
 *
 * Ann Robinson: Original version
 * Edit History:
 * Andrew Shore: Mon Nov 18 17:02:34 1985
 * End Edit History.
 *
 * This includes all the Diablo functions except:
 * backwards printing (it is meaningless)
 * HI-PLOT, the extended character set, communications, download of fonts
 *    (printwheel information), the ability to specify which spoke to
 *   print rather than a letter, and some hardware reset commands
 *
 * Tue Oct 29 14:55:42 1985
 * This program does not produce page-independent PostScript and I'm
 * not interested in it enough to fix it.  This code is horribly
 * complex, mostly because it started it's life as something that could
 * both emulate (as in the emulator built into the LW) and translate.
 * As a "fix", I have omitted the true magic number (%!PS-Adobe-1.0)
 * and the number supplied is now just "%!".  This means that
 * the spoolers won't try and page-reverse it, thus avoiding the
 * problem.  Sorry.	--AIS
 * 
 * RCSLOG:
 * $Log:	ps630.c,v $
 * Revision 1.1  89/03/27  18:20:43  root
 * Initial check-in for 3.7
 * 
 * Revision 1.1  86/11/17  20:21:59  root
 * Initial revision
 * 
 * Revision 2.1  85/11/24  11:49:36  shore
 * Product Release 2.0
 * 
 * Revision 1.3  85/11/20  00:18:26  shore
 * support for System V
 * new options parsing (getopt!)
 * fixed -s bug
 * char/int int/long distinctions handled better
 * 
 * Revision 1.2  85/05/14  11:22:56  shore
 * *** empty log message ***
 * 
 *
 */

#include "transcript.h"
/* key definitions */
#define CTRLB 2
#define CTRLC 3
#define CTRLF 6
#define CTRLG 7
#define CTRLH 8
#define CTRLI 9
#define CTRLJ 10
#define CTRLK 11
#define CTRLL 12
#define CTRLM 13
#define CTRLO 15
#define CTRLQ 17
#define CTRLR 18
#define CTRLS 19
#define CTRLT 20
#define CTRLX 24
#define CTRLY 25

#define BS 8
#define CR 13
#define DC1 17
#define ESC 27
#define EM 25
#define ETB 23
#define FFEED 12
#define HT 9
#define LF 10
#define RS 30
#define SO 14
#define US 31
#define VT 11

#include "types.h"
#include <stdio.h>
#include <pwd.h>
#ifdef SYSV
extern struct passwd *getpwuid();
#endif

#define true 1
#define false 0
#define outstrmax 257                /* size of output buffer +1 */
#define papertop 530                /* top of paper in VUs */
#define paperright 1020             /* right edge of paper in VUs */
#define paperleft 18                /* first print column on paper */
#define paperbot 0                  /* bottom of paper ?? 0 or 1 ?? */
#define vmisave 8                   /* standard VMI */
#define totalhtabs 159              /* total number of horizontal tabs-1 */
				    /* in PROLOG too! */
#define normal 1                    /* normal font */
#define bold 2                      /* bold font */
#define shadow 4                    /* shadow font */
#define uline 8                     /* underline font */
#define subscript 16                /* effect of neg half linefeed */
#define superscript 32              /* effect of half linefeed */

extern int atoi();
	
private char outchars[outstrmax];
private int vtabs[totalhtabs+1], totalpages;
private char *regularname = "Courier";
private char *boldname = "Courier-Bold";
private char *Infile = NULL;

private int SeenFile = 0;
 

private cardinal outstrlen, outchleng;	/* number of chars in outchars */

private integer margintop, marginbot,	/* top and bottom margins (points) */
      numlines,                       /* number of lines on page */
      hmi,vmi,vmi2,                   /* horizontal and vertical increments */
      hmisave,                        /* original ones */
      currline,                       /* current print line (starts at 1) */
      paperloc,                      /* current loc on paper-in diablo units*/
      offset;                         /* modification to proportnl spacing */
private boolean autolf,			/* true if auto linefeed after cr */
      autocenter,                     /* true of auto centering is on */
      autojustify,                    /* true if autojustify is on */
      reverseprint,                   /* true if line is to be reversed */
      graphics;                       /* true when in graphics mode */

private cardinal printmode,		/* one of normal bold shadow uline */
         fontpitch;                   /* current pitch */

private boolean stringonstack, dirtypage;

private cardinal nspaces;

#define mkHU(n) ((real) ((n)*0.6))
#define mkVU(n) ((real) ((n)*1.5))
#define rdchar() ((unsigned char) getc(stdin))

private char *Outfile;
private char *prog;
extern char *optarg;
extern int optind;

/* Units will be diablo units.   In the diablo, the origin is the top left
   corner.  This uses the Postscript coordinate system--so the very top 
   is 792 pts or 528 diablo vertical units.  The top printline is 
   margintop - vmi (distance between lines)                                 */

/* Fontpitch and autolinefeed are set thru the hardware of the printer.     */
/* There will be some procedure on the host to replace this, and the values */
/* will somehow be sent to here.  At present, fontpitch is typed as input   */
/* and this currently assumes the hardware setting of NO AUTO LINEFEED.     */

/** this assumes 11" paper only */

main(argc,argv)
int argc;
char **argv; 
{
 register int argp;

 prog = *argv;
 vmi = vmisave;                    /* default -- reset with esc sequence */
 stringonstack = false;
 dirtypage = false;

 /* Top and bottom margins are specified in lines from the edge of the     */
 /* paper.  (assuming 11 inch). Left and right margins are spaces from   */
 /* left edge of paper. Acceptable pitches are 10, 12, 15 and 0 (variable) */


/* arguments are pitch, font, BFT, infilehandle, outfile handle */
  totalpages = 0;

#define ARGS "f:F:s:p:"

 autolf = false;
 fontpitch = 12;

 while ((argp = getopt(argc, argv, ARGS)) != EOF) {
     switch (argp) {
	case 'f':
	    regularname = optarg;
	    break;
	case 'F':
	    boldname = optarg;
	    break;
	case 's':
	    fontpitch = atoi(optarg);
	    if ((fontpitch != 12) && (fontpitch != 15) && (fontpitch != 10)) {
	        fprintf(stderr,"%s: %d not a valid pitch, 12 will be used\n",
			prog, fontpitch);
		fontpitch = 12;
	    }
	    break;
	case 'p':
	    Outfile = optarg;
	    if (freopen(Outfile,"w",stdout) == NULL) {
		fprintf(stderr,"%s: can't open output file %s\n",prog,Outfile);
		exit(2);
	    }
	    break;
	case '?':
	default:
	    fprintf(stderr,"%s: unknown option -%c\n",prog,argp);
	    exit(2);
     }
 }
 for (; optind < argc ; optind++) {
     Infile = argv[optind];
     if (freopen(Infile, "r", stdin) == NULL) {
	 fprintf(stderr, "%s: can't open %s\n", prog, Infile);
	 exit(1);
     }
     CopyFile();
     VOIDC fclose(stdin);
 }
 if (!SeenFile) {
     CopyFile();
 }
 if (dirtypage)  newsheet(papertop);
  printf("\n%%%%Trailer\n");
  printf("%%%%Pages: %d\n",totalpages);
  VOIDC fclose(stdout);
  VOIDC fclose(stdin);
  exit(0);
    
}

private CopyFile()
{
 register int inchar;

 if (!SeenFile) {
   PSheading();
   if (fontpitch == 12) hmi = 10;
   else if (fontpitch == 15) hmi = 8;
   else hmi = 12;
   hmisave = hmi;
   heading();
   printf("/PSHMI %g def\n",mkHU(hmisave));
   setupfonts();
  }
  printf("%%%%EndProlog\n");
  if (dirtypage == false)
     {dirtypage = true;
      totalpages++;
      printf("%%%%Page: %d %d\n",totalpages,totalpages);
     }
   printf("NFT setfont\n");

   outchars[0] = '\0';
   printmode = normal;
   margintop =  currline = 8;
   marginbot =  numlines = papertop;    /* number of lines * vmi */
   autojustify =  autocenter = false;
   paperloc = papertop - vmi;
   offset = nspaces = 0;
   outchleng = outstrlen = 0;
   reverseprint = graphics = false;

  SeenFile++;

 /* main loop to interpret commands */

 while ((inchar = getc(stdin)) != EOF)  
   {if (inchar > 31) charout(inchar);      /* ascii char to print */
    else if (inchar == ESC)                /* do escape sequence */
     {switch (inchar = rdchar())
       {case '1': {sethtab(true); continue;}        /* set horiz tab */
        case '2': {clearalltabs(); continue;}           /* clear tabs */
        case '3': {showstring(); graphics = true; continue;}       /* turn on graphics mode */
        case '4': {graphics = false; continue;}   /* disable graphics  mode */
        case '5': {showstring(); reverseprint = false; continue;}
                                                  /* enable forward prt */
        case '6': {showstring(); reverseprint = true; continue;}
                                                  /* enable reverse prt */
/*      case '7': {onmode(suppress); continue;}   /* enable print suppressn */
        case '8': {sethtab(false); continue;}          /* clear horiz tab */
        case '9': {lmarset(); continue;}                /* set left margin */

        case '0': {rmarset(); continue;}                /* set right margn */
       }
      switch (inchar)
       {case 'C': {cleartbmar(); continue;}       /* clears top & bot mars */
        case 'D': {neghalflf(); continue;}     /* negative half linefeed */
        case 'E': {onmode(uline); continue;}       /* enable auto underscore */
       }
      switch (inchar)
       {case 'L': {marginbot = currline;  continue;}      /* set bot margin */
        case 'M': {showstring(); autojustify = true; continue;}
              /* turns on auto justify */
        case 'O': {onmode(bold); printmode &=  ~shadow; continue;} 
                           /* enable bold print (which tuns off shadow) */
/*      case 'P': { continue;}          /* enable proport spacing */
/*      case 'Q': { continue;}          /* disable proporl spacng */
        case 'R': {offmode(uline); continue;}       /* disabl auto underscore */
/*      case 'S': { continue;}          /* return hmi to pwheel */
        case 'T': {margintop = currline; continue;}    /* set top margin */
        case 'U': {halflf(); continue;}        /* half line feed */
        case 'W': {onmode(shadow); continue;}           /* enable shadow print */
        case 'X': {clearwdproc(); continue;}        /* cancel ALL wd processg */
       }
 /* Y, Z, a-h  all for print printwheel code */
      if (inchar == '&') offbold();          /* disable bold/shadow  */
      else if (inchar == '=') autocenter = true;      /* enable auto center */
      else if (inchar == '-') setvtab();               /* set vert tab */
/*case '/': {continue;}                        /* enable auto bkward prt */
/*case '\\': {continue;}                       /* disabl auto bkward prt */
/*case '<': { continue;}          /* enable reverse prt */
/*case '>': { continue;}          /* disable reverse prt */
      else switch (inchar)
         {case BS: {backspace(true); continue;}       /* backspace 1/120" */
          case CR: {remotereset(); continue;}             /* remote reset */
          case DC1: {setoffset(rdchar()); continue;}/* set offset to n */
          case EM: {rdchar(); continue;}                  /* sheet feeder */
          case ETB: {rdchar(); continue;}/* single/mul strk ribbon */
          case FFEED: {setnlines(); continue;}/* set lines / pg to n */
          case HT:  {htabto(rdchar()); continue;} /* absolute HT to col n */
          case LF:  {neglf(vmi); continue;}         /* neg line feed */
          case RS:  {vmi =rdchar()-1; setvmi(); continue;}/* set VMI to n-1 */
/*        case SO:  {continue;}       /* enabl pgm mode or dnld */
          case US:  {showstring(); hmi = rdchar()-1; 
                                   continue;}    /* set HMI */
          case VT:  {vtabto(rdchar()); continue;} /* absolute Vtab to n */
	 }
      }
    else {switch (inchar)
           {
/*	    case CTRLB: {continue;}	  /* remote diagnostic */
/*          case CTRLC: { continue;}      /* communication */
/*          case CTRLF: { continue;}      /* communication */
/*          case CTRLG: { continue;}      /* UNCCRAR */
            case CTRLH: {backspace(false); continue;}          /* backspace */
            case CTRLI: {htab(); continue;}               /* horizontal tab */
            case CTRLJ: {linefeed(vmi); continue;}           /* linefeed */
            case CTRLK: {vtab(); continue;}      /* vertical tab */
            case CTRLL: {formfeed(); continue;}  /* formfeed */
            case CTRLM: {carret(); continue;}    /* carriage return */
/*          case CTRLO: {continue;}      /* clear progrm mode */
/*          case CTRLQ: {continue;}      /* communication */
/*          case CTRLR: { continue;}      /* exit downld mode */
/*          case CTRLS: { continue;}      /* communication */
/*          case CTRLT: {continue;}      /* exit pwheel dwnld */
/*          case CTRLX: {continue;}	/* hammer energy */
/*          case CTRLY: {continue;}	/* paper feeder */
	   }
        }
     }

}


/***  carret =  on carriage return, moveto left margin, 
   if autolf then do linefeed  too                     */
private carret()
{showstring();
 if (autolf) linefeed(vmi);
 leftedge(paperloc);
 graphics = reverseprint = false;   /* these always get reset with cr */
 /*  offmode(suppress); */
 offset = 0;
 offbold();    

}

/***   formfeed = on formfeed  shows current string (which gets reset), 
   shows page and moves to the top left corner of the new page   */
private formfeed()
{showstring();
 paperloc -= (numlines-currline + margintop);
 if (paperloc > paperbot)    /* stay on same sheet--page has been */
      {movetoy(paperloc);   /* defined to be different from sheet */
      }
 else {newsheet(paperloc += papertop);
                 /* go to new paper sheet and adjust for any offset */
      }
 currline = margintop;
}

/*** backspace = move back one space (distance determined by HMI
  shows current string and then moves left -- so it will overprint any 
  chars on the line.  If unit1 = true move back 1/120"  */
private backspace(unit1)
boolean unit1;
{real uni;
 showstring();
 uni = -mkHU(unit1 ? 1 : ((graphics == false) ? hmi : 2));
 if (reverseprint) uni = -uni;
 printf("%g R\n",uni);
}

/***  charout = adds character at end of string to go out --               */
private charout(inchar)
char inchar;
{if ((outstrlen + outchleng) == outstrmax) return(false);
 if (dirtypage == false)
     {dirtypage = true;
      totalpages++;
      printf("%%%%Page: %d %d\n",totalpages,totalpages);
     }
 if (graphics)
      {if (inchar == ' ') 
            {printf("%g R\n",mkHU(2));
	    }
       else {printf("(\\%03o)GSH\n",inchar);
             outchleng = outstrlen = 0;
            }
      }
  else {if (inchar == ' ') nspaces++;
        if ((inchar == '\\') || (inchar == '\(') || (inchar == '\)')) {
	    if (reverseprint == TRUE) {
		outchars[outchleng++] = inchar;
		outchars[outchleng++] = '\\';
	    }
	    else {
		outchars[outchleng++] = '\\';
		outchars[outchleng++] = inchar;
	    }

	}
	else outchars[outchleng++] = inchar;
        outchars[outchleng] = '\0';
        if (((autocenter) || (autojustify)) && (reverseprint == false)) {
	    if (stringonstack == false) {
		stringonstack = true;
		printf("mark\n");
            }
	}
     }
}

/* newmode - change in mode */
private newmode()
{if (((autocenter) || (autojustify))&&(reverseprint == false))
  {if (outchleng > 0) 
     {printf("%d(%s)\n",printmode,outchars);
       outchleng = 0;
     }
  }
 else showstring();
}

/*** showstring = puts out string and clears it                   */
private showstring()
{int nspacesave;
 if (outchleng > 0) 
   {printf("%d",printmode);
    if (autojustify)
        /* get rid of any trailing spaces if justified -- unless all spaces */
          {nspacesave = nspaces;
           while (outchars[outchleng-1] == ' ')
              {outchleng--; nspaces--;
              } 
           if (nspaces == 0) 
              {nspaces = nspacesave;
               outchleng += nspaces;
	      }
           else outchars[outchleng] = '\0';
	  }
    if (reverseprint) dreverse();
    printf("(%s)",outchars);
    printmode &=  ~superscript & ~subscript;
                        /* dont do another sub/superscript until told */
   }
 if (((autocenter) || (autojustify))&&(reverseprint == false))
   {if (stringonstack)
      {printf("SJA ");
       stringonstack = false;
       if (autocenter) center();
       else justify();
      }
    }
  else if (outchleng > 0)
         {if (reverseprint) 
            {printf("%g RMV\n",mkHU(offset+hmi-hmisave));
	    }
     showit(offset + hmi - hmisave);
     if (reverseprint) printf("RMVBK\n");
    }
  outchars[0] = '\0';
  outchleng = outstrlen = 0;
  nspaces = 0;
}

/*** showit()  show of str in mode with width adjustments                 */
private showit(incr)
integer incr;
{if (incr == 0) printf("S ");
 else {printf("%g AS\n",mkHU(incr));}
}

/*** center()  autocenter                                              */
/*   NOTE That this is supposed to ignore right and left margins           */
private center()
{printf("%g AC\n",mkHU(offset+hmi-hmisave));
 autocenter = false;
}

/*** justify()   justify a line of text     */
private justify()
{
    printf("%d %g JU\n",(nspaces == 0?1:nspaces),mkHU(offset+hmi-hmisave));
}

/*** movetoy = moves to new y position -- x is kept in postscript, y is here */
private movetoy(y)
integer y;
{
    printf("%ld LF\n",y);
}

/*** leftedge(y) = moves to left margin on line y            */
private leftedge(n)
integer n;
{
    printf("%ld CR\n",n);
}

/*** moveto = moves to new xy position         */
private moveto(x,y)
integer x,y;
{
    printf("%g %g M\n",mkHU(x),mkVU(y));
}

/*** setoffset = sets offset for changing spacing between chars with
 proportional spacing. Offset is 0-63, bit 6 is sign.  On = negative offset */
private setoffset(n)
cardinal n;
{showstring();
 if ((n & 64) == 0) offset = n;
 else offset = -(n & 63);
}

/* heading for translator files */
private PSheading()
{long clock;
 struct passwd *pswd;
 char hostname[40];
 printf("%%!\n");
 printf("%%%%DocumentFonts: %s %s\n",regularname, boldname);
 pswd = getpwuid(getuid());
 VOIDC gethostname(hostname,sizeof hostname);
 printf("%%%%Creator: %s:%s (%s)\n",hostname,pswd->pw_name,
       pswd->pw_gecos);
 printf("%%%%Title: %s\n",((*Infile != '\0')? Infile:"stdin"));
 printf("%%%%CreationDate: %s",(time(&clock), ctime(&clock)));
 printf("%%%%Pages: (atend)\n");
 printf("%%%%EndComments\n");
}
/*  setupfonts - finds fonts and defines them       */
private setupfonts()
{
    printf("/NFT /%s findfont %ld scalefont def\n",regularname,hmi);
    printf("/BFT /%s findfont %ld scalefont def\n",boldname,hmi);
}

/*** heading = puts out heading information.  Once per document            */
private heading()
{char prolog[256];
 char *libdir;
 /* insert fixed prolog */
 *prolog = 0;
 if ((libdir = envget("PSLIBDIR")) == NULL) libdir = LibDir;
 if (copyfile(mstrcat(prolog,libdir,PS630PRO,sizeof prolog),stdout)) {
     fprintf(stderr,"%s: can't copy prolog file %s\n",prog,prolog);
     exit(1);
 }
 clearalltabs();
 printf("MRESET PGI\n");
 setvmi();
 leftedge(paperloc);
}

/*  margin reset   Emulator only                                            */

/* clearwdprocessing = turn off all word processing            */
private clearwdproc()
{offmode(uline);
 showstring();
 autojustify = false;            /* turn off auto justify */
 autocenter = false;
}

/** linefeed = action on line feed                                          */
/*  show current string and reset. move down 1 line distance is VMI         */
private linefeed(incr)
integer incr;
{showstring();
 if (graphics) incr = 1;
 currline += incr;
 if (currline > marginbot)
      {currline =  margintop + incr - vmisave;
       paperloc -= (incr - vmisave + numlines - marginbot + margintop);
      }
          /*  this  splits vmi units across a page */
 else paperloc -= incr;
 if (paperloc >=  paperbot) movetoy(paperloc);
 else newsheet(paperloc += papertop);
}

/** halflf = action on one-half line feed                                   */
/*  show current string and reset. move down 1/2 line.  distance is VMI/2   */
private halflf()
{if (autojustify == false) linefeed(vmi2);
  else {paperloc -= vmi2;
        currline += vmi2;
        onmode(subscript);
        if (printmode & superscript)  /* turn off superscript */
                   printmode &= ~superscript;
       }
}

/** neghalflf = action on negative one-half-line line feed                  */
/*  show current string and reset. move up 1/2 line.  distance is VMI/2     */
private neghalflf()
{if (autojustify == false) neglf(vmi2);
 else {onmode (superscript);
       currline -= vmi2;      
       paperloc += vmi2;
       if (printmode & subscript)   /* turns off subscripting */
                 printmode &= ~subscript;
      }
}

/** neglf = action on negative line feed  (move up 1 line)                  */
/*  show current string and reset. move up 1 line. distance is VMI         */
private neglf(incr)
integer incr;
{showstring();
 if (graphics) incr =1;
 if (paperloc < papertop)           /* can go above margin, but not */
                                          /* top of paper */
      {currline -= incr;
      movetoy(paperloc += incr);
      }
}

/** setnlines = sets number of lines on page */
private setnlines()
{cardinal n;
 n = rdchar();
 if ((n >= 0) && (n <= 126)) marginbot = numlines = n*vmisave;
 currline = margintop;
}

/** cleartbmar = clears top and bottom margins                              */
private cleartbmar()
{margintop = vmi;
 marginbot = numlines;
}

/*  htabto(n)  =   absolute horizontal tab to n                            */
private htabto(n)
integer n;
{showstring();
 n = (n-1)*hmi + (paperleft/0.6);              /* convert from chars to HUs */
 if (n >= 0) moveto(n,paperloc);
}

/*  vtabto(n)   =  absolute vertical tab to n                              */
private vtabto(n)
integer n;
{showstring();
 if ((n >= 0) && (n <= numlines/vmi))
   {paperloc += currline-n*vmi;
    if (paperloc > paperbot)  /* stay on same sheet--page has been */
      {movetoy(paperloc);   
      }
    else {newsheet(paperloc += papertop);
           /* go to new paper sheet and adjust for any offset */
         }
    currline = n*vmi;
    }
}

/*  setvtab = sets tab at location n for vertical tabs             */
private setvtab()
{
 register i;
 int loc;
 if (vtabs[totalhtabs] == 999999)        /* room for another one */
    {loc = findtab(currline);
     if (vtabs[loc] != currline)
          {for (i = totalhtabs; i > loc; i--) vtabs[i-1] = vtabs[i];
           vtabs[loc] = currline;
	  }
    }
}

private int findtab(ypos)
int ypos;
{int i;
 for (i = 0; i <= totalhtabs; i++)
   if (vtabs[i] > ypos) return(i);
 return(totalhtabs+1);
}

/*  sethtab  = sets or clears horizontal  tab at current location       */
private sethtab(setit)
boolean setit;
{
 showstring();
 printf(" %cTAB\n",setit?'S':'C');
}

/* htab() = horizontal tab to next tab set - noop if none set     */
private htab()
{
 showstring();
 printf("DOTAB\n");
}

/* vtab() = vertical tab to next tab set - noop if none set       */
private vtab()
{
 int loc;
 loc = findtab(currline + 1);
 if ((loc = vtabs[loc]) != 999999)
      {paperloc += (currline - loc);
       currline = loc;
       if (paperloc < paperbot) newsheet(paperloc += papertop);
       else movetoy(paperloc);
      }
}

/* clearalltabs = clears horiz and vertical tabs                            */
/**  NOTE: there is a discrepancy in the 620 interface manual -- page 3-9   */
/* says clear horizontal tabs,  page 3-23 says clear horiz and vertical     */
/* the latter (pg 3-23) is assumed here                                     */
private clearalltabs()
{
 int i;
 for (i = 0; i <= totalhtabs; i++) vtabs[i] = 999999;
 printf(" CTABALL\n");
}

/**** lmarset = set left margin at current loc */
private lmarset() 
{
 showstring();
 printf(" MLS\n");
}

/* rmarset = sets right margin at RIGHT SIDE OF curr loc */
private rmarset() 
{
 showstring();
 printf(" %g MRS\n",mkHU(hmi));
}

/* Offbold = turns off bold and shadow font */
private offbold()
{newmode();
 printmode &= (~bold)&(~shadow);
}

/***** onmode = turn on a printing mode */
private onmode(modebit)
character modebit;
{newmode();
 printmode |= modebit;
}

/*********** offmode = turns off printing mode */
private offmode(modebit)
character modebit;
{int i;
 if ((modebit & uline) != 0) 
     {i = 0; 
      while (outchars[outchleng-1] == ' ')
           {outchleng--;
            nspaces--;
            outchars[outchleng] = '\0';
            i++;
           }
     }
 newmode();
 printmode &= ~modebit;
 if (i > 0)
      while (i > 0) {charout(' '); i--;}
}

/* remotereset = resets margins and tabs                                   */
private remotereset()
{if (rdchar() == 'P')          /* 3 char command, check for 3rd char before */
    {cleartbmar();             /* doing it.  If not P, ignore it            */
     clearalltabs();
     numlines = 66;
     printf("MRESET\n");
     hmi = hmisave;
     vmi = vmisave;
     setvmi();
     offset = 0;
    }
}

/** set VMI -- pass to  PS      */
private setvmi() 
{showstring();
 printf("/PSVMI %ld def /PSVMI2 %ld def\n",vmi,vmi/2);
 vmi2 = vmi/2;
}

/* newsheet  go to new page and initialize it  */
private newsheet(ypos)
int ypos;
{
 dirtypage = false;
 printf(" PG %g M\n",mkVU(ypos));
}


/***** dreverse reverses a string in place */
private dreverse()
{register i;
 int x,lim,leng;
 i = 0;
 lim = (outchleng-1)/2;
 leng = outchleng-1;
 while (i <= lim)
     {x = outchars[i];
      outchars[i] = outchars[leng-i];
      outchars[leng-i] = x;
      i++;
     }
}

