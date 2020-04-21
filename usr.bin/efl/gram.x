# define EOS 1
# define NEWLINE 2
# define LPAR 3
# define RPAR 4
# define LBRACK 5
# define RBRACK 6
# define COMMA 7
# define COLON 8
# define ASGNOP 9
# define REPOP 10
# define OR 11
# define AND 12
# define NOT 13
# define RELOP 14
# define ADDOP 15
# define MULTOP 16
# define POWER 17
# define DOUBLEADDOP 18
# define ARROW 19
# define QUALOP 20
# define NAME 21
# define OPTNAME 22
# define STRUCTNAME 23
# define COMNAME 24
# define CONST 25
# define COMMON 26
# define INTERNAL 27
# define EXTERNAL 28
# define AUTOMATIC 29
# define STATIC 30
# define VALUE 31
# define PROCEDURE 32
# define BLOCKDATA 33
# define OPTION 34
# define INCLUDE 35
# define DEFINE 36
# define END 37
# define INTEGER 38
# define REAL 39
# define COMPLEX 40
# define LOGICAL 41
# define CHARACTER 42
# define STRUCT 43
# define FIELD 44
# define ARRAY 45
# define DIMENSION 46
# define LONG 47
# define SHORT 48
# define INITIAL 49
# define EQUIVALENCE 50
# define IMPLICIT 51
# define DEBUG 52
# define IF 53
# define ELSE 54
# define WHILE 55
# define UNTIL 56
# define REPEAT 57
# define DO 58
# define FOR 59
# define SWITCH 60
# define CASE 61
# define DEFAULT 62
# define GO 63
# define GOTO 64
# define BREAK 65
# define EXIT 66
# define NEXT 67
# define RETURN 68
# define CONTINUE 69
# define CALL 70
# define DOUBLE 71
# define PRECISION 72
# define DOUBLEPRECISION 73
# define SIZEOF 74
# define LENGTHOF 75
# define LETTER 76
# define READ 77
# define WRITE 78
# define READBIN 79
# define WRITEBIN 80
# define TRUE 81
# define FALSE 82
# define ESCAPE 83

# line 87 "gram.in"
#include "defs"
ptr bgnexec(), addexec(), bgnproc(), mkvar(), mkcomm(), mkstruct(), mkarrow();
ptr mkiost(), mkioitem(), mkiogroup(), mkformat();
ptr funcinv(), extrfield(), typexpr(), strucelt(), mkfield();
ptr esizeof(), elenof(), mkilab();
ptr ifthen(), doloop();
struct varblock *subscript();

# line 97 "gram.in"
typedef union  { int ival; ptr pval; char *cval; } YYSTYPE;

# line 126 "gram.in"
extern int prevv;
extern YYSTYPE prevl;
ptr p;
ptr procattrs;
int i,n;
static int imptype;
static int ininit =NO;

#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
typedef int yytabelem;
# define YYERRCODE 256

# line 1017 "gram.in"


setyydeb()
{
#ifdef YYDEBUG
	yydebug = 1;
#endif
}
yytabelem yyexca[] ={
-1, 0,
	0, 1,
	-2, 175,
-1, 1,
	0, -1,
	-2, 0,
-1, 80,
	9, 152,
	19, 152,
	20, 152,
	-2, 178,
-1, 165,
	19, 152,
	20, 152,
	-2, 179,
-1, 262,
	6, 191,
	-2, 14,
-1, 351,
	14, 0,
	-2, 144,
	};
# define YYNPROD 284
# define YYLAST 1451
yytabelem yyact[]={

   225,   247,   408,   431,   412,   124,   409,   337,   223,   197,
   157,    20,   258,   224,   185,   156,    19,     6,   140,     3,
    20,   155,    61,   139,   286,    19,    59,   314,     5,   131,
   297,   152,   212,    44,   182,   198,    37,    34,    36,    32,
    33,    35,    21,    22,    16,   191,    14,   135,    38,    39,
    40,    41,    26,    45,    27,    30,    31,    46,    47,    10,
    12,    11,    72,   227,   121,    77,    64,    17,    20,   446,
    66,    60,   413,    19,   179,   189,   425,    58,   195,   196,
   338,    42,   149,    43,   141,    65,    80,    73,   109,   129,
    78,    87,    71,   199,   400,   399,   170,   200,   172,   178,
   167,   175,   176,   177,   154,   211,   233,   305,   120,   315,
   203,   325,   218,   243,   326,   137,    62,   220,   218,    68,
   447,   148,   147,   356,    74,   209,   222,   242,   199,   123,
   204,   256,   200,   201,   202,   213,   229,   216,   215,   214,
   217,   213,    68,   216,   215,   214,   217,   134,    62,    73,
   169,   245,   165,    20,   164,    80,   166,   109,    19,    78,
    87,   278,    76,   132,   260,   133,   273,   161,   201,   202,
   144,   148,   147,   163,   382,   267,   250,   263,   160,   265,
   271,   272,   273,   148,   147,   259,   433,   280,   394,   282,
   148,   147,   228,   268,   269,   270,   239,   279,    69,   287,
   272,   273,   289,   235,   250,   127,   295,   296,   298,   298,
    58,   303,   240,   291,   292,   238,   262,   146,   128,    18,
    65,   252,   251,   344,   343,   449,   307,   148,   147,    55,
   310,   460,   429,   417,   253,   254,   434,   264,   283,   300,
   293,    20,   329,    20,   304,   173,    19,    20,    19,   319,
   322,   321,    19,   171,   372,   428,   316,   330,   429,   317,
   327,   323,   281,   141,   392,   377,     7,   393,   378,   367,
   342,   248,   348,   349,   350,   351,   352,   353,   354,   355,
   335,   364,   143,   154,   365,   357,   360,   234,   339,   363,
   462,   340,   333,   463,   287,   334,   277,   275,   276,   159,
   274,   271,   272,   273,   274,   271,   272,   273,   373,   376,
   255,   158,   375,   159,   380,   145,   302,   371,   366,   358,
   361,   276,   301,   274,   271,   272,   273,   379,   418,   389,
   423,   236,   396,   390,   420,   391,   237,   277,   275,   276,
   414,   274,   271,   272,   273,   398,   134,   416,   331,   397,
   415,   332,   451,   395,   277,   275,   276,   381,   274,   271,
   272,   273,   369,   259,   312,   277,   275,   276,   410,   274,
   271,   272,   273,   279,   445,   407,   279,   436,   465,   427,
   250,   150,   419,   250,   277,   275,   276,   421,   274,   271,
   272,   273,   125,   406,    20,   125,   403,   306,   285,    19,
   284,   424,   422,   308,    74,   122,   277,   275,   276,   249,
   274,   271,   272,   273,   219,   210,   207,   206,   435,    48,
   205,   439,   437,    50,   438,    52,    53,   162,   442,   126,
   410,   125,   441,   440,   444,   388,   443,   410,   448,   450,
   387,   386,   385,   383,   347,   453,   456,   345,   457,   458,
   150,    49,   414,   459,    51,   411,   142,     9,   461,   454,
   188,   370,   186,   208,   464,   187,   107,   192,    68,    99,
    44,    97,   180,    37,    34,    36,    32,    33,    35,    96,
   426,    95,   266,   174,    88,    38,    39,    40,    41,    26,
    45,    27,    30,    31,    46,    47,   455,    86,    84,   341,
   168,   452,   277,   275,   276,   116,   274,   271,   272,   273,
   277,   275,   276,   261,   274,   271,   272,   273,    42,    82,
    43,   193,   194,   150,   113,   114,   111,   112,   195,   196,
   384,   230,   328,   188,   221,   186,   226,   336,   187,   153,
   192,    68,   241,    44,    70,   180,    37,    34,    36,    32,
    33,    35,   151,    63,   136,    54,     8,    75,    38,    39,
    40,    41,    26,    45,    27,    30,    31,    46,    47,   430,
    13,   277,   275,   276,   404,   274,   271,   272,   273,   232,
   277,   275,   276,   231,   274,   271,   272,   273,     4,     2,
   102,    42,   432,    43,   193,   194,   110,   113,   114,   111,
   112,   195,   196,   150,   181,    29,    23,   288,    25,   294,
   299,   290,   346,   188,    85,   186,    15,   246,   187,    67,
   192,    68,   184,    44,   183,   180,    37,    34,    36,    32,
    33,    35,    28,    56,   309,    24,   318,   138,    38,    39,
    40,    41,    26,    45,    27,    30,    31,    46,    47,   401,
   130,   257,   320,     1,     0,   277,   275,   276,     0,   274,
   271,   272,   273,     0,     0,     0,     0,     0,     0,     0,
     0,    42,     0,    43,   193,   194,     0,   113,   114,   111,
   112,   195,   196,     0,   150,   244,     0,     0,     0,     0,
     0,     0,     0,     0,   188,     0,   186,     0,     0,   187,
     0,   192,    68,   190,    44,     0,   180,    37,    34,    36,
    32,    33,    35,     0,     0,     0,     0,     0,     0,    38,
    39,    40,    41,    26,    45,    27,    30,    31,    46,    47,
   374,     0,     0,     0,     0,     0,   277,   275,   276,     0,
   274,   271,   272,   273,     0,     0,     0,     0,     0,     0,
     0,     0,    42,     0,    43,   193,   194,   150,   113,   114,
   111,   112,   195,   196,     0,     0,     0,   188,     0,   186,
     0,     0,   187,     0,   192,    68,   190,    44,     0,   180,
    37,    34,    36,    32,    33,    35,     0,     0,     0,     0,
     0,     0,    38,    39,    40,    41,    26,    45,    27,    30,
    31,    46,    47,   368,     0,     0,     0,     0,     0,   277,
   275,   276,     0,   274,   271,   272,   273,     0,     0,     0,
     0,     0,     0,     0,     0,    42,     0,    43,   193,   194,
     0,   113,   114,   111,   112,   195,   196,   150,     0,     0,
     0,     0,     0,     0,     0,     0,     0,   188,     0,   186,
     0,     0,   187,     0,   192,    68,   190,    44,     0,   180,
    37,    34,    36,    32,    33,    35,     0,     0,     0,     0,
     0,     0,    38,    39,    40,    41,   362,    45,    27,    30,
    31,    46,    47,   313,     0,   277,   275,   276,   311,   274,
   271,   272,   273,     0,   277,   275,   276,     0,   274,   271,
   272,   273,     0,     0,     0,    42,     0,    43,   193,   194,
     0,   113,   114,   111,   112,   195,   196,     0,   150,     0,
     0,     0,     0,     0,     0,     0,     0,     0,   188,     0,
   186,     0,     0,   187,     0,   192,    68,   190,    44,     0,
   180,    37,    34,    36,    32,    33,    35,     0,     0,     0,
     0,     0,     0,    38,    39,    40,    41,   359,    45,    27,
    30,    31,    46,    47,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,    42,     0,    43,   193,
   194,     0,   113,   114,   111,   112,   195,   196,    44,     0,
     0,    37,    34,    36,    32,    33,    35,     0,     0,     0,
   190,    14,     0,    38,    39,    40,    41,    26,    45,    27,
    30,    31,    46,    47,    10,    12,    11,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    83,
    57,     0,     0,     0,     0,     0,    42,     0,    43,     0,
     0,     0,    79,     0,     0,    91,    62,     0,    44,    92,
     0,    37,    34,    36,    32,    33,    35,    21,    22,     0,
     0,     0,     0,    38,    39,    40,    41,    26,    45,    27,
    30,    31,    46,    47,     0,     0,    90,   104,     0,   106,
   190,   105,   116,   115,   108,    94,    93,   101,   100,   118,
   119,   117,    98,   103,    81,     0,    42,     0,    43,     0,
     0,   113,   114,   111,   112,     0,    62,    89,    44,     0,
     0,    37,    34,    36,    32,    33,    35,     0,     0,     0,
     0,     0,     0,    38,    39,    40,    41,    26,    45,    27,
    30,    31,    46,    47,    10,    12,    11,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    44,     0,     0,
    37,    34,    36,    32,    33,    35,    42,     0,    43,     0,
     0,   190,    38,    39,    40,    41,    26,    45,    27,    30,
    31,    46,    47,    10,    12,    11,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,   150,   405,     0,
     0,     0,     0,     0,     0,    42,     0,    43,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    44,     0,     0,
    37,    34,    36,    32,    33,    35,     0,     0,     0,     0,
     0,   324,    38,    39,    40,    41,    26,    45,    27,    30,
    31,    46,    47,     0,     0,     0,   150,   402,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    42,    44,    43,     0,    37,
    34,    36,    32,    33,    35,     0,     0,     0,     0,     0,
     0,    38,    39,    40,    41,    26,    45,    27,    30,    31,
    46,    47,     0,     0,     0,     0,     0,    57,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    62,    42,    44,    43,     0,    37,    34,
    36,    32,    33,    35,     0,     0,     0,     0,     0,     0,
    38,    39,    40,    41,    26,    45,    27,    30,    31,    46,
    47,     0,     0,     0,   150,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    42,    44,    43,     0,    37,    34,    36,
    32,    33,    35,     0,     0,     0,     0,     0,     0,    38,
    39,    40,    41,    26,    45,    27,    30,    31,    46,    47,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    44,     0,     0,    37,    34,    36,    32,    33,    35,     0,
     0,     0,    42,     0,    43,    38,    39,    40,    41,    26,
    45,    27,    30,    31,    46,    47,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,    42,     0,
    43 };
