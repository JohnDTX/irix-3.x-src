#include <sys/types.h>
#include "pxw.h"
#include "io_global.h"
#include "io_data.h"


static char ident[] = "@(#) User Display Process";

extern int		errno;
extern px_status	outb;
extern rglft		rrcv;
#ifdef GL2TERM
	extern int	fid;
#else
	extern int	fd;
#endif
extern int		Pxd_dma_size;

Io_init()
{
		qdevice(MOUSE3);
		qdevice(MOUSE1);
		qdevice(MOUSE2);
		/* initialize 3270 comm */
		i = pxdopen();
		io_outb();
		if (i == -1)
		{
			/* show that comm is dead */

}

/* Function name:	Io_intr
 *
 * Purpose:	Kill the function process
 *
 * Input parameters:
 *	None
 *
 * Output parameters:
 *	None
 *
 * Global variables:
 *
 * Calls:
 *
 * Called by:
 *
 * Algorithm:
 */

Io_intr()
{

	strcpy(VAR cmd, "\ Hey, Mainframe Process!");
	force_ebx_str(cmd);
}
/* Function name:	io_outb
 *
 * Purpose:	Set up outbound pointer for comm code
 *
 * Input parameters:
 *	None
 *
 * Output parameters:
 *	None
 *
 * Global variables:
 *
 * Calls:
 *
 * Called by:
 *
 * Algorithm:
 */

io_outb()
{

	/* Declare our receive buffer to the comm code */
	rrcv.bodyaddr = (u_char *)pxl.dma_buf;
	rrcv.bodylen = Pxd_dma_size-3;
	(void)set_rglout_ptr(&rrcv);
}
 /*
 * Input parameters:
 *	None
 *
 * Output parameters:
 *	abp	Pointer to pointer to message buffer.  The buffer is
 *		managed by this routine.  This parameter is used to
 *		tell the caller its address.
 *	return value
 *		number of bytes read
 *
 * Global variables:
 *
 * Calls:
 *
 * Called by:	Io_read
 *
 * Algorithm:
 *	while we have a command to send
 *		try to send it
 *		if we got a read while trying to send it
 *			go to got_read
 *		unlink the command
 *	read the message
 *  got_read:
 *	if the message is continued
 *		copy it into an allocated buffer
 *	while the message is continued
 *		read the message
 *	return the number of bytes read
 */

io_ipc(VAR abp)
{


	/* Declare our receive buffer to the comm code */

	/* If we don't have an unsent buffer of commands left over from last
	 * time, fill it with any commands that we have
	 */

		/* Loop through our list of commands */
			/* Make sure the next command will fit */

			/* Put the command into the buffer, preceded by the
			 * flag characters
			 */

			/* Unlink the command from the list */

			/* Show that we've sent a command  */

			/* Show that we now have something in the buffer */

	/* If we have a buffer of commands, either from last time or from
	 * this time, try to send it.  I say "try"
	 * because when we make the attempt we may get incoming data back
	 * instead of having sent the command.  If that happens, abandon
	 * the command queue for now and process the incoming data.
	 */

		/* If we got an error, either on the write or the read,
		 * complain, and discard what we tried to send.
		 */

		/* If we got a successful read, go process it.  In this
		 * case, the buffer we tried to send was not actually
		 * sent.  So leave it marked unsent, and we'll try
		 * again the next time we get back this way.
		 */

		/* The write was successful.  Show that it has been sent. */

	/* We're done sending any commands we have to.
	 * Set return value to zero for quick exit on error.
	 */

	/* See if anything was read into our buffer for us */
	n = update3270();

	/* If nothing was read, just return */
	if (n == 0L)
		goto done;

	/* Something happened.  See what. */
	if (n == -1L)

		/* Try to prime the comm again */
		i = Io_purg();

	/* copy the data into the buffer */

	/* As long as the current header says the current message is
	 * continued, keep reading another message
	 */

		/* Check for too many continuations */

			/* We have a good buffer */

		/* add to the allocation */

		/* copy the current message into the buffer */


	/* We've successfully read the entire message.  Return the
	 * number of bytes.
	 */

/* Function name:	Io_purg
 *
 * Purpose:	Try to revive comm
 *
 * Input parameters:
 *	None
 *
 * Output parameters:
 *	None
 *
 * Global variables:
 *
 * Calls:
 *
 * Called by:
 *
 * Algorithm:
 *	close comm
 *	re-open comm
 */

Io_purg()
{
	int i;

		pxdclose();
		force_open_init();
		i = pxdopen();
}

/* Function name:	Io_shut
 *
 * Purpose:	Shut down communications
 *
 * Input parameters:
 *	None
 *
 * Output parameters:
 *	None
 *
 * Global variables:
 *
 * Calls:
 *
 * Called by:	
 *
 * Algorithm:
 *	shut down comm
 */

Io_shut()
{
		pxdclose();
	unqdevice(MOUSE3);
	unqdevice(MOUSE1);
	unqdevice(MOUSE2);
}


/* Function name:	type_str
 *
 * Purpose:	type an ascii string to the 3274
 *
 * Input parameters:
 *	pointer to null terminated string of ascii
 *
 * Output parameters:
 *	-1 for error
 *
 * Global variables:
 *
 * Calls:
 *	pdelay, get3270, getspot, send_key, send_x_key
 *
 * Called by:	
 *
 * Algorithm:
 *	translate and type each character, then send ENTER
 */

type_str(pointer)

type_str(str, len)
u_char *str;
long len;
{
	int rshots = 3;
	u_char c;

	kill_outb();
reset_again:
	if (Sure_reset())
		return -1;
	send_x_key(X_ER_EOF);
	pdelay(MS_20);			/* time for nano to erase screen */
	get3270();
	if (getspot(8)) {
		if (rshots--)
			goto reset_again;
	}
	while (len--)
		send_key(Asc_xlat[*str++]);
	send_x_key(X_ENTER);
}
