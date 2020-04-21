/**************************************************************************
 *									  *
 * 		 Copyright (C) 1984, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/
#include <sys/types.h>
#include "pxw.h"

char tbl1[] = {"Display_xlat"};
/* the text transfer uses EM to represent the ascii HT */
/*		EBCDIC cent (4a) is ASCII [ (5b)	*/
/*		EBCDIC solid bar (4f) is ASCII ] (5d)	*/
unsigned char Display_xlat[] = {
       	      ' ',0x09,0x0c,0x0a,0x00,0x0d,' ',' ', /* NUL,EM,FF,NL,0xde,CR,0 */
       	      '>','<','[',']',')','(','}','{',         /* 08-0f */
       	      ' ','=','\'','"','/','\\','|','.',  /* 10-17 17 was 0x0a '.' wpc*/
       	      '?','!','$','.','.','.','t','0', /* 18-1f '.' cent,pound,Yen wpc*/
       	      '0','1','2','3','4','5','6','7',         /* 20-27 */
              '8','9','B','S','#','@','%','_',         /* 28-2f */
       	      '&','-','.',',',':','+','^','.', /* 30-37 36 not^ 37 ovrbar. wpc*/
       	      '.',' ','^','~','"','`','\'',',',        /* 38-3f */
/* foreign language characters */
       	      'a','e','i','o','u','a','o','y',         /* 40-47 */
       	      'a','e','e','i','o','u','u',',',         /* 48-4f */
       	      'a','e','i','o','u','a','e','i',         /* 50-57 */
       	      'o','u','a','e','i','o','u','n',         /* 58-5f */
       	      'A','E','I','O','U','A','O','Y',         /* 60-67 */
	      'A','E','E','I','O','U','Y','C',         /* 68-6f */
       	      'A','E','I','O','U','A','E','I',         /* 70-77 */
       	      'O','U','A','E','I','O','U','N',         /* 78-7f */
/* letters */
             'a','b','c','d','e','f','g','h',          /* 80-87 */
	     'i','j','k','l','m','n','o','p',          /* 88-8f */
             'q','r','s','t','u','v','w','x',          /* 90-97 */
	     'y','z',0x0a,'0','a','c',';','*',         /* 98-9f 9a for NL*/
             'A','B','C','D','E','F','G','H',          /* a0,a7 */
             'I','J','K','L','M','N','O','P',          /* a8-af */
             'Q','R','S','T','U','V','W','X',          /* b0-b7 */
	     'Y','Z','.','0','A','C',';','*',          /* b8-bf */
/* indicators */
       	      ' ',' ',' ',' ',' ',' ',' ',' ',         /* c0-c7 */
       	      ' ',' ',' ',' ',' ',' ',' ',' ',         /* c8-cf */
             'P','S','a','^','b','6','>','=',          /* d0-d7 */
             '>','\\','^','O','B','v','?','c',         /* d8-df */
       	      ' ',' ',' ',' ',' ',' ',' ',' ',         /* e0-e7 */
       	      ' ',' ',' ',' ',' ',' ',' ',' ',         /* e8-ef */
             '+','-','z','_','(',')','X','=',          /* f0-f7 */
             '<','\\','k','e','4','A','-','O'          /* f8-ff */
	};


