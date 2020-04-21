/*
 * $Source: /d2/3.7/src/sys/gl1/RCS/dials.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:28:15 $
 */

#define INITDIALBOX	0x20	/* Dial values => 0; Switches => off	*/
				/* Leds => off; All masks cleared	*/
#define DIAGMODE	0x21	/* Enter diagnostic mode		*/
#define CSCHKON		0x22	/* Turns on checksum requirement	*/
#define CSCHKOFF	0x23	/* Turns off checksum			*/

#define ERSATZON	0x24
#define ERSATZOFF	0x25

#define INITDILS	0x26	/* Dials values => 0; ASVAL and ASDELTA	*/
				/* masks are cleared			*/
#define DIALVALUEBASE	0x30	/* 0x30 requests the value of dial 0,	*/
				/* 0x31 the value of dial 1, etc.	*/
#define DIALDELTABASE	0x41
#define ASVAL		0x50	/* automatic send dial changes.  This	*/
				/* is followed by a 2 byte mask.	*/
#define ASDELTA		0x51	/* Similar to ASVAL, but send deltas.	*/
#define FILTER		0x53	/* Set filter value for a single dial.	*/
				/* use: FILTER <dial #> <filterval>	*/
				/* filterval = 0 => 1024 clicks / rev	*/
				/*           = 1 =>  512 clicks / rev   */
				/*		. . .			*/
				/* filterval = 7 =>    8 clicks / rev	*/

#define INITSW		0x28	/* all switches => off			*/
#define TOGTYPE		0x70	/* followed by 4 byte mask to indicate	*/
				/* which switches will be treated as	*/
				/* toggles				*/
#define MOMTYPE		0x71	/* followed by 4 byte mask indicating	*/
				/* momentary-type switches		*/
#define ASTOG		0x72	/* auto send toggles, followed by 4	*/
				/* byte mask				*/
#define ASMOM		0x73	/* auto send momentaries		*/
#define RDSWS		0x4a	/* read switches			*/

#define LEDSALL		0x4b	/* turn on all leds			*/
#define LEDSNONE	0x4c	/* all leds off				*/
#define LEDSW		0x74	/* marry switch to led according to	*/
				/* the 4 byte mask that follows		*/
#define LEDSON		0x75	/* turn on leds for all 1s in the	*/
				/* 4 byte mask that follows		*/

/* Responses from switch/dial box: */

#define BOXINITED	0x20	/* box initialized successfully		*/
#define CHKSUMEN	0x22	/* checksum enabled			*/
#define CHKSUMDIS	0x23	/* checksum disabled			*/

#define DIALSINIT	0x26	/* dials initialized			*/
#define DIALNBASE	0x30
#define DELTANBASE	0x41

#define SWINITIED	0x28
#define SWVAL		0x4a	/* 4 bytes of switch values follow	*/

#define MOMPRESSBASE	0xc0	/* base values for various switdh chars */
#define MOMRELEASEBASE	0xe0
#define TOGONBASE	0x50
#define TOGOFFBASE	0xa0

#define DIALCOUNT	10
#define SWITCHCOUNT	32

#define LIGHTPENX	8
#define LIGHTPENY	9
#define	DIALMAXRAW	0x10000

