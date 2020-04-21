% included prolog for ps630 - diablo translator
/maxtabs 159 def
/htabs maxtabs 1 add array def
/JA 514 array def
/MRESET	{/MR 1020 def /ML 18 def}def% paperright paperleft
/MRS {currentpoint pop add /MR exch def}def
/MLS {/ML currentpoint pop def}def
/NM 1 def
/BM 2 def
/SM 4 def
/UM 8 def
/SBM 16 def
/SPM 32 def
/CR{VU ML exch moveto}def
/LF{VU currentpoint pop exch moveto}def
/M/moveto load def
/R {0 rmoveto} def
/PGI{.2 setlinewidth}def
/PG{currentpoint pop showpage PGI}def
/HU{.6 mul}def
/VU{1.5 mul}def
/SP{gsave exec grestore 1 0 rmoveto exec -1 0 rmoveto}def
/UP{currentpoint /y exch def /x exch def
    grestore gsave currentpoint 1.4 sub newpath moveto x y 1.4
    sub lineto stroke grestore x  y moveto} def
/SETM{dup NM and 0 ne {NFT setfont}if
      BM and 0 ne {BFT setfont}if
     }def
/GETW{/W 0 def 0 2 SARR length 1 sub dup
      /nelts exch def 
	 {dup SARR exch get SETM 1 add SARR exch get incr 0 ne 
	     {dup length incr mul W add /W exch def}if
	  stringwidth pop W add /W exch def}for W}def
/JUMS{dup SARR exch get dup /mode exch def SETM
       mode SBM and 0 ne{0 PSVMI2 neg rmoveto}if
       mode SPM and 0 ne{0 PSVMI2 rmoveto}if
       mode UM and 0 ne dup{gsave}if exch}def
/JU{/incr exch def /nspaces exch def
    GETW dup MR ML sub .58 mul ge
    {neg MR add currentpoint pop sub nspaces div /incsp exch def}
    {pop /incsp 0 def}ifelse
    0 2 nelts 
       {JUMS 1 add incsp exch 0 exch 32 exch incr exch 0 exch 
	SARR exch get mode SM and 0 ne
	{{awidthshow} 7 copy SP}{awidthshow}ifelse{UP} if} for
   }def
/S{/incr 0 def CTEST mode SM and 0 ne{{show} 2 copy SP}{show}ifelse{UP}if}def
/GSH {gsave show grestore}def
/CTEST{/str exch def dup /mode exch def
       UM and 0 ne dup {gsave} if mode SETM str}def
/AS{/incr exch def CTEST
 incr exch 0 exch mode SM and 0 ne{{ashow} 4 copy SP}{ashow}ifelse{UP}if}def
/CTABALL{htabs /tabs exch def 0 1 maxtabs{tabs exch 999999 put}for}def
/STAB{currentpoint pop htabs
      /tabs exch def round /tabloc exch def tabs dup maxtabs get 999999 eq
      {tabloc TF dup dup tabs exch get tabloc ne
      {1 add maxtabs exch 1 neg exch{tabs exch dup tabs exch 1 sub get put}for
       tabloc put}{pop pop pop}ifelse}{pop}ifelse}def
/CTAB{currentpoint pop htabs
      /tabs exch def round dup/tabloc exch def TF dup tabs exch get tabloc eq
      {1 maxtabs 1 sub{tabs exch 2 copy 1 add get put}for
       tabs maxtabs 999999 put}{pop}ifelse}def
/DOTAB{currentpoint exch true htabs 
       /tabs exch def /sr exch def 1 add TF dup maxtabs le
       {tabs exch get dup 999999 ne
	{sr{exch moveto}{3 -1 roll sub neg moveto}ifelse}{CL}ifelse
       }{CL}ifelse}def
/CL{pop pop sr not{pop}if}def
/TF{/val exch def maxtabs 1 add 0 1 maxtabs
    {dup tabs exch get val ge {exch pop exit}{pop}ifelse}for}def
/AC{/incr exch def GETW neg MR add ML add 2 div currentpoint exch pop moveto
    0 2 nelts{JUMS  1 add  SARR exch get incr exch 0 exch mode SM and 0 ne
     {{ashow}4 copy SP}{ashow}ifelse{UP}if}for}def
/SJA{counttomark JA exch 0 exch getinterval astore exch pop /SARR exch def}def
/RMV{/incr exch def dup dup stringwidth pop PSHMI sub
 exch length 1 sub incr mul add neg 0 rmoveto gsave}def
/RMVBK{grestore PSHMI incr add neg 0 rmoveto}def