yytabelem yypact[]={

    10, -1000,   450,   450,   453,   450,   450, -1000, -1000,  1035,
    98, -1000,   121,   141, -1000,  1034, -1000, -1000, -1000,   402,
   402, -1000, -1000, -1000, -1000, -1000,   428,   426, -1000, -1000,
 -1000, -1000, -1000, -1000, -1000, -1000, -1000,   202, -1000, -1000,
 -1000, -1000,    17, -1000, -1000,   142, -1000, -1000, -1000, -1000,
 -1000, -1000, -1000, -1000,    93, -1000, -1000,  1095, -1000,   275,
 -1000,   402, -1000,   308, -1000,   208, -1000,   378, -1000,  1377,
   304,   292, -1000,   171,    98,   424, -1000, -1000,   164,    98,
 -1000,    98, -1000, -1000,  1034, -1000, -1000, -1000, -1000, -1000,
 -1000,   245, -1000,   237, -1000, -1000, -1000, -1000,   754, -1000,
   107,   109,   417, -1000,   414, -1000,   413, -1000,   412, -1000,
    80, -1000, -1000, -1000, -1000,   411,   754, -1000, -1000, -1000,
 -1000, -1000, -1000, -1000, -1000,   754,   520, -1000, -1000, -1000,
 -1000, -1000, -1000, -1000, -1000,    69,   280,   194,   330, -1000,
 -1000,   275,  1292,    95,   187,    98, -1000,   106,    90, -1000,
   681,   264, -1000,   406,  1377, -1000, -1000,   402,   401,    98,
    98,   306,   127,   754,   102, -1000,   102, -1000, -1000, -1000,
 -1000, -1000,   229, -1000,   754, -1000, -1000, -1000,   355,   152,
 -1000, -1000,  1341, -1000, -1000, -1000,   754,    98,   754, -1000,
 -1000, -1000,    -3,   397,   395, -1000, -1000, -1000,   600, -1000,
 -1000,   754, -1000,    72,   107,   754,   754,   754,   754,   315,
   754,    86,    82, -1000, -1000, -1000, -1000, -1000, -1000, -1000,
   396,   520,   884,   360, -1000,   875, -1000,    85,    85,   341,
  1134, -1000,   975, -1000, -1000,    89,  1095, -1000, -1000, -1000,
 -1000,   754,   378, -1000, -1000,   344,   288, -1000,  1377,     4,
 -1000, -1000, -1000,   102,   102, -1000, -1000,   284, -1000, -1000,
   355, -1000, -1000, -1000, -1000, -1000,   216,   355,   446,   443,
 -1000,   754,   754,   754,   754,   754,   754,   754,   754, -1000,
   149,   102,   290,   103,   915,   834, -1000,   355,   754,   355,
   277, -1000, -1000,   600,   262,   355,   799,   358,   355, -1000,
   247, -1000, -1000,   726, -1000, -1000, -1000, -1000,   754,   261,
 -1000, -1000, -1000,   520,   353, -1000,   158, -1000,   442, -1000,
 -1000,   441,   440,   439,   434, -1000, -1000, -1000,   754,   355,
 -1000, -1000,   754, -1000,   754, -1000,   260, -1000,   173, -1000,
    95,   326, -1000, -1000,   754,    41, -1000,    38,   184,   149,
   149,   165,   309,   290,   355,   355, -1000,   645,  1243,   392,
   570,  1194,   389,   355, -1000,    72, -1000,   447, -1000, -1000,
 -1000,   346, -1000,   226, -1000, -1000,   327, -1000,   520, -1000,
   355, -1000, -1000,  1134,   324, -1000, -1000, -1000, -1000,   355,
   355,   355, -1000,     4,     0, -1000, -1000, -1000,   355, -1000,
   376, -1000, -1000, -1000, -1000, -1000, -1000, -1000,   251, -1000,
   561, -1000,   228,   335, -1000, -1000, -1000, -1000, -1000, -1000,
   754, -1000, -1000,   432, -1000, -1000, -1000,   754, -1000,   447,
 -1000, -1000,   371,    44, -1000,   217,   447,   348, -1000,   500,
 -1000, -1000,   492, -1000, -1000,   754, -1000, -1000, -1000, -1000,
   225, -1000, -1000, -1000, -1000, -1000,   286, -1000, -1000, -1000,
 -1000, -1000, -1000,   754,   374, -1000 };
yytabelem yypgo[]={

     0,   653,    18,   652,    17,    47,   219,   651,    12,    22,
    27,   650,    23,   637,   636,    34,    92,   456,    67,   635,
   108,    64,   634,     8,    13,   633,    26,    71,    21,    15,
   632,    29,     0,    74,    14,    75,   624,   622,    70,   619,
   617,   616,   614,   612,   611,    35,    24,    72,    30,   610,
   609,     2,     6,     4,     3,   608,   606,   605,   604,    45,
   596,    32,   592,   590,     9,   589,   419,   588,   583,    28,
   579,   570,   557,   556,   555,   554,     5,   553,   186,   552,
     1,   544,    66,   542,    31,   539,   537,     7,    62,    10,
   534,   532,    63,   531,   530,    65,   519,   513,   500,   499,
   498,   497,   484,   483,   482,   481,   480,   479,   471,   469,
   466,   463,   461,   377 };
yytabelem yyr1[]={

     0,     1,     1,     1,     1,     1,     1,     1,    66,     3,
     3,     3,     3,     5,    70,     5,    67,    67,    67,    67,
    72,    71,    71,     6,     6,     7,     7,     8,    65,    73,
    74,    74,    74,    75,    75,    75,    69,    68,    76,    13,
    13,    13,    12,    12,     2,     2,     2,     2,     2,     2,
    14,    14,    14,    77,    77,    83,    82,    79,    79,    84,
    84,    85,    86,    86,    87,    87,    81,    81,    88,    16,
    16,    17,    17,    18,    18,    20,    20,    90,    21,    22,
    22,    23,    23,    24,    24,    25,    25,    26,    26,    27,
    91,    27,     9,    15,    15,    28,    28,    29,    29,    29,
    29,    29,    29,    29,    56,    56,    56,    56,    56,    19,
    19,    92,    10,    10,    55,    55,    55,    55,    55,    55,
    30,    30,    30,    11,    11,    93,    94,    31,    89,    89,
    57,    57,    32,    32,    32,    32,    32,    32,    32,    32,
    32,    32,    32,    32,    32,    32,    32,    32,    32,    32,
    32,    32,    33,    38,    38,    38,    38,    38,    39,    34,
    34,    34,    40,    40,    36,    36,    36,    37,    37,    37,
    58,    58,    59,    59,     4,    41,    95,    95,    95,    95,
    95,    95,    95,    95,    95,    95,    95,    95,   102,   102,
    98,    99,   100,   100,   100,   100,   103,   100,   104,   104,
    42,    42,    42,    42,   105,   106,   107,    43,    43,   101,
   101,   101,   101,   101,   101,   101,    45,    44,    44,    46,
    46,    64,    64,    64,    64,   109,   109,   109,   109,   109,
    60,    60,    60,    61,    61,    61,    61,    61,    61,   108,
   108,   108,   108,    47,    47,    47,    47,    47,    47,   110,
   111,   111,    49,   112,   112,    48,    48,    96,    97,    35,
    63,    63,    63,    63,    50,    50,    51,    51,    52,    52,
    52,    52,    52,    52,    52,   113,    53,    54,    54,    54,
    54,    62,    78,    80 };
yytabelem yyr2[]={

     0,     1,     5,     5,     9,     5,     5,     3,     3,     5,
     5,     5,     5,     1,     1,     7,     3,     5,     9,    11,
     3,     3,     5,     3,     3,     3,     7,     3,     5,     3,
     0,     4,     6,     3,     7,     7,     3,     3,     1,     2,
     4,     7,     2,     2,     5,     9,     5,     9,     5,     5,
     2,     4,     7,     2,     6,     1,     9,     2,     6,     3,
     8,     3,     2,     6,     3,     7,     2,     6,     7,     7,
     7,     2,     5,     5,     5,     1,     2,     1,     9,     2,
     7,     3,     7,     2,     3,     1,     2,     2,     7,     5,
     1,    11,     3,     2,     5,     2,     5,     3,     5,     3,
     9,     9,     3,     3,     3,     3,     3,     3,     3,    11,
    11,     1,     1,     2,     3,     3,     3,     3,     5,     3,
     3,     9,     5,     3,     3,     1,     1,    13,     2,     2,
     3,     3,     3,     2,     3,     5,     2,     2,     3,     7,
     7,     7,     5,     5,     7,     7,     7,     5,     7,     7,
     2,     3,     3,     2,     5,     7,     9,     7,     3,     5,
     7,     7,     7,     7,     9,     9,     9,     9,     9,     9,
     2,     7,     2,     2,     5,     1,     7,     5,     3,     5,
     6,    11,     5,     3,     3,     3,     3,     3,     1,     3,
     1,     1,     7,     5,     9,     7,     1,    10,     3,     7,
    17,     9,     9,     7,     9,     1,     3,     3,    11,     3,
     5,     2,     5,     7,     7,     9,     7,     3,     7,     2,
     5,     3,     3,     5,     3,     3,     5,     5,     7,     7,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     9,
     9,     9,     3,     9,     7,    13,    11,    17,    15,     5,
     5,     9,     4,     5,     9,     1,     2,     5,     1,    13,
     3,     3,     3,     3,     2,     1,     3,     7,     3,     7,
     5,     3,     5,     9,     7,     1,     9,     3,     9,    13,
     7,     7,     1,     1 };