/*	used to provide EBCDIC status line in display.c */
unsigned char Ebx_x8[] = {
		0,0x19,0x0c,0x15,0x3f,0x0d,0,0,		/*00-07*/
		0x6e,0x4c,0,0,0x5d,0x4d,0xd0,0xc0,	/*08-0f*/
		0x40,0x7e,0x7d,0x7f,0x61,0xe0,0x4f,0x6a, /*10-17*/
		0x6f,0x5a,0x5b,0x4a,0,0,0,0,		/*18-1f*/
		0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7, /*20-27*/
		0xf8,0xf9,0,0,0x7b,0x7c,0x6c,0x6d, 	/*28-2f*/
		0x50,0x60,0x4b,0x6b,0x7a,0x4e,0x5f,0x60, /*30-37*/
		0,0,0,0xa1,0,0x79,0,0,			/*38-3f*/
		0,0,0,0,0,0,0,0,			/*40-47*/
		0,0,0,0,0,0,0,0,			/*48-4f*/
		0,0,0,0,0,0,0,0,			/*50-57*/
		0,0,0,0,0,0,0,0,			/*58-5f*/
		0,0,0,0,0,0,0,0,			/*60-67*/
		0,0,0,0,0,0,0,0,			/*68-6f*/
		0,0,0,0,0,0,0,0,			/*70-77*/
		0,0,0,0,0,0,0,0,			/*78-7f*/
		0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88, /*80-87*/
		0x89,0x91,0x92,0x93,0x94,0x95,0x96,0x97, /*88-8f*/
		0x98,0x99,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7, /*90-97*/
		0xa8,0xa9,0,0,0,0,0,0,			/*98-9f*/
		0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8, /*a0-a7*/
		0xc9,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7, /*a8-af*/
		0xd8,0xd9,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7, /*b0-b7*/
		0xe8,0xe9,0,0,0,0,0x5e,0x5c,		/*b8-bf*/
		0,0,0,0,0,0,0,0,			/*c0-c7*/
		0,0,0,0,0,0,0,0,			/*c8-cf*/
		0xd7,0xe2,0x81,0xc1,0x82,0xf6,0x6e,0x60, /*d0-d7*/
		0x6e,0xe0,0xc1,0xd6,0xc2,0xe5,0x6f,0xc3, /*d8-df*/
		0,0,0,0,0,0,0,0,			/*e0-e7*/
		0,0,0,0,0,0,0,0,			/*e8-ef*/
		0xe0,0x60,0xa9,0x6d,0xc3,0xc4,0xe7,0xa1, /*f0-f7*/
		0x4c,0xe0,0x92,0x85,0xf4,0xc1,0x60,0x96  /*f8-ff*/
		};


unsigned char Asc2ebc[] = {
		0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40, /* 0- 7*/
		0x40,0x05,0x15,0x40,0x40,0x0c,0x0d,0x40, /* 8- f*/
		0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40, /*10-17*/
		0x40,0x19,0x40,0x40,0x40,0x40,0x40,0x40, /*18-1f*/
		0x40,0x5a,0x7f,0x7b,0x5b,0x6c,0x50,0x7d, /*20-27*/
		0x4d,0x5d,0x5c,0x4e,0x6b,0x60,0x4b,0x61, /*28-2f*/
		0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7, /*30-37*/
		0xf8,0xf9,0x7a,0x5e,0x4c,0x7e,0x6e,0x6f, /*38-3f*/
		0x7c,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7, /*40-47*/
		0xc8,0xc9,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6, /*48-4f*/
		0xd7,0xd8,0xd9,0xe2,0xe3,0xe4,0xe5,0xe6, /*50-57*/
		0xe7,0xe8,0xe9,0x41,0xe0,0x42,0x5f,0x6d, /*58-5f*/
		0x79,0x81,0x82,0x83,0x84,0x85,0x86,0x87, /*60-67*/
		0x88,0x89,0x91,0x92,0x93,0x94,0x95,0x96, /*68-6f*/
		0x97,0x98,0x99,0xa2,0xa3,0xa4,0xa5,0xa6, /*70-77*/
		0xa7,0xa8,0xa9,0xc0,0x4f,0xd0,0xa1,0x40  /*78-7f*/
		};


