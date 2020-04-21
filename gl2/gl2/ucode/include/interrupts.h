/* interrupts.h
 *
 * FBC interrupt code definitions for GL2 microcode
 */

#define _INTHIT		1		/* hit detected	*/
#define _INTPIXEL	3		/* pixel data readback	*/
#define _INTILLEGAL	5		/* illegal command	*/
#define _INTCHPOSN	7		/* read char posn 	*/
#define _INTEOF		9		/* EOF command token 	*/
#define _INTPIXEL32	10		/* 32-word pixel readback */
#define _INTPIXELRGB	11		/* RGB (32-word) pixel readback */
#define _INTDUMP	12		/* DUMPuP command	*/
#define _INTXFORMPT	14		/* XFORM_POINT command	*/
#define _INTUNIMPL	15		/* unimplemented command */
#define _INTBPCBUS	16		/* readback of BPC font ram data */
#define _INTRGBPIXEL	17		/* 32-plane pixel data readback	*/
#define _INTCURSOR	19		/* cursor signal received; want Y */
#define _INTFEEDBACK	20		/* dumping bufferful	*/
#define _INTDIVIDE	21		/* division exception	*/