yytabelem yychk[]={

 -1000,    -1,   -65,    -2,   -67,   -69,    -4,   256,   -73,   -17,
    49,    51,    50,   -71,    36,   -41,    34,   -18,    -6,   -29,
   -89,    32,    33,   -56,   -19,   -55,    42,    44,   -30,   -57,
    45,    46,    29,    30,    27,    31,    28,    26,    38,    39,
    40,    41,    71,    73,    23,    43,    47,    48,   -66,     1,
   -66,     1,   -66,   -66,   -74,    -6,   -25,     5,   -18,   -26,
   -27,    -9,    21,   -77,   -82,   -33,   -38,   -39,    21,   -78,
   -81,   -16,   -88,   -33,     3,   -72,    21,   -95,   -33,    18,
   -38,    70,   -96,     5,  -100,   -42,  -101,   -35,  -102,    83,
    52,    21,    25,    62,    61,  -105,  -107,  -108,    68,  -109,
    64,    63,   -63,    69,    53,    57,    55,  -110,    60,   -47,
   -60,    79,    80,    77,    78,    59,    58,    67,    65,    66,
   -20,   -21,     3,   -20,   -76,     3,     3,     3,    16,    72,
   -11,   -31,    21,    23,     5,    -5,   -75,    22,   -13,   -12,
    -2,   -26,   -17,     7,   -20,     7,     9,    20,    19,   -34,
     3,   -79,   -84,   -85,   -15,   -28,   -29,   -89,     7,     7,
     7,   -16,     3,     9,   -33,   -38,   -33,    -4,   -98,   -95,
   -76,     8,   -76,     8,  -103,   -76,   -76,   -76,   -32,   -33,
    25,   -58,   -15,   -36,   -37,   -34,    15,    18,    13,   -35,
   256,   -59,    20,    74,    75,    81,    82,   -64,   -45,    21,
    25,    61,    62,     3,    21,     3,     3,     3,  -111,    -4,
     3,    25,   -61,    55,    59,    58,    57,    60,    32,     3,
   -32,   -90,   -32,   -23,   -24,   -32,    16,   -92,   -92,   -76,
   -93,   -68,   -70,    37,     7,     9,     1,     6,   -27,     9,
   -82,   -83,    21,    23,     4,   -32,   -40,   -80,     7,     3,
   -28,   -21,   -88,   -33,   -33,     4,     4,    -7,    -8,    -9,
   -32,   -97,    -5,   -76,     8,   -76,  -104,   -32,    -4,    -4,
    -4,    15,    16,    17,    14,    11,    12,    10,     9,   -34,
   -32,   -33,   -32,   -59,     3,     3,   -46,   -32,     7,   -32,
   -44,   -64,   -64,   -45,   -50,   -32,   -32,   -48,   -32,   -49,
   -48,     7,     1,   -32,   -61,    25,     1,   -76,     7,   -22,
   -23,     4,     4,     8,   -10,    24,   -10,   -31,   -14,    -2,
    -3,    -2,    -4,   -69,   256,    22,    25,   -12,   -91,   -32,
   -34,     4,     7,     4,     7,   -84,   -86,   -87,    76,     4,
     7,   -99,   -76,     8,     7,     1,   -43,     1,   -32,   -32,
   -32,   -32,   -32,   -32,   -32,   -32,    20,   -32,   -15,    42,
   -32,   -15,    42,   -32,     4,     7,   -46,     7,     4,     4,
  -112,    -4,     7,   -76,     4,   -76,   -32,     4,     7,   -24,
   -32,     4,    16,     1,   -94,     1,     1,     1,     1,   -32,
   -32,   -32,     4,     7,    15,    -8,     6,   -76,   -32,    54,
    56,     4,     4,     4,     4,     4,     4,   -64,   -51,   -52,
   -32,     8,   -53,   -47,     5,     4,     1,     7,     1,   -76,
     7,   -23,    -2,     6,   -87,    76,  -106,     3,     4,     7,
     8,   -54,   -62,   -78,     8,   -53,  -113,   -76,   -76,   -32,
     1,   -76,   -32,   -52,   -54,     3,    25,    76,   -54,     8,
   -51,     4,     1,   -76,    -4,     4,   -32,   -80,   -80,   -54,
     6,   -76,     4,     7,   -32,     4 };
yytabelem yydef[]={

    -2,    -2,     0,     0,     0,     0,     0,     7,    30,    85,
     0,   282,     0,    16,    36,   188,    29,    71,    21,    75,
    75,    23,    24,    97,    38,    99,     0,     0,   102,   103,
   128,   129,   104,   105,   106,   107,   108,     0,   114,   115,
   116,   117,     0,   119,   120,     0,   130,   131,     2,     8,
     3,    13,     5,     6,    28,    22,    44,     0,    72,    86,
    87,    75,    92,    46,    53,     0,   152,   153,   158,     0,
    48,    49,    66,     0,     0,    17,    20,   174,     0,     0,
    -2,     0,   175,   190,   188,   183,   184,   185,   186,   187,
    38,   158,    38,     0,   196,    38,    38,    38,   209,   211,
     0,     0,     0,   189,     0,   206,     0,   175,     0,   242,
   225,   260,   261,   262,   263,     0,     0,   230,   231,   232,
    73,    76,    77,    74,    98,     0,     0,   111,   111,   118,
    38,   122,   123,   124,   125,    14,    31,    33,     0,    39,
    42,    43,    85,     0,    89,     0,    55,     0,     0,   154,
     0,   283,    57,    59,    61,    93,    95,     0,     0,     0,
     0,     0,     0,     0,   177,    -2,     0,   258,    13,   182,
   257,    38,   193,    38,     0,   175,   175,   175,   210,   132,
   133,   134,     0,   136,   137,   138,     0,     0,     0,   150,
   151,   170,     0,     0,     0,   172,   173,   212,     0,   221,
   222,     0,   224,     0,     0,   265,     0,   255,   255,     0,
     0,   226,   227,   233,   234,   235,   236,   237,   238,   249,
    38,     0,     0,     0,    81,    83,    84,   112,   112,     0,
     0,     4,   175,    37,    32,     0,    40,    45,    88,    90,
    54,     0,   155,   157,   159,     0,     0,    47,     0,     0,
    94,    96,    67,    70,    69,    68,    18,     0,    25,    27,
   176,   180,    -2,   192,    38,   195,     0,   198,     0,     0,
   203,     0,     0,     0,     0,     0,     0,     0,     0,   135,
   142,   143,   147,     0,     0,     0,   214,   219,     0,   223,
     0,   217,   213,     0,     0,   264,     0,     0,   256,   175,
     0,   250,    38,     0,   228,   229,    38,   244,     0,     0,
    79,   100,   101,     0,     0,   113,     0,   121,   126,    50,
    15,     0,     0,     0,     0,    34,    35,    41,     0,    56,
   156,   160,     0,   161,     0,    58,     0,    62,    64,    19,
     0,     0,   194,    38,     0,   201,   202,   207,   139,   140,
   141,    -2,   145,   146,   149,   148,   171,     0,     0,     0,
     0,     0,     0,   220,   216,     0,   215,     0,   204,   239,
   240,     0,   252,     0,   241,   243,    38,    78,     0,    82,
    83,   109,   110,    51,     0,     9,    10,    11,    12,    91,
   162,   163,    60,     0,     0,    26,   181,   197,   199,   205,
     0,   164,   165,   166,   167,   168,   169,   218,     0,   266,
   268,   282,   271,     0,   275,   253,    38,   251,    38,   246,
     0,    80,    52,     0,    63,    65,    38,     0,   259,     0,
   282,   270,   277,     0,   282,   272,     0,     0,   245,    38,
   127,   175,     0,   267,   269,     0,   283,   283,   274,   282,
     0,   254,    38,   248,   200,   208,     0,   280,   281,   273,
   276,   247,   278,     0,     0,   279 };
typedef struct { char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
	"EOS",	1,
	"NEWLINE",	2,
	"LPAR",	3,
	"RPAR",	4,
	"LBRACK",	5,
	"RBRACK",	6,
	"COMMA",	7,
	"COLON",	8,
	"ASGNOP",	9,
	"REPOP",	10,
	"OR",	11,
	"AND",	12,
	"NOT",	13,
	"RELOP",	14,
	"ADDOP",	15,
	"MULTOP",	16,
	"POWER",	17,
	"DOUBLEADDOP",	18,
	"ARROW",	19,
	"QUALOP",	20,
	"NAME",	21,
	"OPTNAME",	22,
	"STRUCTNAME",	23,
	"COMNAME",	24,
	"CONST",	25,
	"COMMON",	26,
	"INTERNAL",	27,
	"EXTERNAL",	28,
	"AUTOMATIC",	29,
	"STATIC",	30,
	"VALUE",	31,
	"PROCEDURE",	32,
	"BLOCKDATA",	33,
	"OPTION",	34,
	"INCLUDE",	35,
	"DEFINE",	36,
	"END",	37,
	"INTEGER",	38,
	"REAL",	39,
	"COMPLEX",	40,
	"LOGICAL",	41,
	"CHARACTER",	42,
	"STRUCT",	43,
	"FIELD",	44,
	"ARRAY",	45,
	"DIMENSION",	46,
	"LONG",	47,
	"SHORT",	48,
	"INITIAL",	49,
	"EQUIVALENCE",	50,
	"IMPLICIT",	51,
	"DEBUG",	52,
	"IF",	53,
	"ELSE",	54,
	"WHILE",	55,
	"UNTIL",	56,
	"REPEAT",	57,
	"DO",	58,
	"FOR",	59,
	"SWITCH",	60,
	"CASE",	61,
	"DEFAULT",	62,
	"GO",	63,
	"GOTO",	64,
	"BREAK",	65,
	"EXIT",	66,
	"NEXT",	67,
	"RETURN",	68,
	"CONTINUE",	69,
	"CALL",	70,
	"DOUBLE",	71,
	"PRECISION",	72,
	"DOUBLEPRECISION",	73,
	"SIZEOF",	74,
	"LENGTHOF",	75,
	"LETTER",	76,
	"READ",	77,
	"WRITE",	78,
	"READBIN",	79,
	"WRITEBIN",	80,
	"TRUE",	81,
	"FALSE",	82,
	"ESCAPE",	83,
	"-unknown-",	-1	/* ends search */
};