#define ENUL 0x2d
unsigned char Ebc2asc[] = {
		0,' ',' ',' ',' ',' ',' ',' ',		/*00-07*/
		' ',' ',' ',' ',0x0c,0x0d,' ',' ',	/*08-0f*/
		' ',' ',' ',' ',' ',0x0a,' ',' ',	/*10-17*/
		' ',0x09,' ',' ',' ',' ',' ',' ',	/*18-1f*/
		' ',' ',' ',' ',' ',' ',' ',' ',	/*20-27*/
		' ',' ',' ',' ',' ',' ',' ',' ',	/*28-2f*/
		' ',' ',' ',' ',' ',' ',' ',' ',	/*30-37*/
		' ',' ',' ',' ',' ',' ',' ',' ',	/*38-3f*/
		' ','[',']',ENUL,ENUL,ENUL,ENUL,ENUL,   /*40-47*/
		ENUL,ENUL,'c','.','<','(','+','|',	/*48-4f*/
		'&',ENUL,ENUL,ENUL,ENUL,ENUL,ENUL,ENUL,	/*50-57*/
		ENUL,ENUL,'!','$','*',')',';','^',	/*58-5f*/
		'-','/',ENUL,ENUL,ENUL,ENUL,ENUL,ENUL,	/*60-67*/
		ENUL,ENUL,'|',',','%','_','>','?',	/*68-6f*/
		ENUL,ENUL,ENUL,ENUL,ENUL,ENUL,ENUL,ENUL,/*70-77*/
		ENUL,'`',':','#','@','\'','=','"',	/*78-7f*/
		ENUL,'a','b','c','d','e','f','g',	/*80-87*/
		'h','i',ENUL,ENUL,ENUL,ENUL,ENUL,ENUL,	/*88-8f*/
		ENUL,'j','k','l','m','n','o','p',	/*90-97*/
		'q','r',ENUL,ENUL,ENUL,ENUL,ENUL,ENUL,	/*98-9f*/
		ENUL,'~','s','t','u','v','w','x',	/*a0-a7*/
		'y','z',ENUL,ENUL,ENUL,ENUL,ENUL,ENUL,	/*a8-af*/
		ENUL,ENUL,ENUL,ENUL,ENUL,ENUL,ENUL,ENUL,/*b0-b7*/
		ENUL,ENUL,ENUL,ENUL,ENUL,ENUL,ENUL,ENUL,/*b8-bf*/
		'{','A','B','C','D','E','F','G',	/*c0-c7*/
		'H','I',ENUL,ENUL,ENUL,ENUL,ENUL,ENUL,	/*c8-cf*/
		'}','J','K','L','M','N','O','P',	/*d0-d7*/
		'Q','R',ENUL,ENUL,ENUL,ENUL,ENUL,ENUL,	/*d8-df*/
		'\\',ENUL,'S','T','U','V','W','X',	/*e0-e7*/
		'Y','Z',ENUL,ENUL,ENUL,ENUL,ENUL,ENUL,	/*e8-ef*/
		'0','1','2','3','4','5','6','7',	/*f0-f7*/
		'8','9',ENUL,ENUL,ENUL,ENUL,ENUL,ENUL	/*f8-ff*/
		};





char tbl2[] = {"Asc_xlat"};
/* the text transfer uses 3274 EM to represent the ascii HT */
unsigned char Asc_xlat[] = {		/* ASCII to 3274 xlat table */
		0,0,0,0,0,0,0,0,	/*NUL,SOH,STX,ETX,EOT,ENQ,ACK,BEL*/
		0,1,3,0,2,5,0,0,	/*BS,HT,LF,VT,FF,CR,SO,SI*/
		0,0,0,0,0,0,0,0,	/*DLE,DC1,DC2,DC3,DC4,NAK,SYN,ETB*/
		0,0,0,0,0,0,0,0,	/*CAN,EM,SUB,ESC,FS,GS,RS,US*/
		0x10,	/* SP */
		0x19,	/* ! */
		0x13,	/* " */
		0x2c,	/* # */
		0x1a,	/* $ */
		0x2e,	/* % */
		0x30,	/* & */
		0x12,	/* ' */
		0x0d,	/* ( */
		0x0c,	/* ) */
		0xbf,	/* * */
		0x35,	/* + */
		0x33,	/* , */
		0x31,	/* - */
		0x32,	/* . */
		0x14,	/* / */
		0x20,	/* 0 */
		0x21,	/* 1 */
		0x22,	/* 2 */
		0x23,	/* 3 */
		0x24,	/* 4 */
		0x25,	/* 5 */
		0x26,	/* 6 */
		0x27,	/* 7 */
		0x28,	/* 8 */
		0x29,	/* 9 */
		0x34,	/* : */
		0xbe,	/* ; */
		0x09,	/* < */
		0x11,	/* = */
		0x08,	/* > */
		0x18,	/* ? */
		0x2d,	/* @ */
		0xa0,	/* A */
		0xa1,	/* B */
		0xa2,	/* C */
		0xa3,	/* D */
		0xa4,	/* E */
		0xa5,	/* F */
		0xa6,	/* G */
		0xa7,	/* H */
		0xa8,	/* I */
		0xa9,	/* J */
		0xaa,	/* K */
		0xab,	/* L */
		0xac,	/* M */
		0xad,	/* N */
		0xae,	/* O */
		0xaf,	/* P */
		0xb0,	/* Q */
		0xb1,	/* R */
		0xb2,	/* S */
		0xb3,	/* T */
		0xb4,	/* U */
		0xb5,	/* V */
		0xb6,	/* W */
		0xb7,	/* X */
		0xb8,	/* Y */
		0xb9,	/* Z */
		0x0a,	/* [ */
		0x15,	/* \ */
		0x0b,	/* ] */
		0x36,	/* logical not; use ^ carat to represent  .... wpc */
		0x2f,	/* _ */
		0x3d,	/* ` */
		0x80,	/* a */
		0x81,	/* b */
		0x82,	/* c */
		0x83,	/* d */
		0x84,	/* e */
		0x85,	/* f */
		0x86,	/* g */
		0x87,	/* h */
		0x88,	/* i */
		0x89,	/* j */
		0x8a,	/* k */
		0x8b,	/* l */
		0x8c,	/* m */
		0x8d,	/* n */
		0x8e,	/* o */
		0x8f,	/* p */
		0x90,	/* q */
		0x91,	/* r */
		0x92,	/* s */
		0x93,	/* t */
		0x94,	/* u */
		0x95,	/* v */
		0x96,	/* w */
		0x97,	/* x */
		0x98,	/* y */
		0x99,	/* z */
		0x0f,	/* { */
		0x16,	/* | to solid bar */
		0x0e,	/* } */
		0x3b,	/* ~ */
		0x00 	/* del */
	};



