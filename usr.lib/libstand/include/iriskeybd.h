/*
 * definitions for the SGI up/down encoded Keytronics keyboard
 */
#ifndef KEYBOARD
#define KEYBOARD

#define ResetKeybd	0x10		/* reset the 8748 inside the keybd -
					   causes a 0xAA to be sent back  */

#define BeepCmd	0x0			/* low bit clear -> beep commands */
#define LEDCmd	0x1			/* low bit set -> LED commands */

/* Beeper commands */

#define ShortBeep	(BeepCmd | 0x2)
#define LongBeep	(BeepCmd | 0x4)
#define ClickEnable	0x0
#define ClickDisable	0x8

/* LED commands */

#define KEY_LED0	(LEDCmd) 		/* LED 0,1 are inverses */
#define	KEY_LED1	(LEDCmd | 0x2)
#define	KEY_LED2	(LEDCmd | 0x4)
#define	KEY_LED3	(LEDCmd | 0x8)
#define	KEY_LED4	(LEDCmd | 0x10)
#define	KEY_LED5	(LEDCmd | 0x20)
#define	KEY_LED6	(LEDCmd | 0x40)
#define KEY_LED7	(LEDCmd | 0x80)


#endif