char * yyreds[] =
{
	"-no such reduction-",
	"graal : /* empty */",
	"graal : option endchunk",
	"graal : dcl endchunk",
	"graal : procst EOS stats end",
	"graal : define endchunk",
	"graal : exec endchunk",
	"graal : error",
	"endchunk : EOS",
	"stat : dcl EOS",
	"stat : exec EOS",
	"stat : define EOS",
	"stat : error EOS",
	"stats : /* empty */",
	"stats : stats",
	"stats : stats stat",
	"procst : oproc",
	"procst : oproc procname",
	"procst : oproc procname LPAR RPAR",
	"procst : oproc procname LPAR args RPAR",
	"procname : NAME",
	"oproc : proc",
	"oproc : attrs proc",
	"proc : PROCEDURE",
	"proc : BLOCKDATA",
	"args : arg",
	"args : args COMMA arg",
	"arg : varname",
	"option : optson optionnames",
	"optson : OPTION",
	"optionnames : /* empty */",
	"optionnames : optionnames optelt",
	"optionnames : optionnames optelt COMMA",
	"optelt : OPTNAME",
	"optelt : OPTNAME ASGNOP OPTNAME",
	"optelt : OPTNAME ASGNOP CONST",
	"define : DEFINE",
	"end : END",
	"contnu : /* empty */",
	"dcls1 : dcl1",
	"dcls1 : dcls1 EOS",
	"dcls1 : dcls1 EOS dcl1",
	"dcl1 : dcl",
	"dcl1 : varlist",
	"dcl : attrs vars",
	"dcl : attrs LBRACK dcls1 RBRACK",
	"dcl : INITIAL initlist",
	"dcl : IMPLICIT letton implist lettoff",
	"dcl : EQUIVALENCE equivsets",
	"dcl : EQUIVALENCE equivlist",
	"dcls : dcl",
	"dcls : dcls EOS",
	"dcls : dcls EOS dcl",
	"initlist : init",
	"initlist : initlist COMMA init",
	"init : lhs ASGNOP",
	"init : lhs ASGNOP expr",
	"implist : impgroup",
	"implist : implist COMMA impgroup",
	"impgroup : impspec",
	"impgroup : impspec LPAR impsets RPAR",
	"impspec : specs",
	"impsets : impset",
	"impsets : impsets COMMA impset",
	"impset : LETTER",
	"impset : LETTER ADDOP LETTER",
	"equivsets : equivset",
	"equivsets : equivsets COMMA equivset",
	"equivset : LPAR equivlist RPAR",
	"equivlist : lhs COMMA lhs",
	"equivlist : equivlist COMMA lhs",
	"attrs : attr",
	"attrs : attrs attr",
	"attr : spec dim",
	"attr : array dim",
	"dim : /* empty */",
	"dim : dimbound",
	"dimbound : LPAR",
	"dimbound : LPAR bounds RPAR",
	"bounds : bound",
	"bounds : bounds COMMA bound",
	"bound : ubound",
	"bound : expr COLON ubound",
	"ubound : expr",
	"ubound : MULTOP",
	"vars : /* empty */",
	"vars : varlist",
	"varlist : var",
	"varlist : varlist COMMA var",
	"var : varname dim",
	"var : varname dim ASGNOP",
	"var : varname dim ASGNOP expr",
	"varname : NAME",
	"specs : specarray",
	"specs : specs specarray",
	"specarray : spec",
	"specarray : array dimbound",
	"spec : sclass",
	"spec : comclass contnu",
	"spec : stype",
	"spec : CHARACTER LPAR expr RPAR",
	"spec : FIELD LPAR bound RPAR",
	"spec : deftype",
	"spec : prec",
	"sclass : AUTOMATIC",
	"sclass : STATIC",
	"sclass : INTERNAL",
	"sclass : VALUE",
	"sclass : EXTERNAL",
	"comclass : COMMON LPAR comneed comname RPAR",
	"comclass : COMMON MULTOP comneed comname MULTOP",
	"comneed : /* empty */",
	"comname : /* empty */",
	"comname : COMNAME",
	"stype : INTEGER",
	"stype : REAL",
	"stype : COMPLEX",
	"stype : LOGICAL",
	"stype : DOUBLE PRECISION",
	"stype : DOUBLEPRECISION",
	"deftype : STRUCTNAME",
	"deftype : STRUCT structname contnu struct",
	"deftype : STRUCT struct",
	"structname : NAME",
	"structname : STRUCTNAME",
	"struct : LBRACK",
	"struct : LBRACK dcls",
	"struct : LBRACK dcls RBRACK EOS",
	"array : ARRAY",
	"array : DIMENSION",
	"prec : LONG",
	"prec : SHORT",
	"expr : lhs",
	"expr : CONST",
	"expr : logcon",
	"expr : specs parexprs",
	"expr : sizeof",
	"expr : lengthof",
	"expr : parexprs",
	"expr : expr ADDOP expr",
	"expr : expr MULTOP expr",
	"expr : expr POWER expr",
	"expr : ADDOP expr",
	"expr : DOUBLEADDOP lhs",
	"expr : expr RELOP expr",
	"expr : expr OR expr",
	"expr : expr AND expr",
	"expr : NOT expr",
	"expr : lhs ASGNOP expr",
	"expr : expr REPOP expr",
	"expr : iostat",
	"expr : error",
	"lhs : lhs1",
	"lhs1 : lhsname",
	"lhs1 : lhsname parexprs",
	"lhs1 : lhs QUALOP NAME",
	"lhs1 : lhs QUALOP NAME parexprs",
	"lhs1 : lhs ARROW STRUCTNAME",
	"lhsname : NAME",
	"parexprs : LPAR RPAR",
	"parexprs : LPAR expr RPAR",
	"parexprs : LPAR exprlist RPAR",
	"exprlist : expr COMMA expr",
	"exprlist : exprlist COMMA expr",
	"sizeof : SIZEOF LPAR expr RPAR",
	"sizeof : SIZEOF LPAR specs RPAR",
	"sizeof : SIZEOF LPAR CHARACTER RPAR",
	"lengthof : LENGTHOF LPAR expr RPAR",
	"lengthof : LENGTHOF LPAR specs RPAR",
	"lengthof : LENGTHOF LPAR CHARACTER RPAR",
	"logcon : logval",
	"logcon : QUALOP logval QUALOP",
	"logval : TRUE",
	"logval : FALSE",
	"exec : beginexec exec1",
	"beginexec : /* empty */",
	"exec1 : lhs ASGNOP expr",
	"exec1 : DOUBLEADDOP lhs",
	"exec1 : lhs1",
	"exec1 : CALL lhs1",
	"exec1 : debug exec enddebug",
	"exec1 : LBRACK beginblock stats endblock RBRACK",
	"exec1 : labels exec1",
	"exec1 : control",
	"exec1 : branch",
	"exec1 : iostat",
	"exec1 : null",
	"exec1 : ESCAPE",
	"null : /* empty */",
	"null : CONTINUE",
	"beginblock : /* empty */",
	"endblock : /* empty */",
	"labels : NAME COLON contnu",
	"labels : CONST contnu",
	"labels : CONST contnu COLON contnu",
	"labels : DEFAULT COLON contnu",
	"labels : CASE",
	"labels : CASE caselist COLON contnu",
	"caselist : expr",
	"caselist : caselist COMMA expr",
	"control : ifclause contnu exec EOS ELSE elsecode contnu exec",
	"control : ifclause contnu exec EOS",
	"control : repeat contnu exec until",
	"control : leftcont contnu exec",
	"ifclause : IF LPAR expr RPAR",
	"elsecode : /* empty */",
	"repeat : REPEAT",
	"until : EOS",
	"until : EOS UNTIL LPAR expr RPAR",
	"branch : RETURN",
	"branch : RETURN expr",
	"branch : break",
	"branch : GOTO label",
	"branch : GO NAME label",
	"branch : GOTO parlablist compgotoindex",
	"branch : GO NAME parlablist compgotoindex",
	"parlablist : LPAR lablist RPAR",
	"lablist : label",
	"lablist : lablist COMMA label",
	"compgotoindex : expr",
	"compgotoindex : COMMA expr",
	"label : NAME",
	"label : CONST",
	"label : CASE expr",
	"label : DEFAULT",
	"break : brk",
	"break : brk CONST",
	"break : brk blocktype",
	"break : brk CONST blocktype",
	"break : brk blocktype CONST",
	"brk : NEXT",
	"brk : BREAK",
	"brk : EXIT",
	"blocktype : WHILE",
	"blocktype : FOR",
	"blocktype : DO",
	"blocktype : REPEAT",
	"blocktype : SWITCH",
	"blocktype : PROCEDURE",
	"leftcont : WHILE LPAR exprnull RPAR",
	"leftcont : for forinit fortest forincr",
	"leftcont : SWITCH LPAR expr RPAR",
	"leftcont : do",
	"do : DO expr EOS contnu",
	"do : DO expr contnu",
	"do : DO expr COMMA expr EOS contnu",
	"do : DO expr COMMA expr contnu",
	"do : DO expr COMMA expr COMMA expr EOS contnu",
	"do : DO expr COMMA expr COMMA expr contnu",
	"for : FOR LPAR",
	"forinit : exec COMMA",
	"forinit : exec EOS contnu COMMA",
	"fortest : exprnull COMMA",
	"forincr : exec RPAR",
	"forincr : exec EOS contnu RPAR",
	"exprnull : /* empty */",
	"exprnull : expr",
	"debug : DEBUG contnu",
	"enddebug : /* empty */",
	"iostat : iokwd LPAR iounit COMMA iolist RPAR",
	"iokwd : READBIN",
	"iokwd : WRITEBIN",
	"iokwd : READ",
	"iokwd : WRITE",
	"iounit : expr",
	"iounit : /* empty */",
	"iolist : ioitem",
	"iolist : iolist COMMA ioitem",
	"ioitem : expr",
	"ioitem : expr COLON format",
	"ioitem : COLON format",
	"ioitem : iobrace",
	"ioitem : do iobrace",
	"ioitem : do iobrace COLON format",
	"ioitem : iobrace COLON format",
	"iobrace : LBRACK",
	"iobrace : LBRACK iolist RBRACK",
	"format : letter",
	"format : letter LPAR expr RPAR",
	"format : letter LPAR expr COMMA expr RPAR",
	"format : letton CONST lettoff",
	"letter : letton LETTER lettoff",
	"letton : /* empty */",
	"lettoff : /* empty */",
};
#endif /* YYDEBUG */
/*	@(#)yaccpar	1.9	*/

/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#define YYERROR		goto yyerrlab
#define YYACCEPT	return(0)
#define YYABORT		return(1)
#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( "syntax error - cannot backup" );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}
#define YYRECOVERING()	(!!yyerrflag)
#ifndef YYDEBUG
#	define YYDEBUG	1	/* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;			/* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG		(-1000)

/*
** global variables used by the parser
*/
YYSTYPE yyv[ YYMAXDEPTH ];	/* value stack */
int yys[ YYMAXDEPTH ];		/* state stack */