unsigned char Binxlat[] = {
	0xa0,0xa1,0xa2,0xa3,
	0xa4,0xa5,0xa6,0xa7,	/* 00 07*/
	0xa8,0xa9,0xaa,0xab,
	0xac,0xad,0xae,0xaf,	/* 08 0f*/
	0xb0,0xb1,0xb2,0xb3,
	0xb4,0xb5,0xb6,0xb7,	/* 10 17*/
	0xb8,0xb9,0x80,0x81,
	0x82,0x83,0x84,0x85,	/* 18 1f*/
	0x86,0x87,0x88,0x89,
	0x8a,0x8b,0x8c,0x8d,	/* 20 27*/
	0x8e,0x8f,0x90,0x91,
	0x92,0x93,0x94,0x95,	/* 28 2f*/
	0x96,0x97,0x98,0x99,
	0x20,0x21,0x22,0x23,	/* 30 37*/
	0x24,0x25,0x26,0x27,
	0x28,0x29,0x32,0x33	/* 38 3f*/
};


unsigned char Ebx_xlat[] = {		/* EBCDIC to 3274 xlat table */
		0,' ',' ',' ',' ',' ',' ',' ',		/*00-07*/
		' ',' ',' ',' ',0x02,0x05,' ',' ',	/*08-0f*/
		' ',' ',' ',' ',' ',0x03,' ',' ',	/*10-17*/
		' ',0x01,' ',' ',' ',' ',' ',' ',	/*18-1f*/
		' ',' ',' ',' ',' ',' ',' ',' ',	/*20-27*/
		' ',' ',' ',' ',' ',' ',' ',' ',	/*28-2f*/
		' ',' ',' ',' ',' ',' ',' ',' ',	/*30-37*/
		' ',' ',' ',' ',' ',' ',' ',' ',	/*38-3f*/
		0x10,	/* SP */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0x1b,	/* cent */
		0x32,	/* . */
		0x09,	/* < */
		0x0d,	/* ( */
		0x35,	/* + */
		0x16,	/* solid | */
		0x30,	/* & */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0x19,	/* ! */
		0x1a,	/* $ */
		0xbf,	/* * */
		0x0c,	/* ) */
		0xbe,	/* ; */
		0x36,	/* not */
		0x31,	/* - */
		0x14,	/* / */
		0, 0, 0,  /* EBCDIC 0x62, 0x63, 0x64 ............... wpc */
		0x46,     /* o tilde for Inbound control EBCDIC 0x65 wpc */
		0,0,0,0,  /* EBCDIC 0x66, 0x67, 0x68, 0x69 ......... wpc */
		0x17,	/* broken vertical bar, was 0x16 now 0x17 .. wpc */
		0x33,	/* , */
		0x2e,	/* % */
		0x2f,	/* _ */
		0x08,	/* > */
		0x18,	/* ? */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0x3d,	/* ` */
		0x34,	/* : */
		0x2c,	/* # */
		0x2d,	/* @ */
		0x12,	/* ' */
		0x11,	/* = */
		0x13,	/* " */
		0,
		0x80,	/* a */
		0x81,	/* b */
		0x82,	/* c */
		0x83,	/* d */
		0x84,	/* e */
		0x85,	/* f */
		0x86,	/* g */
		0x87,	/* h */
		0x88,	/* i */
		0, 0, 0, 0, 0, 0, 
		0,
		0x89,	/* j */
		0x8a,	/* k */
		0x8b,	/* l */
		0x8c,	/* m */
		0x8d,	/* n */
		0x8e,	/* o */
		0x8f,	/* p */
		0x90,	/* q */
		0x91,	/* r */
		0, 0, 0, 0, 0, 0, 
		0, 0x3b,/* ~ */
		0x92,	/* s */
		0x93,	/* t */
		0x94,	/* u */
		0x95,	/* v */
		0x96,	/* w */
		0x97,	/* x */
		0x98,	/* y */
		0x99,	/* z */
		0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0x0f,	/* { */
		0xa0,	/* A */
		0xa1,	/* B */
		0xa2,	/* C */
		0xa3,	/* D */
		0xa4,	/* E */
		0xa5,	/* F */
		0xa6,	/* G */
		0xa7,	/* H */
		0xa8,	/* I */
		0, 0, 0, 0, 0, 0, 
		0x0e,	/* } */
		0xa9,	/* J */
		0xaa,	/* K */
		0xab,	/* L */
		0xac,	/* M */
		0xad,	/* N */
		0xae,	/* O */
		0xaf,	/* P */
		0xb0,	/* Q */
		0xb1,	/* R */
		0, 0, 0, 0, 0, 0, 
		0x15, 0,/* \ */
		0xb2,	/* S */
		0xb3,	/* T */
		0xb4,	/* U */
		0xb5,	/* V */
		0xb6,	/* W */
		0xb7,	/* X */
		0xb8,	/* Y */
		0xb9,	/* Z */
		0, 0, 0, 0, 0, 0, 
		0x20,	/* 0 */
		0x21,	/* 1 */
		0x22,	/* 2 */
		0x23,	/* 3 */
		0x24,	/* 4 */
		0x25,	/* 5 */
		0x26,	/* 6 */
		0x27,	/* 7 */
		0x28,	/* 8 */
		0x29,	/* 9 */
		0, 0, 0, 0, 0, 0 
	};
