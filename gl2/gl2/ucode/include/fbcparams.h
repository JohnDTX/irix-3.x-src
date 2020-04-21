/* fbcparams.h -- for UC4
 * allowable symbols for designated parameters
 */

/* wrcode */
#define REGWRE		1
#define REGWRD		2

/* rdwrtype */
#define RAMRD		10
#define RAMWR		11
#define RAMNOP		12

/* qstatus */
#define QOPERAND	20
#define NONQOP		21

/* moredata */
#define MORE		30
#define NOMORE		31

/* injust */
#define RJUST		40
#define LJUST		41

/* marcode */
#define LOAD		50
#define INC		51
#define DEC		52
#define HOLD		53

/* DIinput - also used for disrc pseudo-field */
#define UCONST		60
#define UCOUNT		61
#define INLJUST		62
#define INRJUST		63
#define OUTPUTREG	64
#define OUTPUTCOUNT	65
#define OUTPUTADDRESS	66
#define BPCDATA		67
#define MULTIBUS	68

/* busparts */
#define ALL16		70
#define HI8		71
#define SWAP		71
#define LO8		72
#define NONE		73

/* qshift */
#define LDQ		80
#define QR		81
#define QL		82
#define OLDQ		83

/* fshift */
#define FLL		90
#define FLR		91
#define FAR		92
#define FAL		93
#define FF		94

/* incr */
#define P0		101
#define P1		102
/*	enhancements for COMP micro-ops	(SUBRSOP) */
#define LE		101
#define GT		101
#define LT		102
#define GE		102
#define EQ		102
#define NQ		102

/* op2903 */
#define ADDOP		110
#define SUBRSOP		111
#define SUBSROP		112
#define FHIGHOP		113
#define SONLYOP		114
#define RONLYOP		115
#define FLOWOP		116
#define ANDOP		117
#define XNOROP		118
#define XOROP		119
#define NOROP		120
#define NANDOP		121
#define IOROP		122
#define COMPROP		123
#define COMPSOP		124

/* condtype */
#define IFFALSE		130
#define IFTRUE		131
#define IFNZ		132
#define IFNQ		132
#define IFNEG		133
#define IFLT		133
#define IFLE		133
#define IFOVF		134
#define IFZ		135
#define IFEQ		135
#define IFNNEG		136
#define IFGT		136
#define IFGE		136
#define IFNFLAG		137

/* op2910 */
#define CONT		140
#define JSUB		141
#define JUMP		142
#define PUSH		143
#define JSRP		144
#define VECT		145
#define CJRP		146
#define LOUP		147
#define RPCT		148
#define RETN		149
#define CJPP		150
#define THWB		151
#define LDCT		152
#define TEST		153
#define JMAP		154
#define JZER		155

/* BPCcommand */
/*		strobes		*/
#define LOADED		160
#define LOADEC		161
#define LOADXS		162
#define LOADXE		163
#define LOADYS		164
#define LOADYE		165
#define LOADFA		166
#define LOADSAF		167
#define LOADSAI		168
#define LOADEAF		169
#define LOADEAI		170
#define LOADSDF		171
#define LOADSDI		172
#define LOADEDF		173
#define LOADEDI		174
#define LOADMODE	175
#define LOADREPEAT	176
#define LOADCONFIG	177
/*		commands	*/
#define READFONT	178
#define WRITEFONT	179
#define READREPEAT	180
#define SETADDRS	181
#define SAVEWORD	182
#define DRAWWORD	183
#define READLSTIP	184
#define NOOP		185
#define DRAWCHAR	186
#define FILLRECT	187
#define FILLTRAP	188
#define OCT0VECT	189
#define OCT1VECT	190
#define OCT2VECT	191
#define OCT3VECT	192
#define SETSCRMASKX	193
#define SETSCRMASKY	194
#define SETCOLORAB	195
#define SETCOLORCD	196
#define SETWEAB		197
#define SETWECD		198
#define READPIXELAB	199
#define READPIXELCD	200
#define DRAWPIXELAB	201
#define DRAWPIXELCD	202
#define OCT0RVECT	203
#define OCT1RVECT	204
#define OCT2RVECT	205
#define OCT3RVECT	206

/* etctype */
#define INTERRUPT	220
#define FBREAD		221

/* seqtype (pseudo-field) */
#define BRANCH		1
#define COUNTER		2
#define NONBRANCH	3
#define SPECIAL		4
