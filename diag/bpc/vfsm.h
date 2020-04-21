/*
 *	Kurt Akeley					16 May 1983
 *
 *	Definitions which simplify the use of the video finite state
 *	  machine on the display controller board.  Dcdev.h must be
 *	  expanded prior to this file.
 *
 *	Modified on 10/3/84 to include DC4 definitions - deleted DC2
 *	  definitions.  KBA.
 */

/* Command fields - merge into dcr word */
#ifdef DC3
#define VFSM_CLEAR		0
#define VFSM_CYCLE		DCPALCTRL0
#define VFSM_STICKY_CYCLE	(DCPALCTRL0 | DCPALCTRL2)
#define VFSM_EVENT		VFSM_CYCLE
#define VFSM_STICKY_EVENT	VFSM_STICKY_CYCLE
#define VFSM_EVEN_CHECKSUM	DCPALCTRL1
#define VFSM_ODD_CHECKSUM	(DCPALCTRL1 | DCPALCTRL2)
#define VFSM_SET		(DCPALCTRL0 | DCPALCTRL1 | DCPALCTRL2)
#define VFSM_CMND_MASK		(DCPALCTRL0 | DCPALCTRL1 | DCPALCTRL2)
#endif DC3

#ifdef DC4
#define VFSM_CLEAR		0
#define VFSM_STICKY_EVENT_2	DCPALCTRL0
#define VFSM_STICKY_EVENT_4	(DCPALCTRL0 | DCPALCTRL2)
#define VFSM_STICKY_CYCLE_2	DCPALCTRL1
#define VFSM_STICKY_CYCLE_4	(DCPALCTRL1 | DCPALCTRL2)
#define VFSM_SET		(DCPALCTRL0 | DCPALCTRL1 | DCPALCTRL2)
#define VFSM_CMND_MASK		(DCPALCTRL0 | DCPALCTRL1 | DCPALCTRL2)
#endif DC4

/* Macro to adjust dcr readback for use with definitions below */
#define VFSM_ADJUST(dcr)	((~(dcr>>8))&0xff)

/* Return value masks - AND with dcr readback (with VFSM_ADJUST!) */
#ifdef DC3
#define VFSM_EVEN_EVENT_MASK	0x01
#define VFSM_ODD_EVENT_MASK	0x02
#define VFSM_EVEN_CYCLE_MASK	0x0c
#define VFSM_ODD_CYCLE_MASK	0x30
#define VFSM_EVEN_CHECKSUM_MASK	0xfd
#define VFSM_ODD_CHECKSUM_MASK	0xfe
#define VFSM_ALL_MASK		0xff
#define VFSM_EVENT_MASK		(VFSM_EVEN_EVENT_MASK | VFSM_ODD_EVENT_MASK)
#define VFSM_CYCLE_MASK		(VFSM_EVEN_CYCLE_MASK | VFSM_ODD_CYCLE_MASK)
#define VFSM_CLEAR_MASK		VFSM_ALL_MASK
#define VFSM_SET_MASK		VFSM_ALL_MASK
#endif DC3

#ifdef DC4
#define VFSM_0_EVENT_MASK	0x01
#define VFSM_1_EVENT_MASK	0x02
#define VFSM_2_EVENT_MASK	0x04
#define VFSM_3_EVENT_MASK	0x08
#define VFSM_EVEN_EVENT_MASK	VFSM_0_EVENT_MASK
#define VFSM_ODD_EVENT_MASK	VFSM_1_EVENT_MASK
#define VFSM_0_CYCLE_MASK	0x01
#define VFSM_1_CYCLE_MASK	0x02
#define VFSM_2_CYCLE_MASK	0x04
#define VFSM_3_CYCLE_MASK	0x08
#define VFSM_EVEN_CYCLE_MASK	VFSM_0_CYCLE_MASK
#define VFSM_ODD_CYCLE_MASK	VFSM_1_CYCLE_MASK
#define VFSM_EVEN_CHECKSUM_MASK	0
#define VFSM_ODD_CHECKSUM_MASK	0
#define VFSM_ALL_MASK		0xff
#define VFSM_EVENT_MASK		(VFSM_EVEN_EVENT_MASK | VFSM_ODD_EVENT_MASK)
#define VFSM_EVENT_MASK_4	(VFSM_EVENT_MASK | \
				 VFSM_2_EVENT_MASK | VFSM_3_EVENT_MASK)
#define VFSM_CYCLE_MASK		(VFSM_EVEN_CYCLE_MASK | VFSM_ODD_CYCLE_MASK)
#define VFSM_CYCLE_MASK_4	(VFSM_CYCLE_MASK | \
				 VFSM_2_CYCLE_MASK | VFSM_3_CYCLE_MASK)
#define VFSM_CLEAR_MASK		VFSM_ALL_MASK
#define VFSM_SET_MASK		VFSM_ALL_MASK
#endif DC4

/* Colormap values to control bits 0 and 1 */
#define VFSM_00			0	/* colormap values to use with	*/
#define VFSM_01			1	/*   the dcpal.  Right digit is	*/
#define VFSM_10			2	/*   bit 0, left is bit 1	*/
#define VFSM_11			3