char tbl3[] = {"FxDispl_xlat"};    /* add this for File Transfer        wpc*/
/* the text transfer uses EM to represent the ascii HT ..File Transfer  wpc*/
/*		EBCDIC cent (4a) is ASCII [ (5b)       ..File Transfer  wpc*/
/*		EBCDIC solid bar (4f) is ASCII ] (5d)  ..File Transfer  wpc*/
unsigned char FxDispl_xlat[] = {                    /* ..File Transfer  wpc*/
       	      ' ',0x09,0x0c,0x0a,0x00,0x0d,' ',' ', /* NUL,EM,FF,NL,0xde,CR,0 */
       	      '>','<','[',']',')','(','}','{',         /* 08-0f */
       	      ' ','=','\'','"','/','\\','|',0x85, /* 10-17 170x0a brkn bar wpc*/
       	      '?','!','$',0x81,0x82,0x83,'t','0', /* 18-1f cent,Pound,Yen wpc*/
       	      '0','1','2','3','4','5','6','7',         /* 20-27 */
              '8','9','B','S','#','@','%','_',         /* 28-2f */
       	      '&','-','.',',',':','+','^',0x84, /* 30-37 37 ovrbar is 0x84 wpc*/
       	      '.',' ','^','~','"','`','\'',',',        /* 38-3f */
/* foreign language characters */
       	      'a','e','i','o','u','a','o','y',         /* 40-47 */
       	      'a','e','e','i','o','u','u',',',         /* 48-4f */
       	      'a','e','i','o','u','a','e','i',         /* 50-57 */
       	      'o','u','a','e','i','o','u','n',         /* 58-5f */
       	      'A','E','I','O','U','A','O','Y',         /* 60-67 */
	      'A','E','E','I','O','U','Y','C',         /* 68-6f */
       	      'A','E','I','O','U','A','E','I',         /* 70-77 */
       	      'O','U','A','E','I','O','U','N',         /* 78-7f */
/* letters */
             'a','b','c','d','e','f','g','h',          /* 80-87 */
	     'i','j','k','l','m','n','o','p',          /* 88-8f */
             'q','r','s','t','u','v','w','x',          /* 90-97 */
	     'y','z',0x0a,'0','a','c',';','*',         /* 98-9f 9a for NL*/
             'A','B','C','D','E','F','G','H',          /* a0,a7 */
             'I','J','K','L','M','N','O','P',          /* a8-af */
             'Q','R','S','T','U','V','W','X',          /* b0-b7 */
	     'Y','Z','.','0','A','C',';','*',          /* b8-bf */
/* indicators */
       	      ' ',' ',' ',' ',' ',' ',' ',' ',         /* c0-c7 */
       	      ' ',' ',' ',' ',' ',' ',' ',' ',         /* c8-cf */
             'P','S','a','^','b','6','>','=',          /* d0-d7 */
             '>','\\','^','O','B','v','?','c',         /* d8-df */
       	      ' ',' ',' ',' ',' ',' ',' ',' ',         /* e0-e7 */
       	      ' ',' ',' ',' ',' ',' ',' ',' ',         /* e8-ef */
             '+','-','z','_','(',')','X','=',          /* f0-f7 */
             '<','\\','k','e','4','A','-','O'          /* f8-ff */
	};