YYSTYPE *yypv;			/* top of value stack */
int *yyps;			/* top of state stack */

int yystate;			/* current state */
int yytmp;			/* extra var (lasts between blocks) */

int yynerrs;			/* number of errors */
int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */



/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
int
yyparse()
{
	register YYSTYPE *yypvt;	/* top of value stack for $vars */

	/*
	** Initialize externals - yyparse may be called more than once
	*/
	yypv = &yyv[-1];
	yyps = &yys[-1];
	yystate = 0;
	yytmp = 0;
	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;

	goto yystack;
	{
		register YYSTYPE *yy_pv;	/* top of value stack */
		register int *yy_ps;		/* top of state stack */
		register int yy_state;		/* current state */
		register int  yy_n;		/* internal state number info */

		/*
		** get globals into registers.
		** branch to here only if YYBACKUP was called.
		*/
	yynewstate:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;
		goto yy_newstate;

		/*
		** get globals into registers.
		** either we just started, or we just finished a reduction
		*/
	yystack:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;

		/*
		** top of for (;;) loop while no reductions done
		*/
	yy_stack:
		/*
		** put a state and value onto the stacks
		*/
#if YYDEBUG
		/*
		** if debugging, look up token value in list of value vs.
		** name pairs.  0 and negative (-1) are special values.
		** Note: linear search is used since time is not a real
		** consideration while debugging.
		*/
		if ( yydebug )
		{
			register int yy_i;

			printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ YYMAXDEPTH ] )	/* room on stack? */
		{
			yyerror( "yacc stack overflow" );
			YYABORT;
		}
		*yy_ps = yy_state;
		*++yy_pv = yyval;

		/*
		** we have a new state - find out what to do
		*/
	yy_newstate:
		if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
			goto yydefault;		/* simple state */
#if YYDEBUG
		/*
		** if debugging, need to mark whether new token grabbed
		*/
		yytmp = yychar < 0;
#endif
		if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
			yychar = 0;		/* reached EOF */
#if YYDEBUG
		if ( yydebug && yytmp )
		{
			register int yy_i;

			printf( "Received token " );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
			goto yydefault;
		if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )	/*valid shift*/
		{
			yychar = -1;
			yyval = yylval;
			yy_state = yy_n;
			if ( yyerrflag > 0 )
				yyerrflag--;
			goto yy_stack;
		}

	yydefault:
		if ( ( yy_n = yydef[ yy_state ] ) == -2 )
		{
#if YYDEBUG
			yytmp = yychar < 0;
#endif
			if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
				yychar = 0;		/* reached EOF */
#if YYDEBUG
			if ( yydebug && yytmp )
			{
				register int yy_i;

				printf( "Received token " );
				if ( yychar == 0 )
					printf( "end-of-file\n" );
				else if ( yychar < 0 )
					printf( "-none-\n" );
				else
				{
					for ( yy_i = 0;
						yytoks[yy_i].t_val >= 0;
						yy_i++ )
					{
						if ( yytoks[yy_i].t_val
							== yychar )
						{
							break;
						}
					}
					printf( "%s\n", yytoks[yy_i].t_name );
				}
			}
#endif /* YYDEBUG */
			/*
			** look through exception table
			*/
			{
				register int *yyxi = yyexca;

				while ( ( *yyxi != -1 ) ||
					( yyxi[1] != yy_state ) )
				{
					yyxi += 2;
				}
				while ( ( *(yyxi += 2) >= 0 ) &&
					( *yyxi != yychar ) )
					;
				if ( ( yy_n = yyxi[1] ) < 0 )
					YYACCEPT;
			}
		}

		/*
		** check for syntax error
		*/
		if ( yy_n == 0 )	/* have an error */
		{
			/* no worry about speed here! */
			switch ( yyerrflag )
			{
			case 0:		/* new error */
				yyerror( "syntax error" );
				goto skip_init;
			yyerrlab:
				/*
				** get globals into registers.
				** we have a user generated syntax type error
				*/
				yy_pv = yypv;
				yy_ps = yyps;
				yy_state = yystate;
				yynerrs++;
			skip_init:
			case 1:
			case 2:		/* incompletely recovered error */
					/* try again... */
				yyerrflag = 3;
				/*
				** find state where "error" is a legal
				** shift action
				*/
				while ( yy_ps >= yys )
				{
					yy_n = yypact[ *yy_ps ] + YYERRCODE;
					if ( yy_n >= 0 && yy_n < YYLAST &&
						yychk[yyact[yy_n]] == YYERRCODE)					{
						/*
						** simulate shift of "error"
						*/
						yy_state = yyact[ yy_n ];
						goto yy_stack;
					}
					/*
					** current state has no shift on
					** "error", pop stack
					*/
#if YYDEBUG
#	define _POP_ "Error recovery pops state %d, uncovers state %d\n"
					if ( yydebug )
						printf( _POP_, *yy_ps,
							yy_ps[-1] );
#	undef _POP_
#endif
					yy_ps--;
					yy_pv--;
				}
				/*
				** there is no state on stack with "error" as
				** a valid shift.  give up.
				*/
				YYABORT;
			case 3:		/* no shift yet; eat a token */
#if YYDEBUG
				/*
				** if debugging, look up token in list of
				** pairs.  0 and negative shouldn't occur,
				** but since timing doesn't matter when
				** debugging, it doesn't hurt to leave the
				** tests here.
				*/
				if ( yydebug )
				{
					register int yy_i;

					printf( "Error recovery discards " );
					if ( yychar == 0 )
						printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						printf( "token -none-\n" );
					else
					{
						for ( yy_i = 0;
							yytoks[yy_i].t_val >= 0;
							yy_i++ )
						{
							if ( yytoks[yy_i].t_val
								== yychar )
							{
								break;
							}
						}
						printf( "token %s\n",
							yytoks[yy_i].t_name );
					}
				}
#endif /* YYDEBUG */
				if ( yychar == 0 )	/* reached EOF. quit */
					YYABORT;
				yychar = -1;
				goto yy_newstate;
			}
		}/* end if ( yy_n == 0 ) */
		/*
		** reduction by production yy_n
		** put stack tops, etc. so things right after switch
		*/
#if YYDEBUG
		/*
		** if debugging, print the string that is the user's
		** specification of the reduction which is just about
		** to be done.
		*/
		if ( yydebug )
			printf( "Reduce by (%d) \"%s\"\n",
				yy_n, yyreds[ yy_n ] );
#endif
		yytmp = yy_n;			/* value to switch over */
		yypvt = yy_pv;			/* $vars top of value stack */
		/*
		** Look in goto table for next state
		** Sorry about using yy_state here as temporary
		** register variable, but why not, if it works...
		** If yyr2[ yy_n ] doesn't have the low order bit
		** set, then there is no action to be done for
		** this reduction.  So, no saving & unsaving of
		** registers done.  The only difference between the
		** code just after the if and the body of the if is
		** the goto yy_stack in the body.  This way the test
		** can be made before the choice of what to do is needed.
		*/
		{
			/* length of production doubled with extra bit */
			register int yy_len = yyr2[ yy_n ];

			if ( !( yy_len & 01 ) )
			{
				yy_len >>= 1;
				yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
				yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
					*( yy_ps -= yy_len ) + 1;
				if ( yy_state >= YYLAST ||
					yychk[ yy_state =
					yyact[ yy_state ] ] != -yy_n )
				{
					yy_state = yyact[ yypgo[ yy_n ] ];
				}
				goto yy_stack;
			}
			yy_len >>= 1;
			yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
			yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
				*( yy_ps -= yy_len ) + 1;
			if ( yy_state >= YYLAST ||
				yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
			{
				yy_state = yyact[ yypgo[ yy_n ] ];
			}
		}
					/* save until reenter driver code */
		yystate = yy_state;
		yyps = yy_ps;
		yypv = yy_pv;
	}
	/*
	** code supplied by user is placed in this switch
	*/
	switch( yytmp )
	{
		
case 1:
# line 140 "gram.in"
{ graal = PARSEOF; } break;
case 2:
# line 142 "gram.in"
{ graal = PARSOPT; } break;
case 3:
# line 144 "gram.in"
{ graal = PARSDCL; doinits(yypvt[-1].pval);  frchain( & yypvt[-1].pval); } break;
case 4:
# line 146 "gram.in"
{ endproc(); graal = PARSPROC; } break;
case 5:
# line 148 "gram.in"
{ graal = PARSDEF; } break;
case 6:
# line 150 "gram.in"
{ graal = PARSERR; } break;
case 7:
# line 152 "gram.in"
{ graal = PARSERR;
		  errmess("Syntax error", "", "");
		} break;
case 8:
# line 157 "gram.in"
{ eofneed = 1; } break;
case 9:
# line 160 "gram.in"
{ if(!dclsect)
			warn("declaration amid executables");
		    yyval.pval = bgnexec();
		   TEST fprintf(diagfile,"stat: dcl\n");
		  doinits(yypvt[-1].pval); frchain( & yypvt[-1].pval); } break;
case 10:
# line 166 "gram.in"
{ if(dclsect && yypvt[-1].pval->tag!=TSTFUNCT)
			dclsect = 0;
		    TEST fprintf(diagfile, "stat: exec\n"); } break;
case 11:
# line 170 "gram.in"
{ yyval.pval = bgnexec(); } break;
case 12:
# line 172 "gram.in"
{ yyerrok;
		  errmess("Syntax error", "", "");
		  yyval.pval = bgnexec();
		} break;
case 13:
# line 179 "gram.in"
{ yyval.pval = bgnexec(); } break;
case 14:
# line 180 "gram.in"
{ thisexec->copylab = 1; } break;
case 15:
# line 181 "gram.in"
{ yyval.pval = addexec(); thisexec->copylab = 0; } break;
case 16:
# line 185 "gram.in"
{ procname = 0; thisargs = 0;
		  if(procclass == 0) procclass = PRMAIN;
		  goto proctype;
		} break;
case 17:
# line 190 "gram.in"
{ thisargs = 0; goto proctype; } break;
case 18:
# line 192 "gram.in"
{ thisargs = 0; goto proctype; } break;
case 19:
# line 194 "gram.in"
{ thisargs = yypvt[-1].pval;
	proctype:
		if(procattrs)
			if(procname == 0)
				dclerr("attributes on unnamed procedure", "");
			else	{
				attvars(procattrs, mkchain(procname,CHNULL));
				procclass = PRFUNCT;
				}
		fprintf(diagfile, "Procedure %s:\n", procnm() );
		if(verbose)
			fprintf(diagfile, "    Pass 1\n");
		} break;
case 20:
# line 210 "gram.in"
{ procname = mkvar(yypvt[-0].pval);
		  extname(procname);
		} break;
case 21:
# line 216 "gram.in"
{ procattrs = 0; } break;
case 22:
# line 218 "gram.in"
{ procattrs = yypvt[-1].pval;
		  if(procclass == 0) procclass = PRFUNCT;
		} break;
case 23:
# line 224 "gram.in"
{ yyval.pval = bgnproc(); procclass = 0; } break;
case 24:
# line 226 "gram.in"
{ yyval.pval = bgnproc(); procclass = PRBLOCK; } break;
case 25:
# line 230 "gram.in"
{ yyval.pval = mkchain(yypvt[-0].pval,CHNULL); } break;
case 26:
# line 232 "gram.in"
{ hookup(yypvt[-2].pval, mkchain(yypvt[-0].pval,CHNULL) ); } break;
case 27:
# line 236 "gram.in"
{ if(yypvt[-0].pval->vclass == CLUNDEFINED)
			yypvt[-0].pval->vclass = CLARG;
		  else dclerr("argument already used", yypvt[-0].pval->sthead->namep);
		} break;
case 28:
# line 242 "gram.in"
{ optneed = 0; } break;
case 29:
# line 246 "gram.in"
{ if(blklevel > 0)
			{
			execerr("Option statement inside procedure", "");
			execerr("procedure %s terminated prematurely", procnm());
			endproc();
			}
		  optneed = 1;
		  } break;
case 33:
# line 262 "gram.in"
{ setopt(yypvt[-0].pval,CNULL); cfree(yypvt[-0].pval); } break;
case 34:
# line 264 "gram.in"
{ setopt(yypvt[-2].pval,yypvt[-0].pval); cfree(yypvt[-2].pval); cfree(yypvt[-0].pval); } break;
case 35:
# line 266 "gram.in"
{ setopt(yypvt[-2].pval,yypvt[-0].pval->leftp); cfree(yypvt[-2].pval); cfree(yypvt[-0].pval); } break;
case 36:
# line 270 "gram.in"
{ defneed = 1; } break;
case 37:
# line 274 "gram.in"
{ if(thisctl->subtype != STPROC)
			execerr("control stack not empty upon END", "");
		  exnull();
		  popctl();
		} break;
case 38:
# line 282 "gram.in"
{ igeol=1; /* continue past newlines  */ } break;
case 41:
# line 289 "gram.in"
{ yyval.pval = hookup(yypvt[-2].pval,yypvt[-0].pval); } break;
case 44:
# line 297 "gram.in"
{ attvars(yypvt[-1].pval,yypvt[-0].pval); yyval.pval = yypvt[-0].pval; } break;
case 45:
# line 299 "gram.in"
{ attvars(yypvt[-3].pval,yypvt[-1].pval); yyval.pval = yypvt[-1].pval; } break;
case 46:
# line 301 "gram.in"
{ yyval.pval = 0; } break;
case 47:
# line 303 "gram.in"
{ yyval.pval = 0; } break;
case 48:
# line 305 "gram.in"
{ yyval.pval = 0; } break;
case 49:
# line 307 "gram.in"
{ mkequiv(yypvt[-0].pval); yyval.pval = 0; } break;
case 52:
# line 313 "gram.in"
{ yyval.pval = hookup(yypvt[-2].pval,yypvt[-0].pval); } break;
case 55:
# line 320 "gram.in"
{ininit = YES; } break;
case 56:
# line 321 "gram.in"
 { ininit = NO;  mkinit(yypvt[-3].pval,yypvt[-0].pval);  frexpr(yypvt[-3].pval); } break;
case 59:
# line 329 "gram.in"
{ setimpl(imptype, 'a', 'z'); } break;
case 61:
# line 334 "gram.in"
{ imptype = yypvt[-0].pval->attype; cfree(yypvt[-0].pval); } break;
case 64:
# line 342 "gram.in"
{ setimpl(imptype, yypvt[-0].ival, yypvt[-0].ival); } break;
case 65:
# line 344 "gram.in"
{ setimpl(imptype, yypvt[-2].ival, yypvt[-0].ival); } break;
case 68:
# line 352 "gram.in"
{ mkequiv(yypvt[-1].pval); } break;
case 69:
# line 356 "gram.in"
{ yyval.pval = mkchain(yypvt[-2].pval, mkchain(yypvt[-0].pval,CHNULL)); } break;
case 70:
# line 358 "gram.in"
{ yyval.pval = hookup(yypvt[-2].pval, mkchain(yypvt[-0].pval,CHNULL)); } break;
case 72:
# line 362 "gram.in"
{ attatt(yypvt[-1].pval,yypvt[-0].pval); } break;
case 73:
# line 365 "gram.in"
{ yypvt[-1].pval->atdim = yypvt[-0].pval; } break;
case 74:
# line 366 "gram.in"
{ yyval.pval = ALLOC(atblock); yyval.pval->atdim = yypvt[-0].pval; } break;
case 75:
# line 369 "gram.in"
{ yyval.pval = 0; } break;
case 77:
# line 373 "gram.in"
{ inbound = 1; } break;
case 78:
# line 374 "gram.in"
{ inbound = 0;  yyval.pval = arrays = mkchain(yypvt[-1].pval,arrays); } break;
case 80:
# line 378 "gram.in"
{ hookup(yypvt[-2].pval,yypvt[-0].pval); } break;
case 81:
# line 382 "gram.in"
{
		yyval.pval = ALLOC(dimblock);
		yyval.pval->lowerb = 0;
		yyval.pval->upperb = yypvt[-0].pval;
		} break;
case 82:
# line 388 "gram.in"
{
		yyval.pval = ALLOC(dimblock);
		yyval.pval->lowerb = yypvt[-2].pval;
		yyval.pval->upperb = yypvt[-0].pval;
		} break;
case 84:
# line 396 "gram.in"
{ yyval.pval = 0; } break;
case 85:
# line 399 "gram.in"
{ yyval.pval = 0; } break;
case 88:
# line 404 "gram.in"
{ hookup(yypvt[-2].pval,yypvt[-0].pval); } break;
case 89:
# line 408 "gram.in"
{
		if(yypvt[-0].pval!=0)
			if(yypvt[-1].pval->vdim==0)
				yypvt[-1].pval->vdim = yypvt[-0].pval;
			else if(!eqdim(yypvt[-0].pval,yypvt[-1].pval->vdim))
				dclerr("multiple dimension", yypvt[-1].pval->namep);
		yyval.pval = mkchain(yypvt[-1].pval,CHNULL);
		} break;
case 90:
# line 416 "gram.in"
{ ininit = YES; } break;
case 91:
# line 417 "gram.in"
{
		ininit = NO;
		if(yypvt[-2].ival!=OPASGN)
			dclerr("illegal initialization operator", yypvt[-4].pval->sthead->namep);
		if(yypvt[-3].pval!=0)
			if(yypvt[-4].pval->vdim==0)
				yypvt[-4].pval->vdim = yypvt[-3].pval;
			else if(!eqdim(yypvt[-3].pval,yypvt[-4].pval->vdim))
				dclerr("multiple dimension", yypvt[-4].pval->sthead->namep);
		if(yypvt[-0].pval!=0 && yypvt[-4].pval->vinit!=0)
			dclerr("multiple initialization", yypvt[-4].pval->sthead->namep);
		yypvt[-4].pval->vinit = yypvt[-0].pval;
		yyval.pval = mkchain(yypvt[-4].pval,CHNULL);
		} break;
case 92:
# line 434 "gram.in"
{ yyval.pval = mkvar(yypvt[-0].pval); } break;
case 94:
# line 439 "gram.in"
{ attatt(yypvt[-1].pval,yypvt[-0].pval); } break;
case 96:
# line 444 "gram.in"
{ yyval.pval = ALLOC(atblock); yyval.pval->atdim = yypvt[-0].pval; } break;
case 97:
# line 448 "gram.in"
{
		yyval.pval = ALLOC(atblock);
		if(yypvt[-0].ival == CLEXT)
			yyval.pval->atext = 1;
		yyval.pval->atclass = yypvt[-0].ival;
		} break;
case 98:
# line 455 "gram.in"
{
		yyval.pval = ALLOC(atblock);
		yyval.pval->atclass = CLCOMMON;
		yyval.pval->atcommon = yypvt[-1].pval;
		} break;
case 99:
# line 461 "gram.in"
{ yyval.pval = ALLOC(atblock); yyval.pval->attype = yypvt[-0].ival; } break;
case 100:
# line 463 "gram.in"
{ yyval.pval = ALLOC(atblock); yyval.pval->attype = TYCHAR; yyval.pval->attypep = yypvt[-1].pval; } break;
case 101:
# line 465 "gram.in"
{ yyval.pval = ALLOC(atblock); yyval.pval->attype = TYFIELD;
		  yyval.pval->attypep = mkfield(yypvt[-1].pval); } break;
case 102:
# line 468 "gram.in"
{ yyval.pval = ALLOC(atblock); yyval.pval->attype = TYSTRUCT;
		  yyval.pval->attypep = yypvt[-0].pval; } break;
case 103:
# line 471 "gram.in"
{ yyval.pval = ALLOC(atblock); yyval.pval->atprec = yypvt[-0].ival; } break;
case 104:
# line 474 "gram.in"
{ yyval.ival = CLAUTO;
			  fprintf(diagfile,"AUTOMATIC not yet implemented\n"); } break;
case 105:
# line 476 "gram.in"
{ yyval.ival = CLSTAT; } break;
case 106:
# line 477 "gram.in"
{ yyval.ival = CLSTAT; } break;
case 107:
# line 478 "gram.in"
{ yyval.ival = CLVALUE;
			  fprintf(diagfile, "VALUE not yet implemented\n");  } break;
case 108:
# line 480 "gram.in"
{ yyval.ival = CLEXT; } break;
case 109:
# line 484 "gram.in"
{ yyval.pval = yypvt[-1].pval; } break;
case 110:
# line 486 "gram.in"
{ yyval.pval = yypvt[-1].pval; } break;
case 111:
# line 489 "gram.in"
{ comneed = 1; } break;
case 112:
# line 492 "gram.in"
{ yyval.pval = mkcomm(""); } break;
case 114:
# line 496 "gram.in"
{ yyval.ival = TYINT; } break;
case 115:
# line 497 "gram.in"
{ yyval.ival = TYREAL; } break;
case 116:
# line 498 "gram.in"
{ yyval.ival = TYCOMPLEX; } break;
case 117:
# line 499 "gram.in"
{ yyval.ival = TYLOG; } break;
case 118:
# line 501 "gram.in"
{ yyval.ival = TYLREAL; /* holdover from Fortran */ } break;
case 119:
# line 503 "gram.in"
{ yyval.ival = TYLREAL; /* holdover from Fortran */ } break;
case 120:
# line 507 "gram.in"
{ yyval.pval = yypvt[-0].pval->varp; } break;
case 121:
# line 509 "gram.in"
{ yyval.pval = mkstruct(yypvt[-2].pval,yypvt[-0].pval); } break;
case 122:
# line 511 "gram.in"
{ yyval.pval = mkstruct(PNULL,yypvt[-0].pval); } break;
case 123:
# line 515 "gram.in"
{ if(yypvt[-0].pval->varp && yypvt[-0].pval->varp->blklevel<blklevel)
			hide(yypvt[-0].pval);
		  yypvt[-0].pval->tag = TSTRUCT;
		} break;
case 124:
# line 520 "gram.in"
{ if(yypvt[-0].pval->varp)
			if(yypvt[-0].pval->varp->blklevel<blklevel)
				hide(yypvt[-0].pval);
			else dclerr("multiple declaration for type %s", yypvt[-0].pval->namep);
		} break;
case 125:
# line 527 "gram.in"
{ ++instruct; } break;
case 126:
# line 527 "gram.in"
{ --instruct; } break;
case 127:
# line 528 "gram.in"
{ yyval.pval = yypvt[-3].pval; prevv = -1; } break;
case 130:
# line 535 "gram.in"
{ yyval.ival = 1; } break;
case 131:
# line 536 "gram.in"
{ yyval.ival = 0; } break;
case 132:
# line 541 "gram.in"
{ if(yypvt[-0].pval->tag == TCALL)
			yypvt[-0].pval = funcinv(yypvt[-0].pval);
		  if(yypvt[-0].pval->vtype==TYUNDEFINED && yypvt[-0].pval->vext==0)
			impldecl(yypvt[-0].pval);
		  else if(yypvt[-0].pval->tag==TNAME && yypvt[-0].pval->vdcldone==0
			  && yypvt[-0].pval->vext==0 && !inbound)
				dclit(yypvt[-0].pval);
		  if(yypvt[-0].pval->vtype==TYFIELD)
			yyval.pval = extrfield(yypvt[-0].pval);
		} break;
case 134:
# line 553 "gram.in"
{ yyval.pval = mkconst(TYLOG, (yypvt[-0].ival == TRUE ? ".true." : ".false.") ); } break;
case 135:
# line 555 "gram.in"
{ yyval.pval = typexpr(yypvt[-1].pval,yypvt[-0].pval); } break;
case 138:
# line 559 "gram.in"
{ if( !ininit && yypvt[-0].pval->tag== TLIST)
			yyval.pval = compconst(yypvt[-0].pval); 
		  else yypvt[-0].pval->needpar = 1; } break;
case 139:
# line 563 "gram.in"
{ yyval.pval = mknode(TAROP,yypvt[-1].ival,yypvt[-2].pval,yypvt[-0].pval); } break;
case 140:
# line 565 "gram.in"
{ yyval.pval = mknode(TAROP,yypvt[-1].ival,yypvt[-2].pval,yypvt[-0].pval); } break;
case 141:
# line 567 "gram.in"
{ yyval.pval = mknode(TAROP,yypvt[-1].ival,yypvt[-2].pval,yypvt[-0].pval); } break;
case 142:
# line 569 "gram.in"
{ if(yypvt[-1].ival==OPMINUS)
			yyval.pval = mknode(TNEGOP,OPMINUS, yypvt[-0].pval, PNULL);
		  else	yyval.pval = yypvt[-0].pval;  } break;
case 143:
# line 573 "gram.in"
{ yyval.pval =  mknode(TASGNOP,yypvt[-1].ival,yypvt[-0].pval,mkint(1)); } break;
case 144:
# line 575 "gram.in"
{ yyval.pval = mknode(TRELOP,yypvt[-1].ival,yypvt[-2].pval,yypvt[-0].pval); } break;
case 145:
# line 577 "gram.in"
{ yyval.pval = mknode(TLOGOP,yypvt[-1].ival,yypvt[-2].pval,yypvt[-0].pval); } break;
case 146:
# line 579 "gram.in"
{ yyval.pval = mknode(TLOGOP,yypvt[-1].ival,yypvt[-2].pval,yypvt[-0].pval); } break;
case 147:
# line 581 "gram.in"
{ yyval.pval = mknode(TNOTOP,yypvt[-1].ival,yypvt[-0].pval,PNULL); } break;
case 148:
# line 583 "gram.in"
{ if(yypvt[-2].pval->tag == TCALL)
			{
			exprerr("may not assign to a function", CNULL);
			yyval.pval = errnode();
			}
		  else
			yyval.pval = mknode(TASGNOP,yypvt[-1].ival,yypvt[-2].pval,yypvt[-0].pval);
		} break;
case 149:
# line 592 "gram.in"
{ yyval.pval = mknode(TREPOP,0,yypvt[-2].pval,yypvt[-0].pval); } break;
case 151:
# line 595 "gram.in"
{ yyval.pval = errnode(); } break;
case 152:
# line 599 "gram.in"
{ if(yypvt[-0].pval->tag==TNAME && yypvt[-0].pval->vdcldone==0 &&
			yypvt[-0].pval->vsubs==0 && yypvt[-0].pval->vext==0 && !inbound)
				dclit(yypvt[-0].pval);
		} break;
case 154:
# line 607 "gram.in"
{
		if(yypvt[-0].pval->tag!=TLIST)
			yypvt[-0].pval = mknode(TLIST,0, mkchain(yypvt[-0].pval,CHNULL), PNULL);
		if(yypvt[-1].pval->vdim)
			{
			if(yypvt[-1].pval->vdcldone==0 && yypvt[-1].pval->vext==0)
				dclit(yypvt[-1].pval);
			yyval.pval = subscript(yypvt[-1].pval,yypvt[-0].pval);
			}
		else	yyval.pval = mkcall(yypvt[-1].pval,yypvt[-0].pval);
		} break;
case 155:
# line 619 "gram.in"
{ yyval.pval = strucelt(yypvt[-2].pval,yypvt[-0].pval); } break;
case 156:
# line 621 "gram.in"
{ if(yypvt[-0].pval->tag != TLIST)
			yypvt[-0].pval = mknode(TLIST,0, mkchain(yypvt[-0].pval,CHNULL), PNULL);
		  yyval.pval = subscript(strucelt(yypvt[-3].pval,yypvt[-1].pval), yypvt[-0].pval);
		} break;
case 157:
# line 626 "gram.in"
{ yyval.pval = mkarrow(yypvt[-2].pval,yypvt[-0].pval); } break;
case 158:
# line 630 "gram.in"
{ if(yypvt[-0].pval->varp == 0) mkvar(yypvt[-0].pval);
		  if(inbound)
			  yypvt[-0].pval->varp->vadjdim = 1;
		  yyval.pval = cpexpr(yypvt[-0].pval->varp); } break;
case 159:
# line 637 "gram.in"
{ yyval.pval = mknode(TLIST, 0, PNULL, PNULL); } break;
case 160:
# line 639 "gram.in"
{ yyval.pval = yypvt[-1].pval; } break;
case 161:
# line 641 "gram.in"
{ yyval.pval = mknode(TLIST,0,yypvt[-1].pval,PNULL); } break;
case 162:
# line 645 "gram.in"
{ yyval.pval = mkchain(yypvt[-2].pval, mkchain(yypvt[-0].pval, CHNULL) ); } break;
case 163:
# line 647 "gram.in"
{ hookup(yypvt[-2].pval, mkchain(yypvt[-0].pval,CHNULL) ); } break;
case 164:
# line 651 "gram.in"
{ yyval.pval = esizeof(yypvt[-1].pval->vtype, yypvt[-1].pval->vtypep, yypvt[-1].pval->vdim);
		  frexpr(yypvt[-1].pval); } break;
case 165:
# line 654 "gram.in"
{ if(yypvt[-1].pval->attype==TYREAL && yypvt[-1].pval->atprec)
			yypvt[-1].pval->attype = TYLREAL;
		  yyval.pval = esizeof(yypvt[-1].pval->attype, yypvt[-1].pval->attypep, yypvt[-1].pval->atdim);
		  cfree(yypvt[-1].pval);
		} break;
case 166:
# line 660 "gram.in"
{ yyval.pval = mkint(tailor.ftnsize[FTNINT]/tailor.ftnchwd); } break;
case 167:
# line 664 "gram.in"
{ yyval.pval = elenof(yypvt[-1].pval->vtype, yypvt[-1].pval->vtypep, yypvt[-1].pval->vdim);
		  frexpr(yypvt[-1].pval); } break;
case 168:
# line 667 "gram.in"
{ yyval.pval = elenof(yypvt[-1].pval->attype, yypvt[-1].pval->attypep, yypvt[-1].pval->atdim);
		  cfree(yypvt[-1].pval);
		} break;
case 169:
# line 671 "gram.in"
{ yyval.pval = mkint(1); } break;
case 171:
# line 676 "gram.in"
{ yyval.ival = yypvt[-1].ival; } break;
case 174:
# line 685 "gram.in"
{ TEST fprintf(diagfile, "exec done\n"); } break;
case 175:
# line 689 "gram.in"
{ yyval.pval = bgnexec();  if(ncases > 0) ncases = 0; } break;
case 176:
# line 693 "gram.in"
{
		if(yypvt[-2].pval->tag==TCALL)
			{
			dclerr("no statement functions in EFL",
				yypvt[-2].pval->sthead->namep);
			frexpr(yypvt[-2].pval);
			frexpr(yypvt[-0].pval);
			}
		else exasgn(yypvt[-2].pval,yypvt[-1].ival,yypvt[-0].pval);
		} break;
case 177:
# line 704 "gram.in"
{ exasgn(yypvt[-0].pval, yypvt[-1].ival, mkint(1) ); } break;
case 178:
# line 706 "gram.in"
{ excall(yypvt[-0].pval); } break;
case 179:
# line 708 "gram.in"
{ excall(yypvt[-0].pval); } break;
case 181:
# line 711 "gram.in"
{ TEST fprintf(diagfile, "exec: { stats }\n");
		  addexec(); } break;
case 182:
# line 714 "gram.in"
{ thisexec->labeled = 1; } break;
case 183:
# line 716 "gram.in"
{ thisexec->uniffable = 1;  popctl(); } break;
case 184:
# line 718 "gram.in"
{ thisexec->brnchend = 1; } break;
case 185:
# line 720 "gram.in"
{ exio(yypvt[-0].pval, 0); } break;
case 186:
# line 722 "gram.in"
{ exnull(); } break;
case 187:
# line 724 "gram.in"
{
		exnull();
		putsii(ICCOMMENT, yypvt[-0].pval);
		cfree(yypvt[-0].pval);
		exnull();
		} break;
case 188:
# line 733 "gram.in"
{ TEST fprintf(diagfile, "exec:empty\n"); } break;
case 189:
# line 735 "gram.in"
{ TEST fprintf(diagfile, "exec: continue\n"); } break;
case 190:
# line 739 "gram.in"
{
		thisexec->copylab = 1;
		++blklevel;
		dclsect = 1;
		ndecl[blklevel] = 0;
		nhid [blklevel] = 0;
		} break;
case 191:
# line 749 "gram.in"
{
		if(ndecl[blklevel]) unhide();
		--blklevel;
		dclsect = 0;
		} break;
case 192:
# line 757 "gram.in"
{ mklabel(yypvt[-2].pval,YES); } break;
case 193:
# line 759 "gram.in"
{ mklabel(mkilab(yypvt[-1].pval),YES); } break;
case 194:
# line 761 "gram.in"
{ mklabel(mkilab(yypvt[-3].pval),YES); } break;
case 195:
# line 763 "gram.in"
{ brkcase(); mkcase(PNULL,1); } break;
case 196:
# line 764 "gram.in"
{ brkcase(); } break;
case 198:
# line 768 "gram.in"
{ mkcase(yypvt[-0].pval,1); } break;
case 199:
# line 770 "gram.in"
{ mkcase(yypvt[-0].pval,1); } break;
case 200:
# line 774 "gram.in"
{ TEST fprintf(diagfile, "if-then-else\n");
		  i = yypvt[-5].pval->brnchend & yypvt[-0].pval->brnchend;
		  addexec();
		  yyval.pval = addexec();
		  thisexec->brnchend = i;
		  TEST fprintf(diagfile, "exec: if(expr) exec else exec\n"); } break;
case 201:
# line 781 "gram.in"
{ TEST fprintf(diagfile, "if-then\n");
		  pushlex = 1;
		  yyclearin;
		  yyval.pval = ifthen();
		  TEST fprintf(diagfile, "exec: if(expr) exec\n"); } break;
case 202:
# line 787 "gram.in"
{ TEST fprintf(diagfile, "repeat done\n"); } break;
case 203:
# line 789 "gram.in"
{ TEST fprintf(diagfile, "exec: control exec\n"); yyval.pval = addexec(); } break;
case 204:
# line 793 "gram.in"
{ pushctl(STIF,yypvt[-1].pval); } break;
case 205:
# line 797 "gram.in"
{
		if(thisctl->breaklab == 0)
			thisctl->breaklab = nextlab();
		/* if(thisexec->prevexec->brnchend == 0) */
			exgoto(thisctl->breaklab);
		exlab( indifs[thisctl->indifn] = nextlab() );
		} break;
case 206:
# line 807 "gram.in"
{ pushctl(STREPEAT, PNULL); } break;
case 207:
# line 811 "gram.in"
{ pushlex = 1;
		  yyclearin;
		  yyval.pval = addexec();
		  exgoto(thisctl->xlab);
		  TEST fprintf(diagfile, " no until\n"); } break;
case 208:
# line 817 "gram.in"
{ yyval.pval = addexec();
		  exnull();
		  exlab( indifs[thisctl->indifn] = nextlab() );
		  exifgo(mknode(TNOTOP,OPNOT,yypvt[-1].pval,PNULL), thisctl->xlab);
		  TEST fprintf(diagfile, "until\n");
		} break;
case 209:
# line 826 "gram.in"
{ exretn(PNULL); } break;
case 210:
# line 828 "gram.in"
{ exretn(yypvt[-0].pval); } break;
case 212:
# line 831 "gram.in"
{ exgoto(yypvt[-0].ival); } break;
case 213:
# line 833 "gram.in"
{ if( !equals(yypvt[-1].pval->namep, "to") )
			execerr("go %s ??\n", yypvt[-1].pval->namep);
		  else	 exgoto(yypvt[-0].ival);
		} break;
case 214:
# line 838 "gram.in"
{ excompgoto(yypvt[-1].pval, yypvt[-0].pval); } break;
case 215:
# line 840 "gram.in"
{ if(!equals(yypvt[-2].pval->namep, "to") )
			execerr("go %s ??\n", yypvt[-2].pval->namep);
		  else excompgoto(yypvt[-1].pval, yypvt[-0].pval);
		} break;
case 216:
# line 847 "gram.in"
{ yyval.pval = yypvt[-1].pval; } break;
case 217:
# line 852 "gram.in"
{ yyval.pval = mkchain(yypvt[-0].ival,CHNULL); } break;
case 218:
# line 854 "gram.in"
{ yyval.pval = hookup(yypvt[-2].pval, mkchain(yypvt[-0].ival,CHNULL) ); } break;
case 220:
# line 859 "gram.in"
{ yyval.pval = yypvt[-0].pval; } break;
case 221:
# line 864 "gram.in"
{ yyval.ival = mklabel(yypvt[-0].pval,NO); } break;
case 222:
# line 866 "gram.in"
{ yyval.ival = mklabel(mkilab(yypvt[-0].pval),NO); } break;
case 223:
# line 868 "gram.in"
{ yyval.ival = mkcase(yypvt[-0].pval,0); } break;
case 224:
# line 870 "gram.in"
{ yyval.ival = mkcase(PNULL,0); } break;
case 225:
# line 874 "gram.in"
{ exbrk(yypvt[-0].ival, PNULL, 0); } break;
case 226:
# line 876 "gram.in"
{ exbrk(yypvt[-1].ival, yypvt[-0].pval, 0); } break;
case 227:
# line 878 "gram.in"
{ exbrk(yypvt[-1].ival, PNULL, yypvt[-0].ival); } break;
case 228:
# line 880 "gram.in"
{ exbrk(yypvt[-2].ival,yypvt[-1].pval,yypvt[-0].ival); } break;
case 229:
# line 882 "gram.in"
{ exbrk(yypvt[-2].ival,yypvt[-0].pval,yypvt[-1].ival); } break;
case 230:
# line 885 "gram.in"
{ yyval.ival = 1; } break;
case 231:
# line 886 "gram.in"
{ yyval.ival = 0; } break;
case 232:
# line 887 "gram.in"
{ yyval.ival = 0; } break;
case 233:
# line 890 "gram.in"
{ yyval.ival = STWHILE; } break;
case 234:
# line 891 "gram.in"
{ yyval.ival = STFOR; } break;
case 235:
# line 892 "gram.in"
{ yyval.ival = STDO; } break;
case 236:
# line 893 "gram.in"
{ yyval.ival = STREPEAT; } break;
case 237:
# line 894 "gram.in"
{ yyval.ival = STSWITCH; } break;
case 238:
# line 895 "gram.in"
{ yyval.ival = STPROC; } break;
case 239:
# line 899 "gram.in"
{ pushctl(STWHILE, yypvt[-1].pval);
		    TEST fprintf(diagfile, "while(expr)\n"); } break;
case 240:
# line 902 "gram.in"
{ exlab(thisctl->xlab);
		  if(yypvt[-1].pval)
			exifgo(mknode(TNOTOP,OPNOT,yypvt[-1].pval,PNULL),
				thisctl->breaklab = nextlab() );
		  else exnull();
		  TEST fprintf(diagfile, "for (forlist)\n"); } break;
case 241:
# line 909 "gram.in"
{ pushctl(STSWITCH,  simple(LVAL,yypvt[-1].pval));
		  TEST fprintf(diagfile, "switch (expr)\n"); } break;
case 242:
# line 912 "gram.in"
{ pushctl(STDO, yypvt[-0].pval);
		  TEST fprintf(diagfile, "do loop\n"); } break;
case 243:
# line 917 "gram.in"
{ yyval.pval = doloop(yypvt[-2].pval, PNULL, PNULL); } break;
case 244:
# line 919 "gram.in"
{ yyval.pval = doloop(yypvt[-1].pval, PNULL, PNULL); } break;
case 245:
# line 921 "gram.in"
{ yyval.pval = doloop(yypvt[-4].pval, yypvt[-2].pval, PNULL); } break;
case 246:
# line 923 "gram.in"
{ yyval.pval = doloop(yypvt[-3].pval, yypvt[-1].pval, PNULL); } break;
case 247:
# line 925 "gram.in"
{ yyval.pval = doloop(yypvt[-6].pval,yypvt[-4].pval,yypvt[-2].pval); } break;
case 248:
# line 927 "gram.in"
{ yyval.pval = doloop(yypvt[-5].pval,yypvt[-3].pval,yypvt[-1].pval); } break;
case 249:
# line 931 "gram.in"
{ pushctl(STFOR, PNULL); } break;
case 250:
# line 935 "gram.in"
{ exgoto(thisctl->xlab);
		  exlab(thisctl->nextlab);
		  addexec();
		  } break;
case 251:
# line 940 "gram.in"
{ exgoto(thisctl->xlab);
		  exlab(thisctl->nextlab);
		  addexec();
		  } break;
case 253:
# line 950 "gram.in"
{ addexec(); } break;
case 254:
# line 952 "gram.in"
{ addexec(); } break;
case 255:
# line 955 "gram.in"
{ yyval.pval = 0; } break;
case 257:
# line 960 "gram.in"
{ if(dbgopt) ++dbglevel; } break;
case 258:
# line 964 "gram.in"
{ if(dbgopt) --dbglevel; } break;
case 259:
# line 968 "gram.in"
{ yyval.pval = mkiost(yypvt[-5].ival, yypvt[-3].pval, yypvt[-1].pval); } break;
case 260:
# line 971 "gram.in"
{ yyval.ival = 0; } break;
case 261:
# line 972 "gram.in"
{ yyval.ival = 1; } break;
case 262:
# line 973 "gram.in"
{ yyval.ival = 2; } break;
case 263:
# line 974 "gram.in"
{ yyval.ival = 3; } break;
case 265:
# line 978 "gram.in"
{ yyval.pval = NULL; } break;
case 266:
# line 981 "gram.in"
{ yyval.pval = mkchain(yypvt[-0].pval,CHNULL); } break;
case 267:
# line 982 "gram.in"
{ hookup(yypvt[-2].pval, mkchain(yypvt[-0].pval,CHNULL)); } break;
case 268:
# line 985 "gram.in"
{ yyval.pval = mkioitem(yypvt[-0].pval,CNULL); } break;
case 269:
# line 986 "gram.in"
{ yyval.pval = mkioitem(yypvt[-2].pval,yypvt[-0].pval); } break;
case 270:
# line 987 "gram.in"
{ yyval.pval = mkioitem(PNULL,yypvt[-0].pval); } break;
case 271:
# line 988 "gram.in"
{ yyval.pval = mkiogroup(yypvt[-0].pval, CNULL, PNULL); } break;
case 272:
# line 989 "gram.in"
{ yyval.pval = mkiogroup(yypvt[-0].pval, CNULL, yypvt[-1].pval); } break;
case 273:
# line 990 "gram.in"
{ yyval.pval = mkiogroup(yypvt[-2].pval,yypvt[-0].pval,yypvt[-3].pval); } break;
case 274:
# line 991 "gram.in"
{ yyval.pval = mkiogroup(yypvt[-2].pval,yypvt[-0].pval,PNULL); } break;
case 275:
# line 994 "gram.in"
{ ++iobrlevel; } break;
case 276:
# line 995 "gram.in"
{ --iobrlevel;  yyval.pval = yypvt[-1].pval; } break;
case 277:
# line 999 "gram.in"
{ yyval.pval = mkformat(yypvt[-0].ival, PNULL, PNULL); } break;
case 278:
# line 1001 "gram.in"
{ yyval.pval = mkformat(yypvt[-3].ival, yypvt[-1].pval, PNULL); } break;
case 279:
# line 1003 "gram.in"
{ yyval.pval = mkformat(yypvt[-5].ival,yypvt[-3].pval,yypvt[-1].pval); } break;
case 280:
# line 1005 "gram.in"
{ yyval.pval = yypvt[-1].pval->leftp; frexpblock(yypvt[-1].pval); } break;
case 281:
# line 1008 "gram.in"
{ yyval.ival = yypvt[-1].ival; } break;
case 282:
# line 1011 "gram.in"
{ lettneed = YES;} break;
case 283:
# line 1014 "gram.in"
{ lettneed = NO; } break;
	}
	goto yystack;		/* reset registers in driver code */
}
