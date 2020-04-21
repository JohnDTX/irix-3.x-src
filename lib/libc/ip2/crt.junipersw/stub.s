
|* This module is required only so that we can build "boot1"
|* using the old (release 3.4 ) version of cc that lives in
|* the root of SLsrc.  In rel 3.4 and before there was a
|* module called /usr/lib/junipersw.o that contained the
|* imath routines.  In release 3.5 the compiler generates
|* inline imath code.  The imath routiens are moved into libc
|* so that they can be linked in on an as needed basis.  This
|* is just to be nice to the customer, since if he just does
|* a "make clean; make all" then the imath routines are no
|* longer needed.