char tbl4[] = {"FxAsc_xlat"};       /* Add this for File Transfer          wpc*/
/* the text transfer uses 3274 EM to represent the ascii HT .File Transfer wpc*/
unsigned char FxAsc_xlat[] = {		/* ASCII to 3274 xlat table File   wpc*/
		0,0,0,0,0,0,0,0,	/*NUL,SOH,STX,ETX,EOT,ENQ,ACK,BEL*/
		0,1,3,0,2,5,0,0,	/*BS,HT,LF,VT,FF,CR,SO,SI*/
		0,0,0,0,0,0,0,0,	/*DLE,DC1,DC2,DC3,DC4,NAK,SYN,ETB*/
		0,0,0,0,0,0,0,0,	/*CAN,EM,SUB,ESC,FS,GS,RS,US*/
		0x10,	/* SP */
		0x19,	/* ! */
		0x13,	/* " */
		0x2c,	/* # */
		0x1a,	/* $ */
		0x2e,	/* % */
		0x30,	/* & */
		0x12,	/* ' */
		0x0d,	/* ( */
		0x0c,	/* ) */
		0xbf,	/* * */
		0x35,	/* + */
		0x33,	/* , */
		0x31,	/* - */
		0x32,	/* . */
		0x14,	/* / */
		0x20,	/* 0 */
		0x21,	/* 1 */
		0x22,	/* 2 */
		0x23,	/* 3 */
		0x24,	/* 4 */
		0x25,	/* 5 */
		0x26,	/* 6 */
		0x27,	/* 7 */
		0x28,	/* 8 */
		0x29,	/* 9 */
		0x34,	/* : */
		0xbe,	/* ; */
		0x09,	/* < */
		0x11,	/* = */
		0x08,	/* > */
		0x18,	/* ? */
		0x2d,	/* @ */
		0xa0,	/* A */
		0xa1,	/* B */
		0xa2,	/* C */
		0xa3,	/* D */
		0xa4,	/* E */
		0xa5,	/* F */
		0xa6,	/* G */
		0xa7,	/* H */
		0xa8,	/* I */
		0xa9,	/* J */
		0xaa,	/* K */
		0xab,	/* L */
		0xac,	/* M */
		0xad,	/* N */
		0xae,	/* O */
		0xaf,	/* P */
		0xb0,	/* Q */
		0xb1,	/* R */
		0xb2,	/* S */
		0xb3,	/* T */
		0xb4,	/* U */
		0xb5,	/* V */
		0xb6,	/* W */
		0xb7,	/* X */
		0xb8,	/* Y */
		0xb9,	/* Z */
		0x0a,	/* [ */
		0x15,	/* \ */
		0x0b,	/* ] */
		0x36,	/* logical not; use ^ carat to represent.. wpc */
		0x2f,	/* _ */
		0x3d,	/* ` */
		0x80,	/* a */
		0x81,	/* b */
		0x82,	/* c */
		0x83,	/* d */
		0x84,	/* e */
		0x85,	/* f */
		0x86,	/* g */
		0x87,	/* h */
		0x88,	/* i */
		0x89,	/* j */
		0x8a,	/* k */
		0x8b,	/* l */
		0x8c,	/* m */
		0x8d,	/* n */
		0x8e,	/* o */
		0x8f,	/* p */
		0x90,	/* q */
		0x91,	/* r */
		0x92,	/* s */
		0x93,	/* t */
		0x94,	/* u */
		0x95,	/* v */
		0x96,	/* w */
		0x97,	/* x */
		0x98,	/* y */
		0x99,	/* z */
		0x0f,	/* { */
		0x16,	/* | to solid bar */
		0x0e,	/* } */
		0x3b,	/* ~ */
		0x00,	/* del comma added for table expansion below wpc */
		0x46,   /* o tilde to replace $ as Inbound control wpc */
		0x1b,   /* cent at 0x81   wpc */
		0x1c,   /* Pound at 0x82  wpc */
		0x1d,   /* Yen at 0x83    wpc */
		0x37,   /* over-bar 0x84  wpc */
		0x17    /* broken solid bar 0x85 wpc */
	};
